#include "TFT_ST7789V.h"
/*  OS库(仅提供Delay)  */
#include "FreeRTOS.h"
#include "task.h"
/*  串口调试库  */
#include "U_USART.h"

/*	后天早上去学校了...
 *	大学...在学校里摆烂的更好呢
 *	但是这两天还是很难说的焦虑
 *			-2025/2/20-17:17
 */
/*	这里是硬件SPI+DMA的屏幕驱动
 *	想绿豆糕糕了呜呜....
 *	好想要陪陪....
 *	真糟糕真糟糕真糟糕...
 *			-2025/9/26-19:54.秦羽
 */

/*  移植配置区域  */
/*	当前在处理F4_UI板子
 *	PD14	(3)		-> GND
 *	PD13	(4)		-> VCC
 *	PD12	(5)		-> GND
 *	PD11	(6)		-> RST
 *	PD10	(7)		-> DC
 *	PB15	(8)		-> SDA
 *	PB13	(9)		-> SCK
 *	PD9		(10)	-> VCC
 *	PB14	(11)	-> VCC
 *	PB12	(12)	-> CS
 */
#define TFT_ROTATION 0x60	//YXV0 0000
/*	方向为: （适合f4_ui的OV7670方向输出）
 *		+-——-——-——-——>
 *		|			 x	
 *		|	1—————>
 *		|	2—————>
 *		|	3—————>
 *		|
 *		v y 
 */


/*  需要修改的接口  */


/*  SPI通讯处理  */
	//片选线
#define TFT_SPI_CS_L()		GPIOB->BSRRH = GPIO_Pin_12
#define TFT_SPI_CS_H()		GPIOB->BSRRL = GPIO_Pin_12
/*  TFT屏幕处理  */
	//低电平复位
#define TFT_RST_L()		GPIOC->BSRRH = GPIO_Pin_4
#define TFT_RST_H()		GPIOC->BSRRL = GPIO_Pin_4
	//低电平指令
#define TFT_DC_L()		GPIOC->BSRRH = GPIO_Pin_5
	//高电平数据
#define TFT_DC_H()		GPIOC->BSRRL = GPIO_Pin_5
/**@brief  接口 配置相关引脚初始化
  *@param  void
  *@retval void
  */
static void TFT_PinInit()
{
	//时钟初始化
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
	//引脚初始化
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
		//PD
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14|GPIO_Pin_13|GPIO_Pin_12|GPIO_Pin_9;
	GPIO_Init(GPIOD,&GPIO_InitStruct);
		//PB
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_14;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
		//PC
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;
	GPIO_Init(GPIOC,&GPIO_InitStruct);
	//电平初始化
	GPIO_WriteBit(GPIOC,GPIO_Pin_4,Bit_SET);
	//复用引脚初始化
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_15;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource15,GPIO_AF_SPI2);
}


/*  下面是驱动，无需修改  */


static void TFT_SoftwareInit(void);
static void TFT_SPI_Init(uint8_t* data_addr);
/**@brief  TFT初始化
  *@param  void
  *@retval void
  */
void Init_TFT(uint8_t* data_addr)
{
	//引脚初始化
	TFT_PinInit();
		//SPI初始电平
	TFT_SPI_CS_H();
	//SPI初始化
	TFT_SPI_Init(data_addr);
	vTaskDelay(100);
	//硬件复位
	TFT_RST_L();
	vTaskDelay(100);
	TFT_RST_H();
	vTaskDelay(50);
	//软件初始化
	TFT_SoftwareInit();
	vTaskDelay(50);
	//初始化图像
	uint16_t height = 240/3;
	uint16_t width = 320;
	uint16_t rgb565 = TFT_RGB888To565(0xffc7c7);
	TFT_SetCursor(0,0,width,height);
	for(int i=0;i<width*height;i++)
	{
		TFT_Write16Data(rgb565);
	}
	rgb565 = TFT_RGB888To565(0xf6f6f6);
	TFT_SetCursor(0,height*1,width,height);
	for(int i=0;i<width*height;i++)
	{
		TFT_Write16Data(rgb565);
	}
	rgb565 = TFT_RGB888To565(0x71c9ce);
	TFT_SetCursor(0,height*2,width,height);
	for(int i=0;i<width*height;i++)
	{
		TFT_Write16Data(rgb565);
	}
	U_Printf("TFT初始化完成 \r\n");
}
/**@brief  硬件SPI初始化
  */
DMA_InitTypeDef DMA_InitStruct;
static void TFT_SPI_Init(uint8_t* data_addr)
{
	//SPI初始化
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);
	SPI_InitTypeDef SPI_InitStruct;
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
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
	DMA_InitStruct.DMA_BufferSize = 160*128*2;
	DMA_InitStruct.DMA_Channel = DMA_Channel_0;
	DMA_InitStruct.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t)data_addr;
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
/**@brief  SPI开始通信
  */
inline void TFT_SPI_Start(void)
{
	TFT_DC_H();
	TFT_SPI_CS_L();
}
/**@brief  SPI停止通信
  */
inline void TFT_SPI_Stop(void)
{
	TFT_SPI_CS_H();
}
/**@brief  SPI发送数据
  *@param  byte 一字节
  *@retval void
  */
void TFT_SPI_Send(uint8_t byte)
{
	SPI2->DR = byte;
	//下面这行while不能放在其他位置，可能跟OS会打断SPI时序有关
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)==RESET);
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_BSY)==SET);
}
/**@brief  用DMA的方式发送一串数据
  *@param  counts 要发送的数据量(字节数)
  *@retval void
  */
inline void TFT_SPI_DMA(uint16_t counts)
{
	DMA_SetCurrDataCounter(DMA1_Stream4,counts);
	TFT_SPI_Start();
	DMA_Cmd(DMA1_Stream4,ENABLE);
	while(DMA_GetFlagStatus(DMA1_Stream4,DMA_FLAG_TCIF4)!=SET);
	DMA_ClearFlag(DMA1_Stream4,DMA_FLAG_TCIF4);
	TFT_SPI_Stop();
}
/**@brief  设置DMA地址
  *@param  addr 要设置的地址
  *@retval void
  */
inline void TFT_SPI_SetAddr(uint8_t* addr)
{
	DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t)addr;
	DMA_Init(DMA1_Stream4,&DMA_InitStruct);
}
/**@brief （内部函数）向TFT发送8位指令
  *@param  cmd   要发送的指令
  *@retval void
  */
static void TFT_WriteCmd(uint8_t cmd)
{
	TFT_DC_L();
	TFT_SPI_CS_L();
	TFT_SPI_Send(cmd);
	TFT_SPI_CS_H();
}
/**@brief （内部函数）向TFT发送8位数据
  *@param  data  要发送的数据
  *@retval void
  */
static void TFT_WriteData(uint8_t data)
{
	TFT_DC_H();
	TFT_SPI_CS_L();
	TFT_SPI_Send(data);
	TFT_SPI_CS_H();
}
/**@brief  发送16位数据
  *@param  data  16位数据
  *@retval void
  */
void TFT_Write16Data(uint16_t RGB_565)
{
	TFT_DC_H();
	TFT_SPI_CS_L();
	TFT_SPI_Send((RGB_565>>8));
	TFT_SPI_Send(RGB_565);
	TFT_SPI_CS_H();
}
/**@brief  从RGB888改成RGB555
  *@param  color RGB888数据
  *@retval RGB565数据
  */
uint16_t TFT_RGB888To565(uint32_t RGB_888)
{
	uint32_t color_retval = 0x00;
	uint32_t color_temp = 0;
	//R 从256改成32
	color_temp |= ( (RGB_888>>16)&0xFF );
//	U_Printf("R:%x\r\n",color_temp);
	color_retval |= (color_temp*32)/256;
	//G 从256改成64
	color_temp = 0;
	color_temp |= ( (RGB_888>>8)&0xFF );
//	U_Printf("G:%x\r\n",color_temp);
	color_retval = color_retval<<6;
	color_retval |= (color_temp*64)/256;
	//B 从256改成32
	color_temp = 0;
	color_temp |= ( RGB_888&0xFF );
//	U_Printf("B:%x\r\n",color_temp);
	color_retval = color_retval<<5;
	color_retval |= (color_temp*32)/256;
	
	return (uint16_t)color_retval;
}
/**@brief  设置写入范围
  *@param  x_start y_start 	写入位置
  *@param  weight height	宽度/高度
  *@retval void
  *@add	   可以把0x2a和0x2b交换来实现某种意义上的换方向(?)
  */
void TFT_SetCursor(uint16_t x,uint16_t y,uint16_t width,uint16_t height)
{
	uint16_t x_end = x+width-1;
	uint16_t y_end = y+height-1;
	TFT_WriteCmd(0x2a);//x轴
	TFT_Write16Data(x);
	TFT_Write16Data(x_end);
	
	
	TFT_WriteCmd(0x2b);//y轴
	TFT_Write16Data(y);
	TFT_Write16Data(y_end);
	
	TFT_WriteCmd(0x2C);//开始写入
}

/**@brief  旋转方向
  *@param  rotation YXV0 0000
  */
void TFT_SetRotation(uint8_t rotation)
{
	TFT_WriteCmd(0x36); 	//MX, MY, RGB mode 
	TFT_WriteData(rotation);	//YXV0 0000 翻转 前两位分别是Y X
}
/**@brief  TFT屏幕软件初始化
  *@param  void
  *@retval void
  */
static void TFT_SoftwareInit(void)
{			
	TFT_WriteCmd(0x11); 
    vTaskDelay(120);         //Delay 120ms 

    TFT_WriteCmd(0x36); 
    TFT_WriteData(TFT_ROTATION); //a0：横屏，00:竖屏，60：横屏镜像
    TFT_WriteCmd(0x3a); 
    TFT_WriteData(0x05); 

    TFT_WriteCmd(0xb2); 
    TFT_WriteData(0x0c); 
    TFT_WriteData(0x0c); 
    TFT_WriteData(0x00); 
    TFT_WriteData(0x33); 
    TFT_WriteData(0x33); 
    TFT_WriteCmd(0xb7); 
    TFT_WriteData(0x35); 
   
    //24脚2.4寸屏初始化程序
    TFT_WriteCmd(0xbb); 
    TFT_WriteData(0x2b); 
    TFT_WriteCmd(0xc0); 
    TFT_WriteData(0x2c); 
    TFT_WriteCmd(0xc2); 
    TFT_WriteData(0x01); 
    TFT_WriteCmd(0xc3); 
    TFT_WriteData(0x11); 
    TFT_WriteCmd(0xc4); 
    //ST7789V 54
    TFT_WriteData(0x20); 
    TFT_WriteCmd(0xc6); 
    TFT_WriteData(0x0f); 
    TFT_WriteCmd(0xd0); 
    TFT_WriteData(0xa4); 
    TFT_WriteData(0xa1); 

    TFT_WriteCmd(0xe0); 
    TFT_WriteData(0xd0); 
    TFT_WriteData(0x00); 
    TFT_WriteData(0x05); 
    TFT_WriteData(0x0e); 
    TFT_WriteData(0x15); 
    TFT_WriteData(0x0d); 
    TFT_WriteData(0x37); 
    TFT_WriteData(0x43); 
    TFT_WriteData(0x47); 
    TFT_WriteData(0x09); 
    TFT_WriteData(0x15); 
    TFT_WriteData(0x12); 
    TFT_WriteData(0x16); 
    TFT_WriteData(0x19); 
    TFT_WriteCmd(0xe1); 
    TFT_WriteData(0xd0); 
    TFT_WriteData(0x00); 
    TFT_WriteData(0x05); 
    TFT_WriteData(0x0d); 
    TFT_WriteData(0x0c); 
    TFT_WriteData(0x06); 
    TFT_WriteData(0x2d); 
    TFT_WriteData(0x44); 
    TFT_WriteData(0x40); 
    TFT_WriteData(0x0e); 
    TFT_WriteData(0x1c); 
    TFT_WriteData(0x18); 
    TFT_WriteData(0x16); 
    TFT_WriteData(0x19); 
    TFT_WriteCmd(0xe7); 
    TFT_WriteData(0x10);
    TFT_WriteCmd(0x29); //display on
}









