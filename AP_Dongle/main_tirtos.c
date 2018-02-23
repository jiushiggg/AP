/*
 * Copyright (c) 2016-2017, Texas Instruments Incorporated
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
 *  ======== main_tirtos.c ========
 */
#include <stdint.h>

/* POSIX Header files */
#include <pthread.h>

/* RTOS header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>

/* Example/Board Header files */
#include "Board.h"
#include "hw_rf.h"

/* Stack size in bytes */
#define THREADSTACKSIZE    1024
#define TASK0_STACKSIZE   (1024)
#define TASK2_STACKSIZE   1024

static void app_init(void);
extern void *mainThread(void *arg0);


Char task0_Stack[TASK0_STACKSIZE];
//Char task2_Stack[TASK2_STACKSIZE];
Task_Struct task0_Struct;
//Task_Struct task2_Struct;

/*
 *  ======== main ========
 */
int main(void)
{
    Board_initGeneral();
//    Board_initWatchdog();
    rf_init();
    semaphore_init();
    app_init();

    BIOS_start();    /* Start BIOS */
    return (0);

}

void app_init(void)
{
    Task_Params taskParams_0;
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


