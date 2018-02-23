/*
 * Copyright (c) 2015-2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== clock.c ========
 */

/* XDC module Headers */
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS module Headers */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/GPIO.h>

/* Example/Board Header files */
#include "Board.h"

Void clk0Fxn(UArg arg0);
Void clk1Fxn(UArg arg0);

Clock_Struct clk0Struct, clk1Struct;
Clock_Handle clk2Handle;

#define TASK0_STACKSIZE   (1024)
Char task0_Stack[TASK0_STACKSIZE];
Task_Struct task0_Struct;


void *mainThread(void *arg0)
{
    UInt32 time;
    while(1){
        time = Clock_getTicks();
        if (time < 500){
            System_printf("System time in clk0Fxn = %lu\n", (ULong)time);
            System_flush();
        }
        Task_sleep(5000/Clock_tickPeriod);
    }

}
/*
 *  ======== main ========
 */
int main()
{
    /* Construct BIOS Objects */
    Clock_Params clkParams;
    Task_Params taskParams_0;

    /* Call driver init functions */
    Board_initGeneral();

    GPIO_init();
    GPIO_write(Board_GPIO_LED0, Board_GPIO_LED_ON);

    Clock_Params_init(&clkParams);
    clkParams.period = 4000000/Clock_tickPeriod;
    clkParams.startFlag = TRUE;

    /* Construct a periodic Clock Instance */
    Clock_construct(&clk0Struct, (Clock_FuncPtr)clk0Fxn,
                    4000000/Clock_tickPeriod, &clkParams);


    Task_Params_init(&taskParams_0);
    taskParams_0.arg0 = 1000000 / Clock_tickPeriod;
    taskParams_0.stackSize = TASK0_STACKSIZE;
    taskParams_0.stack = &task0_Stack;
    taskParams_0.priority = 2;
    Task_construct(&task0_Struct, (Task_FuncPtr)mainThread, &taskParams_0, NULL);


//    clkParams.period = 0;
//    clkParams.startFlag = FALSE;
//
//    /* Construct a one-shot Clock Instance */
//    Clock_construct(&clk1Struct, (Clock_FuncPtr)clk1Fxn,
//                    11000/Clock_tickPeriod, &clkParams);
//
//    clk2Handle = Clock_handle(&clk1Struct);
//
//    Clock_start(clk2Handle);

    BIOS_start();    /* does not return */
    return(0);
}

/*
 *  ======== clk0Fxn =======
 */
Void clk0Fxn(UArg arg0)
{
    UInt32 time;
    GPIO_toggle(Board_GPIO_LED0);
    time = Clock_getTicks();
    if (time < 500){
    System_printf("System time in clk0Fxn = %lu\n", (ULong)time);
    System_flush();
    }
}

/*
 *  ======== clk1Fxn =======
 */
Void clk1Fxn(UArg arg0)
{
    UInt32 time;

    time = Clock_getTicks();
    System_printf("System time in clk1Fxn = %lu\n", (ULong)time);
    System_printf("Calling BIOS_exit() from clk1Fxn\n");
    BIOS_exit(0);
}
