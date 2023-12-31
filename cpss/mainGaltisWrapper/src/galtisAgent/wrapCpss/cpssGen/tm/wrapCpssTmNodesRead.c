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
* @file wrapCpssTmNodesRead.c
*
* @brief Wrapper functions for TM Shaping APIs.
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/generic/tm/cpssTmNodesRead.h>
#include <cpss/generic/tm/cpssTmPublicDefs.h>


/**
* @internal wrCpssTmQueueConfigurationRead function
* @endinternal
*
* @brief   Read queue software configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTmQueueConfigurationRead
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_TM_QUEUE_PARAMS_STC            params;
    GT_U32                              index;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* initialize profileStrPtr */
    cmdOsMemSet(&params , 0, sizeof(params));


    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    index = (CPSS_TM_LEVEL_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssTmQueueConfigurationRead(devNum, index, &params);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }


    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d",
                    params.shapingProfilePtr,params.quantum,params.dropProfileInd,
                    params.eligiblePrioFuncId);

    return CMD_OK;
}


/**
* @internal wrCpssTmAnodeConfigurationRead function
* @endinternal
*
* @brief   Read A-node software configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTmAnodeConfigurationRead
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8                        devNum;
    GT_U32                       index;
    CPSS_TM_A_NODE_PARAMS_STC    params;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* init with zeros */
    cmdOsMemSet(&params, 0, sizeof(params));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssTmAnodeConfigurationRead(devNum, index, &params);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d%d%d%d%d%d%d%d", params.shapingProfilePtr, params.quantum,params.schdModeArr[0],
                 params.schdModeArr[1],params.schdModeArr[2],params.schdModeArr[3],params.schdModeArr[4],
                 params.schdModeArr[5],params.schdModeArr[6],params.schdModeArr[7],
                 params.dropProfileInd,params.eligiblePrioFuncId, params.numOfChildren);

    return CMD_OK;

}


/**
* @internal wrCpssTmBnodeConfigurationRead function
* @endinternal
*
* @brief   Read B-node software configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTmBnodeConfigurationRead
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8                        devNum;
    GT_U32                       index;
    CPSS_TM_B_NODE_PARAMS_STC    params;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* init with zeros */
    cmdOsMemSet(&params, 0, sizeof(params));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssTmBnodeConfigurationRead(devNum, index, &params);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d%d%d%d%d%d%d%d", params.shapingProfilePtr, params.quantum,params.schdModeArr[0],
                 params.schdModeArr[1],params.schdModeArr[2],params.schdModeArr[3],params.schdModeArr[4],
                 params.schdModeArr[5],params.schdModeArr[6],params.schdModeArr[7],
                 params.dropProfileInd,params.eligiblePrioFuncId, params.numOfChildren);

    return CMD_OK;
}


/**
* @internal wrCpssTmCnodeConfigurationRead function
* @endinternal
*
* @brief   Read C-node software configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTmCnodeConfigurationRead
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8                        devNum;
    GT_U32                       index;
    CPSS_TM_C_NODE_PARAMS_STC    params;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* init with zeros */
    cmdOsMemSet(&params, 0, sizeof(params));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssTmCnodeConfigurationRead(devNum, index, &params);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", params.shapingProfilePtr, params.quantum,params.schdModeArr[0],
                 params.schdModeArr[1],params.schdModeArr[2],params.schdModeArr[3],params.schdModeArr[4],
                 params.schdModeArr[5],params.schdModeArr[6],params.schdModeArr[7],
                 params.dropCosMap,params.dropProfileIndArr[0],params.dropProfileIndArr[1],params.dropProfileIndArr[2],
                 params.dropProfileIndArr[3],params.dropProfileIndArr[4],params.dropProfileIndArr[5],
                 params.dropProfileIndArr[6],params.dropProfileIndArr[7],params.eligiblePrioFuncId,params.numOfChildren);

    return CMD_OK;
}


/**
* @internal wrCpssTmPortConfigurationRead function
* @endinternal
*
* @brief   Read port software configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTmPortConfigurationRead
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8                         devNum;
    GT_U32                        index;
    CPSS_TM_PORT_PARAMS_STC       params;
    CPSS_TM_PORT_DROP_PER_COS_STC cos;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* init with zeros */
    cmdOsMemSet(&params, 0, sizeof(params));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssTmPortConfigurationRead(devNum, index, &params, &cos);

    /*test2*/

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", params.cirBw, params.eirBw,
                 params.cbs, params.ebs, params.quantumArr[0], params.quantumArr[1], params.quantumArr[2],
                 params.quantumArr[3], params.quantumArr[4], params.quantumArr[5],params.quantumArr[6],
                 params.quantumArr[7], params.schdModeArr[0], params.schdModeArr[1], params.schdModeArr[2],
                 params.schdModeArr[3], params.schdModeArr[4], params.schdModeArr[5],
                 params.schdModeArr[6], params.schdModeArr[7], params.dropProfileInd,
                 params.eligiblePrioFuncId, params.numOfChildren,
                 cos.dropCosMap, cos.dropProfileIndArr[0], cos.dropProfileIndArr[1], cos.dropProfileIndArr[2],
                 cos.dropProfileIndArr[3], cos.dropProfileIndArr[4], cos.dropProfileIndArr[5],
                 cos.dropProfileIndArr[6], cos.dropProfileIndArr[7]);

    return CMD_OK;
}


/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssTmAnodeConfigurationRead",
        &wrCpssTmAnodeConfigurationRead,
        2, 0},
    {"cpssTmBnodeConfigurationRead",
        &wrCpssTmBnodeConfigurationRead,
        2, 0},
    {"cpssTmCnodeConfigurationRead",
        &wrCpssTmCnodeConfigurationRead,
        2, 0},
    {"cpssTmPortConfigurationRead",
        &wrCpssTmPortConfigurationRead,
        2, 0},
    {"cpssTmQueueConfigurationRead",
        &wrCpssTmQueueConfigurationRead,
        2, 0}

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssTmNodesRead function
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
GT_STATUS cmdLibInitCpssTmNodesRead
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

