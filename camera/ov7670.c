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

//高电平    GPIOx->BSRRL = GPIO_Pin;
//低电平    GPIOx->BSRRH = GPIO_Pin;
//SCL	->	PB9
#define	OV_SCL_H()	GPIOB->BSRRL = GPIO_Pin_9;for(int i=0;i<120;i++);
#define	OV_SCL_L()	GPIOB->BSRRH = GPIO_Pin_9;for(int i=0;i<120;i++);
//SDA	->	PD3
#define	OV_SDA_H()	GPIOD->BSRRL = GPIO_Pin_3;for(int i=0;i<100;i++);
#define	OV_SDA_L()	GPIOD->BSRRH = GPIO_Pin_3;for(int i=0;i<100;i++);
//SDA
#define OV_SDA_O()	GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_3)
//VS
#define OV_VS()		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8)
#define OV_HS()		GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4)
#define OV_PLK()	GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6)

/*  需要用的函数  */
void Test_OV(void);
void OV_SoftwareInit(void);
void OV_config_window(unsigned int startx,unsigned int starty,unsigned int width, unsigned int height);
void OV_XCLK(void);
void OV_XCLK_Init(void);

//用于随时更改SDA状态
void Init_OV(void)
{
	//引脚时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	//引脚初始化
	GPIO_InitTypeDef GPIO_InitStruct;
	//单片机向摄像头输出
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
		//GND和VCC内容
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_4;
	GPIO_Init(GPIOD,&GPIO_InitStruct);
	GPIO_WriteBit(GPIOD,GPIO_Pin_5,Bit_SET);
	GPIO_WriteBit(GPIOD,GPIO_Pin_4,Bit_RESET);
		//RST	1运行
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOD,&GPIO_InitStruct);
	GPIO_WriteBit(GPIOD,GPIO_Pin_6,Bit_SET);
		//PWDN 0正常模式
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOD,&GPIO_InitStruct);
	GPIO_WriteBit(GPIOD,GPIO_Pin_7,Bit_RESET);
		//SCCB内容
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;//PB9->SCL
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	OV_SCL_H();
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;//PD3->SDA
	GPIO_Init(GPIOD,&GPIO_InitStruct);
	
	//摄像头向单片机
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
		//D0~D7
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_Init(GPIOC,&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6;
	GPIO_Init(GPIOE,&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
		//VS
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
		//HS
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
		//PLK
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	//XCLK时钟处理
	OV_XCLK_Init();
	
	//软件初始化
	vTaskDelay(100);
	OV_SoftwareInit();
	OV_config_window(272,16,240*2,240);
	
	Test_OV();
	
	U_Printf("这里是OV7670初始化 \r\n");
}
void OV_XCLK_Init(void)
{
	//时钟处理
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource8,GPIO_AF_MCO);
	RCC_MCO1Config(RCC_MCO1Source_HSI,RCC_MCO1Div_1);	//16MHz
}
void OV_XCLK_DeInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
}
void OV_XCLK(void)
{
	GPIO_WriteBit(GPIOA,GPIO_Pin_8,Bit_SET);
	GPIO_WriteBit(GPIOA,GPIO_Pin_8,Bit_RESET);
}
void OV_ChangeSDA(GPIOMode_TypeDef GPIO_Mode_X)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_X;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOD,&GPIO_InitStruct);
}

/*  以上是需要引脚适配的接口  */

void SCCB_Start(void)
{
	OV_SDA_H();
	OV_SCL_H();
	OV_SDA_L();
	OV_SCL_L();
}
void SCCB_End(void)
{
	OV_SDA_L();
	OV_SCL_H();
	OV_SDA_H();
}
void SCCB_SendAct(uint8_t act)
{
	OV_SCL_L();
	if(act==0)
	{
		OV_SDA_L();
	}
	else
	{
		OV_SDA_H();
	}
	OV_SCL_H();
	OV_SCL_L();
}
uint8_t SCCB_ReceiveAct(void)
{
	uint8_t act = 0;
	OV_ChangeSDA(GPIO_Mode_IN);
	OV_SCL_H();
	act=OV_SDA_O();
	OV_SCL_L();
	OV_ChangeSDA(GPIO_Mode_OUT);
	if(act!=0)
	{
		U_Printf("OV7670初始化通讯异常 \r\n");
	}
	return act;
}
void SCCB_SendByte(uint8_t data)
{
	for(int i=0;i<8;i++)
	{
		OV_SCL_L();
		if( (data&(0x80>>i)) != 0 )
		{
			OV_SDA_H();
		}
		else
		{
			OV_SDA_L();
		}
		OV_SCL_H();
	}
	OV_SCL_L();
}
uint8_t SCCB_ReceiveByte(void)
{
	uint8_t data = 0 ;
	uint8_t temp;
	OV_ChangeSDA(GPIO_Mode_IN);
	for(int i=0;i<8;i++)
	{
		data =data<<1;
		OV_SCL_L();
		temp = OV_SDA_O();
		OV_SCL_H();
		if( temp != 0 )
		{
			data+=1;
		}
	}
	OV_SCL_L();
	OV_ChangeSDA(GPIO_Mode_OUT);
	return data;
}
void SCCB_WriteReg(uint8_t addr,uint8_t data)
{
	SCCB_Start();
	SCCB_SendByte(0x42);
	SCCB_ReceiveAct();
	SCCB_SendByte(addr);
	SCCB_ReceiveAct();
	SCCB_SendByte(data);
	SCCB_ReceiveAct();
	SCCB_End();
}
uint8_t SCCB_ReadReg(uint8_t addr)
{
	SCCB_Start();
	SCCB_SendByte(0x42);
	SCCB_ReceiveAct();
	SCCB_SendByte(addr);
	SCCB_ReceiveAct();
	SCCB_End();
	SCCB_Start();
	SCCB_SendByte(0x43);
	SCCB_ReceiveAct();
	uint8_t data = SCCB_ReceiveByte();
	SCCB_SendAct(1);
	SCCB_End();
	return data;
}

/*  接受RGB数据  */
uint8_t OV_RGBData(void)
{
	uint8_t data = 0;
	data<<=1;data += GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6);
	data<<=1;data += GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_5);
	data<<=1;data += GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_6);
	data<<=1;data += GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4);
	data<<=1;data += GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_1);
	data<<=1;data += GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0);
	data<<=1;data += GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7);
	data<<=1;data += GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_6);
	return data;
}
void OV_PixelsGet(uint8_t scale,void (*Func)(uint16_t))
{
	uint8_t quit_signal = 0;
	uint8_t is_read = 0;
	uint8_t is_high = 1;
	uint16_t pixel = 0;
	uint32_t pixel_count = 0;
	
	while(1)
	{
		while(OV_VS()==0);
		while(OV_VS()!=0);
		OV_XCLK_DeInit();
		while(OV_VS()==0)
		{
			quit_signal = 1;
			while(OV_HS()!=0)
			{
				if(OV_VS()!=0)
				{
					break;
				}	
				if(OV_PLK()==0)
				{
					is_read = 1;
				}
				if(OV_PLK()!=0&&is_read!=0)
				{
					is_read = 0;
					if(is_high!=0)
					{
						is_high = 0;
						pixel = OV_RGBData();
						pixel<<=8;
					}
					else
					{
						is_high++;
						pixel_count++;
						pixel |= OV_RGBData();
						//涉及TFT算法
						//输出图像是240*240/scale
						if(pixel_count%scale==0&&pixel_count%(240*scale)<240)
						{
							Func(pixel);
						}
					}
				}
				OV_XCLK();
			}
			OV_XCLK();
		}
		if(quit_signal!=0)
		{
			OV_XCLK_Init();
			U_Printf("pixel_count:%d\r\n",pixel_count);
			break;
		}
	}	
}


void Test_OV(void)
{	
	
}

void OV_config_window(unsigned int startx,unsigned int starty,unsigned int width, unsigned int height)
{
	unsigned int endx;
	unsigned int endy;// "v*2"必须
	unsigned char temp_reg1, temp_reg2;
	unsigned char temp=0;
	
	endx=(startx+width);
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
void OV_SoftwareInit(void)
{
	SCCB_WriteReg(0x12, 0x80);	//SCCB复位
	vTaskDelay(200);            
	SCCB_WriteReg(0x8c, 0x00);    //RGB444
	SCCB_WriteReg(0x3a, 0x04);    //行缓冲测试
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
	SCCB_WriteReg(0x11, 0x08);    //PCLK 时钟分频 fps 数值越大刷新越慢但越稳定 建议0x07
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
	 
	SCCB_WriteReg(0x32,0xb6);
	
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


