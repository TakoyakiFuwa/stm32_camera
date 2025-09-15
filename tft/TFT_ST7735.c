#include "TFT_ST7735.h"
/*  协议库  */
#include "SPI_SF.h"
#include "SPI_HW.h"
/*  OS库  */
#include "FreeRTOS.h"
#include "task.h"
/*  串口调试库  */
#include "U_USART.h"


/*	后天早上去学校了...
 *	大学...在学校里摆烂的更好呢
 *	但是这两天还是很难说的焦虑
 *			-2025/2/20-17:17
 */

/*  SPI接口  */
/*  这部分移植在TFT_SPI_SendData  */
	//软件
//#define SPI_Init()		SPI_SF_Init()
	//硬件
#define SPI_Init()		SPI_HW_Init()


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
/*	方向为: （适合BMP输出）
 *		
 *	^ y	
 *	|		
 *	|	3————>
 *	|	2————>
 *	|	1————>
 *	|			 x
 *	+-——-——-——-——>
 */

/*  TFT屏幕处理  */
	//低电平复位
#define TFT_RST_L()		GPIOD->BSRRH = GPIO_Pin_11
#define TFT_RST_H()		GPIOD->BSRRL = GPIO_Pin_11
	//低电平指令
#define TFT_DC_L()		GPIOD->BSRRH = GPIO_Pin_10
	//高电平数据
#define TFT_DC_H()		GPIOD->BSRRL = GPIO_Pin_10
/**@brief  接口 配置相关引脚初始化
  *@param  void
  *@retval void
  */
static void TFT_PinInit()
{
	//时钟初始化
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
	//引脚初始化
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14;
	GPIO_Init(GPIOD,&GPIO_InitStruct);
	
	GPIO_WriteBit(GPIOD,GPIO_Pin_14|GPIO_Pin_12,Bit_RESET);
	GPIO_WriteBit(GPIOD,GPIO_Pin_13|GPIO_Pin_9,Bit_SET);
	GPIO_WriteBit(GPIOB,GPIO_Pin_14,Bit_SET);
}
/**@brief  发送数据
  *@-	   接口这里...就...很难办
  */
static void TFT_SPI_SendData(uint8_t data)
{
	//软件
//	SPI_SF_CS_L();
//	SPI_SF_Send(data);
//	SPI_SF_CS_H();
	//硬件
	SPI_HW_CS_L();
	SPI_HW_Send(data);
	SPI_HW_CS_H();
}

static void TFT_SoftwareInit(void);
/**@brief  TFT初始化
  *@param  void
  *@retval void
  */
void Init_TFT(void)
{
	//通信初始化
	SPI_Init();
	vTaskDelay(200);//SPI外设初始化后应Delay一小段时间等待完成后继续进行
	//引脚初始化
	TFT_PinInit();
	//硬件复位
	TFT_RST_L();
	vTaskDelay(100);
	TFT_RST_H();
	vTaskDelay(50);
	//软件初始化
	TFT_SoftwareInit();
	vTaskDelay(50);
	//初始化图像
	uint8_t height = 140/3;
	uint8_t width = 162;
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
/**@brief （内部函数）向TFT发送8位指令
  *@param  cmd   要发送的指令
  *@retval void
  */
static void TFT_WriteCmd(uint8_t cmd)
{
	TFT_DC_L();
	TFT_SPI_SendData(cmd);
}
/**@brief （内部函数）向TFT发送8位数据
  *@param  data  要发送的数据
  *@retval void
  */
static void TFT_WriteData(uint8_t data)
{
	TFT_DC_H();
	TFT_SPI_SendData(data);
}
/**@brief  发送16位数据
  *@param  data  16位数据
  *@retval void
  */
void TFT_Write16Data(uint16_t RGB_565)
{
	TFT_DC_H();
	TFT_SPI_SendData((RGB_565>>8));
	TFT_SPI_SendData(RGB_565);
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
  */
void TFT_SetCursor(uint8_t x,uint8_t y,uint8_t weight,uint8_t height)
{
	TFT_WriteCmd(0x2a);//x轴
	TFT_WriteData(0x00);
	TFT_WriteData(x);//x起始
	TFT_WriteData(0x00);
	TFT_WriteData(x+weight-1);//x终止
	
	TFT_WriteCmd(0x2b);//y轴
	TFT_WriteData(0x00);
	TFT_WriteData(y);//y起始
	TFT_WriteData(0x00);
	TFT_WriteData(y+height-1);//y终止
	
	TFT_WriteCmd(0x2C);//开始写入
}
/**@brief  TFT单色清屏
  *@param  color 清屏颜色(RGB888)
  *@retval void
  */
void TFT_Clear(uint32_t RGB_888)
{
	TFT_SetCursor(0,0,160,160);
	uint16_t rgb565 = TFT_RGB888To565(RGB_888);
	for(int i=0;i<160*160;i++)
	{
		TFT_Write16Data(rgb565);
	}
}

void TFT_Test(void)
{
	U_Printf("TFT测试区 \r\n");
	//XY轴测试
	//红 黄
	//蓝 绿
	uint16_t tft_delay_time = 0;
	uint8_t tft_csdelay_time = 0;
	uint32_t RED=0xd95763;
	uint32_t YEL=0xfbf236;
	uint32_t BLU=0x5fcde4;
	uint32_t GRE=0x6abe30;
	TFT_SetCursor(0,0,50,50);
	vTaskDelay(tft_csdelay_time);
	uint16_t rgb565 = TFT_RGB888To565(RED);
	for(int i=0;i<50*50;i++)
	{
		TFT_Write16Data(rgb565);
		vTaskDelay(tft_delay_time);
	}
	TFT_SetCursor(50,0,50,50);
	vTaskDelay(tft_csdelay_time);
	rgb565 = TFT_RGB888To565(YEL);
	for(int i=0;i<50*50;i++)
	{
		TFT_Write16Data(rgb565);
		vTaskDelay(tft_delay_time);
	}
	TFT_SetCursor(0,50,50,50);
	vTaskDelay(tft_csdelay_time);
	rgb565 = TFT_RGB888To565(BLU);
	for(int i=0;i<50*50;i++)
	{
		TFT_Write16Data(rgb565);
		vTaskDelay(tft_delay_time);
	}	
	TFT_SetCursor(50,50,50,50);
	vTaskDelay(tft_csdelay_time);
	rgb565 = TFT_RGB888To565(GRE);
	for(int i=0;i<50*50;i++)
	{
		TFT_Write16Data(rgb565);
		vTaskDelay(tft_delay_time);
	}
}

/**@brief  TFT屏幕软件初始化
  *@param  void
  *@retval void
  */
static void TFT_SoftwareInit(void)
{		
		//解除睡眠
	TFT_WriteCmd(0x11);		//11h解除睡眠 10h进入睡眠
	vTaskDelay(120);
		//初始化
	
	//ST7735R Frame Rate
	TFT_WriteCmd(0xB1); 	//Frame Raye Control 帧频
	TFT_WriteData(0x01); 
	TFT_WriteData(0x2C); 
	TFT_WriteData(0x2D); 

	TFT_WriteCmd(0xB2); 
	TFT_WriteData(0x01); 
	TFT_WriteData(0x2C); 
	TFT_WriteData(0x2D); 

	TFT_WriteCmd(0xB3); 
	TFT_WriteData(0x01); 
	TFT_WriteData(0x2C); 
	TFT_WriteData(0x2D); 
	TFT_WriteData(0x01); 
	TFT_WriteData(0x2C); 
	TFT_WriteData(0x2D); 
	
	TFT_WriteCmd(0xB4);		//Display Inversion Control 反转控制
	TFT_WriteData(0x07); 
	
	//ST7735R Power Sequence
	TFT_WriteCmd(0xC0); 
	TFT_WriteData(0xA2); 
	TFT_WriteData(0x02); 
	TFT_WriteData(0x84); 
	TFT_WriteCmd(0xC1); 
	TFT_WriteData(0xC5); 

	TFT_WriteCmd(0xC2); 
	TFT_WriteData(0x0A); 
	TFT_WriteData(0x00); 

	TFT_WriteCmd(0xC3); 
	TFT_WriteData(0x8A); 
	TFT_WriteData(0x2A); 
	TFT_WriteCmd(0xC4); 
	TFT_WriteData(0x8A); 
	TFT_WriteData(0xEE); 
	
	TFT_WriteCmd(0xC5); //VCOM 
	TFT_WriteData(0x0E); 
	
	TFT_WriteCmd(0x36); 	//MX, MY, RGB mode 
	TFT_WriteData(0xE0);	//YXV0 0000 翻转 前两位分别是Y X
							//第三位V是XY控制交换

	//ST7735R Gamma Sequence
	TFT_WriteCmd(0xe0); 	//Gamma (‘+’polarity) Correction Characteristics Setting
	TFT_WriteData(0x0f); 	//伽马（正极性）
	TFT_WriteData(0x1a); 
	TFT_WriteData(0x0f); 
	TFT_WriteData(0x18); 
	TFT_WriteData(0x2f); 
	TFT_WriteData(0x28); 
	TFT_WriteData(0x20); 
	TFT_WriteData(0x22); 
	TFT_WriteData(0x1f); 
	TFT_WriteData(0x1b); 
	TFT_WriteData(0x23); 
	TFT_WriteData(0x37); 
	TFT_WriteData(0x00); 	
	TFT_WriteData(0x07); 
	TFT_WriteData(0x02); 
	TFT_WriteData(0x10); 

	TFT_WriteCmd(0xe1); 	//伽马（负极性）
	TFT_WriteData(0x0f); 
	TFT_WriteData(0x1b); 
	TFT_WriteData(0x0f); 
	TFT_WriteData(0x17); 
	TFT_WriteData(0x33); 
	TFT_WriteData(0x2c); 
	TFT_WriteData(0x29); 
	TFT_WriteData(0x2e); 
	TFT_WriteData(0x30); 
	TFT_WriteData(0x30); 
	TFT_WriteData(0x39); 
	TFT_WriteData(0x3f); 
	TFT_WriteData(0x00); 
	TFT_WriteData(0x07); 
	TFT_WriteData(0x03); 
	TFT_WriteData(0x10);  
	
	TFT_WriteCmd(0x2a);		//Column Address Set (CASET)
	TFT_WriteData(0x00);	//横向范围设置
	TFT_WriteData(0x00);	//起始位置低位 0
	TFT_WriteData(0x00);
	TFT_WriteData(0x7f);	//终止位置低位 127

	TFT_WriteCmd(0x2b);		//Row Address Set (RASET)
	TFT_WriteData(0x00);	//纵向范围设置
	TFT_WriteData(0x00);	//起始位置低位 0
	TFT_WriteData(0x00);
	TFT_WriteData(0x9f);	//终止位置低位 159
	
	TFT_WriteCmd(0xF0); //Enable test command  
	TFT_WriteData(0x01); 
	TFT_WriteCmd(0xF6); //Disable ram power save mode 
	TFT_WriteData(0x00); 
	
	TFT_WriteCmd(0x3A); //65k mode 
	TFT_WriteData(0x05); //RGB565
	
	
	TFT_WriteCmd(0x29);//Display on 29h打开显示 28h关闭显示
}









