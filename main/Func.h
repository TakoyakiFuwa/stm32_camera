#ifndef __FUNC_H__
#define __FUNC_H__
/*  ST库  */
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"

void Init_SD(void);
void Init_Func(void);
void Task_Func(void* pvParameters);
void Func_Pic_Index_Init(void);
void Func_Pic_To_BMP(void);
void Init_Light(void);
void Cmd_Func(void);
void Func_TFT_Show(void);
void Func_Pic_ToBMP_OnePhoto(void);
void Task_Camera(void* pvParameters);
void Task_UI(void* pvParameters);
void Task_GetADC(void* pvParameters);

#endif
