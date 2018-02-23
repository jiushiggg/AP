#ifndef _TIMER_H_
#define _TIMER_H_

#include "datatype.h"


#define     TIMER_UP_CNT            0
#define     TIMER_DOWN_CNT          1

#define     ALL_TIMER_ACTIVE        0xff

UINT8 TIM_Open(INT32 nms, UINT16 cnt, UINT8 direction);
void TIM_Close(UINT8 t);
UINT8 TIM_CheckTimeout(UINT8 t);
INT32 TIM_GetCount(UINT8 t);
void TIM_SetSoftTimeout(UINT8 t);
void TIM_SetSoftInterrupt(UINT8 enable, void (*p)(void));
void TIM_SoftInterrupt(void);

#endif

