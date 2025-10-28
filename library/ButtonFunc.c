#include "ButtonFunc.h"
/*  ST库  */
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
/*  OS库  */
#include "FreeRTOS.h"
#include "task.h"
//
#include "Botton.h"
/*  串口库  */
#include "U_USART.h"
/*  SD卡  */
#include "bmp.h"
/*  fatfs  */
#include "ff.h"
/*  UI库  */
#include "UI_Core.h"
#include "UI_Render.h"
#include "UI_Instance.h"
/*  宏定义库  */
#include "cmr_def.h"

/*  UI_Core.c  */
extern qy_pointer	CURSOR;
extern qy_ui		UI[];
extern qy_page		PAGE[];
/*  Shadow.c  */
extern uint8_t camera_data[];
extern uint16_t pic_index;
extern uint16_t pic_num;
extern int8_t SD_on;
extern int8_t camera_on;
extern const char BMP_PATH_bmp[];
extern const char BMP_PATH_fast[]; 
/*  Botton.c  */
extern void (*BOT_LEFT_before)(void);
extern void (*BOT_LEFT_long)(void);
extern void (*BOT_LEFT_after)(void);
extern void (*BOT_MIDDLE_before)(void);
extern void (*BOT_MIDDLE_long)(void);
extern void (*BOT_MIDDLE_after)(void);
extern void (*BOT_RIGHT_before)(void);
extern void (*BOT_RIGHT_long)(void);
extern void (*BOT_RIGHT_after)(void);

/*	我刚刚想了一下....	
 *	我不是有....有做UI处理吗...
 *	是不是可以把按键接入UI处理....(?)
 *			——2025/10/28.19:53
 */

void Init_BUT(void)
{
	U_Printf("BUT初始化 \r\n");
	//按键映射
	BOT_RIGHT_after = BUT_KeepPhoto;
	BOT_MIDDLE_after = BUT_AlbumControl;
	BOT_LEFT_after = BUT_Album_Prior;

	//相片数查询
	pic_index = BUT_FindMaxNum(BMP_PATH_fast,&pic_num);
	UI_AddRender(&UI[InUI_Fix_PicNum]);
	RenderCircle_UI();
}

uint16_t BUT_FindMaxNum(const char* path,uint16_t* total_num)
{
	if(SD_on!=1)
	{
		return 0;
	}
	uint16_t max_num = 0;
	uint16_t _total_num = 0;
	DIR dp;
	FILINFO info;
	f_opendir(&dp,path);
	f_readdir(&dp,&info);
	max_num = BMP_StringToNum(info.fname);
	uint16_t temp_num = 0;
	while(info.fname[0]!='\0')
	{
		_total_num++;
		temp_num = BMP_StringToNum(info.fname);
		max_num = max_num>temp_num?max_num:temp_num;
		f_readdir(&dp,&info);
	}
	f_closedir(&dp);
	U_Printf("SD_FindMaxNum(%s):%d , 共[%d]个文件 \r\n",path,max_num,_total_num);
	if(total_num!=0)
	{
		*total_num = _total_num;
	}
	return max_num;
}
#include "TFT_ST7789V.h"
void BUT_KeepPhoto(void)
{
	//关闭摄影功能
	camera_on = 0;
	//蓝粉白
	uint16_t rgb565 = TFT_RGB888To565(0xffc7c7);//0xFE38
	UIR_DrawRect(0,0,304,80,rgb565);
	rgb565 = TFT_RGB888To565(0xf6f6f6);//0xF7BE
	UIR_DrawRect(0,80,304,80,rgb565);
	rgb565 = TFT_RGB888To565(0x71c9ce);//0x7659
	UIR_DrawRect(0,160,304,80,rgb565);
	UI_AddRender(&UI[InUI_Fix_BKQY]);
	RenderCircle_UI();
	//
	if(SD_on!=1)
	{
		U_Printf("SD卡异常，无法保存图片  \r\n");
		UIR_ShowString(5,80,"SD Error,  :(",18,InFT_Consolas_3216,0xF030,0xF7BE);
		UIR_ShowString(5,120,"Can't Keep Photo.",18,InFT_Consolas_3216,0x0,0xF7BE);
		vTaskDelay(1500);
			//继续摄影功能
		camera_on = 1;
		return;
	}
	//快速写入
	uint8_t path[6];
	BMP_NumToString(++pic_index,(char*)path);
	SD_Fast_Write((const char*)path,(uint16_t*)&camera_data[0],DEF_PIC_HEIGHT*DEF_PIC_WIDTH);
	pic_num++;
		//重新渲染数量
	UI_AddRender(&UI[InUI_Fix_PicNum]);
	RenderCircle_UI();
	//继续摄影功能
	camera_on = 1;
}
//关于相册，这里最合适的做法是...
//打开UI 让UI管理接管按键
#include "Func.h"
int16_t album_index = 1;
int16_t album_num = 1;
void BUT_AlbumControl(void)
{
	if(camera_on==1)
	{
		//状态位和功能键
		camera_on = 0;
		BOT_RIGHT_after = BUT_Album_Next;
		//显示第一张图片
		album_index = pic_index;
		album_num = pic_num;
		char path[6];
		BMP_NumToString(album_index,path);
		SD_Fast_Read((const char*)path,(uint16_t*)&camera_data[0],DEF_PIC_HEIGHT*DEF_PIC_WIDTH);
		Func_TFT_Show();
		//渲染侧边栏
		UI_ChangePage(InPG_Album);
		RenderCircle_UI();
	}
	else
	{
		BOT_RIGHT_after = BUT_KeepPhoto;
		//重新渲染侧边栏
		UI_ChangePage(InPG_Fix);
		RenderCircle_UI();
		camera_on = 1;
	}
}
void BUT_Album_Next(void)
{
	++album_num;
	++album_index;
	char path[6];
	BMP_NumToString(album_index,path);
	while(SD_Fast_Read((const char*)path,(uint16_t*)&camera_data[0],DEF_PIC_HEIGHT*DEF_PIC_WIDTH)!=1)
	{
		album_index++;
		UI_AddRender(&UI[InUI_Album_File]);
		RenderCircle_UI();
		if(album_index>pic_index)
		{
			album_num = 1;
			album_index = 0;
		}
		BMP_NumToString(album_index,path);
	}
	UI_AddRender(&UI[InUI_Album_File]);
	UI_AddRender(&UI[InUI_Album_Index]);
	RenderCircle_UI();
	Func_TFT_Show();
}
void BUT_Album_Prior(void)
{
	--pic_num;
	--album_index;
	char path[6];
	BMP_NumToString(album_index,path);
	while(SD_Fast_Read((const char*)path,(uint16_t*)&camera_data[0],DEF_PIC_HEIGHT*DEF_PIC_WIDTH)!=1)
	{
		album_index--;
		UI_AddRender(&UI[InUI_Album_File]);
		RenderCircle_UI();
		if(album_index<0)
		{
			album_num = pic_num;
			album_index = pic_index;
		}
		BMP_NumToString(album_index,path);
	}
	UI_AddRender(&UI[InUI_Album_File]);
	UI_AddRender(&UI[InUI_Album_Index]);
	RenderCircle_UI();
	Func_TFT_Show();
}
void BUT_Album_Delete(void)
{

}
void BUT_LEDControl(void)
{
	
}


















