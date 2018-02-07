#ifndef _TIMER_H_
#define _TIMER_H_

#include "datatype.h"

UINT8 TIM_Open(INT32 nms, UINT16 cnt, UINT8 down);
void TIM_Close(UINT8 t);
UINT8 TIM_CheckTimeout(UINT8 t);
void TIM_ISR(void);
INT32 TIM_GetCount(UINT8 t);
UINT8 TIM_Delay1MS(UINT16 n);
void TIM_SetSoftTimeout(UINT8 t);
void TIM_SetSoftInterrupt(UINT8 enable, void (*p)(void));
void TIM_SoftInterrupt(void);

#endif

