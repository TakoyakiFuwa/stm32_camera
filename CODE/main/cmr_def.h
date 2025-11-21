#ifndef __CMR_DEF_H__
#define __CMR_DEF_H__
#include "stdint.h"

/*  有关全局变量 在Shadow.c中  */

/*  320*240  */
/*  相机配置  */
#define DEF_OV_X		162		//建议152,0 参考ov7670.c:Init_OV->OV_config_window(DEF_OV_X,DEF_OV_Y,-,-);
#define DEF_OV_Y		10
/*  宽度/高度  */
#define DEF_PIC_WIDTH	304
#define DEF_PIC_HEIGHT	240	 //162	//建议是3的倍数，参考Task_Camera->屏幕显示部分
/*  屏幕旋转配置  */
#define DEF_TFT_ROTA 	0x70	//YXV0 0000	//配置0xC0时 数字在侧面显示
#define DEF_TFT_ROTAUI	0xC0
/*  屏幕位置  */
#define DEF_TFT_DX		0		//参考TFT_SetCursor(DET_TFT_DX,DEF_TFT_DY,DEF_PIC_WIDTH,DEF_PIC_HEIGHT);
#define DEF_TFT_DY		0
//？


#endif
