/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions and global data for interfacing with the
host's hardware-specific IO in order to control and query the Marvell
COMPHY_112G_X4

These functions as written were tested with a USB-interface to
access Marvell's EVB. These functions must be ported to
the host's specific platform.
********************************************************************/
#include "../mcesdTop.h"
#include "../mcesdApiTypes.h"
#include "mcesdC112GX4_Defs.h"
#include "mcesdC112GX4_HwCntl.h"
#include "mcesdC112GX4_HwCntl.h"
#ifdef SHARED_MEMORY
#include "../mvComphyIfCallbacks.h"
#endif



#ifdef C112GX4


MCESD_STATUS API_C112GX4_HwWriteReg
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 reg,
    IN MCESD_U32 value
)
{
    FMCESD_WRITE_REG    fmcesdWriteReg = devPtr->fmcesdWriteReg;

#ifdef SHARED_MEMORY
    fmcesdWriteReg = mvHwsComphyRegisterWriteCallback;
#endif

    if (fmcesdWriteReg== NULL)
    {
        MCESD_DBG_ERROR("fmcesdWriteReg function pointer is NULL\n");
        return MCESD_FAIL;
    }

    if (fmcesdWriteReg(devPtr, reg, value) == MCESD_FAIL)
    {
        MCESD_DBG_INFO("fmcesdWriteReg 0x%X failed to write to dev=C112GX4, reg=0x%X\n", value, reg);
        return MCESD_FAIL;
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_HwReadReg
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 reg,
    OUT MCESD_U32 *data
)
{
    FMCESD_READ_REG    fmcesdReadReg = devPtr->fmcesdReadReg;

#ifdef SHARED_MEMORY
    fmcesdReadReg = mvHwsComphyRegisterReadCallback;
#endif
    if (fmcesdReadReg == NULL)
    {
        MCESD_DBG_ERROR("fmcesdReadReg function pointer is NULL\n");
        return MCESD_FAIL;
    }

    if (fmcesdReadReg(devPtr, reg, data) == MCESD_FAIL)
    {
        MCESD_DBG_INFO("fmcesdReadReg failed from dev=C112GX4, reg=0x%X\n", reg);
        return MCESD_FAIL;
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_HwSetPinCfg
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C112GX4_PIN pin,
    IN MCESD_U16 pinValue
)
{
    FMCESD_SET_PIN_CFG  fmcesdSetPinCfg = devPtr->fmcesdSetPinCfg;

#ifdef SHARED_MEMORY
     fmcesdSetPinCfg = mvHwsComphySetPinCallback;
#endif

    if (fmcesdSetPinCfg == NULL)
    {
        MCESD_DBG_ERROR("fmcesdSetPinCfg function pointer is NULL\n");
        return MCESD_FAIL;
    }

    if (fmcesdSetPinCfg(devPtr, pin, pinValue) == MCESD_FAIL)
    {
        MCESD_DBG_INFO("fmcesdSetPinCfg 0x%X failed to configure dev=C112GX4, pin=0x%X\n", pinValue, pin);
        return MCESD_FAIL;
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_HwGetPinCfg
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C112GX4_PIN pin,
    OUT MCESD_U16 *pinValue
)
{
    FMCESD_GET_PIN_CFG  fmcesdGetPinCfg= devPtr->fmcesdGetPinCfg;;

#ifdef SHARED_MEMORY
        fmcesdGetPinCfg = mvHwsComphyGetPinCallback;
#endif

    if (fmcesdGetPinCfg == NULL)
    {
        MCESD_DBG_ERROR("fmcesdGetPinCfg function pointer is NULL\n");
        return MCESD_FAIL;
    }

    if (fmcesdGetPinCfg(devPtr, pin, pinValue) == MCESD_FAIL)
    {
        MCESD_DBG_INFO("fmcesdGetPinCfg failed from dev=C112GX4, pin=0x%X\n", pin);
        return MCESD_FAIL;
    }

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_Wait
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 ms
)
{
    FMCESD_WAIT_FUNC    fmcesdWaitFunc = devPtr->fmcesdWaitFunc;

#ifdef SHARED_MEMORY
    fmcesdWaitFunc = mvHwsComphyWait;
#endif

    if (fmcesdWaitFunc == NULL)
    {
        MCESD_DBG_ERROR("fmcesdWaitFunc function pointer is NULL\n");
        return MCESD_FAIL;
    }

    if (fmcesdWaitFunc(devPtr, ms) == MCESD_FAIL)
    {
        MCESD_DBG_INFO("fmcesdWaitFunc failed from dev=C112GX4\n");
        return MCESD_FAIL;
    }

    return MCESD_OK;
}

#endif /* C112GX4 */
