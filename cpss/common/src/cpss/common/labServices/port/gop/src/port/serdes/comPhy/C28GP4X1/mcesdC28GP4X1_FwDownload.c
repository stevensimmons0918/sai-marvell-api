/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to download MCU firmware to Marvell
COMPHY_28G_PIPE4_RPLL_1P2V
********************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "../mcesdTop.h"
#include "../mcesdApiTypes.h"
#include "../mcesdUtils.h"
#include "mcesdC28GP4X1_Defs.h"
#include "mcesdC28GP4X1_API.h"
#include "mcesdC28GP4X1_RegRW.h"
#include "mcesdC28GP4X1_FwDownload.h"

#ifdef C28GP4X1

MCESD_STATUS API_C28GP4X1_DownloadFirmware
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32* fwCodePtr,
    IN MCESD_U32 fwCodeSizeDW,
    OUT MCESD_U16 *errCode
)
{
    MCESD_FIELD fieldProgRamSel = F_C28GP4X1R1P0_PROG_RAM_SEL;

    MCESD_U32 expectedChecksum, actualChecksum, checksumPass;

    MCESD_FIELD resetChecksum = FIELD_DEFINE(0xA22C, 0, 0);
    MCESD_U8 retries =  10;

    MCESD_ATTEMPT(calculateChecksum(fwCodePtr, fwCodeSizeDW, &expectedChecksum));
    MCESD_ATTEMPT(API_C28GP4X1_WriteReg(devPtr, 0xA224, expectedChecksum));

    do
    {
        /* Select MCU program memory */
        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &fieldProgRamSel, 0));

        /* Write to MCU program memory */
        MCESD_ATTEMPT(API_C28GP4X1_UpdateRamCode(devPtr, fwCodePtr, fwCodeSizeDW, C28GP4X1_FW_MAX_SIZE, C28GP4X1_FW_BASE_ADDR, errCode));

        MCESD_ATTEMPT(API_C28GP4X1_ReadReg(devPtr, 0xA228, &actualChecksum));
        MCESD_ATTEMPT(API_C28GP4X1_ReadReg(devPtr, 0xA22C, &checksumPass));

        if ((checksumPass & (1<<1)) && (actualChecksum == expectedChecksum))
        {
            *errCode = 0;
            return MCESD_OK;
        }

        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &resetChecksum, 0));
        retries--;
    } while (retries != 0);

    *errCode = MCESD_IO_ERROR;
    MCESD_DBG_ERROR("API_28GP4X1_DownloadFirmware Failed: expectedChecksum = 0x%08x, actualChecksum = 0x%08x\n",expectedChecksum ,actualChecksum);
    return MCESD_FAIL;
}

MCESD_STATUS API_C28GP4X1_ProgCmnXData
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 *xDataPtr,
    IN MCESD_U32 xDataSizeDW,
    OUT MCESD_U16 *errCode
)
{
    MCESD_U32 expectedChecksum, actualChecksum, checksumPass;
    MCESD_FIELD resetChecksum = FIELD_DEFINE(0xA360, 0, 0);
    MCESD_U8 retries =  10;

    MCESD_ATTEMPT(calculateChecksum(xDataPtr, xDataSizeDW, &expectedChecksum));
    MCESD_ATTEMPT(API_C28GP4X1_WriteReg(devPtr, 0xA358, expectedChecksum));

    do
    {
        /* Write Common XDATA to MCU */
        MCESD_ATTEMPT(API_C28GP4X1_UpdateRamCode(devPtr, xDataPtr, xDataSizeDW, C28GP4X1_XDATA_CMN_MAX_SIZE, C28GP4X1_XDATA_CMN_BASE_ADDR, errCode));

        MCESD_ATTEMPT(API_C28GP4X1_ReadReg(devPtr,0xA35C, &actualChecksum));
        MCESD_ATTEMPT(API_C28GP4X1_ReadReg(devPtr,0xA360, &checksumPass));

        if ((checksumPass & (1<<1)) && (actualChecksum == expectedChecksum))
        {
            *errCode = 0;
            return MCESD_OK;
        }

        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &resetChecksum, 0));
        retries--;

    } while (retries != 0);

    *errCode = MCESD_IO_ERROR;
    MCESD_DBG_ERROR("API_C28GP4X1_ProgCmnXData Failed: expectedChecksum = 0x%08x, actualChecksum = 0x%08x\n",expectedChecksum ,actualChecksum);
    return MCESD_FAIL;
}

MCESD_STATUS API_C28GP4X1_ProgLaneXData
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 *xDataPtr,
    IN MCESD_U32 xDataSizeDW,
    OUT MCESD_U16 *errCode
)
{
    MCESD_U32 expectedChecksum, actualChecksum, checksumPass;
    MCESD_FIELD resetChecksum = FIELD_DEFINE(0x2294, 0, 0);
    MCESD_FIELD ethernetCfg   = FIELD_DEFINE(0xE604, 2, 0);

    MCESD_U8 retries =  10;

    MCESD_ATTEMPT(calculateChecksum(xDataPtr, xDataSizeDW, &expectedChecksum));
    MCESD_ATTEMPT(API_C28GP4X1_WriteReg(devPtr, 0x22F8, expectedChecksum));

    do
    {
        /* Write Lane XDATA to MCU */
        API_C28GP4X1_UpdateRamCode(devPtr, xDataPtr, xDataSizeDW, C28GP4X1_XDATA_LANE_MAX_SIZE, C28GP4X1_XDATA_LANE_BASE_ADDR, errCode);

        MCESD_ATTEMPT(API_C28GP4X1_ReadReg(devPtr, 0x22FC, &actualChecksum));
        MCESD_ATTEMPT(API_C28GP4X1_ReadReg(devPtr, 0x2294, &checksumPass));

        if ((checksumPass & (1<<29)) && (actualChecksum == expectedChecksum))
        {
            MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &ethernetCfg, 0x4));
            *errCode = 0;
            return MCESD_OK;
        }

        MCESD_ATTEMPT(API_C28GP4X1_WriteField(devPtr, &resetChecksum, 0));
        retries--;
    } while (retries != 0);

    *errCode = MCESD_IO_ERROR;
    MCESD_DBG_ERROR("API_C28GP4X1_ProgLaneXData Failed: expectedChecksum = 0x%08x, actualChecksum = 0x%08x\n",expectedChecksum ,actualChecksum);
    return MCESD_FAIL;
}

MCESD_STATUS API_C28GP4X1_UpdateRamCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 code[],
    IN MCESD_U32 codeSize,
    IN MCESD_U32 memSize,
    IN MCESD_U32 address,
    OUT MCESD_U16 *errCode
)
{
    MCESD_U32 index;

    /*******************************************************************************
       Check if the code about to be downloaded can fit into the device's memory
    *******************************************************************************/
    if (codeSize > memSize)
    {
        *errCode = MCESD_IMAGE_TOO_LARGE_TO_DOWNLOAD;
        return MCESD_FAIL;
    }

    /* Write to program memory incrementally */
    for (index = 0; index < codeSize; index++)
    {
        MCESD_ATTEMPT(API_C28GP4X1_WriteReg(devPtr, address, code[index]));
        address += 4;
    }

    *errCode = 0;
    return MCESD_OK;
}

#endif /* C28GP4X1 */
