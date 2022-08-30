/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to download MCU firmware to Marvell
COMPHY_28G_PIPE4_RPLL_X4_1P2V
********************************************************************/
#ifndef MCESD_C28GP4X4_FW_DOWNLOAD_H
#define MCESD_C28GP4X4_FW_DOWNLOAD_H

#ifdef C28GP4X4

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

@param[out] errCode - if return status is MCESD_FAIL, errCode may contain additional information about the error: MCESD_IO_ERROR, MCESD_IMAGE_TOO_LARGE_TO_DOWNLOAD

@note Can only be downloaded as part of the API_C28GP4X4_PowerOnSeq() sequence.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_DownloadFirmware
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 *fwCodePtr,
    IN MCESD_U32 fwCodeSizeDW,
    OUT MCESD_U16 *errCode
);

/**
@brief  Programs common XDATA

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  xDataPtr - pointer to common XDATA
@param[in]  xDataSizeDW - size of common XDATA in DWORDS

@param[out] errCode - if return status is MCESD_FAIL, errCode may contain additional information about the error: MCESD_IO_ERROR, MCESD_IMAGE_TOO_LARGE_TO_DOWNLOAD

@note Can only be downloaded as part of the API_C28GP4X4_PowerOnSeq() sequence.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_ProgCmnXData
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 *xDataPtr,
    IN MCESD_U32 xDataSizeDW,
    OUT MCESD_U16 *errCode
);

/**
@brief  Programs lane XDATA

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  xDataPtr - pointer to lane XDATA
@param[in]  xDataSizeDW - size of lane XDATA in DWORDS

@param[out] errCode - if return status is MCESD_FAIL, errCode may contain additional information about the error: MCESD_IO_ERROR, MCESD_IMAGE_TOO_LARGE_TO_DOWNLOAD

@note Can only be downloaded as part of the API_C28GP4X4_PowerOnSeq() sequence.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_ProgLaneXData
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 *xDataPtr,
    IN MCESD_U32 xDataSizeDW,
    OUT MCESD_U16 *errCode
);

/**
@brief  Write firmware image to MCU program memory

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  code - firmware code to be downloaded into MCU program memory
@param[in]  codeSize - firmware code size in DWORDs (32-bit) data.
@param[in]  memSize - actual size of program memory. codeSize is checked to ensure that it is no larger than memSize
@param[in]  address - the starting address to write the code

@param[out] errCode - if return status is MCESD_FAIL, errCode may contain additional information about the error: MCESD_IO_ERROR, MCESD_IMAGE_TOO_LARGE_TO_DOWNLOAD

@note Expected to only be called by API_C28GP4X4_DownloadFirmware()

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_UpdateRamCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 code[],
    IN MCESD_U32 codeSize,
    IN MCESD_U32 memSize,
    IN MCESD_U32 address,
    OUT MCESD_U16 *errCode
);

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif

#endif /* C28GP4X4 */

#endif /* defined MCESD_C28GP4X4_FW_DOWNLOAD_H */
