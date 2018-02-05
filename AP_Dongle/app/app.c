/*
 * app.c
 *
 *  Created on: 2018年2月2日
 *      Author: ggg
 */

/* For usleep() */
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include "../rf/rf.h"
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

/* Board Header file */
#include "Board.h"
#include "datatype.h"
#include "extern_flash.h"

#define TASK0_STACKSIZE   1024
#define TASK2_STACKSIZE   (6*1024)

void *mainThread(void *arg0);


Char task0_Stack[TASK0_STACKSIZE];
Char task2_Stack[TASK2_STACKSIZE];
Task_Struct task0_Struct;
Task_Struct task2_Struct;

Mailbox_Struct  rf_rx_timeout_Struct;
Mailbox_Params  rf_rx_timeout_params;
Mailbox_Handle  rf_rx_timeout_mailbox;
MailboxMsgObj   mailboxBuffer_2[5];


void app_init(void)
{
    Task_Params taskParams_0,taskParams_2;
    //
    //    Power_setConstraint(PowerCC26XX_SB_VIMS_CACHE_RETAIN);
    //    Power_setConstraint(PowerCC26XX_NEED_FLASH_IN_IDLE);
    //
    //
    Task_Params_init(&taskParams_0);
    taskParams_0.arg0 = 1000000 / Clock_tickPeriod;
    taskParams_0.stackSize = TASK0_STACKSIZE;
    taskParams_0.stack = &task0_Stack;
    taskParams_0.priority = 2;
    Task_construct(&task0_Struct, (Task_FuncPtr)mainThread, &taskParams_0, NULL);
    //
    //    Task_Params_init(&taskParams_2);
    //    taskParams_2.arg0 = 1000000 / Clock_tickPeriod;
    //    taskParams_2.stackSize = TASK2_STACKSIZE;
    //    taskParams_2.stack = &task2_Stack;
    //    taskParams_2.priority = 1;
    //    Task_construct(&task2_Struct, (Task_FuncPtr)protocol_Fxn, &taskParams_2, NULL);
}
void mailbox_semaphore_init(void)
{
    //    Mailbox_Params_init(&protocol_params);
    //    protocol_params.buf = (Ptr)mailboxBuffer_1;
    //    protocol_params.bufSize = sizeof(mailboxBuffer_1);
    //    Mailbox_construct(&Protocol_Struct, sizeof(MsgObj), NUMMSGS, &protocol_params, NULL);
    //    Protocol_Mailbox = Mailbox_handle(&Protocol_Struct);

        Mailbox_Params_init(&rf_rx_timeout_params);
        rf_rx_timeout_params.buf = (Ptr)mailboxBuffer_2;
        rf_rx_timeout_params.bufSize = sizeof(mailboxBuffer_2);
        Mailbox_construct(&rf_rx_timeout_Struct, sizeof(MsgObj), 5, &rf_rx_timeout_params, NULL);
        rf_rx_timeout_mailbox = Mailbox_handle(&rf_rx_timeout_Struct);

    //    Semaphore_Params_init(&led_semparam);
    //    led_semparam.mode = ti_sysbios_knl_Semaphore_Mode_BINARY;
    //    Semaphore_construct(&led_semStruct, 0, &led_semparam);
    //    led_flash_sem = Semaphore_handle(&led_semStruct);
    //
    //    Semaphore_Params_init(&led_off_semparam);
    //    led_off_semparam.mode = ti_sysbios_knl_Semaphore_Mode_BINARY;
    //    Semaphore_construct(&led_off_semStruct, 0, &led_off_semparam);
    //    led_off_sem = Semaphore_handle(&led_off_semStruct);
    //
    //    Semaphore_Params_init(&reed_semparam);
    //    reed_semparam.mode = ti_sysbios_knl_Semaphore_Mode_BINARY;
    //    Semaphore_construct(&reed_semStruct, 0, &reed_semparam);
    //    reed_sem = Semaphore_handle(&reed_semStruct);
    //
    //    Event_Params_init(&protocol_eventParam);
    //    Event_construct(&protocol_eventStruct, &protocol_eventParam);
    //    protocol_eventHandle = Event_handle(&protocol_eventStruct);

}
#define AP_ID   0x52567854
uint8_t buf1[26] = {1,2,3,4,5,6,7,8,9,0};
uint8_t buf[26] = {1,2,3,4,5,6,7,8,9,0};
void send(void){
    uint8_t  count = 5;
    set_rf_parameters(DATA_RATE_100K, RF_TX_POWER_0DB , 2400+(2/2), 2%2);

    while(count)
    {
        RF_EventMask result = Rf_tx_package(rfHandle, AP_ID, 26, buf1);
        if(!(result & RF_EventLastCmdDone))
        {
            while(1);
        }
        count--;
    }
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    /* 1 second delay */
    uint32_t time = 1;
    bool xStatus;
    uint8_t* packetDataPointer;
    MsgObj  rf_get_msg;
    /* Call driver init functions */
    GPIO_init();
    // I2C_init();
    // SPI_init();
    // UART_init();
    // Watchdog_init();
    /* Turn on user LED */
    //   GPIO_write(Board_GPIO_LED0, Board_GPIO_LED_ON);
    init_nvs_spi_flash();
    while (1) {

        send();

        set_rf_parameters(DATA_RATE_100K, RF_TX_POWER_0DB , 2400+(2/2), 2%2);
        Rf_rx_package(rfHandle, &dataQueue, AP_ID, 26, TRUE , (4 * 100000));
        xStatus =  Mailbox_pend (rf_rx_timeout_mailbox, &rf_get_msg, (4 * 100000));
        RF_yield(rfHandle);    //使射频进入sleep状态
        if(xStatus == TRUE)
        {
            if(rf_get_msg.val == RF_RX_DONE)
            {
                currentDataEntry = RFQueue_getDataEntry();
                packetDataPointer = (uint8_t*)(&currentDataEntry->data);
                memcpy(buf, packetDataPointer, 26);
                RFQueue_nextEntry();
            }
        }


    }
}

