#include "cc2640r2_rf.h"
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <xdc/runtime/Error.h>


#include <ti/drivers/pin/PINCC26XX.h>

#define RF_TEST
#define RF_TX_TEST_IO       IOID_28
#define RF_RX_TEST_IO       IOID_29
#define RF_RX_DATA_TEST_IO  IOID_16
#define RF_RX_SYNC_TEST_IO  IOID_17



/***** Defines *****/
#define RF_convertMsToRatTicks(microsecond_10)    ((uint32_t)(microsecond_10) * 4 * 10)   // ((uint32_t)(milliseconds) * 4 * 1000)
#define DATA_ENTRY_HEADER_SIZE 8  /* Constant header size of a Generic Data Entry */
#define MAX_LENGTH             30 /* Max length byte the radio will accept */
#define NUM_DATA_ENTRIES       1  /* NOTE: Only two data entries supported at the moment */
#define NUM_APPENDED_BYTES     2  /* The Data Entries data field will contain:
                                   * 1 Header byte (RF_cmdPropRx.rxConf.bIncludeHdr = 0x1)
                                   * Max 30 payload bytes
                                   * 1 status byte (RF_cmdPropRx.rxConf.bAppendStatus = 0x1) */

#if defined(__TI_COMPILER_VERSION__)
    #pragma DATA_ALIGN (rxDataEntryBuffer, 4);
        static uint8_t rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                                 MAX_LENGTH,
                                                                 NUM_APPENDED_BYTES)];
#elif defined(__IAR_SYSTEMS_ICC__)
    #pragma data_alignment = 4
        static uint8_t rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                                 MAX_LENGTH,
                                                                 NUM_APPENDED_BYTES)];
#elif defined(__GNUC__)
        static uint8_t rxDataEntryBuffer [RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
            MAX_LENGTH, NUM_APPENDED_BYTES)] __attribute__ ((aligned (4)));
#else
    #error This compiler is not supported.
#endif



static void RF_MapIO(void);
void clear_queue_buf(void);
RF_EventMask Rf_rx_package(RF_Handle h,dataQueue_t *dataQueue, uint32_t syncWord, uint8_t pktLen,uint8_t enableTrigger,  uint32_t  timeout);

rfc_dataEntryGeneral_t* currentDataEntry;
RF_Object rfObject;
RF_Handle rfHandle;
dataQueue_t dataQueue;
static UINT8 _hb_rssi = 0;

Semaphore_Handle txDoneSem;
Semaphore_Handle rxDoneSem;

void txcallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    if(e & RF_EventRxEntryDone)
    {
        Semaphore_post(txDoneSem);
    }
    if(e & RF_EventLastCmdDone)
    {
          //指令已经执行完成
    }
}

void rxcallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    if(e & RF_EventRxEntryDone)
    {
        Semaphore_post(rxDoneSem);
    }
    if(e & RF_EventLastCmdDone)
    {
          //指令已经执行完成
    }
}
void semaphore_init(void)
{
    Semaphore_Params params;
    Error_Block eb;

    /* Init params */
    Semaphore_Params_init(&params);
    Error_init(&eb);

    /* Create semaphore instance */
    txDoneSem = Semaphore_create(0, &params, &eb);
    rxDoneSem = Semaphore_create(0, &params, &eb);
}


void rf_init(void)
{
    RF_Params rfParams;
    RF_Params_init(&rfParams);

    if( RFQueue_defineQueue(&dataQueue, rxDataEntryBuffer,sizeof(rxDataEntryBuffer),NUM_DATA_ENTRIES, MAX_LENGTH + NUM_APPENDED_BYTES))
    {
         /* Failed to allocate space for all data entries */
         while(1);
    }
    /* Request access to the radio */
    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioSetup, &rfParams);
    /* Set the frequency */
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);

#ifdef  RF_TEST
    RF_MapIO();
#endif
//    RF_yield(rfHandle);    //使射频进入低功耗状态
}

#define POWER_LEVEL  6
const uint16_t rf_tx_power[POWER_LEVEL]={0x3161, 0x4214,0x4e18,0x5a1c, 0x9324, 0x9330};
void set_rf_parameters(uint8_t Data_rate, uint16_t Tx_power, uint16_t  Frequency, uint8_t fractFreq_flag)
{

    RF_cmdFs.frequency = Frequency;
    RF_cmdFs.fractFreq = (fractFreq_flag ? 32768 : 0);
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);
    switch(Data_rate)
    {
        case DATA_RATE_100K:
            RF_cmdPropRadioSetup.symbolRate.preScale = 15;
            RF_cmdPropRadioSetup.symbolRate.rateWord = 65536;
            RF_cmdPropRadioSetup.modulation.modType = 0x0;
            RF_cmdPropRadioSetup.modulation.deviation = 744;
            RF_cmdPropRadioSetup.rxBw = 9;
        break;
        case  DATA_RATE_500K:
            RF_cmdPropRadioSetup.symbolRate.preScale = 15;
            RF_cmdPropRadioSetup.symbolRate.rateWord = 327680;
            RF_cmdPropRadioSetup.modulation.modType = 0x0;
            RF_cmdPropRadioSetup.modulation.deviation = 744;
            RF_cmdPropRadioSetup.rxBw = 10;
        break;
        case  DATA_RATE_1M:
        break;
        case  DATA_RATE_2M:
        break;
        default:
            RF_cmdPropRadioSetup.symbolRate.preScale = 15;
            RF_cmdPropRadioSetup.symbolRate.rateWord = 327680;
            RF_cmdPropRadioSetup.modulation.modType = 0x0;
            RF_cmdPropRadioSetup.modulation.deviation = 744;
            RF_cmdPropRadioSetup.rxBw = 10;
        break;
    }
    RF_cmdPropRadioSetup.txPower = rf_tx_power[Tx_power];
    RF_control(rfHandle, RF_CTRL_UPDATE_SETUP_CMD, NULL); //Signal update Rf core
//    RF_yield(rfHandle);  // Force a power down using RF_yield() API. This will power down RF after all pending radio commands are complete.
}
void set_frequence(uint16_t  Frequency, uint8_t fractFreq_flag)
{
    RF_cmdFs.frequency = 2400+Frequency;
    RF_cmdFs.fractFreq = (fractFreq_flag ? 32768 : 0);
    RF_postCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);
}

void set_power_rate(uint16_t Tx_power, uint8_t Data_rate)
{
    switch(Data_rate)
    {
        case DATA_RATE_100K:
            RF_cmdPropRadioSetup.symbolRate.preScale = 15;
            RF_cmdPropRadioSetup.symbolRate.rateWord = 65536;
            RF_cmdPropRadioSetup.modulation.modType = 0x0;
            RF_cmdPropRadioSetup.modulation.deviation = 744;
            RF_cmdPropRadioSetup.rxBw = 9;
        break;
        case  DATA_RATE_500K:
            RF_cmdPropRadioSetup.symbolRate.preScale = 15;
            RF_cmdPropRadioSetup.symbolRate.rateWord = 327680;
            RF_cmdPropRadioSetup.modulation.modType = 0x0;
            RF_cmdPropRadioSetup.modulation.deviation = 744;
            RF_cmdPropRadioSetup.rxBw = 10;
        break;
        case  DATA_RATE_1M:
        break;
        case  DATA_RATE_2M:
        break;
        default:
            RF_cmdPropRadioSetup.symbolRate.preScale = 15;
            RF_cmdPropRadioSetup.symbolRate.rateWord = 327680;
            RF_cmdPropRadioSetup.modulation.modType = 0x0;
            RF_cmdPropRadioSetup.modulation.deviation = 744;
            RF_cmdPropRadioSetup.rxBw = 10;
        break;
    }
    if (Tx_power < POWER_LEVEL){
        RF_cmdPropRadioSetup.txPower = rf_tx_power[Tx_power];
    }
    RF_control(rfHandle, RF_CTRL_UPDATE_SETUP_CMD, NULL); //Signal update Rf core
}

//RF_EventMask Rf_tx_package(RF_Handle h, uint32_t syncWord, uint8_t pktLen, uint8_t* pPkt)
//{
//    RF_cmdPropTxAdv.pktLen = pktLen;
//    RF_cmdPropTxAdv.pPkt = pPkt;
//    RF_cmdPropTxAdv.syncWord = syncWord;
//    RF_EventMask result = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTxAdv, RF_PriorityNormal, NULL, 0);
//  //  RF_yield(rfHandle);
//    return result;
//}
void send_data_init(UINT8 *id, UINT8 *data, UINT8 len, UINT32 timeout)
{
    RF_cmdPropTxAdv.startTrigger.triggerType = TRIG_ABSTIME;
    RF_cmdPropTxAdv.startTrigger.pastTrig = 1;
    RF_cmdPropTxAdv.startTime = 0;
    RF_cmdPropTxAdv.pktLen = len;
    RF_cmdPropTxAdv.pPkt = data;
    RF_cmdPropTxAdv.syncWord = ((uint32_t)id[0]<<24) | ((uint32_t)id[1]<<16) | ((uint32_t)id[2]<<8) | id[3];
}
RF_EventMask send_async(uint32_t interal)
{
    RF_EventMask result;
 //   RF_cmdPropTxAdv.startTime += interal;
    result = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTxAdv, RF_PriorityNormal, NULL, 0);
    return result;
}

void send_pend(RF_EventMask result)
{
    RF_pendCmd(rfHandle, result, RF_EventTxEntryDone);
}
uint8_t send_data(uint8_t *id, uint8_t *data, uint8_t len, uint8_t ch, uint16_t timeout)
{
    RF_EventMask result;

    set_frequence(ch/2, ch%2);
    send_data_init(id, data, len, timeout);
    result = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropTxAdv, RF_PriorityNormal, NULL, 0);
    RF_pendCmd(rfHandle, result, RF_EventTxEntryDone);

    return len;

}
UINT8 recv_data(uint8_t *id, uint8_t *data, uint8_t len, uint8_t ch, uint32_t timeout)
{
    uint32_t sync_word=0;
 //   uint32_t tmp_timeout = 0;
    RF_EventMask rx_event;
    set_frequence(ch/2, ch%2);

    sync_word = ((uint32_t)id[0]<<24) | ((uint32_t)id[1]<<16) | ((uint32_t)id[2]<<8) | id[3];
//    tmp_timeout = EasyLink_10us_To_RadioTime(timeout/10);

    rx_event = Rf_rx_package(rfHandle, &dataQueue, sync_word, len, TRUE , timeout/10);
    if (TRUE ==  Semaphore_pend (rxDoneSem, (timeout/Clock_tickPeriod))){
        currentDataEntry = RFQueue_getDataEntry();
        memcpy(data, (uint8_t*)(&currentDataEntry->data), len);
        RFQueue_nextEntry();
    }else{
        RF_cancelCmd(rfHandle, rx_event,0);
        clear_queue_buf();
        len = 0;
    }

    return len;
}
void clear_queue_buf(void)
{
    uint8_t buf[128];
    currentDataEntry = RFQueue_getDataEntry();

    if(DATA_ENTRY_PENDING != currentDataEntry->status)
    {
        memcpy(buf, (uint8_t*)(&currentDataEntry->data), currentDataEntry->length);
        RFQueue_nextEntry();
    }
}

RF_EventMask send_without_wait(UINT8 *id, UINT8 *data, UINT8 len, UINT8 ch, UINT32 timeout)
{
    RF_EventMask result;
    set_frequence(ch/2, ch%2);
    send_data_init(id, data, len, timeout);
    result = send_async(timeout);
    return result;
}

RF_EventMask Rf_rx_package(RF_Handle h,dataQueue_t *dataQueue, uint32_t syncWord, uint8_t pktLen,uint8_t enableTrigger,  uint32_t  timeout)
{
    /* Modify CMD_PROP_RX command for application needs */
    RF_cmdPropRxAdv.syncWord0 = syncWord;
    RF_cmdPropRxAdv.pQueue = dataQueue;           /* Set the Data Entity queue for received data */
    RF_cmdPropRxAdv.maxPktLen = pktLen;        /* Implement packet length filtering to avoid PROP_ERROR_RXBUF */
    RF_cmdPropRxAdv.endTrigger.triggerType = (enableTrigger? TRIG_ABSTIME : TRIG_NEVER );
    RF_cmdPropRxAdv.endTrigger.bEnaCmd = (enableTrigger? 1 : 0 );
    RF_cmdPropRxAdv.endTime = RF_getCurrentTime();
    RF_cmdPropRxAdv.endTime += RF_convertMsToRatTicks(timeout);  //10us
//    RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropRx, RF_PriorityNormal, &callback, IRQ_RX_ENTRY_DONE);
//    RF_yield(rfHandle);
    RF_EventMask result = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRxAdv, RF_PriorityNormal, &rxcallback, IRQ_RX_ENTRY_DONE);
    return result;
}

void wait(uint32_t nus)
{

}


void enter_txrx(void)
{

}
void exit_txrx(void)
{}

void rf_preset_for_hb_recv(void)
{
}
UINT8 get_rssi(void)
{
    return 0;
}


UINT8 recv_data_for_hb(UINT8 *id, UINT8 *data, UINT8 len, UINT8 ch, UINT16 timeout)
{
    return 0;
}

void rf_exit_from_hb_recv(void)
{

}

UINT8 get_hb_rssi(void)
{
    return _hb_rssi;
}





static void RF_MapIO(void)
{
    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_SYSGPOCTL) = RFC_DBELL_SYSGPOCTL_GPOCTL0_CPEGPO0 |RFC_DBELL_SYSGPOCTL_GPOCTL1_RATGPO0 | RFC_DBELL_SYSGPOCTL_GPOCTL2_MCEGPO1 | RFC_DBELL_SYSGPOCTL_GPOCTL3_RATGPO1;
//    IOCIOPortIdSet(RF_RX_SYNC_TEST_IO, IOC_PORT_RFC_GPO3);
//    IOCIOPortIdSet(RF_RX_DATA_TEST_IO, IOC_PORT_RFC_GPO2);
    IOCIOPortIdSet(RF_TX_TEST_IO,      IOC_PORT_RFC_GPO1);
    IOCIOPortIdSet( RF_RX_TEST_IO,      IOC_PORT_RFC_GPO0);
}
