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
* @file mv_services.h
*
* @brief srvCpu services declarations.
*
* @version   1
********************************************************************************
*/

#ifndef __MV_SERVICES_H__
#define __MV_SERVICES_H__
#include <stdint.h>
#include "common.h"

#define SRV_CPU_DEFAULT_SYS_TCLK  ( 250000000 )
#define SRV_CPU_AC5_SYS_TCLK      ( 328000000 )

/* MG base address */
#define SRV_CPU_EXT_MEM_BASE        0xA0100000
#define SRV_CPU_DFX_WIN_BASE        0xA0100000
#define SRV_CPU_DFX_WIN_SIZE        (0xF<<16)     /* 1M */

#define SRV_CPU_DFX_REG_BASE        0x80000000

/* configuration parameters */
#define SRV_CPU_CM3_INTERNAL_WIN_SIZE   0x100000
#define SRV_CPU_CM3_INTERNAL_WIN_BASE   0x0

#define SRV_CPU_SOFT_RESET_REG_HARRIER  0xf8204 /* in DFX */
#define SRV_CPU_SOFT_RESET_BIT_HARRIER  1

#define SRV_CPU_SW_WIN_BASE         0xa0300000

/**
* @enum DEVICE_TYPE
 *
 * @brief Device type enum
*/
enum SRV_CPU_DEVICE_TYPE {
    SRV_CPU_DEVICE_TYPE_ALDRIN_E,
    SRV_CPU_DEVICE_TYPE_PIPE_E,
    SRV_CPU_DEVICE_TYPE_BC3_E,
    SRV_CPU_DEVICE_TYPE_ALDRIN2_E,
    SRV_CPU_DEVICE_TYPE_FALCON_E,
    SRV_CPU_DEVICE_TYPE_AC5_E,
    SRV_CPU_DEVICE_TYPE_AC5P_E,
    SRV_CPU_DEVICE_TYPE_HARRIER_E
};

#define SRV_CPU_DEVICE_ID_ALDRIN  0xc800
#define SRV_CPU_DEVICE_ID_FALCON  0x8400
#define SRV_CPU_DEVICE_ID_ALDRIN2 0xcc00
#define SRV_CPU_DEVICE_ID_AC5     0xb400
#define SRV_CPU_DEVICE_ID_AC5X    0x9800
#define SRV_CPU_DEVICE_ID_AC5P    0x9400
#define SRV_CPU_DEVICE_ID_BC3     TBD
#define SRV_CPU_DEVICE_ID_PIPE    0xf500
#define SRV_CPU_DEVICE_ID_HARRIER 0x9000

#define SRV_CPU_AC5_MG_SOURCE_ID  0x604

/**
* @internal srvCpuCm3ToMsysInterRegs function
* @endinternal
*
* @brief   set base and size to configure internal access window to RUnit
*
* @param base           - base
* @param size           - size
*
* @retval void          - none
*/
void   srvCpuCm3ToMsysInterRegs(unsigned long base, unsigned long size);

/**
* @internal srvCpuRegRead function
* @endinternal
*
* @brief   Read a switch address space register using 2nd window, previous window base is preserved
*
* @param offset           - absolute offset of regiser
*
* @retval MV_U32          - the value of the wanted register
*/
MV_U32 srvCpuRegRead(MV_U32 offset);

/**
* @internal srvCpuRegWrite function
* @endinternal
*
* @brief   Write a switch address space register using 2nd window, previous window base is preserved
*
* @param offset           - absolute offset of regiser
* @param value            - value to write
*
* @retval none
*/
MV_U32 srvCpuRegWrite(MV_U32 offset, MV_U32 value);

/**
* @internal srvCpuCm3SysmapInit function
* @endinternal
*
* @brief   Setup mapping windows
*          This routine sets up mapping windows for various units such as DFX, SPI.
*
* @retval void          - none
*/
void   srvCpuCm3SysmapInit(void);

#endif /*__MV_SERVICES_H__*/