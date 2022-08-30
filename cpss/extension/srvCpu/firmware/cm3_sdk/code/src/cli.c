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
#ifndef DISABLE_CLI
/* Includes */
#include <FreeRTOS.h>
#include <stdint.h>
#include <string.h>
#include <task.h>
#include <semphr.h>
#include <FreeRTOS_CLI.h>
#include <hw.h>
#include "cli.h"
#include "printf.h"

CLIStateType        CLIState;
signed char     CLIOutput[ CLI_MAX_CMD ];
xSemaphoreHandle    CLISemaphore = NULL;

void iCLIHandler( void )
{
    unsigned char i;

    if ( CLIState.cmdrdy == CLI_CMD_END ) {
        CLIState.cmdpos = 0;
        CLIState.cmdrdy = CLI_CMD_NULL;
    }

    while ( mvUartTstc( CLI_UART_PORT ) == 1 ) {
        i = mvUartGetc( CLI_UART_PORT );

        if (( i >= 32 ) && ( i <= 126 )) {
            if ( CLIState.escchr ) {
                if ( !(( i == '[' ) || ( i == 'O' ) ||
                        ( i == ';' ) ||
                        (( i >= '0' ) && ( i <= '9' ))))
                    CLIState.escchr = 0;

                i = 0;
            } else
                mvUartPutc( CLI_UART_PORT, i );
        } else {
            switch ( i ) {
            case 0x8:
            case 0x7f:
                if (( CLIState.cmdpos > 0 ) &&
                        ( CLIState.command[ CLIState.cmdpos - 1 ] > 0 )) {
                    CLIState.cmdpos--;
                    CLIState.command[ CLIState.cmdpos ] = 0;
                    mvUartPutc( CLI_UART_PORT, 0x8 );
                    mvUartPutc( CLI_UART_PORT, ' ' );
                    mvUartPutc( CLI_UART_PORT, 0x8 );
                }
                i = 0;
                break;
            case 0xd:
                mvUartPutc( CLI_UART_PORT, '\n' );
                mvUartPutc( CLI_UART_PORT, '\r' );
                if ( CLIState.cmdrdy == CLI_CMD_NULL ) {
                    CLIState.cmdrdy = CLI_CMD_PRE;
                    i = 0;
                }
                break;
            case 0x1b:
                CLIState.escchr = 1;
                i = 0;
                break;
            default:
                i = 0;
            }
        }

        if (( i > 0 ) && ( CLIState.cmdpos < CLI_MAX_CMD - 1 )) {
            CLIState.command[ CLIState.cmdpos ] = i;
            CLIState.cmdpos++;
        }
    }

    if ( CLIState.cmdrdy == CLI_CMD_PRE ) {
        CLIState.cmdrdy = CLI_CMD_RDY;
        CLIState.command[ CLIState.cmdpos ] = 0;
        CLIState.cmdpos++;

    }

   return;
}

static void prvCLITask( void *pvParameters )
{
    portBASE_TYPE   xMoreDataToFollow;

    while(1) {
        printf( "> " );

        do {
            /* Sleep for 500 us */
            vTaskDelay( configTICK_RATE_HZ / 2000 );

            iCLIHandler();
        } while ( CLIState.cmdrdy != CLI_CMD_RDY );
        /* Repeat until the command does not generate any more output. */
        xMoreDataToFollow = pdTRUE;
        while (( CLIState.command[ 0 ] != 0 ) &&
                ( xMoreDataToFollow != pdFALSE )) {
            CLIOutput[ 0 ] = 0;

            /* Pass the string to FreeRTOS+CLI. */
            xMoreDataToFollow = FreeRTOS_CLIProcessCommand(
                    ( int8_t * )CLIState.command, ( int8_t * )CLIOutput,
                    CLI_MAX_CMD );

            /* Print the output. */
            CLIOutput[ CLI_MAX_CMD - 1 ] = 0;
            printf( ( char * )CLIOutput );
        }

        CLIState.cmdrdy = CLI_CMD_END;
    }

}

void vCLIMain( void *pvParameters )
{
    prvCLITask(pvParameters);
}


static portBASE_TYPE prvHeapCommand( int8_t *pcWriteBuffer,
    size_t xWriteBufferLen, const int8_t *pcCommandString )
{
    printf("Heap remaining space: %d\n\n", xPortGetFreeHeapSize());

    return pdFALSE;
}


static const CLI_Command_Definition_t xHeapCommand =
{
    ( const int8_t * const ) "heap",
    ( const int8_t * const ) "heap:\n Print heap status.\n\n",
    prvHeapCommand,
    0
};

void vCLISetup( void )
{

    /*Create the CLI task in case there is no microinit */
    xTaskCreate( prvCLITask, ( signed char * ) "CLI",
                 configMINIMAL_STACK_SIZE, NULL,
                 tskIDLE_PRIORITY + 1, NULL );

    /* Register 'heap' command */
    FreeRTOS_CLIRegisterCommand( &xHeapCommand );
}

MV_STATUS gCLIHandleFileCommand(char * cmd, size_t cmdLen)
{
    int c = 0;
    size_t lineLength = 0;

    // Restore original command line
    cmd[cmdLen] = 0x20;

    // Ensure null termination and place for end of command line
    cmd[CLI_MAX_CMD-2] = '\0';
    lineLength = strlen(cmd);

    // Put line forward character and end of command line
    cmd[lineLength++] = '\n';
    cmd[lineLength] = '\0';

    // Fill the CLI control sructure
    memcpy(&CLIState.command[0],cmd,lineLength);
    CLIState.command[lineLength] = '\0';
    CLIState.cmdpos = lineLength+1;
    CLIState.cmdrdy = CLI_CMD_RDY;


    // Wait 10 sec for CLI command completion
    do {
        /* Sleep for 1 sec */
        vTaskDelay( configTICK_RATE_HZ );

        if (++c > 10) {
            printf( "error: CLI can't complete the command %s.\n", cmd );
            return MV_ERROR;
        }

    } while (CLI_CMD_END != CLIState.cmdrdy);

    return MV_OK;
}
#endif /* DISABLE_CLI */
