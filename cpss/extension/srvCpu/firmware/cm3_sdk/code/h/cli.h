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

#ifndef CLI_H
#define CLI_H

#include <uart.h>

#define CLI_MAX_CMD		( 160 )
#define CLI_UART_PORT	( CLIUartPort )

#define mvputc( c )				mvUartPutc( CLI_UART_PORT, c )

typedef enum {
	CLI_CMD_NULL = 0,
	CLI_CMD_PRE,
	CLI_CMD_RDY,
	CLI_CMD_END
} CLICmdStat;

typedef struct {
	signed char	command[ CLI_MAX_CMD ];
	long		cmdpos;
	CLICmdStat	cmdrdy;
	long		escchr;
} CLIStateType;

void vCLISetup( void );
void iCLIHandler( void );
MV_STATUS gCLIHandleFileCommand(char * cmd, size_t cmdLen);
void vCLIMain( void *pvParameters );

#endif

