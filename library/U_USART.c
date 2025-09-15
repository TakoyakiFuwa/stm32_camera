#include "U_USART.h"
/*  ST库  */
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"

/*	主要用于覆写printf	
 *	以及实现命令行
 *		——2025/5/20-15:02
 */

uint8_t USART_Buff[256];	 //缓冲区
int     USART_Buf_length = 0;//缓冲区有效数据长度
int8_t 	USART_RX_Signal  = 0;//接收到新数据的标志位 1代表有新数据

/**@brief  接口 用于初始化USART外设 按需修改
  *@param  void
  *@retval void
  *@add	   当前配置: USART2 
  *					 T:PA2	 R:PA3
  */
static void U_USART_Init(void)
{
	//时钟初始化
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	//引脚初始化
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART3);
	//USART外设初始化
	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate = 9600;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2,&USART_InitStruct);
	//USART中断配置
	USART_ITConfig(USART2,USART_IT_IDLE,ENABLE);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 5;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 5;
	NVIC_Init(&NVIC_InitStruct);
	//DMA配置 (USART2->DMA1 通道4 数据流5)
	USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);
	DMA_InitTypeDef DMA_InitStruct;
	DMA_StructInit(&DMA_InitStruct);
	DMA_InitStruct.DMA_BufferSize = 511;
	DMA_InitStruct.DMA_Channel = DMA_Channel_4;
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStruct.DMA_Memory0BaseAddr = (uint32_t)USART_Buff;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&(USART2->DR);
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;
	DMA_Init(DMA1_Stream5,&DMA_InitStruct);
	DMA_Cmd(DMA1_Stream5,ENABLE);
	//启用USART2外设
	USART_Cmd(USART2,ENABLE);
}
/**@brief  初始化
  */
void Init_USART(void)
{
	U_USART_Init();
	U_Printf("Init_USART初始化完成 \r\n");
}
/**@brief  通过USART发送一个字符
  *@param  CHAR	要发送的字符
  */
void U_Putchar(uint8_t CHAR)
{
	while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)!=SET);
	USART_SendData(USART2,CHAR);
}
/**@brief  USART2的中断处理接口
  */
void USART2_IRQHandler(void)
{
	if(USART_GetITStatus(USART2,USART_IT_IDLE)==SET)
	{
		uint16_t counts = 511 - DMA_GetCurrDataCounter(DMA1_Stream5);
		USART_Buff[counts++]='\0';
		DMA_Cmd(DMA1_Stream5,DISABLE);
		DMA_SetCurrDataCounter(DMA1_Stream5,511);
		DMA_Cmd(DMA1_Stream5,ENABLE);
		USART_Buf_length = counts;
		USART_RX_Signal = 1;
		//清楚空闲标志位
		USART2->SR;USART2->DR;
	}
}
/**@brief  测试接口
  */
void U_Test(void)
{
	U_Printf("这里是U_USART.c的测试接口 \r\n");
}



/*  关于命令行的实现  */
/**@brief  检测串口数据缓存和cmd是否一致
  *@param  cmd	命令行
  *@retval 0不一致 1一致
  */
int8_t Command(const char* cmd)
{
	 for(int i=0;cmd[i]!='\0';i++)
	 {
		if(cmd[i]!=USART_Buff[i])
		{
			return 0;
		}
	 }
	 return 1;
}

/*  下方是关于printf的实现 :)  */
/**@brief  发送一段文本
  *@param  words 要发送的文本
  *@retval void
  */
void U_WriteWords(const uint8_t* words)
{
	for(uint16_t i=0;words[i]!='\0';i++)
	{
		U_Putchar(words[i]);
	}
}
/**@brief  发送一个整数
  *@param  num 要发送的数字
  *@retval void
  */
void U_WriteNum(uint32_t num)
{
	if(num==0)
	{
		U_Putchar('0');
		return;
	}
	uint32_t num_length = 1;
	//测量数字位数
	for(;num_length<=num;num_length*=10);
	//从高位开始发送
	for(num_length/=10;num_length>=1;num_length/=10)
	{
		U_Putchar( num/num_length +'0');
		//减去最高位
		num -= (num - (num%num_length));
	}
}
/**@brief  把hex转成字符
  *@param  hex 要转的十六进制
  *@retval 字符
  *@add    内容太敷衍了 但是这个函数又不能没有...
  */
static char HexToChar(uint8_t hex)
{
	if(hex<10)
	{
		return hex+'0';
	}
	else 
	{
		return hex+'A'-10;
	}
}
/**@brief  发送一个十六进制
  *@param  hex 要发送的十六进制
  *@retval void
  */
void U_WriteHex(uint32_t hex)
{
	U_WriteWords("0x");
	if(hex==0)
	{
		U_Putchar('0');
		return;
	}
	//测量位数
	uint32_t hex_length = 0x01;
	int length_num = 0;
	for(;hex_length<=hex;hex_length = hex_length<<1)
	{
		length_num++;
	}
	while(length_num%4!=0)
	{
		length_num++;
		hex_length = hex_length<<1;
	}
	length_num /= 4;
	length_num--;
	//从高位发送数字
	for(;length_num>=0;length_num--)
	{
		U_Putchar(HexToChar( (( hex>>(length_num*4) )&0xF) ));
	}
}
/**@brief  发送二进制
  *@param  binary 要发送的二进制
  *@retval void
  */
void U_WriteBinary(uint32_t binary)
{
	U_WriteWords("0b");
	if(binary==0)
	{
		U_Putchar('0');
		return;
	}
	//测量长度
	uint32_t length = 0x1;
	uint32_t length_num=0;
	for(;length<=binary;length=length<<1)
	{
		length_num++;
	}
	while(length_num%4!=0)
	{
		length_num++;
		length=length<<1;
	}
	length=length>>1;
	for(;length>=0x01;length=length>>1)
	{
		if(length_num%4==0)
		{
			U_Putchar(' ');
		}
		if( (binary&length) == 0 )
		{
			U_Putchar('0');
		}
		else 
		{
			U_Putchar('1');
		}
		length_num--;
	}
}
/**@brief  printf
  *@param  ...
  *@retval void
  */
/*  变参函数  */
#include "stdarg.h"
void U_Printf(const char* words,...)
{
	va_list ap;
	va_start(ap,words);
	for(int i=0;words[i]!='\0';i++)
	{
		if(words[i]=='%')
		{
			switch(words[++i])
			{
			//整数
			case 'D':case 'd':U_WriteNum(va_arg(ap,uint32_t));break;
			//字符串
			case 'S':case 's':U_WriteWords(va_arg(ap,uint8_t*));break;
			//单个字符
			case 'C':case 'c':U_Putchar(va_arg(ap,int));break;
			//十六进制，建议使用标准库X
			case 'H':case 'h':case 'X':case 'x':U_WriteHex(va_arg(ap,uint32_t));break;
			//二进制
			case 'B':case 'b':U_WriteBinary(va_arg(ap,uint32_t));break;
			}
			continue;
		}
		U_Putchar(words[i]);
	}
	va_end(ap);
}
