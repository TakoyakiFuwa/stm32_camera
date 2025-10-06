#ifndef __BMP_H__
#define __BMP_H__
#include "stdint.h"

void Init_BMP(void);
//文件读取
void BMP_Read_ByData(const char* file_name,uint16_t* data,uint32_t max_length);
void BMP_Read_ByFunc(const char* file_name,void(*Func)(uint16_t));
//文件写入
void BMP_Write_ByData(const char* file_name,uint16_t* data,uint16_t width,uint16_t height);
void Cmd_BMP(void);

#endif
