/*
    FreeRTOS V7.3.0 - Copyright (C) 2012 Real Time Engineers Ltd.

    FEATURES AND PORTS ARE ADDED TO FREERTOS ALL THE TIME.  PLEASE VISIT
    http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?"                                     *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************


    http://www.FreeRTOS.org - Documentation, training, latest versions, license
    and contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool.

    Real Time Engineers ltd license FreeRTOS to High Integrity Systems, who sell
    the code with commercial support, indemnification, and middleware, under
    the OpenRTOS brand: http://www.OpenRTOS.com.  High Integrity Systems also
    provide a safety engineered and independently SIL3 certified version under
    the SafeRTOS brand: http://www.SafeRTOS.com.
*/

/* Scheduler includes. */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>

/* Demo app includes. */
#include <portmacro.h>
#include <hw.h>
#include <ic.h>
#include <uart.h>
#include <mvTwsi.h>
#include <mv_services.h>
#include <mvGpio.h>
#ifdef QSPI
  #include <qspi_shell.h>
#endif
#include <doorbell.h>
#include <watchdog.h>
#include <xorDma.h>
#include <stdint.h>
#include <string.h>
#include <FreeRTOS_CLI.h>

#include "global.h"
#include "cli.h"
#include "printf.h"

#define SDK_VER     "1.0.3"

#define I2C_INTERFACE   0

#define AC5_MG_BASE_ADDRESS        0x7F900000
#define AC5_CM3_MEMORY_SPACE_SIZE   0x00100000
#define NUM_OF_CM3_IN_AC5           3

MV_U32 AC5_CORE_OFFSET_ADDRESS;
MV_U32 AC5_REG_BASE_ADDRESS;

const CLI_Command_Definition_t xI2cCommand;

/* needed by mvShmUart */
const MV_BOOL enableUart = MV_TRUE;

/* Globals */
enum SRV_CPU_DEVICE_TYPE dev_type; /* Keeps the PP device type (Aldrin / BC3 / Pipe) */

/* Memory Dump and Write commands */
#define GETMEM( x, o )      (*(( volatile unsigned long * )( x + o )))
static portBASE_TYPE prvDumpCommand( int8_t *pcWriteBuffer,
    size_t xWriteBufferLen, const int8_t *pcCommandString )
{
    static unsigned long base, size = 0;
    char * ptr;

    if ( size == 0 ) {
        ptr = ( char * )&pcCommandString[ 2 ];
        base = strtoul(( const char * )ptr, &ptr, 0 );
        size = strtoul(( const char * )ptr, &ptr, 0 );
    }

    ptr = ( char * )pcWriteBuffer;
    sprintf( ptr, "%p: %08x %08x %08x %08x  %08x %08x %08x %08x\n",
            base, GETMEM( base, 0 ), GETMEM( base, 4 ), GETMEM( base, 8 ), GETMEM( base, 12 ),
            GETMEM( base, 16 ), GETMEM( base, 20 ), GETMEM( base, 24 ), GETMEM( base, 28 ));
    base += 32;
    size = ( size >= 32 ? size - 32 : 0 );
    if ( size )
        return pdTRUE;
    else
        return pdFALSE;
}

static portBASE_TYPE prvWriteCommand( int8_t *pcWriteBuffer,
    size_t xWriteBufferLen, const int8_t *pcCommandString )
{
    unsigned long base, value;
    char * ptr;

    ptr = ( char * )&pcCommandString[ 2 ];
    base = strtoul(( const char * )ptr, &ptr, 0 );
    value = strtoul(( const char * )ptr, &ptr, 0 );

    ptr = ( char * )pcWriteBuffer;
    sprintf( ptr, "Write value %p to memory %p.\n", value, base );
    GETMEM( base, 0 ) = value;

    return pdFALSE;
}

static portBASE_TYPE prvRegReadCommand( int8_t *pcWriteBuffer,
    size_t xWriteBufferLen, const int8_t *pcCommandString )
{
    static unsigned long base;
    char * ptr;

    ptr = ( char * )&pcCommandString[ 5 ];
    base = strtoul(( const char * )ptr, &ptr, 0 );

    ptr = ( char * )pcWriteBuffer;
    sprintf( ptr, "%p: %08x \n", base, srvCpuRegRead( base));

    return pdFALSE;
}

static portBASE_TYPE prvRegWriteCommand( int8_t *pcWriteBuffer,
    size_t xWriteBufferLen, const int8_t *pcCommandString )
{
    unsigned long base, value;
    char * ptr;

    ptr = ( char * )&pcCommandString[ 5 ];
    base = strtoul(( const char * )ptr, &ptr, 0 );
    value = strtoul(( const char * )ptr, &ptr, 0 );

    ptr = ( char * )pcWriteBuffer;
    sprintf( ptr, "Write value %p to memory %p.\n", value, base );
    srvCpuRegWrite(base, value);

    return pdFALSE;
}

#if defined SDK_DEBUG && defined WATCHDOG
static portBASE_TYPE prvTriggerWatchdogInterruptCommand( int8_t *pcWriteBuffer,
    size_t xWriteBufferLen, const int8_t *pcCommandString )
{
    /* Read a non-existent memory address to get stuck and trigger WatchDog interrupt */
    srvCpuRegRead( 0x99999999);
    return pdFALSE;
}

static portBASE_TYPE prvStopWatchdogInterruptCommand( int8_t *pcWriteBuffer,
    size_t xWriteBufferLen, const int8_t *pcCommandString )
{
    srvCpuWatchdogStop();
    return pdFALSE;
}

static portBASE_TYPE prvStartWatchdogInterruptCommand( int8_t *pcWriteBuffer,
    size_t xWriteBufferLen, const int8_t *pcCommandString )
{
    srvCpuWatchdogStart();
    return pdFALSE;
}
#endif /* SDK_DEBUG && WATCHDOG */

static const CLI_Command_Definition_t xDumpCommand =
{
    ( const int8_t * const ) "md",
    ( const int8_t * const ) "md <address> <byte count>:\n"
            " Dump memory region to console.\n\n",
    prvDumpCommand,
    -1
};

static const CLI_Command_Definition_t xWriteCommand =
{
    ( const int8_t * const ) "mw",
    ( const int8_t * const ) "mw <address> <value>:\n"
            " Write value to memory.\n\n",
    prvWriteCommand,
    2
};

static const CLI_Command_Definition_t xRegReadCommand =
{
    ( const int8_t * const ) "reg r",
    ( const int8_t * const ) "reg r <address>:\n"
            " Read register.\n\n",
    prvRegReadCommand,
    2
};

static const CLI_Command_Definition_t xRegWriteCommand =
{
    ( const int8_t * const ) "reg w",
    ( const int8_t * const ) "reg w <address> <value>:\n"
            " Write register.\n\n",
    prvRegWriteCommand,
    3
};

#if defined SDK_DEBUG && defined WATCHDOG
static const CLI_Command_Definition_t xTriggerWatchdogInterruptCommand =
{
    ( const int8_t * const ) "trigger-watchdog",
    ( const int8_t * const ) "trigger-watchdog:\n"
            " Generates watchdog interrupt.\n\n",
    prvTriggerWatchdogInterruptCommand,
    0
};

static const CLI_Command_Definition_t xStopWatchdogCommand =
{
    ( const int8_t * const ) "stop-watchdog",
    ( const int8_t * const ) "stop-watchdog:\n"
            " Stop watchdog interrupt.\n\n",
    prvStopWatchdogInterruptCommand,
    0
};

static const CLI_Command_Definition_t xStartWatchdogCommand =
{
    ( const int8_t * const ) "start-watchdog",
    ( const int8_t * const ) "start-watchdog:\n"
            " Start watchdog interrupt.\n\n",
    prvStartWatchdogInterruptCommand,
    0
};
#endif /* SDK_DEBUG && WATCHDOG */

/*-----------------------------------------------------------*/
int main( void )
{
    int     clock_divisor, core_id;

    srvCpuCm3SysmapInit();

    /* setup core ID and adress */
    if (dev_type ==  SRV_CPU_DEVICE_TYPE_AC5_E || dev_type ==  SRV_CPU_DEVICE_TYPE_AC5P_E)
    {
        core_id = MV_MEMIO32_READ(MG_BASE | SRV_CPU_AC5_MG_SOURCE_ID);
        AC5_CORE_OFFSET_ADDRESS = ( (core_id - 1) % NUM_OF_CM3_IN_AC5) * AC5_CM3_MEMORY_SPACE_SIZE ;
        AC5_REG_BASE_ADDRESS = AC5_CORE_OFFSET_ADDRESS + AC5_MG_BASE_ADDRESS;
    }
    /* Setup interrupts */
    vICInit();

    //Initialize CM3 UART
    if (dev_type <= SRV_CPU_DEVICE_TYPE_FALCON_E)
        clock_divisor = ( SRV_CPU_DEFAULT_SYS_TCLK / 16 ) / CLIUartBaud;
    else
        clock_divisor = ( SRV_CPU_AC5_SYS_TCLK / 16 ) / CLIUartBaud;

#ifdef SHM_UART
    clock_divisor++;    /* dummy - avoid compilation error */
    mvShmUartInit((MV_U32*)SHM_UART_BASE, SHM_UART_SIZE);
#else
    mvUartInit(CLIUartPort, clock_divisor, mvUartBase(CLIUartPort));
#endif

    printf( "\n\nFreeRTOS 7,3,0 - cm3 SDK %s, %s\n\n",SDK_VER, __DATE__);

    /* Setup CLI task */
    vCLISetup();

    /* i2c init */
    if (dev_type <= SRV_CPU_DEVICE_TYPE_FALCON_E)
        srvCpuTwsiInit(100000, SRV_CPU_DEFAULT_SYS_TCLK, I2C_INTERFACE);
    else
        srvCpuTwsiInit(100000, SRV_CPU_AC5_SYS_TCLK, I2C_INTERFACE);

    srvCpuIpcInit((void*)(CM3_SRAM_SIZE - _2K));

    cm3_interrupt_init();

#ifdef WATCHDOG
    srvCpuWatchdogInit(1000); /* set 1000 milliseconds (= 1 second) to Watchdog timer expiration */
    srvCpuWatchdogStart();
    /*Create the WD task */
    watchdogCreateTask(50); /* set 50 milliseconds delay in the task, in order to clear Watchdog timer before timer expired */
#endif

    FreeRTOS_CLIRegisterCommand( &xDumpCommand );
    FreeRTOS_CLIRegisterCommand( &xWriteCommand );
    FreeRTOS_CLIRegisterCommand( &xRegReadCommand );
    FreeRTOS_CLIRegisterCommand( &xRegWriteCommand );
    FreeRTOS_CLIRegisterCommand( &xI2cCommand );

#ifdef SDK_DEBUG
    /* Register invoke doorbellinterrupt command */
    regiserInvokeDoorbellCommand();
    #ifdef WATCHDOG /* SDK_DEBUG && WATCHDOG */
    FreeRTOS_CLIRegisterCommand( &xTriggerWatchdogInterruptCommand);
    FreeRTOS_CLIRegisterCommand( &xStopWatchdogCommand);
    FreeRTOS_CLIRegisterCommand( &xStartWatchdogCommand);
    #endif
#endif /* SDK_DEBUG */

    /* Register 'gpio r', 'gpio w' and 'gpio c' commands */
    regiserGpioCliCommands();

#ifdef QSPI
    /* Register 'qspi_init' and 'qspi r/w' commands */
    regiserQspiCommands();
#endif

#ifdef XOR_DMA
    /* Register 'xor_dma' commands */
    regiserXorDmaCommand();
#endif

    /* Start the scheduler */
    vTaskStartScheduler();

    /* Will only get here if there was insufficient heap to start the
    scheduler. */

    return 0;
}

