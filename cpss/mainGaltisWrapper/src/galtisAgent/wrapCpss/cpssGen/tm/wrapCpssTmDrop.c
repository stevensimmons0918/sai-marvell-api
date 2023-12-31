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
* @file wrapCpssTmDrop.c
*
* @brief Wrapper functions for TM Drop APIs.
*
* @version   2
********************************************************************************
*/

/* Common galtis includes */

#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/common/cmdWrapUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>



/* Feature specific includes. */
#include <cpss/generic/tm/cpssTmPublicDefs.h>
#include <cpss/generic/tm/cpssTmDrop.h>


/**
* @internal wrCpssTmDropWredCurveCreate function
* @endinternal
*
* @brief   Create a WRED curve for a TM level.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_FULL                  - on full allocation of WRED Curves table.
* @retval GT_NOT_INITIALIZED       - on not configured AQM Mode params for this lvl.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. The CoS parameter is relevant for C and P level only, else set CPSS_TM_INVAL_CNS.
*       2. For P level in Global mode set 'cos' = CPSS_TM_INVAL_CNS, else curve will be created for CoS mode.
*
*/
static CMD_STATUS wrCpssTmDropWredCurveCreate
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    CPSS_TM_LEVEL_ENT   level;
    GT_U32              cos;
    GT_U32              probabilityArr[CPSS_TM_DROP_PROB_ARR_SIZE_CNS];
    GT_U32              curveInd;
    GT_U32              ii;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    level = (CPSS_TM_LEVEL_ENT)inArgs[1];
    cos = (GT_U32)inArgs[2];

    for(ii=0; ii<CPSS_TM_DROP_PROB_ARR_SIZE_CNS; ii++) {
        probabilityArr[ii] = (GT_32)inArgs[3+ii];
    }

    /* call cpss api function */
    result = cpssTmDropWredCurveCreate(devNum, level, cos, probabilityArr, &curveInd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", curveInd);

    return CMD_OK;
}


/**
* @internal wrCpssTmDropWredTraditionalCurveCreate function
* @endinternal
*
* @brief   Create a WRED traditional curve for a level.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_FULL                  - on full allocation of WRED Curves table.
* @retval GT_NOT_INITIALIZED       - on not configured AQM Mode params for this lvl.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. The CoS parameter is relevant for C and P level only, else set CPSS_TM_INVAL_CNS.
*       2. For P level in Global mode set 'cos' = CPSS_TM_INVAL_CNS, else curve will be created for CoS mode.
*
*/
static CMD_STATUS wrCpssTmDropWredTraditionalCurveCreate
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    CPSS_TM_LEVEL_ENT   level;
    GT_U32              cos;
    GT_U32              maxProbability;
    GT_U32              curveInd;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    level = (CPSS_TM_LEVEL_ENT)inArgs[1];
    cos = (GT_U32)inArgs[2];
    maxProbability = (GT_U16)inArgs[3];

    /* call cpss api function */
    result = cpssTmDropWredTraditionalCurveCreate(devNum, level, cos, maxProbability, &curveInd);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", curveInd);

    return CMD_OK;
}


/**
* @internal wrCpssTmDropProfileCreate function
* @endinternal
*
* @brief   Create Drop profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_FULL                  - on full allocation of level's Drop profile table.
* @retval GT_NOT_INITIALIZED       - on not configured AQM Mode params for this lvl.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. Cos of Drop Profile matches Cos of given curve.
*       2. The CoS parameter is relevant for C and P level only, else set CPSS_TM_INVAL_CNS.
*       3. For P level in Global mode set 'cos' = CPSS_TM_INVAL_CNS, else profile will be created for CoS mode.
*
*/
static CMD_STATUS wrCpssTmDropProfileCreate
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    CPSS_TM_LEVEL_ENT   level;
    GT_U32              cos;
    CPSS_TM_DROP_PROFILE_PARAMS_STC         profileStrPtr;
    GT_U32              profile;
    CPSS_DP_LEVEL_ENT   ii;
    GT_U32              bytes;


    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* init with zeros */
    cmdOsMemSet(&profileStrPtr, 0, sizeof(profileStrPtr));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    level = (CPSS_TM_LEVEL_ENT)inArgs[1];
    cos = (GT_U32)inArgs[2];
/*
    profileStrPtr.wredCaTdBw = (GT_U32)inArgs[3];
    profileStrPtr.cbTdBw = (GT_U32)inArgs[4];
    profileStrPtr.cbTdThresholdRatio = (GT_U32)inArgs[5];
    profileStrPtr.aqlExponent = (GT_U32)inArgs[6];
    profileStrPtr.wredCaTdMode = (CPSS_TM_DROP_MODE_ENT)inArgs[7];

    for(ii=0; ii<3; ii++) {
        profileStrPtr.curvePtr[ii] = (GT_32)inArgs[8+ii];
    }
    for(ii=0; ii<3; ii++) {
        profileStrPtr.dpRatio[ii] = (GT_32)inArgs[11+ii];
    }
    for(ii=0; ii<3; ii++) {
        profileStrPtr.wredMinThresholdRatio[ii] = (GT_32)inArgs[14+ii];
    }
    for(ii=0; ii<3; ii++) {
        profileStrPtr.wredMaxThresholdRatio[ii] = (GT_32)inArgs[17+ii];
    }
*/

	profileStrPtr.dropMode = (CPSS_TM_DROP_MODE_ENT)inArgs[7];
	/* cb threshold in bytes = bytes (cbTdBw) *  cbTdThresholdRatio(percents)/100% */
	/* be aware about overflow */
	bytes = (GT_U32)inArgs[4];
	if (bytes > 10000000 )	bytes = (bytes /100) * (GT_U32)inArgs[5];
	else					bytes = (bytes * (GT_U32)inArgs[5]) /100;

	profileStrPtr.cbTdThresholdBytes = cpssTmDropConvertBWToBytes(bytes);
	switch(profileStrPtr.dropMode)
	{
		case CPSS_TM_DROP_MODE_CB_TD_CA_WRED_E:
			profileStrPtr.caWredDp.aqlExponent  = (GT_U8)inArgs[6];
			for(ii = CPSS_DP_GREEN_E; ii <= CPSS_DP_RED_E; ii++)
			{
				profileStrPtr.caWredDp.curveIndex[ii] = (GT_U8)inArgs[8+ii];
				profileStrPtr.caWredDp.dpCurveScale[ii] = (GT_U8)inArgs[11+ii];
				/* wred threshold in bytes = bytes (wredCaTdBw) *  min/max_threshold(percents)/100% */

				bytes = (GT_U32)inArgs[3];
				if (bytes > 10000000 )	bytes = (bytes /100) * (GT_U32)inArgs[14+ii];
				else					bytes = (bytes * (GT_U32)inArgs[14+ii]) /100;
				profileStrPtr.caWredDp.caWredTdMinThreshold[ii] = cpssTmDropConvertBWToBytes(bytes);

				bytes = (GT_U32)inArgs[3];
				if (bytes > 10000000 )	bytes = (bytes /100) * (GT_U32)inArgs[17+ii];
				else					bytes = (bytes * (GT_U32)inArgs[17+ii]) /100;
				profileStrPtr.caWredDp.caWredTdMaxThreshold[ii] = cpssTmDropConvertBWToBytes(bytes);
			}
			break;
		case CPSS_TM_DROP_MODE_CB_TD_CA_TD_E:
			for (ii = CPSS_DP_GREEN_E; ii <= CPSS_DP_RED_E ; ii++)
			{
				/* caTd threshold in bytes = bytes (wredCaTdBw) *  min_threshold(percents)/100% */
				bytes = (GT_U32)inArgs[3];
				if (bytes > 10000000 )	bytes = (bytes /100) * (GT_U32)inArgs[14+ii];
				else					bytes = (bytes * (GT_U32)inArgs[14+ii]) /100;
				profileStrPtr.caTdDp.caTdThreshold[ii] = cpssTmDropConvertBWToBytes(bytes) ;
			}
			break;
		case CPSS_TM_DROP_MODE_CB_TD_ONLY_E:
		default:
			break;
	}

    /* call cpss api function */
    result = cpssTmDropProfileCreate(devNum, level, cos, &profileStrPtr, &profile);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", profile);

    return CMD_OK;
}

/**
* @internal wrCpssTmDropProfileDelete function
* @endinternal
*
* @brief   Delete Drop profile.
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
* @note 1. The CoS parameter is relevant for C and P level only, else set CPSS_TM_INVAL_CNS.
*       2. For P level in Global mode set 'cos' = CPSS_TM_INVAL_CNS, else profile will be deleted for CoS mode.
*
*/
static CMD_STATUS wrCpssTmDropProfileDelete
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    CPSS_TM_LEVEL_ENT   level;
    GT_U32              cos;
    GT_U32              profile;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    level = (CPSS_TM_LEVEL_ENT)inArgs[1];
    cos = (GT_U32)inArgs[2];
    profile = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssTmDropProfileDelete(devNum, level, cos, profile);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}


/**
* @internal wrCpssTmDropProfileRead function
* @endinternal
*
* @brief   Read Drop profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_INITIALIZED       - on not configured AQM Mode params for this lvl.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. The cbTdThresholdRatio is calculated aproximately from the register's values.
*       2. The CoS parameter is relevant for C and P level only, else set CPSS_TM_INVAL_CNS.
*       3. For P level in Global mode set 'cos' = CPSS_TM_INVAL_CNS, else profile will be read for CoS mode.
*
*/
static CMD_STATUS wrCpssTmDropProfileRead
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_TM_LEVEL_ENT                   level;
    GT_U32                              cos;
    GT_U32                              profileInd;
    CPSS_TM_DROP_PROFILE_PARAMS_STC     profileStrPtr;

    GT_U32 wredCaTdBw = 0;
    GT_U32 cbTdBw = 0;
    GT_U32 cbTdThresholdRatio = 100;
    GT_U32 aqlExponent = 0;
    GT_U32 wredCaTdMode = CPSS_TM_DROP_MODE_CB_TD_ONLY_E;
    GT_U32 curvePtr[3];
    GT_U32 dpRatio[3];
    GT_U32 wredMinThresholdRatio[3];
    GT_U32 wredMaxThresholdRatio[3];

	GT_U8 color_num;
	GT_U8 resolution;

	int i;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

	/* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* initialize profileStrPtr */
    cmdOsMemSet(&profileStrPtr , 0, sizeof(profileStrPtr));


    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    level = (CPSS_TM_LEVEL_ENT)inArgs[1];
    cos = (GT_U32)inArgs[2];
    profileInd = (GT_U32)inArgs[3];


    /* call cpss api function */
    result = cpssTmDropProfileRead(devNum, level, cos, profileInd, &profileStrPtr);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

	/* convert readed parameters to old API */
	result = cpssTmDropColorNumResolutionGet(devNum, level, &color_num, &resolution);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    color_num = 3;

	aqlExponent = 0;
	for (i = CPSS_DP_GREEN_E; i <= CPSS_DP_RED_E ; i++ )
	{
		curvePtr[i] = 0;
		dpRatio[i] = 0;
		wredMinThresholdRatio[i] = 0;
		wredMaxThresholdRatio[i] = 0;
	}

    wredCaTdMode = profileStrPtr.dropMode;
	cbTdBw = cpssTmDropConvertBytesToBW(profileStrPtr.cbTdThresholdBytes);
    cbTdThresholdRatio = 100;
	switch (wredCaTdMode)
	{
		case CPSS_TM_DROP_MODE_CB_TD_CA_WRED_E:
			aqlExponent = profileStrPtr.caWredDp.aqlExponent;
			wredCaTdBw = 0;
			for (i = 0; i < color_num ; i++)
			{
				if (wredCaTdBw < profileStrPtr.caWredDp.caWredTdMaxThreshold[i] ) wredCaTdBw = profileStrPtr.caWredDp.caWredTdMaxThreshold[i] ;
			}
			for(i = 0; i < color_num; i++)
			{
				curvePtr[i] = profileStrPtr.caWredDp.curveIndex[i];
				dpRatio[i] = profileStrPtr.caWredDp.dpCurveScale[i];
				if (wredCaTdBw) wredMinThresholdRatio[i] = 100 * profileStrPtr.caWredDp.caWredTdMinThreshold[i] / wredCaTdBw ;
				else			wredMinThresholdRatio[i] = 100;
				if (wredCaTdBw) wredMaxThresholdRatio[i] = 100 * profileStrPtr.caWredDp.caWredTdMaxThreshold[i] / wredCaTdBw ;
				else			wredMaxThresholdRatio[i] = 100;
			}
			wredCaTdBw = cpssTmDropConvertBytesToBW(wredCaTdBw);
			break;
		case CPSS_TM_DROP_MODE_CB_TD_CA_TD_E:
			 /* calculate max value of thresholds per color */
			wredCaTdBw = 0;
			for (i = 0; i < color_num ; i++)
			{
				if (wredCaTdBw < profileStrPtr.caTdDp.caTdThreshold[i] ) wredCaTdBw = profileStrPtr.caTdDp.caTdThreshold[i] ;
			}
			for (i = 0; i < color_num ; i++)
			{
				if (wredCaTdBw) wredMinThresholdRatio[i] = 100 *profileStrPtr.caTdDp.caTdThreshold[i]  / wredCaTdBw ;
				else			wredMinThresholdRatio[i] = 100;
			}
			wredCaTdBw = cpssTmDropConvertBytesToBW(wredCaTdBw);
			break;
		case CPSS_TM_DROP_MODE_CB_TD_ONLY_E:
		default:
			break;
	}

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
                 wredCaTdBw,
				 cbTdBw,
				 cbTdThresholdRatio,
                 aqlExponent,
				 wredCaTdMode,
				 curvePtr[CPSS_DP_GREEN_E],
                 curvePtr[CPSS_DP_YELLOW_E],
				 curvePtr[CPSS_DP_RED_E],
				 dpRatio[CPSS_DP_GREEN_E],
                 dpRatio[CPSS_DP_YELLOW_E],
				 dpRatio[CPSS_DP_RED_E],
                 wredMinThresholdRatio[CPSS_DP_GREEN_E],
				 wredMinThresholdRatio[CPSS_DP_YELLOW_E],
				 wredMinThresholdRatio[CPSS_DP_RED_E],
                 wredMaxThresholdRatio[CPSS_DP_GREEN_E],
				 wredMaxThresholdRatio[CPSS_DP_YELLOW_E],
                 wredMaxThresholdRatio[CPSS_DP_RED_E]);

    return CMD_OK;
}

/**
* @internal wrCpssTmDropColorNumSet function
* @endinternal
*
* @brief   Set Drop (Egress) Colors number per level.
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
* @note 1. This API should be called before all the rest Drop APIs (if need).
*       2. By default there are two colors per each level.
*
*/
static CMD_STATUS wrCpssTmDropColorNumSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_TM_LEVEL_ENT                   level;
    CPSS_TM_COLOR_NUM_ENT               number;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    level = (CPSS_TM_LEVEL_ENT)inArgs[1];
    number = (CPSS_TM_COLOR_NUM_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssTmDropColorNumSet(devNum, level, number);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssTmAgingChangeStatus function
* @endinternal
*
* @brief   Change the aging status.
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
static CMD_STATUS wrCpssTmAgingChangeStatus
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                              status;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    status = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssTmAgingChangeStatus(devNum, status);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssTmAgingStatusGet function
* @endinternal
*
* @brief   Retrieve the aging status.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTmAgingStatusGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                              status;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssTmAgingStatusGet(devNum, &status);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d",status);

    return CMD_OK;
}



/**
* @internal wrCpssTmDpSourceSet function
* @endinternal
*
* @brief   Change Drop Probability (DP) source.
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
static CMD_STATUS wrCpssTmDpSourceSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_TM_LEVEL_ENT                   level;
    CPSS_DP_LEVEL_ENT                   color;
    CPSS_TM_DP_SOURCE_ENT               source;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    level = (CPSS_TM_LEVEL_ENT)inArgs[1];
    color = (GT_U32)inArgs[2];
    source = (CPSS_TM_DP_SOURCE_ENT)inArgs[3];

    /* call cpss api function */
    result = cpssTmDpSourceSet(devNum, level, color, source);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssTmDpSourceGet function
* @endinternal
*
* @brief   Change Drop Probability (DP) source.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; xCat3; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Bobcat3; Aldrin2; Falcon; Puma2; Puma3; ExMx; Aldrin; AC3X.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTmDpSourceGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_TM_LEVEL_ENT                   level;
    CPSS_DP_LEVEL_ENT                   color;
    CPSS_TM_DP_SOURCE_ENT               source;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    level = (CPSS_TM_LEVEL_ENT)inArgs[1];
    color = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssTmDpSourceGet(devNum, level, color, &source);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "&d",source);

    return CMD_OK;
}



/**
* @internal wrCpssTmDropQueryResponceSet function
* @endinternal
*
* @brief   Drop Query Response Select.
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
static CMD_STATUS wrCpssTmDropQueryResponceSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                              portDp;
    CPSS_TM_LEVEL_ENT                   level;


    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    portDp = (GT_U32)inArgs[1];
    level = (CPSS_TM_LEVEL_ENT)inArgs[2];

    /* call cpss api function */
    result = cpssTmDropQueryResponceSet(devNum, portDp, level);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssTmDropQueryResponceGet function
* @endinternal
*
* @brief   Retrieve Drop Query Response state.
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
static CMD_STATUS wrCpssTmDropQueryResponceGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                              portDp;
    CPSS_TM_LEVEL_ENT                   level;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);


    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssTmDropQueryResponceGet(devNum, &portDp, &level);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d",portDp,level);

    return CMD_OK;
}

/**
* @internal wrCpssTmDropQueueCosSet function
* @endinternal
*
* @brief   Drop Queue Cos Select.
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
static CMD_STATUS wrCpssTmDropQueueCosSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                              index;
    GT_U32                              cos;


    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];
    cos = (GT_U32)inArgs[2];

    /* call cpss api function */
    result = cpssTmDropQueueCosSet(devNum, index, cos);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssTmDropQueueCosGet function
* @endinternal
*
* @brief   Drop Queue Cos Select.
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
static CMD_STATUS wrCpssTmDropQueueCosGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    GT_U32                              index;
    GT_U32                              cos;


    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    index = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssTmDropQueueCosGet(devNum, index, &cos);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d",cos);

    return CMD_OK;
}

/**
* @internal wrCpssTmDropProfileAgingBlockCreate function
* @endinternal
*
* @brief   Create Aging profile for CPSS_TM_LEVEL_Q_E.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_FULL                  - on full allocation of level's Drop profile table.
* @retval GT_NOT_INITIALIZED       - on not configured AQM Mode params for this lvl.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. In case of Color Blind TD only set wredCaTdBw=0 and wredCaTdMode=CBTD_ONLY.
*       2. In case of Color Blind TD disabled set cbTdBw=TM_MAX_BW, cbTdThresholdRatio=0.
*
*/
static CMD_STATUS wrCpssTmDropProfileAgingBlockCreate
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    CPSS_TM_DROP_PROFILE_PARAMS_STC         profileStrPtr[CPSS_TM_AGING_PROFILES_CNS];
    GT_U32              profile;
    GT_U32              j;
    CPSS_DP_LEVEL_ENT   ii;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* init with zeros */
    cmdOsMemSet(&profileStrPtr, 0, sizeof(profileStrPtr));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    for (j = 0; j < CPSS_TM_AGING_PROFILES_CNS; j++ ) {
/*
        profileStrPtr[j].wredCaTdBw = (GT_U32)inArgs[1+ (17*j)];
        profileStrPtr[j].cbTdBw = (GT_U32)inArgs[2+ (17*j)];
        profileStrPtr[j].cbTdThresholdRatio = (GT_U32)inArgs[3+ (17*j)];
        profileStrPtr[j].aqlExponent = (GT_U32)inArgs[4+ (17*j)];
        profileStrPtr[j].wredCaTdMode = (CPSS_TM_DROP_MODE_ENT)inArgs[5+ (17*j)];

        for(ii=0; ii<3; ii++) {
            profileStrPtr[j].curvePtr[ii] = (GT_32)inArgs[6+ii+ (17*j)];
        }
        for(ii=0; ii<3; ii++) {
            profileStrPtr[j].dpRatio[ii] = (GT_32)inArgs[9+ii+ (17*j)];
        }
        for(ii=0; ii<3; ii++) {
            profileStrPtr[j].wredMinThresholdRatio[ii] = (GT_32)inArgs[12+ii+ (17*j)];
        }
        for(ii=0; ii<3; ii++) {
            profileStrPtr[j].wredMaxThresholdRatio[ii] = (GT_32)inArgs[15+ii+ (17*j)];
        }
*/
		profileStrPtr[j].dropMode = (CPSS_TM_DROP_MODE_ENT)inArgs[5+ (17*j)];
		profileStrPtr[j].cbTdThresholdBytes = cpssTmDropConvertBWToBytes((GT_U32)inArgs[2+ (17*j)]) * (GT_U32)inArgs[3+ (17*j)] / 100;
		switch(profileStrPtr[j].dropMode)
		{
			case CPSS_TM_DROP_MODE_CB_TD_CA_WRED_E:
				profileStrPtr[j].caWredDp.aqlExponent  = (GT_U8)inArgs[4+ (17*j)];
				for(ii = CPSS_DP_GREEN_E; ii <= CPSS_DP_RED_E; ii++)
				{
					profileStrPtr[j].caWredDp.curveIndex[ii] =(GT_U8)inArgs[6+ii+ (17*j)];
					profileStrPtr[j].caWredDp.dpCurveScale[ii] = (GT_U8)inArgs[9+ii+ (17*j)];
					/* wred threshold in bytes = bytes (wredCaTdBw) *  min/max_threshold(percents)/100% */
					profileStrPtr[j].caWredDp.caWredTdMinThreshold[ii] = cpssTmDropConvertBWToBytes((GT_U32)inArgs[1+ (17*j)]) * (GT_32)inArgs[12+ii+ (17*j)] / 100 ;
					profileStrPtr[j].caWredDp.caWredTdMaxThreshold[ii] = cpssTmDropConvertBWToBytes((GT_U32)inArgs[1+ (17*j)]) * (GT_32)inArgs[15+ii+ (17*j)] / 100 ;
				}
				break;
			case CPSS_TM_DROP_MODE_CB_TD_CA_TD_E:
				for (ii = CPSS_DP_GREEN_E; ii <= CPSS_DP_RED_E ; ii++)
				{
					/* caTd threshold in bytes = bytes (wredCaTdBw) *  min_threshold(percents)/100% */
					profileStrPtr[j].caTdDp.caTdThreshold[ii] = cpssTmDropConvertBWToBytes((GT_U32)inArgs[1+ (17*j)]) * (GT_32)inArgs[12+ii+ (17*j)] / 100 ;
				}
				break;
			case CPSS_TM_DROP_MODE_CB_TD_ONLY_E:
			default:
				break;
		}

    }

    /* call cpss api function */
    result = cpssTmDropProfileAgingBlockCreate(devNum, profileStrPtr, &profile);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", profile);

    return CMD_OK;
}

/**
* @internal wrCpssTmDropProfileAgingBlockDelete function
* @endinternal
*
* @brief   Delete Aging profile.
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
static CMD_STATUS wrCpssTmDropProfileAgingBlockDelete
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    GT_U32              profile;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    profile = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssTmDropProfileAgingBlockDelete(devNum, profile);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssTmNodeDropProfileIndexRead function
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
static CMD_STATUS wrCpssTmNodeDropProfileIndexRead
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8                       devNum;
    CPSS_TM_LEVEL_ENT           level;
	GT_U8                       cos;
    GT_U32                      nodeIndex;
    GT_U32                      profileIndex;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);


    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    level = (CPSS_TM_LEVEL_ENT)inArgs[1];
    cos = (GT_U8)inArgs[2];
    nodeIndex = (GT_U32)inArgs[3];

    /* call cpss api function */
    result = cpssTmNodeDropProfileIndexRead(devNum, level, cos, nodeIndex, &profileIndex);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }


    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", profileIndex);

    return CMD_OK;
}



/**
* @internal wrCpssTmDropProfileCreate_2 function
* @endinternal
*
* @brief   Create Drop profile new API.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_FULL                  - on full allocation of level's Drop profile table.
* @retval GT_NOT_INITIALIZED       - on not configured AQM Mode params for this lvl.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. Cos of Drop Profile matches Cos of given curve.
*       2. The CoS parameter is relevant for C and P level only, else set CPSS_TM_INVAL_CNS.
*       3. For P level in Global mode set 'cos' = CPSS_TM_INVAL_CNS, else profile will be created for CoS mode.
*
*/

static CMD_STATUS wrCpssTmDropProfileCreate_2
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    CPSS_TM_LEVEL_ENT   level;
    GT_U32              cos;
    CPSS_TM_DROP_PROFILE_PARAMS_STC         profileStrPtr;
    GT_U32              profile;
    CPSS_DP_LEVEL_ENT   ii;
	GT_U32				structureStartIndex;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* init with zeros */
    cmdOsMemSet(&profileStrPtr, 0, sizeof(profileStrPtr));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    level = (CPSS_TM_LEVEL_ENT)inArgs[1];
    cos = (GT_U32)inArgs[2];

	structureStartIndex = 3;

	profileStrPtr.dropMode = (CPSS_TM_DROP_MODE_ENT)inArgs[structureStartIndex];
	profileStrPtr.cbTdThresholdBytes = (GT_U32)inArgs[structureStartIndex+1];
	switch(profileStrPtr.dropMode)
	{
		case CPSS_TM_DROP_MODE_CB_TD_CA_TD_E:
			for (ii = CPSS_DP_GREEN_E; ii <= CPSS_DP_RED_E; ii++)
			{
				profileStrPtr.caTdDp.caTdThreshold[ii] = (GT_U32)inArgs[structureStartIndex+2+ii] ;
			}
			break;
		case CPSS_TM_DROP_MODE_CB_TD_CA_WRED_E:
			profileStrPtr.caWredDp.aqlExponent  = (GT_U8)inArgs[structureStartIndex+5];
			for (ii = CPSS_DP_GREEN_E; ii <= CPSS_DP_RED_E ; ii++)
			{
				profileStrPtr.caWredDp.curveIndex[ii] =(GT_U8)inArgs[structureStartIndex+6+ii];
				profileStrPtr.caWredDp.dpCurveScale[ii] = (GT_U8)inArgs[structureStartIndex+9+ii];
				profileStrPtr.caWredDp.caWredTdMinThreshold[ii] = (GT_U32)inArgs[structureStartIndex+12+ii] ;
				profileStrPtr.caWredDp.caWredTdMaxThreshold[ii] = (GT_U32)inArgs[structureStartIndex+15+ii] ;
			}
			break;
		case CPSS_TM_DROP_MODE_CB_TD_ONLY_E:
		default:
			break;
	}

	/* call cpss api function */
    result = cpssTmDropProfileCreate(devNum, level, cos, &profileStrPtr, &profile);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", profile);

    return CMD_OK;
}


/**
* @internal wrCpssTmDropProfileUpdate_2 function
* @endinternal
*
* @brief   Update Drop profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_FULL                  - on full allocation of level's Drop profile table.
* @retval GT_NOT_INITIALIZED       - on not configured AQM Mode params for this lvl.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. In case of Color Blind TD disabled set cbTdThresholdBytes=CPSS_TM_MAX_DROP_THRESHOLD_CNS
*       2. Cos of Drop Profile matches Cos of given curve.
*       3. The CoS parameter is relevant for C and P level only, else set CPSS_TM_INVAL_CNS.
*       4. For P level in Global mode set 'cos' = CPSS_TM_INVAL_CNS, else profile will be created for CoS mode.
*       5. For Q/A/B/C levels the user is aware to pause traffic during update process
*       6. For P level update is possible only if profile is not in use.
*
*/
static CMD_STATUS wrCpssTmDropProfileUpdate_2
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    CPSS_TM_LEVEL_ENT   level;
    GT_U32              cos;
    GT_U32              profileIndex;
    CPSS_TM_DROP_PROFILE_PARAMS_STC         profileStrPtr;
    CPSS_DP_LEVEL_ENT   ii;
    GT_U32              structureStartIndex;


    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* init with zeros */
    cmdOsMemSet(&profileStrPtr, 0, sizeof(profileStrPtr));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    level = (CPSS_TM_LEVEL_ENT)inArgs[1];
    cos = (GT_U32)inArgs[2];
    profileIndex = (GT_U32)inArgs[3];

	structureStartIndex = 4;

	profileStrPtr.dropMode = (CPSS_TM_DROP_MODE_ENT)inArgs[structureStartIndex];
	profileStrPtr.cbTdThresholdBytes = (GT_U32)inArgs[structureStartIndex+1];
	switch(profileStrPtr.dropMode)
	{
		case CPSS_TM_DROP_MODE_CB_TD_CA_TD_E:
			for (ii = CPSS_DP_GREEN_E; ii <= CPSS_DP_RED_E; ii++)
			{
				profileStrPtr.caTdDp.caTdThreshold[ii] = (GT_U32)inArgs[structureStartIndex+2+ii] ;
			}
			break;
		case CPSS_TM_DROP_MODE_CB_TD_CA_WRED_E:
			profileStrPtr.caWredDp.aqlExponent  = (GT_U8)inArgs[structureStartIndex+5];
			for (ii = CPSS_DP_GREEN_E; ii <= CPSS_DP_RED_E; ii++)
			{
				profileStrPtr.caWredDp.curveIndex[ii] =(GT_U8)inArgs[structureStartIndex+6+ii];
				profileStrPtr.caWredDp.dpCurveScale[ii] = (GT_U8)inArgs[structureStartIndex+9+ii];
				profileStrPtr.caWredDp.caWredTdMinThreshold[ii] = (GT_U32)inArgs[structureStartIndex+12+ii] ;
				profileStrPtr.caWredDp.caWredTdMaxThreshold[ii] = (GT_U32)inArgs[structureStartIndex+15+ii] ;
			}
			break;
		case CPSS_TM_DROP_MODE_CB_TD_ONLY_E:
		default:
			break;
	}

    /* call cpss api function */
    result = cpssTmDropProfileUpdate(devNum, level, cos, profileIndex , &profileStrPtr);

	galtisOutput(outArgs, result, "");

    return CMD_OK;
}

/**
* @internal wrCpssTmDropProfileRead_2 function
* @endinternal
*
* @brief   Read Drop profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_INITIALIZED       - on not configured AQM Mode params for this lvl.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. The cbTdThresholdRatio is calculated aproximately from the register's values.
*       2. The CoS parameter is relevant for C and P level only, else set CPSS_TM_INVAL_CNS.
*       3. For P level in Global mode set 'cos' = CPSS_TM_INVAL_CNS, else profile will be read for CoS mode.
*
*/
static CMD_STATUS wrCpssTmDropProfileRead_2
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_TM_LEVEL_ENT                   level;
    GT_U32                              cos;
    GT_U32                              profileInd;
    CPSS_TM_DROP_PROFILE_PARAMS_STC     profileStrPtr;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

	/* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* initialize profileStrPtr */
    cmdOsMemSet(&profileStrPtr, 0, sizeof(profileStrPtr));


    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    level = (CPSS_TM_LEVEL_ENT)inArgs[1];
    cos = (GT_U32)inArgs[2];
    profileInd = (GT_U32)inArgs[3];


    /* call cpss api function */
    result = cpssTmDropProfileRead(devNum, level, cos, profileInd, &profileStrPtr);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

	/* pack output arguments to galtis string */

    galtisOutput(outArgs, result, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
				 profileStrPtr.dropMode,
				 profileStrPtr.cbTdThresholdBytes,
				 profileStrPtr.caTdDp.caTdThreshold[CPSS_DP_GREEN_E],
				 profileStrPtr.caTdDp.caTdThreshold[CPSS_DP_YELLOW_E],
				 profileStrPtr.caTdDp.caTdThreshold[CPSS_DP_RED_E],
                 profileStrPtr.caWredDp.aqlExponent,
				 profileStrPtr.caWredDp.curveIndex[CPSS_DP_GREEN_E],
                 profileStrPtr.caWredDp.curveIndex[CPSS_DP_YELLOW_E],
				 profileStrPtr.caWredDp.curveIndex[CPSS_DP_RED_E],
				 profileStrPtr.caWredDp.dpCurveScale[CPSS_DP_GREEN_E],
                 profileStrPtr.caWredDp.dpCurveScale[CPSS_DP_YELLOW_E],
				 profileStrPtr.caWredDp.dpCurveScale[CPSS_DP_RED_E],
                 profileStrPtr.caWredDp.caWredTdMinThreshold[CPSS_DP_GREEN_E],
                 profileStrPtr.caWredDp.caWredTdMinThreshold[CPSS_DP_YELLOW_E],
                 profileStrPtr.caWredDp.caWredTdMinThreshold[CPSS_DP_RED_E],
                 profileStrPtr.caWredDp.caWredTdMaxThreshold[CPSS_DP_GREEN_E],
				 profileStrPtr.caWredDp.caWredTdMaxThreshold[CPSS_DP_YELLOW_E],
                 profileStrPtr.caWredDp.caWredTdMaxThreshold[CPSS_DP_RED_E]);

    return CMD_OK;
}


/**
* @internal wrCpssTmDropColorNumResolutionGet function
* @endinternal
*
* @brief   Retrieves Drop (Egress) Colors number and curve resolution birs count per level.
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

static CMD_STATUS wrCpssTmDropColorNumResolutionGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                           result;
    GT_U8                               devNum;
    CPSS_TM_LEVEL_ENT                   level;

	GT_U8 color_num;
	GT_U8 resolution;

    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

	/* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;


    /* map input arguments to locals */

    devNum = (GT_U8)inArgs[0];
    level = (CPSS_TM_LEVEL_ENT)inArgs[1];

    /* call cpss api function */
    result = cpssTmDropColorNumResolutionGet(devNum, level, &color_num , &resolution);

    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "%d", -1);
        return CMD_OK;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d",
				 color_num, resolution);

    return CMD_OK;
}


/**
* @internal wrCpssTmDropProfileAgingBlockCreate_2 function
* @endinternal
*
* @brief   Create Aging profile for CPSS_TM_LEVEL_Q_E.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_FULL                  - on full allocation of level's Drop profile table.
* @retval GT_NOT_INITIALIZED       - on not configured AQM Mode params for this lvl.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. In case of Color Blind TD disabled set cbTdThresholdBytes=CPSS_TM_MAX_DROP_THRESHOLD_CNS
*       2. Cos of Drop Profile matches Cos of given curve.
*       3. The CoS parameter is relevant for C and P level only, else set CPSS_TM_INVAL_CNS.
*       4. For P level in Global mode set 'cos' = CPSS_TM_INVAL_CNS, else profile will be created for CoS mode.
*
*/
static CMD_STATUS wrCpssTmDropProfileAgingBlockCreate_2
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;
    GT_U8     devNum;
    CPSS_TM_DROP_PROFILE_PARAMS_STC         profileStrPtr[CPSS_TM_AGING_PROFILES_CNS];
    GT_U32              profile;
    GT_U32              j;
    CPSS_DP_LEVEL_ENT   ii;
    GT_U32              structureStartIndex;


    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* init with zeros */
    cmdOsMemSet(&profileStrPtr, 0, sizeof(profileStrPtr));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
	structureStartIndex  = 1;
    for (j=0;j<CPSS_TM_AGING_PROFILES_CNS; j++ )
	{
		profileStrPtr[j].dropMode = (CPSS_TM_DROP_MODE_ENT)inArgs[structureStartIndex];
		profileStrPtr[j].cbTdThresholdBytes = (GT_U32)inArgs[structureStartIndex+1];
		switch(profileStrPtr[j].dropMode)
		{
			case CPSS_TM_DROP_MODE_CB_TD_CA_TD_E:
				for (ii = CPSS_DP_GREEN_E; ii <= CPSS_DP_RED_E; ii++)
				{
					profileStrPtr[j].caTdDp.caTdThreshold[ii] = (GT_U32)inArgs[structureStartIndex+2+ii] ;
				}
				break;
			case CPSS_TM_DROP_MODE_CB_TD_CA_WRED_E:
				profileStrPtr[j].caWredDp.aqlExponent  = (GT_U8)inArgs[structureStartIndex+5];
				for (ii = CPSS_DP_GREEN_E; ii <= CPSS_DP_RED_E; ii++)
				{
					profileStrPtr[j].caWredDp.curveIndex[ii] =(GT_U8)inArgs[structureStartIndex+6+ii];
					profileStrPtr[j].caWredDp.dpCurveScale[ii] = (GT_U8)inArgs[structureStartIndex+9+ii];
					profileStrPtr[j].caWredDp.caWredTdMinThreshold[ii] = (GT_U32)inArgs[structureStartIndex+12+ii] ;
					profileStrPtr[j].caWredDp.caWredTdMaxThreshold[ii] = (GT_U32)inArgs[structureStartIndex+15+ii] ;
				}
			case CPSS_TM_DROP_MODE_CB_TD_ONLY_E:
			default:
				break;
		}
		structureStartIndex += 18;
	}

    /* call cpss api function */
    result = cpssTmDropProfileAgingBlockCreate(devNum, profileStrPtr, &profile);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", profile);

    return CMD_OK;
}

/**
* @internal wrCpssTmDropProfileAgingUpdate_2 function
* @endinternal
*
* @brief   Update Aging Drop profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat; Lion; xCat2; DxCh1; DxCh1_Diamond; DxCh2; DxCh3; Lion2; Puma2; Puma3; ExMx; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_FULL                  - on full allocation of level's Drop profile table.
* @retval GT_NOT_INITIALIZED       - on not configured AQM Mode params for this lvl.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static CMD_STATUS wrCpssTmDropProfileAgingUpdate_2
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
	GT_STATUS							result;
    GT_U8								devNum;
    GT_U32								profileIndex;
    IN  GT_U32							profileOffset;
    CPSS_TM_DROP_PROFILE_PARAMS_STC		profileStrPtr;

    CPSS_DP_LEVEL_ENT   ii;
    GT_U32              structureStartIndex;


    GT_UNUSED_PARAM(inFields);
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* init with zeros */
    cmdOsMemSet(&profileStrPtr, 0, sizeof(profileStrPtr));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    profileIndex = (GT_U32)inArgs[1];
    profileOffset = (GT_U32)inArgs[2];

	structureStartIndex = 3;

	profileStrPtr.dropMode = (CPSS_TM_DROP_MODE_ENT)inArgs[structureStartIndex];
	profileStrPtr.cbTdThresholdBytes = (GT_U32)inArgs[structureStartIndex+1];
	switch(profileStrPtr.dropMode)
	{
		case CPSS_TM_DROP_MODE_CB_TD_CA_TD_E:
			for (ii = CPSS_DP_GREEN_E; ii <= CPSS_DP_RED_E ; ii++)
			{
				profileStrPtr.caTdDp.caTdThreshold[ii] = (GT_U32)inArgs[structureStartIndex+2+ii] ;
			}
			break;
		case CPSS_TM_DROP_MODE_CB_TD_CA_WRED_E:
			profileStrPtr.caWredDp.aqlExponent  = (GT_U8)inArgs[structureStartIndex+5];
			for (ii = CPSS_DP_GREEN_E; ii <= CPSS_DP_RED_E ; ii++)
			{
				profileStrPtr.caWredDp.curveIndex[ii] =(GT_U8)inArgs[structureStartIndex+6+ii];
				profileStrPtr.caWredDp.dpCurveScale[ii] = (GT_U8)inArgs[structureStartIndex+9+ii];
				profileStrPtr.caWredDp.caWredTdMinThreshold[ii] = (GT_U32)inArgs[structureStartIndex+12+ii] ;
				profileStrPtr.caWredDp.caWredTdMaxThreshold[ii] = (GT_U32)inArgs[structureStartIndex+15+ii] ;
			}
			break;
		case CPSS_TM_DROP_MODE_CB_TD_ONLY_E:
		default:
			break;
	}
    /* call cpss api function */
	result = cpssTmDropProfileAgingUpdate(devNum, profileIndex, profileOffset,&profileStrPtr);
	galtisOutput(outArgs, result, "");

    return CMD_OK;
}



/**** database initialization **************************************/


static CMD_COMMAND dbCommands[] =
{
    {"cpssTmDropWredCurveCreate",
        &wrCpssTmDropWredCurveCreate,
        33, 0},
    {"cpssTmDropWredTraditionalCurveCreate",
        &wrCpssTmDropWredTraditionalCurveCreate,
        4, 0},
    {"cpssTmDropProfileCreate",
        &wrCpssTmDropProfileCreate,
        20, 0},
    {"cpssTmDropProfileDelete",
        &wrCpssTmDropProfileDelete,
        4, 0},
    {"cpssTmDropProfileRead",
        &wrCpssTmDropProfileRead,
        4, 0},
    {"cpssTmDropColorNumSet",
        &wrCpssTmDropColorNumSet,
        3, 0},
    {"cpssTmAgingChangeStatus",
        &wrCpssTmAgingChangeStatus,
        2, 0},
    {"cpssTmAgingStatusGet",
        &wrCpssTmAgingStatusGet,
        1, 0},
    {"cpssTmDpSourceSet",
        &wrCpssTmDpSourceSet,
        4, 0},
    {"cpssTmDpSourceGet",
        &wrCpssTmDpSourceGet,
        3, 0},
    {"cpssTmDropQueryResponceSet",
        &wrCpssTmDropQueryResponceSet,
        3, 0},
    {"cpssTmDropQueryResponceGet",
        &wrCpssTmDropQueryResponceGet,
        1, 0},
    {"cpssTmDropQueueCosSet",
        &wrCpssTmDropQueueCosSet,
        3, 0},
    {"cpssTmDropQueueCosGet",
        &wrCpssTmDropQueueCosGet,
        2, 0},
    {"cpssTmDropProfileAgingBlockCreate",
        &wrCpssTmDropProfileAgingBlockCreate,
        69, 0},
    {"cpssTmDropProfileAgingBlockDelete",
        &wrCpssTmDropProfileAgingBlockDelete,
        2, 0},
    {"cpssTmNodeDropProfileIndexRead",
        &wrCpssTmNodeDropProfileIndexRead,
        4, 0},
    {"cpssTmDropProfileCreate_2",
        &wrCpssTmDropProfileCreate_2,
        21, 0},
    {"cpssTmDropProfileUpdate_2",
        &wrCpssTmDropProfileUpdate_2,
        22, 0},
    {"cpssTmDropProfileRead_2",
        &wrCpssTmDropProfileRead_2,
        4, 0},
    {"cpssTmDropColorNumResolutionGet",
        &wrCpssTmDropColorNumResolutionGet,
        2, 0},
    {"cpssTmDropProfileAgingBlockCreate_2",
        &wrCpssTmDropProfileAgingBlockCreate_2,
        58, 0},
    {"cpssTmDropProfileAgingUpdate_2",
        &wrCpssTmDropProfileAgingUpdate_2,
        22, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssTmDrop function
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
GT_STATUS cmdLibInitCpssTmDrop
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}

