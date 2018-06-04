#ifndef _CORE_H_
#define _CORE_H_

#include "datatype.h"
#include "xmodem.h"

#define CORE_CMD_SCAN_DEVICE            0x1006          //use uart 1step
#define CORE_CMD_ESL_UPDATA_REQUEST		0x1041
#define CORE_CMD_ESL_ACK_REQUEST		0x1042
#define CORE_CMD_FW_UPDATA_REQUEST		0x1044
#define CORE_CMD_ESL_HB_REQUEST			0x1048          //3step
#define CORE_CMD_RCREQ_REQUEST			0x1049

#define CORE_CMD_SOFT_REBOOT			0x1000
#define CORE_CMD_QUERY_SOFT_VER			0x1001          //2step
#define CORE_CMD_QUERY_STATUS			0x1002
#define CORE_CMD_BACK_TO_IDLE			0x1003
#define CORE_CMD_SET_DEBUG_LEVEL		0x1004
#define CORE_CMD_SET_RF_LOG				0x1005

#define CORE_CMD_FT_RR_TXNULL			0x10A0
#define CORE_CMD_FT_RF_BER				0x10A1
#define CORE_CMD_SCAN_BG				0x10A2
#define CORE_CMD_RF_TXRX				0x10A3

#define CORE_CMD_SCAN_WKUP				0x10B1
#define CORE_CMD_ASS_ACK				0x10B2

#define CORE_CMD_FW_DATA				0x0400

#define CORE_CMD_ACK					0x10F0
#define CORE_CMD_BUSY					0x10F1
#define CORE_CMD_FLASH_ERROR			0x10F2
#define CORE_CMD_FLASH_ERROR2			0x10F3
#define CORE_CMD_RAM_ERROR				0x10F4
#define CORE_CMD_PARA_ERROR				0x10F5
#define CORE_CMD_ERROR					0x10FF



/* Stack size in bytes */
#define GPRAM_BASE  0x11000000
#define TASK0_STACKSIZE   (2048- 512)
#define TASK0_ADDR  (GPRAM_BASE)

#if defined(TASK1)
#define TASK1_STACKSIZE   (2048)
#else
#define TASK1_STACKSIZE   (0)
#endif
#define TASK1_ADDR              (GPRAM_BASE+TASK0_STACKSIZE)


#define XCB_BUF_ADDR            (GPRAM_BASE+TASK0_STACKSIZE+TASK1_STACKSIZE)


#define XMODEM_LEN_ALL_SIZE     XMODEM_LEN_ALL
#define XMODEM_LEN_ALL_ADDR     (GPRAM_BASE+TASK0_STACKSIZE+TASK1_STACKSIZE+XCB_BUF_SIZE)

#define CORE_TASK_SIZE          (sizeof(core_task_t))
#define CORE_TASK_ADDR          (GPRAM_BASE+TASK0_STACKSIZE+TASK1_STACKSIZE+XCB_BUF_SIZE+XMODEM_LEN_ALL_SIZE)



#define CORE_CMD_LEN            XMODEM_LEN_ALL
typedef struct
{
    UINT16 cmd;
    UINT32 cmd_len;
    UINT8 cmd_buf[CORE_CMD_LEN];
    UINT8 *data_ptr;
    UINT32 data_len;
    UINT32 flash_data_addr;
    UINT32 flash_data_len;
    UINT32 flash_ack_addr;
    UINT32 flash_ack_len;
    UINT16 ack;
    UINT8 ack_len;
    UINT8 *ack_ptr;
    UINT8 ack_buf[CORE_CMD_LEN];
}core_task_t;

extern void Core_Init(void);
extern void Core_RxHandler(void);
extern void Core_TxHandler(void);
extern void Core_Mainloop(void);
extern UINT32 Core_GetQuitStatus(void);
extern void Core_ResetQuitStatus(void);

extern core_task_t local_task;

#endif
