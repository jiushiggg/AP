/*
 * main.c
 *
 *  Created on: 2018Äê2ÔÂ6ÈÕ
 *      Author: ggg
 */

#include <hw_rf.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <ti/drivers/GPIO.h>
// #include <ti/drivers/I2C.h>
// #include <ti/drivers/SDSPI.h>
// #include <ti/drivers/SPI.h>
// #include <ti/drivers/UART.h>
// #include <ti/drivers/Watchdog.h>
#include <ti/drivers/Power.h>

#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>

#include <ti/drivers/Power/PowerCC26XX.h>

/* Board Header file */
#include "Board.h"
#include "datatype.h"
#include "event.h"
#include "debug.h"
#include "flash.h"
#include "core.h"

#ifdef GOLD_BOARD
const unsigned char APP_VERSION_STRING[] = "rfg-3.2.2-rc5"; //must < 32
#else
const unsigned char APP_VERSION_STRING[] = "rfd-3.2.2-rc5"; //must < 32
#endif


void *mainThread(void *arg0)
{

    Debug_SetLevel(DEBUG_LEVEL_INFO);
    printf("     \r\n%s.\r\n", APP_VERSION_STRING);
    pinfo("basic init complete.\r\n");

    Event_Reset();


    if(Flash_Init() == 1)
    {
        pinfo("flash init successfully.\r\n");
    }
    else
    {
        Event_Set(EVENT_FLASH_ERR);
        perr("flash init fail.\r\n");
    }

    Core_Init();
    pinfo("core init complete.\r\n");

    pinfo("enter main loop.\r\n");

    Core_Mainloop();
}


