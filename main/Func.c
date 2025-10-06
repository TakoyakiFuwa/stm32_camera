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
/*  FATFS  */
#include "ff.h"

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
int8_t camera_on = 1;
int8_t led_on = 0x10;
void Task_Button(void* pvParameters)
{
	while(1)
	{
		vTaskDelay(50);
		if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_8)==Bit_RESET)
		{
			camera_on = 0;
			vTaskDelay(10);
			while(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_8)==Bit_RESET);
			vTaskDelay(10);
		}
		else if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_9)==Bit_RESET)
		{
			camera_on = 1;
			vTaskDelay(10);
			while(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_9)==Bit_RESET);
			vTaskDelay(10);
		}
		else if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_10)==Bit_RESET)
		{
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
extern uint8_t camera_data[];
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
		TFT_SetCursor(10,20,300,100);
		TFT_SPI_SetAddr(&camera_data[0]);
		TFT_SPI_DMA(300*100*2);
		TFT_SetCursor(10,120,300,100);
		TFT_SPI_SetAddr(&camera_data[300*100*2]);
		TFT_SPI_DMA(300*100*2);
	}
}



/**@brief  Func命令行接口
  */
void Cmd_Func(void)
{

	U_Printf("这里是Func命令行测试 \r\n");
}








