#ifndef __OV7670_H__
#define __OV7670_H__
#include "stdint.h"

void Init_OV(void);
void OV_PixelsGet(void);
void Task_Camera(void* pvParameters);

#endif
