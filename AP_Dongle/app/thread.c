/*
 * main.c
 *
 *  Created on: 2018Äê2ÔÂ6ÈÕ
 *      Author: ggg
 */


#include <unistd.h>
#include <stdint.h>
#include <stddef.h>

// #include <ti/drivers/I2C.h>
// #include <ti/drivers/SDSPI.h>
// #include <ti/drivers/SPI.h>
// #include <ti/drivers/UART.h>
// #include <ti/drivers/Watchdog.h>
#include <ti/drivers/Power.h>
#include <ti/sysbios/knl/Swi.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/BIOS.h>
#include <xdc/runtime/Error.h>

#include <ti/drivers/Power/PowerCC26XX.h>
#include <ti/drivers/GPIO.h>

/* Board Header file */
#include "Board.h"
#include "cc2640r2_rf.h"
#include "datatype.h"
#include "event.h"
#include "debug.h"
#include "flash.h"
#include "core.h"
#include "uart.h"
#include "communicate.h"
#include "bsp.h"

#ifdef GOLD_BOARD
const unsigned char APP_VERSION_STRING[] = "rfg-3.2.2-rc5"; //must < 32
#else
const unsigned char APP_VERSION_STRING[] = "rfd-3.2.2-rc5"; //must < 32
#endif

/* Stack size in bytes */
#define TASK0_STACKSIZE   (1024)
#define TASK1_STACKSIZE   300


void *mainThread(void *arg0);
void *communicate2master(void *arg0);

Char task0_Stack[TASK0_STACKSIZE];
Char task1_Stack[TASK1_STACKSIZE];

Task_Struct task0_Struct;
Task_Struct task1_Struct;


#ifdef MY_SWI
    Swi_Handle swi0Handle;
    Swi_Struct swi0Struct;
#else

#endif

void app_init(void)
{
    Task_Params taskParams_0;

#ifdef MY_SWI
    Swi_Params swiParams;
#endif
    Power_setConstraint(PowerCC26XX_SB_VIMS_CACHE_RETAIN);
    Power_setConstraint(PowerCC26XX_NEED_FLASH_IN_IDLE);

    Task_Params_init(&taskParams_0);
    taskParams_0.arg0 = 1000000 / Clock_tickPeriod;
    taskParams_0.stackSize = TASK0_STACKSIZE;
    taskParams_0.stack = &task0_Stack;
    taskParams_0.priority = 1;
    Task_construct(&task0_Struct, (Task_FuncPtr)mainThread, &taskParams_0, NULL);


#ifdef MY_SWI
    Swi_Params_init(&swiParams);
    swiParams.arg0 = 0;
    swiParams.arg1 = 0;
    swiParams.priority = 0;
    swiParams.trigger = 0;
    Swi_construct(&swi0Struct, (Swi_FuncPtr)swi0Fxn, &swiParams, NULL);
    swi0Handle = Swi_handle(&swi0Struct);
#else
    Task_Params_init(&taskParams_0);
    taskParams_0.arg0 = 1000000 / Clock_tickPeriod;
    taskParams_0.stackSize = TASK1_STACKSIZE;
    taskParams_0.stack = &task1_Stack;
    taskParams_0.priority = 2;
    Task_construct(&task1_Struct, (Task_FuncPtr)communicate2master, &taskParams_0, NULL);
#endif

    Event_init();

}
extern void BSP_GPIO_test(void);
void *mainThread(void *arg0)
{
    BSP_DEBUG_IO;
    Board_initSPI();
    Board_initUART();
    Debug_SetLevel(DEBUG_LEVEL_INFO);
//    printf("     \r\n%s.\r\n", APP_VERSION_STRING);
    debug_peripheral_init();
    pinfo("basic init complete.\r\n");
    BSP_DEBUG_IO;
//        while (1) {
//            log_print("spi_write:%02x:%d:%d",1,2,3);
//            //log_print("ab%x",1);
//            //Task_sleep(100000);
//        }


    if(Flash_Init() == 1)
    {
        pinfo("flash init successfully.\r\n");
    }
    else
    {
        Event_Set(EVENT_FLASH_ERR);
        perr("flash init fail.\r\n");
    }

    UART_appInit();
    BSP_DEBUG_IO;

    Core_Init();
    pinfo("core init complete.\r\n");
    pinfo("enter main loop.\r\n");

    Core_Mainloop();

    return 0;
}
void *communicate2master(void *arg0)
{
    communicate_main();
    return 0;
}
