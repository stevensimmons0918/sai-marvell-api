/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to configure Marvell CE SERDES IP:
COMPHY_28G_PIPE4_RPLL_1P2V
********************************************************************/
#ifndef MCESD_C28GP4X1_DEVICE_INIT_H
#define MCESD_C28GP4X1_DEVICE_INIT_H

#ifdef C28GP4X1

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
MCESD_STATUS API_C28GP4X1_PowerOnSeq
(
    IN MCESD_DEV_PTR devPtr,
    IN S_C28GP4X1_PowerOn powerOn
);

/**
@brief  Enter into Slumber Mode

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X1_PowerOffLane
(
    IN MCESD_DEV_PTR devPtr
);

/**
@brief  It is expected that SoC firmware performs PHY Reset, checks bandgap ready and sets up 400MHz MCU Clock.
        Afterwards, this API should be called which will download MCU firmware and program xdata.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  fwCodePtr - pointer to firmware data
@param[in]  fwCodeSizeDW - size of firmware data in DWORDS
@param[in]  cmnXDataPtr - pointer to common XDATA
@param[in]  cmnXDataSizeDW - size of common XDATA in DWORDS
@param[in]  laneXDataPtr - pointer to lane XDATA
@param[in]  laneXDataSizeDW - size of lane XDATA in DWORDS

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X1_DownloadFirmwareProgXData
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

#endif /* C28GP4X1 */

#endif /* defined MCESD_C28GP4X1_DEVICE_INIT_H */
