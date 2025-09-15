#include "ov7670.h"
/*  串口  */
#include "U_USART.h"
/*  系统  */
#include "FreeRTOS.h"
#include "task.h"

/*	不是....
 *	引脚也太多了点....
 *		2025/9/13-17:07.秦羽
 */
/*	完全基于软件的方式驱动摄像头
 *	感觉甚至可以用F1使用....
 *	但刷新效率很慢很慢，
 *	仅用于电路板和相机驱动测试
 *	摄像头输出尺寸在Init_OV的OV_config_windows改后两个参数
 *		2025/9/14-10:52.秦羽
 */
 
/*	这里是f4_ui板上的相机接口测试
 *	PD5		->	VCC
 *	PB9		->	SCL		//摄像头在上升沿读取
 *	PB8		->	VS
 *	PA6		->	PLK
 *	PE6		->	D7
 *	PB6		->	D5
 *	PE1		->	D3
 *	PC7		->	D1
 *	PD6		->	RST		//0复位 1运行
 *	PD7		->	PWDN	//0正常模式 1省电模式
 *	PC6		->	D0
 *	PE0		->	D2
 *	PE4		->	D4
 *	PE5		->	D6
 *	PA8		->	XLK
 *	PA4		->	HS
 *	PD3		->	SDA		//高位在前
 *	PD4		->	GND
 */
#define OV_Output_width 	161		//最高好像是314 且会有黑边
#define OV_Output_height	131		//最高248 达到240标准

//SCL
#define OV_SCL(x)	GPIO_WriteBit(GPIOB,GPIO_Pin_9,(BitAction)x);for(int i=0;i<100;i++);
//SDA
#define OV_SDA(x)	GPIO_WriteBit(GPIOD,GPIO_Pin_3,(BitAction)x);for(int i=0;i<100;i++);
#define OV_SDA_D()	GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_3)
//VS
#define OV_VS()		(GPIOB->IDR & GPIO_Pin_8)
//HS
#define OV_HS()		(GPIOA->IDR & GPIO_Pin_4)
//PLK
#define OV_PLK()	(GPIOA->IDR & GPIO_Pin_6)


/*  以下是引脚初始化相关内容  */
/*  也是进行移植测试时需要更改的接口  */


/**@brief 可直接用于测试的线程
  *@-
  */
#include "TFT_ST7735.h"
//#include "TFT_ILI9341.h"
void OV_FuncPixel(uint8_t data)
{
	static uint8_t msb_pixel = 1;
	static uint16_t rgb565;
	if(msb_pixel==1)
	{
		msb_pixel = 0;
		rgb565 = data;
		rgb565<<=8;
	}
	else
	{
		msb_pixel = 1;
		rgb565+=data;
		TFT_Write16Data(rgb565);
//		ILI_SendColor(rgb565);
	}
}
void Task_Camera(void* pvParameters)
{
	while(1)
	{
		vTaskDelay(10);
		TFT_SetCursor(0,0,OV_Output_width,OV_Output_height);
//		ILI_SetRect(0,0,OV_Output_width,OV_Output_height);
		OV_PixelsGet(OV_FuncPixel);
	}
}
/**@brief  XCLK采用硬件方式
  *@param  void
  *@retval void
  */
void OV_XCLK_ON(void)
{
	//引脚初始化
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	//复用功能初始化
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource8,GPIO_AF_MCO);
	RCC_MCO1Config(RCC_MCO1Source_HSI,RCC_MCO1Div_1);
}
/**@brief  XCLK采用软件方式
  *@param  void
  *@retval void
  */
void OV_XCLK_OFF(void)
{
	//引脚初始化
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
}
/**@brief  软件方式进行一次震荡
  */
void OV_XCLK(void)
{
	GPIO_WriteBit(GPIOA,GPIO_Pin_8,Bit_SET);
	GPIO_WriteBit(GPIOA,GPIO_Pin_8,Bit_RESET);
}
/**@brief  设置SDA线的方式
  *@param  GPIO_Mode_x 要设置成输出/输入的方式
  *@retval void
  */
void OV_SDA_Set(GPIOMode_TypeDef GPIO_Mode_x)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_x;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_Init(GPIOD,&GPIO_InitStruct);
}
/**@brief  引脚初始化
  *@param  void
  *@retval void
  */
static void OV_PinInit(void)
{
	//时钟初始化
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	//单片机向摄像头输出引脚初始化
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
		//VCC
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOD,&GPIO_InitStruct);
	GPIO_WriteBit(GPIOD,GPIO_Pin_5,Bit_SET);
		//SCL
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	OV_SCL(1);
		//RST
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOD,&GPIO_InitStruct);
	GPIO_WriteBit(GPIOD,GPIO_Pin_6,Bit_SET);
		//PWDN
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOD,&GPIO_InitStruct);
	GPIO_WriteBit(GPIOD,GPIO_Pin_7,Bit_RESET);
		//SDA
	OV_SDA_Set(GPIO_Mode_OUT);
		//XLK (PA8)
	OV_XCLK_ON();
}
/**@brief  软件引脚读一个像素
  *@retval 读出的像素
  */
uint8_t OV_RGBData(void)
{
	uint8_t data = 0;
	//(GPIOx->IDR & GPIO_Pin)
	data<<=1;if((GPIOE->IDR & GPIO_Pin_6)!=0){data+=1;}//D7
	data<<=1;if((GPIOE->IDR & GPIO_Pin_5)!=0){data+=1;}
	data<<=1;if((GPIOB->IDR & GPIO_Pin_6)!=0){data+=1;}
	data<<=1;if((GPIOE->IDR & GPIO_Pin_4)!=0){data+=1;}
	data<<=1;if((GPIOE->IDR & GPIO_Pin_1)!=0){data+=1;}
	data<<=1;if((GPIOE->IDR & GPIO_Pin_0)!=0){data+=1;}
	data<<=1;if((GPIOC->IDR & GPIO_Pin_7)!=0){data+=1;}
	data<<=1;if((GPIOC->IDR & GPIO_Pin_6)!=0){data+=1;}//D0
	return data;
}


/*  以下是SCCB通讯相关内容  */
/*	通讯时SCL处于拉低的状态
 *	结束通信空闲时SCL应该一直处于上拉状态
 *	体现在SCCB_End最后没有拉低SCL
 *	很重要，可能会影响通讯时序
 */
/**@brief  开始通讯
  *@add    SCL高电平，SDA下降沿开始
  */
void SCCB_Start(void)
{
	OV_SDA(1);
	OV_SCL(1);
	OV_SDA(0);
	OV_SCL(0);
}
/**@brief  结束通讯
  *@add    SCL高电平，SDA上升沿结束通讯
  */
void SCCB_End(void)
{
	OV_SDA(0);
	OV_SCL(1);
	OV_SDA(1);
}
/**@brief  发送应答
  *@param  act	0打算继续接收数据 1结束接收数据
  */
void SCCB_SendAct(int8_t act)
{
	if(act!=0)
	{
		OV_SDA(1);
	}
	else
	{
		OV_SDA(0);
	}
	OV_SCL(1);
	OV_SCL(0);
}
/**@brief  接收一次应答
  *@retval 应答 0表示正常
  */
uint8_t SCCB_ReceiveAct(void)
{
	OV_SDA_Set(GPIO_Mode_IN);
	OV_SCL(1);
	uint8_t act = OV_SDA_D();
	OV_SCL(0);
	OV_SDA_Set(GPIO_Mode_OUT);
	if(act!=0)
	{
		U_Printf("ov7670-SCCB通讯出现异常 \r\n");
	}
	return act;
}
/**@brief  SCCB发送1Byte数据
  *@param  byte 要发送的数据
  */
void SCCB_SendByte(uint8_t byte)
{
	for(int i=0;i<8;i++)
	{
		if( (byte&(0x80>>i)) == 0 )
		{
			OV_SDA(0);
		}
		else
		{
			OV_SDA(1);
		}
		OV_SCL(1);
		OV_SCL(0);
	}
}
/**@brief  接收1Byte数据
  *@retval 接收到的数据
  */
uint8_t SCCB_ReceiveByte(void)
{
	uint8_t pin = 0;
	uint8_t data = 0;
	OV_SDA_Set(GPIO_Mode_IN);
	for(int i=0;i<8;i++)
	{
		data<<=1;
		OV_SCL(1);
		pin = OV_SDA_D();
		if(pin!=0)
		{
			data+=1;
		}
		OV_SCL(0);
	}
	OV_SDA_Set(GPIO_Mode_OUT);
	return data;
}
/**@brief  向寄存器写入数据
  *@param  addr  寄存器地址
  *@param  data  要写入的数据
  *@retval void
  */
void SCCB_WriteReg(uint8_t addr,uint8_t data)
{
	//指定地址
	SCCB_Start();
	SCCB_SendByte(0x42);
	SCCB_ReceiveAct();
	SCCB_SendByte(addr);
	SCCB_ReceiveAct();
	//写入数据
	SCCB_SendByte(data);
	SCCB_ReceiveAct();
	SCCB_End();
}
/**@brief  读取寄存器数据
  *@param  addr  寄存器地址
  *@retval 读取到的数据
  */
uint8_t SCCB_ReadReg(uint8_t addr)
{
	//指定地址
	SCCB_Start();
	SCCB_SendByte(0x42);
	SCCB_ReceiveAct();
	SCCB_SendByte(addr);
	SCCB_ReceiveAct();
	SCCB_End();
	//接受数据
	SCCB_Start();
	SCCB_SendByte(0x43);
	SCCB_ReceiveAct();
	uint8_t data = SCCB_ReceiveByte();
	SCCB_SendAct(1);
	SCCB_End();
	return data;
}
/**@breif  测试，扫描硬件应答ID
  *@add    OV7670的应答ID应该是0x42/0x43
  */
void SCCB_Test_ScanID(void)
{
	uint8_t act = 0;
	for(int i=0;i<0xFF;i++)
	{
		SCCB_Start();
		SCCB_SendByte(i);
		act = SCCB_ReceiveAct();
		SCCB_End();
		U_Printf("[%h]:%d \r\n",i,act);
		if(act==0)
		{
			U_Printf("OV7670的ID号应该是0x42和0x43 \r\n");
			break;
		}
	}
}
/**@brief  测试，读取厂商ID号
  *@add    应该是0x76 0x73
  */
void SCCB_Test_IDNumber(void)
{
	uint8_t MSB,LSB;
	MSB = SCCB_ReadReg(0x0A);
	LSB = SCCB_ReadReg(0x0B);
	U_Printf("ov7670的ID应当是0x76-0x73:[%h-%h]",MSB,LSB);
}
/**@brief  初始化
  */
static void OV_config_window(unsigned int startx,unsigned int starty,unsigned int width, unsigned int height);
static void OV_SoftwareInit(void);
void Init_OV(void)
{
	OV_PinInit();
		//这个初始化延迟很有必要
	vTaskDelay(100);
		//软件初始化
	OV_SoftwareInit();
		//设置窗口位置
	OV_config_window(152,0,OV_Output_width,OV_Output_height);
	
	
	U_Printf("ov7670(相机)初始化完成 \r\n");
}


/*  读取像素  */
/**@brief  获取像素并处理
  *@param  scale  缩放
  *@param  Func	  像素处理函数
  *@retval void
  */
void OV_PixelsGet(void (*Func)(uint8_t))
{	
	while(OV_VS()==0);
	while(OV_VS()!=0);
	OV_XCLK_OFF();
	while(OV_VS()==0)
	{
		while(OV_HS()!=0)
		{
			while(OV_PLK()==0)
			{
				OV_XCLK();
			}
			Func(OV_RGBData());
			while(OV_PLK()!=0)
			{
				OV_XCLK();
			}
		}
		OV_XCLK();
	}
	OV_XCLK_ON();
}





/*  所有的软件初始化有够...长...  */
/**@brief  接收图像窗口设置
  */
static void OV_config_window(unsigned int startx,unsigned int starty,unsigned int width, unsigned int height)
{
	unsigned int endx;
	unsigned int endy;// "v*2"必须
	unsigned char temp_reg1, temp_reg2;
	unsigned char temp=0;
	
	endx=(startx+width+width);
	endy=(starty+height+height);// "v*2"必须
    temp_reg1 = SCCB_ReadReg(0x03);
	temp_reg1 &= 0xf0;
	temp_reg2 = SCCB_ReadReg(0x32);
	temp_reg2 &= 0xc0;
	
	// Horizontal
	temp = temp_reg2|((endx&0x7)<<3)|(startx&0x7);
	SCCB_WriteReg(0x32, temp );
	temp = (startx&0x7F8)>>3;
	SCCB_WriteReg(0x17, temp );
	temp = (endx&0x7F8)>>3;
	SCCB_WriteReg(0x18, temp );
	
	// Vertical
	temp =temp_reg1|((endy&0x3)<<2)|(starty&0x3);
	SCCB_WriteReg(0x03,temp);
	temp = starty>>2;
	SCCB_WriteReg(0x19, temp );
	temp = endy>>2;
	SCCB_WriteReg(0x1A, temp );
}
/**@brief  软件初始化
  */
static void OV_SoftwareInit(void)
{
	SCCB_WriteReg(0x12, 0x80);	//SCCB复位
	vTaskDelay(100);            
	SCCB_WriteReg(0x8c, 0x00);    //RGB444
	SCCB_WriteReg(0x3a, 0x00);    //行缓冲测试
	SCCB_WriteReg(0x40, 0xd0);    //RGB565
	SCCB_WriteReg(0x8c, 0x00);    //RGB444
	SCCB_WriteReg(0x12, 0x14);    //QVGA RGB
	SCCB_WriteReg(0x32, 0x80);    //HREF
	SCCB_WriteReg(0x17, 0x16);    //输出格式-行频开始高八位	0001 0110
	SCCB_WriteReg(0x18, 0x04);    //输出格式-行频结束高八位	0000 0100
	SCCB_WriteReg(0x19, 0x02);    //输出格式-场频开始高八位	0000 0010
	SCCB_WriteReg(0x1a, 0x7b);    //输出格式-场频结束高八位	0111 1011
	SCCB_WriteReg(0x03, 0x06);    //帧竖直方向控制		0000 0110
	SCCB_WriteReg(0x0c, 0x04);    
	SCCB_WriteReg(0x3e, 0x00);    
	SCCB_WriteReg(0x70, 0x3a);    
	SCCB_WriteReg(0x71, 0x35);    
	SCCB_WriteReg(0x72, 0x11);    
	SCCB_WriteReg(0x73, 0x70);    //PCLK DIV						建议0x70
	SCCB_WriteReg(0xa2, 0x01);    //PCLK Delay	像素延时//0100	建议0x01
	SCCB_WriteReg(0x11, 0x09);    //PCLK 时钟分频 fps 数值越大刷新越慢但越稳定 建议0x07
	//SCCB_WriteReg(0x15 , 0x31);
	SCCB_WriteReg(0x7a, 0x20); 
	SCCB_WriteReg(0x7b, 0x1c); 
	SCCB_WriteReg(0x7c, 0x28); 
	SCCB_WriteReg(0x7d, 0x3c);
	SCCB_WriteReg(0x7e, 0x55); 
	SCCB_WriteReg(0x7f, 0x68); 
	SCCB_WriteReg(0x80, 0x76);
	SCCB_WriteReg(0x81, 0x80); 
	SCCB_WriteReg(0x82, 0x88);
	SCCB_WriteReg(0x83, 0x8f);
	SCCB_WriteReg(0x84, 0x96); 
	SCCB_WriteReg(0x85, 0xa3);
	SCCB_WriteReg(0x86, 0xaf);
	SCCB_WriteReg(0x87, 0xc4); 
	SCCB_WriteReg(0x88, 0xd7);
	SCCB_WriteReg(0x89, 0xe8);
	 
	SCCB_WriteReg(0x32, 0xb6);
	
	SCCB_WriteReg(0x13, 0xff); 
	SCCB_WriteReg(0x00, 0x00);
	SCCB_WriteReg(0x10, 0x00);
	SCCB_WriteReg(0x0d, 0x00);
	SCCB_WriteReg(0x14, 0x4e);
	SCCB_WriteReg(0xa5, 0x05);
	SCCB_WriteReg(0xab, 0x07); 
	SCCB_WriteReg(0x24, 0x75);
	SCCB_WriteReg(0x25, 0x63);
	SCCB_WriteReg(0x26, 0xA5);
	SCCB_WriteReg(0x9f, 0x78);
	SCCB_WriteReg(0xa0, 0x68);
//	SCCB_WriteReg(0xa1, 0x03);//0x0b,
	SCCB_WriteReg(0xa6, 0xdf);
	SCCB_WriteReg(0xa7, 0xdf);
	SCCB_WriteReg(0xa8, 0xf0); 
	SCCB_WriteReg(0xa9, 0x90); 
	SCCB_WriteReg(0xaa, 0x94); 
	//SCCB_WriteReg(0x13, 0xe5); 
	SCCB_WriteReg(0x0e, 0x61);
	SCCB_WriteReg(0x0f, 0x43);
	SCCB_WriteReg(0x16, 0x02); 
	SCCB_WriteReg(0x1e, 0x37);
	SCCB_WriteReg(0x21, 0x02);
	SCCB_WriteReg(0x22, 0x91);
	SCCB_WriteReg(0x29, 0x07);
	SCCB_WriteReg(0x33, 0x0b);
	SCCB_WriteReg(0x35, 0x0b);
	SCCB_WriteReg(0x37, 0x3f);
	SCCB_WriteReg(0x38, 0x01);
	SCCB_WriteReg(0x39, 0x00);
	SCCB_WriteReg(0x3c, 0x78);
	SCCB_WriteReg(0x4d, 0x40);
	SCCB_WriteReg(0x4e, 0x20);
	SCCB_WriteReg(0x69, 0x00);
	SCCB_WriteReg(0x6b, 0x4a);
	SCCB_WriteReg(0x74, 0x19);
	SCCB_WriteReg(0x8d, 0x4f);
	SCCB_WriteReg(0x8e, 0x00);
	SCCB_WriteReg(0x8f, 0x00);
	SCCB_WriteReg(0x90, 0x00);
	SCCB_WriteReg(0x91, 0x00);
	SCCB_WriteReg(0x92, 0x00);
	SCCB_WriteReg(0x96, 0x00);
	SCCB_WriteReg(0x9a, 0x80);
	SCCB_WriteReg(0xb0, 0x84);
	SCCB_WriteReg(0xb1, 0x0c);
	SCCB_WriteReg(0xb2, 0x0e);
	SCCB_WriteReg(0xb3, 0x82);
	SCCB_WriteReg(0xb8, 0x0a);
	SCCB_WriteReg(0x43, 0x14);
	SCCB_WriteReg(0x44, 0xf0);
	SCCB_WriteReg(0x45, 0x34);
	SCCB_WriteReg(0x46, 0x58);
	SCCB_WriteReg(0x47, 0x28);
	SCCB_WriteReg(0x48, 0x3a);
	
	SCCB_WriteReg(0x59, 0x88);
	SCCB_WriteReg(0x5a, 0x88);
	SCCB_WriteReg(0x5b, 0x44);
	SCCB_WriteReg(0x5c, 0x67);
	SCCB_WriteReg(0x5d, 0x49);
	SCCB_WriteReg(0x5e, 0x0e);
	
	SCCB_WriteReg(0x64, 0x04);
	SCCB_WriteReg(0x65, 0x20);
	SCCB_WriteReg(0x66, 0x05);

	SCCB_WriteReg(0x94, 0x04);
	SCCB_WriteReg(0x95, 0x08);	 

	SCCB_WriteReg(0x6c, 0x0a);
	SCCB_WriteReg(0x6d, 0x55);
	SCCB_WriteReg(0x6e, 0x11);
	SCCB_WriteReg(0x6f, 0x9f); 

	SCCB_WriteReg(0x6a, 0x40);
	SCCB_WriteReg(0x01, 0x40);
	SCCB_WriteReg(0x02, 0x40);
	
	//SCCB_WriteReg(0x13, 0xe7);
	SCCB_WriteReg(0x15, 0x00);
	SCCB_WriteReg(0x4f, 0x80);
	SCCB_WriteReg(0x50, 0x80);
	SCCB_WriteReg(0x51, 0x00);
	SCCB_WriteReg(0x52, 0x22);
	SCCB_WriteReg(0x53, 0x5e);
	SCCB_WriteReg(0x54, 0x80);
	SCCB_WriteReg(0x58, 0x9e);

	SCCB_WriteReg(0x41, 0x08);
	SCCB_WriteReg(0x3f, 0x00);
	SCCB_WriteReg(0x75, 0x05);
	SCCB_WriteReg(0x76, 0xe1);

	SCCB_WriteReg(0x4c, 0x00);
	SCCB_WriteReg(0x77, 0x01);
	
	SCCB_WriteReg(0x3d, 0xc1);
	SCCB_WriteReg(0x4b, 0x09);
	SCCB_WriteReg(0xc9, 0x60);
	//SCCB_WriteReg(0x41, 0x38);	
	SCCB_WriteReg(0x56, 0x40);
	SCCB_WriteReg(0x34, 0x11);
	SCCB_WriteReg(0x3b, 0x02);
	SCCB_WriteReg(0xa4, 0x89);
	
	SCCB_WriteReg(0x96, 0x00);
	SCCB_WriteReg(0x97, 0x30);
	SCCB_WriteReg(0x98, 0x20);
	SCCB_WriteReg(0x99, 0x30);
	SCCB_WriteReg(0x9a, 0x84);
	SCCB_WriteReg(0x9b, 0x29);
	SCCB_WriteReg(0x9c, 0x03);
	SCCB_WriteReg(0x9d, 0x4c);
	SCCB_WriteReg(0x9e, 0x3f);	

	SCCB_WriteReg(0x09, 0x00);
	SCCB_WriteReg(0x3b, 0xc2);
}


