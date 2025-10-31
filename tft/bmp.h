#ifndef __BMP_H__
#define __BMP_H__
#include "stdint.h"

//SD卡初始化
int8_t Init_BMP(void);
//辅助函数
void BMP_NumToString(uint16_t num,char* str);
uint16_t BMP_StringToNum(const char* str);
void BMP_Path(const char* front,uint8_t* path,const char* back);
//文件读取	(会自动把rgb565转rgb888)
void BMP_Read_ByData(const char* file_name,uint16_t* rgb565,uint16_t* width,uint16_t* height,uint32_t max_length);
//文件写入	(会自动把rgb888转rgb565)
void BMP_Write_ByData(const char* file_name,uint16_t* rgb565,uint16_t width,uint16_t height);
//快速读写
void SD_Fast_Write(const char* file_name,uint16_t* data,uint32_t length);
int8_t SD_Fast_Read(const char* file_name,uint16_t* data,uint32_t length);
//读写RGB565
void BMP_WriteRGB565_Data(uint16_t file_name,void* data,uint16_t width,uint16_t height);
void BMP_ReadRGB565_Data(uint16_t file_name,uint16_t* data,uint16_t width,uint16_t height);
//测试
void Cmd_BMP(void);

/*  在做camera项目的性能提高  */
#include "ff.h"
int8_t BMP_Write_ForeProcess(const char* file_name,FIL* fp,uint16_t width,uint16_t height);

#endif
