/*******************************************************************************
Copyright (C) 2014 - 2021, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/


/************************************************************************
This file contains functions to configure RS-FEC/KR-FEC
and read RS-FEC/KR-FEC status for the Marvell X35X0/E25X0 PHY
************************************************************************/
#include "mtdFeatures.h"
#include "mtdApiRegs.h"
#include "mtdApiTypes.h"
#include "mtdHwCntl.h"
#include "mtdAPI.h"
#include "mtdFEC.h"

#define MTD_RS_FEC_CONTROL              MTD_T_UNIT_PMA_PMD,0xA0C8
#define MTD_RS_FEC_STATUS               MTD_T_UNIT_PMA_PMD,0xA0C9
#define MTD_RS_FEC_CTRL_STATUS7         MTD_T_UNIT_PMA_PMD,0xA10B
#define MTD_RS_FEC_CORR_CW_LOW          MTD_T_UNIT_PMA_PMD,0xA0CA
#define MTD_RS_FEC_CORR_CW_UPPER        MTD_T_UNIT_PMA_PMD,0xA0CB
#define MTD_RS_FEC_UNCORR_CW_LOW        MTD_T_UNIT_PMA_PMD,0xA0CC
#define MTD_RS_FEC_UNCORR_CW_UPPER      MTD_T_UNIT_PMA_PMD,0xA0CD
#define MTD_RS_FEC_SYM_ERR_CTR_LOW      MTD_T_UNIT_PMA_PMD,0xA0D2
#define MTD_RS_FEC_SYM_ERR_CTR_UPPER    MTD_T_UNIT_PMA_PMD,0xA0D3

/*******************************************************************************
 MTD_STATUS mtdRsFECEnable
    Enables or disable the RS-FEC
*******************************************************************************/
MTD_STATUS mtdRsFECEnable
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_BOOL readToClear,
    IN MTD_U16 enable
)
{
    MTD_U16 temp;

    if (!devPtr)
    {
        MTD_DBG_ERROR("mtdRsFECEnable: Invalid device pointer. \n");
        return MTD_FAIL;
    }

    if (!MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        MTD_DBG_ERROR("mtdRsFECEnable: RS-FEC is not supported in this device. \n");
        return MTD_FAIL;
    } 

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_CUNIT_PORT_CTRL2, 13, 2, ((1<<1)|enable)));
    temp = (readToClear == MTD_TRUE) ? 0 : 1;
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_RS_FEC_CTRL_STATUS7, 0, 1, temp));

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdSetRsFecControl
    Configure the RS-FEC for those parameters that are configurable
*******************************************************************************/
MTD_STATUS mtdSetRsFecControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    IN MTD_U16 bypassIndicationEnable,
    IN MTD_U16 bypassCorrectionEnable
)
{
    MTD_U16 data16Bits;

    if (!devPtr)
    {
        MTD_DBG_ERROR("mtdSetRsFecControl: Invalid device pointer. \n");
        return MTD_FAIL;
    }

    if (!MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        MTD_DBG_ERROR("mtdSetRsFecControl: RS-FEC is not supported in this device. \n");
        return MTD_FAIL;
    }

    if (bypassCorrectionEnable == MTD_ENABLE && bypassIndicationEnable == MTD_ENABLE)
    {
        MTD_DBG_ERROR("mtdSetRsFecControl: Bypass both is an error.\n");
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_RS_FEC_CONTROL, &data16Bits));
    mtdHwSetRegFieldToWord(data16Bits, bypassCorrectionEnable, 0, 1, &data16Bits);
    mtdHwSetRegFieldToWord(data16Bits, bypassIndicationEnable, 1, 1, &data16Bits);
    MTD_ATTEMPT(mtdHwXmdioWrite(devPtr, port, MTD_RS_FEC_CONTROL, data16Bits));

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdGetRsFecControl
    Read the values of the current settings for the RS-FEC
*******************************************************************************/
MTD_STATUS mtdGetRsFecControl
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *bypassIndicationEnable,
    OUT MTD_U16 *bypassCorrectionEnable
)
{
    MTD_U16 data16Bits;

    *bypassIndicationEnable = *bypassCorrectionEnable = 0;

    if (!devPtr)
    {
        MTD_DBG_ERROR("mtdGetRsFecControl: Invalid device pointer. \n");
        return MTD_FAIL;
    }

    if (!MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        MTD_DBG_ERROR("mtdGetRsFecControl: RS-FEC is not supported in this device. \n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_RS_FEC_CONTROL, &data16Bits));
    mtdHwGetRegFieldFromWord(data16Bits, 0, 1, bypassCorrectionEnable);
    mtdHwGetRegFieldFromWord(data16Bits, 1, 1, bypassIndicationEnable);

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdGetRsFecStatus
    Get the high-level status of the RS-FEC
*******************************************************************************/
MTD_STATUS mtdGetRsFecStatus
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U16 *pcsLaneAlignment,
    OUT MTD_U16 *fecLaneAlignment,
    OUT MTD_U16 *latchedRsFecHighErr,
    OUT MTD_U16 *currRsFecHighErr
)
{
    MTD_U16 data16Bits;

    *pcsLaneAlignment = *fecLaneAlignment = *latchedRsFecHighErr = *currRsFecHighErr = 0;

    if (!devPtr)
    {
        MTD_DBG_ERROR("mtdGetRsFecStatus: Invalid device pointer. \n");
        return MTD_FAIL;
    }

    if (!MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        MTD_DBG_ERROR("mtdGetRsFecStatus: RS-FEC is not supported in this device. \n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_RS_FEC_STATUS, &data16Bits));
    mtdHwGetRegFieldFromWord(data16Bits, 15, 1, pcsLaneAlignment);
    mtdHwGetRegFieldFromWord(data16Bits, 14, 1, fecLaneAlignment);
    mtdHwGetRegFieldFromWord(data16Bits, 2, 1, latchedRsFecHighErr);

    MTD_ATTEMPT(mtdHwGetPhyRegField(devPtr, port, MTD_RS_FEC_STATUS, 2, 1, currRsFecHighErr));

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdRsFECReset
    Reset the RS-FEC rx block, including RS-FEC counters.
*******************************************************************************/
MTD_STATUS mtdRsFECReset
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port
)
{
    if (!devPtr)
    {
        MTD_DBG_ERROR("mtdRsFECReset: Invalid device pointer. \n");
        return MTD_FAIL;
    }

    if (!MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        MTD_DBG_ERROR("mtdRsFECReset: RS-FEC is not supported in this device. \n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_RS_FEC_CONTROL1, 6, 2, 0x3));
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_RS_FEC_CONTROL1, 6, 2, 0x2));
    MTD_ATTEMPT(mtdHwSetPhyRegField(devPtr, port, MTD_RS_FEC_CONTROL1, 6, 2, 0x0));

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdGetRsFecCorrectedCwCntr
    Read the RS-FEC Corrected Code Word Counter
*******************************************************************************/
MTD_STATUS mtdGetRsFecCorrectedCwCntr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U32 *codeWordCounter
)
{
    MTD_U16 correctedCodeWordLower, correctedCodeWordUpper;

    if (!devPtr)
    {
        MTD_DBG_ERROR("mtdGetRsFecCorrectedCwCntr: Invalid device pointer. \n");
        return MTD_FAIL;
    }

    if (!MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        MTD_DBG_ERROR("mtdGetRsFecCorrectedCwCntr: RS-FEC is not supported in this device. \n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_RS_FEC_CORR_CW_LOW, &correctedCodeWordLower));
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_RS_FEC_CORR_CW_UPPER, &correctedCodeWordUpper));

    *codeWordCounter = correctedCodeWordLower;
    *codeWordCounter |= (((MTD_U32)(correctedCodeWordUpper))<<16);

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdGetRsFecUnCorrectedCwCntr
    Read the RS-FEC Uncorrected Code Word Counter
*******************************************************************************/
MTD_STATUS mtdGetRsFecUnCorrectedCwCntr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U32 *codeWordCounter
)
{
    MTD_U16 uncorrectedCodeWordLower, uncorrectedCodeWordUpper;

    *codeWordCounter = 0;

    if (!devPtr)
    {
        MTD_DBG_ERROR("mtdGetRsFecUnCorrectedCwCntr: Invalid device pointer. \n");
        return MTD_FAIL;
    }

    if (!MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        MTD_DBG_ERROR("mtdGetRsFecUnCorrectedCwCntr: RS-FEC is not supported in this device. \n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_RS_FEC_UNCORR_CW_LOW, &uncorrectedCodeWordLower));
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_RS_FEC_UNCORR_CW_UPPER, &uncorrectedCodeWordUpper));

    *codeWordCounter = uncorrectedCodeWordLower;
    *codeWordCounter |= (((MTD_U32)(uncorrectedCodeWordUpper))<<16);

    return MTD_OK;
}

/*******************************************************************************
 MTD_STATUS mtdGetRsFecSymbolErrorCntr
    Read the RS-FEC Symbol Error Counter
*******************************************************************************/
MTD_STATUS mtdGetRsFecSymbolErrorCntr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 port,
    OUT MTD_U32 *errorCounter
)
{
    MTD_U16 symbolErrorCtrLower, symbolErrorCtrUpper;

    *errorCounter = 0;

    if (!devPtr)
    {
        MTD_DBG_ERROR("mtdGetRsFecSymbolErrorCntr: Invalid device pointer. \n");
        return MTD_FAIL;
    }

    if (!MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        MTD_DBG_ERROR("mtdGetRsFecSymbolErrorCntr: RS-FEC is not supported in this device. \n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_RS_FEC_SYM_ERR_CTR_LOW, &symbolErrorCtrLower));
    MTD_ATTEMPT(mtdHwXmdioRead(devPtr, port, MTD_RS_FEC_SYM_ERR_CTR_UPPER, &symbolErrorCtrUpper));

    *errorCounter = symbolErrorCtrLower;
    *errorCounter |= (((MTD_U32)(symbolErrorCtrUpper))<<16);

    return MTD_OK;
}
