#include "BaseFunc.h"
/*  架构库  */
#include "Shadow.h"
/*  ST库  */
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
/*  OS库  */
#include "FreeRTOS.h"
#include "task.h"
/*  外设库  */
#include "U_USART.h"

/*	用来存放系统启动的基本功能
 *	包括指示灯/看门狗/命令行等
 *		:)
 *		——2025/5/20-14:35
 */

/*  指示灯  */
#define BF_LED_R	RCC_AHB1Periph_GPIOE
#define BF_LED_G	GPIOE
#define BF_LED_P	GPIO_Pin_8
/*  看门狗时间  */
uint16_t wdg_ms = 3000;

static void BF_Init_LED(void);
static void BF_Init_WDG(uint16_t wdg_ms);
static void BF_Task_LED(void* pvParameters);
static void BF_Task_CLI(void* pvParameters);

/**@brief  初始化函数
  */
void BF_Start(void)
{
	//初始化
		//串口
	Init_USART();
		//看门狗
//	BF_Init_WDG(3000);
		//指示灯
	BF_Init_LED();
	//进入临界区
	taskENTER_CRITICAL();
		//指示灯&看门狗复位
	TaskHandle_t BF_TASK_LED_Handler;
	xTaskCreate(BF_Task_LED,"BF_LED",32,NULL,6,&BF_TASK_LED_Handler);
		//命令行
	TaskHandle_t BF_TASK_CLI_Handler;
	xTaskCreate(BF_Task_CLI,"BF_CLI",128,NULL,10,&BF_TASK_CLI_Handler);
	//退出临界区
	taskEXIT_CRITICAL();
}
/**@brief  打印各线程剩余栈
  */
void BF_Stack(void)
{
	uint16_t num = uxTaskGetNumberOfTasks();
	taskENTER_CRITICAL();
	TaskStatus_t* task_status = (TaskStatus_t*)pvPortMalloc(num*sizeof(TaskStatus_t));
	uxTaskGetSystemState(task_status,num,NULL);
	U_Printf("\r\n线程名字\t  : \t线程剩余栈 \r\n");
	for(int i=0;i<num;i++)
	{
		U_Printf("%s \t  : \t%d \r\n",task_status[i].pcTaskName,task_status[i].usStackHighWaterMark);
	}
	vPortFree(task_status);
	taskEXIT_CRITICAL();
}
/**@brief  命令行处理
  */
extern int8_t 	USART_RX_Signal;//接收到新数据的标志位 1代表有新数据
static void BF_Task_CLI(void* pvParameters)
{
	while(1)
	{
		while(USART_RX_Signal==0)
		{
			vTaskDelay(1);
		}
		if(Cmd()==1)
		{
			USART_RX_Signal=0;
			continue;
		}
		if(Command("COMMAND"))
		{
			U_Printf("你是说...你不但没有在Shadow.c中改Command指令 \r\n");
			U_Printf("你还把它给删了...(?  \r\n");
		}
		else if(Command("HELP"))
		{	
			U_Printf("这个程序没有添加的HELP指令... \r\n");
		}
		else if(Command("STACK"))
		{
			BF_Stack();
		}
		else if(Command("RESET"))
		{
			U_Printf("系统将在2秒后重置 \r\n");
			vTaskDelay(2000);
			NVIC_SystemReset();
		}
		else
		{
			U_Printf("命令字不匹配... \r\n检查大小写或者用\'COMMAND\'和\'HELP\'获取更多 \r\n");
		}
		USART_RX_Signal = 0;
	}
}
/**@brief  看门狗初始化
  */
static void BF_Init_WDG(uint16_t wdg_ms)
{
	if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST)==SET)
	{
		U_Printf("警告，触发看门狗复位.\r\n");
		RCC_ClearFlag();
	}
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_32);
	IWDG_SetReload(wdg_ms);
	IWDG_Enable();
	U_Printf("看门狗已启动,重装值:%dms\r\n",wdg_ms);
	IWDG_ReloadCounter();
}
/**@brief  指示灯引脚初始化
  */
static void BF_Init_LED(void)
{
	//引脚初始化
	RCC_AHB1PeriphClockCmd(BF_LED_R,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = BF_LED_P;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(BF_LED_G,&GPIO_InitStruct);
	U_Printf("指示灯初始化完成 \r\n");
}
/**@breif  线程函数 指示灯
  *@add	   因为含有看门狗复位 所以优先级应该高些
  */
static void BF_Task_LED(void* pvParameters)
{
	while(1)
	{
		//看门狗复位
		IWDG_ReloadCounter();
		//指示灯
		GPIO_WriteBit(BF_LED_G,BF_LED_P,Bit_SET);
		vTaskDelay(300);
		GPIO_WriteBit(BF_LED_G,BF_LED_P,Bit_RESET);
		vTaskDelay(600);
	}
}
