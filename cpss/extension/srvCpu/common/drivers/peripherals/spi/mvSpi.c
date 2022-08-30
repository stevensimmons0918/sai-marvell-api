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

#include "mvSpi.h"
#include "mvSpiSpec.h"
#include "printf.h"

/*#define MV_DEBUG*/
#ifdef MV_DEBUG
#define DB(x) x
#else
#define DB(x)
#endif

/*******************************************************************************
 * mvSpi16bitDataTxRx - Transmt and receive data
 *
 * DESCRIPTION:
 *       Tx data and block waiting for data to be transmitted
 *
 ********************************************************************************/
static MV_STATUS mvSpi16bitDataTxRx (MV_U16 txData, MV_U16 * pRxData)
{
    MV_U32 i;
    MV_BOOL ready = MV_FALSE;

    /* First clear the bit in the interrupt cause register */
    MV_REG_WRITE(MV_SPI_INT_CAUSE_REG, 0x0);

    /* Transmit data */
    MV_REG_WRITE(MV_SPI_DATA_OUT_REG, MV_16BIT_LE(txData));

    /* wait with timeout for memory ready */
    for (i=0; i<MV_SPI_WAIT_RDY_MAX_LOOP; i++)
    {
        if (MV_REG_READ(MV_SPI_INT_CAUSE_REG))
        {
            ready = MV_TRUE;
            break;
        }
#ifdef MV_SPI_SLEEP_ON_WAIT
        mvOsSleep(1);
#endif /* MV_SPI_SLEEP_ON_WAIT */
    }

    if (!ready)
        return MV_TIMEOUT;

    /* check that the RX data is needed */
    if (pRxData)
    {
        if ((MV_U32)pRxData &  0x1) /* check if address is not alligned to 16bit */
        {
#if defined(MV_CPU_LE)
            /* perform the data write to the buffer in two stages with 8bit each */
            MV_U8 * bptr = (MV_U8*)pRxData;
            MV_U16 data = MV_16BIT_LE(MV_REG_READ(MV_SPI_DATA_IN_REG));
            *bptr = (data & 0xFF);
            ++bptr;
            *bptr = ((data >> 8) & 0xFF);

#elif defined(MV_CPU_BE)

            /* perform the data write to the buffer in two stages with 8bit each */
            MV_U8 * bptr = (MV_U8 *)pRxData;
            MV_U16 data = MV_16BIT_LE(MV_REG_READ(MV_SPI_DATA_IN_REG));
            *bptr = ((data >> 8) & 0xFF);
            ++bptr;
            *bptr = (data & 0xFF);

#else
#error "CPU endianess isn't defined!\n"
#endif

        }
        else
            *pRxData = MV_16BIT_LE(MV_REG_READ(MV_SPI_DATA_IN_REG));
    }

    return MV_OK;
}


/*******************************************************************************
 * mvSpi8bitDataTxRx - Transmt and receive data (8bits)
 *
 * DESCRIPTION:
 *       Tx data and block waiting for data to be transmitted
 *
 ********************************************************************************/
static MV_STATUS mvSpi8bitDataTxRx (MV_U8 txData, MV_U8 * pRxData)
{
    MV_U32 i;
    MV_BOOL ready = MV_FALSE;

    /* set the mode for 8bits temporarly */
    MV_REG_BIT_RESET(MV_SPI_IF_CONFIG_REG, MV_SPI_BYTE_LENGTH_MASK);

    /* First clear the bit in the interrupt cause register */
    MV_REG_WRITE(MV_SPI_INT_CAUSE_REG, 0x0);

    /* Transmit data */
    MV_REG_WRITE(MV_SPI_DATA_OUT_REG, txData);

    /* wait with timeout for memory ready */
    for (i=0; i<MV_SPI_WAIT_RDY_MAX_LOOP; i++)
    {
        if (MV_REG_READ(MV_SPI_INT_CAUSE_REG))
        {
            ready = MV_TRUE;
            break;
        }
#ifdef MV_SPI_SLEEP_ON_WAIT
        mvOsSleep(1);
#endif /* MV_SPI_SLEEP_ON_WAIT */
    }

    /* set the mode back to the default 16bit */
    MV_REG_BIT_SET(MV_SPI_IF_CONFIG_REG, MV_SPI_BYTE_LENGTH_MASK);

    if (!ready)
        return MV_TIMEOUT;

    /* check that the RX data is needed */
    if (pRxData)
        *pRxData = MV_REG_READ(MV_SPI_DATA_IN_REG);

    return MV_OK;
}

/*
#####################################################################################
#####################################################################################
*/

/*******************************************************************************
 * mvSpiInit - Initialize the SPI controller
 *
 * DESCRIPTION:
 *       Perform the neccessary initialization in order to be able to send an
 *       receive over the SPI interface.
 *
 * INPUT:
 *       serialBaudRate: Baud rate (SPI clock frequency)
 *       use16BitMode: Whether to use 2bytes (MV_TRUE) or 1bytes (MV_FALSE)
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       Success or Error code.
 *
 *
 *******************************************************************************/
MV_STATUS mvSpiInit (MV_U32 serialBaudRate)
{
    MV_STATUS ret;

    /* Set the serial clock */
    if ((ret = mvSpiBaudRateSet(serialBaudRate)) != MV_OK)
        return ret;

    /* Configure the default SPI mode to be 16bit */
    MV_REG_BIT_SET(MV_SPI_IF_CONFIG_REG, MV_SPI_BYTE_LENGTH_MASK);

    /* Verify that the CS is deasserted */
    mvSpiCsDeassert();

    return MV_OK;
}

/*******************************************************************************
 * mvSpiBaudRateSet - Set the Frequency of the SPI clock
 *
 * DESCRIPTION:
 *       Set the Prescale bits to adapt to the requested baud rate (the clock
 *       used for thr SPI).
 *
 * INPUT:
 *       serialBaudRate: Baud rate (SPI clock frequency)
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       Success or Error code.
 *
 *
 *******************************************************************************/
MV_STATUS mvSpiBaudRateSet (MV_U32 serialBaudRate)
{
    MV_U32 minBaudOffset = 0xFFFFFFFF;
    MV_U32 tempReg,cpuClk;

    MV_U32 spr, sppr;
    MV_U32 divider;
    MV_U32 bestSpr = 0, bestSppr = 0;
    MV_U8 exactMatch = 0;

    cpuClk = MV_BOARD_TCLK;

    /* Find the best prescale configuration - less or equal */
    for (spr = 1; spr <= 15; spr++) {
        for (sppr = 0; sppr <= 7; sppr++) {
            divider = spr * (1 << sppr);
            /* check for higher - irrelevent */
            if ((cpuClk / divider) > serialBaudRate)
                continue;

            /* check for exact fit */
            if ((cpuClk / divider) == serialBaudRate) {
                bestSpr = spr;
                bestSppr = sppr;
                exactMatch = 1;
                break;
            }

            /* check if this is better than the previous one */
            if ((serialBaudRate - (cpuClk / divider)) < minBaudOffset) {
                minBaudOffset = (serialBaudRate - (cpuClk / divider));
                bestSpr = spr;
                bestSppr = sppr;
            }
        }

        if (exactMatch == 1)
            break;
    }

    if (bestSpr == 0) {
        return MV_OUT_OF_RANGE;
    }

    /* configure the Prescale */
    tempReg = MV_REG_READ(MV_SPI_IF_CONFIG_REG) & ~(MV_SPI_SPR_MASK | MV_SPI_SPPR_0_MASK |
            MV_SPI_SPPR_HI_MASK | MV_SPI_DIRECT_RD_HS_MASK );
    tempReg |= ((bestSpr << MV_SPI_SPR_OFFSET) |
            ((bestSppr & 0x1) << MV_SPI_SPPR_0_OFFSET) |
            ((bestSppr >> 1) << MV_SPI_SPPR_HI_OFFSET));
    MV_REG_WRITE(MV_SPI_IF_CONFIG_REG, tempReg);

    return MV_OK;
}

/*******************************************************************************
 * mvSpiCsAssert - Assert the Chip Select pin indicating a new transfer
 *
 * DESCRIPTION:
 *       Assert The chip select - used to select an external SPI device
 *
 * INPUT:
 *       None.
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       Success or Error code.
 *
 ********************************************************************************/
MV_VOID mvSpiCsAssert(MV_VOID)
{
    MV_REG_BIT_SET(MV_SPI_IF_CTRL_REG, MV_SPI_CS_ENABLE_MASK);
}

/*******************************************************************************
 * mvSpiCsDeassert - DeAssert the Chip Select pin indicating the end of a
 *                   SPI transfer sequence
 *
 * DESCRIPTION:
 *       DeAssert the chip select pin
 *
 * INPUT:
 *       None.
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       Success or Error code.
 *
 ********************************************************************************/
MV_VOID mvSpiCsDeassert(MV_VOID)
{
    MV_REG_BIT_RESET(MV_SPI_IF_CTRL_REG, MV_SPI_CS_ENABLE_MASK);
}

/*******************************************************************************
 * mvSpiRead - Read a buffer over the SPI interface
 *
 * DESCRIPTION:
 *       Receive (read) a buffer over the SPI interface in 16bit chunks. If buffer
 *       points to an odd address, then the first chunk will be 8bit. If last
 *       byte is in odd offset, then the last chunk will be 8bits. Chip select is
 *       not handled at this level.
 *
 * INPUT:
 *       pRxBuff: Pointer to the buffer to hold the received data
 *       buffSize: length of the pRxBuff
 *
 * OUTPUT:
 *       pRxBuff: Pointer to the buffer with the received data
 *
 * RETURN:
 *       Success or Error code.
 *
 *
 *******************************************************************************/
MV_STATUS mvSpiRead (MV_U8* pRxBuff, MV_U32 buffSize)
{

    MV_STATUS ret;
    MV_U32 bytesLeft = buffSize;
    MV_U16* rxPtr;

    /* check for null parameters */
    if (pRxBuff == NULL)
    {
        return MV_BAD_PARAM;
    }

    /* If buffer pointer is odd - receive the 1st byte */
    if ( ((MV_U32)pRxBuff & 1) == 1)
    {
        /* Transmit and wait for the transfer to be completed */
        if ((ret = mvSpi8bitDataTxRx(MV_SPI_DUMMY_WRITE_8BITS, (MV_U8*)pRxBuff)) != MV_OK)
            return ret;
        pRxBuff++;
        bytesLeft--;
    }

    rxPtr = (MV_U16*)((MV_U32)pRxBuff);

    /* Verify that the SPI mode is in 16bit mode */
    MV_REG_BIT_SET(MV_SPI_IF_CONFIG_REG, MV_SPI_BYTE_LENGTH_MASK);

    /* TX/RX as long we have complete 16bit chunks */
    while (bytesLeft >= MV_SPI_16_BIT_CHUNK_SIZE)
    {
        /* Transmitted and wait for the transfer to be completed */
        if ((ret = mvSpi16bitDataTxRx(MV_SPI_DUMMY_WRITE_16BITS, rxPtr)) != MV_OK)
            return ret;

        /* increment the pointers */
        rxPtr++;
        bytesLeft -= MV_SPI_16_BIT_CHUNK_SIZE;
    }

    /* check if the buffer size is odd */
    if (bytesLeft)
    {
        /* Transmitted and wait for the transfer to be completed */
        if ((ret = mvSpi8bitDataTxRx(MV_SPI_DUMMY_WRITE_8BITS, (MV_U8*)rxPtr)) != MV_OK)
            return ret;
    }

    return MV_OK;
}

/*******************************************************************************
 * mvSpiWrite - Transmit a buffer over the SPI interface
 *
 * DESCRIPTION:
 *       Transmit a buffer over the SPI interface in 16bit chunks. If buffer
 *       points to an odd address, then the first chunk will be 8bit. If last
 *       byte is in odd offset, then the last chunk will be 8bits. No chip
 *       select action is taken.
 *
 * INPUT:
 *       pTxBuff: Pointer to the buffer holding the TX data
 *       buffSize: length of the pTxBuff
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       Success or Error code.
 *
 *
 *******************************************************************************/
MV_STATUS mvSpiWrite(MV_U8* pTxBuff, MV_U32 buffSize)
{
    MV_STATUS ret;
    MV_U32 bytesLeft = buffSize;
    MV_U16* txPtr;

    /* check for null parameters */
    if (pTxBuff == NULL)
    {
        return MV_BAD_PARAM;
    }

    /* If buffer pointer is odd - transmit the 1st byte */
    if ((MV_U32)pTxBuff % 2)
    {
        /* Transmit and wait for the transfer to be completed */
        if ((ret = mvSpi8bitDataTxRx(*((MV_U8*)pTxBuff), NULL)) != MV_OK)
            return ret;
        pTxBuff++;
        bytesLeft--;
    }

    txPtr = (MV_U16*)((MV_U32)pTxBuff);

    /* Verify that the SPI mode is in 16bit mode */
    MV_REG_BIT_SET(MV_SPI_IF_CONFIG_REG, MV_SPI_BYTE_LENGTH_MASK);

    /* TX/RX as long we have complete 16bit chunks */
    while (bytesLeft >= MV_SPI_16_BIT_CHUNK_SIZE)
    {
        /* Transmitted and wait for the transfer to be completed */
        if ((ret = mvSpi16bitDataTxRx(*txPtr, NULL)) != MV_OK)
            return ret;

        /* increment the pointers */
        txPtr++;
        bytesLeft -= MV_SPI_16_BIT_CHUNK_SIZE;
    }

    /* check if the buffer size is odd */
    if (bytesLeft)
    {
        /* Transmitted and wait for the transfer to be completed */
        if ((ret = mvSpi8bitDataTxRx(*((MV_U8*)txPtr), NULL)) != MV_OK)
            return ret;
    }

    return MV_OK;
}


/*******************************************************************************
 * mvSpiReadWrite - Read and Write a buffer simultanuosely
 *
 * DESCRIPTION:
 *       Transmit and receive a buffer over the SPI in 16bit chunks. If buffers
 *       point to an odd address, then the first chunks will be 8bit. If last
 *       byte is in odd offset, then the last chunks will be 8bits. The SPI chip
 *       select is not handled implicitely.
 *
 * INPUT:
 *       pRxBuff: Pointer to the buffer to write the RX info in
 *       pTxBuff: Pointer to the buffer holding the TX info
 *       buffSize: length of both the pTxBuff and pRxBuff
 *
 * OUTPUT:
 *       pRxBuff: Pointer of the buffer holding the RX data
 *
 * RETURN:
 *       Success or Error code.
 *
 *
 *******************************************************************************/
MV_STATUS mvSpiReadWrite(MV_U8* pRxBuff, MV_U8* pTxBuff, MV_U32 buffSize)
{
    MV_STATUS ret;
    MV_U32 bytesLeft = buffSize;
    MV_U16 *txPtr, *rxPtr;

    /* check for null parameters */
    if ((pRxBuff == NULL) || (pTxBuff == NULL))
    {
        return MV_BAD_PARAM;
    }

    /* If buffer pointers are odd - transmit the 1st byte */
    if (((MV_U32)pTxBuff % 2) || ((MV_U32)pRxBuff % 2))
    {
        /* If one is odd and the other even - error */
        if (!((MV_U32)pTxBuff % 2) || !((MV_U32)pRxBuff % 2))
            return MV_FAIL;
        /* Transmit and wait for the transfer to be completed */
        if ((ret = mvSpi8bitDataTxRx(*((MV_U8*)pTxBuff), (MV_U8*)pRxBuff)) != MV_OK)
            return ret;
        pTxBuff++;
        pRxBuff++;
        bytesLeft--;
    }

    txPtr = (MV_U16*)((MV_U32)pTxBuff);
    rxPtr = (MV_U16*)((MV_U32)pRxBuff);

    /* Verify that the SPI mode is in 16bit mode */
    MV_REG_BIT_SET(MV_SPI_IF_CONFIG_REG, MV_SPI_BYTE_LENGTH_MASK);

    /* TX/RX as long we have complete 16bit chunks */
    while (bytesLeft >= MV_SPI_16_BIT_CHUNK_SIZE)
    {
        /* Transmitted and wait for the transfer to be completed */
        if ((ret = mvSpi16bitDataTxRx(*txPtr, rxPtr)) != MV_OK)
            return ret;

        /* increment the pointers */
        txPtr++;
        rxPtr++;
        bytesLeft -= MV_SPI_16_BIT_CHUNK_SIZE;
    }

    /* check if the buffer size is odd */
    if (bytesLeft)
    {
        /* Transmitted and wait for the transfer to be completed */
        if ((ret = mvSpi8bitDataTxRx(*((MV_U8*)txPtr), (MV_U8*)rxPtr)) != MV_OK)
            return ret;
    }

    return MV_OK;
}
