#include "SPI_HW.h"
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"

/*	打算用硬件提高性能....
 *	人生....有够糟糕...
 *			——2025/9/15-9:53.秦羽
 */

/*	引脚配置
 *	PB15	(8)		-> SDA
 *	PB13	(9)		-> SCK
 *	PB12	(12)	-> CS
 */

extern uint8_t pic_data[];
void SPI_HW_Init(void)
{
	//引脚初始化
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_15;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_12;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource15,GPIO_AF_SPI2);
	//SPI初始化
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);
	SPI_InitTypeDef SPI_InitStruct;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CRCPolynomial = 7;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_Direction = SPI_Direction_1Line_Tx;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_Init(SPI2,&SPI_InitStruct);
	SPI_Cmd(SPI2,ENABLE);
	//DMA设置 DMA1_Channel0_Stream4
	SPI_I2S_DMACmd(SPI2,SPI_I2S_DMAReq_Tx,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);
	DMA_InitTypeDef DMA_InitStruct;
	DMA_InitStruct.DMA_BufferSize = 160*128*2;
	DMA_InitStruct.DMA_Channel = DMA_Channel_0;
	DMA_InitStruct.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t)&pic_data[1+22];
	DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&(SPI2->DR);
	DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;
	DMA_Init(DMA1_Stream4,&DMA_InitStruct);
	DMA_Cmd(DMA1_Stream4,DISABLE);
}
inline void SPI_HW_Send(uint8_t x)
{
	SPI2->DR = x;
	//下面这行while不能放在其他位置，可能跟OS会打断SPI时序有关
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)==RESET);
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_BSY)==SET);
}
inline void SPI_HW_CS_H(void)
{
	GPIOB->BSRRL = GPIO_Pin_12;
}
inline void SPI_HW_CS_L(void)
{
	GPIOB->BSRRH = GPIO_Pin_12;
}
#include "TFT_ST7735.h"
void Cmd_SPI(void)
{
//	for(int i=0;i<160*130*2;i++)
//	{
//		pic_data[i] = 0xFF;
//	}
	//DMA的方式
	U_Printf("很奇怪:%h \r\n",pic_data[100]);
	DMA_Cmd(DMA1_Stream4,DISABLE);
	TFT_SetCursor(0,0,160,128);
	DMA_SetCurrDataCounter(DMA1_Stream4,128*160*2);
	TFT_Write16Data(0);
	SPI_HW_CS_L();
	DMA_Cmd(DMA1_Stream4,ENABLE);
	while(DMA_GetFlagStatus(DMA1_Stream4,DMA_FLAG_TCIF4)!=SET);
	DMA_ClearFlag(DMA1_Stream4,DMA_FLAG_TCIF4);
	SPI_HW_CS_H();
	
	for(int i=0;i<160*130*2;i++)
	{
		pic_data[i] = 0xFF;
	}
	
	
	U_Printf("我的天%h \r\n",pic_data[100]);
}



