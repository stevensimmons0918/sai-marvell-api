/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to configure Marvell CE SERDES IP:
COMPHY_28G_PIPE4_RPLL_X4_1P2V
********************************************************************/
#ifndef MCESD_C28GP4X4_DEVICE_INIT_H
#define MCESD_C28GP4X4_DEVICE_INIT_H

#ifdef C28GP4X4

#if C_LINKAGE
#if defined __cplusplus
    extern "C" {
#endif
#endif

/**
@brief  It is expected that SoC firmware performs PHY Reset, checks bandgap ready and sets up 400MHz MCU Clock.
        Afterwards, this API should be called which will download MCU firmware and configure SERDES to the specified speed.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  powerOn - power on struct

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_PowerOnSeq
(
    IN MCESD_DEV_PTR devPtr,
    IN S_C28GP4X4_PowerOn powerOn
);

/**
@brief  Enter into Slumber Mode

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_PowerOffLane
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

/**
@brief  It is expected that SoC firmware performs PHY Reset, checks bandgap ready and sets up 400MHz MCU Clock.
        Afterwards, this API should be called which will download MCU firmware and program xdata.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  fwDownload - pointer to firmware download function
@param[in]  cmnXDataPtr - pointer to common XDATA
@param[in]  cmnXDataSizeDW - size of common XDATA in DWORDS
@param[in]  laneXDataPtr - pointer to lane XDATA
@param[in]  laneXDataSizeDW - size of lane XDATA in DWORDS

@param[out] errCode - error code returned

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X4_DownloadFirmwareProgXData
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 *fwCodePtr,
    IN MCESD_U32 fwCodeSizeDW,
    IN MCESD_U32 *cmnXDataPtr,
    IN MCESD_U32 cmnXDataSizeDW,
    IN MCESD_U32 *laneXDataPtr,
    IN MCESD_U32 laneXDataSizeDW
);

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif

#endif /* C28GP4X4 */

#endif /* defined MCESD_C28GP4X4_DEVICE_INIT_H */
