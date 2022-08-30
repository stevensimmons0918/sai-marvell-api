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

MV_BOOL enableUart = MV_TRUE;
MV_U32 CLIUartBaud;
MV_U32 CLIUartPort;

/* static variables */
static MV_U8 *uartRxBase = NULL, *uartTxBase = NULL;
static MV_U32 uartRxOffset = 0, uartTxOffset = 0, uartRingSize = 0;

/*******************************************************************************
* mvShmUartInit - Init a shared memory uart port.
*
* DESCRIPTION:
*       This routine Initialize one of the uarts ports (channels).
*       It initialize the baudrate, stop bit,parity bit etc.
*
* INPUT:
*       base - Absolute base address of buffer rings.
*       size - size allocated for each ring (identical).
*
* OUTPUT:
*       None.
*
* RETURN:
*      None.
*
*******************************************************************************/
MV_VOID mvShmUartInit(MV_U8 *base, MV_U32 size)
{

	MV_U32 i;

	uartRingSize = size;
    uartTxBase = base;
    uartRxBase = base + uartRingSize;

    for (i = 0; i < size * 2; i++)
		base[i] = 0;
	uartRxOffset = 0;
	uartTxOffset = 0;

	CLIUartPort = 0;

	return;
}

/*******************************************************************************
* mvShmUartPutc - Send char to the uart port.
*
* DESCRIPTION:
*       This routine puts one charachetr on one of the uart ports.
*
* INPUT:
*       port - uart port number - ignored.
*       c - character.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvShmUartPutc( MV_U32 port, MV_U8 c )
{
	if (enableUart == MV_TRUE)
		if ( (c != 0) && (*(uartTxBase + uartTxOffset) == 0) ) {
			*(uartTxBase + uartTxOffset) = c;
			uartTxOffset++;
			if (uartTxOffset >= uartRingSize)
				uartTxOffset = 0;
		}

	return;
}

/*******************************************************************************
* mvShmUartGetc - Get char from uart port.
*
* DESCRIPTION:
*       This routine gets one charachetr from one of the uart ports.
*
* INPUT:
*       port - uart port number - ignored
*
* OUTPUT:
*       None.
*
* RETURN:
*       carachter from the uart port.
*
*******************************************************************************/
MV_U8 mvShmUartGetc( MV_U32 port )
{
	char c = 0;

	if (enableUart == MV_TRUE)
		c = *(uartRxBase + uartRxOffset);
		if (c != 0) {
            *(uartRxBase + uartRxOffset) = 0;
			uartRxOffset++;
			if (uartRxOffset >= uartRingSize)
				uartRxOffset = 0;
		}

	return c;
}

/*******************************************************************************
* mvShmUartTstc - test for char in uart port.
*
* DESCRIPTION:
*       This routine checks if a char is ready to be read from the uart port.
*
* INPUT:
*       port - uart port number - ignored
*
* OUTPUT:
*       None.
*
* RETURN:
*      MV_TRUE if char ready.
*
*******************************************************************************/


MV_BOOL mvShmUartTstc( MV_U32 port )
{
    if (enableUart == MV_TRUE)
		if (*(uartRxBase + uartRxOffset) != 0)
			return MV_TRUE;
	return MV_FALSE;
}
