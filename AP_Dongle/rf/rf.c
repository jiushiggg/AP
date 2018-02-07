#include <ti/sysbios/BIOS.h>

#include "rf.h"
#include "datatype.h"
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

rfc_dataEntryGeneral_t* currentDataEntry;
RF_Object rfObject;
RF_Handle rfHandle;
dataQueue_t dataQueue;

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


const uint16_t rf_tx_power[6]={0x3161, 0x4214,0x4e18,0x5a1c, 0x9324, 0x9330};
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

RF_EventMask Rf_tx_package(RF_Handle h, uint32_t syncWord, uint8_t pktLen, uint8_t* pPkt)
{
    RF_cmdPropTxAdv.pktLen = pktLen;
    RF_cmdPropTxAdv.pPkt = pPkt;
    RF_cmdPropTxAdv.syncWord = syncWord;
    RF_EventMask result = RF_runCmd(rfHandle, (RF_Op*)&RF_cmdPropTxAdv, RF_PriorityNormal, NULL, 0);
  //  RF_yield(rfHandle);
    return result;
}

void callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    if(e & RF_EventRxEntryDone)
    {
        MsgObj  rf_rx_msg;
        rf_rx_msg.id = 2;
        rf_rx_msg.val = RF_RX_DONE;
        Mailbox_post(rf_rx_timeout_mailbox, &rf_rx_msg ,BIOS_WAIT_FOREVER);
    }
    if(e & RF_EventLastCmdDone)
    {
          //指令已经执行完成
    }
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
    RF_EventMask result = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdPropRxAdv, RF_PriorityNormal, &callback, IRQ_RX_ENTRY_DONE);
    return result;
}



static void RF_MapIO(void)
{
    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_SYSGPOCTL) = RFC_DBELL_SYSGPOCTL_GPOCTL0_CPEGPO0 |RFC_DBELL_SYSGPOCTL_GPOCTL1_RATGPO0 | RFC_DBELL_SYSGPOCTL_GPOCTL2_MCEGPO1 | RFC_DBELL_SYSGPOCTL_GPOCTL3_RATGPO1;
    IOCIOPortIdSet(RF_RX_SYNC_TEST_IO, IOC_PORT_RFC_GPO3);
    IOCIOPortIdSet(RF_RX_DATA_TEST_IO, IOC_PORT_RFC_GPO2);
    IOCIOPortIdSet(RF_TX_TEST_IO,      IOC_PORT_RFC_GPO1);
    IOCIOPortIdSet( RF_RX_TEST_IO,      IOC_PORT_RFC_GPO0);
}













