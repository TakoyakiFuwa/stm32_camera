#ifndef __UI_INSTANCE_H__
#define __UI_INSTANCE_H__
#include "stdint.h"

	//页面
#define InPG_Test			0
#define InPG_Fix			1
	//UI
#define InUI_Fix_Num		3
#define InUI_Fix_String		4
	//UI
#define InUI_Test_Num		10
#define InUI_Test_Button0	11
#define InUI_Test_Button1	12
#define InUI_Test_Frame		13


uint8_t Test_PageInit(void);
uint8_t PageInit_Fix(void);

#endif


