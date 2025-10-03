#ifndef __BMP_H__
#define __BMP_H__
#include "stdint.h"

typedef struct{
	uint16_t bmp_sign;		//00 文件标识
	uint32_t file_size;		//02 用字节表示文件大小
	uint32_t reserved;		//06 保留 应该是0
	uint32_t data_offset;	//0A 数据偏移位置
	uint32_t header_size;	//0E 信息头长度
	uint32_t width;			//12 位图宽度，单位像素
	uint32_t height;		//16 位图高度，单位像素
	uint16_t planes;		//1A 位面数 总是1
	uint16_t bits_per_px;	//1C 每个像素的位数
	uint32_t compression;	//1E 压缩说明 0不压缩
	uint32_t data_size;		//22 用字节表示位图数据大小（4的倍数）
	uint32_t HResolution;	//26 像素/米表示水平分辨率
	uint32_t VResolution;	//2A 像素/米表示垂直分辨率
	uint32_t colors;		//2E 位图用的颜色数
	uint32_t important_cor; //32 重要颜色数
}bmp_head;
typedef struct{
	uint8_t blue;
	uint8_t green;
	uint8_t red;
//	uint8_t x;		//保留，应当是0
}bmp_rgb;


//void Init_BMP(void);
bmp_head BMP_ReadInfor(const char* path);
void BMP_BMP(const char* path,int8_t d_width,int8_t px_fix,void (*Render)(uint16_t rgb565),void (*SetRect)(uint16_t width,uint16_t height));
void BMP_AdjustBMP(const char* path,int8_t d_width,int8_t px_fix,uint16_t w_max,uint16_t h_max,void (*Render)(uint16_t rgb565),void (*SetRect)(uint16_t width,uint16_t height));
void Test_BMP(void);

void Init_BMP(void);
void Cmd_BMP(void);

#endif
