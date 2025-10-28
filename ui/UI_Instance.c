#include "UI_Instance.h"
/*  UI核心库  */
#include "UI_Core.h"
/*  串口调试库  */
#include "U_USART.h"
/*  字体库  */
#include "UI_Render.h"
/*  全局宏定义  */
#include "cmr_def.h"
/*  屏幕驱动库  */
#include "TFT_ST7789V.h"
extern qy_pointer	CURSOR;
extern qy_ui		UI[200];
extern qy_page		PAGE[8];

extern uint16_t pic_num;

/*  页面初始化函数格式  */
uint8_t PageInit_XXXX(void);

/*  侧边固定的侧边栏  */
void Render_Null(qy_ui* u)
{
	
}
void Render_Fix_Base(qy_ui* u)
{
	UIR_DrawRect(u->x,u->y,16,240,u->ft_color);
}
void Render_Fix_SunRain(qy_ui* u)
{
	UIR_PutPic565(u->x,u->y,u->InFT);
}
void Render_Fix_QY(qy_ui* u)
{
	TFT_SetRotation(DEF_TFT_ROTAUI);
	UIR_ShowString(u->x,u->y,"QY@qq2060653830",15,u->InFT,u->ft_color,u->bk_color);
	TFT_SetRotation(DEF_TFT_ROTA);
}
void Render_Fix_LED(qy_ui* u)
{
	UIR_PutChar(u->x,u->y,u->InFT,' ',u->ft_color,u->bk_color);
}
void Render_Fix_PicNum(qy_ui* u)
{
	TFT_SetRotation(DEF_TFT_ROTAUI);
	UIR_ShowNum(u->x,u->y,pic_num,3,u->InFT,u->ft_color,u->bk_color);
	TFT_SetRotation(DEF_TFT_ROTA);
}
void Render_Fix_Battery(qy_ui* u)
{
	uint8_t value = u->value_num/25;
	for(int i=0;i<4;i++)
	{
		UIR_DrawRect(u->x+4,u->y+10*i,8,6,UI[InUI_Fix_BKGround].bk_color);
	}
	switch(value)
	{
	case 3:
		UIR_DrawRect(u->x+4,u->y+10*0,8,6,0x2351);
	case 2:
		UIR_DrawRect(u->x+4,u->y+10*1,8,6,0x2351);
	case 1:
		UIR_DrawRect(u->x+4,u->y+10*2,8,6,0x2351);
	case 0:
		UIR_DrawRect(u->x+4,u->y+10*3,8,6,0x2351);
	}
	for(int i=0;i<4;i++)
	{
		UIR_DrawFrame(u->x+3,u->y+10*i,10,8,u->bk_color,2);
	}
}
void Render_Fix_BKGround(qy_ui* u)
{
	uint16_t rgb565 = TFT_RGB888To565(0xffc7c7);//0xFE38
	UIR_DrawRect(0,0,304,80,rgb565);
	rgb565 = TFT_RGB888To565(0xf6f6f6);//0xF7BE
	UIR_DrawRect(0,80,304,80,rgb565);
	rgb565 = TFT_RGB888To565(0x71c9ce);//0x7659
	UIR_DrawRect(0,160,304,80,rgb565);
}
void Render_Fix_BKQY(qy_ui* u)
{
	UIR_ShowString(u->x,u->y,"By_",3,InFT_Consolas_3216,0,0x7659);
	for(int i=0;i<5;i++)
	{
		UIR_PutChar(u->x+16*3+32*i,u->y,InFT_QYyqy_3232,' '+i,0,0x7659);
	}
	UIR_PutPic565(u->x+208,u->y,u->InFT);
	UIR_DrawRect(u->x,u->y+34,238,3,0);
	UIR_ShowString(u->x,u->y+38,"qq@2060653830",15,InFT_Consolas_1608,0,0x7659);
}
/**@brief  侧边栏
  *@retval 空位置 没什么用
  */
uint8_t PageInit_Fix(void)
{
	//  真在画页面
		//光标位置
	UI[InUI_Fix_Cursor] = UI_CreateUI(0,0,InFT_Consolas_1608,0,0,Render_Null);
		//背景
	UI[InUI_Fix_Base] = UI_CreateUI(320-16,0,InFT_Consolas_1608,0xD6BA,0xD6BA,Render_Fix_Base);
		//晴雨图标
	UI[InUI_Fix_SunRain] = UI_CreateUI(320-16,3,InPIC_SunRain_1616,0,0,Render_Fix_SunRain);
		//晴雨字符
	UI[InUI_Fix_QY] = UI_CreateUI(24,0,InFT_Consolas_1608,0,UI[InUI_Fix_Base].bk_color,Render_Fix_QY);
		//LED灯状态
	UI[InUI_Fix_LED] = UI_CreateUI(320-16,148,InPIC01_LED_1616,0,UI[InUI_Fix_Base].bk_color,Render_Fix_LED);
		//当前相片数
	UI[InUI_Fix_PicNum] = UI_CreateUI(168,0,InFT_Consolas_1608,0,UI[InUI_Fix_Base].bk_color,Render_Fix_PicNum);
		//电池电压状态
	UI[InUI_Fix_Battery] = UI_CreateUI(320-16,198,InFT_Consolas_1608,0,TFT_RGB888To565(0xAAAAAA),Render_Fix_Battery);
	UI[InUI_Fix_Battery].value_num = 49;
	
	UI[InUI_Fix_BKGround] = UI_CreateUI(0,0,InFT_Consolas_1608,0,0,Render_Fix_BKGround);
	UI[InUI_Fix_BKQY] = UI_CreateUI(10,180,InPIC_SunRain_3232,0,0,Render_Fix_BKQY);
	
	return InUI_Fix_Cursor;
}



/*  开机加载页面  */
void Render_Start_SDstring(qy_ui* u)
{
	UIR_ShowString(u->x,u->y,"Waiting for SD:",20,u->InFT,u->ft_color,u->bk_color);
//	UIR_DrawFrame(180,10,100,160,0,2);
	UIR_DrawFrame(192,17,8+64,6+16,0,1);
	UIR_ShowString(196,20,"BMP TEST",8,InFT_Consolas_1608,0,u->bk_color);
}
void Render_Start_SDstatus(qy_ui* u)
{
	if(u->value_num!=0)
	{
		UIR_ShowString(u->x,u->y,"ERROR",5,u->InFT,0xF000,u->bk_color);
		UIR_ShowNum(u->x+85,u->y,u->value_num,2,u->InFT,0xF0F0,u->bk_color);
		UIR_ShowString(UI[InUI_Start_SDprocess].x,UI[InUI_Start_SDprocess].y,"will start without SD func.",30,InFT_Consolas_1608,0,u->bk_color);
	}
	else
	{
		UIR_ShowString(u->x,u->y,"SD_OK!",6,u->InFT,0x0780,u->bk_color);	
	}
}
void Render_Start_SDprocess(qy_ui* u)
{
	UIR_ShowString(u->x,u->y,"Waiting bmp change:",20,u->InFT,0,u->bk_color);
	if(u->value_num==1)
	{
		UIR_ShowString(UI[InUI_Start_SDstatus].x,u->y+20,"FINISH",6,InFT_Consolas_3216,0x0780,0xF7BE);
	}
}
#include "bmp.h"
extern uint8_t camera_data[];
void Render_Start_SDpic(qy_ui* u)
{
	UIR_PutPic565(u->x,u->y,InPIC_MuGo);
	//应该考虑一下缩放技术了....
//	uint16_t width,height;
//	char aaa[5];
//	for(int i=0;i<6;i++)
//	{
//		BMP_NumToString(i,aaa);
//		BMP_Read_ByData((const char*)aaa,(uint16_t*)&camera_data[0],&width,&height,110*180);
//		TFT_SetCursor(u->x,u->y,width+1,height);
//		TFT_SPI_Start();
//		for(int i=1;i<width*height*2;i++)
//		{
//			TFT_SPI_Send(camera_data[i++]);
//			TFT_SPI_Send(camera_data[i]);
//		}
//		TFT_SPI_Stop();
//		vTaskDelay(1000);
//	}
}
/**@brief  开机加载界面
  */
uint8_t PageInit_Start(void)
{
	PageInit_Fix();
		//
	UI[InUI_Start_SDstring] = UI_CreateUI(10,10,InFT_Consolas_1608,0,0xFE38,Render_Start_SDstring);
	UI[InUI_Start_SDstatus] = UI_CreateUI(60,26,InFT_Consolas_3216,0,0xFE38,Render_Start_SDstatus);
	UI[InUI_Start_SDstatus].value_num = 1;
	UI[InUI_Start_SDprocess] = UI_CreateUI(10,60,InFT_Consolas_1608,0,0xFE38,Render_Start_SDprocess);
	UI[InUI_Start_SDprocess].value_num = 1;
	UI[InUI_Start_SDpic] = UI_CreateUI(180,10,InPIC_MuGo,0,0,Render_Start_SDpic);
	return InUI_Fix_Cursor;
}



/*  相册界面  */
extern int16_t album_index;
extern int16_t album_num;
extern const char BMP_PATH_fast[];
void Render_Album_File(qy_ui* u)
{
	char path[50];
	BMP_NumToString(album_index,path);
	BMP_Path(BMP_PATH_fast,(uint8_t*)path,".qy");
	TFT_SetRotation(DEF_TFT_ROTAUI);
	UIR_ShowString(u->x,u->y,(const char*)path,11,u->InFT,u->ft_color,u->bk_color);
	TFT_SetRotation(DEF_TFT_ROTA);
}
void Render_Album_Index(qy_ui* u)
{
	TFT_SetRotation(DEF_TFT_ROTAUI);
	UIR_ShowNum(u->x,u->y,album_num,3,u->InFT,u->ft_color,u->bk_color);
	TFT_SetRotation(DEF_TFT_ROTA);
}
/**@brief  相册界面
  */
uint8_t PageInit_Album(void)
{
	PageInit_Fix();
		//
	UI[InUI_Album_File] = UI_CreateUI(24,0,InFT_Consolas_1608,TFT_RGB888To565(0x909090),UI[InUI_Fix_Base].bk_color,Render_Album_File);
	Other_StringCpy(UI[InUI_Album_File].value_space,"file_path");
	UI[InUI_Album_Index] = UI_CreateUI(24+92,0,InFT_Consolas_1608,0,UI[InUI_Fix_Base].bk_color,Render_Album_Index);
	return InUI_Fix_Cursor;
}




