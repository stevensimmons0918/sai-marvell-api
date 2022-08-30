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
#include "mcesdC112GX4_Defs.h"
#include "mcesdC112GX4_RegRW.h"
#include "mcesdC112GX4_HwCntl.h"

#ifdef C112GX4

/* Forward protoype for internal function only used by this module */
static MCESD_STATUS mcesdWriteFieldDirect
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_FIELD field,
    IN MCESD_U32 value
);

/* Forward protoype for internal function only used by this module
static MCESD_STATUS mcesdReadFieldDirect
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_FIELD field,
    OUT MCESD_U32* data
);
*/

MCESD_STATUS API_C112GX4_WriteReg
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 reg,
    IN MCESD_U32 value
)
{
    /* Lane switching is necessary for register adresses from 0 ~ 0x7FFC */
    if ((reg < 0x8000) && (lane != 255))
    {
        MCESD_FIELD laneSel = F_C112GX4R1P0_LANE_SEL;
        MCESD_ATTEMPT(mcesdWriteFieldDirect(devPtr, laneSel, (MCESD_U32)lane));
    }

    MCESD_ATTEMPT(API_C112GX4_HwWriteReg(devPtr, reg, value));

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_ReadReg
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 reg,
    OUT MCESD_U32* data
)
{
    /* Lane switching is necessary for register adresses from 0 ~ 0x7FFC */
    if ((reg < 0x8000) && (lane != 255))
    {
        MCESD_FIELD laneSel = F_C112GX4R1P0_LANE_SEL;
        MCESD_ATTEMPT(mcesdWriteFieldDirect(devPtr, laneSel, (MCESD_U32)lane));
    }

    MCESD_ATTEMPT(API_C112GX4_HwReadReg(devPtr, reg, data));
    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_WriteField
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_FIELD_PTR fieldPtr,
    IN MCESD_U32 value
)
{
    MCESD_U32 regValue, combinedValue;

    /* Call high level API_C112GX4_ReadReg() which will switch lanes if necessary */
    MCESD_ATTEMPT(API_C112GX4_ReadReg(devPtr, lane, fieldPtr->reg, &regValue));

    /* Modify the register value with the desired field value */
    combinedValue = (regValue & fieldPtr->retainMask) | (value << fieldPtr->loBit);

    /* Call low level API_C112GX4_HwWriteReg() since the lane switching was already done */
    MCESD_ATTEMPT(API_C112GX4_HwWriteReg(devPtr, fieldPtr->reg, combinedValue));

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_ReadField
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_FIELD_PTR fieldPtr,
    OUT MCESD_U32* data
)
{
    MCESD_U32 regValue;

    /* Call high level API_C112GX4_ReadReg() which will switch lanes if necessary */
    MCESD_ATTEMPT(API_C112GX4_ReadReg(devPtr, lane, fieldPtr->reg, &regValue));

    /* Compute the field value */
    *data = (regValue & fieldPtr->mask) >> fieldPtr->loBit;

    return MCESD_OK;
}

MCESD_STATUS API_C112GX4_PollField
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_FIELD_PTR fieldPtr,
    IN MCESD_U32 value,
    IN MCESD_U32 timeout_ms
)
{
#ifdef GM_USED /* the GM not supports the SERDES */
    /* avoid warnings */
    GT_UNUSED_PARAM(devPtr);
    GT_UNUSED_PARAM(lane);
    GT_UNUSED_PARAM(fieldPtr);
    GT_UNUSED_PARAM(value);
    GT_UNUSED_PARAM(timeout_ms);
    return MCESD_OK;
#else
    MCESD_U32 i;

    for (i = 0; i < timeout_ms; i++)
    {
        MCESD_U32 data;
        MCESD_ATTEMPT(API_C112GX4_ReadField(devPtr, lane, fieldPtr, &data));
        if (data == value)
            return MCESD_OK;

        MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 1));
    }

    return MCESD_FAIL;
#endif
}

MCESD_STATUS API_C112GX4_PollPin
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C112GX4_PIN pin,
    IN MCESD_U16 value,
    IN MCESD_U32 timeout_ms
)
{
    MCESD_U32 i;

    for (i = 0; i < timeout_ms; i++)
    {
        MCESD_U16 data;
        MCESD_ATTEMPT(API_C112GX4_HwGetPinCfg(devPtr, pin, &data));
        if (data == value)
            return MCESD_OK;

        MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 1));
    }

    return MCESD_FAIL;
}

static MCESD_STATUS mcesdWriteFieldDirect
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_FIELD field,
    IN MCESD_U32 value
)
{
    MCESD_U32 regValue, combinedValue;

    MCESD_ATTEMPT(API_C112GX4_HwReadReg(devPtr, field.reg, &regValue));

    /* Modify the register value with the desired field value */
    combinedValue = (regValue & field.retainMask) | (value << field.loBit);

    MCESD_ATTEMPT(API_C112GX4_HwWriteReg(devPtr, field.reg, combinedValue));

    return MCESD_OK;
}

/*
static MCESD_STATUS mcesdReadFieldDirect
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_FIELD field,
    OUT MCESD_U32* data
)
{
    MCESD_U32 regValue;

    MCESD_ATTEMPT(API_C112GX4_HwReadReg(devPtr, field.reg, &regValue));

    Compute the field value
    *data = (regValue & field.mask) >> field.loBit;

    return MCESD_OK;
}
*/

#endif /* C112GX4 */
