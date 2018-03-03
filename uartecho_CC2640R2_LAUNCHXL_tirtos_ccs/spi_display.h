/*
 * spi_display.h
 *
 *  Created on: 2018Äê2ÔÂ27ÈÕ
 *      Author: hanshow
 */

#ifndef SPI_DISPLAY_H_
#define SPI_DISPLAY_H_

#include <ti/drivers/SPI.h>

//#define MASTER
#define MYTEST
#define LOG_SIZE    256

void spi_init();
SPI_Handle spi_open();
int spi_write( uint8_t *buf, int len );
int spi_read( uint8_t *buf, int len );
void spi_close();
void log_print(const char *fmt, ...);
void log_print_bin(uint8_t *data,int len);

#endif /* SPI_DISPLAY_H_ */
