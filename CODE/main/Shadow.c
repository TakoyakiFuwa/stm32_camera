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
#include "M_ADC.h"
/*  屏幕库  */
#include "TFT_ST7789V.h"
/*  相机库  */
#include "ov7670.h"
/*  SD卡存储库  */
#include "bmp.h"
/*  宏定义库  */
#include "cmr_def.h"
/*  UI库  */
#include "UI_Core.h"
#include "UI_Render.h"
#include "UI_Instance.h"
/*  按键库  */
#include "botton.h"

/*  项目中用到的全局变量  */
//缓存数据
uint8_t camera_data[DEF_PIC_HEIGHT*DEF_PIC_WIDTH*2];
//控制UI线程
extern int8_t STATUS_ON_UI;
//控制camera线程
int8_t camera_on = 0;
//SD卡是否正常工作
int8_t SD_on = 1;
//存储位置相关
const char BMP_PATH_bmp[] 	= {"0:/cmr/"};
const char BMP_PATH_fast[] 	= {"0:/f/"};
const char BMP_PATH_RGB565[] = {"0:/rgb565/"};
//图片处理相关（ButtonFunc.c）
uint16_t pic_index[500];		//保存在SD卡中的文件名
int16_t  pic_index_index = 0;	//(笑)index的index
uint16_t pic_num = 0;			//总数量

/*	希望我这次重新写模板可以用的久一点...
 *	想开始做一些很有趣的项目....
 *	以及...很想mo....
 *		——2025/5/20-14:41
 */
/**@brief  用于main中的接口
  */
void Main_Start(void* pvParameters)
{
	//基本功能函数
	BF_Start();
	//初始化 建议格式:Init_XXX()
		//先获取电池电压
	Init_ADC();
		//屏幕 显示开始界面
	Init_TFT((uint8_t*)&camera_data[0]);
	Init_UIR();
	Init_UI();
			///渲染开始界面
	RenderCircle_UI();
		//等待SD卡初始化
	Init_SD();
		//摄像头初始化
		//如果摄像头出现画面偏移 建议检查Task_Camera是否为最高优先级
	Init_OV((uint32_t*)&camera_data[0]);
	camera_data[0] = 0;
		//按键/补光灯内容
	Init_Light();
	Init_Botton();
	vTaskDelay(1500);
		//状态位初始化(打开摄影，关闭UI渲染)
	STATUS_ON_UI = 0;
	camera_on = 1;
	//线程	 建议格式:Task_XXX()
		//进入临界区
	taskENTER_CRITICAL();
		//Func测试
	xTaskCreate(Task_Botton,"Botton",512,NULL,5,NULL);
	xTaskCreate(Task_UI,"UI",128,NULL,7,NULL);
//	xTaskCreate(Task_GetADC,"ADC",256,NULL,5,NULL);
	xTaskCreate(Task_Camera,"Camera",128,NULL,8,NULL);
//	xTaskCreate(Task_Func,"Func",64,NULL,5,NULL);
	
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
	else if(Command("R"))
	{
		U_Printf("系统即将重置 \r\n");
		vTaskDelay(1000);
		NVIC_SystemReset();
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











