#include "Func.h"
/*  ST库  */
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
/*  OS库  */
#include "FreeRTOS.h"
#include "task.h"
/*  外设库  */
#include "U_USART.h"
#include "ov7670.h"
#include "TFT_ST7789V.h"
#include "bmp.h"
/*  FATFS  */
#include "ff.h"
/*  宏定义数据  */
#include "cmr_def.h"

extern uint16_t pic_index;
extern uint8_t camera_data[];
uint16_t	photo_index[234];
int16_t 	photo_index_index = 0;
int8_t 		camera_on = 1;
void SD_FindMaxNum(void)
{
	DIR dp;
	FILINFO info;
	f_opendir(&dp,"0:/f");
	f_readdir(&dp,&info);
	pic_index = BMP_StringToNum(info.fname);
	uint16_t temp_num = 0;
	while(info.fname[0]!='\0')
	{
		f_readdir(&dp,&info);
		temp_num = BMP_StringToNum(info.fname);
		pic_index = pic_index>temp_num?pic_index:temp_num;
	}
	f_closedir(&dp);
	U_Printf("SD_FindMaxNum(%s):%d \r\n","0:/f",pic_index);
}
void SD_KeepPhoto(void)
{
	uint8_t path_string[10];
	BMP_NumToString(pic_index+1,(char*)path_string);
	SD_Fast_Write((const char*)path_string,(uint16_t*)&camera_data[0],DEF_PIC_HEIGHT*DEF_PIC_WIDTH);
	pic_index++;
}
void SD_ReadPhoto(uint16_t index)
{
	uint8_t path_string[10];
	BMP_NumToString(index,(char*)path_string);
	SD_Fast_Read((const char*)path_string,(uint16_t*)&camera_data[0],DEF_PIC_HEIGHT*DEF_PIC_WIDTH);
	Func_TFT_Show();
}
const uint16_t d_height	 = DEF_PIC_HEIGHT/3;
const uint16_t DMA_COUNT = DEF_PIC_WIDTH*DEF_PIC_HEIGHT/3*2;
inline void Func_TFT_Show(void)
{
	TFT_SetCursor(DEF_TFT_DX,DEF_TFT_DY,DEF_PIC_WIDTH,d_height);
	TFT_DMA_SetAddr(&camera_data[0]);
	TFT_DMA_Send(DMA_COUNT);
	TFT_SetCursor(DEF_TFT_DX,DEF_TFT_DY+d_height,DEF_PIC_WIDTH,d_height);
	TFT_DMA_SetAddr(&camera_data[DMA_COUNT]);
	TFT_DMA_Send(DMA_COUNT);
	TFT_SetCursor(DEF_TFT_DX,DEF_TFT_DY+d_height*2,DEF_PIC_WIDTH,d_height);
	TFT_DMA_SetAddr(&camera_data[DMA_COUNT*2]);
	TFT_DMA_Send(DMA_COUNT);
}
void Open_Photo_Count(void)
{//把"相册"翻译成"Photo_Count"真是....hhh
	//获取当前全部相片
	DIR dp;
	FILINFO info;
	f_opendir(&dp,"0:/f");
	f_readdir(&dp,&info);
	photo_index[photo_index_index] = BMP_StringToNum(info.fname);
	while(info.fname[0]!='\0')
	{
		f_readdir(&dp,&info);
		photo_index[++photo_index_index] = BMP_StringToNum(info.fname);
	}
	f_closedir(&dp);
	//关闭摄影状态
	camera_on = 0;
	//显示最近的一张图片
	SD_ReadPhoto(photo_index[photo_index_index]);
}
#include "UI_Core.h"
extern int8_t STATUS_ON_UI;
void Task_UI(void* pvParameters)
{
	while(1)
	{
		vTaskDelay(50);
		if(!STATUS_ON_UI)
		{
			vTaskDelay(100);
			return;
		}
		RenderCircle_UI();
	}
}
/**@brief  Func初始化
  */
void Init_Func(void)
{
	SD_FindMaxNum();
	for(int i=0;i<220;i++)
	{
		photo_index[i] = 0;
	}
}
/**@brief  Func线程示例
  */
void Task_Func(void* pvParameters)
{
	while(1)
	{
		vTaskDelay(200);
	}
}
void Init_Light(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC,&GPIO_InitStruct);
	GPIO_WriteBit(GPIOC,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15,Bit_SET);
}
void Init_Button(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOD,&GPIO_InitStruct);
}
int8_t led_on = 0x10;
/*	 		摄影时				相册
 *	右		摄影					index增加
 *	中		打开相册				返回摄影(短按)/删除(长按)
 *	左		灯					index减小
 */
void Task_Button(void* pvParameters)
{
	uint16_t long_count = 0;
	while(1)
	{
		vTaskDelay(50);
		if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_8)==Bit_RESET)
		{//最右侧
			if(camera_on==1)
			{//摄影时
				camera_on=0;
				
				//我真应该把蓝粉白写成一个函数
				uint16_t height = 240/3;
				uint16_t width = 320;
				uint16_t rgb565 = TFT_RGB888To565(0xffc7c7);
				TFT_SetCursor(0,0,width,height);
				for(int i=0;i<width*height;i++)
				{
					TFT_Write16Data(rgb565);
				}
				rgb565 = TFT_RGB888To565(0xf6f6f6);
				TFT_SetCursor(0,height*1,width,height);
				for(int i=0;i<width*height;i++)
				{
					TFT_Write16Data(rgb565);
				}
				rgb565 = TFT_RGB888To565(0x71c9ce);
				TFT_SetCursor(0,height*2,width,height);
				for(int i=0;i<width*height;i++)
				{
					TFT_Write16Data(rgb565);
				}
				
				SD_KeepPhoto();
				camera_on = 1;
			}
			vTaskDelay(10);
			while(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_8)==Bit_RESET);
			vTaskDelay(10);
		}
		else if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_9)==Bit_RESET)
		{//中间
			vTaskDelay(10);
			while(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_9)==Bit_RESET)
			{
				vTaskDelay(10);
				if(long_count++==200)
				{//长按判定
					U_Printf(":)");
				}
			}
			vTaskDelay(10);
			if(long_count<200)
			{
				if(camera_on==0)
				{
					camera_on = 1;
				}
				else
				{
					Open_Photo_Count();
				}
			}
			long_count = 0;
		}
		else if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_10)==Bit_RESET)
		{//最左侧
			led_on<<=1;//0x0000 1000
			if(led_on>=0x20)
			{
				led_on = 1;
			}
			switch(led_on)
			{
			case 1:GPIO_WriteBit(GPIOC,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15,Bit_SET);GPIO_WriteBit(GPIOC,GPIO_Pin_13,Bit_RESET);break;
			case 2:GPIO_WriteBit(GPIOC,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15,Bit_SET);GPIO_WriteBit(GPIOC,GPIO_Pin_14,Bit_RESET);break;
			case 4:GPIO_WriteBit(GPIOC,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15,Bit_SET);GPIO_WriteBit(GPIOC,GPIO_Pin_15,Bit_RESET);break;
			case 8:GPIO_WriteBit(GPIOC,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15,Bit_RESET);break;
			case 0x10:GPIO_WriteBit(GPIOC,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15,Bit_SET);break;
			}
			vTaskDelay(10);
			while(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_10)==Bit_RESET);
			vTaskDelay(10);
		}
	}
}
/**@brief  摄像头采集，屏幕刷新线程
  */
extern const unsigned char gImage_a[];
void Task_Camera(void* pvParameters)
{
	//赢得了，木勾比赛，第一！
	TFT_SetRotation(0xE0);
	TFT_SetCursor(25,0,107,128);
	TFT_SPI_Start();
	for(int i=0;i<5;i++)
	{
		TFT_SPI_Send(0);
		TFT_SPI_Send(0);
	}
	for(int i=107*128*2-1;i>=0;i--)
	{
		TFT_SPI_Send(gImage_a[i]);
	}
	TFT_SPI_Stop();
	TFT_SetRotation(0x60);
	vTaskDelay(500);
	//屏幕刷新
	while(1)
	{
		vTaskDelay(1);
		if(!camera_on)
		{
			vTaskDelay(10);
			continue;
		}
		//采集数据
		OV_GetPixels();
		//屏幕显示
		Func_TFT_Show();
	}
}



/**@brief  Func命令行接口
  */
void Cmd_Func(void)
{

	U_Printf("这里是Func命令行测试 \r\n");
}








