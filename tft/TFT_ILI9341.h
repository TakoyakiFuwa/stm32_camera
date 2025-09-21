#ifndef __TFT_ILI9341_H__
#define __TFT_ILI9341_H__
#include "stdint.h"

#define COLOR_BLUE     0x07FF
#define COLOR_RED      0xEC10
#define COLOR_YELLOW   0xEF31
#define COLOR_GREEN    0x7FC0

void Init_ILI(void);
void ILI_Swap(uint8_t byte);
void ILI_SendColor(uint16_t rgb565);
void ILI_SetRect(uint16_t X1, uint16_t Y1, uint16_t width, uint16_t height);
uint16_t ILI_RGB888To565(uint32_t RGB888);
void ILI_Test(void);
void ILI_OneByte(uint8_t byte);

#endif
