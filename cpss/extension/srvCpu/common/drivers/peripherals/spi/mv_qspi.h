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
* @file mv_qspi.h
*
* @brief Qspi capabilities declarations.
*
* @version   1
********************************************************************************
*/
#ifndef __MV_QSPI_H__
#define __MV_QSPI_H__
#include <stdint.h>
#include "common.h"

/**
 * @enum SRV_CPU_SPI_MODE
 *
 * @brief SPI mode options
 *
*/
enum SRV_CPU_SPI_MODE {
    SRV_CPU_SPI_MODE_0_E,
    SRV_CPU_SPI_MODE_1_CPHA_E,
    SRV_CPU_SPI_MODE_2_CPOL_E,
    SRV_CPU_SPI_MODE_3_CPOL_CPHA_E,
};

/**
* @internal srvCpuQspiInit function
* @endinternal
*
* @brief  Initialize the QSPI interface for single pin.
*
* @param MV_U32 ch             - select between first QSPI and second QSPI
* @param MV_U32 baudRate       - set baud rate 
* @param SRV_CPU_SPI_MODE mode - set SPI mode
*
* @retval none
*
*/
int srvCpuQspiInit(MV_U32 ch, MV_U32 base_address, MV_U32 baudRate, enum SRV_CPU_SPI_MODE mode);

/**
* @internal srvCpuQspiRead function
* @endinternal
*
* @brief   send a read command with offset & dummy bytes, using FIFO
*
* @param MV_U32 ch          - select between first QSPI and second QSPI
* @param MV_U8  cmd         - 8bit instruction value (assumed to be single byte), 0x00 means no instruction
* @param MV_U32 addr        - value to send for address, up to 4 bytes long
* @param MV_U8  addr_cycles - 1-4 bytes, representing address length
* @param MV_U8  dummy       - number of dummy bytes to send after address, before collecting received bytes
* @param void*  buf         - pointer to buffer for received bytes
* @param MV_U32 size        - size of buffer  
*       
* @retval 0            - on success.
* @retval 1            - on error
*/
int srvCpuQspiRead(MV_U32 ch, MV_U8 cmd, MV_U32 addr, MV_U8 addr_cycles, MV_U8 dummy, void *buf, MV_U32 size);

/**
* @internal srvCpuQspiWrite function
* @endinternal
*
* @brief   send a write command with offset & dummy bytes, using FIFO
*
* @param MV_U32 ch          - select between first QSPI and second QSPI
* @param MV_U8  cmd         - 8bit instruction value (assumed to be single byte), 0x00 means no instruction
* @param MV_U32 addr        - value to send for address, up to 4 bytes long
* @param MV_U8  addr_cycles - 1-4 bytes, representing address length
* @param MV_U8  dummy       - number of dummy bytes to send after address, before collecting received bytes
* @param void*  buf         - pointer to buffer for transmitting
* @param MV_U32 size        - size of buffer
*
* @retval 0            - on success.
* @retval 1            - on error
*/
int srvCpuQspiWrite(MV_U32 ch, MV_U8 cmd, MV_U32 addr, MV_U8 addr_cycles, MV_U8 dummy, void *buf, MV_U32 size);

#endif /*__MV_QSPI_H__*/
