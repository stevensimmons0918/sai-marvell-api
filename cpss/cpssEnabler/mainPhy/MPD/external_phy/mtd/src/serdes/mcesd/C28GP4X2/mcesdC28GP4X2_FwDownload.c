/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to download MCU firmware to Marvell
COMPHY_28G_PIPE4_X2
********************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include <serdes/mcesd/mcesdTop.h>
#include <serdes/mcesd/mcesdApiTypes.h>
#include <serdes/mcesd/mcesdUtils.h>
#include <serdes/mcesd/C28GP4X2/mcesdC28GP4X2_Defs.h>
#include <serdes/mcesd/C28GP4X2/mcesdC28GP4X2_API.h>
#include <serdes/mcesd/C28GP4X2/mcesdC28GP4X2_RegRW.h>
#include <serdes/mcesd/C28GP4X2/mcesdC28GP4X2_FwDownload.h>

#ifdef C28GP4X2

MCESD_STATUS API_C28GP4X2_DownloadFirmware
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32* fwCodePtr,
    IN MCESD_U32 fwCodeSizeDW,
    OUT MCESD_U16 *errCode
)
{
    MCESD_FIELD fieldProgRamSel = F_C28GP4X2R2P0_PROG_RAM_SEL;

    *errCode = MCESD_IO_ERROR;

    /* Select MCU program memory */
    MCESD_ATTEMPT(API_C28GP4X2_WriteField(devPtr, 255 /* ignored */, &fieldProgRamSel, 0));

    /* Write to MCU program memory */
    MCESD_ATTEMPT(API_C28GP4X2_UpdateRamCode(devPtr, 255 /* ignored */, fwCodePtr, fwCodeSizeDW, C28GP4X2_FW_MAX_SIZE, C28GP4X2_FW_BASE_ADDR, errCode));

    *errCode = 0;
    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X2_ProgCmnXData
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 *xDataPtr,
    IN MCESD_U32 xDataSizeDW,
    OUT MCESD_U16 *errCode
)
{
    /* Write Common XDATA to MCU */
    return API_C28GP4X2_UpdateRamCode(devPtr, 255 /* ignored */, xDataPtr, xDataSizeDW, C28GP4X2_XDATA_MAX_SIZE, C28GP4X2_XDATA_CMN_BASE_ADDR, errCode);
}

MCESD_STATUS API_C28GP4X2_ProgLaneXData
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 *xDataPtr,
    IN MCESD_U32 xDataSizeDW,
    OUT MCESD_U16 *errCode
)
{
    /* Write Lane XDATA to MCU */
    return API_C28GP4X2_UpdateRamCode(devPtr, lane, xDataPtr, xDataSizeDW, C28GP4X2_XDATA_MAX_SIZE, C28GP4X2_XDATA_LANE_BASE_ADDR, errCode);
}

MCESD_STATUS API_C28GP4X2_UpdateRamCode
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
        MCESD_ATTEMPT(API_C28GP4X2_WriteReg(devPtr, lane, address, code[index]));
        address += 4;
    }

    *errCode = 0;
    return MCESD_OK;
}

#endif /* C28GP4X2 */
