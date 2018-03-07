#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
// #include <ti/drivers/I2C.h>
// #include <ti/drivers/SDSPI.h>
// #include <ti/drivers/SPI.h>
// #include <ti/drivers/UART.h>
// #include <ti/drivers/Watchdog.h>
#include <ti/drivers/Power.h>

#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/drivers/Power/PowerCC26XX.h>
#include <xdc/runtime/Error.h>

/* Board Header file */
#include "Board.h"
#include "datatype.h"
#include "../peripheral/extern_flash.h"
#include "core.h"
#include "timer.h"
#include "debug.h"
#include "flash.h"
#include "esl.h"
#include "corefunc.h"
#include "coremem.h"
#include "corehandlecmd.h"
#include "xmodem.h"
#include "bsp.h"
#include "rftest.h"
#include "rcuplink.h"
#include "heartbeat.h"
#include "assap.h"
#include "updata.h"
#include "thread.h"
#include "cc2640r2_rf.h"
#include "communicate.h"
#include "event.h"



core_task_t local_task;
UINT32 core_idel_flag = 0;

UINT32 Core_GetQuitStatus()
{
	return core_idel_flag;
}

void Core_ResetQuitStatus()
{
	core_idel_flag = 0;
}

void Core_Init(void)
{
	heartbeat_init();
	
	core_idel_flag = 0;
	
	memset(&local_task, 0 , sizeof(core_task_t));
	
	Xmodem_InitCallback();
	
//	USBD_SetRecvMode(1); //call back mode
}

void Core_TxHandler(void)
{
	/* tx ack */	
	if(Core_SendCmd(local_task.ack, local_task.ack_len, local_task.ack_ptr) == 0)
	{
		perr("Core_TxHandler tx cmd 0x%04X\r\n", local_task.ack);
	}
	else
	{
		pinfo("Core_TxHandler tx cmd 0x%04X success!\r\n", local_task.ack);
	}
}

void Core_RxHandler(void)
{
	memset(&local_task, 0 , sizeof(core_task_t));
	
	local_task.data_ptr = Xmodem_GetCallbackData(&local_task.data_len);
	memcpy((void *)&local_task.cmd, local_task.data_ptr, 2);
	memcpy((void *)&local_task.cmd_len, local_task.data_ptr+2, 4);
	pinfo("Core_RxHandler() rx cmd = 0x%04X, cmd len = %d\r\n", local_task.cmd, local_task.cmd_len);
//	pdebughex(local_task.data_ptr, local_task.data_len);
	if(local_task.cmd_len > sizeof(local_task.cmd_buf))
	{
		perr("Core_RxHandler() cmd len too big!\r\n");
		goto done;
	}	
	memcpy(local_task.cmd_buf, local_task.data_ptr+6, local_task.cmd_len);
	/* !!!! do not use xmodem_send() function in below fucntions */
	switch(local_task.cmd)
	{
		case CORE_CMD_ESL_UPDATA_REQUEST:
			Core_HandleEslUpdataReq(&local_task);
			break;
		case CORE_CMD_ESL_HB_REQUEST:
			Core_HandleG3Heartbeat(&local_task);
			break;
		case CORE_CMD_RCREQ_REQUEST:
			Core_HandleRcReqRequest(&local_task);
			break;
		case CORE_CMD_SOFT_REBOOT:
			Core_HandleSoftReboot();
			break;
		case CORE_CMD_FT_RR_TXNULL:
			Core_HandleRummanTest(&local_task);
			break;
		case CORE_CMD_SCAN_BG:
			Core_HandleScanBG(&local_task);
			break;
		case CORE_CMD_FT_RF_BER:
			Core_HandleFTBerTest(&local_task);
			break;
		case CORE_CMD_QUERY_SOFT_VER:
			Core_HandleQuerySoftVer(&local_task);
			break;
		case CORE_CMD_QUERY_STATUS:
			Core_HandleQueryStatus(&local_task);
			break;
		case CORE_CMD_SET_DEBUG_LEVEL:
			Core_SetDebugLevel(&local_task);
			break;
		case CORE_CMD_SET_RF_LOG:
			Core_SetRfLog(&local_task);
			break;
		case CORE_CMD_BACK_TO_IDLE:
			Core_BackToIdel(&local_task);
			break;
		case CORE_CMD_SCAN_WKUP:
			Core_HandleScanWkup(&local_task);
			break;
		case CORE_CMD_ASS_ACK:
			Core_HandleAssAck(&local_task);
			break;
		case CORE_CMD_RF_TXRX:
			core_handle_rf_txrx(&local_task);
			break;
		case CORE_CMD_ESL_ACK_REQUEST:
//			Core_HandleQueryEslUpdataAck(&local_task);
		case CORE_CMD_SCAN_DEVICE:   //0x1006
		    Core_HandleScanAck(&local_task);
		    break;
		default:
			perr("Core_RxHandler() invalid cmd: 0x%04X\r\n", local_task.cmd);
			break;
	}
	
done:
	return;
}
extern INT32 wakeup_start(UINT32 addr, UINT32 len, UINT8 type);

void Core_Mainloop(void)
{
    uint32_t event = 0;
    while (1) {

        event = Event_PendCore();

//        if(event & EVENT_RX_TO_FLASH)
//        {
//            pinfo("core recv data to flash start.\r\n");
//            memcpy((UINT8 *)&local_task.flash_data_len, local_task.cmd_buf, sizeof(local_task.flash_data_len));
//            BSP_GPIO_ToggleDebugPin();
//            if(Core_MallocFlash(&local_task.flash_data_addr, local_task.flash_data_len) == 1)
//            {
//                BSP_GPIO_ToggleDebugPin();
//                if(Core_SendCmd(0x10F0, 0, NULL) == 1)
//                {
//                    BSP_GPIO_ToggleDebugPin();
//                    if(Core_RecvDataToFlash(local_task.flash_data_addr, local_task.flash_data_len) == 1)
//                    {
//                        Event_Set(EVENT_PARSE_DATA);
//                        BSP_GPIO_ToggleDebugPin();
//                    }
//                }
//            }
//            else
//            {
//                Core_SendCmd(CORE_CMD_FLASH_ERROR, 0, NULL);
//            }
//            pinfo("core recv data to flash exit.\r\n");
//            Event_Clear(EVENT_RX_TO_FLASH);
//        }

        if(event & EVENT_PARSE_DATA)
        {
            pinfo("core parse flash data.\r\n");
            Core_ParseFlashData(local_task.flash_data_addr);
            Event_Clear(EVENT_PARSE_DATA);
        }


        if(event & EVENT_ESL_UPDATA)
        {
            esl_updata_t *this_updata;
            pinfo("core updata esl start.\r\n");
            local_task.flash_ack_addr = MALLOC_FAIL;
            local_task.flash_ack_len = 0;// reset ack info
            this_updata=Core_Malloc(sizeof(esl_updata_t));
            if(this_updata == NULL)
            {
                perr("core malloc g3 updata table!\r\n");
                Core_SendCmd(CORE_CMD_RAM_ERROR, 0, NULL);
            }
            else
            {
                this_updata->sid = 0;
                this_updata->data_addr = local_task.flash_data_addr;
                this_updata->data_len = local_task.flash_data_len;
                this_updata->ack_addr = 0;
                this_updata->ack_len = 0;
                esl_updata(this_updata);

                if((this_updata->ack_addr!=MALLOC_FAIL) && (this_updata->ack_len!=0))
                {
                    Core_SendDataFromFlash(this_updata->ack_addr, this_updata->ack_len);//tx ack
                }
            }
            pinfo("core updata esl exit.\r\n");
            Event_Clear(EVENT_ESL_UPDATA);
        }
#if 1
        if(event & EVENT_G3_HEARTBEAT)
        {
            g3_hb_table_t *hb_table;
            pinfo("core g3 hb start.\r\n");
            hb_table = Core_Malloc(sizeof(g3_hb_table_t));

            //BSP_Delay1MS(100);
            if(hb_table == NULL)
            {
                perr("core malloc g3 hb table!\r\n");
                Core_SendCmd(CORE_CMD_RAM_ERROR, 0, NULL);
            }
            else
            {
                if(Core_SendCmd(0x10f0, 0, NULL) == 1)
                {
                    heartbeat_mainloop(local_task.cmd_buf, local_task.cmd_len, hb_table, Core_SendData);
                }
            }
            pinfo("core g3 hb exit.\r\n");
            Event_Clear(EVENT_G3_HEARTBEAT);
        }

        if(event & EVENT_RC_REQ)
        {
            rcreq_table_t *rcreq_table;

            pinfo("core enter handle rc req\r\n");
            rcreq_table = Core_Malloc(sizeof(rcreq_table_t));

            //BSP_Delay1MS(100);

            if(rcreq_table == NULL)
            {
                perr("core malloc rcreq_table(size = %d)!\r\n", sizeof(rcreq_table_t));
                Core_SendCmd(CORE_CMD_RAM_ERROR, 0, NULL);
            }
            else
            {
                if(RcReq_ParseCmd(local_task.cmd_buf, local_task.cmd_len, rcreq_table) < 0)
                {
                    perr("RcReq_ParseCmd\r\n");
                    Core_SendCmd(0x10FF, 0, NULL);
                }
                else
                {
//                    RcReq_Mainloop(rcreq_table, Core_SendData);
                }
            }

            pinfo("core exit handle rc req\r\n");
            Event_Clear(EVENT_RC_REQ);
        }

//        if(event & EVENT_FW_UPDATA)
//        {
//            UINT8 flag = 0;
//
//            pinfo("core updata firmware.\r\n");
//            if(Core_CheckBinData(local_task.flash_data_addr) == 1)
//            {
//                if(Core_SetBootData(local_task.flash_data_addr) == 1)
//                {
//                    flag = 1;
//                }
//                else
//                {
//                    perr("core set boot.\r\n");
//                }
//            }
//            else
//            {
//                perr("core check bin.\r\n");
//            }
//
//            if(flag == 1)
//            {
//                if(Core_SendCmd(0x10F0, 0, NULL) == 1)
//                {
//                    Core_HandleSoftReboot();
//                }
//            }
//            else
//            {
//                Core_SendCmd(0x10FF, 0, NULL);
//            }
//            Event_Clear(EVENT_FW_UPDATA);
//        }
//        if(event & EVENT_TX_ESL_ACK)
//        {
//            pinfo("core tx esl ack.\r\n");
//            Event_Clear(EVENT_TX_ESL_ACK);
//        }

        if(event & EVENT_SYSTEM_REBOOT)
        {
            pinfo("core system reboot.\r\n");
            BSP_Reboot();
            Event_Clear(EVENT_SYSTEM_REBOOT);
        }

        //EVENT_FT_BER
        if(event & EVENT_FT_BER)
        {
            INT32 fret = 0;
            pinfo("core ft ber.\r\n");
            fret = rft_check_ber_data(local_task.cmd_buf, local_task.cmd_len);
            if(fret < 0)
            {
                Core_SendCmd(0x10F5, 0, NULL);
            }
            else if(fret != 0) //test board
            {
                Core_SendCmd(0x10F0, 0, NULL);
                rft_ber(local_task.ack_buf, sizeof(local_task.ack_buf));
            }
            else //gold board
            {
                fret = rft_ber(local_task.ack_buf, sizeof(local_task.ack_buf));
                Core_SendCmd(0x10F0, fret, local_task.ack_buf);
            }
            Event_Clear(EVENT_FT_BER);
        }

        //EVENT_SCAN_BG
        if(event & EVENT_SCAN_BG)
        {
            INT32 fret = 0;
            pinfo("core scan bg\r\n");
            fret = rft_scan_bg(local_task.cmd_buf, local_task.cmd_len, local_task.ack_buf, sizeof(local_task.ack_buf));
            if(fret <= 0)
            {
                Core_SendCmd(CORE_CMD_ERROR, 0, NULL);
            }
            else
            {
                Core_SendCmd(0x10F0, fret, local_task.ack_buf);
            }
            Event_Clear(EVENT_SCAN_BG);
        }

        if(event & EVENT_ASS_ACK)
        {
            assap_ack_table_t *p_assap_ack_table = Core_Malloc(sizeof(assap_ack_table_t));
            INT32 ret = 0;

            if(p_assap_ack_table == NULL)
            {
                Core_SendCmd(0x10F4, 0, NULL);
            }
            else
            {
                if(assap_ack_parse_cmd(local_task.cmd_buf, local_task.cmd_len, p_assap_ack_table) < 0)
                {
                    Core_SendCmd(0x10FF, 0, NULL);
                }
                else
                {
                    ret = assap_ack(p_assap_ack_table);
                    if(ret < 0)
                    {
                        pinfo("assap_ack break\r\n");
                    }
                    else
                    {
                        Core_SendData(p_assap_ack_table->data_buf, p_assap_ack_table->data_len);
                    }
                }
            }
            Event_Clear(EVENT_ASS_ACK);
        }

        if(event & EVENT_SCAN_WKUP)
        {
            INT32 ds = assap_scanwkup_ret_size();
            UINT8 *dst = Core_Malloc(ds);
            INT32 len = 0;

            if(assap_scanwkup_parse_cmd(local_task.cmd_buf, local_task.cmd_len) < 0)
            {
                Core_SendCmd(0x10FF, 0, NULL);
            }
            else
            {
                if(ds == NULL)
                {
                    Core_SendCmd(0x10F4, 0, NULL);
                }
                else
                {
                    len = assap_scan_wkup(dst, ds);
                    Core_SendData(dst, len);
                }
            }
            Event_Clear(EVENT_SCAN_WKUP);
        }

        if(event & EVENT_RF_TXRX)
        {
            local_task.ack_len = rf_txrx(local_task.cmd_buf, local_task.cmd_len, local_task.ack_buf, CORE_CMD_LEN);
            Core_SendData(local_task.ack_buf, local_task.ack_len);
            Event_Clear(EVENT_RF_TXRX);
        }

        if(Core_GetQuitStatus() == 1)
        {
            pinfo("Core_ResetQuitStatus\r\n");
            Core_ResetQuitStatus();
        }
#endif
    }
}

