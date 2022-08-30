/*******************************************************************************
   Copyright (C) Marvell International Ltd. and its affiliates
********************************************************************************
Marvell GNU General Public License FreeRTOS Exception

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the Lesser
General Public License Version 2.1 plus the following FreeRTOS exception.
An independent module is a module which is not derived from or based on
FreeRTOS.
Clause 1:
Linking FreeRTOS statically or dynamically with other modules is making a
combined work based on FreeRTOS. Thus, the terms and conditions of the GNU
General Public License cover the whole combination.
As a special exception, the copyright holder of FreeRTOS gives you permission
to link FreeRTOS with independent modules that communicate with FreeRTOS solely
through the FreeRTOS API interface, regardless of the license terms of these
independent modules, and to copy and distribute the resulting combined work
under terms of your choice, provided that:
1. Every copy of the combined work is accompanied by a written statement that
details to the recipient the version of FreeRTOS used and an offer by yourself
to provide the FreeRTOS source code (including any modifications you may have
made) should the recipient request it.
2. The combined work is not itself an RTOS, scheduler, kernel or related
product.
3. The independent modules add significant and primary functionality to
FreeRTOS and do not merely extend the existing functionality already present in
FreeRTOS.
Clause 2:
FreeRTOS may not be used for any competitive or comparative purpose, including
the publication of any form of run time or compile time metric, without the
express permission of Real Time Engineers Ltd. (this is the norm within the
industry and is intended to ensure information accuracy).
*******************************************************************************/

#include "FreeRTOS.h"
#include "hw.h"
#include "uart.h"
#include <printf.h>

#define mvSemaLock(x)
#define mvSemaUnlock(x)

#ifdef CONFIG_A8K
long        CLIUartPort = 1;
#else
long        CLIUartPort = 0;
#endif

unsigned long   CLIUartBaud = 115200;

/* static variables */
static volatile MV_UART_PORT * uartBase[ MV_UART_MAX_CHAN ];

/*******************************************************************************
* mvUartInit - Init a uart port.
*
* DESCRIPTION:
*       This routine Initialize one of the uarts ports (channels).
*   It initialize the baudrate, stop bit,parity bit etc.
*
* INPUT:
*       port - uart port number.
*   baudDivisor - baud divisior to use for the uart port.
*
* OUTPUT:
*       None.
*
* RETURN:
*   None.
*
*******************************************************************************/
MV_VOID mvUartInit( MV_U32 port, MV_U32 baudDivisor, MV_UART_PORT *base )
{
    volatile MV_UART_PORT *pUartPort;
    unsigned long i;

    uartBase[ port ] = pUartPort = ( volatile MV_UART_PORT * )base;

    mvSemaLock( MV_SEMA_UART );

    pUartPort->ier = 0x00;
    pUartPort->lcr = LCR_DIVL_EN | LCR_8N1; /* Access baud rate */
    if (pUartPort->dll == 0) {
        pUartPort->dll = baudDivisor & 0xff;
        pUartPort->dlm = ( baudDivisor >> 8 ) & 0xff;
    }
    pUartPort->lcr = LCR_8N1;   /* 8 data, 1 stop, no parity */
    pUartPort->mcr = 0x00;

    MSS_REG_BIT_RESET( REG_EXT_CTRL, 1 << ( EXT_CTRL_DMA_MODE_OFFS + port ) );
    mvUartWriteReg( base, REG_FAR, 0x00 );
    mvUartWriteReg( base, REG_HTX, 0x00 );

    /* Clear & enable FIFOs */
    pUartPort->fcr = FCR_FIFO_EN | FCR_RXSR | FCR_TXSR;
    for ( i = 0; i < MV_UART_FIFO_SIZE; i++ )
        pUartPort->rbr;

    /* Enable Rx/Tx interrupts */
    pUartPort->ier = IER_RX_INT_EN;

    mvSemaUnlock( MV_SEMA_UART );

    return;
}


MV_VOID mvUartPortInit(MV_U32 port, MV_UART_PORT *base )
{
    uartBase[ port ] = ( volatile MV_UART_PORT * )base;
}


/*******************************************************************************
* mvUartPutc - Send char to the uart port.
*
* DESCRIPTION:
*       This routine puts one charachetr on one of the uart ports.
*
* INPUT:
*       port - uart port number.
*   c - character.
*
* OUTPUT:
*       None.
*
* RETURN:
*   None.
*
*******************************************************************************/
MV_VOID mvUartPutc( MV_U32 port, MV_U8 c )
{
    volatile MV_UART_PORT *pUartPort = uartBase[ port ];
    while (( pUartPort->lsr & LSR_THRE ) == 0 );
    pUartPort->thr = c;
    return;
}


/*-------------------------------------------------------------------
    outputString() -

    Description
        This is the workhorse display function used for both general
        messages as well as debugging messages.

    Parameters
        pszStr - A pointer to the null-terminated string to display.

    Return Value
        None
-------------------------------------------------------------------*/
MV_VOID mvOsOutputString(const char *pszStr)
{
    while (*pszStr != '\0') {
        if ('\n' == *pszStr)
            mvUartPutc(CLIUartPort,'\r');
        mvUartPutc(CLIUartPort,*pszStr++);
    }
}


/*******************************************************************************
* mvUartGetc - Get char from uart port.
*
* DESCRIPTION:
*       This routine gets one charachetr from one of the uart ports.
*
* INPUT:
*       port - uart port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*   carachter from the uart port.
*
*******************************************************************************/
MV_U8 mvUartGetc( MV_U32 port )
{
    volatile MV_UART_PORT *pUartPort = uartBase[ port ];
    while (( pUartPort->lsr & LSR_DR ) == 0 );
    return pUartPort->rbr;
}

/*******************************************************************************
* mvUartTstc - test for char in uart port.
*
* DESCRIPTION:
*       This routine heck if a charachter is ready to be read from one of the
*   the uart ports.
*
* INPUT:
*       port - uart port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*   None.
*
*******************************************************************************/
MV_BOOL mvUartTstc( MV_U32 port )
{
    volatile MV_UART_PORT *pUartPort = uartBase[ port ];

    /* Read USR register to clear BUSY interrupt */
    if (( mvUartReadReg( pUartPort, REG_USR ) & USR_RFNE ) ||
            ( pUartPort->lsr & LSR_DR ))
        return MV_TRUE;
    return MV_FALSE;
}
