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
	TFT_SetRotation(0xC0);
	vTaskDelay(1500);
	//屏幕刷新
	while(1)
	{
		vTaskDelay(1);
		//采集数据
		OV_GetPixels();
		//屏幕显示
		TFT_SetCursor(0,0,131,162);
		TFT_SPI_DMA(131*162*2);
	}
}



/**@brief  Func命令行接口
  */
void Cmd_Func(void)
{

	U_Printf("这里是Func命令行测试 \r\n");
}








