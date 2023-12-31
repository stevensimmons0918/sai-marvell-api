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
* @file wrapCpssTmSched.c
*
* @brief Wrapper functions for TM Scheduler APIs.
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/generic/tm/cpssTmSched.h>
#include <cpss/generic/tm/cpssTmPublicDefs.h>


/**
* @internal wrCpssTmSchedPortExternalBpSet function
* @endinternal
*
* @brief   Enable/Disable Port External Backpressure.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx, Aldrin, AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note to enable Backpressure response from TM Ports to physical ports,
*       egressEnable at cpssDxChTmGlueFlowControlEnableSet must also be enabled.
*
*/
static CMD_STATUS wrCpssTmSchedPortExternalBpSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_BOOL   portExtBp;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    portExtBp = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssTmSchedPortExternalBpSet(devNum, portExtBp);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/* next relate to scheduling periodic scheme */
#define MAX_LEVELS_CNS                      (CPSS_TM_LEVEL_P_E+1)
static GT_U32                               level = 0;
static CPSS_TM_LEVEL_PERIODIC_PARAMS_STC    periodicSchemeArray[MAX_LEVELS_CNS];
static GT_STATUS   res;

/**
* @internal wrCpssTmSchedPeriodicSchemeConfig function
* @endinternal
*
* @brief   Configure Periodic Scheme.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_BAD_VALUE             - on not found parameter or parameter's value can't be obtained.
* @retval GT_BAD_SIZE              - on maxBw token size can not be presented even when using resolution.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. The API may be invoked once in a system lifetime.
*       2. To indicate the shaping is disabled for a level the
*       periodicState field must be set to GT_FALSE.
*       3 paramsPtr is handled as 5 elements array including also a
*       port level scheduling configuration.
*
*/
static CMD_STATUS wrCpssTmSchedPeriodicSchemeConfig
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    CPSS_TM_LEVEL_ENT inLevel;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    if (level >= MAX_LEVELS_CNS)
    {
        galtisOutput(outArgs, GT_FULL, "%d",-1);/* Error ! */
        return CMD_OK;
    }

    /* map input arguments to locals */
    inLevel = (CPSS_TM_LEVEL_ENT)inFields[0];
/*
    periodicSchemeArray[inLevel].minBw = (GT_U32)inFields[1];
    periodicSchemeArray[inLevel].maxBw = (GT_U32)inFields[2];
    periodicSchemeArray[inLevel].increamentalBw = (GT_U32)inFields[3];
    periodicSchemeArray[inLevel].bwAccuracy = (float)inFields[4];
    leftDecimalShiftValue = (GT_U32)inFields[5];

    for (ii=0; ii<leftDecimalShiftValue; ii++) {
        periodicSchemeArray[inLevel].bwAccuracy= (float)(periodicSchemeArray[inLevel].bwAccuracy/10);
    }
*/
    periodicSchemeArray[inLevel].periodicState = (GT_BOOL)inFields[6];
    periodicSchemeArray[inLevel].shaperDecoupling = (GT_BOOL)inFields[7];


    level++;

    if(level == MAX_LEVELS_CNS)/* end registration*/
    {
        /* map input arguments to locals */
        devNum = (GT_U8)inArgs[0];


        /* call cpss api function */
        result = cpssTmSchedPeriodicSchemeConfig(devNum, periodicSchemeArray);

        /* pack output arguments to galtis string */

        res= result;

        galtisOutput(outArgs, GT_OK, "");
        return CMD_OK;
    }


    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}
static CMD_STATUS wrCpssTmSchedPeriodicSchemeConfigFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    level=0;
    return wrCpssTmSchedPeriodicSchemeConfig(inArgs,inFields,numFields,outArgs);
}

static CMD_STATUS wrCpssTmSchedPeriodicSchemeConfigEnd
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inArgs);

     /* pack output arguments to galtis string */
    if (res!= GT_OK)
        galtisOutput(outArgs, GT_BAD_STATE, "");
    else
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/**
* @internal wrCpssTmShapingPeriodicUpdateEnableSet function
* @endinternal
*
* @brief   Enable/Disable shaping for a given level: Q, A, B, C or Port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note If periodic update will be disabled for some level, traffic through nodes with shaping-enabled eligible functions will be stopped !
*
*/
static CMD_STATUS wrCpssTmShapingPeriodicUpdateEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    CPSS_TM_LEVEL_ENT  level;
    GT_BOOL            status;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    level = (CPSS_TM_LEVEL_ENT)inArgs[1];
    status = (GT_BOOL)inArgs[2];

    /* call cpss api function */
    result = cpssTmShapingPeriodicUpdateEnableSet(devNum, level, status);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssTmShapingPeriodicUpdateStatusGet function
* @endinternal
*
* @brief   Configure Periodic Scheme.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1 paramsPtr is handled as 5 elements array including also a
*       port level scheduling configuration.
*
*/
static CMD_STATUS wrCpssTmShapingPeriodicUpdateStatusGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;


    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];


    /* call cpss api function */
    result = cpssTmShapingPeriodicUpdateStatusGet(devNum, periodicSchemeArray);

	if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }


    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d",
                    periodicSchemeArray[0].periodicState,
                    periodicSchemeArray[1].periodicState,
                    periodicSchemeArray[2].periodicState,
                    periodicSchemeArray[3].periodicState,
                    periodicSchemeArray[4].periodicState);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");

    return CMD_OK;
}

/**
* @internal wrCpssTmSchedPortLvlDwrrBytesPerBurstLimitSet function
* @endinternal
*
* @brief   Set the number of DWRR bytes per burst limit for all ports.
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
static CMD_STATUS wrCpssTmSchedPortLvlDwrrBytesPerBurstLimitSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32    bytes;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    bytes = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssTmSchedPortLvlDwrrBytesPerBurstLimitSet(devNum, bytes);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssTmPortQuantumLimitsGet function
* @endinternal
*
* @brief   get maximum and minimum possible quantum values and quantum value granularity for port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTmPortQuantumLimitsGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    CPSS_TM_QUANTUM_LIMITS_STC  cpssQuantumLimits;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssTmPortQuantumLimitsGet(devNum, &cpssQuantumLimits);

     if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%",
                    cpssQuantumLimits.minQuantum,
                    cpssQuantumLimits.maxQuantum,
                    cpssQuantumLimits.resolution);

    return CMD_OK;
}

/**
* @internal wrCpssTmNodeQuantumLimitsGet function
* @endinternal
*
* @brief   get maximum and minimum possible quantum values and quantum value granularity for nodes.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTmNodeQuantumLimitsGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    CPSS_TM_QUANTUM_LIMITS_STC  cpssQuantumLimits;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssTmNodeQuantumLimitsGet(devNum, &cpssQuantumLimits);

     if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%",
                    cpssQuantumLimits.minQuantum,
                    cpssQuantumLimits.maxQuantum,
                    cpssQuantumLimits.resolution);

    return CMD_OK;
}

/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssTmSchedPortExternalBpSet",
        &wrCpssTmSchedPortExternalBpSet,
        2, 0},

    /* start table : cpssTmSchedPeriodicSchemeConfig */
     {"cpssTmSchedPeriodicSchemeConfigSet",
        &wrCpssTmSchedPeriodicSchemeConfigFirst,
        1, 8},
    {"cpssTmSchedPeriodicSchemeConfigSetFirst",
        &wrCpssTmSchedPeriodicSchemeConfigFirst,
        1, 8},
    {"cpssTmSchedPeriodicSchemeConfigSetNext",
        &wrCpssTmSchedPeriodicSchemeConfig,
        1, 8},

    {"cpssTmSchedPeriodicSchemeConfigEndSet",
        &wrCpssTmSchedPeriodicSchemeConfigEnd,
        0, 0},

     {"cpssTmShapingPeriodicUpdateEnableSet",
        &wrCpssTmShapingPeriodicUpdateEnableSet,
        3, 0},

     {"cpssTmShapingPeriodicUpdateStatusGet",
        &wrCpssTmShapingPeriodicUpdateStatusGet,
        1, 0},

    {"cpssTmSchedPortLvlDwrrBytesPerBurstLimitSet",
        &wrCpssTmSchedPortLvlDwrrBytesPerBurstLimitSet,
        2, 0},

    {"cpssTmPortQuantumLimitsGet",
        &wrCpssTmPortQuantumLimitsGet,
        1, 0},

    {"cpssTmNodeQuantumLimitsGet",
        &wrCpssTmNodeQuantumLimitsGet,
        1, 0},
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssTmSched function
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
GT_STATUS cmdLibInitCpssTmSched
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

