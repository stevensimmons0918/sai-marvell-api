/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to configure Marvell CE SERDES IP:
5FFP_COMPHY_56G_PIPE5_X4_4PLL
********************************************************************/
#ifndef MCESD_N5XC56GP5X4_DEVICE_INIT_H
#define MCESD_N5XC56GP5X4_DEVICE_INIT_H

#ifdef N5XC56GP5X4

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
MCESD_STATUS API_N5XC56GP5X4_PowerOnSeq
(
    IN MCESD_DEV_PTR devPtr,
    IN S_N5XC56GP5X4_PowerOn powerOn
);

/**
@brief  Enter into Slumber Mode

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_PowerOffLane
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

#if C_LINKAGE
#if defined __cplusplus 
}
#endif 
#endif

#endif /* N5XC56GP5X4 */

#endif /* defined MCESD_N5XC56GP5X4_DEVICE_INIT_H */
