#ifndef __OV7670_H__
#define __OV7670_H__
#include "stdint.h"

void Init_OV(void);
void OV_GetPixels(void (*Func)(uint8_t));

#endif
