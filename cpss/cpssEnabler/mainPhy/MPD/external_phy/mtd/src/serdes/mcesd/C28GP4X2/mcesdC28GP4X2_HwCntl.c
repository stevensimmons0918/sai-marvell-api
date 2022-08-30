/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions and global data for interfacing with the
host's hardware-specific IO in order to control and query the Marvell
COMPHY_28G_PIPE4_X2

These functions as written were tested with a USB-interface to
access Marvell's EVB. These functions must be ported to 
the host's specific platform.
********************************************************************/
#include <serdes/mcesd/mcesdTop.h>
#include <serdes/mcesd/mcesdApiTypes.h>
#include <serdes/mcesd/C28GP4X2/mcesdC28GP4X2_Defs.h>
#include <serdes/mcesd/C28GP4X2/mcesdC28GP4X2_HwCntl.h>

#ifdef C28GP4X2

MCESD_STATUS API_C28GP4X2_HwWriteReg
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 reg,
    IN MCESD_U32 value
)
{
    if (devPtr->fmcesdWriteReg == NULL)
    {
        MCESD_DBG_ERROR("fmcesdWriteReg function pointer is NULL\n");
        return MCESD_FAIL;
    }

    if (devPtr->fmcesdWriteReg(devPtr, reg, value) == MCESD_FAIL)
    {
        MCESD_DBG_INFO("fmcesdWriteReg 0x%X failed to write to dev=C28GP4X2, reg=0x%X\n", value, reg);
        return MCESD_FAIL;
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X2_HwReadReg
( 
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 reg, 
    OUT MCESD_U32 *data
)
{
    if (devPtr->fmcesdReadReg == NULL)
    {
        MCESD_DBG_ERROR("fmcesdReadReg function pointer is NULL\n");
        return MCESD_FAIL;
    }

    if (devPtr->fmcesdReadReg(devPtr, reg, data) == MCESD_FAIL)
    {
        MCESD_DBG_INFO("fmcesdReadReg failed from dev=C28GP4X2, reg=0x%X\n", reg);
        return MCESD_FAIL;
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X2_HwSetPinCfg
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X2_PIN pin,
    IN MCESD_U16 pinValue
)
{
    if (devPtr->fmcesdSetPinCfg == NULL)
    {
        MCESD_DBG_ERROR("fmcesdSetPinCfg function pointer is NULL\n");
        return MCESD_FAIL;
    }

    if (devPtr->fmcesdSetPinCfg(devPtr, pin, pinValue) == MCESD_FAIL)
    {
        MCESD_DBG_INFO("fmcesdSetPinCfg 0x%X failed to configure dev=C28GP4X2, pin=0x%X\n", pinValue, pin);
        return MCESD_FAIL;
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X2_HwGetPinCfg
( 
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X2_PIN pin, 
    OUT MCESD_U16 *pinValue
)
{
    if (devPtr->fmcesdGetPinCfg == NULL)
    {
        MCESD_DBG_ERROR("fmcesdGetPinCfg function pointer is NULL\n");
        return MCESD_FAIL;
    }

    if (devPtr->fmcesdGetPinCfg(devPtr, pin, pinValue) == MCESD_FAIL)
    {
        MCESD_DBG_INFO("fmcesdGetPinCfg failed from dev=C28GP4X2, pin=0x%X\n", pin);
        return MCESD_FAIL;
    }

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X2_Wait
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 ms
)
{
    if (devPtr->fmcesdWaitFunc == NULL)
    {
        MCESD_DBG_ERROR("fmcesdWaitFunc function pointer is NULL\n");
        return MCESD_FAIL;
    }

    if (devPtr->fmcesdWaitFunc(devPtr, ms) == MCESD_FAIL)
    {
        MCESD_DBG_INFO("fmcesdWaitFunc failed from dev=C28GP4X2\n");
        return MCESD_FAIL;
    }

    return MCESD_OK;
}

#endif /* C28GP4X2 */
