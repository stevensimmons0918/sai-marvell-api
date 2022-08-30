/*******************************************************************************
Copyright (C) 2016 Marvell International Ltd.

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the three
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation, either version 2 of the License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

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

********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Marvell nor the names of its contributors may be
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

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <FreeRTOS.h>
#include <task.h>
#include <printf.h>
#include <mv_services.h>
#include <hw.h>
#include <FreeRTOS_CLI.h>
#include "mv_services.h"
#include <mvOsTimer_cm3.h>
#include "xorDma.h"

#define DELIMETERS   " "
#define isdigit(_c)  (((_c)>='0' && (_c)<='9')?1:0)

/* dma engine registers */
#define DMA_DESQ_BALR_OFF       0x000
#define DMA_DESQ_BAHR_OFF       0x004
#define DMA_DESQ_SIZE_OFF       0x008
#define DMA_DESQ_DONE_OFF       0x00c
#define DMA_DESQ_DONE_PENDING_MASK  0x7fff
#define DMA_DESQ_DONE_PENDING_SHIFT 0
#define DMA_DESQ_DONE_READ_PTR_MASK 0x1fff
#define DMA_DESQ_DONE_READ_PTR_SHIFT    16
#define DMA_DESQ_ARATTR_OFF     0x010
#define DMA_DESQ_ATTR_CACHE_MASK        0x3f3f
#define DMA_DESQ_ATTR_OUTER_SHAREABLE   0x202
#define DMA_DESQ_ATTR_CACHEABLE         0x3c3c
#define DMA_IMSG_CDAT_OFF       0x014
#define DMA_IMSG_THRD_OFF       0x018
#define DMA_IMSG_THRD_MASK      0x7fff
#define DMA_IMSG_THRD_SHIFT     0x0
#define DMA_DESQ_AWATTR_OFF     0x01c
/* same flags as DMA_DESQ_ARATTR_OFF */
#define DMA_DESQ_ALLOC_OFF      0x04c
#define DMA_DESQ_ALLOC_WRPTR_MASK   0xffff
#define DMA_DESQ_ALLOC_WRPTR_SHIFT  16
#define DMA_IMSG_BALR_OFF       0x050
#define DMA_IMSG_BAHR_OFF       0x054
#define DMA_DESQ_CTRL_OFF       0x100
#define DMA_DESQ_CTRL_32B       1
#define DMA_DESQ_CTRL_128B      7
#define DMA_DESQ_STOP_OFF       0x800
#define DMA_DESQ_STOP_QUEUE_RESET_OFFS  1
#define DMA_DESQ_STOP_QUEUE_RESET_ENA   1
#define DMA_DESQ_STOP_QUEUE_DIS_OFFS    0
#define DMA_DESQ_STOP_QUEUE_DIS_ENA 0
#define DMA_DESQ_DEALLOC_OFF    0x804
#define DMA_DESQ_ADD_OFF        0x808

/* dma engine global registers */
#define GLOB_BW_CTRL                0x10004
#define GLOB_BW_CTRL_NUM_OSTD_RD_SHIFT      0
#define GLOB_BW_CTRL_NUM_OSTD_RD_VAL        8
#define GLOB_BW_CTRL_NUM_OSTD_WR_SHIFT      8
#define GLOB_BW_CTRL_NUM_OSTD_WR_VAL        8
#define GLOB_BW_CTRL_RD_BURST_LEN_SHIFT     12
#define GLOB_BW_CTRL_RD_BURST_LEN_VAL       4
#define GLOB_BW_CTRL_RD_BURST_LEN_VAL_Z1    2
#define GLOB_BW_CTRL_WR_BURST_LEN_SHIFT     16
#define GLOB_BW_CTRL_WR_BURST_LEN_VAL       4
#define GLOB_BW_CTRL_WR_BURST_LEN_VAL_Z1    2
#define GLOB_PAUSE                  0x10014
#define GLOB_PAUSE_AXI_TIME_DIS_VAL     0x8
#define GLOB_SYS_INT_CAUSE          0x10200
#define GLOB_SYS_INT_MASK           0x10204
#define GLOB_MEM_INT_CAUSE          0x10220
#define GLOB_MEM_INT_MASK           0x10224
#define GLOB_SECURE                 0x10300
#define GLOB_SECURE_SECURE              0
#define GLOB_SECURE_UNSECURE            1
#define GLOB_SECURE_SECURE_OFF          0
#define GLOB_SECURE_SECURE_MASK         0x1

#define MV_XOR_V2_MIN_DESC_SIZE 32
#define MV_XOR_V2_EXT_DESC_SIZE 128

#define MV_XOR_V2_DESC_RESERVED_SIZE        12
#define MV_XOR_V2_DESC_BUFF_D_ADDR_SIZE     12

#define MV_XOR_V2_CMD_LINE_NUM_MAX_D_BUF    8

/*
 * descriptors queue size;
 * 128 descriptors per dma engine is the current minimum
 * to support a few dma engines operation
 */
#define MV_XOR_MAX_DESC_NUM          128
#define MV_XOR_DEFUALT_DESC_NUM      1
#define MV_XOR_DEFUALT_DESC_ID       0

/* dma engine base address */
#define MV_XOR_BASE               0x80060000
#define MV_XOR_ENGINE(n)          (MV_XOR_BASE + (n) * 0x20000)
#define MV_XOR_ENGINE_NUM         2
#define MV_XOR_DEFAULT_ENGINE     0

#define MV_XOR_MAX_BURST_SIZE       2   /* 256B read or write transfers */
#define MV_XOR_MAX_BURST_SIZE_MASK  0xff
#define MV_XOR_MAX_TRANSFER_SIZE    ((UINT32_MAX) & ~MV_XOR_MAX_BURST_SIZE_MASK)

/*
 * struct mv_xor_v2_hw_desc - dma hardware descriptor
 * @desc_id: used by software; not affected by hardware
 * @flags: error and status flags
 * @crc32_result: crc32 calculation result
 * @desc_ctrl: operation mode and control flags
 * @buff_size: amount of bytes to be processed
 * @fill_pattern_src_addr: fill-pattern or source-address and
 * aw-attributes
 */
struct mv_xor_v2_hw_desc {
    MV_U16 desc_id;

/* definitions for flags */
#define DESC_BYTE_CMP_STATUS_OFFS   9
#define DESC_BYTE_CMP_STATUS_MASK   0x1
#define DESC_BYTE_CMP_STATUS_FAIL   0
#define DESC_BYTE_CMP_STATUS_OK     1

    MV_U16 flags;
    MV_U32 crc32_result;
    MV_U32 desc_ctrl;

/* definitions for desc_ctrl */
#define DESC_BYTE_CMP_CRC_FIRST_SHIFT   19
#define DESC_BYTE_CMP_CRC_FIRST_ENA     1
#define DESC_BYTE_CMP_CRC_LAST_SHIFT    20
#define DESC_BYTE_CMP_CRC_LAST_ENA      1
#define DESC_NUM_ACTIVE_D_BUF_SHIFT     22
#define DESC_OP_MODE_SHIFT              28

    MV_U32 buff_size;
    MV_U32 fill_pattern_src_addr_lo;
    MV_U32 fill_pattern_src_addr_hi;
    MV_U32 fill_pattern_dst_addr_lo;
    MV_U32 fill_pattern_dst_addr_hi;
};

#define XOR_DMA_CLI

/* DMA descriptor queue is 256B alignment per HW requirement
   after filling all the the fields, the descriptor will copied to user defined address */
static struct mv_xor_v2_hw_desc qmem __aligned(0x100) = {0};

static inline void clrSetBits(MV_U32 addr, MV_U32 val, MV_U32 mask)
{
    srvCpuRegWrite(addr, (srvCpuRegRead(addr) & ~mask) | val);
}

/**
* @internal setupMappingWindowForDesc function
* @endinternal
*
* @brief   Setup mapping windows
*          This routine sets up mapping windows for storing the descriptor
*
* @param descAddr - absolute address to store the descriptor
*
* @retval void    - none
*/
static void setupMappingWindowForDesc(MV_U32 descAddr)
{
    /* configure window 2 to store the descriptor */
    MV_MEMIO32_WRITE((MG_BASE | 0x4c0), 0x10000003); /* Base addr + Target, accessed @ 0xb0100000 */
    MV_MEMIO32_WRITE((MG_BASE | 0x4c4), 0x000f0000); /* Size = 1MB */
    MV_MEMIO32_WRITE((MG_BASE | 0x4c8), 0x00000000); /* High base addr - this window is for general switch access */
    MV_MEMIO32_WRITE((MG_BASE | 0x4cc), descAddr | 0xe); /* Low Remap address */
    return;
}

static void mv_xor_v2_init(MV_U32 xor_id, MV_U32 descAddr)
{
    MV_U32 reg_val;
    MV_U32 xor_base = MV_XOR_ENGINE(xor_id);

    /* setting the descriptor  */
    const MV_U32 XOR_DMA_DESC_ADDR = descAddr;
    setupMappingWindowForDesc(XOR_DMA_DESC_ADDR);

    /* set descriptor size to dma engine */
    srvCpuRegWrite(xor_base + DMA_DESQ_CTRL_OFF, DMA_DESQ_CTRL_32B);

    /* set descriptors queue size to dma engine */
    srvCpuRegWrite(xor_base + DMA_DESQ_SIZE_OFF, 0x1);

    /*
     * enable secure mode in dma engine xorg secure reg to align
     * dma operations between sram and dram because of secure mode
     * of sram at ble stage
     */
    srvCpuRegWrite(xor_base + GLOB_SECURE, 0x0);

    /* set descriptors queue address to dma engine */
    srvCpuRegWrite(xor_base + DMA_DESQ_BALR_OFF, XOR_DMA_DESC_ADDR);
    srvCpuRegWrite(xor_base + DMA_DESQ_BAHR_OFF, 0x0);


    /*
     * set attributes for reading and writing data buffers to:
     * - outer-shareable - snoops to be performed on cpu caches;
     * - enable cacheable - bufferable, modifiable, other allocate, and allocate.
     */
    reg_val = srvCpuRegRead(xor_base + DMA_DESQ_ARATTR_OFF);
    reg_val &= ~DMA_DESQ_ATTR_CACHE_MASK;
    reg_val |= DMA_DESQ_ATTR_OUTER_SHAREABLE | DMA_DESQ_ATTR_CACHEABLE;
    srvCpuRegWrite(xor_base + DMA_DESQ_ARATTR_OFF, reg_val);

    reg_val = srvCpuRegRead(xor_base + DMA_DESQ_AWATTR_OFF);
    reg_val &= ~DMA_DESQ_ATTR_CACHE_MASK;
    reg_val |= DMA_DESQ_ATTR_OUTER_SHAREABLE | DMA_DESQ_ATTR_CACHEABLE;
    srvCpuRegWrite(xor_base + DMA_DESQ_AWATTR_OFF, reg_val);

    /*
     * bandwidth control to optimize dma performance:
     * - set write/read burst lengths to maximum write/read transactions;
     * - set outstanding write/read data requests to maximum value.
     */
    reg_val = (GLOB_BW_CTRL_NUM_OSTD_RD_VAL << GLOB_BW_CTRL_NUM_OSTD_RD_SHIFT) |
          (GLOB_BW_CTRL_NUM_OSTD_WR_VAL << GLOB_BW_CTRL_NUM_OSTD_WR_SHIFT) |
          (MV_XOR_MAX_BURST_SIZE << GLOB_BW_CTRL_RD_BURST_LEN_SHIFT) |
          (MV_XOR_MAX_BURST_SIZE << GLOB_BW_CTRL_WR_BURST_LEN_SHIFT);
    srvCpuRegWrite(xor_base + GLOB_BW_CTRL, reg_val);

    /* disable axi timer feature */
    reg_val = srvCpuRegRead(xor_base + GLOB_PAUSE);
    reg_val |= GLOB_PAUSE_AXI_TIME_DIS_VAL;
    srvCpuRegWrite(xor_base + GLOB_PAUSE, reg_val);

    /* clear all previous pending interrupt indications, prior to using engine */
    srvCpuRegWrite(xor_base + GLOB_SYS_INT_CAUSE, 0);

    /* enable dma engine */
    srvCpuRegWrite(xor_base + DMA_DESQ_STOP_OFF, DMA_DESQ_STOP_QUEUE_DIS_ENA << DMA_DESQ_STOP_QUEUE_DIS_OFFS);
}


static void mv_xor_v2_desc_prep(MV_U32 xor_id, enum mv_xor_v2_desc_op_mode op_mode,
                MV_U32* src, MV_U32* dst, MV_U32 size)
{
    struct mv_xor_v2_hw_desc *desc = &qmem;
    memset((void *)desc, 0, sizeof(*desc));
    desc->desc_id = MV_XOR_DEFUALT_DESC_ID;
    desc->flags = 0;
    desc->crc32_result = 0;
    desc->buff_size = size;

    switch (op_mode) {
    case DESC_OP_MODE_MEMSET:
        desc->desc_ctrl = DESC_OP_MODE_MEMSET << DESC_OP_MODE_SHIFT;
        break;
    case DESC_OP_MODE_MEMCPY:
        desc->desc_ctrl = DESC_OP_MODE_MEMCPY << DESC_OP_MODE_SHIFT;
        break;
    case DESC_OP_MODE_MEMCMP:
        desc->desc_ctrl = (DESC_OP_MODE_MEMCMP << DESC_OP_MODE_SHIFT) |
                  (DESC_BYTE_CMP_CRC_FIRST_ENA << DESC_BYTE_CMP_CRC_FIRST_SHIFT) |
                  (DESC_BYTE_CMP_CRC_LAST_ENA << DESC_BYTE_CMP_CRC_LAST_SHIFT);
        break;
    default:
        printf("XOR DMA: unsupported operation mode\n");
        return;
    }
    desc->fill_pattern_src_addr_lo = src[1];
    desc->fill_pattern_src_addr_hi = src[0];
    desc->fill_pattern_dst_addr_lo = dst[1];
    desc->fill_pattern_dst_addr_hi = dst[0];

    /* address of descriptor (base address from the CM3 FW point of view) is 0xB0100000 */
    memcpy((void*)0xB0100000, desc, sizeof(*desc));
}

static void mv_xor_v2_enqueue(MV_U32 xor_id)
{
    MV_U32 xor_base = MV_XOR_ENGINE(xor_id);
    /* write a number of new descriptors in the descriptors queue */
    srvCpuRegWrite(xor_base + DMA_DESQ_ADD_OFF, MV_XOR_DEFUALT_DESC_NUM);
}

static MV_U32 mv_xor_v2_done(MV_U32 xor_id)
{
    MV_U32 xor_base = MV_XOR_ENGINE(xor_id);
    /* return a number of completed descriptors */
    return srvCpuRegRead(xor_base + DMA_DESQ_DONE_OFF) & DMA_DESQ_DONE_PENDING_MASK;
}

static void mv_xor_v2_finish(MV_U32 xor_id)
{
    MV_U32 xor_base = MV_XOR_ENGINE(xor_id);

    /* reset dma engine */
    srvCpuRegWrite(xor_base + DMA_DESQ_STOP_OFF, DMA_DESQ_STOP_QUEUE_RESET_ENA << DMA_DESQ_STOP_QUEUE_RESET_OFFS);

    /* clear all pending interrupt indications, to release DMA engine in a clean state */
    srvCpuRegWrite(xor_base + GLOB_SYS_INT_CAUSE, 0);

    /*
     * disable secure mode in dma engine xorg secure reg to return dma
     * to the state (unsecure) prior to mv_xor_v2_init() call
     */
    clrSetBits(xor_base + GLOB_SECURE, GLOB_SECURE_UNSECURE << GLOB_SECURE_SECURE_OFF,
               GLOB_SECURE_SECURE_MASK << GLOB_SECURE_SECURE_OFF);
}

static MV_U32 mv_xor_v2_memcmp_status_get(MV_U32 xor_id)
{
    volatile struct mv_xor_v2_hw_desc *desc = (volatile void*)(0xB0100000);
    return (desc->flags >> DESC_BYTE_CMP_STATUS_OFFS) & DESC_BYTE_CMP_STATUS_MASK;
}

/**
 * copy/set pattern/compare memory region from source to destination using a dma engine
 *
 * @param    op              wanted operation mode (copy, compare, set pattern)
 * @param    src             start addresses of source memory regions per dma
 *                           when operation is defined as compare, this variable used to pattern value to set
 * @param    dst             start addresses of destination memory regions per dma
 * @param    size            size of memory region to copy
 * @param    descAddr        absolute address to store the descriptor
 *
 * @retval 0 success (in compare: 0 - equal, 1 - not equal)
 */
int srvCpuXorDmaOperation(enum mv_xor_v2_desc_op_mode op, MV_U32 *src, MV_U32 *dst, MV_U32 size, MV_U32 descAddr)
{
    MV_U32 dma_id = MV_XOR_DEFAULT_ENGINE;
    int ret = 0;
    /* initialize dma descriptor queue memory region */
    memset(&qmem, 0, sizeof(qmem));

    /* initialize dma engine */
    mv_xor_v2_init(dma_id, descAddr);

    /* prepare dma hw descriptor */
    mv_xor_v2_desc_prep(dma_id, op,
                src, dst, size);

    /* enqueue dma descriptor to start processing */
    mv_xor_v2_enqueue(dma_id);

    /* wait for transfer completion */
    do {
        srvCpuOsUsDelay(5);
    }while (mv_xor_v2_done(dma_id) != MV_XOR_DEFUALT_DESC_NUM);

    /* disable dma engine */
    mv_xor_v2_finish(dma_id);

    if(op == DESC_OP_MODE_MEMCMP)
    {
        if (mv_xor_v2_memcmp_status_get(dma_id) == 0)
            ret++;
    }

    return ret;
}

static void parse64bitTo32(char* value, char *buf, char *buf32_h, char *buf32_l)
{
    int i, index=0, len = 0;

    for(len = 0; value[len]!=0;len++);
    memcpy(buf,value,len);

    if(len > 8)
    {
        for(i=0;i<len-8;i++)
            buf32_h[i]=buf[i];

        if((buf[0]=='0') && (buf[1]=='x' || buf[1]=='X'))
        {
            memcpy(buf32_l,"0x",2);
            index = 2;
        }

        for(i=len-8; i<len; i++,index++)
            buf32_l[index] = buf[i];
    }
    else
    {
        for(i=0;i<len;i++)
            buf32_l[i]=buf[i];
    }
    return;
}

#ifdef XOR_DMA_CLI
/*******************************************************************************
* prvXorDmaCommand -  This routine parse cpy/cmp (using xor dma) commands
*******************************************************************************/
static portBASE_TYPE prvXorDmaParseCommand( int8_t *pcWriteBuffer,
                                       size_t xWriteBufferLen,
                                       const int8_t *pcCommandString )
{

    /* src - in cpy/cmp is used as source, in set_pattern is used as pattern*/
    MV_U32 src[2] = {0}, dst[2] = {0}, size = 0;
    enum {XOR_DMA_CMD_TYPE_COPY, XOR_DMA_CMD_TYPE_COMPARE, XOR_DMA_CMD_TYPE_SET_PATTERN} xor_dma_cmd_type;
    char *value;
    int ret;
    char buf[10] = {0}, buf32_l[10] = {0},buf32_h[10] ={0};

    value = strtok( ( char * )&pcCommandString[0], DELIMETERS );
    /* Get the command type. */
    if ( (value == NULL) || ( strcmp(value, "xor_dma") ) )
        goto parseXorDmaCommand_fail;

    /* Get the command type: cpy/cmp. */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseXorDmaCommand_fail;
    if (!strcmp(value, "cpy"))
       xor_dma_cmd_type = XOR_DMA_CMD_TYPE_COPY;
    else if (!strcmp(value, "cmp"))
        xor_dma_cmd_type = XOR_DMA_CMD_TYPE_COMPARE;
    else if (!strcmp(value, "set_pattern"))
        xor_dma_cmd_type = XOR_DMA_CMD_TYPE_SET_PATTERN;
    else
        goto parseXorDmaCommand_fail;

    /* Source / Pattern */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseXorDmaCommand_fail;
    /* getting number in 64 bit and parse it to two 32 bits variables */
    parse64bitTo32(value, buf, buf32_h, buf32_l);
    src[0] = (MV_U32)strtoul(buf32_h, NULL, 0);
    src[1] = (MV_U32)strtoul(buf32_l, NULL, 0);

    /* Destination */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseXorDmaCommand_fail;
    /* getting number in 64 bit and parse it to two 32 bits variables */
    parse64bitTo32(value, buf, buf32_h, buf32_l);
    dst[0] = (MV_U32)strtoul(buf32_h, NULL, 0);
    dst[1] = (MV_U32)strtoul(buf32_l, NULL, 0);

    /* Size */
    value = strtok( NULL, DELIMETERS );
    if ((value == NULL) || (!isdigit(value[0])))
        goto parseXorDmaCommand_fail;
    size = (MV_U32)strtoul(value, &value, 0);

    if (xor_dma_cmd_type == XOR_DMA_CMD_TYPE_COPY)
    {
        ret = srvCpuXorDmaOperation(DESC_OP_MODE_MEMCPY, src, dst, size, 0x7FE00000);
        if (!ret)
            printf("Succeed copy using XOR_DMA engine\n");
        else
            printf("Failed to copy using XOR_DMA engine\n",src, dst);
    }
    else if(xor_dma_cmd_type == XOR_DMA_CMD_TYPE_COMPARE)
    {
        ret = srvCpuXorDmaOperation(DESC_OP_MODE_MEMCMP, src, dst, size, 0x7FE00000);
        if (!ret)
            printf("XOR DMA Result:\nThe two memory addresses are equal\n");
        else
            printf("XOR DMA Result:\n There are differences between the two memory addresses\n");
    }
    else
    {/* xor_dma_cmd_type == XOR_DMA_CMD_TYPE_SET_PATTERN  */
        ret = srvCpuXorDmaOperation(DESC_OP_MODE_MEMSET, src, dst, size, 0x7FE00000);
        if (!ret)
            printf("Succeed to set pattern using XOR_DMA engine\n");
        else
            printf("Failed to set pattern using XOR_DMA engine\n");
    }
    return pdFALSE;
parseXorDmaCommand_fail:
    printf( "error: Wrong input. Usage:\n xor_dma cpy/cmp/set_pattern <src|pattern> <dst> <size>\n" );
    return pdFALSE;
}

static const CLI_Command_Definition_t xXorDmaCommand =
{
    ( const int8_t * const ) "xor_dma",
    ( const int8_t * const ) "xor_dma cpy/cmp/set_pattern <src|pattern> <dst> <size>\n"
            " Copy/Compare/Set Pattern (64bit), using XOR DMA engine\n\n",
    prvXorDmaParseCommand,
    -1
};

void regiserXorDmaCommand()
{
    FreeRTOS_CLIRegisterCommand( &xXorDmaCommand);
}
#endif /* XOR_DMA_CLI */