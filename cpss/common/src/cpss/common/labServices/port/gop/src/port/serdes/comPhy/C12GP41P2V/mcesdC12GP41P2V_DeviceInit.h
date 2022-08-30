/*******************************************************************************
Copyright (C) 2018, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to configure Marvell CE SERDES IP:
COMPHY_12G_PIPE4_1P2V
********************************************************************/
#ifndef MCESD_C12GP41P2V_DEVICE_INIT_H
#define MCESD_C12GP41P2V_DEVICE_INIT_H

#ifdef C12GP41P2V

#if C_LINKAGE
#if defined __cplusplus
extern "C" {
#endif
#endif

/**
@brief  power on sequence

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  powerOn - power on struct

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_PowerOnSeq
(
    IN MCESD_DEV_PTR devPtr,
    IN S_C12GP41P2V_PowerOn powerOn
);

/**
@brief  Enter into Slumber Mode

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_PowerOffLane
(
    IN MCESD_DEV_PTR devPtr
);

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif

#endif /* C12GP41P2V */

#endif /* defined MCESD_C12GP41P2V_DEVICE_INIT_H */