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

#include "mvSpi2Uart.h"
#include "mvSpiSpec.h"
#include "mvSpi.h"
#include "uart.h"

long			CLIUartPort = 0;
unsigned long	CLIUartBaud = 115200;

static unsigned long get_baud_rate( unsigned long );
#define SPI2UART_RX_MAX_CHARS ( 512 )
static MV_U8 rx_buff[ SPI2UART_RX_MAX_CHARS ];
static long rx_overflow; /* counter */
static long rx_start;
static long rx_chars;

/* Single common function supporting transmission */
static inline MV_STATUS spi2uart_txrx( MV_U16 txData, MV_U16 * pRxData )
{
	MV_STATUS ret;
	MV_U16 rx, cmd;
	long read_idx;

	cmd = txData & MAX3100_CMD_MASK;
	txData = MV_16BIT_BE( txData );

	if (( cmd == MAX3100_RD ) && ( rx_chars > 0 )) {
		read_idx = rx_start - rx_chars;
		rx_chars--;
		if ( read_idx < 0 )
			read_idx += SPI2UART_RX_MAX_CHARS;
		*pRxData = ( MV_U16 )rx_buff[ read_idx ] | MAX3100_R;
		return MV_OK;
	}

	mvSpiCsAssert();
	ret = mvSpi16bitDataTxRx( txData, &rx );
	mvSpiCsDeassert();
	if ( ret != MV_OK )
		return -EPERM;

	rx = MV_16BIT_BE( rx );
	*pRxData = rx;
	if (( cmd == MAX3100_WD ) && ( rx & MAX3100_R )) {
		if ( rx_chars < SPI2UART_RX_MAX_CHARS ) {
			rx_buff[ rx_start++ ] = rx & 0xFF;
			rx_chars++;
			if ( rx_start == SPI2UART_RX_MAX_CHARS )
				rx_start = 0;
		} else
			rx_overflow++;
	}

	return ret;
}

long spi2uart_init( unsigned long uart_baud_rate )
{
	MV_U16 rx = 0;
	MV_STATUS ret;

	ret = mvSpiInit( MV_SFLASH_BASIC_SPI_FREQ );
	if ( ret != MV_OK )
		return -EPERM;

	ret = spi2uart_txrx( MAX3100_WC | get_baud_rate( uart_baud_rate ), &rx );
	if ( ret != MV_OK )
		return -EPERM;

	return 0;
}

long spi2uart_putc( long c )
{
	MV_U16 rx = 0;
	MV_STATUS ret;

	while ( spi2uart_tstc_tx() == 0 )
		;

	ret = spi2uart_txrx( MAX3100_WD | c, &rx );
	if ( ret != MV_OK )
		return EOF;

	return c;
}

long spi2uart_getc( void )
{
	MV_U16 rx = 0;
	MV_STATUS ret;

	do {
		ret = spi2uart_txrx( MAX3100_RD, &rx );
		if ( ret != MV_OK )
			return EOF;
	} while (( rx & MAX3100_R ) == 0 );

	return ( rx & 0xFF );
}

long spi2uart_tstc_rx( void )
{
	MV_U16 rx = 0;
	MV_STATUS ret;

	if ( rx_chars > 0 )
		return 1;

	ret = spi2uart_txrx( MAX3100_RC, &rx );
	if ( ret != MV_OK )
		return -EPERM;

	if ( rx & MAX3100_R )
		return 1;
	return 0;
}

long spi2uart_tstc_tx( void )
{
	MV_U16 rx = 0;
	MV_STATUS ret;

	ret = spi2uart_txrx( MAX3100_RC, &rx );
	if ( ret != MV_OK )
		return -EPERM;

	if ( rx & MAX3100_T )
		return 1;
	return 0;
}

unsigned long get_baud_rate( unsigned long baudrate )
{
	unsigned long wconf;

	switch ( baudrate ) {
	case 1200:
		wconf = MAX3100_B_1200;
		break;
	case 2400:
		wconf = MAX3100_B_2400;
		break;
	case 4800:
		wconf = MAX3100_B_4800;
		break;
	case 9600:
		wconf = MAX3100_B_9600;
		break;
	case 19200:
		wconf = MAX3100_B_19200;
		break;
	case 38400:
		wconf = MAX3100_B_38400;
		break;
	case 57600:
		wconf = MAX3100_B_57600;
		break;
	default:
	case 115200:
		wconf = MAX3100_B_115200;
		break;
	case 230400:
		wconf = MAX3100_B_230400;
		break;
	}

	return wconf;
}

