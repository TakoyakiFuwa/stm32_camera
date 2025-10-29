#include "Func.h"
/*  ST库  */
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
/*  OS库  */
#include "FreeRTOS.h"
#include "task.h"
/*  外设库  */
#include "U_USART.h"
#include "bmp.h"
/*  FATFS  */
#include "ff.h"
/*  宏定义数据  */
#include "cmr_def.h"

extern uint8_t camera_data[];

/**@brief  UI线程
  */
/*  UI库  */
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
			continue;
		}
		RenderCircle_UI();
	}
}
/*  SD初始化  */
#include "UI_Instance.h"
extern qy_pointer	CURSOR;
extern qy_ui		UI[200];
extern qy_page		PAGE[8];
extern int8_t SD_on;
void Func_Pic_Index_Init(void);
void Init_SD(void)
{
		//等待SD卡挂载
	if(Init_BMP()==0)
	{//SD卡正常挂载
		UI[InUI_Start_SDstatus].value_num = 0;
		UI[InUI_Start_SDstatus].is_present = 1;
		UI_AddRender(&UI[InUI_Start_SDstatus]);
		RenderCircle_UI();
		//显示SD测试BMP
		UI[InUI_Start_SDpic].is_present = 1;
		UI_AddRender(&UI[InUI_Start_SDpic]);
		RenderCircle_UI();
		UI[InUI_Start_SDprocess].is_present = 1;
		UI_AddRender(&UI[InUI_Start_SDprocess]);
		RenderCircle_UI();
		//获取已有图片的数量和ID
		Func_Pic_Index_Init();
			//旧图片处理
//		Func_Pic_To_BMP();
				//待补充功能
		UI_AddRender(&UI[InUI_Start_SDprocess]);
		RenderCircle_UI();
	}
	else
	{//SD卡挂载异常
		UI[InUI_Start_SDstatus].value_num = Init_BMP();
		UI_AddRender(&UI[InUI_Start_SDstatus]);
		RenderCircle_UI();
		SD_on = 0;
	}
}
extern uint16_t pic_index[];		//保存在SD卡中的文件名
extern int16_t  pic_index_index;	//(笑)index的index
extern uint16_t pic_num;			//总数量
extern const char BMP_PATH_fast[];
void Func_Pic_Index_Init(void)
{
	if(SD_on!=1)
	{
		return;
	}
	DIR dp;
	FILINFO info;
	f_opendir(&dp,BMP_PATH_fast);
	f_readdir(&dp,&info);
	pic_index_index = 0;
	pic_index[0] = 0;
	while(info.fname[0]!='\0')
	{
		pic_index[pic_index_index++] = BMP_StringToNum(info.fname);
		f_readdir(&dp,&info);
	}
	pic_index[pic_index_index] = 999;
	f_closedir(&dp);
	pic_num = pic_index_index;//总数量
	// :) C语言基础，手写数组排序
	uint16_t temp = 0;
	for(int i=0;i<pic_index_index;i++)
	{
		for(int j=pic_index_index;j>i;j--)
		{
			if(pic_index[j]<pic_index[j-1])
			{
				temp = pic_index[j];
				pic_index[j] = pic_index[j-1];
				pic_index[j-1] = temp;
			}
		}
	}
//	for(int i=0;i<=pic_index_index;i++)
//	{
//		U_Printf("[%d]->%d \r\n",i,pic_index[i]);
//	}
}
#include "ButtonFunc.h"
extern const char BMP_PATH_bmp[];
extern const char BMP_PATH_fast[];
void Func_Pic_To_BMP(void)
{
	//:)
	//我突然感觉边拍边保存成BMP格式也不错....
	char path[50];
	for(int i=1;i<pic_index_index;i++)
	{
		//读出数据
		BMP_NumToString(pic_index[i],path);
		SD_Fast_Read(path,(uint16_t*)&camera_data[0],DEF_PIC_HEIGHT*DEF_PIC_WIDTH);
		//制作BMP
		BMP_NumToString(pic_index[i],path);
		BMP_Write_ByData(path,(uint16_t*)&camera_data[1],DEF_PIC_WIDTH,DEF_PIC_HEIGHT);
		U_Printf("%d.bmp处理完成 \r\n",pic_index[i]);
	}
}
/*  ？？？写的好乱好乱....又要烂尾了...  */
/**@brief 获取ADC数据
  */
/*  ADC库  */
#include "M_ADC.h"
void Task_GetADC(void* pvParameters)
{
	int16_t old_value = 0;
	while(1)
	{
		vTaskDelay(3000);
		int16_t value = 0;
		for(int i=0;i<10;i++)
		{
			value+=M_ADC_Get();
		}
		value/=10;
		if(value-old_value>4 || old_value-value>4)
		{
			old_value = value;
		}
		U_Printf("电压输出:[%d]/[%d] \r\n",value,old_value);
	}
}
/**@brief  Func初始化
  */
void Init_Func(void)
{
	
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
/**@brief  LED初始化
  */
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
/**@brief  摄影时的屏幕渲染函数
  */
/*  屏幕库  */
#include "TFT_ST7789V.h"
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
/**@brief  摄像头采集，屏幕刷新线程
  */
/*  摄像头库  */
#include "ov7670.h"
extern int8_t camera_on;
void Task_Camera(void* pvParameters)
{
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








