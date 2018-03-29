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
#include "datatype.h"
#include "CC2592.h"

#define TRUE  1
#define FALSE 0

#define RF_RX_TIMEOUT     2
#define RF_RX_DONE        3

#define PEND_START  1
#define PEND_STOP   0
#define RF_DEFAULT_POWER    0XFF

#define  DATA_RATE_100K     (100)
#define  DATA_RATE_500K     (500)
#define  DATA_RATE_1M       ((uint8_t)10)
#define  DATA_RATE_2M       ((uint8_t)20)
#define  RF_TX_POWER_0DB    ((uint8_t)0)
#define  RF_TX_POWER_1DB    ((uint8_t)1)
#define  RF_TX_POWER_2DB    ((uint8_t)2)
#define  RF_TX_POWER_3DB    ((uint8_t)3)
#define  RF_TX_POWER_4DB    ((uint8_t)4)
#define  RF_TX_POWER_5DB    ((uint8_t)5)

/// \brief macro to convert from Radio Time Ticks to ms
#define EasyLink_RadioTime_To_ms(radioTime) ((1000 * radioTime) / 4000000)

/// \brief macro to convert from ms to Radio Time Ticks
#define EasyLink_ms_To_RadioTime(ms) (ms*(4000000/1000))

#define EasyLink_us_To_RadioTime(us) (us*(4000000/1000000))


extern uint8_t packet[26];
extern dataQueue_t dataQueue;
extern rfc_dataEntryGeneral_t* currentDataEntry;
extern uint8_t packetLength;
extern uint8_t* packetDataPointer;
extern RF_Handle rfHandle;

//extern uint8_t buffer[26];
//extern uint8_t buffer_temp[26];



extern void rf_init(void);
extern Semaphore_Handle txDoneSem;
extern Semaphore_Handle rxDoneSem;
extern void semaphore_RFInit(void);

extern void set_rf_parameters(uint16_t Data_rate, uint16_t Tx_power, uint16_t  Frequency);
//extern RF_EventMask Rf_tx_package(RF_Handle h, uint32_t syncWord, uint8_t pktLen, uint8_t* pPkt);
//extern RF_EventMask Rf_rx_package(RF_Handle h,dataQueue_t *dataQueue, uint32_t syncWord, uint8_t pktLen,uint8_t enableTrigger,  uint32_t  timeout);
extern void send_data_init(uint8_t *id, uint8_t *data, uint8_t len, uint32_t timeout);
extern RF_EventMask send_async(uint32_t interval);
extern void send_pend(RF_EventMask result);
extern uint8_t send_data(uint8_t *id, uint8_t *data, uint8_t len, uint8_t ch, uint16_t timeout);
extern uint8_t recv_data(uint8_t *id, uint8_t *data, uint8_t len, uint8_t ch, uint32_t timeout);

extern void set_frequence(uint8_t  Frequency);
extern void set_power_rate(uint8_t Tx_power, uint16_t Data_rate);
extern RF_EventMask send_without_wait(uint8_t *id, uint8_t *data, uint8_t len, uint8_t ch, uint32_t timeout);

extern void enter_txrx(void);
extern void exit_txrx(void);
extern void wait(uint32_t nus);
extern void rf_preset_hb_recv(uint8_t b);
void rf_preset_for_hb_recv(void);
uint8_t get_rssi(void);
extern void rf_exit_from_hb_recv(void);
extern UINT8 recv_data_for_hb(UINT8 *id, UINT8 *data, UINT8 len, UINT8 ch, UINT32 timeout);
extern UINT8 get_hb_rssi(void);
extern void rfCancle(RF_EventMask result);
#endif
