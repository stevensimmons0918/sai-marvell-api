/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to download MCU firmware to Marvell
5FFP_COMPHY_56G_PIPE5_X4_4PLL
********************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "../mcesdTop.h"
#include "../mcesdApiTypes.h"
#include "../mcesdUtils.h"
#include "mcesdN5XC56GP5X4_Defs.h"
#include "mcesdN5XC56GP5X4_RegRW.h"
#include "mcesdN5XC56GP5X4_FwDownload.h"

#ifdef N5XC56GP5X4

MCESD_STATUS API_N5XC56GP5X4_DownloadFirmware
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 *fwCodePtr,
    IN MCESD_U32 fwCodeSizeDW,
    IN MCESD_U32 address,
    OUT MCESD_U16 *errCode
)
{
    MCESD_U32 index;

    *errCode = MCESD_IO_ERROR;

    /* Check if the code about to be downloaded can fit into the device's memory */
    if (fwCodeSizeDW > N5XC56GP5X4_FW_MAX_SIZE)
    {
        *errCode = MCESD_IMAGE_TOO_LARGE_TO_DOWNLOAD;
        return MCESD_FAIL;
    }

    /* Write to program memory incrementally */
    for (index = 0; index < fwCodeSizeDW; index++)
    {
        MCESD_ATTEMPT(API_N5XC56GP5X4_WriteReg(devPtr, 255 /* ignored */, address, fwCodePtr[index]));
        address += 4;
    }

    *errCode = 0;
    return MCESD_OK;
}

#endif /* N5XC56GP5X4 */