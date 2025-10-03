#include "Shadow.h"
/*  架构库  */
#include "Func.h"
#include "BaseFunc.h"
/*  ST库  */
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
/*  OS库  */
#include "FreeRTOS.h"
#include "task.h"
/*  外设库  */
#include "U_USART.h"
#include "TFT_ST7789V.h"
#include "ov7670.h"
#include "bmp.h"

/*	希望我这次重新写模板可以用的久一点...
 *	想开始做一些很有趣的项目....
 *	以及...很想mo....
 *		——2025/5/20-14:41
 */

uint8_t camera_data[300*204*2];
/**@brief  用于main中的接口
  */
void Main_Start(void* pvParameters)
{
	//基本功能函数
	BF_Start();
	//初始化 建议格式:Init_XXX()
	Init_Func();
	Init_TFT((uint8_t*)&camera_data[0]);	
	Init_OV((uint32_t*)&camera_data[0]);
	camera_data[0] = 0;
	Init_BMP();
		//按键/补光灯内容
	Init_Light();
	Init_Button();
	
	//线程	 建议格式:Task_XXX()
		//进入临界区
	taskENTER_CRITICAL();
		//Func测试
//	TaskHandle_t TASK_FUNC_Handler;
//	xTaskCreate(Task_Func,"Func",64,NULL,1,&TASK_FUNC_Handler);
	xTaskCreate(Task_Camera,"Camera",128,NULL,8,NULL);
	xTaskCreate(Task_Button,"Button",32,NULL,5,NULL);
	
		//退出临界区
	taskEXIT_CRITICAL();	
	//打印各线程栈
	BF_Stack();
	//删除自身线程
	vTaskDelete(NULL);
}
/**@brief  命令行创建接口
  *@param  1有匹配 0没匹配转到BaseFunc
  */
int8_t Cmd(void)
{
	//COMMAND
	if(Command("COMMAND"))
	{
		U_Printf("这里是相机板子的驱动程序... \r\n");
		U_Printf("320*240的ST7789V屏幕驱动，使用OV7670摄像头... \r\n");
		U_Printf("SD卡的程序...正在写... \r\n");
	}
		//FUNC测试
	else if(Command("FUNC"))
	{
		Cmd_Func();
	}
	else if(Command("BMP"))
	{
		Cmd_BMP();
	}
	
	//CLI :>
	else if(Command("HELP"))
	{
		U_Printf("HELP	  : 获取可用命令行 \r\n");
		U_Printf("COMMAND : 查看当前程序信息 \r\n");
		U_Printf("RESET	  : 系统重启 \r\n");
		U_Printf("STACK	  : 获取各线程剩余栈 \r\n");
		U_Printf("FUNC	  : 架构库Func.h测试命令行 \r\n");
	}
	else
	{
		return 0;
	}
	return 1;
} 











