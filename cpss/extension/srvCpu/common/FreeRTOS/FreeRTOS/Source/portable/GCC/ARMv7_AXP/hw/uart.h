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

#define MV_UART_MAX_CHAN			( 4 )
#define MV_UART_REGS_OFFSET( port )	( 0x12000 | 0x100 * ( port ))
#define MV_UART_FIFO_SIZE			( 16 )

#define mvUartBase( port )		\
		(( MV_UART_PORT * )( INTER_REGS_BASE | MV_UART_REGS_OFFSET( port )))
#define mvUartReadReg( base, reg )	\
		( *(( volatile MV_U8 * )(( MV_U32 )( base ) | ( reg ))))
#define mvUartWriteReg( base, reg, val )	\
		( *(( volatile MV_U8 * )(( MV_U32 )( base ) | ( reg )))) = ( val )

/* This structure describes the registers offsets for one UART port(channel) */
	typedef struct mvUartPort {
		MV_U8 rbr;	/* 0 = 0-3 */
		MV_U8 pad1[ 3 ];

		MV_U8 ier;	/* 1 = 4-7 */
		MV_U8 pad2[ 3 ];

		MV_U8 fcr;	/* 2 = 8-b */
		MV_U8 pad3[ 3 ];

		MV_U8 lcr;	/* 3 = c-f */
		MV_U8 pad4[ 3 ];

		MV_U8 mcr;	/* 4 = 10-13 */
		MV_U8 pad5[ 3 ];

		MV_U8 lsr;	/* 5 = 14-17 */
		MV_U8 pad6[ 3 ];

		MV_U8 msr;	/* 6 =18-1b */
		MV_U8 pad7[ 3 ];

		MV_U8 scr;	/* 7 =1c-1f */
		MV_U8 pad8[ 3 ];
	} MV_UART_PORT;

#define REG_USR				( 0x7C )
#define REG_FAR				( 0x70 )
#define REG_HTX				( 0xA4 )
#define REG_EXT_CTRL		( 0x10700 )

/* aliases - for registers which has the same offsets */
#define thr rbr
#define iir fcr
#define dll rbr
#define dlm ier

/* registers feilds */
#define FCR_FIFO_EN			BIT0	/* fifo enable */
#define FCR_RXSR			BIT1	/* reciever soft reset */
#define FCR_TXSR			BIT2	/* transmitter soft reset */

#define MCR_RTS				BIT1	/* ready to send */
#define MCR_AFCE			BIT5	/* Auto Flow Control Enable */

#define LCR_WLS_OFFS		0
#define LCR_WLS_MASK		( 0x3 << LCR_WLS_OFFS )	/* character length mask  */
#define LCR_WLS_5			( 0x0 << LCR_WLS_OFFS )	/* 5 bit character length */
#define LCR_WLS_6			( 0x1 << LCR_WLS_OFFS )	/* 6 bit character length */
#define LCR_WLS_7			( 0x2 << LCR_WLS_OFFS )	/* 7 bit character length */
#define LCR_WLS_8			( 0x3 << LCR_WLS_OFFS )	/* 8 bit character length */
#define LCR_STP_OFFS		2
#define LCR_1_STB			( 0x0 << LCR_STP_OFFS )	/* Number of stop Bits */
#define LCR_2_STB			( 0x1 << LCR_STP_OFFS )	/* Number of stop Bits */
#define LCR_PEN				0x8	/* Parity eneble */
#define LCR_PS_OFFS			4
#define LCR_EPS				( 0x1 << LCR_PS_OFFS )	/* Even Parity Select */
#define LCR_OPS				( 0x0 << LCR_PS_OFFS )	/* Odd Parity Select */
#define LCR_SBRK_OFFS		0x6
#define LCR_SBRK			( 0x1 << LCR_SBRK_OFFS )	/* Set Break */
#define LCR_DIVL_OFFS		7
#define LCR_DIVL_EN			( 0x1 << LCR_DIVL_OFFS )	/* Divisior latch enable */

#define LSR_DR				BIT0	/* Data ready */
#define LSR_OE				BIT1	/* Overrun */
#define LSR_PE				BIT2	/* Parity error */
#define LSR_FE				BIT3	/* Framing error */
#define LSR_BI				BIT4	/* Break */
#define LSR_THRE			BIT5	/* Xmit holding register empty */
#define LSR_TEMT			BIT6	/* Xmitter empty */
#define LSR_ERR				BIT7	/* Error */

#define IER_TX_INT_EN		BIT1	/* Enable Tx interrupt */
#define IER_RX_INT_EN		BIT0	/* Enable Rx interrupt */

#define	USR_BUSY			BIT0
#define	USR_RFNE			BIT3
#define USR_TFNF			BIT1

#define EXT_CTRL_DMA_MODE_OFFS ( 8 )

/* useful defaults for LCR*/
#define LCR_8N1				( LCR_WLS_8 | LCR_1_STB )

/* Global variables */
extern long CLIUartPort;
extern unsigned long CLIUartBaud;

/* APIs */
	MV_VOID mvUartPutc( MV_U32 port, MV_U8 c );
	MV_U8 mvUartGetc( MV_U32 port );
	MV_BOOL mvUartTstc( MV_U32 port );
	MV_VOID mvUartInit( MV_U32 port, MV_U32 baudDivisor, MV_UART_PORT *base );

#endif /* __INCmvUarth */
