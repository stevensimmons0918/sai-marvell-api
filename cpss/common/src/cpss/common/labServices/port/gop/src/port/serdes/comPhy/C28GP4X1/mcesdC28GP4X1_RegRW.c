/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
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
#include "mcesdC28GP4X1_Defs.h"
#include "mcesdC28GP4X1_RegRW.h"
#include "mcesdC28GP4X1_HwCntl.h"

#ifdef C28GP4X1

/* Forward protoype for internal function only used by this module
static MCESD_STATUS mcesdWriteFieldDirect
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_FIELD field,
    IN MCESD_U32 value
);
*/

MCESD_STATUS API_C28GP4X1_WriteReg
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 reg,
    IN MCESD_U32 value
)
{
    MCESD_ATTEMPT(API_C28GP4X1_HwWriteReg(devPtr, reg, value));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_ReadReg
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 reg,
    OUT MCESD_U32* data
)
{
    MCESD_ATTEMPT(API_C28GP4X1_HwReadReg(devPtr, reg, data));
    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_WriteField
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_FIELD_PTR fieldPtr,
    IN MCESD_U32 value
)
{
    MCESD_U32 regValue, combinedValue;

    MCESD_ATTEMPT(API_C28GP4X1_ReadReg(devPtr, fieldPtr->reg, &regValue));

    /* Modify the register value with the desired field value */
    combinedValue = (regValue & fieldPtr->retainMask) | ( value << fieldPtr->loBit );

    /* Call low level API_C28GP4X1_HwWriteReg() */
    MCESD_ATTEMPT(API_C28GP4X1_HwWriteReg(devPtr, fieldPtr->reg, combinedValue));

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_ReadField
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_FIELD_PTR fieldPtr,
    OUT MCESD_U32* data
)
{
    MCESD_U32 regValue;

    /* Call high level API_C28GP4X1_ReadReg() */
    MCESD_ATTEMPT(API_C28GP4X1_ReadReg(devPtr, fieldPtr->reg, &regValue));

    /* Compute the field value */
    *data = (regValue & fieldPtr->mask) >> fieldPtr->loBit;

    return MCESD_OK;
}

MCESD_STATUS API_C28GP4X1_PollField
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_FIELD_PTR fieldPtr,
    IN MCESD_U32 value,
    IN MCESD_U32 timeout_ms
)
{
    MCESD_U32 i;

    for (i = 0; i < timeout_ms; i++)
    {
        MCESD_U32 data;
        MCESD_ATTEMPT(API_C28GP4X1_ReadField(devPtr, fieldPtr, &data));
        if (data == value)
            return MCESD_OK;

        MCESD_ATTEMPT(API_C28GP4X1_Wait(devPtr, 1));
    }

    return MCESD_FAIL;
}

MCESD_STATUS API_C28GP4X1_PollPin
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X1_PIN pin,
    IN MCESD_U16 value,
    IN MCESD_U32 timeout_ms
)
{
    MCESD_U32 i;

    for (i = 0; i < timeout_ms; i++)
    {
        MCESD_U16 data;
        MCESD_ATTEMPT(API_C28GP4X1_HwGetPinCfg(devPtr, pin, &data));
        if (data == value)
            return MCESD_OK;

        MCESD_ATTEMPT(API_C28GP4X1_Wait(devPtr, 1));
    }

    return MCESD_FAIL;
}

/*
static MCESD_STATUS mcesdWriteFieldDirect
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_FIELD field,
    IN MCESD_U32 value
)
{
    MCESD_U32 regValue, combinedValue;

    MCESD_ATTEMPT(API_C28GP4X1_HwReadReg(devPtr, field.reg, &regValue));

    Modify the register value with the desired field value
    combinedValue = (regValue & field.retainMask) | ( value << field.loBit );

    MCESD_ATTEMPT(API_C28GP4X1_HwWriteReg(devPtr, field.reg, combinedValue));

    return MCESD_OK;
}
*/

#endif /* C28GP4X1 */
