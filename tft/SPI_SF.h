#ifndef __SPI_SF_H__
#define __SPI_SF_H__
#include <stdint.h>

#define SPI_SF_CS_L()		GPIOB->BSRRH = GPIO_Pin_12
#define SPI_SF_CS_H()		GPIOB->BSRRL = GPIO_Pin_12

void SPI_SF_Init(void);
void SPI_SF_Test(void);
void SPI_SF_Send(uint8_t data);
uint8_t SPI_SF_Receive(void);

#endif
