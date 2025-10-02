#include "bmp.h"
/*  ST库  */
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
/*  OS库  */
#include "FreeRTOS.h"
#include "task.h"
/*  外设库  */
#include "U_USART.h"
#include "ov7670.h"
#include "TFT_ST7735.h"
/*  FATFS  */
#include "ff.h"

FATFS fs;

/**@brief  SD卡挂载
  *@add    SD卡的驱动文件好像有没什么用的引脚初始化，应该把这个初始化放在前面...
  *		   避免SDIO改变其他引脚...
  */
void Init_BMP(void)
{
	if(f_mount(&fs,"0:",1)!=FR_OK)
	{
		U_Printf("SD卡初始化异常,代码:%d \r\n",f_mount(&fs,"0:",1));
		return;
	}
	else
	{
		U_Printf("Init_BMP：正常挂载SD卡，初始化完成 \r\n");
	}
}
void Cmd_BMP(void)
{	
	
	U_Printf("这里是BMP的指令 \r\n");
}

