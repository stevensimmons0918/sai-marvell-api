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

#ifndef __INCmvSpi2UartH
#define __INCmvSpi2UartH

#define MAX3100_WC		( 3 << 14 )	/* write configuration	*/
#define MAX3100_RC		( 1 << 14 )	/* read configuration	*/
#define MAX3100_WD		( 2 << 14 )	/* write data			*/
#define MAX3100_RD		( 0 << 14 )	/* read data			*/

#define MAX3100_CMD_MASK	( 3 << 14 )

/* configuration register bits */
#define MAX3100_FEN		( 1 << 13 )	/* FIFO enable			*/
#define MAX3100_SHDN	( 1 << 12 )	/* shutdown bit			*/
#define MAX3100_TM		( 1 << 11 )	/* T bit irq mask		*/
#define MAX3100_RM		( 1 << 10 )	/* R bit irq mask		*/
#define MAX3100_PM		( 1 << 9 )	/* P bit irq mask		*/
#define MAX3100_RAM		( 1 << 8 )	/* mask for RA/FE bit	*/
#define MAX3100_IR		( 1 << 7 )	/* IRDA timing mode		*/
#define MAX3100_ST		( 1 << 6 )	/* transmit stop bit	*/
#define MAX3100_PE		( 1 << 5 )	/* parity enable bit	*/
#define MAX3100_L		( 1 << 4 )	/* Length bit			*/
#define MAX3100_B_MASK	( 0x000F )	/* baud rate bits mask	*/
#define MAX3100_B( x )	(( x ) & 0x000F )	/* baud rate select bits */

/* data register bits ( write ) */
#define MAX3100_TE		( 1 << 10 )	/* transmit enable bit ( active low )		*/
#define MAX3100_RTS		( 1 << 9 )	/* request-to-send bit ( inverted ~RTS pin ) */

/* data register bits ( read ) */
#define MAX3100_RA		( 1 << 10 )	/* receiver activity when in shutdown mode */
#define MAX3100_FE		( 1 << 10 )	/* framing error when in normal mode		*/
#define MAX3100_CTS		( 1 << 9 )	/* clear-to-send bit ( inverted ~CTS pin )	*/

/* data register bits ( both directions ) */
#define MAX3100_R		( 1 << 15 )	/* receive bit		*/
#define MAX3100_T		( 1 << 14 )	/* transmit bit		*/
#define MAX3100_P		( 1 << 8 )	/* parity bit		*/
#define MAX3100_D_MASK	( 0x00FF )	/* data bits mask	*/
#define MAX3100_D( x )	(( x ) & 0x00FF )	/* data bits */

/* these definitions are valid only for fOSC = 3.6864MHz */
#define MAX3100_B_230400	MAX3100_B( 0 )
#define MAX3100_B_115200	MAX3100_B( 1 )
#define MAX3100_B_57600		MAX3100_B( 2 )
#define MAX3100_B_38400		MAX3100_B( 9 )
#define MAX3100_B_19200		MAX3100_B( 10 )
#define MAX3100_B_9600		MAX3100_B( 11 )
#define MAX3100_B_4800		MAX3100_B( 12 )
#define MAX3100_B_2400		MAX3100_B( 13 )
#define MAX3100_B_1200		MAX3100_B( 14 )
#define MAX3100_B_600		MAX3100_B( 15 )

#endif				/* __INCmvSpi2UartH */
