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
#include "mcesdTop.h"
#include "mcesdApiTypes.h"
#include "mcesdInitialization.h"

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
)
{
    /* Check FMCESD_READ_REG and FMCESD_WRITE_REG pointers */
    if ((readReg == NULL) || (writeReg == NULL))
    {
        MCESD_DBG_ERROR("mcesdLoadDriver FMCESD_READ_REG or FMCESD_WRITE_REG pointers are NULL.\n");
        return MCESD_API_ERR_SYS_CFG;
    }

    /* Check FMCESD_SET_PIN_CFG and FMCESD_GET_PIN_CFG pointers */
    if ((setPinCfg == NULL) || (getPinCfg == NULL))
    {
        MCESD_DBG_ERROR("mcesdLoadDriver FMCESD_SET_PIN_CFG or FMCESD_GET_PIN_CFG pointers are NULL.\n");
        return MCESD_API_ERR_SYS_CFG;
    }

    /* Check FMCESD_WAIT_FUNC pointer */
    if (waitFunc == NULL)
    {
        MCESD_DBG_ERROR("mcesdLoadDriver FMCESD_WAIT_FUNC pointer is NULL.\n");
        return MCESD_API_ERR_SYS_CFG;
    }

    /* Check MCESD_DEV pointer */
    if (devPtr == NULL)
    {
        MCESD_DBG_ERROR("mcesdLoadDriver MCESD_DEV pointer is NULL.\n");
        return MCESD_API_ERR_DEV;
    }

    devPtr->ipMajorRev = ipMajorRev;
    devPtr->ipMinorRev = ipMinorRev;
    devPtr->fmcesdReadReg = readReg;
    devPtr->fmcesdWriteReg = writeReg;
    devPtr->fmcesdSetPinCfg = setPinCfg;
    devPtr->fmcesdGetPinCfg = getPinCfg;
    devPtr->fmcesdWaitFunc = waitFunc;
    devPtr->appData = appData;
    devPtr->devEnabled = MCESD_TRUE;

    MCESD_DBG_INFO("mcesdLoadDriver successful.\n");
    return MCESD_OK;
}

/*
MCESD_STATUS mcesdUnloadDriver
(
    IN MCESD_DEV_PTR devPtr
)
{
    Does nothing for now
    devPtr = NULL;
    return MCESD_OK;
}
*/