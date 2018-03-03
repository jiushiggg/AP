//------------------------------------------------------------------------------
//  Copyright (C) 2011-2016 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//
//  Content        : This file contains the function used to transfer data via
//                   the UART port.
//
//------------------------------------------------------------------------------
#include "bsp_uart.h"
#include <stdbool.h>
#include <ti/devices/cc26x0r2/driverlib/uart.h>
#include <ti/devices/cc26x0r2/driverlib/ioc.h>
#include <ti/devices/cc26x0r2/driverlib/prcm.h>

void UART_flush(void);
//*****************************************************************************
//
//! Sends data over a UART port.
//!
//! \param pui8Data is the buffer containing the data to write out to the UART
//! port.
//! \param ui32Size is the number of bytes provided in pui8Data buffer that will
//! be written out to the UART port.
//!
//! This function sends ui32Size bytes of data from the buffer pointed to by
//! pui8Data via the UART port.
//!
//! \return None
//
//*****************************************************************************
void UART_send(const uint8_t *pui8Data, uint32_t ui32Size)
{
    int iOffset;

    //
    // Transmit the number of bytes requested on the UART port.
    //
    for(iOffset = 0; iOffset < ui32Size; iOffset++)
    {
        //
        // Wait until space is available.
        //
        while(HWREG(UART0_BASE + UART_O_FR) & UART_FR_TXFF)
        {
        }

        //
        // Send the byte.
        //
        HWREG(UART0_BASE + UART_O_DR) = pui8Data[iOffset];
    }

    //
    // Wait for all data to be transmitted by the UART port.
    //
    UART_flush();
}

//*****************************************************************************
//
//! Waits until all data has been transmitted by the UART port.
//!
//! This function waits until all data written to the UART port has been
//! transmitted.
//!
//! \return None
//
//*****************************************************************************
void UART_flush(void)
{
    //
    // Wait for the UART FIFO to empty and then wait for the shifter to get
    // the bytes out the port.
    //
    while(!(HWREG(UART0_BASE + UART_O_FR) & UART_FR_TXFE))
    {
    }

    //
    // Wait for the FIFO to not be busy so that the shifter completes.
    //
    while((HWREG(UART0_BASE + UART_O_FR) & UART_FR_BUSY))
    {
    }
}

//*****************************************************************************
//
//! Receives data over a UART port.
//!
//! \param pui8Buffer is the buffer to read data into from the UART port.
//! \param ui32Size is the number of bytes provided in the pui8Data buffer that
//! should be written with data from the UART port.
//!
//! This function reads back ui32Size bytes of data from the UART port, into the
//! buffer that is pointed to by pui8Data. This function will not return until
//! ui32Size number of bytes have been received.
//!
//! \return None
//
//*****************************************************************************
void UART_receive(uint8_t *pui8Buffer, uint32_t ui32Size)
{
    int iOffset;

    //
    // Receive the number of bytes requested.
    //
    for(iOffset = 0; iOffset < ui32Size; iOffset++)
    {
        //
        // Wait until a char is available from UART.
        //
        while(HWREG(UART0_BASE + UART_O_FR) & UART_FR_RXFE)
        {
        }

        //
        // Now get the char.
        //
        pui8Buffer[iOffset] = (HWREG(UART0_BASE + UART_O_DR));
    }
}

//*****************************************************************************
//
//! Sets up the UART port.
//!
//! \param iProcRatio is a fixed-point representation of the ratio of the
//! processor to UART port data rate. This value is broken down in two parts,
//! a fractional portion and an integer portion. The lower 6 bits are the
//! fractional divisor represented as fract_value/(2^6). Bits 21 through 6 are
//! the integer portion of the divisor.
//!
//! This function configures the UART port for 8 data bits, no parity, one stop
//! bit, and sets the baud rate based on the iProcRatio. The iProcRatio can be
//! easily set by the auto-baud feature. The AutoBaud() function returns a
//! value that can be passed in as the iProcRatio parameter.
//!
//! \return None
//
//*****************************************************************************
#define CPU_FREQ      48000000ul
#define UART_BAND   115200
#define PIN_UART_RXD   IOID_3
#define PIN_UART_TXD    IOID_2

void my_UART_open(void)
{
    /* GPIO power && IIC power domain */
    PRCMPowerDomainOn(PRCM_DOMAIN_PERIPH | PRCM_DOMAIN_SERIAL);
    while (PRCMPowerDomainStatus(PRCM_DOMAIN_PERIPH | PRCM_DOMAIN_SERIAL)
           != PRCM_DOMAIN_POWER_ON);


    /* GPIO power */
    PRCMPeripheralRunEnable(PRCM_PERIPH_GPIO);
    PRCMLoadSet();
    while (!PRCMLoadGet());

    /* UART power */
    PRCMPeripheralRunEnable(PRCM_PERIPH_UART0);
    PRCMLoadSet();
    while (!PRCMLoadGet());


//    // Wait for stable UART0 clock
//    while(!(HWREG(PRCM_BASE + PRCM_O_CLKLOADCTL) & PRCM_CLKLOADCTL_LOAD_DONE))
//    {
//    }

    IOCPinTypeUart(UART0_BASE,  PIN_UART_RXD, PIN_UART_TXD, IOID_UNUSED, IOID_UNUSED);

    UARTConfigSetExpClk (UART0_BASE,  CPU_FREQ, UART_BAND,
                         UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE );
    UARTEnable(UART0_BASE);

}

void my_UART_close(void)
{
    // Power down UART
    PRCMPeripheralRunDisable(PRCM_PERIPH_UART0);
    PRCMLoadSet();
    while (!PRCMLoadGet());

//    PRCMPeripheralRunDisable(PRCM_PERIPH_GPIO);
//    PRCMLoadSet();
//    while (!PRCMLoadGet());

    PRCMPowerDomainOff(PRCM_DOMAIN_SERIAL | PRCM_DOMAIN_PERIPH);
    while (PRCMPowerDomainStatus(PRCM_DOMAIN_SERIAL | PRCM_DOMAIN_PERIPH)
           != PRCM_DOMAIN_POWER_OFF);
}


