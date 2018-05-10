#include <string.h>
#include "corehandlecmd.h"
#include "event.h"
#include "timer.h"
#include "debug.h"
#include "corefunc.h"
#include "bsp.h"
#include "rftest.h"
#include "thread.h"
#include "communicate.h"



eventStatus Core_CheckBusy(void)
{
	uint32_t e = Event_Get();
	
	if((e & EVENT_ESL_UPDATA) 
		|| (e & EVENT_G3_HEARTBEAT)
		|| (e & EVENT_RC_REQ)
		|| (e & EVENT_SCAN_WKUP)
		|| (e & EVENT_ASS_ACK)
		|| (e & EVENT_SCAN_BG)
		|| (e & EVENT_RF_TXRX))
	{
		return EVENT_BUSY;
	}
	else
	{
		return EVENT_IDLE;
	}
}

extern UINT8 gFTRummanTestChannel;
extern UINT8 gFTRummanTestPower;

void Core_HandleRummanTest(core_task_t *task)
{
	/* handle cmd */
	if(EVENT_BUSY == Core_CheckBusy())
	{
		task->ack = 0x10F1; // busy
	}
	else
	{
		gFTRummanTestChannel = task->cmd_buf[0];
		gFTRummanTestPower = task->cmd_buf[1];
		/* check para */
		if(gFTRummanTestPower > 3)
		{
			task->ack = CORE_CMD_PARA_ERROR; // ack
		}
		else
		{
			task->ack = CORE_CMD_ACK; // ack
			pinfo("Core cmd rumman test, channel: %d, power: %d\r\n", gFTRummanTestChannel, gFTRummanTestPower);
			rft_tx_null();
		}		
	}
	
	task->ack_len = 0;
	task->ack_ptr = NULL;
		
	TIM_SetSoftInterrupt(1, Core_TxHandler);
}

static void _ack_busy(core_task_t *task)
{
	task->ack = 0x10F1; // busy
	task->ack_len = 0;
	task->ack_ptr = NULL;	
	TIM_SetSoftInterrupt(1, Core_TxHandler);
}

void Core_HandleFTBerTest(core_task_t *task)
{
	/* handle cmd */
	if(EVENT_BUSY == Core_CheckBusy())
	{
		task->ack = 0x10F1; // busy
		task->ack_len = 0;
		task->ack_ptr = NULL;	
		TIM_SetSoftInterrupt(1, Core_TxHandler);
	}
	else
	{
		Event_Set(EVENT_FT_BER);
	}
}

void Core_HandleScanAck(core_task_t *task)
{
    /* handle cmd */
    if(EVENT_BUSY == Core_CheckBusy())
    {
        task->ack = 0x10F1; // busy
        task->ack_len = 0;
        task->ack_ptr = NULL;
        TIM_SetSoftInterrupt(1, Core_TxHandler);
    }
    else
    {
        Event_communicateSet(EVENT_COMMUNICATE_SCAN_DEVICE);
    }
}

void Core_HandleScanBG(core_task_t *task)
{
	/* handle cmd */
	if(EVENT_BUSY == Core_CheckBusy())
	{
		task->ack = 0x10F1; // busy
		task->ack_len = 0;
		task->ack_ptr = NULL;	
		TIM_SetSoftInterrupt(1, Core_TxHandler);
	}
	else
	{
		Event_Set(EVENT_SCAN_BG);
	}
}

void Core_HandleEslUpdataReq(core_task_t *task)
{
	/* handle cmd */
	if(EVENT_BUSY == Core_CheckBusy())
	{
		task->ack = 0x10F1; // busy
		task->ack_len = 0;
		task->ack_ptr = NULL;

		TIM_SetSoftInterrupt(1, Core_TxHandler);
	}
	else
	{
		task->ack = CORE_CMD_ACK; // ack
		task->ack_len = 0;
		task->ack_ptr = NULL;

		Event_communicateSet(EVENT_COMMUNICATE_RX_TO_FLASH);
	}
}

void Core_HandleQueryEslUpdataAck(core_task_t *task)
{
	/* handle cmd */
	if(EVENT_BUSY == Core_CheckBusy())
	{
		task->ack = 0x10F1; // busy
		task->ack_len = 0;
		task->ack_ptr = NULL;

		TIM_SetSoftInterrupt(1, Core_TxHandler);
	}
	else
	{
	    Event_communicateSet(EVENT_COMMUNICATE_TX_ESL_ACK);
	}
}

void Core_HandleG3Heartbeat(core_task_t *task)
{
	/* handle cmd */
	if(EVENT_BUSY == Core_CheckBusy())
	{
		task->ack = 0x10F1; // busy
		task->ack_len = 0;
		task->ack_ptr = NULL;
		
		TIM_SetSoftInterrupt(1, Core_TxHandler);
	}
	else
	{
		task->ack = CORE_CMD_ACK; // ack
		task->ack_len = 0;
		task->ack_ptr = NULL;
		
		Event_Set(EVENT_G3_HEARTBEAT);
	}
}

void Core_HandleRcReqRequest(core_task_t *task)
{
	/* handle cmd */
	if(EVENT_BUSY == Core_CheckBusy())
	{
		task->ack = 0x10F1; // busy
		task->ack_len = 0;
		task->ack_ptr = NULL;
		
		TIM_SetSoftInterrupt(1, Core_TxHandler);
	}
	else
	{
		task->ack = CORE_CMD_ACK; // ack
		task->ack_len = 0;
		task->ack_ptr = NULL;
		
		Event_Set(EVENT_RC_REQ);
	}
}

void Core_HandleSoftReboot(void)
{
//	BSP_EnterCri();
	pinfo("core set system reboot!\r\n");
	BSP_Reboot();
}

extern const unsigned char APP_VERSION_STRING[];
void Core_HandleQuerySoftVer(core_task_t *task)
{
	task->ack = CORE_CMD_ACK; // ack
	task->ack_len = strlen((const char *)APP_VERSION_STRING)+1;
	strcpy((char *)task->ack_buf, (const char *)APP_VERSION_STRING);
	task->ack_buf[task->ack_len-1] = 0;
	task->ack_ptr = task->ack_buf;
	
	TIM_SetSoftInterrupt(1, Core_TxHandler);
}

void Core_HandleQueryStatus(core_task_t *task)
{
	UINT32 status = Event_GetStatus();
	
	task->ack = CORE_CMD_ACK; // ack
	task->ack_len = sizeof(status);
	memcpy(task->ack_buf, &status, sizeof(status));
	task->ack_ptr = task->ack_buf;
	
	TIM_SetSoftInterrupt(1, Core_TxHandler);
}

extern volatile UINT32 s_debug_level;
void Core_SetDebugLevel(core_task_t *task)
{
	memcpy((void *)&s_debug_level, task->cmd_buf, sizeof(s_debug_level));
	pinfo("core set debug level = %d.\r\n", s_debug_level);
	if(s_debug_level > DEBUG_LEVEL_DEBUG)
	{
		s_debug_level = 2;
		pinfo("warning: change to %d.\r\n", s_debug_level);
	}
}

extern UINT32 g3_print_ctrl;
void Core_SetRfLog(core_task_t *task)
{
	memcpy((void *)&g3_print_ctrl, task->cmd_buf, sizeof(g3_print_ctrl));
	pinfo("core set rf log print = %d.\r\n", g3_print_ctrl);
}

extern UINT32 core_idel_flag;
void Core_BackToIdel(core_task_t *task)
{
	pinfo("core set system back to idel!\r\n");
	core_idel_flag = 1;
}

void Core_HandleScanWkup(core_task_t *task)
{
	if(EVENT_BUSY == Core_CheckBusy())
	{
		task->ack = 0x10F1; // busy
		task->ack_len = 0;
		task->ack_ptr = NULL;
		
		TIM_SetSoftInterrupt(1, Core_TxHandler);
	}
	else
	{	
		Event_Set(EVENT_SCAN_WKUP);
	}
}

void Core_HandleAssAck(core_task_t *task)
{
	if(EVENT_BUSY == Core_CheckBusy())
	{
		task->ack = 0x10F1; // busy
		task->ack_len = 0;
		task->ack_ptr = NULL;
		
		TIM_SetSoftInterrupt(1, Core_TxHandler);
	}
	else
	{	
		Event_Set(EVENT_ASS_ACK);
	}
}

void core_handle_rf_txrx(core_task_t *task)
{
	if(EVENT_BUSY == Core_CheckBusy())
	{
		_ack_busy(task);
	}
	else
	{	
		Event_Set(EVENT_RF_TXRX);
	}
}
