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
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "mvSFlash.h"
#include "mvSFlashSpec.h"
#include "mvSpi.h"
#include "mvSpiSpec.h"
#include "mvSpiCmnd.h"
#include "printf.h"

#ifndef CONFIG_FALCON
#include "mv_qspi.h"
#else
#define srvCpuQspiRead(x, ...) x
#define srvCpuQspiWrite(x, ...) x
#define srvCpuQspiInit(x, ...) x
#endif

#ifdef MICRO_INIT
#include <task.h>
#define vTaskDelay(X)         vTaskDelay( X / portTICK_RATE_MS ) // Xmsec
#define mvOsOutputString      mvOsPrintf
#else
#define vTaskDelay(X)       srvCpuOsMsDelay(X)    // Xmsec
#endif

#undef MV_DEBUG
#ifdef MV_DEBUG
#define DB(x) x
#else
#define DB(x)
#endif

#define ARRAY_SIZE ( sizeof(sFlashTypes) / sizeof(SFLASH_TYPE_TABLE) )


/* Globals */
/******************************************************************************
 * This table contain the supported SPI Flash devices information needed by  the driver:
 *
 * Entry`s structure: (all values are in decimal)
 *           -Manufacture ID,
 *           -Device ID,
 *           -number of sectors
 *           -size of sectors (Sector sizes are in kb).
 *           -page size
 * The end of the list is pointed with a zero.
 ******************************************************************************/

static SFLASH_TYPE_TABLE sFlashTypes[] = {
    /*      4_byte     ven_id,        dev_id,    num of   size of  page
            addr_mode                           sectors,  block,   size */
    /* 0 */  {0,      ST_FLASH,       M25P32,        64,      64,   MV_M25P_PAGE_SIZE },  /* 4MB  */
    /* 1 */  {0,      ST_FLASH,       N25Q032,       64,      64,   MV_M25P_PAGE_SIZE },  /* 4MB  */
    /* 2 */  {0,      ST_FLASH,       M25P64,       128,      64,   MV_M25P_PAGE_SIZE },  /* 8MB  */
    /* 3 */  {0,      ST_FLASH,       M25P128,       64,     256,   MV_M25P_PAGE_SIZE },  /* 16MB */
    /* 4 */  {1,      ST_FLASH,       N25Q256,      512,      64,   MV_M25P_PAGE_SIZE },  /* 32MB */
    /* 5 */  {0,      ST_FLASH,       N25Q128_3_0V, 256,      64,   MV_M25P_PAGE_SIZE },  /* 16MB */
    /* 6 */  {0,      ST_FLASH,       N25Q128_1_8V, 256,      64,   MV_M25P_PAGE_SIZE },  /* 16MB */
    /* 7 */  {1,      ST_FLASH,       MT25QU256,    512,      64,   MV_M25P_PAGE_SIZE },  /* 32MB */
    /* 8 */  {0,      MACRONIX_FLASH, MX25L3206E,    64,      64,   MV_MXIC_PAGE_SIZE },  /* 4MB */
    /* 9 */  {0,      MACRONIX_FLASH, MX25L6405,    128,      64,   MV_MXIC_PAGE_SIZE },  /* 8MB  */
    /* 10 */ {0,      MACRONIX_FLASH, MX25L12845,   256,      64,   MV_MXIC_PAGE_SIZE },  /* 16MB */
    /* 11 */ {1,      MACRONIX_FLASH, MX25L25635,   512,      64,   MV_MXIC_PAGE_SIZE },  /* 32MB */
    /* 12 */ {0,      SPANSION_FLASH, 0x215,         64,      64,   MV_S25FL_PAGE_SIZE},  /* 4MB */
    /* 13 */ {0,      SPANSION_FLASH, S25FL128P,     64,     256,   MV_S25FL_PAGE_SIZE},  /* 16MB */

    /* Although S25FL256S support 4B Address mode - it can be function also as 3B Address mode
       Therefore value 0 is set for the FLASH_4BYTES_ADDRESS field */
    /* 13 */ {0,      SPANSION_FLASH, S25FL256S,  128,     256,   2*MV_S25FL_PAGE_SIZE},  /* 32MB */
    /* GD25Q41B Larch*/
    /* 14 */ {0,      GIGA_FLASH,     GD25Q41B,   128,       4,   MV_GD25Q41B_PAGE_SIZE},  /* 4M-bit */

    /* Added 1/10/2017 */
    /* 15 */ {0,      SPANSION_FLASH, S25FL164K,  128,      64,   MV_S25FL_PAGE_SIZE},  /* 8MB */
    /* 16 */ {0,      MACRONIX_FLASH, MX25L6405,  128,      64,   MV_MXIC_PAGE_SIZE},  /* 8MB  */

    /* Added 3/15/2017 */
    /* 17 */ {0,      MACRONIX_FLASH, MX25U25635, 512,      64,   MV_MXIC_PAGE_SIZE},  /* 32MB */

    /* WINBOND_FLASH */
    /* 18 */ {0,      WINBOND_FLASH, 0x3013,     128,       4,   256},  /* 512K  */
    /* 19 */ {0,      WINBOND_FLASH, W25Q128FV,  256,      64,   MV_W25Q128FV_PAGE_SIZE},  /* 128MB */

    /* 20 */ {0,      SPANSION_FLASH, S25FL064L,  128,      64,   2*MV_S25FL_PAGE_SIZE},  /* 8MB */
    /* 21 */ {0,      GIGA_FLASH,     GD25Q32C,   64,       64,   MV_GD25Q41B_PAGE_SIZE},  /* 4MB */
    /* 22 */ {0,      WINBOND_FLASH, W25Q32JVS,  64,       64,   MV_W25Q128FV_PAGE_SIZE},  /* 32M-bit */
    /* 23 */ {0,      ISSI_FLASH,     IS25LP032D, 64,       64,   MV_IS25LP032D_PAGE_SIZE},  /* 32M-bit */

    /* 24 */ {0,      SPANSION_FLASH, S25FL128L,  256,      64,   2*MV_S25FL_PAGE_SIZE},  /* 16MB */

    /* 25 */ {0,      WINBOND_FLASH, W25Q64JVS,  128,      64,   MV_W25Q128FV_PAGE_SIZE},  /* 8MB */
    /* 26 */ {0,      WINBOND_FLASH, W25Q128JVS, 256,      64,   MV_W25Q128FV_PAGE_SIZE},  /* 16MB */
};

/*global variables to be used only from within this module:*/
/***********************************************************/

static MV_U8    flashIdx   = 0;                           /* Global index into FlashTypes array. Makes finding things easier */
static MV_U32   sFlashBaseAddress = 0x0;                  /* Base address of sFlash. */
static MV_BOOL  FlashInitFlag     = MV_FALSE;             /* Global flag - was the flash initialized? */
static MV_BOOL  FlashExtdAddress  = MV_FALSE;             /* Do we have to use extended 4-byte addressing */
static MV_U8    VendorID          = 0;
static MV_U16   DeviceID          = 0;
static MV_SFLASH_SPI_IF_TYPE ifType = MV_SPI_IF_SPI;      /* Type of SPI controller */

/* Static Functions */
static MV_STATUS    mvWriteEnable   (void);
static MV_STATUS    mvStatusRegGet  (MV_U8 * pStatReg);
static MV_STATUS    mvStatusRegSet  (MV_U8 sr);
static MV_STATUS    mvWaitOnWipClear(void);
static MV_STATUS    mvSFlashPageWr  (MV_U32 offset, MV_U8* pPageBuff, MV_U32 buffSize);
static MV_STATUS    mvSFlashWithDefaultsIdGet (MV_U8* manId, MV_U16* devId);

MV_STATUS           mvSFlashSet4Byte (MV_BOOL enable);

/*******************************************************************************
 * mvSFlashSectorSizeGet
 *
 * DESCRIPTION:
 *       get sector size of flash
 *
 * INPUT:
 *        None
 *
 * OUTPUT:
 *       MV_U32
 *
 * RETURN:
 *       Success or Error code.
 *
 *******************************************************************************/
MV_U32 mvSFlashSectorSizeGet ()
{
    return (sFlashTypes[flashIdx].sectorSize * 1024);
}


/*******************************************************************************
 * mvWriteEnable - serialize the write enable sequence
 *
 * DESCRIPTION:
 *       transmit the sequence for write enable
 *
 ********************************************************************************/
static MV_STATUS mvWriteEnable(void)
{
    MV_U8 cmd[MV_SFLASH_WREN_CMND_LENGTH];

    cmd[0] = MV_M25P_WREN_CMND_OPCD;      /* Write Enable - Identical command for all vendors. */

    return (ifType == MV_SPI_IF_SPI) ? 
        mvSpiWriteThenRead(cmd, MV_SFLASH_WREN_CMND_LENGTH, NULL, 0, 0) :
        srvCpuQspiWrite(0, 0, 0, 0, 0, (void *)cmd, MV_SFLASH_WREN_CMND_LENGTH);

}

/*******************************************************************************
 * mvStatusRegGet - Retrieve the value of the status register
 *
 * DESCRIPTION:
 *       perform the RDSR sequence to get the 8bit status register
 *
 ********************************************************************************/
static MV_STATUS mvStatusRegGet(MV_U8 * pStatReg)
{
    MV_STATUS ret;
    MV_U8 cmd[MV_SFLASH_RDSR_CMND_LENGTH];
    MV_U8 sr [MV_SFLASH_RDSR_REPLY_LENGTH];

    cmd[0] = MV_M25P_RDSR_CMND_OPCD;     /* Read Status Register - Identical command for all vendors. */

    ret = (ifType == MV_SPI_IF_SPI) ?
        mvSpiWriteThenRead(cmd, MV_SFLASH_RDSR_CMND_LENGTH, sr, MV_SFLASH_RDSR_REPLY_LENGTH,0) :
        srvCpuQspiRead(0, cmd[0], 0, 0, 0, (void *)sr, MV_SFLASH_RDSR_REPLY_LENGTH);

    if (ret != MV_OK)
        return ret;

    *pStatReg = sr[0];

    return MV_OK;
}

/*******************************************************************************
 * mvWaitOnWipClear - Block waiting for the WIP (write in progress) to be cleared
 *
 * DESCRIPTION:
 *       Block waiting for the WIP (write in progress) to be cleared
 *
 ********************************************************************************/
static MV_STATUS mvWaitOnWipClear(void)
{
    MV_STATUS ret;
    MV_U32 i;
    MV_U8 stat;

    for (i=0; i<MV_SFLASH_MAX_WAIT_LOOP; i++)
    {
        if ((ret = mvStatusRegGet(&stat)) != MV_OK)
            return ret;

        if ((stat & MV_SFLASH_STATUS_REG_WIP_MASK) == 0)
            return MV_OK;
        vTaskDelay( 1 ); // 1msec
    }

    DB(mvOsOutputString("%s WARNING: Write Timeout!\n", __func__);)
        return MV_TIMEOUT;
}

#ifndef MICRO_INIT
/*******************************************************************************
 * mvWaitOnChipEraseDone - Block waiting for the WIP (write in progress) to be
 *                         cleared after a chip erase command which is supposed
 *                         to take about 2:30 minutes
 *
 * DESCRIPTION:
 *       Block waiting for the WIP (write in progress) to be cleared
 *
 ********************************************************************************/
static MV_STATUS mvWaitOnChipEraseDone(void)
{
    MV_STATUS ret;
    MV_U32 i;
    MV_U8 stat;

    for (i=0; i<MV_SFLASH_CHIP_ERASE_MAX_WAIT_LOOP; i++)
    {
        if ((ret = mvStatusRegGet(&stat)) != MV_OK)
            return ret;

        if ((stat & MV_SFLASH_STATUS_REG_WIP_MASK) == 0)
            return MV_OK;
    }

    DB(mvOsOutputString("%s WARNING: Write Timeout!\n", __func__);)
        return MV_TIMEOUT;
}
#endif // MICRO_INIT

/*******************************************************************************
 *  mvStatusRegSet - Set the value of the 8bit status register
 *
 * DESCRIPTION:
 *       Set the value of the 8bit status register
 *
 ********************************************************************************/
static MV_STATUS mvStatusRegSet(MV_U8 sr)
{
    MV_STATUS ret;
    MV_U8 cmd[MV_SFLASH_WRSR_CMND_LENGTH];

    /* Issue the Write enable command prior the WRSR command */
    if ((ret = mvWriteEnable()) != MV_OK)
        return ret;

    /* Write the SR with the new values */
    cmd[0] = MV_M25P_WRSR_CMND_OPCD;     /* Write Status Register - Identical command for all vendors. */
    cmd[1] = sr;


    return (ifType == MV_SPI_IF_SPI) ? 
        mvSpiWriteThenRead(cmd, MV_SFLASH_WRSR_CMND_LENGTH, NULL, 0, 0) :
        srvCpuQspiWrite(0, cmd[0], 0, 0, 0, (void *)&cmd[1], MV_SFLASH_WRSR_CMND_LENGTH - 1);
    if (ret != MV_OK)
        return ret;

    if ((ret = mvWaitOnWipClear()) != MV_OK)
        return ret;

    vTaskDelay( 1 ); // 1msec

    return MV_OK;
}

/*******************************************************************************
 * mvSFlashPageWr - Write up to 256 Bytes in the same page
 *
 * DESCRIPTION:
 *       Write a buffer up to the page size in length provided that the whole address
 *        range is within the same page (alligned to page bounderies)
 *
 *******************************************************************************/
static MV_STATUS mvSFlashPageWr (MV_U32 offset, MV_U8* pPageBuff, MV_U32 buffSize)
{
    MV_STATUS ret;
    MV_U8 cmd[MV_SFLASH_PP_CMND_LENGTH];
    MV_U8 n;
    int i = 24;

    /* check that we do not cross the page bounderies */
    if (((offset & (sFlashTypes[flashIdx].pageSize - 1)) + buffSize) >
            sFlashTypes[flashIdx].pageSize)
    {
        DB(mvOsOutputString("%s WARNING: Page allignment problem!\n", __func__);)
            return MV_OUT_OF_RANGE;
    }

    /* Issue the Write enable command prior the page program command */
    if ((ret = mvWriteEnable()) != MV_OK)
        return ret;

    n = 0;
    cmd[n++] = MV_M25P_PP_CMND_OPCD;         /* Page Program - Identical command for all vendors. */

    /* If 3 byte addressing, need only shift of 16, 8 and 0 */
    if(!FlashExtdAddress)
        i -= 8;

    for (; i >= 0; i -= 8)
        cmd[n++] = ((offset >> i) & 0xFF);

    ret = (ifType == MV_SPI_IF_SPI) ?
        mvSpiWriteThenWrite(cmd, n, pPageBuff, buffSize) :
        srvCpuQspiWrite(0, cmd[0], offset, n - 1, 0, (void *)pPageBuff, buffSize);
    if (ret != MV_OK)
        return ret;

    if ((ret = mvWaitOnWipClear()) != MV_OK)
        return ret;

    return MV_OK;
}

/*******************************************************************************
 * mvSFlashWithDefaultsIdGet - Try to read the manufacturer and Device IDs from
 *       the device using the default RDID opcode and the default WREN opcode.
 *
 * DESCRIPTION:
 *       This is used to detect a generic device that uses the default opcodes
 *       for the WREN and RDID.
 *
 ********************************************************************************/
static MV_STATUS mvSFlashWithDefaultsIdGet (MV_U8* manId, MV_U16* devId)
{
    MV_STATUS ret;
    MV_U8 cmdRDID[MV_SFLASH_RDID_CMND_LENGTH];
    MV_U8 id[MV_SFLASH_RDID_REPLY_LENGTH];

    cmdRDID[0] = MV_SFLASH_DEFAULT_RDID_OPCD;   /* unknown model try default */

    /* Use the default RDID opcode to read the IDs */
    ret = (ifType == MV_SPI_IF_SPI) ?
        mvSpiWriteThenRead(cmdRDID, MV_SFLASH_RDID_CMND_LENGTH, id, MV_SFLASH_RDID_REPLY_LENGTH, 0) :
        srvCpuQspiRead(0, cmdRDID[0], 0, 0, 0, (void *)id, MV_SFLASH_RDID_REPLY_LENGTH);

    if (ret != MV_OK)
        return ret;

    *manId = id[0];
    *devId = (MV_U16)((id[1] << 8) | id[2]);

    return MV_OK;
}

/*
#####################################################################################
#####################################################################################
*/

/*******************************************************************************
 * mvSFlashSet4Byte - Set 4-byte addressing mode
 *
 * DESCRIPTION:
 *       Sets (enters or exits)  4-byte addressing mode.
 *
 * INPUT:
 *        enable: MV_TRUE - enter, MV_FALSE - exit 4-byte mode
 *
 * OUTPUT:
 *       None
 *
 * RETURN:
 *       Success or Error code.
 *
 *******************************************************************************/
MV_STATUS mvSFlashSet4Byte (MV_BOOL enable)
{
    MV_STATUS ret;
    MV_U8 cmd[1];
    unsigned long spi_cfg;

    if(!FlashExtdAddress)
        return MV_BAD_VALUE;

    /* Issue the Write enable command prior the EN4B command */
    if ((ret = mvWriteEnable()) != MV_OK)
    {
        DB(mvOsOutputString("%s WARNING: Error calling mvWriteEnable()!\n", __func__);)
        return ret;
    }

    if (ifType == MV_SPI_IF_SPI) {
        spi_cfg = MV_REG_READ(0x10604) & ~0x300;
        if(enable) 
            spi_cfg |= 0x300; // 4-bytes address length
        else
            spi_cfg |= 0x200;
        MV_REG_WRITE(0x10604, spi_cfg);
    }

    cmd[0] = (enable) ? MV_EN4B_CMND_OPCD : MV_EX4B_CMND_OPCD;

    return (ifType == MV_SPI_IF_SPI) ?
        mvSpiWriteThenRead(cmd, 1, NULL, 0, 0) : srvCpuQspiWrite(0, 0, 0, 0, 0, (void *)cmd, 1);

}

/*******************************************************************************
 * mvSFlashInit - Initialize the serial flash device
 *
 * DESCRIPTION:
 *       Perform the neccessary initialization and configuration
 *
 * INPUT:
 *       pFlinfo: pointer to the Flash information structure
 *           pFlinfo->baseAddr: base address in fast mode.
 *           pFlinfo->index: Index of the flash in the sflash tabel. If the SPI
 *                           flash device does not support read Id command with
 *                           the standard opcode, then the user should supply this
 *                           as an input to skip the autodetection process!!!!
 *
 * OUTPUT:
 *       pFlinfo: pointer to the Flash information structure after detection
 *           pFlinfo->manufacturerId: Manufacturer ID
 *           pFlinfo->deviceId: Device ID
 *           pFlinfo->sectorSize: size of the sector (all sectors are the same).
 *           pFlinfo->sectorNumber: number of sectors.
 *           pFlinfo->pageSize: size of the page.
 *           pFlinfo->index: Index of the detected flash in the sflash tabel
 *
 * RETURN:
 *       Success or Error code.
 *
 *
 *******************************************************************************/
MV_STATUS mvSFlashInit (MV_SFLASH_INFO * pFlinfo)
{
    MV_STATUS ret;
    MV_U8     manf;
    MV_U16    dev;
    MV_BOOL   detectFlag = MV_FALSE;
    MV_U32    serialBaudRate;

    /*  MV_U32    offset, data; */

    /* check for NULL pointer */
    if (pFlinfo == NULL)
    {
        DB(mvOsOutputString("%s ERROR: Null pointer parameter!\n", __func__));
        return MV_BAD_PARAM;
    }

    serialBaudRate = MX25_MAXFREQ;
    ifType = pFlinfo->if_type;

    /* Initialize the SPI interface with low frequency to make sure that the read ID succeeds */
    ret = (ifType == MV_SPI_IF_SPI) ? mvSpiInit(serialBaudRate) : srvCpuQspiInit(0, pFlinfo->baseAddr, serialBaudRate, SRV_CPU_SPI_MODE_0_E);

    if (ret != MV_OK)
    {
        DB(mvOsOutputString("%s ERROR: Failed to initialize the SPI interface!\n", __func__));
        return ret;
    }

    /* First try to read the Manufacturer and Device IDs */
    if ((ret = mvSFlashIdGet(&manf, &dev)) != MV_OK)
    {
        mvOsOutputString("Flash ID not found!\n");
#ifndef MICRO_INIT
        NOR_FLASH_NOT_EXIST;
#endif
        return ret;
    }

    /* loop over the whole table and look for the appropriate SFLASH */
    flashIdx = 0;

    while (flashIdx < ARRAY_SIZE)  /* avoid endless loop */
    {
        if ((sFlashTypes[flashIdx].manufacturerId == manf) &&
                (sFlashTypes[flashIdx].deviceId      == dev))
        {
            /* save the base address locally. */
            sFlashBaseAddress = pFlinfo->baseAddr;

            /* fill the info based on the model detected */
            pFlinfo->manufacturerId = manf;
            pFlinfo->deviceId       = dev;
            pFlinfo->index          = flashIdx;
            pFlinfo->sectorSize     = (sFlashTypes[flashIdx].sectorSize * 1024); /* Sector sizes are in kb */
            pFlinfo->sectorNumber   = sFlashTypes[flashIdx].sectorNumber;
            pFlinfo->pageSize       = sFlashTypes[flashIdx].pageSize;
            FlashExtdAddress        = sFlashTypes[flashIdx].flash4BytesAddr ? MV_TRUE : MV_FALSE;
            detectFlag              = MV_TRUE;
            FlashInitFlag           = MV_TRUE;
            VendorID                = manf;
            DeviceID                = dev;
            break;
        }


        /* Move to next flash type entry  */
        flashIdx++;
    }
#ifdef MICRO_INIT
        mvOsOutputString("\nFlash manuf_id: 0x%x , dev_id: 0x%x\n",manf,dev);
#else
        mvOsOutputString("\nFlash manuf_id: 0x");
        mvOsPrintUint(manf);
        mvOsOutputString(" , Flash dev_id: 0x");
        mvOsPrintUint(dev);
#endif


    if(!detectFlag)
    {
        mvOsOutputString("\nUnknown SPI flash device!\n");
#ifndef MICRO_INIT
        NOR_FLASH_NOT_SUPPORTED;
#endif
        return MV_FAIL;
    }


    /* Enable 4-byte addressing if selected chip supports it */
    if (FlashExtdAddress) {
        if ((ret = mvSFlashSet4Byte(MV_TRUE)) != MV_OK)
            return ret;
    } else { /* Set to 3 Byte address mode
                (This setting is required incase previous application set it differently) */
        if (ifType == MV_SPI_IF_SPI) {
            unsigned long spi_cfg;
            spi_cfg = MV_REG_READ( 0x10604 );
            /*  3-bytes address length */
            spi_cfg &= ~0x300;
            spi_cfg |=  0x200;
            MV_REG_WRITE(0x10604, spi_cfg);
        }
    }

    /* As default lock the SR */
    if ((ret = mvSFlashStatRegLock(MV_TRUE)) != MV_OK)
        return ret;

    return MV_OK;
}

/*******************************************************************************
 * mvSFlashSectorErase - Erasse a single sector of the serial flash
 *
 * DESCRIPTION:
 *       Issue the erase sector command and address
 *
 * INPUT:
 *        secNumber: sector Number to erase (0 -> (sectorNumber-1))
 *
 * OUTPUT:
 *       None
 *
 * RETURN:
 *       Success or Error code.
 *
 *******************************************************************************/
MV_STATUS mvSFlashSectorErase (MV_U32 secNumber)
{
    MV_U32 secAddr, wordsPerSector, erasedWord = 0xFFFFFFFF;
    MV_BOOL eraseNeeded = MV_FALSE;
    MV_U32 i;
    MV_U32 buffer;
    MV_U8 cmd[MV_SFLASH_SE_CMND_LENGTH];
    MV_U8 n = 0;
    MV_STATUS ret;

    /* Protection - check if the model was detected */
    if(!FlashInitFlag)
    {
        DB(mvOsOutputString("%s ERROR: Unknown SPI flash device!\n", __func__);)
            return MV_BAD_PARAM;
    }

    /* check that the sector number is valid */
    if (secNumber >= sFlashTypes[flashIdx].sectorNumber)
    {
        DB(mvOsOutputString("%s WARNING: Invaild parameter sector number!\n", __func__);)
            return MV_BAD_PARAM;
    }

    secAddr = (secNumber * (sFlashTypes[flashIdx].sectorSize * 1024)); /* Sector sizes are in kb */
    wordsPerSector = ((sFlashTypes[flashIdx].sectorSize * 1024) / sizeof(MV_U32));

    /* First compare to FF and check if erase is needed */
    /* TODO: uncomment first part after the SPI flash will be correctly mapped */
#if 0
    MV_U32 * pW = (MV_U32*) (secAddr + sFlashBaseAddress);
    for (i=0; i<wordsPerSector; i++)
    {
        if (memcmp(pW, &erasedWord, sizeof(MV_U32)) != 0)
        {
            eraseNeeded = MV_TRUE;
            break;
        }

        ++pW;
    }

#endif
    for ( i = 0; i < wordsPerSector; i++ )
    {
#ifndef MICRO_INIT
        mvSFlashBlockRd( secAddr, ( void * )&buffer, sizeof( buffer ) );
#else
        buffer = *((MV_U32*)(SPI_BASE_ADDR + secAddr));
#endif
        if ( buffer != erasedWord )
        {
            eraseNeeded = MV_TRUE;
            break;
        }
        secAddr += sizeof( buffer );
    }

    if (!eraseNeeded)
        return MV_OK;

    cmd[n++] = MV_M25P_SE_CMND_OPCD;           /* Sector Erase - Identical command for all vendors. */
    if(FlashExtdAddress)
        cmd[n++] = ((secAddr >> 24) & 0xFF);
    cmd[n++] = ((secAddr >> 16) & 0xFF);
    cmd[n++] = ((secAddr >> 8) & 0xFF);
    cmd[n++] = (secAddr & 0xFF);

    /* Issue the Write enable command prior the sector erase command */
    if ((ret = mvWriteEnable()) != MV_OK)
        return ret;

    ret = (ifType == MV_SPI_IF_SPI) ?
        mvSpiWriteThenWrite(cmd, n, (MV_U8 *)secAddr, 0) :
        srvCpuQspiWrite(0, cmd[0], secAddr, n - 1, 0, 0, 0);

    if (ret != MV_OK)
        return ret;

    if ((ret = mvWaitOnWipClear()) != MV_OK)
        return ret;

    /* Verify sectore erased */
    secAddr = (secNumber * (sFlashTypes[flashIdx].sectorSize * 1024)); /* Sector sizes are in kb */
    for ( i = 0; i < wordsPerSector; i++ )
    {
#ifndef MICRO_INIT
        mvSFlashBlockRd( secAddr, ( void * )&buffer, sizeof( buffer ) );
#else
        buffer = *((MV_U32*)(SPI_BASE_ADDR + secAddr));
#endif
        if ( buffer != erasedWord )
        {
            mvOsOutputString("Failed erasing flash at offset: 0x%x",secAddr);
            return MV_ERROR;
        }
        secAddr += sizeof( buffer );
    }
    return MV_OK;
}

#ifndef MICRO_INIT
/*******************************************************************************
 * mvSFlashBlockRd - Read from the serial flash
 *
 * DESCRIPTION:
 *       Issue the read command and address then perfom the needed read
 *
 * INPUT:
 *        offset: byte offset with the flash to start reading from
 *        pReadBuff: pointer to the buffer to read the data in
 *        buffSize: size of the buffer to read.
 *
 * OUTPUT:
 *       pReadBuff: pointer to the buffer containing the read data
 *
 * RETURN:
 *       Success or Error code.
 *
 *
 *******************************************************************************/
MV_STATUS mvSFlashBlockRd (MV_U32 offset, MV_U8* pReadBuff, MV_U32 buffSize)
{
    MV_STATUS ret;
    MV_U8 cmd[MV_SFLASH_READ_CMND_LENGTH];
    MV_U8 n;

    /* Protection - check if the model was detected */
    if(!FlashInitFlag)
    {
        DB(mvOsOutputString("%s ERROR: Unknown SPI flash device!\n", __func__);)
            return MV_BAD_PARAM;
    }

    n = 0;
    cmd[n++] = MV_M25P_READ_CMND_OPCD;     /* Sequential Read  - Identical command for all vendors. */
    if( FlashExtdAddress )
        cmd[n++] = ((offset >> 24) & 0xFF);
    cmd[n++] = ((offset >> 16) & 0xFF);
    cmd[n++] = ((offset >> 8) & 0xFF);
    cmd[n++] = (offset & 0xFF);

    return (ifType == MV_SPI_IF_SPI) ?
        mvSpiWriteThenRead(cmd, n, pReadBuff, buffSize, 0) :
        srvCpuQspiRead(0, cmd[0], offset, n - 1, 0, (void *)pReadBuff, buffSize);
}

/*******************************************************************************
 * mvSFlashFastBlockRd - Fast read from the serial flash
 *
 * DESCRIPTION:
 *       Issue the fast read command and address then perfom the needed read
 *
 * INPUT:
 *        offset: byte offset with the flash to start reading from
 *        pReadBuff: pointer to the buffer to read the data in
 *        buffSize: size of the buffer to read.
 *
 * OUTPUT:
 *       pReadBuff: pointer to the buffer containing the read data
 *
 * RETURN:
 *       Success or Error code.
 *
 *
 *******************************************************************************/
MV_STATUS mvSFlashFastBlockRd (MV_U32 offset, MV_U8* pReadBuff, MV_U32 buffSize)
{
/* Removed for micro-init - fast read caused problems with cisco spi flash types */
#if 0
    MV_U8     cmd[MV_SFLASH_READ_CMND_LENGTH];
    MV_U32    spiMaxFreq          = 0x0;
    MV_U32    spiMaxFastFreq      = 0x0;
    MV_U32    spiFastRdDummyBytes = 0x0;
    MV_STATUS ret;
    MV_U8     n;

    /* check for NULL pointer */
    if (pReadBuff == NULL)
    {
        mvOsOutputString("%s ERROR: Null pointer parameter!\n", __func__);
        return MV_BAD_PARAM;
    }

    /* Protection - check if the model was detected */
    if(!FlashInitFlag)
    {
        DB(mvOsOutputString("%s ERROR: Unknown SPI flash device!\n", __func__);)
            return MV_BAD_PARAM;
    }

    /* Removed for micro-init - some spi flash not working with 50Mhz. (spi clock set to 20Mhz) */
#if 0
    /* Set the SPI frequency to the MAX allowed for fast-read operations */
    spiMaxFastFreq = sFlashTypes[flashIdx].maxFastFreq;
    if ((ret = mvSpiBaudRateSet(spiMaxFastFreq)) != MV_OK)
    {
        mvOsOutputString("%s ERROR: Failed to set the SPI fast frequency!\n", __func__);
        return ret;
    }
#endif

    cmd[n++] = MV_M25P_FAST_RD_CMND_OPCD;  /* Fast Read - Identical command for all vendors. */
    if( FlashExtdAddress)
        cmd[n++] = ((offset >> 24) & 0xFF);
    cmd[n++] = ((offset >> 16) & 0xFF);
    cmd[n++] = ((offset >> 8) & 0xFF);
    cmd[n++] = (offset & 0xFF);

    spiFastRdDummyBytes = MX25_FAST_READ_DUMMY_BYTES;


    /* Removed for micro-init - some spi flash not working with 50Mhz. (spi clock set to 20Mhz) */
#if 0
    /* Reset the SPI frequency to the MAX allowed for the device for best performance */
    spiMaxFreq = sFlashTypes[flashIdx].maxFreq;
    if ((ret = mvSpiBaudRateSet(spiMaxFreq)) != MV_OK)
    {
        mvOsOutputString("%s ERROR: Failed to set the SPI frequency!\n", __func__);
        return ret;
    }
#endif

return ret;
#else
        return mvSFlashBlockRd(offset, pReadBuff, buffSize);
#endif

}
#endif /* MICRO_INIT */

/*******************************************************************************
 * mvSFlashBlockWr - Write a buffer with any size
 *
 * DESCRIPTION:
 *       write regardless of the page boundaries and size limit per Page
 *        program command
 *
 * INPUT:
 *        offset: byte offset within the flash region
 *        pWriteBuff: pointer to the buffer holding the data to program
 *        buffSize: size of the buffer to write
 *
 * OUTPUT:
 *       None
 *
 * RETURN:
 *       Success or Error code.
 *
 *******************************************************************************/
MV_STATUS mvSFlashBlockWr (MV_U32 offset, MV_U8* pWriteBuff, MV_U32 buffSize)
{
    MV_STATUS ret;
    MV_U32  data2write   = buffSize;
    MV_U32  preAllOffset = (offset & MV_SFLASH_PAGE_ALLIGN_MASK(MV_MXIC_PAGE_SIZE));
    MV_U32  preAllSz     = (preAllOffset ? (MV_MXIC_PAGE_SIZE - preAllOffset) : 0);

    MV_U32  writeOffset  = offset;

    /* Protection - check if the model was detected */
    if(!FlashInitFlag)
    {
        DB(mvOsOutputString("%s ERROR: Unknown SPI flash device!\n", __func__);)
            return MV_BAD_PARAM;
    }

    /* check that the buffer size does not exceed the flash size */
    if ((offset + buffSize) > mvSFlashSizeGet())
    {
        DB(mvOsOutputString("%s WARNING: Write exceeds flash size!\n", __func__);)
            return MV_OUT_OF_RANGE;
    }

    /* check if the total block size is less than the first chunk remainder */
    if (data2write < preAllSz)
        preAllSz = data2write;

    /* check if programing does not start at a 64byte alligned offset */
    if (preAllSz)
    {
        if ((ret = mvSFlashPageWr(writeOffset, pWriteBuff, preAllSz)) != MV_OK)
            return ret;

        /* increment pointers and counters */
        writeOffset += preAllSz;
        data2write -= preAllSz;
        pWriteBuff += preAllSz;
    }

    /* program the data that fits in complete page chunks */
    while (data2write >= sFlashTypes[flashIdx].pageSize)
    {
        preAllSz = sFlashTypes[flashIdx].pageSize;

        if ((ret = mvSFlashPageWr(writeOffset, pWriteBuff, preAllSz)) != MV_OK)
            return ret;

        /* increment pointers and counters */
        writeOffset += preAllSz;
        data2write  -= preAllSz;
        pWriteBuff  += preAllSz;
    }

    /* program the last partial chunk */
    if (data2write)
    {
        if ((ret = mvSFlashPageWr(writeOffset, pWriteBuff, data2write)) != MV_OK)
            return ret;
    }

    return MV_OK;
}

/*******************************************************************************
 * mvSFlashIdGet - Get the manufacturer and device IDs.
 *
 * DESCRIPTION:
 *       Get the Manufacturer and device IDs from the serial flash through
 *        writing the RDID command then reading 3 bytes of data. In case that
 *       this command was called for the first time in order to detect the
 *       manufacturer and device IDs, then the default RDID opcode will be used
 *       unless the device index is indicated by the user (in case the SPI flash
 *       does not use the default RDID opcode).
 *
 * INPUT:
 *        pManId: pointer to the 8bit variable to hold the manufacturing ID
 *        pDevId: pointer to the 16bit variable to hold the device ID
 *
 * OUTPUT:
 *        pManId: pointer to the 8bit variable holding the manufacturing ID
 *        pDevId: pointer to the 16bit variable holding the device ID
 *
 * RETURN:
 *       Success or Error code.
 *
 *******************************************************************************/
MV_STATUS mvSFlashIdGet (MV_U8* pManId, MV_U16* pDevId)
{
    MV_STATUS ret;
    MV_U8 cmd[MV_SFLASH_RDID_CMND_LENGTH];
    MV_U8 id[MV_SFLASH_RDID_REPLY_LENGTH];

    /* check for NULL pointer */
    if ((pManId == NULL) || (pDevId == NULL))
    {
        DB(mvOsOutputString("%s ERROR: Null pointer parameter!\n", __func__));
        return MV_BAD_PARAM;
    }

    if (flashIdx >= ((sizeof(sFlashTypes[flashIdx])/sizeof(unsigned int))))
        return mvSFlashWithDefaultsIdGet(pManId, pDevId);
    else
    {
        cmd[0] = MV_M25P_RDID_CMND_OPCD; /* Read ID - Identical command for all vendors. */
    }

    ret = (ifType == MV_SPI_IF_SPI) ?
        mvSpiWriteThenRead(cmd, MV_SFLASH_RDID_CMND_LENGTH, id, MV_SFLASH_RDID_REPLY_LENGTH, 0) :
        srvCpuQspiRead(0, cmd[0], 0, 0, 0, (void *)id, MV_SFLASH_RDID_REPLY_LENGTH);
    if (ret != MV_OK)
        return ret;

    *pManId = id[0];
    *pDevId = 0;
    *pDevId |= (id[1] << 8);
    *pDevId |= (MV_U16)id[2];

    return MV_OK;
}

#if 0
/*******************************************************************************
 * mvSFlashWpRegionSet - Set the Write-Protected region
 *
 * DESCRIPTION:
 *       Set the Write-Protected region
 *
 * INPUT:
 *        wpRegion: which region will be protected
 *
 * OUTPUT:
 *       None
 *
 * RETURN:
 *       Success or Error code.
 *
 *******************************************************************************/
MV_STATUS mvSFlashWpRegionSet (MV_SFLASH_WP_REGION wpRegion)
{
    MV_U8 wpMask = MV_BAD_PARAM;

    /* Protection - check if the model was detected */
    if(!FlashInitFlag)
    {
        DB(mvOsOutputString("%s ERROR: Unknown SPI flash device!\n", __func__);)
            return MV_BAD_PARAM;
    }

    /* Check if the chip is an ST flash; then WP supports only 3 bits.            */
    /* If the manufacturer is MXIC (MACRONIX) or spansion, then the WP is 4 bits. */
    if( VendorID == MACRONIX_FLASH && (DeviceID == MX25L6405 || DeviceID == MX25L12845))
    {
        switch (wpRegion)
        {
            case MV_WP_NONE:        wpMask = MV_MX25L_STATUS_BP_NONE;       break;
            case MV_WP_UPR_1OF128:  wpMask = MV_MX25L_STATUS_BP_1_OF_128;   break;
            case MV_WP_UPR_1OF64:   wpMask = MV_MX25L_STATUS_BP_1_OF_64;    break;
            case MV_WP_UPR_1OF32:   wpMask = MV_MX25L_STATUS_BP_1_OF_32;    break;
            case MV_WP_UPR_1OF16:   wpMask = MV_MX25L_STATUS_BP_1_OF_16;    break;
            case MV_WP_UPR_1OF8:    wpMask = MV_MX25L_STATUS_BP_1_OF_8;     break;
            case MV_WP_UPR_1OF4:    wpMask = MV_MX25L_STATUS_BP_1_OF_4;     break;
            case MV_WP_UPR_1OF2:    wpMask = MV_MX25L_STATUS_BP_1_OF_2;     break;
            case MV_WP_ALL:         wpMask = MV_MX25L_STATUS_BP_ALL;        break;
            default:
                                    DB(mvOsOutputString("%s WARNING: Invaild parameter WP region!\n", __func__);)
                                        return MV_BAD_PARAM;
        }
    }

    else
        if( VendorID == MACRONIX_FLASH && DeviceID == MX25L25635)
        {
            switch (wpRegion)
            {
                case MV_WP_NONE:        wpMask = MV_MX25L256_STATUS_BP_NONE;       break;
                case MV_WP_UPR_1OF128:  wpMask = MV_MX25L256_STATUS_BP_1_OF_128;   break;
                case MV_WP_UPR_1OF64:   wpMask = MV_MX25L256_STATUS_BP_1_OF_64;    break;
                case MV_WP_UPR_1OF32:   wpMask = MV_MX25L256_STATUS_BP_1_OF_32;    break;
                case MV_WP_UPR_1OF16:   wpMask = MV_MX25L256_STATUS_BP_1_OF_16;    break;
                case MV_WP_UPR_1OF8:    wpMask = MV_MX25L256_STATUS_BP_1_OF_8;     break;
                case MV_WP_UPR_1OF4:    wpMask = MV_MX25L256_STATUS_BP_1_OF_4;     break;
                case MV_WP_UPR_1OF2:    wpMask = MV_MX25L256_STATUS_BP_1_OF_2;     break;
                case MV_WP_ALL:         wpMask = MV_MX25L256_STATUS_BP_ALL;        break;
                default:
                                        DB(mvOsOutputString("%s WARNING: Invaild parameter WP region!\n", __func__);)
                                            return MV_BAD_PARAM;
            }
        }

        else
        {
            switch (wpRegion)
            {
                case MV_WP_NONE:
                    wpMask = MV_M25P_STATUS_BP_NONE;
                    break;

                case MV_WP_UPR_1OF128:
                    DB(mvOsOutputString("%s WARNING: Invaild option for this flash chip!\n", __func__);)
                        return MV_NOT_SUPPORTED;
                    break;

                case MV_WP_UPR_1OF64:
                    wpMask = MV_M25P_STATUS_BP_1_OF_64;
                    break;

                case MV_WP_UPR_1OF32:
                    wpMask = MV_M25P_STATUS_BP_1_OF_32;
                    break;

                case MV_WP_UPR_1OF16:
                    wpMask = MV_M25P_STATUS_BP_1_OF_16;
                    break;

                case MV_WP_UPR_1OF8:
                    wpMask = MV_M25P_STATUS_BP_1_OF_8;
                    break;

                case MV_WP_UPR_1OF4:
                    wpMask = MV_M25P_STATUS_BP_1_OF_4;
                    break;

                case MV_WP_UPR_1OF2:
                    wpMask = MV_M25P_STATUS_BP_1_OF_2;
                    break;

                case MV_WP_ALL:
                    wpMask = MV_M25P_STATUS_BP_ALL;
                    break;

                default:
                    DB(mvOsOutputString("%s WARNING: Invaild parameter WP region!\n", __func__);)
                        return MV_BAD_PARAM;

            }   /* switch () */
        } /* else */

    /* Verify that the SRWD bit is always set - register is s/w locked */
    wpMask |= MV_SFLASH_STATUS_REG_SRWD_MASK;

    return mvStatusRegSet(wpMask);
}

/*******************************************************************************
 * mvSFlashWpRegionGet - Get the Write-Protected region configured
 *
 * DESCRIPTION:
 *       Get from the chip the Write-Protected region configured
 *
 * INPUT:
 *        pWpRegion: pointer to the variable to return the WP region in
 *
 * OUTPUT:
 *        wpRegion: pointer to the variable holding the WP region configured
 *
 * RETURN:
 *       Success or Error code.
 *
 *******************************************************************************/
MV_STATUS mvSFlashWpRegionGet (MV_SFLASH_WP_REGION * pWpRegion)
{
    MV_STATUS ret;
    MV_U8 reg;

    /* check for NULL pointer */
    if (pWpRegion == NULL)
    {
        mvOsOutputString("%s ERROR: Null pointer parameter!\n", __func__);
        return MV_BAD_PARAM;
    }

    /* Protection - check if the model was detected */
    if(!FlashInitFlag)
    {
        DB(mvOsOutputString("%s ERROR: Unknown SPI flash device!\n", __func__);)
        return MV_BAD_PARAM;
    }

    if ((ret = mvStatusRegGet(&reg)) != MV_OK)
        return ret;

    /* Check if the chip is an ST flash; then WP supports only 3 bits.            */
    /* If the manufacturer is MXIC (MACRONIX) or spansion, then the WP is 4 bits. */
    if( VendorID == MACRONIX_FLASH && (DeviceID == MX25L6405 || DeviceID == MX25L12845))
    {
        switch (reg & MV_MX25L_STATUS_REG_WP_MASK)
        {
            case MV_MX25L_STATUS_BP_NONE:       *pWpRegion = MV_WP_NONE;        break;
            case MV_MX25L_STATUS_BP_1_OF_128:   *pWpRegion = MV_WP_UPR_1OF128;  break;
            case MV_MX25L_STATUS_BP_1_OF_64:    *pWpRegion = MV_WP_UPR_1OF64;   break;
            case MV_MX25L_STATUS_BP_1_OF_32:    *pWpRegion = MV_WP_UPR_1OF32;   break;
            case MV_MX25L_STATUS_BP_1_OF_16:    *pWpRegion = MV_WP_UPR_1OF16;   break;
            case MV_MX25L_STATUS_BP_1_OF_8:     *pWpRegion = MV_WP_UPR_1OF8;    break;
            case MV_MX25L_STATUS_BP_1_OF_4:     *pWpRegion = MV_WP_UPR_1OF4;    break;
            case MV_MX25L_STATUS_BP_1_OF_2:     *pWpRegion = MV_WP_UPR_1OF2;    break;
            case MV_MX25L_STATUS_BP_ALL:        *pWpRegion = MV_WP_ALL;         break;
            default:
                                                DB(mvOsOutputString("%s WARNING: Unidentified WP region in h/w!\n", __func__);)
                                                    return MV_BAD_VALUE;
        }
    }

    else
        if( VendorID == MACRONIX_FLASH && DeviceID == MX25L25635)
        {
            switch (reg & MV_MX25L_STATUS_REG_WP_MASK)
            {
                case MV_MX25L256_STATUS_BP_NONE:        *pWpRegion = MV_WP_NONE;        break;
                                                        /*          case MV_MX25L256_STATUS_BP_1_OF_256:    *pWpRegion = MV_WP_UPR_1OF128;  break;*/
                case MV_MX25L256_STATUS_BP_1_OF_128:    *pWpRegion = MV_WP_UPR_1OF128;  break;
                case MV_MX25L256_STATUS_BP_1_OF_64:     *pWpRegion = MV_WP_UPR_1OF64;   break;
                case MV_MX25L256_STATUS_BP_1_OF_32:     *pWpRegion = MV_WP_UPR_1OF32;   break;
                case MV_MX25L256_STATUS_BP_1_OF_16:     *pWpRegion = MV_WP_UPR_1OF16;   break;
                case MV_MX25L256_STATUS_BP_1_OF_8:      *pWpRegion = MV_WP_UPR_1OF8;    break;
                case MV_MX25L256_STATUS_BP_1_OF_4:      *pWpRegion = MV_WP_UPR_1OF4;    break;
                case MV_MX25L256_STATUS_BP_1_OF_2:      *pWpRegion = MV_WP_UPR_1OF2;    break;
                case MV_MX25L256_STATUS_BP_ALL:         *pWpRegion = MV_WP_ALL;         break;
                default:
                                                        DB(mvOsOutputString("%s WARNING: Unidentified WP region in h/w!\n", __func__);)
                                                            return MV_BAD_VALUE;
            }
        }

        else
        {
            /* SPANSION */
            switch (reg & MV_S25FL_STATUS_REG_WP_MASK)
            {
                case MV_S25FL_STATUS_BP_NONE:       *pWpRegion = MV_WP_NONE;        break;
                case MV_S25FL_STATUS_BP_1_OF_128:   *pWpRegion = MV_WP_UPR_1OF128;  break;
                case MV_S25FL_STATUS_BP_1_OF_64:    *pWpRegion = MV_WP_UPR_1OF64;   break;
                case MV_S25FL_STATUS_BP_1_OF_32:    *pWpRegion = MV_WP_UPR_1OF32;   break;
                case MV_S25FL_STATUS_BP_1_OF_16:    *pWpRegion = MV_WP_UPR_1OF16;   break;
                case MV_S25FL_STATUS_BP_1_OF_8:     *pWpRegion = MV_WP_UPR_1OF8;    break;
                case MV_S25FL_STATUS_BP_1_OF_4:     *pWpRegion = MV_WP_UPR_1OF4;    break;
                case MV_S25FL_STATUS_BP_1_OF_2:     *pWpRegion = MV_WP_UPR_1OF2;    break;
                case MV_S25FL_STATUS_BP_ALL:        *pWpRegion = MV_WP_ALL;         break;
                default:
                                                    DB(mvOsOutputString("%s WARNING: Unidentified WP region in h/w!\n", __func__);)
                                                        return MV_BAD_VALUE;
            }
        }

    switch (reg & MV_M25P_STATUS_REG_WP_MASK)
    {
        case MV_M25P_STATUS_BP_NONE:
            *pWpRegion = MV_WP_NONE;
            break;

        case MV_M25P_STATUS_BP_1_OF_64:
            *pWpRegion = MV_WP_UPR_1OF64;
            break;

        case MV_M25P_STATUS_BP_1_OF_32:
            *pWpRegion = MV_WP_UPR_1OF32;
            break;

        case MV_M25P_STATUS_BP_1_OF_16:
            *pWpRegion = MV_WP_UPR_1OF16;
            break;

        case MV_M25P_STATUS_BP_1_OF_8:
            *pWpRegion = MV_WP_UPR_1OF8;
            break;

        case MV_M25P_STATUS_BP_1_OF_4:
            *pWpRegion = MV_WP_UPR_1OF4;
            break;

        case MV_M25P_STATUS_BP_1_OF_2:
            *pWpRegion = MV_WP_UPR_1OF2;
            break;

        case MV_M25P_STATUS_BP_ALL:
            *pWpRegion = MV_WP_ALL;
            break;

        default:
            DB(mvOsOutputString("%s WARNING: Unidentified WP region in h/w!\n", __func__);)
                return MV_BAD_VALUE;
    }   /* switch () */

    return MV_OK;
}
#endif

/*******************************************************************************
 * mvSFlashStatRegLock - Lock the status register for writing - W/Vpp
 *        pin should be low to take effect
 *
 * DESCRIPTION:
 *       Lock the access to the Status Register for writing. This will
 *        cause the flash to enter the hardware protection mode if the W/Vpp
 *        is low. If the W/Vpp is hi, the chip will be in soft protection mode, but
 *        the register will continue to be writable if WREN sequence was used.
 *
 * INPUT:
 *        srLock: enable/disable (MV_TRUE/MV_FALSE) status registor lock mechanism
 *
 * OUTPUT:
 *       None
 *
 * RETURN:
 *       Success or Error code.
 *
 *******************************************************************************/
MV_STATUS mvSFlashStatRegLock (MV_BOOL srLock)
{
    MV_STATUS ret;
    MV_U8 reg;

    if ((ret = mvStatusRegGet(&reg)) != MV_OK)
        return ret;

    if (srLock)
        reg |= MV_SFLASH_STATUS_REG_SRWD_MASK;
    else
        reg &= ~MV_SFLASH_STATUS_REG_SRWD_MASK;

    ret = mvStatusRegSet(reg);
    return ret;
}

/*******************************************************************************
 * mvSFlashSizeGet - Get the size of the SPI flash
 *
 * DESCRIPTION:
 *       based on the sector number and size of each sector calculate the total
 *       size of the flash memory.
 *
 * INPUT:
 *
 * OUTPUT:
 *       None.
 *
 * RETURN:
 *       Size of the flash in bytes.
 *
 *******************************************************************************/
MV_U32 mvSFlashSizeGet (void)
{
    return ((sFlashTypes[flashIdx].sectorSize * 1024) * sFlashTypes[flashIdx].sectorNumber);
}

#if 0
/*******************************************************************************
 * mvFullStatusRegGet - Retrieve the value of the status register
 *
 * DESCRIPTION:
 *       perform the RDSR sequence to get the 16bit status register
 *
 ********************************************************************************/
MV_STATUS mvFullStatusRegGet(MV_U16 * pStatReg)
{
    MV_STATUS ret;
    MV_U8 cmd[MV_SFLASH_RDSR_CMND_LENGTH];
    MV_U8 sr [MV_SFLASH_RDSR_REPLY_LENGTH];
    MV_U16 res=0;

    cmd[0] = MV_GD25L_RDSR_CMND_OPCD;     /* Read Status Register - Identical command for all vendors. */

    if ((ret = mvSpiWriteThenRead(cmd, MV_SFLASH_RDSR_CMND_LENGTH, sr,
                    MV_SFLASH_RDSR_REPLY_LENGTH,0)) != MV_OK)
        return ret;


    res = sr[0]&0xFF;

    cmd[0] = MV_GD25H_RDSR_CMND_OPCD;     /* Read High part of Status Register. */

    if ((ret = mvSpiWriteThenRead(cmd, MV_SFLASH_RDSR_CMND_LENGTH, sr,
                    MV_SFLASH_RDSR_REPLY_LENGTH,0)) != MV_OK)
        return ret;

    res |= sr[0]<<8;

    *pStatReg = res;

    return MV_OK;
}
#endif

MV_BOOL isFlashInitialized(void)
{
    return FlashInitFlag;
}
