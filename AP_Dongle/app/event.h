#ifndef _EVENT_H_
#define _EVENT_H_

#include "datatype.h"

#define EVENT_NONE				0
#define EVENT_PARSE_DATA		((uint32_t)1<<0) //1
#define EVENT_ESL_UPDATA		((uint32_t)1<<1) //2
#define EVENT_FW_UPDATA			((uint32_t)1<<2) //4
#define EVENT_RX_TO_FLASH		((uint32_t)1<<3) //8
#define EVENT_TX_FROM_FLASH		((uint32_t)1<<4) //A
#define EVENT_TX_ESL_ACK		((uint32_t)1<<5) //32
#define EVENT_G3_HEARTBEAT		((uint32_t)1<<6) //64
#define EVENT_RC_REQ			((uint32_t)1<<7) //64
#define EVENT_ALL                              0xFFFFFFFF

#define EVENT_SCAN_WKUP			(1<<11)
#define EVENT_ASS_ACK			(1<<12)

#define EVENT_RF_TXRX			(1<<17)
#define EVENT_SCAN_BG			(1<<18) 
#define EVENT_FT_BER			(1<<19) 

#define EVENT_SYSTEM_REBOOT		(1<<21) //64

#define EVENT_FLASH_ERR			(1<<29)
#define EVENT_RF_ERR			(1<<30)

typedef enum _eventStatus{
    EVENT_BUSY,
    EVENT_IDLE,
}eventStatus;

extern UINT32 Event_Get(void);
extern void Event_Set(UINT32 event);
extern void Event_Reset(void);
extern void Event_Clear(UINT32 event);
extern UINT32 Event_GetStatus(void);
extern UINT32 Event_PentCore(void);

#endif
