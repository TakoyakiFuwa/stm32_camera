#ifndef __BUTTONFUNC_H__
#define __BUTTONFUNC_H__
#include "stdint.h"

void Init_BUT(void);
uint16_t BUT_FindMaxNum(const char* path,uint16_t* total_num);
void BUT_KeepPhoto(void);
void BUT_AlbumControl(void);
void BUT_Album_Next(void);
void BUT_Album_Prior(void);
void BUT_Album_Delete(void);
void BUT_LEDControl(void);

#endif
