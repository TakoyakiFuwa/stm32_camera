#ifndef __UI_INSTANCE_H__
#define __UI_INSTANCE_H__
#include "stdint.h"

	/*  页面  */
#define InPG_Fix			0
	/*  UI  */
#define InUI_Fix_BKGround	0
#define InUI_Fix_Cursor		1
#define InUI_Fix_SunRain	2
#define InUI_Fix_QY			3
#define InUI_Fix_LED		4
#define InUI_Fix_PicNum		5
#define InUI_Fix_Battery	6


/*  侧边固定的侧边栏  */
uint8_t PageInit_Fix(void);

#endif


