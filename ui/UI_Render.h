#ifndef __UI_RENDER_H__
#define __UI_RENDER_H__
#include "stdint.h"

#define InFT_Consolas_1608		0
#define InFT_Consolas_3216		1
#define InPIC_MuGo				2
#define InPIC_SunRain_1616		3
#define InPIC01_LED_1616		4

/*  字体结构体  */
typedef struct qy_font{
	const char* font;
	uint16_t width;
	uint16_t height;
}qy_font;

/*  初始化(字体绑定)  */
void Init_UIR(void);
/*  图案显示  */
void UIR_DrawRect(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint16_t color);
void UIR_DrawFrame(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint16_t color,int8_t thick);
/*  取模显示  */
void UIR_PutChar(uint16_t x,uint16_t y,uint8_t InFT,char _char,uint16_t ft_color,uint16_t bk_color);
void UIR_PutPic565(uint16_t x,uint16_t y,uint8_t InPIC);
/*  字符串显示  */
void UIR_ShowNum(uint16_t x,uint16_t y,uint32_t num,int8_t digits,uint8_t InFT,uint16_t ft_color,uint16_t bk_color);
void UIR_ShowString(uint16_t x,uint16_t y,const char* text,int8_t NumOfChar,uint8_t InFT,uint16_t ft_color,uint16_t bk_color);


#endif
