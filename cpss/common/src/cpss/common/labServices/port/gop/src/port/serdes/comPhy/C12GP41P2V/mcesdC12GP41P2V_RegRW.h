/*******************************************************************************
Copyright (C) 2018, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions to read/write registers for higher level API
to bridge host's hardware-specific IO.
********************************************************************/
#ifndef MCESD_C12GP41P2V_REGRW_H
#define MCESD_C12GP41P2V_REGRW_H

#ifdef C12GP41P2V

#if C_LINKAGE
#if defined __cplusplus
extern "C" {
#endif
#endif

/**
@brief  Writes a 16-bit value to the specified address

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  reg - register address
@param[in]  value - data to write

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_WriteReg
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
MCESD_STATUS API_C12GP41P2V_ReadReg
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 reg,
    OUT MCESD_U16 *data
);

/**
@brief  Writes a 16-bit value to the specified field

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  fieldPtr - pointer to MCESD_FIELD to write data
@param[in]  value - data to write

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_WriteField
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_FIELD_PTR fieldPtr,
    IN MCESD_U16 value
);

/**
@brief  Reads a 16-bit data from the specified field

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  fieldPtr - pointer to MCESD_FIELD to read from

@param[out] data - Returns 16 bit data from field

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_ReadField
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_FIELD_PTR fieldPtr,
    OUT MCESD_U16 *data
);

/**
@brief  Waits for the specified value to be present for the field

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  fieldPtr - pointer to MCESD_FIELD to check value
@param[in]  value - value to match
@param[in]  timeout_ms - timeout in milliseconds

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_PollField
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_FIELD_PTR fieldPtr,
    IN MCESD_U16 value,
    IN MCESD_U32 timeout_ms
);

/**
@brief  Waits for the specified value to be present for the pin

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  pin - enum of type E_C12GP41P2V_PIN
@param[in]  value - value to match
@param[in]  timeout_ms - timeout in milliseconds

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C12GP41P2V_PollPin
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_PIN pin,
    IN MCESD_U16 value,
    IN MCESD_U32 timeout_ms
);

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif

#endif /* C12GP41P2V */

#endif /* defined MCESD_C12GP41P2V_REGRW_H */