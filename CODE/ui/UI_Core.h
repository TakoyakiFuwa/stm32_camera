#ifndef __UI_CORE_H__
#define __UI_CORE_H__
#include "stdint.h"

/*  UI结构体  */
typedef struct qy_ui{
	//位置
	uint16_t 	x;
	uint16_t 	y;
	//字体号/前景色/背景色
	uint8_t 	InFT;
	uint16_t 	ft_color;
	uint16_t 	bk_color;
	//状态位置
	uint8_t 	is_present;		//渲染尾-0-0-0	0-0-已在队列-可以渲染
	
	//存储内容
	uint16_t 	value_num;
	uint8_t 	value_space[24];
	
	//其他临时参数
	uint16_t	parameters[3];
	
	//绑定的函数方法
	void (*Func_Render_N)(struct qy_ui* self);
	void (*Func_Event_UP)(struct qy_ui* self);
	void (*Func_Event_DOWN)(struct qy_ui* self);
	void (*Func_Event_LEFT)(struct qy_ui* self);
	void (*Func_Event_RIGHT)(struct qy_ui* self);
	void (*Func_Event_OK)(struct qy_ui* self);
}qy_ui;
/*  页面结构体  */
typedef struct qy_page{
	uint8_t InUI_s[128];	//当前页面的UI组，=250时指空
	uint8_t (*Func_PageInit)(void);
}qy_page;
/*  光标结构体  */
typedef struct qy_pointer{
	uint8_t 	InPG;			//当前所在的页面
	qy_ui* 		ui;
	uint16_t 	ui_color[2];
	uint16_t	ptr_color[2];
	uint16_t 	parameter;
}qy_pointer;

/*  架构函数  */
void Init_UI(void);
void RenderCircle_UI(void);
void Cmd_UI(void);
/*  功能函数  */
	//渲染
void UI_AddRender(qy_ui* ui);
	//创建
qy_ui UI_CreateUI(uint16_t x,uint16_t y,uint8_t InFT,uint16_t ft_cor,uint16_t bk_cor,void (*Func_Render)(qy_ui* self));
void UI_CreatePage(qy_page* PG,uint8_t* ui_index,uint8_t ui_count,uint8_t (*Func_PageInit)(void));
	//切换
void	UI_ChangeUI(uint8_t InUI);
void 	UI_ChangePage(uint8_t InPG);
/*  辅助函数  */
void Other_StringCpy(uint8_t* target,const char* _string);

#endif





