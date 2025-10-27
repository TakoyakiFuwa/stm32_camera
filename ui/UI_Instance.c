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

/*  页面初始化函数格式  */
uint8_t XXXX_PageInit(void);

/*  侧边固定的侧边栏  */
void Render_Null(qy_ui* u)
{
	
}
void Render_FIX_BKGround(qy_ui* u)
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
	UIR_ShowNum(u->x,u->y,24,3,u->InFT,u->ft_color,u->bk_color);
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
/**@brief  侧边栏
  *@retval 空位置 没什么用
  */
uint8_t PageInit_Fix(void)
{
	//  真在画页面
		//光标位置
	UI[InUI_Fix_Cursor] = UI_CreateUI(0,0,InFT_Consolas_1608,0,0,Render_Null);
		//背景
	UI[InUI_Fix_BKGround] = UI_CreateUI(320-16,0,InFT_Consolas_1608,0xD6BA,0xD6BA,Render_FIX_BKGround);
		//晴雨图标
	UI[InUI_Fix_SunRain] = UI_CreateUI(320-16,3,InPIC_SunRain_1616,0,0,Render_Fix_SunRain);
		//晴雨字符
	UI[InUI_Fix_QY] = UI_CreateUI(24,0,InFT_Consolas_1608,0,UI[InUI_Fix_BKGround].bk_color,Render_Fix_QY);
		//LED灯状态
	UI[InUI_Fix_LED] = UI_CreateUI(320-16,148,InPIC01_LED_1616,0,UI[InUI_Fix_BKGround].bk_color,Render_Fix_LED);
		//当前相片数
	UI[InUI_Fix_PicNum] = UI_CreateUI(168,0,InFT_Consolas_1608,0,UI[InUI_Fix_BKGround].bk_color,Render_Fix_PicNum);
		//电池电压状态
	UI[InUI_Fix_Battery] = UI_CreateUI(320-16,198,InFT_Consolas_1608,0,TFT_RGB888To565(0xAAAAAA),Render_Fix_Battery);
	UI[InUI_Fix_Battery].value_num = 70;
	
	return InUI_Fix_Cursor;
}










