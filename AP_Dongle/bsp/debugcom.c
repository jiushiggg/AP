#include "bsp_config.h"
#include "debugcom.h"
#include "bsp.h"
#include <stm32f10x.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"

void DebugCom_Conifg(void)
{
	USART_InitTypeDef USART_InitStructure;

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(DEBUG_COM, &USART_InitStructure);	
	USART_ClearFlag(DEBUG_COM, 0xff); // clear all flag
	USART_Cmd(DEBUG_COM, ENABLE);
}

UINT16 DebugCom_SendBuf(const UINT8* src,  UINT16 len)
{
	UINT8* p_data = (UINT8*)src;
	UINT16 left_len = len;

	while(left_len > 0)
	{
		USART_SendData(DEBUG_COM, *p_data);
		while(USART_GetFlagStatus(DEBUG_COM, USART_FLAG_TXE) == RESET);	
		p_data++;
		left_len--;
	}

	return len;
}

UINT16 DebugCom_RecvBuf(UINT8 *dst, UINT16 len)
{
	INT32 left_len = len;
	UINT16 recv_len = 0;
	int timeout = 0;
	
	while(left_len > 0)
	{
		timeout = 300000; // 10ms
		while((USART_GetFlagStatus(DEBUG_COM, USART_FLAG_RXNE) == RESET))
		{
			if((timeout--) <= 0)
			{
				break;
			}
		}
		if((timeout--) <= 0)
		{
			break;
		}
		
		*(dst+recv_len) = (UINT8)USART_ReceiveData(DEBUG_COM);
		recv_len++;
		left_len--;
	}
	
	return recv_len;
}

volatile int s_usar2_isr_flag;

void DebugCom_SetISR(UINT8 enable)
{
	s_usar2_isr_flag = 0;
	
	if(enable == 0)
	{
		USART_ITConfig(DEBUG_COM, USART_IT_RXNE, DISABLE);
	}
	else
	{
		USART_ITConfig(DEBUG_COM, USART_IT_RXNE, ENABLE);
	}	
}

int DebugCom_Select(UINT16 ms)
{
	INT32 n = ms*1000;
	
	while((n--) > 0)
	{
		if(s_usar2_isr_flag == 1)
		{
			return 1;
		}
		BSP_Delay1US(1);
	}
	
	return 0;
}

UINT8 isr_recv_buf[100] = {0};
int isr_recv_len = 0;

UINT16 DebugCom_GetISRBuf(UINT8 *dst, UINT16 len)
{
	UINT16 read_len = len >isr_recv_len? isr_recv_len : len;
	
	memcpy(dst, isr_recv_buf, read_len);
	
	return read_len;
}

void DebugCom_ISR(void)
{
	if(USART_GetITStatus(DEBUG_COM, USART_IT_RXNE) != RESET)
	{
		memset(isr_recv_buf, 0,sizeof(isr_recv_buf));
		isr_recv_len = DebugCom_RecvBuf(isr_recv_buf, sizeof(isr_recv_buf));
		USART_ClearITPendingBit(DEBUG_COM, USART_IT_RXNE);
		s_usar2_isr_flag = 1;
	}
}

/* prototype of fputc, used for printf */
int fputc(int ch, FILE *f)
{	
	//add watch dog code here
	USART_SendData(DEBUG_COM, (uint8_t) ch);
	while(USART_GetFlagStatus(DEBUG_COM, USART_FLAG_TXE) == RESET);	
	return ch;
}

