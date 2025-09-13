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
void Task_Camera(void* pvParameters)
{
	while(1)
	{
		vTaskDelay(10);
		TFT_SetCursor(0,0,120,120);
		OV_PixelsGet(2,TFT_Write16Data);
	}
}
/**@brief  Func命令行接口
  */
void Cmd_Func(void)
{

	U_Printf("这里是Func命令行测试 \r\n");
}








