#ifndef RF_H
#define RF_H

#include <stdint.h>
#include <ti/drivers/rf/RF.h>
#include "smartrf_settings.h"
#include <stdlib.h>
#include <ti/devices/cc26x0r2/driverlib/rf_data_entry.h>
#include <ti/devices/cc26x0/inc/hw_rfc_dbell.h>
#include <ti/devices/cc26x0r2/driverlib/rf_prop_mailbox.h>
#include <ti/devices/cc26x0r2/driverlib/rf_mailbox.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Semaphore.h>
#include "RFQueue.h"

#define TRUE  1
#define FALSE 0

#define RF_RX_TIMEOUT     2
#define RF_RX_DONE        3

#define  DATA_RATE_100K     ((uint8_t)100)
#define  DATA_RATE_500K     ((uint8_t)500)
#define  DATA_RATE_1M       ((uint8_t)1000)
#define  DATA_RATE_2M       ((uint8_t)2000)
#define  RF_TX_POWER_0DB    ((uint8_t)0)
#define  RF_TX_POWER_1DB    ((uint8_t)1)
#define  RF_TX_POWER_2DB    ((uint8_t)2)
#define  RF_TX_POWER_3DB    ((uint8_t)3)
#define  RF_TX_POWER_4DB    ((uint8_t)4)
#define  RF_TX_POWER_5DB    ((uint8_t)5)

extern Mailbox_Handle  rf_rx_timeout_mailbox;
extern Semaphore_Handle  rf_rx_done_sem;
extern uint8_t packet[26];
extern dataQueue_t dataQueue;
extern rfc_dataEntryGeneral_t* currentDataEntry;
extern uint8_t packetLength;
extern uint8_t* packetDataPointer;
extern RF_Object rfObject;
extern RF_Handle rfHandle;

extern uint8_t buffer[26];
extern uint8_t buffer_temp[26];



void rf_init(void);
void set_rf_parameters(uint8_t Data_rate, uint16_t Tx_power, uint16_t  Frequency, uint8_t fractFreq_flag);
RF_EventMask Rf_tx_package(RF_Handle h, uint32_t syncWord, uint8_t pktLen, uint8_t* pPkt);
RF_EventMask Rf_rx_package(RF_Handle h,dataQueue_t *dataQueue, uint32_t syncWord, uint8_t pktLen,uint8_t enableTrigger,  uint32_t  timeout);

#endif
