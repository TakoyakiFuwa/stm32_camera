#include "Botton.h"
/*  ST库  */
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
/*  OS库  */
#include "FreeRTOS.h"
#include "task.h"

#define DEF_BOTTON_LONG		100
void (*BOT_LEFT_before)(void);
void (*BOT_LEFT_long)(void);
void (*BOT_LEFT_after)(void);
void (*BOT_MIDDLE_before)(void);
void (*BOT_MIDDLE_long)(void);
void (*BOT_MIDDLE_after)(void);
void (*BOT_RIGHT_before)(void);
void (*BOT_RIGHT_long)(void);
void (*BOT_RIGHT_after)(void);

/**@brief  按键初始化
  */
void Init_Botton(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOD,&GPIO_InitStruct);
	BOT_LEFT_before = Botton_Func_Null;
	BOT_LEFT_long = Botton_Func_Null;
	BOT_LEFT_after = Botton_Func_Null;
	BOT_MIDDLE_before = Botton_Func_Null;
	BOT_MIDDLE_long = Botton_Func_Null;
	BOT_MIDDLE_after = Botton_Func_Null;
	BOT_RIGHT_before = Botton_Func_Null;
	BOT_RIGHT_long = Botton_Func_Null;
	BOT_RIGHT_after = Botton_Func_Null;
}
void Botton_Func_Null(void)
{
	
}
void Task_Botton(void* pvParameters)
{
	uint16_t botton_long_judge = 0;
	while(1)
	{
		vTaskDelay(100);
		if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_8)==Bit_RESET)
		{
			BOT_LEFT_before();
			vTaskDelay(12);
			while(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_8)==Bit_RESET)
			{
				vTaskDelay(10);
				if(botton_long_judge++==DEF_BOTTON_LONG)//1s
				{//长判定处理
					BOT_LEFT_long();
				}
			}
			vTaskDelay(12);
			BOT_LEFT_after();
		}
		if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_9)==Bit_RESET)
		{
			BOT_MIDDLE_before();
			vTaskDelay(12);
			while(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_9)==Bit_RESET)
			{
				vTaskDelay(10);
				if(botton_long_judge++==DEF_BOTTON_LONG)//1s
				{//长判定处理
					BOT_MIDDLE_long();
				}
			}
			vTaskDelay(12);
			BOT_MIDDLE_after();
		}
		if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_10)==Bit_RESET)
		{
			BOT_RIGHT_before();
			vTaskDelay(12);
			while(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_10)==Bit_RESET)
			{
				vTaskDelay(10);
				if(botton_long_judge++==DEF_BOTTON_LONG)//1s
				{//长判定处理
					BOT_RIGHT_long();
				}
			}
			vTaskDelay(12);
			BOT_RIGHT_after();
		}
		botton_long_judge = 0;
	}
}

