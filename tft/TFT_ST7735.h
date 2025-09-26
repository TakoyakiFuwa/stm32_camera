#ifndef __TFT_ST7735_H__
#define __TFT_ST7735_H__
#include <stdint.h>

//工具函数
uint16_t TFT_RGB888To565(uint32_t RGB_888);
//通信
void TFT_SPI_Start(void);
void TFT_SPI_Stop(void);
void TFT_SPI_Send(uint8_t byte);
void TFT_SPI_DMA(uint16_t counts);
//主要的接口
void Init_TFT(uint8_t* data_addr);
void TFT_SetCursor(uint8_t x,uint8_t y,uint8_t weight,uint8_t height);
void TFT_Write16Data(uint16_t RGB_565);
void TFT_SetRotation(uint8_t rotation);

#endif
