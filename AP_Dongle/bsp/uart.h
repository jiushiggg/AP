/*
 * uart.h
 *
 *  Created on: 2018��2��28��
 *      Author: ggg
 */

#ifndef BSP_UART_H_
#define BSP_UART_H_

#include <ti/drivers/UART.h>

extern UART_Handle uart_handle;
extern void bsp_uart_init(void);

#endif /* BSP_UART_H_ */
