/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to download MCU firmware to Marvell
COMPHY_112G_X4
********************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "../mcesdTop.h"
#include "../mcesdApiTypes.h"
#include "../mcesdUtils.h"
#include "mcesdC112GX4_Defs.h"
#include "mcesdC112GX4_RegRW.h"
#include "mcesdC112GX4_FwDownload.h"

#ifdef C112GX4

MCESD_STATUS API_C112GX4_DownloadFirmware
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32* fwCodePtr,
    IN MCESD_U32 fwCodeSizeDW,
    OUT MCESD_U16 *errCode
)
{
    MCESD_U32 expectedChecksum, actualChecksum;

    MCESD_ATTEMPT(calculateChecksum(fwCodePtr, fwCodeSizeDW, &expectedChecksum));
    MCESD_ATTEMPT(API_C112GX4_WriteReg(devPtr, 255, 0xA224, expectedChecksum));

    /* Write to MCU program memory */
    MCESD_ATTEMPT(API_C112GX4_UpdateRamCode(devPtr, 255 /* ignored */, fwCodePtr, fwCodeSizeDW, C112GX4_FW_MAX_SIZE, C112GX4_FW_BASE_ADDR, errCode));

    MCESD_ATTEMPT(API_C112GX4_ReadReg(devPtr, 255, 0xA228, &actualChecksum));

    return MCESD_OK;
#if 0
    if (actualChecksum == expectedChecksum)
    {
        *errCode = 0;
        return MCESD_OK;
    }
    else
    {
        *errCode = MCESD_IO_ERROR;
        return MCESD_FAIL;
        MCESD_DBG_ERROR("API_C112GX4_DownloadFirmware Failed: expectedChecksum = 0x%08x, actualChecksum = 0x%08x\n",expectedChecksum ,actualChecksum);
    }
#endif
}

MCESD_STATUS API_C112GX4_ProgCmnXData
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 *xDataPtr,
    IN MCESD_U32 xDataSizeDW,
    OUT MCESD_U16 *errCode
)
{
    MCESD_U32 expectedChecksum, actualChecksum, checksumPass;

    MCESD_ATTEMPT(calculateChecksum(xDataPtr, xDataSizeDW, &expectedChecksum));
    MCESD_ATTEMPT(API_C112GX4_WriteReg(devPtr, 255, 0xA3A4, expectedChecksum));

    /* Write Common XDATA to MCU */
    MCESD_ATTEMPT(API_C112GX4_UpdateRamCode(devPtr, 255 /* ignored */, xDataPtr, xDataSizeDW, C112GX4_XDATA_MAX_SIZE, C112GX4_XDATA_CMN_BASE_ADDR, errCode));

    MCESD_ATTEMPT(API_C112GX4_ReadReg(devPtr, 255, 0xA3A8, &actualChecksum));
    MCESD_ATTEMPT(API_C112GX4_ReadReg(devPtr, 255, 0xA3AC, &checksumPass));

    if ((checksumPass & (1<<1)) && (actualChecksum == expectedChecksum))
    {
        *errCode = 0;
        return MCESD_OK;
    }
    else
    {
        MCESD_DBG_ERROR("API_C112GX4_ProgCmnXData Failed: expectedChecksum = 0x%08x, actualChecksum = 0x%08x\n",expectedChecksum ,actualChecksum);
        *errCode = MCESD_IO_ERROR;
        return MCESD_FAIL;
    }
}

MCESD_STATUS API_C112GX4_ProgLaneXData
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 *xDataPtr,
    IN MCESD_U32 xDataSizeDW,
    OUT MCESD_U16 *errCode
)
{
    MCESD_U32 expectedChecksum, actualChecksum, checksumPass;

    MCESD_ATTEMPT(calculateChecksum(xDataPtr, xDataSizeDW, &expectedChecksum));
    MCESD_ATTEMPT(API_C112GX4_WriteReg(devPtr, lane, 0x22E4, expectedChecksum));

    /* Write Lane XDATA to MCU */
    API_C112GX4_UpdateRamCode(devPtr, lane, xDataPtr, xDataSizeDW, C112GX4_XDATA_MAX_SIZE, C112GX4_XDATA_LANE_BASE_ADDR, errCode);

    MCESD_ATTEMPT(API_C112GX4_ReadReg(devPtr, lane, 0x22E8, &actualChecksum));
    MCESD_ATTEMPT(API_C112GX4_ReadReg(devPtr, lane, 0x2294, &checksumPass));

    if ((checksumPass & (1<<29)) && (actualChecksum == expectedChecksum))
    {
        *errCode = 0;
        return MCESD_OK;
    }
    else
    {
        MCESD_DBG_ERROR("API_C112GX4_ProgLaneXData Failed: expectedChecksum = 0x%08x, actualChecksum = 0x%08x\n",expectedChecksum ,actualChecksum);
        *errCode = MCESD_IO_ERROR;
        return MCESD_FAIL;

    }

}

MCESD_STATUS API_C112GX4_UpdateRamCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
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
        MCESD_ATTEMPT(API_C112GX4_WriteReg(devPtr, lane, address, code[index]));
        address += 4;
    }

    *errCode = 0;
    return MCESD_OK;
}

#endif /* C112GX4 */
