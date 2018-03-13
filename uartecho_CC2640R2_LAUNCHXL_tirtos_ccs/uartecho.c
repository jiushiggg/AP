/*
 * Copyright (c) 2015-2016, Texas Instruments Incorporated
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
 *  ======== uartecho.c ========
 */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include <ti/sysbios/knl/Task.h>

#include "spi_display.h"
#include <ti/devices/cc26x0r2/driverlib/ssi.h>
/* Example/Board Header files */
#include "Board.h"
#include "bsp_uart.h"

#define BUF_SIZE    64
//#define MASTER

UART_Handle uart = NULL;

UART_Handle uart_open()
{
    UART_Params uartParams;
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = 115200;
    uartParams.writeTimeout = 1000/10;
    uart = UART_open(Board_UART0, &uartParams);
    return uart;
}

/*
 *  ======== mainThread ========
 */
#define BUF_LEN 4096*2
uint8_t buf[BUF_LEN] = {0};
volatile uint16_t  send_index = 0;
volatile uint16_t  rec_index = 0;
volatile uint16_t  rec_len = 0;
uint16_t  uart_send_len = 0;


// Callback function
void transferCallback(SPI_Handle handle, SPI_Transaction *transaction)
{
    rec_len = transaction->count;
    memcpy(buf+rec_index, transaction->rxBuf, rec_len);
    rec_index += transaction->count;
    SPI_transfer(handle, transaction);

}

void *mainThread(void *arg0)
{
    const char  echoPrompt[] = "Echoing characters:\r\n";
    /* Call driver init functions */
    GPIO_init();
    SPI_init();
    /* Turn on user LED */
    GPIO_write(Board_GPIO_LED0, Board_GPIO_LED_ON);

    my_UART_open();
    UART_send(echoPrompt, sizeof(echoPrompt));

    if(spi_open() == NULL)
    {
        while(1);
    }

    /* Loop forever echoing */

#ifdef MASTER
    uint8_t buffer[64];
    memset(buffer,1,sizeof(buffer));
    while (1) {
  //      spi_write(tx_buf,LOG_SIZE);
        //log_print("spi_write:%02x:%d:%d",1,2,3);
        log_print("02%x",1);
//        log_print_bin(buffer,sizeof(buffer));
        Task_sleep(100000);
    }
#else

    while (1) {
        while(send_index < rec_index){
            UART_send(buf+send_index, 1);
            send_index++;
        }
        if (send_index == rec_index){
            rec_index=0;
            send_index = 0;
        }
    }
#endif
}

