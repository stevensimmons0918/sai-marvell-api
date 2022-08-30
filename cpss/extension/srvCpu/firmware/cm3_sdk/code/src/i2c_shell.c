#include <hw.h>
#include <FreeRTOS_CLI.h>
#include <printf.h>
#include <string.h>
#include "mvTwsi.h"

#define DELIMETERS   " "
#define isdigit(_c)  (((_c)>='0' && (_c)<='9')?1:0)

/*******************************************************************************
* xParseI2cCommand -  This routine parse i2c commands
*******************************************************************************/
static portBASE_TYPE xParseI2cCommand( int8_t *pcWriteBuffer,
                                       size_t xWriteBufferLen,
                                       const int8_t *pcCommandString )
{
    MV_U8 channel, address = 0, width = 0, size = 0, buff[32] = {0};
    MV_U32 offset = 0;
    MV_STATUS rc;
    enum {I2C_CMD_TYPE_READ, I2C_CMD_TYPE_WRITE, I2C_CMD_TYPE_PROBE} i2c_cmd_type = I2C_CMD_TYPE_READ;
    char *value;

    value = strtok( ( char * )&pcCommandString[0], DELIMETERS );

    /* Get the command type. */
    if ( (value == NULL) || ( strcmp(value, "i2c") ) )
        goto parseI2cCommand_fail;

    /* Get the command type. */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseI2cCommand_fail;
    if (!strcmp(value, "probe"))
        i2c_cmd_type = I2C_CMD_TYPE_PROBE;
    else if (!strcmp(value, "w"))
        i2c_cmd_type = I2C_CMD_TYPE_WRITE;
    else if (strcmp(value, "r"))
        goto parseI2cCommand_fail;

    /* Channel */
    value = strtok( NULL, DELIMETERS );
    if (i2c_cmd_type == I2C_CMD_TYPE_PROBE) {
        if (value == NULL)
            channel = 0;
        else if (isdigit(value[0]))
            channel = (MV_U8)strtoul(value, &value, 0);
        else
            goto parseI2cCommand_fail;
        goto parseI2cCommand_probe;
    }

    if ((value == NULL) || (!isdigit(value[0])))
        goto parseI2cCommand_fail;
    channel = (MV_U8)strtoul(value, &value, 0);
    /* Address */
    value = strtok( NULL, DELIMETERS );
    if ((value == NULL) || (!isdigit(value[0])))
        goto parseI2cCommand_fail;
    address = (MV_U8)strtoul(value, &value, 0);
    /* width */
    value = strtok( NULL, DELIMETERS );
    if ((value == NULL) || (!isdigit(value[0])))
        goto parseI2cCommand_fail;
    width = (MV_U8)strtoul(value, &value, 0);
    /* offset */
    value = strtok( NULL, DELIMETERS );
    if ((value == NULL) || (!isdigit(value[0])))
        goto parseI2cCommand_fail;
    offset = strtoul(value, &value, 0);

parseI2cCommand_probe:
    if (i2c_cmd_type == I2C_CMD_TYPE_PROBE) {
        buff[0] = 0;
        printf("Devices on channel %d:", channel);
        for (offset = 1; offset < 0x80; offset++) {
            address = offset;
            if (MV_OK == srvCpuI2cWrite( channel, address, offset, width, buff, 1 ))
                printf(" %02x", offset);
        }
    } else {

        value = strtok(NULL, DELIMETERS);
        if (i2c_cmd_type == I2C_CMD_TYPE_READ) {
            /* Amount of bytes to read */
            if ((value == NULL) || (!isdigit(value[0])))
                goto parseI2cCommand_fail;
            size = (MV_U8)strtoul(value, &value, 0);
            rc = srvCpuI2cRead( channel, address, offset, width, buff, size );
            printf("Read %d bytes from dev 0x%02x, channel %d, offset 0x%04x, width %d:", size, address, channel, offset, width);
        } else {
            /* Populate the buffer */
            while (value != NULL) {
                buff[size++] = (MV_U8)strtoul(value, &value, 16);
                value = strtok(NULL, DELIMETERS);
            }
            rc = srvCpuI2cWrite( channel, address, offset, width, buff, 1 );
            printf("write %d bytes to dev 0x%02x, channel %d, offset 0x%04x, width %d:", size, address, channel, offset, width);
        }
        if ( rc == MV_OK ) {
            width = 0;
            while (width < size)
                printf(" %02x", buff[width++]);
        } else
            printf("Failed");
    }
    printf("\n");

    return pdFALSE;
parseI2cCommand_fail:
    printf( "error: rWrong input. Usage:\ni2c r/w ch dev width offs size / bytes\ni2c probe [channel]\n" );
    return pdFALSE;
}

const CLI_Command_Definition_t xI2cCommand =
{
    ( const int8_t * const ) "i2c",
    ( const int8_t * const ) "i2c probe/r/w ch address offset_width offset size / bytes (xx xx xx in hex)\n"
                             " Probe / Read / Write from / to I2C device\n"
                         "\n",
    xParseI2cCommand,
    -1
};
