/*******************************************************************************
Copyright (C) 2018, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions and global data for interfacing with the
host's hardware-specific IO in order to control and query the Marvell
COMPHY_12G_PIPE4_1P2V

These functions as written were tested with a USB-interface to
access Marvell's EVB. These functions must be ported to
the host's specific platform.
********************************************************************/
#ifndef MCESD_C12GP41P2V_HWCNTL_H
#define MCESD_C12GP41P2V_HWCNTL_H

#ifdef C12GP41P2V

#if C_LINKAGE
#if defined __cplusplus
extern "C" {
#endif
#endif

/*******************************************************************
PORT THESE FUNCTIONS
*******************************************************************/

/**
@brief  Writes a 16-bit value to the specified address

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  reg - register address
@param[in]  value - data to write

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_HwWriteReg
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 reg,
    IN MCESD_U16 value
);

/**
@brief  Reads a 16-bit data from the specified address

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  reg - register address

@param[out] data - Returns 16 bit data from register address

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_HwReadReg
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 reg,
    OUT MCESD_U16 *data
);

/**
@brief  Configures the specified pin

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  pin - the selected E_C28GP4X2_PIN to configure
@param[in]  pinValue - configuration value

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_HwSetPinCfg
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_PIN pin,
    IN MCESD_U16 pinValue
);

/**
@brief  Retrieves the pin configuration

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  pin - the selected E_C28GP4X2_PIN to configure

@param[out] pinValue - configuration value of the specified pin

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_HwGetPinCfg
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_PIN pin,
    OUT MCESD_U16 *pinValue
);

/**
@brief  Waits for the specified number of milliseconds

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  ms - number of milliseconds to wait

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_Wait
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 ms
);

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif

#endif /* C12GP41P2V */

#endif /* defined MCESD_C12GP41P2V_HWCNTL_H */