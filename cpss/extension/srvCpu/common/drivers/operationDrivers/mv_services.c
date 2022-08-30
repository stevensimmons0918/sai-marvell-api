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
* @file mv_services.c
*
* @brief srvCpu services implementation.
*
* @version   1
********************************************************************************
*/

#include <stdio.h>
#include <stdint.h>
#include <printf.h>
#include "mv_services.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include "global.h"

#define cm3_internal_win_conf_write( val )  \
    (*(( volatile unsigned long * )(MG_BASE | REG_MBUS_SYSTEM_INT_WINDOW_CONFIG ))) = (val)

#define CM3_BASE_ADDR_VAL(addr,attr,target)     ( (addr-EXT_MEM_BASE) | attr<<8 | target)

/* Globals */
enum SRV_CPU_DEVICE_TYPE dev_type; /* Keeps the PP device type (Aldrin / BC3 / Pipe) */
static MV_U32 qspi_base_offset;
static xSemaphoreHandle xMutex = NULL;

#if 0
static void prvISR(void)
{
    /* remote int */
    if (MV_MEMIO32_READ(0xa0018114) & BIT_15 ) {
        MSS_REG_BIT_RESET(0x18114, BIT_15); // will also clear cause reg 0x3F8
        printf("Remote interrupt\n");
        return;
    }

    if (prvPpReadRegister(0x9C)) {  // // cause bit is clear-on-read. Will be cleared if was asserted
        printf("Doorbell interrupt\n");
    } else
        printf("Unknown interrupt\n");
}

static void intInit(void)
{
    if (iICRegisterHandler(NVIC_GLOBAL_IRQ_0, 0, prvISR, IRQ_ENABLE, 0) <0)
        printf( "iICRegisterHandler failed\n");
    // Remote interrupt
    // MV_MEMIO32_WRITE(0x60000034, 8); // Enable global int - not enabled by SW, as board does not load with int-line connected, so line is
    // only connected after it loads
    MV_MEMIO32_WRITE(0xa0018118, BIT_15); // unmask GPIO_15 level interrupt
    MV_MEMIO32_WRITE(0xa0018114, 0);      // clear, in order to fresh-start
    MV_MEMIO32_WRITE(0x600003fc, BIT_21); // unmask int tree: GPIO_15_8_INT

    // Doorbell interrupt
    // MG-0
    prvPpWriteRegister(0xa0, BIT_7);
    prvPpWriteRegister(0x34, BIT_18);
    // MG-1
    MV_MEMIO32_WRITE(0x600003fc, BIT_21 | BIT_1); // add bit 1
    MV_MEMIO32_WRITE(0x60000034, BIT_3);
}
#endif

/**
* @internal srvCpuCm3ToMsysInterRegs function
* @endinternal
*
* @brief   set base and size to configure internal access window to RUnit
*
* @param base           - base
* @param size           - size
*
* @retval void              - none
*/
void srvCpuCm3ToMsysInterRegs(unsigned long base, unsigned long size)
{
    if (size < _128K)
        size = _128K;

    if (base >= _128K)
        base = _128K - 1;

    cm3_internal_win_conf_write( base | size);
}

/************************************************************************** 
 * Map window[2] into switch address space, return offset from window
 *
 * offset - absolute offset of regiser
 *************************************************************************/
static MV_U32 remapWindow(MV_U32 offset)
{
    int port = 0xD, value;
    if (dev_type ==  SRV_CPU_DEVICE_TYPE_AC5_E) {
        value = MV_MEMIO32_READ(MG_BASE | 0x4a0);
       switch( offset & 0xFFF00000) {
           case 0x7F900000:
                port = 4;
            break;
            case 0x7FA00000:
                port = 5;
            break;
            case 0x7FB00000:
                port = 6;
            break;
            default:
            break;
        }
        MV_MEMIO32_WRITE(MG_BASE | 0x4a0, (value  & 0xfffffff0) | port);
    }
    MV_MEMIO32_WRITE(MG_BASE | 0x4ac, (offset & 0xfff00000) | 0xe);
    return offset & 0xfffff;
}

/**
* @internal srvCpuRegRead function
* @endinternal
*
* @brief   Read a switch address space register using 2nd window, previous window base is preserved
*
* @param offset           - absolute offset of register
*
* @retval MV_U32          - the value of the wanted register
*/
MV_U32 srvCpuRegRead(MV_U32 offset)
{
    MV_U32 retValue = 0;

	if (offset >= 0x7f000000 && offset <= 0x7f200000)
		retValue = MV_MEMIO32_READ(0xa0000000 | (offset & 0x00ffffff));
	else
	{
		/*  See if can obtain the semaphore prior to critical section. If the semaphore is not
        available wait 1 ticks to see if it becomes free. */
		if( xSemaphoreTake( xMutex, 1) == pdTRUE )
		{
			retValue =  MV_MEMIO32_READ(SRV_CPU_SW_WIN_BASE | remapWindow(offset));
			/* finish accessing the critical section. Release the
			semaphore. */
			xSemaphoreGive( xMutex );
		}
	}
    return retValue;
}

/**
* @internal srvCpuRegWrite function
* @endinternal
*
* @brief   Write a switch address space register using 2nd window, previous window base is preserved
*
* @param offset           - absolute offset of register
* @param value            - value to write
*
* @retval none
*/
MV_U32 srvCpuRegWrite(MV_U32 offset, MV_U32 value)
{
    MV_U32 retValue = 0;

    if (offset >= 0x7f000000 && offset <= 0x7f200000)
        retValue =  MV_MEMIO32_WRITE(0xa0000000 | (offset & 0x00ffffff), value);
	else
	{
		/*  See if can obtain the semaphore prior to critical section. If the semaphore is not
        available wait 1 ticks to see if it becomes free. */
		if( xSemaphoreTake( xMutex, 1) == pdTRUE )
		{
			retValue =  MV_MEMIO32_WRITE(SRV_CPU_SW_WIN_BASE | remapWindow(offset), value);
			/* finish accessing the critical section. Release the
			semaphore. */
			xSemaphoreGive( xMutex );
		}
	}
    return retValue;
}
#if 0
/************************************************************************** 
 * Reset the device
 *************************************************************************/
static void reset(void)
{
    switch (dev_type) {
    case SRV_CPU_DEVICE_TYPE_HARRIER_E:
        MV_REG_BIT_RESET(SRV_CPU_DFX_REG_BASE | SOFT_RESET_REG_HARRIER, SOFT_RESET_BIT_HARRIER);
        break;
    default:
        MV_REG_BIT_SET( SYSREG_RSTOUT, 1 << SYSBIT_RSTOUT_GLOBAL_EN );
        MV_REG_BIT_SET( SYSREG_SOFT_RESET, 1 << SYSBIT_SOFT_RESET_GLOB );
    }
}
#endif

/**
* @internal srvCpuCm3SysmapInit function
* @endinternal
*
* @brief   Setup mapping windows
*          This routine sets up mapping windows for various units such as DFX, SPI.
*
* @retval void          - none
*/
void srvCpuCm3SysmapInit(void)
{
    MV_U32 deviceId;
    MV_U32 data, dfx_base = 0;

    /* Create mutex before starting tasks */
    xMutex = xSemaphoreCreateMutex();

    deviceId = (MV_MEMIO32_READ(MG_BASE | 0x4c) >> 4) & 0xff00;
    switch (deviceId) {
    case SRV_CPU_DEVICE_ID_ALDRIN:
        dev_type = SRV_CPU_DEVICE_TYPE_ALDRIN_E;
        break;
    case SRV_CPU_DEVICE_ID_ALDRIN2:
        dev_type = SRV_CPU_DEVICE_TYPE_ALDRIN2_E;
        break;
    case SRV_CPU_DEVICE_ID_PIPE:
        dev_type = SRV_CPU_DEVICE_TYPE_PIPE_E;
        break;
#ifdef CNM
    case 0:
#else
    case SRV_CPU_DEVICE_ID_AC5:
    case SRV_CPU_DEVICE_ID_AC5X:
#endif
        dev_type = SRV_CPU_DEVICE_TYPE_AC5_E;
        dfx_base = 0x1be0000e;
        break;
    case SRV_CPU_DEVICE_ID_AC5P:
        dev_type = SRV_CPU_DEVICE_TYPE_AC5P_E;
        dfx_base = 0x0b80000e;
        break;
    case SRV_CPU_DEVICE_ID_HARRIER:
        dev_type = SRV_CPU_DEVICE_TYPE_HARRIER_E;
        dfx_base = 0;
        break;
    default:
        if ((deviceId & 0xfc00) == SRV_CPU_DEVICE_ID_FALCON)
        {
            dev_type = SRV_CPU_DEVICE_TYPE_FALCON_E;
            dfx_base = 0x1be0000e;
        }
    }

    /* configure internal access window to RUnit */
    srvCpuCm3ToMsysInterRegs(SRV_CPU_CM3_INTERNAL_WIN_BASE, SRV_CPU_CM3_INTERNAL_WIN_SIZE);

    /* configure illegal access default target */
    MV_MEMIO32_WRITE((MG_BASE | 0x200), 0);
    MV_MEMIO32_WRITE((MG_BASE | 0x204), 0xe1);
    MV_MEMIO32_WRITE((MG_BASE | 0x340), 0);

    /* configure window 0 to DFX server */
    MV_MEMIO32_WRITE((MG_BASE | 0x484), 0x000f0000); /* Size = 1MB */
    MV_MEMIO32_WRITE((MG_BASE | 0x488), 0x00000000); /* High base addr */
    if (dev_type >= SRV_CPU_DEVICE_TYPE_FALCON_E) {
        MV_MEMIO32_WRITE((MG_BASE | 0x480), 0x0000000D); /* Base addr + Target, accessed @ 0xa0100000 */
        MV_MEMIO32_WRITE((MG_BASE | 0x48c), dfx_base | 0xe);   /* Remap */
    } else {
        MV_MEMIO32_WRITE((MG_BASE | 0x480), 0x00000008); /* Base addr + Target, accessed @ 0xa0100000 */
        MV_MEMIO32_WRITE((MG_BASE | 0x48c), 0x0000000e); /* Remap */
    }

    if (dev_type <= SRV_CPU_DEVICE_TYPE_FALCON_E) {
        MV_MEMIO32_WRITE((MG_BASE | 0x490), 0x10001e01); /* Base addr + Target, accessed @ 0xb0100000 */
        MV_MEMIO32_WRITE((MG_BASE | 0x494), 0x00ff0000); /* Size = 16MB */
        MV_MEMIO32_WRITE((MG_BASE | 0x498), 0x00000000); /* High base addr */
        MV_MEMIO32_WRITE((MG_BASE | 0x49c), 0x0000000e); /* Remap */
    }

    /* set other windows and enable debug */
    switch (dev_type) {
    case SRV_CPU_DEVICE_TYPE_ALDRIN_E:
        MV_MEMIO32_WRITE((INTER_REGS_BASE | 0x18000), 0x00222222); /* Set MPP[5:0] to SPI */
        MV_MEMIO32_WRITE((SRV_CPU_DFX_REG_BASE | 0xf8288), 0x067FC108); /* Enable CM3 JTAG */
        break;
    case SRV_CPU_DEVICE_TYPE_PIPE_E:
        /* PUP enable */
        MV_MEMIO32_WRITE((SRV_CPU_DFX_REG_BASE | 0xf828c), 0x00000006);
        /* set console owner to 1 in case only core 1 is active 
        if ((MV_MEMIO32_READ(SRV_CPU_DFX_REG_BASE | PIPE_SAR) & (0x03 << 9)) == (0x02 << 9))
            mvUartSetOwner(1); */
        break;
    case SRV_CPU_DEVICE_TYPE_BC3_E:
        break;
    case SRV_CPU_DEVICE_TYPE_ALDRIN2_E:
        MV_MEMIO32_WRITE((INTER_REGS_BASE | 0x18008),0x11111000);
        break;
    case SRV_CPU_DEVICE_TYPE_FALCON_E:
        data = MV_MEMIO32_READ(SRV_CPU_DFX_REG_BASE | 0xf828c);
        MV_MEMIO32_WRITE((SRV_CPU_DFX_REG_BASE | 0xf828c), (data | 0x00000001)); /* Enable CM3 JTAG */
        break;
    case SRV_CPU_DEVICE_TYPE_AC5_E:
        /* configure window 1 to QSPI controller */
        MV_MEMIO32_WRITE((MG_BASE | 0x490), 0x0010000D); /* Base addr + Target, accessed @ 0xa0200000 */
        MV_MEMIO32_WRITE((MG_BASE | 0x494), 0x000f0000); /* Size = 1MB */
        MV_MEMIO32_WRITE((MG_BASE | 0x498), 0x00000000); /* High base addr */
        MV_MEMIO32_WRITE((MG_BASE | 0x49c), 0x8050000e); /* Remap */
        /* configure window 1 to QSPI controller */
        MV_MEMIO32_WRITE((MG_BASE | 0x49c), 0x8000000e); /* Change window base to MPP RFU base */
        MV_MEMIO32_WRITE(0xa0220104, 0x11112222); /* set MPP 12-15 to SPI */
        MV_MEMIO32_WRITE((MG_BASE | 0x49c), 0x8050000e); /* Change window base back to QSPI */
        MV_MEMIO32_WRITE((MG_BASE | 0x4a0), 0x0020000D); /* Base addr + Target, accessed @ 0xa0300000 */
        MV_MEMIO32_WRITE((MG_BASE | 0x4a4), 0x000f0000); /* Size = 1MB */
        MV_MEMIO32_WRITE((MG_BASE | 0x4a8), 0x00000000); /* High base addr - this window is for general switch access */
        qspi_base_offset = 0xa0000;
        break;
    case SRV_CPU_DEVICE_TYPE_AC5P_E:
        data = MV_MEMIO32_READ(SRV_CPU_DFX_REG_BASE | 0xf8284);
        MV_MEMIO32_WRITE((SRV_CPU_DFX_REG_BASE | 0xf8284), (data | 0x00003c00)); /* Enable CM3 JTAG */
        break;
    case SRV_CPU_DEVICE_TYPE_HARRIER_E:
        /* configure window 1 to QSPI controller */
        MV_MEMIO32_WRITE((MG_BASE | 0x490), 0x0010000D); /* Base addr + Target, accessed @ 0xa0200000 */
        MV_MEMIO32_WRITE((MG_BASE | 0x494), 0x000f0000); /* Size = 1MB */
        MV_MEMIO32_WRITE((MG_BASE | 0x498), 0x00000000); /* High base addr */
        MV_MEMIO32_WRITE((MG_BASE | 0x49c), 0x3c10000e); /* Remap - this window is reserved for QSPI */
        MV_MEMIO32_WRITE((MG_BASE | 0x4a0), 0x0020000D); /* Base addr + Target, accessed @ 0xa0300000 */
        MV_MEMIO32_WRITE((MG_BASE | 0x4a4), 0x000f0000); /* Size = 1MB */
        MV_MEMIO32_WRITE((MG_BASE | 0x4a8), 0x00000000); /* High base addr - this window is for general switch access */
        data = MV_MEMIO32_READ(SRV_CPU_DFX_REG_BASE | 0xf8280);
        MV_MEMIO32_WRITE((SRV_CPU_DFX_REG_BASE | 0xf8280), (data | 0x00000003)); /* Enable CM3 JTAG */
        qspi_base_offset = 0x90000;
        break;
    }
}