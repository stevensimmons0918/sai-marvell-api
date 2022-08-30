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

#ifndef __INCmvUarth
#define __INCmvUarth

#include "common.h"

#define MV_UART_MAX_CHAN			( 1 )

/* 6 MHz is the minimum possible SPI frequency when tclk is set 166MHz*/
#define MV_SFLASH_BASIC_SPI_FREQ	10000000 /* Haim - for 200MHz */

/* Global variables */
extern long CLIUartPort;
extern unsigned long CLIUartBaud;

/* API
	MV_VOID mvUartPutc( MV_U32 port, MV_U8 c );
	MV_U8 mvUartGetc( MV_U32 port );
	MV_BOOL mvUartTstc( MV_U32 port );
	MV_VOID mvUartInit( MV_U32 port, MV_U32 baudDivisor, MV_UART_PORT *base );
*/
long spi2uart_init ( unsigned long );
long spi2uart_putc( long );
long spi2uart_getc( void );
long spi2uart_tstc_rx( void );
long spi2uart_tstc_tx( void );

#define mvUartTstc( port )		spi2uart_tstc_rx()
#define mvUartPutc( port, c )	spi2uart_putc(( long )c )
#define mvUartGetc( port )		spi2uart_getc()

#endif /* __INCmvUarth */
