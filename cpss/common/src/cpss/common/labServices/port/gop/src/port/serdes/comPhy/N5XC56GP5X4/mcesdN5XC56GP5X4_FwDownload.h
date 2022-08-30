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
#ifndef MCESD_N5XC56GP5X4_FW_DOWNLOAD_H
#define MCESD_N5XC56GP5X4_FW_DOWNLOAD_H

#ifdef N5XC56GP5X4

#if C_LINKAGE
#if defined __cplusplus
extern "C" {
#endif
#endif

/**
@brief  Downloads MCU firmware to program memory

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  fwCodePtr - pointer to firmware data
@param[in]  fwCodeSizeDW - size of the firmware data in DWORDS
@param[in]  address - the starting address to write the code

@param[out] errCode - if return status is MCESD_FAIL, errCode may contain additional information about the error: MCESD_IO_ERROR, MCESD_IMAGE_TOO_LARGE_TO_DOWNLOAD

@note Can only be downloaded as part of the API_N5XC56GP5X4_PowerOnSeq() sequence.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_DownloadFirmware
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 *fwCodePtr,
    IN MCESD_U32 fwCodeSizeDW,
    IN MCESD_U32 address,
    OUT MCESD_U16 *errCode
);

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif

#endif /* N5XC56GP5X4 */

#endif /* defined MCESD_N5XC56GP5X4_FW_DOWNLOAD_H */
