#ifndef __U_USART_H__
#define __U_USART_H__
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"

void Init_USART(void);
void U_Test(void);
int8_t Command(const char* cmd);
//void U_Putchar(uint8_t CHAR);
//void U_WriteWords(const uint8_t* words);
//void U_WriteNum(uint32_t num);
//void U_WriteHex(uint32_t hex);
//void U_WriteBinary(uint32_t binary);
void U_Printf(const char* words,...);

#endif
