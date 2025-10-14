#include "UI_Instance.h"
/*  UI核心库  */
#include "UI_Core.h"
/*  串口调试库  */
#include "U_USART.h"
/*  字体库  */
#include "UI_Render.h"

extern qy_pointer	CURSOR;
extern qy_ui		UI[200];
extern qy_page		PAGE[8];

/*  页面初始化函数格式  */
uint8_t XXXX_PageInit(void);

//测试页面

static void Render_Num(qy_ui* u)
{
	UIR_ShowNum(u->x,u->y,u->value_num,3,u->InFT,u->ft_color,u->bk_color);
}
static void Render_Box(qy_ui* u)
{
	UIR_DrawRect(u->x,u->y,40,40,u->ft_color);
}
static void Render_Frame(qy_ui* u)
{
	UIR_DrawFrame(u->x,u->y,40,40,u->ft_color,2);
}
static void LEFT_Num(qy_ui* u)
{
	if(u->value_num>0)
	{
		u->value_num--;
	}
	UI_AddRender(u);
}
static void RIGHT_Num(qy_ui* u)
{
	u->value_num++;
	UI_AddRender(u);
}
static void LEFT_Button1(qy_ui* u)
{
	UI_ChangeUI(InUI_Test_Button0);
}
static void RIGHT_Button1(qy_ui* u)
{
	UI_ChangeUI(InUI_Test_Num);
}
static void LEFT_Button0(qy_ui* u)
{
	UI_ChangeUI(InUI_Test_Num);
}
static void RIGHT_Button0(qy_ui* u)
{
	UI_ChangeUI(InUI_Test_Button1);
}



uint8_t Test_PageInit(void)
{
	U_Printf("测试页面初始化完成 \r\n");
	return InUI_Test_Button1;
}

#include "TFT_ST7789V.h"
void Render_FixNum(qy_ui* u)
{
	TFT_SetRotation(0xC0);
	UIR_ShowNum(u->x,u->y,u->value_num,3,u->InFT,u->ft_color,u->bk_color);
	TFT_SetRotation(0x60);
}
void Render_FixString(qy_ui* u)
{
	TFT_SetRotation(0xC0);
	UIR_ShowString(u->x,u->y,"By QinYuYQY.qq2060653830",24,u->InFT,u->ft_color,u->bk_color);
	TFT_SetRotation(0x60);
}

uint8_t PageInit_Fix(void)
{
	/*  测试  */
	//数字测试
	UI[InUI_Test_Num] = UI_CreateUI(0,0,InFT_Consolas_3216,0x2300,0x23FF,Render_Num);
	UI[InUI_Test_Num].value_num = 20;
	UI[InUI_Test_Num].Func_Event_LEFT = LEFT_Num;
	UI[InUI_Test_Num].Func_Event_RIGHT = RIGHT_Num;
	//按键0
	UI[InUI_Test_Button0] = UI_CreateUI(80,10,InFT_Consolas_1608,0xFFFF,0x00,Render_Box);
	UI[InUI_Test_Button0].Func_Event_LEFT = LEFT_Button0;
	UI[InUI_Test_Button0].Func_Event_RIGHT = RIGHT_Button0;
	//按键1
	UI[InUI_Test_Button1] = UI_CreateUI(80,60,InFT_Consolas_1608,0x0,0xFFFF,Render_Box);
	UI[InUI_Test_Button1].Func_Event_LEFT = LEFT_Button1;
	UI[InUI_Test_Button1].Func_Event_RIGHT = RIGHT_Button1;
	//框架测试
	UI[InUI_Test_Frame] = UI_CreateUI(80,110,InFT_Consolas_1608,0xFF00,0x00FF,Render_Frame);
	
	/*  真在画页面  */
		//照片数
	UI[InUI_Fix_Num] = UI_CreateUI(240-24,0,InFT_Consolas_1608,0x0,0xFFFF,Render_FixNum);
	UI[InUI_Fix_Num].value_num = 12;
		//秦羽
	UI[InUI_Fix_String] = UI_CreateUI(0,0,InFT_Consolas_1608,0xF200,0x0F20,Render_FixString);
//	Other_StringCpy(UI[InUI_Fix_String],"Power By QinYuYQY.qq2060653830");
	
	return InUI_Test_Button0;
}










