/*  架构库  */
#include "Shadow.h"
/*  ST库  */
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
/*  OS库  */
#include "FreeRTOS.h"
#include "task.h"

/*	所以...今天想创建并整理github库...
 *	所以把代码以UTF8的格式...
 *	以及当前觉得很规范的方式...
 *	重新写一遍...
 *			——2025/5/20-14:14
 *	啊...今天是520 偷偷在代码里夹带悄悄话...
 *	很爱moran w 🥰
 */

/**@brief  主函数 程序入口说是
  */
int main(void)
{
	//创建线程
	TaskHandle_t MAIN_START_Handler;
	xTaskCreate(Main_Start,"Main_Start",512,NULL,9,&MAIN_START_Handler);
	//开启任务调度器
	vTaskStartScheduler();
}
