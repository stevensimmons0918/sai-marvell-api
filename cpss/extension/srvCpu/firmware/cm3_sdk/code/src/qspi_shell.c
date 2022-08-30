#include <hw.h>
#include <FreeRTOS_CLI.h>
#include <printf.h>
#include <string.h>
#include "qspi_shell.h"
#include <mv_services.h>
#include <mv_qspi.h>

#define DELIMETERS   " "
#define isdigit(_c)  (((_c)>='0' && (_c)<='9')?1:0)
#define AC5_MPP_MUX(n) (0x80020100 + 4 * n) /* Each register controls 8 pins */

/*******************************************************************************
* xQspiInitialCommand -  This routine initial QSPI commands
*******************************************************************************/
static portBASE_TYPE xQspiInitialCommand( int8_t *pcWriteBuffer,
                                       size_t xWriteBufferLen,
                                       const int8_t *pcCommandString )
{
    MV_U32 baudRate = 0, mode = 0, ch = 0;
    enum SRV_CPU_SPI_MODE spiMode;
    MV_STATUS rc;
    char *value;

    value = strtok( ( char * )&pcCommandString[0], DELIMETERS );

    /* Get the command type. */
    if ( (value == NULL) || ( strcmp(value, "qspi_init") ) )
        goto parseQspiInitCommand_fail;

    /* Channel */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseQspiInitCommand_fail;
    ch = (MV_U32)strtoul(value, &value, 0);
    if (ch > 1)
        goto parseQspiInitCommand_fail;

    /*Baud rate*/
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseQspiInitCommand_fail;
    baudRate = (MV_U32)strtoul(value, &value, 0);

    /* Mode */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseQspiInitCommand_fail;
    mode = (MV_U32)strtoul(value, &value, 0);
    if (mode > 3)
        goto parseQspiInitCommand_fail;
    spiMode = (enum SRV_CPU_SPI_MODE)mode;

    /* Configure MPPs for QSPI */
    if(ch == 0) {
        srvCpuRegWrite(AC5_MPP_MUX(1), (srvCpuRegRead(AC5_MPP_MUX(1)) & 0x0000ffff) | 0x11110000);
        srvCpuRegWrite(AC5_MPP_MUX(2), (srvCpuRegRead(AC5_MPP_MUX(2)) & 0xffffff00) | 0x00000011);
    }
    else
        srvCpuRegWrite(AC5_MPP_MUX(2), (srvCpuRegRead(AC5_MPP_MUX(2)) & 0xffff00ff) | 0x33330011);

    rc = srvCpuQspiInit(ch, 0x805a0000+ch*0x8000, baudRate, spiMode);
    if (rc != 0)
        printf("QSPI init failed\n");
    else
        printf("QSPI initialized successfully  with ch %d, baudRate %d, mode %d \n", ch, baudRate, mode);

    return pdFALSE;
parseQspiInitCommand_fail:
    printf( "error: Wrong input. Usage:\nqspi_init ch baudrate mode\n");
    return pdFALSE;
}

/*******************************************************************************
* xQspiReadWriteCommand -  This routine read/write from QSPI commands
*******************************************************************************/
static portBASE_TYPE xQspiReadWriteCommand( int8_t *pcWriteBuffer,
                                       size_t xWriteBufferLen,
                                       const int8_t *pcCommandString )
{
    MV_U8 cmd = 0, addr_cycles = 0, dummy = 0, buf[32] = {0};
    MV_U32 addr = 0, size = 0, ch = 0, i;
    MV_STATUS rc;
    enum {QSPI_CMD_TYPE_READ, QSPI_CMD_TYPE_WRITE} qspi_cmd_type;
    char *value;

    value = strtok( ( char * )&pcCommandString[0], DELIMETERS );
    /* Get the command type. */
    if ( (value == NULL) || ( strcmp(value, "qspi") ) )
        goto parseQspiCommand_fail;

    /* Get the command type. */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseQspiCommand_fail;
    if (!strcmp(value, "w"))
        qspi_cmd_type = QSPI_CMD_TYPE_WRITE;
    else if (!strcmp(value, "r"))
        qspi_cmd_type = QSPI_CMD_TYPE_READ;
    else
        goto parseQspiCommand_fail;

    /* Channel */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseQspiCommand_fail;
    ch = (MV_U32)strtoul(value, &value, 0);
    if (ch > 1)
        goto parseQspiCommand_fail;

    /* Command */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseQspiCommand_fail;
    cmd = (MV_U8)strtoul(value, &value, 0);

    /* Address */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL)
        goto parseQspiCommand_fail;
    addr = (MV_U32)strtoul(value, &value, 0);

    /* Address cycles */
    value = strtok( NULL, DELIMETERS );
    if ((value == NULL) || (!isdigit(value[0])))
        goto parseQspiCommand_fail;
    addr_cycles = (MV_U8)strtoul(value, &value, 0);

    /* dummy */
    value = strtok( NULL, DELIMETERS );
    if ((value == NULL) || (!isdigit(value[0])))
        goto parseQspiCommand_fail;
    dummy = strtoul(value, &value, 0);

    /* Size / buffer  */
    value = strtok(NULL, DELIMETERS);
    if (qspi_cmd_type == QSPI_CMD_TYPE_READ) {
    /* Amount of bytes to read */
        if (value == NULL)
            goto parseQspiCommand_fail;
        size = (MV_U32)strtoul(value, &value, 0);
        rc = srvCpuQspiRead(ch, cmd, addr, addr_cycles, dummy, buf, size);
        if (rc != 0)
            printf("read failed\n");
        else {
            printf("read: ch %d, cmd %02x, addr %x, add_cyc %d, dummy %d, size %d\n", ch, cmd, addr, addr_cycles, dummy, size);
            for(i = 0; i < size; i++)
                printf("%02x ", buf[i]);
            printf("\n");
        }
    }
    else { /* qspi_cmd_type == QSPI_CMD_TYPE_WRITE */
        /* Populate the buffer */
        while (value != NULL) {
            buf[size++] = (MV_U8)strtoul(value, &value, 16);
            value = strtok(NULL, DELIMETERS);
        }
        rc = srvCpuQspiWrite(ch, cmd, addr, addr_cycles, dummy, buf, size);
        if (rc != 0)
            printf("write failed\n");
        else
            printf("write: ch %d, cmd %02x, addr %x, add_cyc %d, dummy %d, buf %d\n",ch, cmd, addr, addr_cycles, dummy, buf);
    }

    return pdFALSE;
parseQspiCommand_fail:
    printf( "error: Wrong input. Usage:\n qspi r/w ch cmd addr addr_cycles dummy size/buf\n" );
    return pdFALSE;
}

static const CLI_Command_Definition_t xQspiInitCommand =
{
    ( const int8_t * const ) "qspi_init",
    ( const int8_t * const ) "qspi_init <ch> <baudrate> <mode>\n"
                        " Initialize the QSPI interface for single pin.\n\n",
    xQspiInitialCommand,
    -1
};

static const CLI_Command_Definition_t xQspiRdWrtCommand =
{
    ( const int8_t * const ) "qspi",
    ( const int8_t * const ) "qspi r/w <ch> <cmd> <addr> <addr_cycles> <dummy> <size>/<bytes> (xx xx xx in hex) \n"
                             " Read / Write command with offset & dummy bytes, using FIFO\n\n",
    xQspiReadWriteCommand,
    -1
};

void regiserQspiCommands( void )
{
    FreeRTOS_CLIRegisterCommand( &xQspiInitCommand );
    FreeRTOS_CLIRegisterCommand( &xQspiRdWrtCommand );
}

