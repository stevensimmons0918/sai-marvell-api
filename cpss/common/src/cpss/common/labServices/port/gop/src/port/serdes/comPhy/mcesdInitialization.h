/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions for initializing the driver and setting
up the user-provide access functions for the Marvell CE SERDES IPs
********************************************************************/
#ifndef MCESDINITIALIZATION_H
#define MCESDINITIALIZATION_H

#if C_LINKAGE
#ifdef __cplusplus
extern "C" {
#endif
#endif

/**
@brief  Initializes driver

@param[in] ipMajorRev - major revision of IP
@param[in] ipMinorRev - minor revision of IP
@param[in] readReg - pointer to host's function to perform Read Register
@param[in] writeReg - pointer to host's function to perform Write Register
@param[in] setPinCfg - pointer to host's function to set pin configuration
@param[in] getPinCfg - pointer to host's function to get pin configuration
@param[in] waitFunc - pointer to host's function to wait for the specified milliseconds to elapse
@param[in] appData - pointer to host specific data structure

@param[out] devPtr - pointer to holds device information to be used for each API call.

@note Marvell Driver Initialization Routine.
@note This is the first routine that needs be called by system software.
@note It takes parameters from system software, and retures a pointer (*dev) to a data structure which includes infomation related to this Marvell Phy device. This pointer (*dev) is then used for all the API functions.
@note The following is the job performed by this routine:
@note 1. store read/write function into the given MCESD_DEV structure
@note 2. run any device specific initialization routine
@note 3. create semaphore if required
@note mcesdUnloadDriver is also provided to do driver cleanup.
@note An MCESD_DEV is required for each type of PHY device in the system.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS mcesdLoadDriver
(
    IN MCESD_U8             ipMajorRev,
    IN MCESD_U8             ipMinorRev,
    IN FMCESD_READ_REG      readReg,
    IN FMCESD_WRITE_REG     writeReg,
    IN FMCESD_SET_PIN_CFG   setPinCfg,
    IN FMCESD_GET_PIN_CFG   getPinCfg,
    IN FMCESD_WAIT_FUNC     waitFunc,
    IN MCESD_PVOID          appData,
    OUT MCESD_DEV_PTR       devPtr
);

/*
This function cleans up memory allocated by mcesdLoadDriver

devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

This function should be called only after successful execution of mcesdLoadDriver().

MCESD_OK - on success
MCESD_FAIL - on error

MCESD_STATUS mcesdUnloadDriver
(
    IN MCESD_DEV_PTR devPtr
);
*/

#if C_LINKAGE
#ifdef __cplusplus
}
#endif
#endif

#endif /* MCESDINITIALIZATION_H */
