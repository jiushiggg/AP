/*
 * spi_display.c
 *
 *  Created on: 2018年2月27日
 *      Author: hanshow
 */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/spi/SPICC26XXDMA.h>
#include <ti/sysbios/knl/Task.h>

#include "Board.h"
#include "spi_display.h"

static SPI_Handle handle = NULL;
unsigned char tx_buf[LOG_SIZE];
unsigned char rx_buf[LOG_SIZE];

//引脚配置
//#define CC2640R2_LAUNCHXL_SPI0_MISO             IOID_19          /* RF1.20 */
//#define CC2640R2_LAUNCHXL_SPI0_MOSI             IOID_18          /* RF1.18 */
//#define CC2640R2_LAUNCHXL_SPI0_CLK              IOID_17         /* RF1.16 */
//#define CC2640R2_LAUNCHXL_SPI0_CSN              IOID_16

#ifdef MASTER
SPI_Handle spi_open()
{
    SPI_Params params;
    //PIN_Id misoPinId;

    SPI_Params_init(&params);
    params.bitRate     = 1000000;
    params.frameFormat = SPI_POL1_PHA1;
    params.mode        = SPI_MASTER;
    params.transferMode = SPI_MODE_BLOCKING;
    handle = SPI_open(Board_SPI0, &params);
    //SPI_control(handle, SPICC26XXDMA_RETURN_PARTIAL_ENABLE, NULL);
    // Get pinHandle
    //pinHandle = ((SPICC26XXDMA_Object *)handle->object)->pinHandle;
    // Get miso pin id
    //misoPinId = ((SPICC26XXDMA_HWAttrsV1 *)handle->hwAttrs)->misoPin;
    // Apply low power sleep pull config for MISO
    //PIN_setConfig(pinHandle, PIN_BM_PULLING, PIN_PULLUP | misoPinId);
    return handle;
}

#else
SPI_Handle spi_open()
{
    SPI_Params params;

    // Init SPI and specify non-default parameters
    SPI_Params_init(&params);
    params.bitRate             = 1000000;
    params.frameFormat         = SPI_POL1_PHA1;
    params.mode                = SPI_SLAVE;
    params.transferMode        = SPI_MODE_BLOCKING;

//    if(pinHandle == NULL)
//        pinHandle = PIN_open(&SPICSN_Pin_State, SPICSN_GpioInitTable);

    // Open the SPI and initiate the first transfer
    handle = SPI_open(Board_SPI0, &params);
    //SPI_control(handle, SPICC26XXDMA_RETURN_PARTIAL_ENABLE, NULL);
    return handle;
}
#endif

void spi_init()
{
    SPI_init();
}

int spi_write( uint8_t *buf, int len )
{
    SPI_Transaction masterTransaction;

    masterTransaction.count  = len;
    masterTransaction.txBuf  = (void*)buf;//!< 将要传输的数据存放的地址赋给*txBuf
    masterTransaction.arg    = NULL;
    masterTransaction.rxBuf  = NULL;

    return SPI_transfer(handle, &masterTransaction) ? 1 : 0;//!< 调用SPI_transfer()写入数据
}

int spi_read( uint8_t *buf, int len )
{
    SPI_Transaction masterTransaction;

    masterTransaction.count = len;
    masterTransaction.rxBuf = buf;//!< 将要接收到的数据存入buf所指的地址中
    masterTransaction.txBuf = NULL;
    masterTransaction.arg = NULL;

    return SPI_transfer(handle, &masterTransaction) ? 1 : 0;//!< 调用SPI_transfer()读出数据
}

void spi_close()
{
    SPI_close(handle);
}

void log_print(const char *fmt, ...)
{
    int len = 0;
    int i = 0;
    uint8_t *ptr = tx_buf;
    memset(tx_buf,0,sizeof(tx_buf));
    va_list ap;
    va_start(ap, fmt);
    vsnprintf((char *)tx_buf, LOG_SIZE - 1, fmt, ap);
    va_end(ap);

    len = strlen(tx_buf);
    for(i=0;i<len;i++)
    {
        spi_write(ptr++,1);
        Task_sleep(1000);
    }
    spi_write("\r\n",2);
    Task_sleep(1000);

}

void log_print_bin(uint8_t *data,int len)
{
   uint8_t buf[3];
   uint8_t *ptr = data;
   int i,j;
   for(i=0;i<len;i++)
   {
       sprintf((char *)buf,"%02x ",(char)*ptr++);
       for(j=0;j<3;j++)
       {
           spi_write(buf+j,1);
           Task_sleep(1000);
       }
   }
   spi_write("\r\n",2);
   Task_sleep(1000);
}
