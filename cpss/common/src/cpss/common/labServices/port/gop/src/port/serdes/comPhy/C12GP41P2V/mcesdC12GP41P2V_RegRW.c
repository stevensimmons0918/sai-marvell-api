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
#include "../mcesdTop.h"
#include "../mcesdApiTypes.h"
#include "mcesdC12GP41P2V_Defs.h"
#include "mcesdC12GP41P2V_RegRW.h"
#include "mcesdC12GP41P2V_HwCntl.h"

#ifdef C12GP41P2V

MCESD_STATUS API_C12GP41P2V_WriteReg
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 reg,
    IN MCESD_U16 value
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_HwWriteReg(devPtr, reg, value));
    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_ReadReg
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 reg,
    OUT MCESD_U16 *data
)
{
    MCESD_ATTEMPT(API_C12GP41P2V_HwReadReg(devPtr, reg, data));
    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_WriteField
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_FIELD_PTR fieldPtr,
    IN MCESD_U16 value
)
{
    MCESD_U16 regValue, combinedValue;

    MCESD_ATTEMPT(API_C12GP41P2V_HwReadReg(devPtr, fieldPtr->reg, &regValue));

    /* Modify the register value with the desired field value */
    combinedValue = (regValue & fieldPtr->retainMask) | (value << fieldPtr->loBit);

    /* Call low level API_C12GP41P2V_HwWriteReg()*/
    MCESD_ATTEMPT(API_C12GP41P2V_HwWriteReg(devPtr, fieldPtr->reg, combinedValue));

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_ReadField
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_FIELD_PTR fieldPtr,
    OUT MCESD_U16 *data
)
{
    MCESD_U16 regValue;

    /* Call high level API_C12GP41P2V_ReadReg()*/
    MCESD_ATTEMPT(API_C12GP41P2V_ReadReg(devPtr, fieldPtr->reg, &regValue));

    /* Compute the field value */
    *data = (regValue & fieldPtr->mask) >> fieldPtr->loBit;

    return MCESD_OK;
}

MCESD_STATUS API_C12GP41P2V_PollField
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_FIELD_PTR fieldPtr,
    IN MCESD_U16 value,
    IN MCESD_U32 timeout_ms
)
{
    MCESD_U32 i;

    for (i = 0; i < timeout_ms; i++)
    {
        MCESD_U16 data;
        MCESD_ATTEMPT(API_C12GP41P2V_ReadField(devPtr, fieldPtr, &data));
        if (data == value)
            return MCESD_OK;

        MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 1));
    }

    return MCESD_FAIL;
}

MCESD_STATUS API_C12GP41P2V_PollPin
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C12GP41P2V_PIN pin,
    IN MCESD_U16 value,
    IN MCESD_U32 timeout_ms
)
{
    MCESD_U32 i;

    for (i = 0; i < timeout_ms; i++)
    {
        MCESD_U16 data;
        MCESD_ATTEMPT(API_C12GP41P2V_HwGetPinCfg(devPtr, pin, &data));
        if (data == value)
            return MCESD_OK;

        MCESD_ATTEMPT(API_C12GP41P2V_Wait(devPtr, 1));
    }

    return MCESD_FAIL;
}

#endif /* C28GP4X2 */