#ifndef __UI_INSTANCE_H__
#define __UI_INSTANCE_H__
#include "stdint.h"

	/*  页面  */
#define InPG_Fix			0
#define InPG_Start			1
	/*  UI  */
		//固定的侧边栏内容（前10个）
#define InUI_Fix_Base		0
#define InUI_Fix_Cursor		1
#define InUI_Fix_SunRain	2
#define InUI_Fix_QY			3
#define InUI_Fix_LED		4
#define InUI_Fix_PicNum		5
#define InUI_Fix_Battery	6
#define InUI_Fix_BKGround	7		
#define InUI_Fix_BKQY		8
		//开始时的等待页面
#define InUI_Start_SDstring		13		//Waiting for SD:
#define InUI_Start_SDstatus		14		//OK / Error  加载成功or失败
#define InUI_Start_SDpic		15		//加载成功后随机显示一个图片
#define InUI_Start_SDprocess	16		//Waiting for SD Process:	Finish


/*  侧边固定的侧边栏  */
uint8_t PageInit_Fix(void);
/*  开始等待界面  */
uint8_t PageInit_Start(void);

#endif


