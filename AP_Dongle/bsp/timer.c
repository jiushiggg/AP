#include "timer.h"
#include "debug.h"
#include <ti/drivers/timer/GPTimerCC26XX.h>
#include <ti/sysbios/BIOS.h>
#include "CC2640R2_LAUNCHXL.h"
#include "bsp.h"
#include "event.h"

typedef struct
{
    GPTimerCC26XX_Handle TIMn;
    GPTimerCC26XX_HwiFxn fnx;
    volatile INT32 count;
    volatile INT32 direction;
    volatile UINT8 timeout;
    CC2640R2_LAUNCHXL_GPTimerName sn;
}timer_t;

void hwi_timerCallback0(GPTimerCC26XX_Handle handle, GPTimerCC26XX_IntMask interruptMask);
void hwi_timerCallback1(GPTimerCC26XX_Handle handle, GPTimerCC26XX_IntMask interruptMask);
void hwi_timerCallback2(GPTimerCC26XX_Handle handle, GPTimerCC26XX_IntMask interruptMask);
static void ISR_Handle(volatile timer_t* n);

//static
volatile timer_t ts[] = {
	{NULL, hwi_timerCallback0, 0, 0, 0, CC2640R2_LAUNCHXL_GPTIMER0A},
	{NULL, hwi_timerCallback1, 0, 0, 0, CC2640R2_LAUNCHXL_GPTIMER1A},
	{NULL, hwi_timerCallback2, 0, 0, 0, CC2640R2_LAUNCHXL_GPTIMER2A},
};

UINT8   getTimerCount(void)
{
    return sizeof(ts)/sizeof(timer_t);
}

static UINT8 get_a_free_timer(void)
{
	INT32 i;
	UINT8 t = ALL_TIMER_ACTIVE;
	UINT8 timer_num = sizeof(ts)/sizeof(timer_t);

	for(i = 0; i < timer_num; i++)
	{
		if(ts[i].TIMn == NULL)
		{
			t = ts[i].sn/2;
			break;
		}
	}

	return t;
}

static GPTimerCC26XX_Handle get_real_timer(UINT8 t)
{
	if(t <= (sizeof(ts)/sizeof(timer_t)))
	{

		return ts[t].TIMn;
	}
	else
	{
		return NULL;
	}
}
#if 0
//设置定时nms*cnt
UINT8 TIM_Open(INT32 nms, UINT16 cnt, UINT8 direction)
{
	UINT8 t = ALL_TIMER_ACTIVE;
	xdc_runtime_Types_FreqHz  freq;
	GPTimerCC26XX_Params timer;
	GPTimerCC26XX_Value loadVal;


//	if(nms > 1000)
//	{
//		goto done;
//	}


	if((t=get_a_free_timer()) == ALL_TIMER_ACTIVE)
	{
		goto done;
	}

	if(direction == TIMER_UP_CNT)
	{
		ts[t].count = 0;
		ts[t].direction = cnt;
	}
	else
	{
		ts[t].count = cnt;
		ts[t].direction = 0;
	}
	ts[t].timeout = 0;

	pinfoEsl("Ts%d,%d,%d,%d\r\n", t, nms, ts[t].count,ts[t].direction);
    GPTimerCC26XX_Params_init(&timer);
    timer.width          = GPT_CONFIG_32BIT;
    timer.mode           = GPT_MODE_PERIODIC_UP;
    timer.debugStallMode = GPTimerCC26XX_DEBUG_STALL_OFF;
    ts[t].TIMn = GPTimerCC26XX_open(ts[t].sn, &timer);
    if(ts[t].TIMn == NULL) {
        while(1);
    }
	BIOS_getCpuFreq(&freq);
	loadVal = (freq.lo / 1000)*nms - 1; //47999定时1ms
	GPTimerCC26XX_setLoadValue(ts[t].TIMn, loadVal);
	GPTimerCC26XX_registerInterrupt(ts[t].TIMn, ts[t].fnx, GPT_INT_TIMEOUT);
	GPTimerCC26XX_start(ts[t].TIMn);

done:
	return t;
}

void TIM_SetSoftTimeout(UINT8 t)
{
	if(get_real_timer(t) != NULL)
	{
		ts[t].timeout = 1;
	}
}

void TIM_Close(UINT8 t)
{
    pinfoEsl("Tc%d\r\n", t);
	if(get_real_timer(t) != NULL)
	{
	    GPTimerCC26XX_unregisterInterrupt(ts[t].TIMn);
		GPTimerCC26XX_close(ts[t].TIMn);
		ts[t].TIMn = NULL;
		ts[t].count = 0;
		ts[t].direction = 0;
		ts[t].timeout = 0;
	}
}
#else
UINT8 TIM_Init(void)
{
    GPTimerCC26XX_Params timer;
    uint8_t t;
    if((t=get_a_free_timer()) == ALL_TIMER_ACTIVE)
    {
        while(1);
    }

    GPTimerCC26XX_Params_init(&timer);
    timer.width          = GPT_CONFIG_32BIT;
    timer.mode           = GPT_MODE_PERIODIC_UP;
    timer.debugStallMode = GPTimerCC26XX_DEBUG_STALL_OFF;
    ts[t].TIMn = GPTimerCC26XX_open(CC2640R2_LAUNCHXL_GPTIMER0A, &timer);
    GPTimerCC26XX_registerInterrupt(ts[t].TIMn, ts[t].fnx, GPT_INT_TIMEOUT);
    if(ts[t].TIMn == NULL) {
        while(1);
    }
}
uint8_t TIM_Open(int32_t nms, uint16_t cnt, uint8_t direction)
{
    uint8_t t = 0;
    xdc_runtime_Types_FreqHz  freq;
    GPTimerCC26XX_Value loadVal;
    uint32_t key;
//    GPTimerCC26XX_disableInterrupt(ts[t].TIMn, GPT_INT_TIMEOUT);
    key = swiDisable();

    if(direction == TIMER_UP_CNT)
    {
        ts[t].count = 0;
        ts[t].direction = cnt;
    }
    else
    {
        ts[t].count = cnt;
        ts[t].direction = 0;
    }
    ts[t].timeout = 0;

    BIOS_getCpuFreq(&freq);
    loadVal = (freq.lo / 1000)*nms - 1; //47999定时1ms
    GPTimerCC26XX_setLoadValue(ts[t].TIMn, loadVal);
    GPTimerCC26XX_enableInterrupt(ts[t].TIMn, GPT_INT_TIMEOUT);
    GPTimerCC26XX_start(ts[t].TIMn);

    swiRestore(key);
//done:
    return t;
}

void TIM_Close(uint8_t t)
{
      //  GPTimerCC26XX_unregisterInterrupt(ts[t].TIMn);
    uint32_t key;
    key = swiDisable();
        GPTimerCC26XX_stop(ts[t].TIMn);
 //       GPTimerCC26XX_disableInterrupt(ts[t].TIMn, GPT_INT_TIMEOUT);
     swiRestore(key);

}
#endif

UINT8 TIM_CheckTimeout(UINT8 t)
{
	if(get_real_timer(t) != NULL)
	{
//	    pinfoEsl("%d",ts[t].count);
//		pdebug("timecnt=%d\r\n", ts[t-1].count);
		return ts[t].timeout;
	}
	else
	{
		return 1;
	}
}

INT32 TIM_GetCount(UINT8 t)
{
	if(t <= (sizeof(ts)/sizeof(timer_t)))
	{
		return ts[t].count;
	}
	else
	{
		return -1;
	}
}

void hwi_timerCallback0(GPTimerCC26XX_Handle handle, GPTimerCC26XX_IntMask interruptMask)
{
    ISR_Handle(&ts[0]);
}
void hwi_timerCallback1(GPTimerCC26XX_Handle handle, GPTimerCC26XX_IntMask interruptMask)
{
    ISR_Handle(&ts[1]);
}
void hwi_timerCallback2(GPTimerCC26XX_Handle handle, GPTimerCC26XX_IntMask interruptMask)
{
    ISR_Handle(&ts[2]);
}

extern uint8_t my_frame1_flg;
static void ISR_Handle(volatile timer_t* n)
{
    if (1 == my_frame1_flg){
        //BSP_GPIO_test(DEBUG_IO3);
    }

	if(n->TIMn != NULL)
	{
		if(n->direction > 0)
		{
			if((++n->count) >= n->direction)
			{
				n->timeout = 1;
			}
		}
		else
		{
			if((--n->count) <= 0)
			{
				n->timeout = 1;
			}
		}
	}
}





//void TIM_SetSoftInterrupt(UINT8 enable, void (*p)(void))
//{
//	if(enable == 0)
//	{
//		TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
//		TIM_Cmd(TIM4, ENABLE);
//		tim_soft_callback = NULL;
//	}
//	else
//	{
//		TIM_TimeBaseInitTypeDef timer;
//
//		tim_soft_callback = p;
//
//		TIM_TimeBaseStructInit(&timer);
//		TIM_DeInit(TIM4);
//
//		timer.TIM_Period = 100;
//		timer.TIM_Prescaler = 7199; //100us
//
//		TIM_TimeBaseInit(TIM4, &timer);
//		TIM_ClearFlag(TIM4, TIM_IT_Update);
//		TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
//		TIM_ARRPreloadConfig(TIM4, DISABLE);
//		TIM_Cmd(TIM4, ENABLE);
//	}
//}

//void TIM_SoftInterrupt(void)
//{
//	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
//	{
//		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
//		if(tim_soft_callback != NULL)
//		{
//			tim_soft_callback();
//		}
//	}
//}

