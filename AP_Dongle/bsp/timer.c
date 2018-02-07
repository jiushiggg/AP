#include "timer.h"
#include "debug.h"


typedef UINT8 TIM_TypeDef ;

typedef struct
{
	UINT8 sn;
	TIM_TypeDef TIMn;
	volatile UINT8 inuse;
	volatile INT32 count;
	volatile UINT8 timeout;
	volatile INT32 order;
}timer_t;

timer_t ts[] = {
	{1, 1, 0, 0, 0, 0},
	{2, 2, 0, 0, 0, 0},
	{3, 3, 0, 0, 0, 0},
//	{4, 4, 0, 0, 0, 0},
};

static UINT8 get_a_free_timer(void)
{
	INT32 i;
	UINT8 t = 0;
	UINT8 timer_num = sizeof(ts)/sizeof(timer_t);
//
//	for(i = 0; i < timer_num; i++)
//	{
//		if(ts[i].inuse == 0)
//		{
//			t = ts[i].sn;
//			break;
//		}
//	}
//
	return t;
}

static TIM_TypeDef *get_real_timer(UINT8 t)
{
//	if((t > 0) && (t <= (sizeof(ts)/sizeof(timer_t))))
//	{
//		return ts[t-1].TIMn;
//	}
//	else
//	{
//		return NULL;
//	}
}

static UINT8 get_timer_sn(TIM_TypeDef *tim)
{
	UINT8 t = 0;

//	if(tim == TIM1)
//	{
//		t = 1;
//	}
//	else if(tim == TIM2)
//	{
//		t = 2;
//	}
//	else if(tim == TIM3)
//	{
//		t = 3;
//	}
//	else if(tim == TIM4)
//	{
//		t = 4;
//	}
//	else
//	{
//		t = 0;
//	}
	
	return t;
}

/*
** Tout=(arr+1)*(pcs+1)/Fclk
** Tout 单位是 us
** Fclk 单位是 Mhz
*/
UINT8 TIM_Open(INT32 nms, UINT16 cnt, UINT8 down)
{
	UINT8 t = 0;
//	TIM_TimeBaseInitTypeDef timer;
//	TIM_TypeDef *tim;
	INT32 period = 0;
//
//	period = nms*10 - 1;
//	if((period > 60000) || (period <= 0))
//	{
//		goto done;
//	}
//
//	if((t=get_a_free_timer()) == 0)
//	{
//		goto done;
//	}
//	ts[t-1].inuse = 1;
//	ts[t-1].timeout = 0;
//	tim = ts[t-1].TIMn;
//
//	TIM_TimeBaseStructInit(&timer);
//	TIM_DeInit(tim);
//
//	timer.TIM_Period = period;
//	timer.TIM_Prescaler = 7199; //100us
//
//	if(down == 0) //up count
//	{
//		ts[t-1].count = 0;
//		ts[t-1].order = cnt;
//	}
//	else //down count
//	{
//		ts[t-1].count = cnt;
//		ts[t-1].order = 0;
//	}
//
//	TIM_TimeBaseInit(tim, &timer);
//	TIM_ClearFlag(tim, TIM_IT_Update);
//	TIM_ITConfig(tim, TIM_IT_Update, ENABLE);
//	TIM_ARRPreloadConfig(tim, DISABLE);
//	TIM_Cmd(tim, ENABLE);
//
//done:
	return t;
}

void TIM_SetSoftTimeout(UINT8 t)
{
//	if(get_real_timer(t) != NULL)
//	{
//		ts[t-1].timeout = 1;
//	}
}

void TIM_Close(UINT8 t)
{
//	if(get_real_timer(t) != NULL)
//	{
//		ts[t-1].inuse = 0;
//
//		TIM_Cmd(get_real_timer(t), DISABLE);
//		TIM_ITConfig(get_real_timer(t), TIM_IT_Update, DISABLE);
//	}
}

UINT8 TIM_CheckTimeout(UINT8 t)
{
//	if(get_real_timer(t) != NULL)
//	{
////		pdebug("timecnt=%d\r\n", ts[t-1].count);
//		return ts[t-1].timeout;
//	}
//	else
//	{
//		return 1;
//	}
}

INT32 TIM_GetCount(UINT8 t)
{
//	if((t > 0) && (t <= (sizeof(ts)/sizeof(timer_t))))
//	{
//		return ts[t-1].count;
//	}
//	else
//	{
//		return -1;
//	}
}

UINT8 TIM_Delay1MS(UINT16 n)
{
	UINT8 t;
	
//	if((t=TIM_Open(1, n, 0)) == 0)
//	{
//		goto done;
//	}
//
//	while(TIM_CheckTimeout(t));
//	TIM_Close(t);
	
done:
	return t;
}

void TIM_ISR(void)
{
//	if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
//	{
//		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
//		/* user tim1 isr */
//		if(ts[get_timer_sn(TIM1)-1].order > 0)
//		{
//			if((++ts[get_timer_sn(TIM1)-1].count) >= ts[get_timer_sn(TIM1)-1].order)
//			{
//				ts[get_timer_sn(TIM1)-1].timeout = 1;
//			}
//		}
//		else
//		{
//			if((--ts[get_timer_sn(TIM1)-1].count) <= 0)
//			{
//				ts[get_timer_sn(TIM1)-1].timeout = 1;
//			}
//		}
//	}
//
//	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
//	{
//		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
//
//		if(ts[get_timer_sn(TIM2)-1].order > 0)
//		{
//			if((++ts[get_timer_sn(TIM2)-1].count) >= ts[get_timer_sn(TIM2)-1].order)
//			{
//				ts[get_timer_sn(TIM2)-1].timeout = 1;
//			}
//		}
//		else
//		{
//			if((--ts[get_timer_sn(TIM2)-1].count) <= 0)
//			{
//				ts[get_timer_sn(TIM2)-1].timeout = 1;
//			}
//		}
//	}
//
//	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
//	{
//		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
//
//		if(ts[get_timer_sn(TIM3)-1].order > 0)
//		{
//			if((++ts[get_timer_sn(TIM3)-1].count) >= ts[get_timer_sn(TIM3)-1].order)
//			{
//				ts[get_timer_sn(TIM3)-1].timeout = 1;
//			}
//		}
//		else
//		{
//			if((--ts[get_timer_sn(TIM3)-1].count) <= 0)
//			{
//				ts[get_timer_sn(TIM3)-1].timeout = 1;
//			}
//		}
//	}
	
//	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) 
//	{
//		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
//		
//		if(ts[get_timer_sn(TIM4)-1].order > 0)
//		{
//			if((++ts[get_timer_sn(TIM4)-1].count) >= ts[get_timer_sn(TIM4)-1].order)
//			{
//				ts[get_timer_sn(TIM4)-1].timeout = 1;
//			}		
//		}
//		else
//		{
//			if((--ts[get_timer_sn(TIM4)-1].count) <= 0)
//			{
//				ts[get_timer_sn(TIM4)-1].timeout = 1;
//			}	
//		}
//	}
}

void (*tim_soft_callback)(void);

void TIM_SetSoftInterrupt(UINT8 enable, void (*p)(void))
{
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
}

void TIM_SoftInterrupt(void)
{
//	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
//	{
//		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
//		if(tim_soft_callback != NULL)
//		{
//			tim_soft_callback();
//		}
//	}
}

