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
* @file mvGpio.h
*
* @brief GPIO services declarations.
*
* @version   1
********************************************************************************
*/

#ifndef __MVGPIO_H__
#define __MVGPIO_H__
#include <stdint.h>
#include "common.h"


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
MV_BOOL srvCpuGpioInOutConfig(MV_U32 gpioGrp, MV_U32 regBitmap);

/**
* @internal srvCpuGpioInRead function
* @endinternal
*
* @brief  Read the GPIO pins value
*         Each bit in this field reflects the value of the corresponding GPIO pin.
*
* @param MV_U32 gpioGrp - GPIO group number (0: 0-31, 1: 32-48)
*
* @retval MV_U32 GPIO pins value
*
*/
MV_U32 srvCpuGpioInRead(MV_U32 gpioGrp);

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
MV_BOOL srvCpuGpioOutWrite(MV_U32 gpioGrp, MV_U32 regBitmap);

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
MV_BOOL srvCpuGpioBlinkConfig(MV_U32 gpioGrp, MV_U32 regBitmap);

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
MV_BOOL srvCpuGpioPolarityConfig(MV_U32 gpioGrp, MV_U32 polarityInBitmap);

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
MV_BOOL srvCpuGpioPinConfig(MV_U32 gpioPinNumber, MV_U32 gpioPinConfig);

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
MV_U32 srvCpuGpioPinRead(MV_U32 gpioPinNumber);

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
MV_BOOL srvCpuGpioPinWrite(MV_U32 gpioPinNumber, MV_U32 gpioPinValue);

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
MV_BOOL srvCpuGpioPinBlinkConfig(MV_U32 gpioPinNumber, MV_U32 gpioPinBlinkValue);

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
MV_BOOL srvCpuGpioPinPolarityConfig(MV_U32 gpioPinNumber, MV_U32 gpioPinPolarityValue);


/**
* @internal regiserGpioCommands function
* @endinternal
*
* @brief   Register GPIO CLI commands
* @retval  None
*
*/
void regiserGpioCliCommands(void);
#endif /*__MVGPIO_H__*/
