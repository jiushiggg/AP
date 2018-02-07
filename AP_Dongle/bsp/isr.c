#include "bsp_config.h"
#include "usb_istr.h"
#include "timer.h"
#include "debugcom.h"

void USB_LP_CAN1_RX0_IRQHandler(void)
{
	USB_Istr();
}

void TIM1_UP_IRQHandler(void)
{
	TIM_ISR();
}

void TIM2_IRQHandler(void)
{
	TIM_ISR();
}

void TIM3_IRQHandler(void)
{
	TIM_ISR();
}

void TIM4_IRQHandler(void)
{
	TIM_SoftInterrupt();
}

void DEBUG_COM_ISR_FUNCTION(void)
{
	DEBUG_COM_ISR_FUNC();
}









