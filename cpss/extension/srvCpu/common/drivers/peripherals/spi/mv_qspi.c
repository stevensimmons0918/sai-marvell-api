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
* @file mv_qspi.c
*
* @brief Qspi capabilities implementation.
*
* @version   1
********************************************************************************
*/

#include <stdio.h>
#include <stdint.h>
#include <printf.h>
#include <mv_services.h>
#include <mvOsTimer_cm3.h>

#include "mv_qspi.h"

#define QSPI_REG_BASE_ADDRESS      AC5_QSPI_REG_BASE_ADDRESS[ch]
#define QSPI_CTRL_REG              QSPI_REG_BASE_ADDRESS
#define QSPI_CFG_REG               (QSPI_REG_BASE_ADDRESS + 0x04)
#define QSPI_DATA_OUT_REG          (QSPI_REG_BASE_ADDRESS + 0x08)
#define QSPI_DATA_IN_REG           (QSPI_REG_BASE_ADDRESS + 0x0c)
#define QSPI_INSTRUCTION_REG       (QSPI_REG_BASE_ADDRESS + 0x10)
#define QSPI_ADDR_REG              (QSPI_REG_BASE_ADDRESS + 0x14)
#define QSPI_READ_MODE_REG         (QSPI_REG_BASE_ADDRESS + 0x18)
#define QSPI_HDR_COUNT_REG         (QSPI_REG_BASE_ADDRESS + 0x1c)
#define QSPI_DATA_IN_CNT_REG       (QSPI_REG_BASE_ADDRESS + 0x20)
#define QSPI_TIMING_REG            (QSPI_REG_BASE_ADDRESS + 0x24)
#define QSPI_INT_STATUS_REG        (QSPI_REG_BASE_ADDRESS + 0x28)
#define QSPI_INT_ENABLE_REG        (QSPI_REG_BASE_ADDRESS + 0x2c)
#define QSPI_START_ADDR_REG        (QSPI_REG_BASE_ADDRESS + 0x30)
#define QSPI_END_ADDR_REG          (QSPI_REG_BASE_ADDRESS + 0x34)
#define QSPI_PHYS_ADDR_REG         (QSPI_REG_BASE_ADDRESS + 0x38)

/* QSPI_CTRL_REG */
#define SPI_EN_3        19
#define SPI_EN_2        18
#define SPI_EN_1        17
#define SPI_EN_0        16
#define ADDR_ERR        12
#define WFIFO_OVRFLW    11
#define WFIFO_UNDRFLW   10
#define RFIFO_OVRFLW    9
#define RFIFO_UNDRFLW   8
#define WFIFO_FULL      7
#define WFIFO_EMPTY     6
#define RFIFO_FULL      5
#define RFIFO_EMPTY     4
#define WFIFO_RDY       3
#define RFIFO_RDY       2
#define SPI_RDY         1
#define SPI_XFER_DONE   0

/* QSPI_CFG_REG */
#define WFIFO_THRS_OFFSET       28
#define WFIFO_THRS_MASK         7
#define RFIFO_THRS_OFFSET       24
#define RFIFO_THRS_MASK         7
#define AUTO_CS_EN              20
#define DMA_WR_EN               19
#define DMA_RD_EN               18
#define FIFO_MODE               17
#define SRST                    16
#define XFER_START              15
#define XFER_STOP               14
#define INSTR_PIN               13
#define ADDR_PIN                12
#define DATA_PIN                10
#define DATA_PIN_MASK           3
#define FIFO_FLUSH              9
#define RW_EN                   8
#define SPI_CLK_POL             7
#define SPI_CLK_PHASE           6
#define BYTE_LEN                5
#define SPI_CLK_PRESCALE_OFFSET 0
#define SPI_CLK_PRESCALE_MASK   0x1f

/* QSPI_HDR_COUNT_REG */
#define DUMMY_CNT     12
#define RM_CNT        8
#define ADDR_CNT      4
#define INSTR_CNT     0

/* QSPI_TIMING_REG */
#define SPI_SIG_MON_SEL_OFFSET 28
#define SPI_SIG_MON_SEL_MASK   7
#define CS_SETUP_HOLD_OFFSET   12
#define CS_SETUP_HOLD_MASK     3
#define CLK_OUT_DLY_OFFSET     8
#define CLK_OUT_DLY_MASK       3
#define CLK_CAPT_EDGE          7
#define CLK_IN_DLY_OFFSET      4
#define CLK_IN_DLY_MASK        3
#define DATA_IN_DLY_OFFSET     0
#define DATA_IN_DLY_MASK       3

/* QSPI_INT_*_REG */
#define ADDR_ERR       12
#define WFIFO_OVRFLW   11
#define WFIFO_UNDRFLW  10
#define RFIFO_OVRFLW   9
#define RFIFO_UNDRFLW  8
#define WFIFO_FULL     7
#define WFIFO_EMPTY    6
#define RFIFO_FULL     5
#define RFIFO_EMPTY    4
#define WFIFO_RDY      3
#define RFIFO_RDY      2
#define SPI_RDY        1
#define XFER_DONE      0

/* Delay*/
/* one bit(~100nsec) * 8 bits =~1mSec for one byte*/
#define DELAY_32_BITS_IN_MILLISEC 4
#define DELAY_8_BITS_IN_MILLISEC  1

/* Commands */
#define QSPI_FLASH_READ_CMD 0x03
#define QSPI_FLASH_READ_ID  0x9F
#define QSPI_FLASH_READ_QUADRO_CMD 0x6B

MV_U32 AC5_QSPI_REG_BASE_ADDRESS[2];

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
int srvCpuQspiInit(MV_U32 ch, MV_U32 base_address, MV_U32 baudrate, enum SRV_CPU_SPI_MODE mode)
{
    MV_U32 value;

    AC5_QSPI_REG_BASE_ADDRESS[ch] = base_address;

    /* Reset the qspi controller to defaults */
    srvCpuRegWrite(QSPI_CFG_REG, (1 << SRST));
    srvCpuRegWrite(QSPI_CFG_REG, 0);

    /*buadrate - set 0x805A0004[4:0] = 200000000 / baudrate */
    value = srvCpuRegRead(QSPI_CFG_REG) & 0xffffff20; /* mask SPI_CLK_PRESCALE [4:0], SPI_CLK_PHASE [6] and SPI_CLK_POL [7] */
    value |=  (200000000 / baudrate) & 0x1f; /* baud rate */
    value |=  ((MV_U32)mode << 6) & 0xC0; /* mode */
    srvCpuRegWrite(QSPI_CFG_REG, value);

    value = srvCpuRegRead(QSPI_CFG_REG);
    value |= 1 << FIFO_MODE; /* Enable FIFO, default speed 20MHz */
    srvCpuRegWrite(QSPI_CFG_REG, value);

    return 0;
}

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
int srvCpuQspiRead(MV_U32 ch, MV_U8 cmd, MV_U32 addr, MV_U8 addr_cycles, MV_U8 dummy, void *buf, MV_U32 size)
{

    MV_U32 i = 0, length = size;
    MV_U32 *buffer32 = (MV_U32 *)buf;
    MV_U8 *buffer = (MV_U8 *)buf;

    /* Flush FIFO by writing '1' to FIFO_FLUSH */
    srvCpuRegWrite(QSPI_CFG_REG, srvCpuRegRead(QSPI_CFG_REG) | (1 << FIFO_FLUSH));
    /* Wait until FIFO is flushed (FIFO_FLUSH = '0') */
    while (srvCpuRegRead(QSPI_CFG_REG) & (1 << FIFO_FLUSH));
    /* Assert SPI_CS0 */
    srvCpuRegWrite(QSPI_CTRL_REG, srvCpuRegRead(QSPI_CTRL_REG) | (1 << SPI_EN_0));
    /* Program SPI_INSTR */
    if (cmd) {
        srvCpuRegWrite(QSPI_INSTRUCTION_REG, cmd);
        i = 1;
    }

    /* Program SPI Header count */
    srvCpuRegWrite(QSPI_HDR_COUNT_REG, i + (addr_cycles << ADDR_CNT) + (dummy << DUMMY_CNT));
    /* Program SPI_ADDR */
    srvCpuRegWrite(QSPI_ADDR_REG, addr);
    /* Program SPI_RM */
    srvCpuRegWrite(QSPI_READ_MODE_REG, 0);
    /* Program SPI_IN_CNT */
    srvCpuRegWrite(QSPI_DATA_IN_CNT_REG, length);
    /* Program SPI Cfg reg FIFO_MODE = 1, RW_EN = 0, XFER_START = 1 */
    srvCpuRegWrite(QSPI_CFG_REG, (srvCpuRegRead(QSPI_CFG_REG) & ~(1 << RW_EN)) | (1 << XFER_START));

    /* Code here is duplicated for performance sake */
    /* Read bytes from FIFO */
    if ((srvCpuRegRead(QSPI_CFG_REG) & (DATA_PIN_MASK << DATA_PIN)) == (0x2 << DATA_PIN))
    {
        size >>= 2; /* We read bytes in 32bit words */
        while (size >= 8) {
            i = 100;
            while (!(srvCpuRegRead(QSPI_CTRL_REG) & (1 << RFIFO_FULL)))
            {
                srvCpuOsUsDelay(DELAY_32_BITS_IN_MILLISEC);
                if (i-- < 1)
                    return 1;
            }
            for (i = 0; i < 8; i++, size--){
                *buffer32++ = srvCpuRegRead(QSPI_DATA_IN_REG);
            }
        }
        while (!(srvCpuRegRead(QSPI_CTRL_REG) & (1 << RFIFO_EMPTY)) && size > 0) {
            *buffer32++ = srvCpuRegRead(QSPI_DATA_IN_REG);
            size--;
        }
    } else {
        while (size >= 8) {
            i = 100;
            while (!(srvCpuRegRead(QSPI_CTRL_REG) & (1 << RFIFO_FULL)))
            {
                srvCpuOsUsDelay(DELAY_8_BITS_IN_MILLISEC);
                if (i-- < 1)
                    return 1;
            }
            for (i = 0; i < 8; i++, size--) {
                *buffer++ =  srvCpuRegRead(QSPI_DATA_IN_REG);
            }
        }
        /* Wait until XFER DONE before de-asserting CS */
        i = 100;
        while (!(srvCpuRegRead(QSPI_CTRL_REG) & (1 << SPI_XFER_DONE)))
        {
            srvCpuOsUsDelay(DELAY_8_BITS_IN_MILLISEC);
            if (i-- < 1)
                return 1;
        }
        while (!(srvCpuRegRead(QSPI_CTRL_REG) & (1 << RFIFO_EMPTY)) && size > 0) {
            *buffer++ = srvCpuRegRead(QSPI_DATA_IN_REG);
            size--;
        }
    }
    /* De-assert SPI_CS0 */
    srvCpuRegWrite(QSPI_CTRL_REG, srvCpuRegRead(QSPI_CTRL_REG) & ~(1 << SPI_EN_0));

    return 0;
}

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
int srvCpuQspiWrite(MV_U32 ch, MV_U8 cmd, MV_U32 addr, MV_U8 addr_cycles, MV_U8 dummy, void *buf, MV_U32 size)
{

    MV_U32 i = 0;
    MV_U32 *buffer32 = (MV_U32 *)buf;
    MV_U8 *buffer = (MV_U8 *)buf;

    /* Flush FIFO by writing '1' to FIFO_FLUSH */
    srvCpuRegWrite(QSPI_CFG_REG, srvCpuRegRead(QSPI_CFG_REG) | (1 << FIFO_FLUSH));
    /* Wait until FIFO is flushed (FIFO_FLUSH = '0') */
    while (srvCpuRegRead(QSPI_CFG_REG) & (1 << FIFO_FLUSH));

    /* Assert SPI_CS0 */
    srvCpuRegWrite(QSPI_CTRL_REG, srvCpuRegRead(QSPI_CTRL_REG) | (1 << SPI_EN_0));

    /* Program SPI_INSTR */
    if (cmd) {
        srvCpuRegWrite(QSPI_INSTRUCTION_REG, cmd);
        i = 1;
    }
    /* Program SPI Header count */
    srvCpuRegWrite(QSPI_HDR_COUNT_REG, i + (addr_cycles << ADDR_CNT) + (dummy << DUMMY_CNT));
    /* Program SPI_ADDR */
    srvCpuRegWrite(QSPI_ADDR_REG, addr);
    /* Program SPI Cfg reg FIFO_MODE = 1, RW_EN = 1, XFER_START = 1 */
    srvCpuRegWrite(QSPI_CFG_REG, srvCpuRegRead(QSPI_CFG_REG) | (1 << RW_EN) | (1 << XFER_START));

    /* if buffer is empty, just wait for header transmit done */
    if (size == 0)
        while (!(srvCpuRegRead(QSPI_CTRL_REG) & (1 << SPI_RDY)));

    /* Code here is duplicated for performance sake */
    /* send bytes from buffer to FIFO */
    if ((srvCpuRegRead(QSPI_CFG_REG) & (DATA_PIN_MASK << DATA_PIN)) == (0x2 << DATA_PIN))
    {
        if (size % 4)
            size += 4;
        size >>= 2; /* We send bytes in 32bit words */

        while (size-- > 0) { /* Fill Fifo */
            i = 100;
            while (srvCpuRegRead(QSPI_CTRL_REG) & (1 << WFIFO_FULL))
            {
                srvCpuOsUsDelay(DELAY_32_BITS_IN_MILLISEC);
                if (i-- < 1)
                    return 1;
            }
            srvCpuRegWrite(QSPI_DATA_OUT_REG, *buffer32++); /* Add word to Fifo */
        }
    } else {
        while (size-- > 0) { /* Fill Fifo */
            i = 100;
            while (srvCpuRegRead(QSPI_CTRL_REG) & (1 << WFIFO_FULL))
            {
                srvCpuOsUsDelay(DELAY_8_BITS_IN_MILLISEC);
                if (i-- < 1)
                    return 1;
            }
            srvCpuRegWrite(QSPI_DATA_OUT_REG, *buffer++); /* Add byte to Fifo */
        }
    }
    /* Wait until Fifo is empty, transaction is done */
    i = 100;
    while (!(srvCpuRegRead(QSPI_CTRL_REG) & (1 << WFIFO_EMPTY)))
    {
        srvCpuOsUsDelay(DELAY_8_BITS_IN_MILLISEC);
            if (i-- < 1)
                return 1;
    }

    /* Program SPI Cfg reg XFER_STOP = 1 */
    srvCpuRegWrite(QSPI_CFG_REG, srvCpuRegRead(QSPI_CFG_REG) | (1 << XFER_STOP));
    /* Wait until XFER DONE before de-asserting CS */
    i = 100;
    while (!(srvCpuRegRead(QSPI_CTRL_REG) & (1 << SPI_XFER_DONE)))
    {
        srvCpuOsUsDelay(DELAY_8_BITS_IN_MILLISEC);
        if (i-- < 1)
            return 1;
    }
    /* De-assert SPI_CS0 */
    srvCpuRegWrite(QSPI_CTRL_REG, srvCpuRegRead(QSPI_CTRL_REG) & ~(1 << SPI_EN_0));

    return 0;
}
