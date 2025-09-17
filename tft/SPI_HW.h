#ifndef __SPI_HW_H__
#define __SPI_HW_H__
#include "stdint.h"

void SPI_HW_Init(void);
void SPI_HW_Send(uint8_t x);
void SPI_HW_CS_H(void);
void SPI_HW_CS_L(void);
void Cmd_SPI(void);
void SPI_DMA_Reset(void);

#endif
