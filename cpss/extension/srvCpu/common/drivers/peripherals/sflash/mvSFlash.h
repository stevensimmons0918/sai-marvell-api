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

#ifndef __INCmvSFlashH
#define __INCmvSFlashH

#ifdef MICRO_INIT
#include "common.h"

#ifdef CM3
#define SPI_BASE_ADDR                       0xB0100000
#else
#define SPI_BASE_ADDR                       0xD4000000
#endif

#else
#include "common.h"
#endif


/* MCAROS */
#define MV_SFLASH_PAGE_ALLIGN_MASK(pgSz)    (pgSz-1)
#define MV_ARRAY_SIZE(a)                    ((sizeof(a)) / (sizeof(a[0])))

/* Constants */
#define MV_INVALID_DEVICE_NUMBER            0xFFFFFFFF
/* 6 MHz is the minimum possible SPI frequency when tclk is set 166MHz*/
#define MV_SFLASH_BASIC_SPI_FREQ            MV_M25P128_MAX_SPI_FREQ /*  <-- 20MHz */  /* <-- 6MHz = old value */

/* enumerations */
typedef enum
{
  MV_WP_NONE,             /* Unprotect the whole chip */
  MV_WP_UPR_1OF128,       /* Write protect the upper 1/128 part */
  MV_WP_UPR_1OF64,        /* Write protect the upper 1/64 part */
  MV_WP_UPR_1OF32,        /* Write protect the upper 1/32 part */
  MV_WP_UPR_1OF16,        /* Write protect the upper 1/16 part */
  MV_WP_UPR_1OF8,         /* Write protect the upper 1/8 part */
  MV_WP_UPR_1OF4,         /* Write protect the upper 1/4 part */
  MV_WP_UPR_1OF2,         /* Write protect the upper 1/2 part */
  MV_WP_ALL               /* Write protect the whole chip */
} MV_SFLASH_WP_REGION;

typedef enum
{
  MV_SPI_IF_SPI,          /* Legacy SPI interface (until Falcon) */
  MV_SPI_IF_QSPI          /* QSPI interface (AC5 and newer) */
} MV_SFLASH_SPI_IF_TYPE;

/* Type Definitions */

typedef struct
{
  MV_U32  baseAddr;       /* Flash Base Address used in fast mode */
  MV_U8   manufacturerId; /* Manufacturer ID */
  MV_U16  deviceId;       /* Device ID */
  MV_U32  sectorSize;     /* Size of each sector - all the same */
  MV_U16  sectorNumber;   /* Number of sectors */
  MV_U16  pageSize;       /* Page size - affect alignment */
  MV_U8   index;          /* index of the device in the sflash table (internal parameter) */
  MV_SFLASH_SPI_IF_TYPE if_type; 
} MV_SFLASH_INFO;

typedef struct SFLASH_TYPE_TABLE
{
  MV_U8   flash4BytesAddr;  /* flash 4 bytes address */
  MV_U8   manufacturerId;   /* Manufacturer ID */
  MV_U16  deviceId;         /* Device ID */
  MV_U16  sectorNumber;     /* Number of sectors */
  MV_U16  sectorSize;       /* Size of each sector - all the same */
  MV_U16  pageSize;         /* Page size - affect alignment */
} SFLASH_TYPE_TABLE;


/* Function Prototypes */
/* Init */
MV_STATUS   mvSFlashInit        (MV_SFLASH_INFO * pFlinfo);

/* erase */
MV_STATUS   mvSFlashSectorErase (MV_U32 secNumber);

#ifndef MICRO_INIT
/* Read */
MV_STATUS   mvSFlashBlockRd     (MV_U32 offset, MV_U8* pReadBuff, MV_U32 buffSize);
MV_STATUS   mvSFlashFastBlockRd (MV_U32 offset, MV_U8* pReadBuff, MV_U32 buffSize);
#endif

/* write regardless of the page boundaries and size limit per Page program command */
MV_STATUS   mvSFlashBlockWr     (MV_U32 offset, MV_U8* pWriteBuff, MV_U32 buffSize);
/* Get IDs */
MV_STATUS   mvSFlashIdGet       (MV_U8* pManId, MV_U16* pDevId);

#if 0
/* Set and Get the Write Protection region - if the Status register is not locked */
MV_STATUS   mvSFlashWpRegionSet (MV_SFLASH_WP_REGION wpRegion);
MV_STATUS   mvSFlashWpRegionGet (MV_SFLASH_WP_REGION * pWpRegion);
#endif

/* Lock the status register for writing - W/Vpp pin should be low to take effect */
MV_STATUS   mvSFlashStatRegLock (MV_BOOL srLock);

/* Get the regions sizes */
MV_U32      mvSFlashSizeGet     (void);

MV_STATUS mvSFlashSet4Byte (MV_BOOL enable);

#if 0
MV_STATUS mvFullStatusRegGet(MV_U16 * pStatReg);
#endif

MV_BOOL isFlashInitialized      (void);

MV_U32 mvSFlashSectorSizeGet (void);
#endif /* __INCmvSFlashH */
