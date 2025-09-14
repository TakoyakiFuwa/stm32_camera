#include "TFT_ILI9341.h"
//utf-8
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "FreeRTOS.h"
#include "task.h"
#include "U_USART.h"

/*	TFT的底层驱动
 *	ILI9341
 *		2025/7/30-11:06
 */

/*	以下是关于引脚接线内容
 *	可根据单片机类型和接口并口/SPI(软件/硬件)进行调整和适配
 *	ILI9341
 *  当前配置为:CS低电平选中，RST低电平复位，D/C高电平数据/低电平指令
 *            MOSI传数据，SCK下降沿时屏幕采集信号
 */
/* 	
 * 	当前显示方向:				   y^
 *								|
 *					<——————3	|
 *					<——————2	|
 *					<——————1	|
 *				x				|
 *				<——-——-——-——-——-+ (0,0)
 */
 
//PE10 ->CS
#define PIN_CSH		GPIOE->BSRRL = (uint32_t)GPIO_Pin_10
#define PIN_CSL		GPIOE->BSRRH = (uint32_t)GPIO_Pin_10
//PE12 ->RST
#define PIN_RSTH	;//GPIOE->BSRRL = (uint32_t)GPIO_Pin_12
#define PIN_RSTL	;//GPIOE->BSRRH = (uint32_t)GPIO_Pin_12
//PE11 ->DC
#define PIN_DC_DATA		GPIOE->BSRRL = (uint32_t)GPIO_Pin_11
#define PIN_DC_CMD		GPIOE->BSRRH = (uint32_t)GPIO_Pin_11
//PC4  ->MOSI
#define PIN_DATAH	GPIOC->BSRRL = (uint32_t)GPIO_Pin_4
#define PIN_DATAL	GPIOC->BSRRH = (uint32_t)GPIO_Pin_4
//PE9  ->SCK
#define PIN_SCKH	GPIOE->BSRRL = (uint32_t)GPIO_Pin_9
#define PIN_SCKL	GPIOE->BSRRH = (uint32_t)GPIO_Pin_9

/**@brief  TFT引脚初始化
  *@param  void
  *@retval void
  *@add    移植时根据接线调整
  */
static void Init_ILI_PIN(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;
	GPIO_Init(GPIOC,&GPIO_InitStruct);
	GPIO_WriteBit(GPIOC,GPIO_Pin_5,Bit_RESET);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_12|GPIO_Pin_11;
	GPIO_Init(GPIOE,&GPIO_InitStruct);
	GPIO_WriteBit(GPIOE,GPIO_Pin_12,Bit_SET);
}
/**@brief  与TFT交换数据
  *@param  byte 交换的数据
  *@retval void
  *@add    交换1byte数据，可根据接口调整成并口
  */
//extern SPI_HandleTypeDef hspi1;
void ILI_Swap(uint8_t byte)
{
	//软件发送1byte
	PIN_CSL;//对hal库不太熟悉 有时间回来改成硬件SPI
	for(int i=0;i<8;i++)
	{
		if( (byte&(0x80>>i)) != 0 )
		{
			PIN_DATAH;
		}
		else 
		{
			PIN_DATAL;
		}
		PIN_SCKL;
		PIN_SCKH;
	}
	PIN_CSH;
}
//屏幕长和宽
uint16_t D_ILI_WIDTH = 320;
uint16_t D_ILI_HEIGHT = 240;



/*
 *	以下是ILI9341的软件驱动
 *	与接口无关 无需更改 
 */


		
//两个初始化函数
static void ILI_Reset(void);
static void ILI_SoftwareInit(void);
/**@brief  TFT(ILI9341)初始化函数
  *@param  void
  *@retval void
  */
void Init_ILI(void)
{
	Init_ILI_PIN();
	//引脚初始电位
	PIN_CSH;
	PIN_SCKH;
	PIN_RSTH;
	
	//屏幕复位
	ILI_Reset();
	//软件初始化
	ILI_SoftwareInit();
	//简单绘制背景
//	ILI_Test();
	uint16_t height = 240/3;
	uint16_t width = 320;
	uint16_t rgb565 = ILI_RGB888To565(0xf6f6f6);
	ILI_SetRect(0,height*1,width,height);
	for(int i=0;i<width*height;i++)
	{
		ILI_SendColor(rgb565);
	}
	rgb565 = ILI_RGB888To565(0x71c9ce);
	ILI_SetRect(0,height*2,width,height);
	for(int i=0;i<width*height;i++)
	{
		ILI_SendColor(rgb565);
	}
	rgb565 = ILI_RGB888To565(0xffc7c7);
	ILI_SetRect(0,0,width,height);
	for(int i=0;i<width*height;i++)
	{
		ILI_SendColor(rgb565);
	}
	U_Printf("ILI9341初始化完成 \r\n");
	
}
/**@brief  硬件复位
  */
static void ILI_Reset(void)
{
	PIN_RSTL;
	vTaskDelay(100);
	PIN_RSTH;
}
/**@brief  发送指令
  */
static void ILI_SendCmd(uint8_t cmd)
{
	PIN_DC_CMD;
	ILI_Swap(cmd);
}
/**@brief  发送数据
  */
static void ILI_SendData(uint8_t data)
{
	PIN_DC_DATA;
	ILI_Swap(data);
}
/**@brief  在使用ILI_SetRect后用于填充颜色和图案
  */
void ILI_SendColor(uint16_t rgb565)
{
	PIN_DC_DATA;
	ILI_Swap((rgb565>>8));
	ILI_Swap(rgb565);
}
/**@brief  设置显示区
  *@param  X Y 显示起点
  *@param  width height 宽度/高度
  *@retval void
  */
void ILI_SetRect(uint16_t X1, uint16_t Y1, uint16_t width, uint16_t height)
{
	width+=X1-1;
	height+=Y1-1;
	
	ILI_SendCmd(0x2A);
	ILI_SendData(X1>>8);
	ILI_SendData(X1);
	ILI_SendData(width>>8);
	ILI_SendData(width);

	ILI_SendCmd(0x2B);
	ILI_SendData(Y1>>8);
	ILI_SendData(Y1);
	ILI_SendData(height>>8);
	ILI_SendData(height);

	ILI_SendCmd(0x2C);
}
/**@brief  设置屏幕显示方向
  *@param  rotation
  *@add    D7~D2:MY MX MV ML BGR MH (建议0x68 0b0110 1100->X反转横向显示)
  *		         MX/MY->XY翻转
  *				 MV->0竖向/1横向显示
  *				 ML/MH->刷新(填充)方向
  *				 BGR->1
  */
static void ILI_SetRotation(uint8_t rotation)
{
	ILI_SendCmd(0x36);
	ILI_SendData(rotation);
}
/**@brief  从RGB888转换成RGB565
  *@param  RGB888 颜色代码
  *@retval RGB565颜色代码
  */
uint16_t ILI_RGB888To565(uint32_t RGB888)
{
	uint16_t RGB565 = 0;
	RGB565 = RGB888>>19;
	RGB565 = RGB565<<6;
	RGB565 |= ((RGB888>>10)&0x3F);
	RGB565 = RGB565<<5;
	RGB565 |= ((RGB888>>3)&0x1F);
	return RGB565;
}
/**@brief  在main.c中的测试接口
  *@param  void
  *@retval void
  */
void ILI_Test(void)
{
	//		红 黄
	//绘制  	蓝 绿
	uint16_t size = (D_ILI_HEIGHT/2)*(D_ILI_WIDTH/2)+20;
	ILI_SetRect(D_ILI_WIDTH/2,0,D_ILI_WIDTH/2,D_ILI_HEIGHT/2);
	for(int i=0;i<size;i++)
	{
		ILI_SendColor(COLOR_GREEN);
	}
	ILI_SetRect(0,D_ILI_HEIGHT/2,D_ILI_WIDTH/2,D_ILI_HEIGHT/2);
	for(int i=0;i<size;i++)
	{
		ILI_SendColor(COLOR_RED);
	}
	ILI_SetRect(D_ILI_WIDTH/2,D_ILI_HEIGHT/2,D_ILI_WIDTH/2,D_ILI_HEIGHT/2);
	for(int i=0;i<size;i++)
	{
		ILI_SendColor(COLOR_YELLOW);
	}
	ILI_SetRect(0,0,D_ILI_WIDTH/2,D_ILI_HEIGHT/2);
	for(int i=0;i<size;i++)
	{
		//想要切实观察刷新方向时可以添加这个Delay
//		HAL_Delay(3);
		ILI_SendColor(COLOR_BLUE);
	}	
}
/**@brief  TFT软件初始化
  */
static void ILI_SoftwareInit(void)
{	
	ILI_SendCmd(0x01);
	vTaskDelay(100);
		
	//POWER CONTROL A
	ILI_SendCmd(0xCB);
	ILI_SendData(0x39);
	ILI_SendData(0x2C);
	ILI_SendData(0x00);
	ILI_SendData(0x34);
	ILI_SendData(0x02);
	
	//POWER CONTROL B
	ILI_SendCmd(0xCF);
	ILI_SendData(0x00);
	ILI_SendData(0xC1);
	ILI_SendData(0x30);
	
	//DRIVER TIMING CONTROL A
	ILI_SendCmd(0xE8);
	ILI_SendData(0x85);
	ILI_SendData(0x00);
	ILI_SendData(0x78);
	
	//DRIVER TIMING CONTROL B
	ILI_SendCmd(0xEA);
	ILI_SendData(0x00);
	ILI_SendData(0x00);
	
	//POWER ON SEQUENCE CONTROL
	ILI_SendCmd(0xED);
	ILI_SendData(0x64);
	ILI_SendData(0x03);
	ILI_SendData(0x12);
	ILI_SendData(0x81);
	
	//PUMP RATIO CONTROL
	ILI_SendCmd(0xF7);
	ILI_SendData(0x20);
	
	//POWER CONTROL,VRH[5:0]
	ILI_SendCmd(0xC0);
	ILI_SendData(0x23);
	
	//POWER CONTROL,SAP[2:0];BT[3:0]
	ILI_SendCmd(0xC1);
	ILI_SendData(0x10);
	
	//VCM CONTROL
	ILI_SendCmd(0xC5);
	ILI_SendData(0x3E);
	ILI_SendData(0x28);
	
	//VCM CONTROL 2
	ILI_SendCmd(0xC7);
	ILI_SendData(0x86);
	
	//MEMORY ACCESS CONTROL
	ILI_SendCmd(0x36);
	ILI_SendData(0x48);
	
	//PIXEL FORMAT
	ILI_SendCmd(0x3A);
	ILI_SendData(0x55);
	
	//FRAME RATIO CONTROL, STANDARD RGB COLOR
	ILI_SendCmd(0xB1);
	ILI_SendData(0x00);
	ILI_SendData(0x18);
	
	//DISPLAY FUNCTION CONTROL
	ILI_SendCmd(0xB6);
	ILI_SendData(0x08);
	ILI_SendData(0x82);
	ILI_SendData(0x27);
	
	//3GAMMA FUNCTION DISABLE
	ILI_SendCmd(0xF2);
	ILI_SendData(0x00);
	
	//GAMMA CURVE SELECTED
	ILI_SendCmd(0x26);
	ILI_SendData(0x01);
	
	//POSITIVE GAMMA CORRECTION
	ILI_SendCmd(0xE0);
	ILI_SendData(0x0F);
	ILI_SendData(0x31);
	ILI_SendData(0x2B);
	ILI_SendData(0x0C);
	ILI_SendData(0x0E);
	ILI_SendData(0x08);
	ILI_SendData(0x4E);
	ILI_SendData(0xF1);
	ILI_SendData(0x37);
	ILI_SendData(0x07);
	ILI_SendData(0x10);
	ILI_SendData(0x03);
	ILI_SendData(0x0E);
	ILI_SendData(0x09);
	ILI_SendData(0x00);
	
	//NEGATIVE GAMMA CORRECTION
	ILI_SendCmd(0xE1);
	ILI_SendData(0x00);
	ILI_SendData(0x0E);
	ILI_SendData(0x14);
	ILI_SendData(0x03);
	ILI_SendData(0x11);
	ILI_SendData(0x07);
	ILI_SendData(0x31);
	ILI_SendData(0xC1);
	ILI_SendData(0x48);
	ILI_SendData(0x08);
	ILI_SendData(0x0F);
	ILI_SendData(0x0C);
	ILI_SendData(0x31);
	ILI_SendData(0x36);
	ILI_SendData(0x0F);
	
	//EXIT SLEEP
	ILI_SendCmd(0x11);
	vTaskDelay(120);
	
	//TURN ON DISPLAY
	ILI_SendCmd(0x29);
	
	//设置旋转方向
	ILI_SetRotation(0x28);
	
	/**@brief  设置屏幕显示方向
	  *@param  rotation
	  *@add    D7~D2:MY MX MV ML / BGR MH (建议 0b0000 1000->)
	  *		         MX/MY->XY翻转
	  *				 MV->0竖向/1横向显示
	  *				 ML/MH->刷新(填充)方向
	  *				 BGR->1
	  */
}














