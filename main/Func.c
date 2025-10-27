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
			return;
		}
		RenderCircle_UI();
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








