/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file wrapCpssTmNodesUpdate.c
*
* @brief Wrapper functions for TM configuration library control interface.
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/generic/tm/cpssTmCtl.h>
#include <cpss/generic/tm/cpssTmNodesUpdate.h>

/**
* @internal wrCpssTmQueueUpdate function
* @endinternal
*
* @brief   Update queue parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. When error occurs, the entry is considered inconsistent.
*       2. 'dropProfileInd' parameter will be updated in any case,
*       set it's value to be the same as in DB if you don't want to change it.
*
*/
static CMD_STATUS wrCpssTmQueueUpdate
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32   index;
    CPSS_TM_QUEUE_PARAMS_STC     params;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];
    params.shapingProfilePtr = (GT_U32)inArgs[2];
    params.quantum = (GT_U32)inArgs[3];
    params.dropProfileInd = (GT_U32)inArgs[4];
    params.eligiblePrioFuncId = (CPSS_TM_ELIG_FUNC_NODE_ENT)inArgs[5];

    /* call cpss api function */
    result = cpssTmQueueUpdate(devNum, index, &params);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssTmAnodeUpdate function
* @endinternal
*
* @brief   Update A-node parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. When error occurs, the entry is considered inconsistent.
*       2. 'numOfChildren' can't be updated.
*       3. 'dropProfileInd' parameter will be updated in any case,
*       set it's value to be the same as in DB if you don't want to change it.
*
*/
static CMD_STATUS wrCpssTmAnodeUpdate
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32   index;
    CPSS_TM_A_NODE_PARAMS_STC     params;
    GT_U32      ii;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];
    params.shapingProfilePtr = (GT_U32)inArgs[2];
    params.quantum = (GT_U32)inArgs[3];
    for(ii=0; ii<8; ii++)
        params.schdModeArr[ii] = (GT_U32)inArgs[4+ii];
    params.dropProfileInd = (GT_U32)inArgs[12];
    params.eligiblePrioFuncId = (CPSS_TM_ELIG_FUNC_NODE_ENT)inArgs[13];
    params.numOfChildren = (GT_U32)inArgs[14];

    /* call cpss api function */
    result = cpssTmAnodeUpdate(devNum, index, &params);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssTmBnodeUpdate function
* @endinternal
*
* @brief   Update B-node parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. When error occurs, the entry is considered inconsistent.
*       2. 'numOfChildren' can't be updated.
*
*/
static CMD_STATUS wrCpssTmBnodeUpdate
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32   index;
    CPSS_TM_B_NODE_PARAMS_STC     params;
    GT_U32      ii;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];
    params.shapingProfilePtr = (GT_U32)inArgs[2];
    params.quantum = (GT_U32)inArgs[3];
    for(ii=0; ii<8; ii++)
        params.schdModeArr[ii] = (GT_U32)inArgs[4+ii];
    params.dropProfileInd = (GT_U32)inArgs[12];
    params.eligiblePrioFuncId = (CPSS_TM_ELIG_FUNC_NODE_ENT)inArgs[13];
    params.numOfChildren = (GT_U32)inArgs[14];

    /* call cpss api function */
    result = cpssTmBnodeUpdate(devNum, index, &params);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssTmCnodeUpdate function
* @endinternal
*
* @brief   Update C-node parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. When error occurs, the entry is considered inconsistent.
*       2. 'numOfChildren' can't be updated.
*
*/
static CMD_STATUS wrCpssTmCnodeUpdate
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32   index;
    CPSS_TM_C_NODE_PARAMS_STC     params;
    GT_U32      ii;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];
    params.shapingProfilePtr = (GT_U32)inArgs[2];
    params.quantum = (GT_U32)inArgs[3];
    for(ii=0; ii<8; ii++)
        params.schdModeArr[ii] = (GT_U32)inArgs[4+ii];
    params.dropCosMap = (GT_U32)inArgs[12];
    for(ii=0; ii<8; ii++)
        params.dropProfileIndArr[ii] = (GT_U32)inArgs[13+ii];
    params.eligiblePrioFuncId = (CPSS_TM_ELIG_FUNC_NODE_ENT)inArgs[21];
    params.numOfChildren = (GT_U32)inArgs[22];

    /* call cpss api function */
    result = cpssTmCnodeUpdate(devNum, index, &params);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**
* @internal wrCpssTmPortShapingUpdate function
* @endinternal
*
* @brief   Update Port Shaping parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_BAD_SIZE              - on Port's min/max token too large.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note When error occurs, the entry is considered inconsistent.
*
*/
static CMD_STATUS wrCpssTmPortShapingUpdate
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8    devNum;
    GT_U32   index;
    GT_U32   cirBw;
    GT_U32   eirBw;
    GT_U32   cbs;
    GT_U32   ebs;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];
    cirBw = (GT_U32)inArgs[2];
    eirBw = (GT_U32)inArgs[3];
    cbs = (GT_U32)inArgs[4];
    ebs = (GT_U32)inArgs[5];

    /* call cpss api function */
    result = cpssTmPortShapingUpdate(devNum, index, cirBw, eirBw, cbs, ebs);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssTmPortSchedulingUpdate function
* @endinternal
*
* @brief   Update Port Scheduling parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note When error occurs, the entry is considered inconsistent.
*
*/
static CMD_STATUS wrCpssTmPortSchedulingUpdate
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    index;
    GT_U32    eligPrioFuncId;
    GT_U32    quantumArrPtr[8]; /* 8 cells array */
    CPSS_TM_SCHD_MODE_ENT   schdModeArr[8];/* 8 cells array */
    GT_U32    ii;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];

    for(ii=0; ii<8; ii++)
        quantumArrPtr[ii] = (GT_U16)inArgs[2+ii];
    for(ii=0; ii<8; ii++)
        schdModeArr[ii] = (GT_BOOL)inArgs[10+ii];

    eligPrioFuncId = (CPSS_TM_ELIG_FUNC_NODE_ENT)inArgs[18];


    /* call cpss api function */
    result = cpssTmPortSchedulingUpdate(devNum, index, eligPrioFuncId, quantumArrPtr, schdModeArr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssTmPortDropUpdate function
* @endinternal
*
* @brief   Update Port Drop parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note When error occurs, the entry is considered inconsistent.
*
*/
static CMD_STATUS wrCpssTmPortDropUpdate
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    index;
    GT_U32    wredProfileRef;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];
    wredProfileRef = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssTmPortDropUpdate(devNum, index, wredProfileRef);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssTmPortDropCosUpdate function
* @endinternal
*
* @brief   Update Port Drop per Cos parameters.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_OUT_OF_RANGE          - on parameter value out of range.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note When error occurs, the entry is considered inconsistent.
*
*/
static CMD_STATUS wrCpssTmPortDropCosUpdate
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    index;
    CPSS_TM_PORT_DROP_PER_COS_STC  params;
    GT_U32    ii;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];
    params.dropCosMap = (GT_U32)inArgs[2];
    for(ii=0; ii<8; ii++)
        params.dropProfileIndArr[ii] = (GT_32)inArgs[3+ii];

    /* call cpss api function */
    result = cpssTmPortDropCosUpdate(devNum, index, &params);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}
/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssTmQueueUpdate",
        &wrCpssTmQueueUpdate,
        6, 0},
    {"cpssTmAnodeUpdate",
        &wrCpssTmAnodeUpdate,
        15, 0},
    {"cpssTmBnodeUpdate",
        &wrCpssTmBnodeUpdate,
        15, 0},
    {"cpssTmCnodeUpdate",
        &wrCpssTmCnodeUpdate,
        23, 0},
    {"cpssTmPortShapingUpdate",
        &wrCpssTmPortShapingUpdate,
        6, 0},
    {"cpssTmPortSchedulingUpdate",
        &wrCpssTmPortSchedulingUpdate,
        19, 0},
    {"cpssTmPortDropUpdate",
        &wrCpssTmPortDropUpdate,
        3, 0},
    {"cpssTmPortDropCosUpdate",
        &wrCpssTmPortDropCosUpdate,
        11, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssTmNodesUpdate function
* @endinternal
*
* @brief   Library database initialization function.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
*
* @note none
*
*/
GT_STATUS cmdLibInitCpssTmNodesUpdate
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

