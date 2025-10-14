#include "UI_Core.h"
/*  调试用的串口库  */
#include "U_USART.h"

/*	之前写的UI库....接口混乱....
 *	说明的注释内容也不多....
 *	干脆打算重新写....
 *	(参考之前库的内容...)
 *	哼哼....现在已经做了些半成熟项目....
 *	代码水平应该有提高一些...
 *		2025/10/13.13:09-秦羽
 */

/*  光标颜色  */
#define UI_CURSOR_FT_COLOR	0xF20D
#define UI_CURSOR_BK_COLOR	0xFFB4
/*  UI用到的全局变量  */
	//实时变化的UI/Page
qy_pointer	CURSOR;
qy_ui		UI[200];
qy_page		PAGE[8];
	//空变量
qy_ui 	NULL_UI;
void 	NULL_FUNC(qy_ui* null){
	U_Printf("未绑定操作函数 \r\n");
}
uint8_t NULL_FUNC_PAGE(void){return 0;}
	//渲染队列
qy_ui* 	UI_RENDER_QUEUE[200];
uint8_t UI_RENDER_QUEUE_INDEX = 0;
/*  状态位，允许UI刷新  */
int8_t STATUS_ON_UI = 1;

#include "UI_Instance.h"
static uint8_t Init_PAGE(void)
{
	uint8_t ui_indexs[] = {InUI_Test_Num,InUI_Test_Button0,InUI_Test_Button1,InUI_Test_Frame};
	UI_CreatePage(&PAGE[InPG_Test],ui_indexs,sizeof(ui_indexs)/sizeof(uint8_t),Test_PageInit);
	uint8_t page_UI_Index_FIX[] = {InUI_Fix_String,InUI_Fix_Num,InUI_Test_Num,InUI_Test_Button0,InUI_Test_Button1,InUI_Test_Frame};
	UI_CreatePage(&PAGE[InPG_Fix],page_UI_Index_FIX,sizeof(page_UI_Index_FIX)/sizeof(uint8_t),PageInit_Fix);
	
	//返回第一个进入的页面
	return InPG_Fix;
}

/**@brief  UI初始化
  *@param  void
  *@retval void
  */
void Init_UI(void)
{
	//空变量初始化
	NULL_UI = UI_CreateUI(0,0,0,0,0,NULL_FUNC);
	NULL_UI.is_present |= 0x80;
	//渲染队列赋初值
	for(int i=0;i<200;i++)
	{
		UI_RENDER_QUEUE[i] = &NULL_UI;
	}
	//相关内容初始化
		//UI初始化
	for(int i=0;i<200;i++)
	{
		UI[i] = NULL_UI;
	}
		//页面初始化
	for(int i=0;i<8;i++)
	{
		for(int j=0;j<128;j++)
		{
			PAGE[i].InUI_s[j] = 250;
		}
		PAGE[i].Func_PageInit = NULL_FUNC_PAGE;
	}
	//创建光标
	CURSOR.InPG = Init_PAGE();
	CURSOR.parameter = 0;
	CURSOR.ptr_color[0] = UI_CURSOR_FT_COLOR;
	CURSOR.ptr_color[1] = UI_CURSOR_BK_COLOR;
	//渲染第一个页面
	UI_ChangePage(CURSOR.InPG);
}
void RenderCircle_UI(void)
{
	//判断状态位
	if(STATUS_ON_UI!=1)
	{//UI关闭，不允许刷新
		return;
	}
	for(int i=0;i<200;i++)
	{
		if((UI_RENDER_QUEUE[i]->is_present&0x80)!=0)
		{
			break;
		}
		UI_RENDER_QUEUE[i]->Func_Render_N(UI_RENDER_QUEUE[i]);
		UI_RENDER_QUEUE[i]->is_present &= 0xFD;//1111 1101
		UI_RENDER_QUEUE[i] = &NULL_UI;
	}
	UI_RENDER_QUEUE_INDEX = 0;
}

	//渲染
void UI_AddRender(qy_ui* ui)
{
	if(ui->is_present&0x01==0 || ui->is_present&0x02==1)
	{
		return;
	}
	UI_RENDER_QUEUE[UI_RENDER_QUEUE_INDEX++] = ui;
	ui->is_present |= 0x02;	//已经渲染的标志位
}
/**@brief  创建UI
  *@param  x,y				位置
  *@param  InFT				字体下标
  *@param  ft_cor/bk_cor	前景色/背景色
  *@param  Func_Render		渲染函数
  *@retval qy_ui			创建好的UI
  */
qy_ui UI_CreateUI(uint16_t x,uint16_t y,uint8_t InFT,uint16_t ft_cor,uint16_t bk_cor,void (*Func_Render)(qy_ui* self))
{
	qy_ui ui;
	//赋值
	ui.x = x;
	ui.y = y;
	ui.InFT = InFT;
	ui.ft_color = ft_cor;
	ui.bk_color = bk_cor;
	ui.Func_Render_N = Func_Render;
	//默认值
	ui.is_present = 0;
	ui.Func_Event_UP = NULL_FUNC;
	ui.Func_Event_DOWN = NULL_FUNC;
	ui.Func_Event_LEFT = NULL_FUNC;
	ui.Func_Event_RIGHT = NULL_FUNC;
	ui.Func_Event_OK = NULL_FUNC;
	//存储内容
	ui.value_num = 0;
	for(int i=0;i<24;i++)
	{
		ui.value_space[i] = 0;
	}
	for(int i=0;i<3;i++)
	{
		ui.parameters[i] = 0;
	}
	return ui;
}
void UI_CreatePage(qy_page* PG,uint8_t* ui_index,uint8_t ui_count,uint8_t (*Func_PageInit)(void))
{
	int i = 0;
	for(;i<ui_count;i++)
	{
		PG->InUI_s[i] = ui_index[i];
	}
	for(;i<128;i++)
	{
		PG->InUI_s[i] = 250;
	}
	PG->Func_PageInit = Func_PageInit;
}
	//切换
void UI_ChangeUI(uint8_t InUI)
{
	//把当前UI颜色改回来
	CURSOR.ui->ft_color = CURSOR.ui_color[0];
	CURSOR.ui->bk_color = CURSOR.ui_color[1];
	UI_AddRender(CURSOR.ui);
	//记录新指向的UI
	CURSOR.ui_color[0] = UI[InUI].ft_color;
	CURSOR.ui_color[1] = UI[InUI].bk_color;
	//更改新指向的UI颜色
	UI[InUI].ft_color = CURSOR.ptr_color[0];
	UI[InUI].bk_color = CURSOR.ptr_color[1];
	//光标指向新UI
	CURSOR.ui = &UI[InUI];
	UI_AddRender(CURSOR.ui);
}
void UI_ChangePage(uint8_t InPG)
{
	uint8_t* _uis = PAGE[CURSOR.InPG].InUI_s;
	uint8_t index = 0;
	//先关闭当前UI的显示
	for(int i=0;i<128;i++)
	{
		index = _uis[i];
		if(index==250)
		{break;}
		UI[index].is_present &= 0xFE;
	}
	//加载新页面
	CURSOR.InPG = InPG;
		//打开新页面的显示
	_uis = PAGE[InPG].InUI_s;
	uint8_t InUI = PAGE[InPG].Func_PageInit();
	for(int i=0;i<128;i++)
	{
		index = _uis[i];
		if(index==250)
		{break;}
		UI[index].is_present = 0x01;
		UI_AddRender(&UI[index]);
	}
		//光标到新的UI上
			//记录新指向的UI
	CURSOR.ui_color[0] = UI[InUI].ft_color;
	CURSOR.ui_color[1] = UI[InUI].bk_color;
			//更改新指向的UI颜色
	UI[InUI].ft_color = CURSOR.ptr_color[0];
	UI[InUI].bk_color = CURSOR.ptr_color[1];
			//光标指向新UI
	CURSOR.ui = &UI[InUI];
	UI_AddRender(CURSOR.ui);
}
/*  辅助函数  */
void Other_StringCpy(uint8_t* target,const char* _string)
{
	int i=0;
	for(;_string[i]!='\0';i++)
	{
		target[i] = _string[i];
	}
	target[i] = '\0';
}
