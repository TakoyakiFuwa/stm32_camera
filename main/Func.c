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
/**@brief  摄像头采集，屏幕刷新线程
  */
extern const unsigned char gImage_a[];
void Task_Camera(void* pvParameters)
{
	//赢得了，木勾比赛，第一！
	TFT_SetCursor(0,0,107,128);
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
	while(1)
	{
		vTaskDelay(100);
		//采集数据
		TFT_SetCursor(0,0,160,130);
		TFT_SPI_Start();
		OV_GetPixels(TFT_SPI_Send);
		TFT_SPI_Stop();
	}
}



/**@brief  Func命令行接口
  */
void Cmd_Func(void)
{

	U_Printf("这里是Func命令行测试 \r\n");
}








