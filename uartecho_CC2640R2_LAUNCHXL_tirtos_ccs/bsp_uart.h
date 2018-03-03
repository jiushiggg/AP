/*
 * bsp_uart.h
 *
 *  Created on: 2017Äê11ÔÂ28ÈÕ
 *      Author: ggg
 */

#ifndef BSP_UART_H_
#define BSP_UART_H_

#include <stdint.h>
extern void my_UART_open(void);
extern void my_UART_close(void);
extern void UART_send(const uint8_t *pui8Data, uint32_t ui32Size);
extern void UART_receive(uint8_t *pui8Buffer, uint32_t ui32Size);
extern void UART_flush(void);

#endif /* BSP_UART_H_ */
