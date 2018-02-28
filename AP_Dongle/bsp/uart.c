/*
 * uart.c
 *
 *  Created on: 2018Äê2ÔÂ28ÈÕ
 *      Author: ggg
 */

/* Driver Header files */
#include "uart.h"
#include "Board.h"


UART_Handle uart_handle;

void bsp_uart_init(void)
{
    UART_Params uartParams;

    /* Call driver init functions */

    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.readTimeout = 5000;
    uartParams.writeTimeout = 5000;
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = 115200;

    uart_handle = UART_open(Board_UART0, &uartParams);
    if (uart_handle == NULL) {
        /* UART_open() failed */
        while (1);
    }
}
#ifdef UART_TEST
void uart_test(void)
{
    const char  echoPrompt[] = "Echoing characters:\r\n";
    char        input;
    UART_write(uart, echoPrompt, sizeof(echoPrompt));

    /* Loop forever echoing */
    while (1) {
        UART_read(uart, &input, 1);
        UART_write(uart, &input, 1);
    }
}
#endif
