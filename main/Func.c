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
#include "TFT_ST7735.h"
/*  FATFS  */
#include "ff.h"

FATFS fs;

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

/*  以下是关于相机测试的内容  */

/*
uint32_t line_count = 0;
uint8_t line_count_signal = 0;
void OV_PixelsGet(void (*Func)(uint8_t))
{	
	while(OV_VS()==0);
	while(OV_VS()!=0);
	OV_XCLK_OFF();
	while(OV_VS()==0)
	{
		line_count_signal = 1;
		while(OV_HS()!=0)
		{
			//测试v
			if(line_count_signal==1)
			{
				line_count++;
				line_count_signal = 0;
			}
			//测试^
			while(OV_PLK()==0)
			{
				OV_XCLK();
			}
			Func(OV_RGBData());
			while(OV_PLK()!=0)
			{
				OV_XCLK();
			}
		}
		OV_XCLK();
	}
	OV_XCLK_ON();
}
extern uint32_t line_count;
uint16_t pixel_count = 0;
const uint8_t scale = 2;
void TFT(uint8_t data)
{
	static uint8_t high_byte = 1;
	static uint16_t pixel = 0;
	if(line_count%scale!=0)
	{
		return;
	}
	if(high_byte==1)
	{
		high_byte = 0;
		pixel = data;
		pixel<<=8;
	}
	else
	{
		high_byte =1;
		if(pixel_count++%scale!=0)
		{
			return;
		}
		pixel+=data;
		TFT_Write16Data(pixel);
	}

}
void Task_Camera_WithPixelCount(void* pvParameters)
{
	while(1)
	{
		vTaskDelay(10);
		TFT_SetCursor(0,0,156,120);
		OV_PixelsGet(TFT);
		U_Printf("hight:%d   width:%d   pixel:%d \r\n",line_count,pixel_count*scale/line_count,pixel_count*scale);
		line_count = 0;
		pixel_count = 0;
	}
}
*/


/**@brief  Func命令行接口
  */
void Cmd_Func(void)
{

	U_Printf("这里是Func命令行测试 \r\n");
}








