#include "UI_Render.h"
/*  串口库  */
#include "U_USART.h"
/*  TFT屏幕  */
#include "TFT_ST7789V.h"

/*	呜....
 *	接口写的好烂.....
 *	但涉及屏幕方向的....
 *	嘶......
 *			2025/10/13.20:53-秦羽
 */

/*  字体宏  */
qy_font FONT[128];
/*  见cmr_def.h  */
#include "cmr_def.h"
//#define UIR_ROTATION 0x60	//YXV0 0000
/*	方向为: （适合f4_ui的OV7670方向输出）
 *		+-——-——-——-——>
 *		|			 x	
 *		|	1—————>
 *		|	2—————>
 *		|	3—————>
 *		|
 *		v y 
 */
/*  屏幕接口  */
static inline void UIR_SetRect(uint16_t x,uint16_t y,uint16_t width,uint16_t height)
{
	TFT_SetCursor(x,y,width,height);
}
static inline void UIR_Pixel(uint16_t rgb565)
{
	TFT_Write16Data(rgb565);
}
/*  字库相关  */
extern const char font_consolas_1608[][];
extern const char font_consolas_3216[][];
extern const unsigned char pic_mugou_240_201[];
extern const unsigned char pic_sunrain_16_16[520];
extern const unsigned char pic01_LED_1616[32];
/**@brief  UI_Render初始化
  *@param  void
  *@retval void
  */
static qy_font UIR_CreateFont(const char* font,uint16_t height,uint16_t width);
void Init_UIR(void)
{
	//字体绑定
	FONT[InFT_Consolas_1608] = UIR_CreateFont((const char*)font_consolas_1608,16,8);
	FONT[InFT_Consolas_3216] = UIR_CreateFont((const char*)font_consolas_3216,32,16);
	FONT[InPIC_SunRain_1616] = UIR_CreateFont((const char*)pic_sunrain_16_16,16,16);
	FONT[InPIC01_LED_1616] = UIR_CreateFont((const char*)pic01_LED_1616,16,16);
	U_Printf("UI_Render初始化完成 \r\n");
}

/*  函数实现部分  */

/*  绑定字体  */
static qy_font UIR_CreateFont(const char* font,uint16_t height,uint16_t width)
{
	qy_font f;
	f.font = font;
	f.width = width;
	f.height = height;
	return f;
}
/*  关于形状的部分  */
/**@brief  填充矩形
  *@param  -
  *@param  color 颜色
  *@retval void
  */
void UIR_DrawRect(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint16_t color)
{
	UIR_SetRect(x,y,width,height);
	for(int i=0;i<width*height;i++)
	{
		UIR_Pixel(color);
	}
}
/**@brief  绘制矩形框
  *@param  x y width height
  *@param  color 框颜色
  *@param  thick 框线宽
  *@retval void
  *@add    注：框架线是向内收缩的 x y width height是最大外边框
  */
void UIR_DrawFrame(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint16_t color,int8_t thick)
{
	if(thick<=0)
	{
		return;
	}
	UIR_DrawRect(x,y,width,thick,color);
	UIR_DrawRect(x,y+height-thick,width,thick,color);
	UIR_DrawRect(x,y+thick,thick,height-thick*2,color);
	UIR_DrawRect(x+width-thick,y+thick,thick,height-thick*2,color);
}
/*  取模显示  */
/**@brief  放置单个字符
  *@param  x/y 		位置
  *@param  InFT		字体宏定义
  *@param  _char	要放置的字符
  *@param  ft_color/bk_color	前景色/背景色
  *@retval void
  */
void UIR_PutChar(uint16_t x,uint16_t y,uint8_t InFT,char _char,uint16_t ft_color,uint16_t bk_color)
{
	qy_font f = FONT[InFT];
	UIR_SetRect(x,y,f.width,f.height);
	uint32_t d_ft = _char - ' ';
	uint16_t size = f.width*f.height/8;
	const char* font = &f.font[d_ft*size];
	for(int i=0;i<size;i++)
	{
		for(int j=0;j<8;j++)
		{
			if( (font[i] & (0x01<<j) ) != 0 )
			{
				UIR_Pixel(ft_color);
			}
			else
			{
				UIR_Pixel(bk_color);
			}
		}
	}
}
/**@brief  放置RGB565图片
  *@param  x,y		位置
  *@param  InPIC	要显示图片的下标
  *@retval void
  */
void UIR_PutPic565(uint16_t x,uint16_t y,uint8_t InPIC)
{
	uint32_t size = FONT[InPIC].height*FONT[InPIC].width*2;
	const char* f = FONT[InPIC].font;
	UIR_SetRect(x,y,FONT[InPIC].width,FONT[InPIC].height);
	uint16_t rgb565;
	for(uint32_t i=1;i<size;i++)
	{
		rgb565 = f[i];
		rgb565 <<= 8 ;
		rgb565 |= f[++i];
		UIR_Pixel( rgb565 );
	}
}
/*  字符串显示  */
/**@brief  显示一个数字
  *@param  -
  *@param  digits  显示的位数，超过位数会使高位输出成字符
  *@retval void
  */
void UIR_ShowNum(uint16_t x,uint16_t y,uint32_t num,int8_t digits,uint8_t InFT,uint16_t ft_color,uint16_t bk_color)
{
	qy_font f = FONT[InFT];
	uint32_t num_length = 1;
	for(;digits>0;digits--)
	{	
		num_length*=10;
	}
	//从高位开始显示
	int8_t i=0;
	for(num_length/=10;num_length>=1;num_length/=10)
	{
		UIR_PutChar(x+f.width*(i++),y,InFT,num/num_length+'0',ft_color,bk_color);
		//减去最高位
		num -= (num - (num%num_length));
	}
}
/**@brief  字符串
  *@param  -
  *@param  NumOfChar  显示的数量
  */
void UIR_ShowString(uint16_t x,uint16_t y,const char* text,int8_t NumOfChar,uint8_t InFT,uint16_t ft_color,uint16_t bk_color)
{
	qy_font f=FONT[InFT];
	int i=0;
	for(;text[i]!='\0';i++)
	{
		if(--NumOfChar<0)
		{
			UIR_PutChar(x+f.width*(--i),y,InFT,'-',ft_color,bk_color);
			return;
		}
		UIR_PutChar(x+f.width*(i),y,InFT,text[i],ft_color,bk_color);
	}
	for(;NumOfChar>0;NumOfChar--)
	{
		UIR_PutChar(x+f.width*(i++),y,InFT,' ',ft_color,bk_color);
	}
}


/*  测试区  */
void Test_UIR(void)
{
	UIR_PutChar(10,20,InFT_Consolas_3216,'F',0,0xFFFF);
	UIR_DrawRect(100,20,30,60,0xFF);
	UIR_DrawFrame(10,100,100,50,0xAAF1,12);
	UIR_ShowNum(100,100,12,3,InFT_Consolas_1608,0x002C,0x3c00);
	UIR_ShowString(100,116,"HelloTFT",8,InFT_Consolas_1608,0x7700,0x0067);	
}

