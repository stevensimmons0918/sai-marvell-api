/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

    *   Neither the name of Marvell nor the names of its contributors may be
        used to endorse or promote products derived from this software without
        specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/
/**
********************************************************************************
* @file mvGpio.c
*
* @brief This file includes the initialization and function of the GPIO
* 
*
* @version   1
********************************************************************************
*/
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <printf.h>
#include <mvGpio.h>
#include <mv_services.h>
#include <hw.h>
#include <FreeRTOS_CLI.h>

#define DELIMETERS   " "
#define CLR_SET_BIT(a,n,value)    (((MV_U32)value << n) | ( a & ~((MV_U32)1 << n)))

#define GPIO_BASE_ADDR                        0x7F018100
/* n is 0-1 represents GPIO group number */
#define GPIO_DATA_OUT_REG(n)                  GPIO_BASE_ADDR + 0x40 * n
#define GPIO_DATA_OUT_ENABLE_REG(n)           GPIO_BASE_ADDR + 0x04 + 0x40 * n
#define GPIO_BLINK_ENABLE_REG(n)              GPIO_BASE_ADDR + 0x08 + 0x40 * n
#define GPIO_DATA_IN_POLARITY_REG(n)          GPIO_BASE_ADDR + 0x0c + 0x40 * n
#define GPIO_DATA_IN_REG(n)                   GPIO_BASE_ADDR + 0x10 + 0x40 * n

/**
* @internal srvCpuGpioInOutConfig function
* @endinternal
*
* @brief  Configure GPIO pins as INPUT/OUTPUT
*         This field is active low. Data is driven when the corresponding bit value is 0.
*
* @param MV_U32 gpioGrp - GPIO group number (0: 0-31, 1: 32-48)
* @param MV_U32 regBitmap - register in bitmap (when OUT - 0, IN - 1)
*
* @retval MV_TRUE
*
*/
MV_BOOL srvCpuGpioInOutConfig(MV_U32 gpioGrp, MV_U32 regBitmap)
{
    srvCpuRegWrite(GPIO_DATA_OUT_ENABLE_REG(gpioGrp), regBitmap);
    return MV_TRUE;
}

/**
* @internal srvCpuGpioInRead function
* @endinternal
*
* @brief  Read the GPIO pins value
*         Each bit in this field reflects the value of the corresponding GPIO pin.
*
* @param MV_U32 gpioGrp   - GPIO group number (0: 0-31, 1: 32-48)
*
* @retval MV_U32 GPIO pins value
*
*/
MV_U32 srvCpuGpioInRead(MV_U32 gpioGrp)
{
    return srvCpuRegRead(GPIO_DATA_IN_REG(gpioGrp));
}

/**
* @internal srvCpuGpioOutWrite function
* @endinternal
*
* @brief  Write GPIO Output Pins Value
*         One bit for each GPIO pin.
*
* @param MV_U32 gpioGrp - GPIO group number (0: 0-31, 1: 32-48)
* @param MV_U32 regBitmap - register in bitmap
*
* @retval MV_TRUE
*
*/
MV_BOOL srvCpuGpioOutWrite(MV_U32 gpioGrp, MV_U32 regBitmap)
{
    srvCpuRegWrite(GPIO_DATA_OUT_REG(gpioGrp), regBitmap);
    return MV_TRUE;
}


/**
* @internal srvCpuGpioBlinkConfig function
* @endinternal
*
* @brief  set the GPIO pin to blink
*         When set GPIO Blink Enable register and the corresponding bit in GPIO Data Out Enable Control register is enabled, 
*         the GPIO pin blinks every ~100 ms (a period of 2^24 TCLK clocks).
*
* @param MV_U32 gpioGrp - GPIO group number (0: 0-31, 1: 32-48)
* @param MV_U32 regBitmap - register in bitmap to define as data out
*
* @retval MV_TRUE
*
*/
MV_BOOL srvCpuGpioBlinkConfig(MV_U32 gpioGrp, MV_U32 regBitmap)
{
    srvCpuRegWrite(GPIO_DATA_OUT_ENABLE_REG(gpioGrp), regBitmap);
    srvCpuRegWrite(GPIO_BLINK_ENABLE_REG(gpioGrp), regBitmap);
    return MV_TRUE;
}

/**
* @internal srvCpuGpioPolarityConfig function
* @endinternal
*
* @brief  Define GPIO polarity
*         GPIO Data in Active Low.
*         When set to 1, the GPIO Data In register reflects the inverted value of the corresponding pin.
*
* @param MV_U32 gpioGrp - GPIO group number (0: 0-31, 1: 32-48)
* @param MV_U32 polarityInBitmap - register in bitmap - (when 0 - is Normal, 1 - inverted value of  GPIO Data In register)
*
* @retval MV_TRUE
*
*/
MV_BOOL srvCpuGpioPolarityConfig(MV_U32 gpioGrp, MV_U32 polarityInBitmap)
{
    srvCpuRegWrite(GPIO_DATA_IN_POLARITY_REG(gpioGrp), polarityInBitmap);
    return MV_TRUE;
}

/**
* @internal srvCpuGpioPinConfig function
* @endinternal
*
* @brief  Configure GPIO single pin as INPUT/OUTPUT
*         This field is active low. Data is driven when the corresponding bit value is 0.
*
* @param MV_U32 gpioPinNumber - GPIO pin number (0-48)
* @param MV_U32 gpioPinConfig - value (0-1) for single pin (when OUT - 0, IN - 1)
*
* @retval MV_TRUE
*
*/
MV_BOOL srvCpuGpioPinConfig(MV_U32 gpioPinNumber, MV_U32 gpioPinConfig)
{
	MV_U32 temp = 0, gpioGrp = gpioPinNumber < 32 ? 0 : 1;
	temp = srvCpuRegRead(GPIO_DATA_OUT_ENABLE_REG(gpioGrp));
	temp = CLR_SET_BIT(temp, gpioPinNumber % 32, gpioPinConfig);
	srvCpuRegWrite(GPIO_DATA_OUT_ENABLE_REG(gpioGrp), temp);

	return MV_TRUE;
}

/**
* @internal srvCpuGpioPinRead function
* @endinternal
*
* @brief  Read GPIO single pin
*
* @param MV_U32 gpioPinNumber - GPIO pin number (0-48)
*
* @retval MV_U32 GPIO pin value (0-1)
*
*/
MV_U32 srvCpuGpioPinRead(MV_U32 gpioPinNumber)
{
	MV_U32 gpioGrp = gpioPinNumber < 32 ? 0 : 1;
	return (srvCpuRegRead(GPIO_DATA_IN_REG(gpioGrp)) & ((MV_U32)1<<(gpioPinNumber % 32)))? 1 : 0;
}

/**
* @internal srvCpuGpioPinWrite function
* @endinternal
*
* @brief  Write one bit for GPIO single pin
*
* @param MV_U32 gpioPinNumber - GPIO pin number (0-48)
* @param MV_U32 gpioPinValue -  value (0-1) for single pin
*
* @retval MV_TRUE
*
*/
MV_BOOL srvCpuGpioPinWrite(MV_U32 gpioPinNumber, MV_U32 gpioPinValue)
{
	MV_U32 temp = 0, gpioGrp = gpioPinNumber < 32 ? 0 : 1;
	temp = srvCpuRegRead(GPIO_DATA_OUT_REG(gpioGrp));
	temp = CLR_SET_BIT(temp, gpioPinNumber % 32, gpioPinValue);
	srvCpuRegWrite(GPIO_DATA_OUT_REG(gpioGrp), temp);

	return MV_TRUE;
}

/**
* @internal srvCpuGpioPinBlinkConfig function
* @endinternal
*
* @brief  set the GPIO single pin to blink
*         When set GPIO Blink Enable register and the corresponding bit in GPIO Data Out Enable Control register is enabled, 
*         the GPIO pin blinks every ~100 ms (a period of 2^24 TCLK clocks).
*
* @param MV_U32 gpioPinNumber - GPIO pin number (0-48)
* @param MV_U32 gpioPinBlinkValue -  value (0-1) for single pin to define as data out
*
* @retval MV_TRUE
*
*/
MV_BOOL srvCpuGpioPinBlinkConfig(MV_U32 gpioPinNumber, MV_U32 gpioPinBlinkValue)
{
	MV_U32 temp = 0, gpioGrp = gpioPinNumber < 32 ? 0 : 1;
	/*First, set GPIO pin Data out Enable*/
	temp = srvCpuRegRead(GPIO_DATA_OUT_ENABLE_REG(gpioGrp));
	temp = CLR_SET_BIT(temp, gpioPinNumber % 32, gpioPinBlinkValue);
	srvCpuRegWrite(GPIO_DATA_OUT_ENABLE_REG(gpioGrp), temp);
	/*set GPIO pin Blink Enable */
	temp = srvCpuRegRead(GPIO_BLINK_ENABLE_REG(gpioGrp));
	temp = CLR_SET_BIT(temp, gpioPinNumber % 32, gpioPinBlinkValue);
	srvCpuRegWrite(GPIO_BLINK_ENABLE_REG(gpioGrp), temp);

	return MV_TRUE;
}

/**
* @internal srvCpuGpioPinPolarityConfig function
* @endinternal
*
* @brief  Define GPIO single pin polarity
*         GPIO Data in Active Low.
*         When set to 1, the GPIO Data In register reflects the inverted value of the corresponding pin.
*
* @param MV_U32 gpioPinNumber - GPIO pin number (0-48)
* @param MV_U32 gpioPinPolarityValue -  value (0-1) for single pin(when 0 - is Normal, 1 - inverted value of GPIO Data In register)
*
* @retval MV_TRUE
*
*/
MV_BOOL srvCpuGpioPinPolarityConfig(MV_U32 gpioPinNumber, MV_U32 gpioPinPolarityValue)
{
	MV_U32 temp = 0, gpioGrp = gpioPinNumber < 32 ? 0 : 1;
	temp = srvCpuRegRead(GPIO_DATA_IN_POLARITY_REG(gpioGrp));
	temp = CLR_SET_BIT(temp, gpioPinNumber % 32, gpioPinPolarityValue);
	srvCpuRegWrite(GPIO_DATA_IN_POLARITY_REG(gpioGrp), temp);

	return MV_TRUE;
}
/************************************************************************************
* prvGpioGrpParseCommand -  This routine parse read/write/config gpio group commands
************************************************************************************/
static portBASE_TYPE prvGpioGrpParseCommand( int8_t *pcWriteBuffer,
                                       size_t xWriteBufferLen,
                                       const int8_t *pcCommandString )
{

    MV_U32 gpioGrp = 0, valueToWrite = 0;
    enum {GPIO_CMD_TYPE_READ, GPIO_CMD_TYPE_WRITE, GPIO_CMD_TYPE_CONFIG} gpio_cmd_type;
    char *value;

    value = strtok( ( char * )&pcCommandString[0], DELIMETERS );
    /* Get the command type. */
    if ( (value == NULL) || ( strcmp(value, "gpio_grp") ) )
        goto parseGpioCommand_fail;

    /* Get the command type: r/w/c. */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseGpioCommand_fail;
    if (!strcmp(value, "r"))
       gpio_cmd_type = GPIO_CMD_TYPE_READ;
    else if (!strcmp(value, "w"))
        gpio_cmd_type = GPIO_CMD_TYPE_WRITE;
    else if (!strcmp(value, "c"))
        gpio_cmd_type = GPIO_CMD_TYPE_CONFIG;
    else
        goto parseGpioCommand_fail;

    /* GPIO NUMBER */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseGpioCommand_fail;
    gpioGrp = (MV_U32)strtoul(value, &value, 0);
    if (gpioGrp > 1)
        goto parseGpioCommand_fail;

    if (gpio_cmd_type == GPIO_CMD_TYPE_READ)
    {
        printf("GPIO #%d: 0x%08x\n",gpioGrp, srvCpuGpioInRead(gpioGrp));
    }
    else
    {/* gpio_cmd_type == GPIO_CMD_TYPE_WRITE || GPIO_CMD_TYPE_CONFIG */
        /* value to write */
        value = strtok( NULL, DELIMETERS );
        if (value == NULL )
        goto parseGpioCommand_fail;
        valueToWrite = (MV_U32)strtoul(value, &value, 0);
        if(gpio_cmd_type == GPIO_CMD_TYPE_WRITE)
        {
            srvCpuGpioOutWrite(gpioGrp, valueToWrite);
            printf("Done set GPIO #%d pins in value %p\n", gpioGrp, valueToWrite);
        }
        else
        {/*gpio_cmd_type == GPIO_CMD_TYPE_CONFIG*/
            srvCpuGpioInOutConfig(gpioGrp, valueToWrite);
            printf("Done Configure GPIO #%d pins in value %p\n", gpioGrp, valueToWrite);
        }
    }

    return pdFALSE;
parseGpioCommand_fail:
    printf( "Error: Wrong input.\nUsage: gpio_grp r/w/c <GPIO Group> <value - only in w/c>\n");
    printf( "Read/Write/Config, GPIO Group: 0/1, value as bitmap, Configure Pins to IN/OUT as bitmap, where IN (1) / OUT (0)\n");
    return pdFALSE;
}

/************************************************************************************
* prvGpioPinParseCommand -  This routine parse read/write/config gpio pin commands
************************************************************************************/
static portBASE_TYPE prvGpioPinParseCommand( int8_t *pcWriteBuffer,
                                       size_t xWriteBufferLen,
                                       const int8_t *pcCommandString )
{

    MV_U32 gpioPin = 0, valueToWrite = 0;
    enum {GPIO_CMD_TYPE_READ, GPIO_CMD_TYPE_WRITE, GPIO_CMD_TYPE_CONFIG} gpio_cmd_type;
    char *value;

    value = strtok( ( char * )&pcCommandString[0], DELIMETERS );
    /* Get the command type. */
    if ( (value == NULL) || ( strcmp(value, "gpio_pin") ) )
        goto parseGpioCommand_fail;

    /* Get the command type: r/w/c. */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseGpioCommand_fail;
    if (!strcmp(value, "r"))
       gpio_cmd_type = GPIO_CMD_TYPE_READ;
    else if (!strcmp(value, "w"))
        gpio_cmd_type = GPIO_CMD_TYPE_WRITE;
    else if (!strcmp(value, "c"))
        gpio_cmd_type = GPIO_CMD_TYPE_CONFIG;
    else
        goto parseGpioCommand_fail;

    /* GPIO PIN */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseGpioCommand_fail;
    gpioPin = (MV_U32)strtoul(value, &value, 0);
    if (gpioPin > 48)
        goto parseGpioCommand_fail;

    if (gpio_cmd_type == GPIO_CMD_TYPE_READ)
    {
        printf("GPIO pin#%d: 0x%x\n",gpioPin, srvCpuGpioPinRead(gpioPin));
    }
    else
    {/* gpio_cmd_type == GPIO_CMD_TYPE_WRITE || GPIO_CMD_TYPE_CONFIG */
        /* value to write */
        value = strtok( NULL, DELIMETERS );
        if (value == NULL )
        goto parseGpioCommand_fail;
        valueToWrite = (MV_U32)strtoul(value, &value, 0);
        if(gpio_cmd_type == GPIO_CMD_TYPE_WRITE)
        {
            srvCpuGpioPinWrite(gpioPin, valueToWrite);
            printf("Done set GPIO pin#%d in value 0x%x\n", gpioPin, valueToWrite);
        }
        else
        {/*gpio_cmd_type == GPIO_CMD_TYPE_CONFIG*/
            srvCpuGpioPinConfig(gpioPin, valueToWrite);
            printf("Done Configure GPIO pin#%d in value 0x%x\n", gpioPin, valueToWrite);
        }
    }

    return pdFALSE;
parseGpioCommand_fail:
    printf( "Error: Wrong input.\nUsage: gpio_pin r/w/c <GPIO PIN> <value - only in w/c>\n");
    printf( "Read/Write/Config, GPIO PIN: 0-48, Configure Pin to Configure Pin to IN (1) / OUT (0)\n\n");
    return pdFALSE;
}

static const CLI_Command_Definition_t xGpioGrpCommand =
{
    ( const int8_t * const ) "gpio_grp",
    ( const int8_t * const ) "gpio_grp r/w/c <GPIO Group> <value - only in w/c> \n"
            " Read/Write/Config, GPIO Group: 0-1, value as bitmap, Config Pins to IN/OUT as bitmap, where IN(1)/OUT(0)\n\n",
    prvGpioGrpParseCommand,
    -1
};

static const CLI_Command_Definition_t xGpioPinCommand =
{
    ( const int8_t * const ) "gpio_pin",
    ( const int8_t * const ) "gpio_pin r/w/c <GPIO PIN> <value - only in w/c> \n"
            " Read/Write/Config, GPIO PIN: 0-48, Configure Pin to IN (1) / OUT (0)\n\n",
    prvGpioPinParseCommand,
    -1
};

void regiserGpioCliCommands()
{
    FreeRTOS_CLIRegisterCommand( &xGpioPinCommand);
    FreeRTOS_CLIRegisterCommand( &xGpioGrpCommand);
}

