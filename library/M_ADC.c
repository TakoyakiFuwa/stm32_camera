#include "M_ADC.h"
/*  ST库  */
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
/*  串口库  */
#include "U_USART.h"

void Init_ADC(void)
{
	//引脚初始化
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	//外设初始化
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	//ADC常规初始化
	ADC_CommonInitTypeDef ADC_CInitStruct;
	ADC_CInitStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CInitStruct.ADC_Mode = ADC_Mode_Independent;
	ADC_CInitStruct.ADC_Prescaler = ADC_Prescaler_Div8;
	ADC_CInitStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;
	ADC_CommonInit(&ADC_CInitStruct);
	//ADC初始化
	ADC_InitTypeDef ADC_InitStruct;
	ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStruct.ADC_NbrOfConversion = 1;
	ADC_InitStruct.ADC_Resolution = ADC_Resolution_10b;
	ADC_InitStruct.ADC_ScanConvMode = DISABLE;
	ADC_Init(ADC1,&ADC_InitStruct);
	//通道配置
	ADC_RegularChannelConfig(ADC1,ADC_Channel_0,1,ADC_SampleTime_144Cycles);
	ADC_Cmd(ADC1,ENABLE);
	//开始转换
	ADC_SoftwareStartConv(ADC1);
	//测试输出
	U_Printf("ADC初始化完成:当前值[%d] \r\n",M_ADC_Get());
}


uint16_t M_ADC_Get(void)
{
	ADC_SoftwareStartConv(ADC1);
	while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC)==RESET)
	{
		
	}
	uint32_t value = ADC_GetConversionValue(ADC1);
	value*=10000;
	value /= 32*1024;
	return (uint16_t)value;
}




