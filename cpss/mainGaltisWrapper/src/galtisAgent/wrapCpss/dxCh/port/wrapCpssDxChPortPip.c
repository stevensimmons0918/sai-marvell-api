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
* @file wrapCpssDxChPortPip.c
*
* @brief Wrapper functions for CPSS DXCH Port PIP API
*
* @version   1
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>


/* Feature specific includes. */
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortPip.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpssCommon/private/prvCpssMath.h>

/**
* @internal wrCpssDxChPortPipGlobalEnableSet function
* @endinternal
*
* @brief   Enable/disable the PIP engine. (Global to the device)
*         if enabled packets are dropped according to their parsed priority if FIFOs
*         before the control pipe are above a configurable threshold
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipGlobalEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS result;
    GT_U8    devNum;
    GT_BOOL             enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    enable  = (GT_BOOL)inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortPipGlobalEnableSet(devNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortPipGlobalEnableGet function
* @endinternal
*
* @brief   Get the state Enable/disable of the PIP engine. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipGlobalEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS  result;
    GT_U8      devNum;
    GT_BOOL    enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortPipGlobalEnableGet(devNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortPipGlobalVidClassificationSet function
* @endinternal
*
* @brief   Set PIP vid classification value per index. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_OUT_OF_RANGE          - on out of range of vid
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipGlobalVidClassificationSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS result;
    GT_U8    devNum;
    GT_U32   index;
    GT_U16   vid;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    index  = (GT_U32)inArgs[ii++];
    vid  = (GT_U16)inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortPipGlobalVidClassificationSet(devNum, index , vid);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortPipGlobalVidClassificationGet function
* @endinternal
*
* @brief   Get PIP vid classification value per index. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipGlobalVidClassificationGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS  result;
    GT_U8      devNum;
    GT_U32   index;
    GT_U16   vid;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    index  = (GT_U32)inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortPipGlobalVidClassificationGet(devNum, index , &vid);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", vid);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortPipGlobalMacDaClassificationEntrySet function
* @endinternal
*
* @brief   Set PIP Mac Da classification entry. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of value in entry
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipGlobalMacDaClassificationEntrySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS result;
    GT_U8    devNum;
    GT_U32   index;
    CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC entry;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&entry,0,sizeof(entry));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    index  = (GT_U32)inArgs[ii++];
    galtisMacAddr(&entry.macAddrValue, (GT_U8*)inArgs[ii++]);
    galtisMacAddr(&entry.macAddrMask, (GT_U8*)inArgs[ii++]);

    /* call cpss api function */
    result = cpssDxChPortPipGlobalMacDaClassificationEntrySet(devNum, index , &entry);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortPipGlobalMacDaClassificationEntryGet function
* @endinternal
*
* @brief   Get PIP Mac Da classification entry. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipGlobalMacDaClassificationEntryGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS result;
    GT_U8    devNum;
    GT_U32   index;
    CPSS_DXCH_PORT_PIP_MAC_DA_CLASSIFICATION_STC entry;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&entry,0,sizeof(entry));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    index  = (GT_U32)inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortPipGlobalMacDaClassificationEntryGet(devNum, index , &entry);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%6b%6b",
                            entry.macAddrValue.arEther,
                            entry.macAddrMask.arEther);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortPipGlobalBurstFifoThresholdsSet function
* @endinternal
*
* @brief   Set Burst FIFO Threshold for specific PIP priority. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_OUT_OF_RANGE          - on out of range threshold
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipGlobalBurstFifoThresholdsSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS result;
    GT_U8    devNum;
    GT_U32   priority;
    GT_U32   threshold;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    priority  = (GT_U32)inArgs[ii++];
    threshold  = (GT_U32)inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortPipGlobalBurstFifoThresholdsSet(devNum,
        CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority, threshold);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortPipGlobalBurstFifoThresholdsGet function
* @endinternal
*
* @brief   Get Burst FIFO Threshold for specific PIP priority. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipGlobalBurstFifoThresholdsGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS result;
    GT_U8    devNum;
    GT_U32   priority;
    GT_U32   threshold;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    priority  = (GT_U32)inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortPipGlobalBurstFifoThresholdsGet(devNum,
        CPSS_DATA_PATH_UNAWARE_MODE_CNS, priority , &threshold);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", threshold);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortPipGlobalBurstFifoThresholdsSet1 function
* @endinternal
*
* @brief   Set Burst FIFO Threshold for specific PIP priority. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_OUT_OF_RANGE          - on out of range threshold
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipGlobalBurstFifoThresholdsSet1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS result;
    GT_U8    devNum;
    GT_U32   priority;
    GT_U32   threshold;
    GT_U32   dataPathBmp;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    dataPathBmp = (GT_DATA_PATH_BMP)inArgs[ii++];
    priority  = (GT_U32)inArgs[ii++];
    threshold  = (GT_U32)inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortPipGlobalBurstFifoThresholdsSet(devNum, dataPathBmp,
        priority, threshold);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortPipGlobalBurstFifoThresholdsGet1 function
* @endinternal
*
* @brief   Get Burst FIFO Threshold for specific PIP priority. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipGlobalBurstFifoThresholdsGet1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS result;
    GT_U8    devNum;
    GT_U32   priority;
    GT_U32   threshold;
    GT_U32   dataPathBmp;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    dataPathBmp = (GT_U8)inArgs[ii++];
    priority  = (GT_U32)inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortPipGlobalBurstFifoThresholdsGet(devNum, dataPathBmp,
        priority, &threshold);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", threshold);
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortPipGlobalEtherTypeProtocolSet function
* @endinternal
*
* @brief   Set the PIP etherTypes for the protocol. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or protocol or index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipGlobalEtherTypeProtocolSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS result;
    GT_U8    devNum;
    CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol;
    GT_U32   index;
    GT_U16   etherType;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    protocol  = (CPSS_DXCH_PORT_PIP_PROTOCOL_ENT)inArgs[ii++];
    index  = (GT_U32)inArgs[ii++];
    etherType  = (GT_U16)inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortPipGlobalEtherTypeProtocolSet(devNum, protocol , index , etherType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}
/**
* @internal wrCpssDxChPortPipGlobalEtherTypeProtocolGet function
* @endinternal
*
* @brief   Get the PIP etherTypes for the protocol. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or protocol or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipGlobalEtherTypeProtocolGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS result;
    GT_U8    devNum;
    CPSS_DXCH_PORT_PIP_PROTOCOL_ENT  protocol;
    GT_U32   index;
    GT_U16   etherType;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    protocol  = (CPSS_DXCH_PORT_PIP_PROTOCOL_ENT)inArgs[ii++];
    index  = (GT_U32)inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortPipGlobalEtherTypeProtocolGet(devNum, protocol , index , &etherType);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", etherType);
    return CMD_OK;
}

/* BC3 has 4 PIP priorities */
#define PIP_MAX_NUM_OF_PRIORITIES_CNS 4
/**
* @internal wrCpssDxChPortPipGlobalDropCounterGet function
* @endinternal
*
* @brief   Get PIP drop counter of specific PIP priority. (Global to the device)
*         NOTE: the counters are 'clear on read'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipGlobalDropCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS result;
    GT_U8    devNum;
    GT_U32   priority;
    GT_U64   counter;
    static  GT_U64   saved_counterArr[PIP_MAX_NUM_OF_PRIORITIES_CNS];
    GT_U64   new_counterArr[PIP_MAX_NUM_OF_PRIORITIES_CNS];

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    priority  = (GT_U32)inArgs[ii++];

    if(priority >= PIP_MAX_NUM_OF_PRIORITIES_CNS)
    {
        /* pack output arguments to galtis string */
        galtisOutput(outArgs, GT_BAD_PARAM, "%d%d", 0,0);
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssDxChPortPipGlobalDropCounterGet(devNum, new_counterArr);
    if(result == GT_OK)
    {
        /* accumulate the new and the saved values */
        saved_counterArr[0] = prvCpssMathAdd64(new_counterArr[0],saved_counterArr[0]);
        saved_counterArr[1] = prvCpssMathAdd64(new_counterArr[1],saved_counterArr[1]);
        saved_counterArr[2] = prvCpssMathAdd64(new_counterArr[2],saved_counterArr[2]);
        saved_counterArr[3] = prvCpssMathAdd64(new_counterArr[3],saved_counterArr[3]);

        /* SIP_5_20 priority range was checked above, check priority for non SIP_5_20 */
        if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) && (priority > 2))
        {
            /* pack output arguments to galtis string */
            galtisOutput(outArgs, GT_BAD_PARAM, "%d%d", 0,0);
            return CMD_OK;
        }

        /* get the needed counter */
        counter = saved_counterArr[priority];

        /* reset the counter in th DB */
        saved_counterArr[priority].l[0] = 0;
        saved_counterArr[priority].l[1] = 0;
    }
    else
    {
        counter.l[0] = 0;
        counter.l[1] = 0;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d", counter.l[0],counter.l[1]);

    return CMD_OK;
}

/**
* @internal wrCpssDxChPortPipGlobalDropCounterGet1 function
* @endinternal
*
* @brief   Get PIP drop counter of specific PIP priority. (Global to the device)
*         NOTE: the counters are 'clear on read'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or priority
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipGlobalDropCounterGet1
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS result;
    GT_U8    devNum;
    GT_U64   counterArr[PIP_MAX_NUM_OF_PRIORITIES_CNS];

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortPipGlobalDropCounterGet(devNum, counterArr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d",
        counterArr[0].l[0],counterArr[0].l[1],
        counterArr[1].l[0],counterArr[1].l[1],
        counterArr[2].l[0],counterArr[2].l[1]
        );
    return CMD_OK;
}


/**
* @internal wrCpssDxChPortPipTrustEnableSet function
* @endinternal
*
* @brief   Set port as trusted/not trusted PIP port.
*         A 'trusted' port will set priority level for ingress packets according to
*         packet's fields.
*         Otherwise, for the 'not trusted' port will use the port�s default priority
*         level.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipTrustEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    portNum  = (GT_PHYSICAL_PORT_NUM)inArgs[ii++];
    enable  = (GT_BOOL)inArgs[ii++];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortPipTrustEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortPipTrustEnableGet function
* @endinternal
*
* @brief   Get port's trusted/not trusted .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipTrustEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS  result;
    GT_U8      devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL    enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    portNum  = (GT_PHYSICAL_PORT_NUM)inArgs[ii++];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortPipTrustEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}
/**
* @internal wrCpssDxChPortPipProfileSet function
* @endinternal
*
* @brief   Set port's PIP profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipProfileSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_U32                          pipProfile;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    portNum  = (GT_PHYSICAL_PORT_NUM)inArgs[ii++];
    pipProfile  = (GT_U32)inArgs[ii++];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortPipProfileSet(devNum, portNum, pipProfile);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortPipProfileGet function
* @endinternal
*
* @brief   Get port's PIP profile.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipProfileGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_U32                          pipProfile;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    portNum  = (GT_PHYSICAL_PORT_NUM)inArgs[ii++];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortPipProfileGet(devNum, portNum, &pipProfile);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", pipProfile);
    return CMD_OK;
}
/**
* @internal wrCpssDxChPortPipVidClassificationEnableSet function
* @endinternal
*
* @brief   Set port PIP enable/disable VID classification .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipVidClassificationEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    portNum  = (GT_PHYSICAL_PORT_NUM)inArgs[ii++];
    enable  = (GT_BOOL)inArgs[ii++];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortPipVidClassificationEnableSet(devNum, portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}
/**
* @internal wrCpssDxChPortPipVidClassificationEnableGet function
* @endinternal
*
* @brief   Get port PIP enable/disable VID classification .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipVidClassificationEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_BOOL                         enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    portNum  = (GT_PHYSICAL_PORT_NUM)inArgs[ii++];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortPipVidClassificationEnableGet(devNum, portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", enable);
    return CMD_OK;
}
/**
* @internal wrCpssDxChPortPipPrioritySet function
* @endinternal
*
* @brief   Set priority per Port for 'classification' type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum or type
* @retval GT_OUT_OF_RANGE          - on out of range priority
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipPrioritySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT type;
    GT_U32                          priority;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    portNum  = (GT_PHYSICAL_PORT_NUM)inArgs[ii++];
    type  = (CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT)inArgs[ii++];
    priority  = (GT_U32)inArgs[ii++];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortPipPrioritySet(devNum, portNum, type , priority);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChPortPipPriorityGet function
* @endinternal
*
* @brief   Get priority per Port for 'classification' type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum or type
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipPriorityGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT type;
    GT_U32                          priority;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    portNum  = (GT_PHYSICAL_PORT_NUM)inArgs[ii++];
    type  = (CPSS_DXCH_PORT_PIP_CLASSIFICATION_TYPE_ENT)inArgs[ii++];

    CONVERT_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* call cpss api function */
    result = cpssDxChPortPipPriorityGet(devNum, portNum, type , &priority);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", priority);
    return CMD_OK;
}


/**
* @internal wrCpssDxChPortPipGlobalProfilePrioritySet function
* @endinternal
*
* @brief   Set priority per PIP profile Per 'classification' type. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or type or pipProfile or fieldIndex
* @retval GT_OUT_OF_RANGE          - on out of range priority
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipGlobalProfilePrioritySet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_U32                          pipProfile;
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type;
    GT_U32                          fieldIndex;
    GT_U32                          priority;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    pipProfile  = (GT_U32)inArgs[ii++];
    type  = (CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT)inArgs[ii++];

    ii = 0;/* now start with fields*/
    fieldIndex  = (GT_U32)inFields[ii++];
    priority  = (GT_U32)inFields[ii++];

    /* call cpss api function */
    result = cpssDxChPortPipGlobalProfilePrioritySet(devNum, pipProfile, type , fieldIndex , priority);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/* cpssDxChPortPipGlobalProfilePriorityGetFirst Table */
static GT_U32   indexCnt;
/**
* @internal wrCpssDxChPortPipGlobalProfilePriorityGetNext function
* @endinternal
*
* @brief   Get priority per PIP profile Per 'classification' type. (Global to the device)
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or type or pipProfile or fieldIndex
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipGlobalProfilePriorityGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_U32                          pipProfile;
    CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT type;
    GT_U32                          priority;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    pipProfile  = (GT_U32)inArgs[ii++];
    type  = (CPSS_DXCH_PORT_PIP_PROFILE_CLASSIFICATION_TYPE_ENT)inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortPipGlobalProfilePriorityGet(devNum, pipProfile, type , indexCnt , &priority);
    if (result == GT_BAD_PARAM)
    {
        if (indexCnt == 0)
        {
            /* bad parameters */
            galtisOutput(outArgs, GT_BAD_PARAM, "%d", -1);
            return CMD_OK;
        }
        else
        {
            /* no more indexes */
            galtisOutput(outArgs, GT_OK, "%d", -1);
            return CMD_OK;
        }
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d",indexCnt,priority);

    indexCnt++;

    return CMD_OK;
}

/* get first */
static CMD_STATUS wrCpssDxChPortPipGlobalProfilePriorityGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    indexCnt = 0;
    return wrCpssDxChPortPipGlobalProfilePriorityGetNext(inArgs,inFields,numFields,outArgs);
}

/* cpssDxChPortPipDropCounter Table */
static GT_PHYSICAL_PORT_NUM   portIndex;
/* look for valid portIndex that hold 'RxDma' */
static GT_STATUS getPortWithRxDma(IN GT_U8    devNum)
{
    GT_U32  portRxdmaNum;
    GT_STATUS rc;

    do{
        rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(devNum, portIndex, PRV_CPSS_DXCH_PORT_TYPE_RxDMA_E, &portRxdmaNum);
        if(rc != GT_OK)
        {
            portIndex++;
            continue;
        }

        return GT_OK;
    }while(portIndex < 256);

    return GT_NO_MORE;
}

/**
* @internal wrCpssDxChPortPipDropCounterGetNext function
* @endinternal
*
* @brief   Get PIP drop counter of specific port.
*         NOTE: the counters are 'clear on read'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Falcon.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wrCpssDxChPortPipDropCounterGetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_U64                          counter;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if (!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];

    if (GT_OK != getPortWithRxDma(devNum))
    {
        /* no more ports with RxDma */
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssDxChPortPipDropCounterGet(devNum, portIndex, &counter);

    portNum = portIndex;
    CONVERT_BACK_DEV_PHYSICAL_PORT_MAC(devNum, portNum);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d",portNum,counter.l[0],counter.l[1]);

    portIndex++;

    return CMD_OK;
}

/* get first */
static CMD_STATUS wrCpssDxChPortPipDropCounterGetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    portIndex = 0;
    return wrCpssDxChPortPipDropCounterGetNext(inArgs,inFields,numFields,outArgs);
}


/**
* @internal cpssDxChPortPipGlobalThresholdsRandomizationNumLsbSet function
* @endinternal
*
* @brief   Set the number of randomized least significant PIP FIFO fill level
*          bits that are compared with PIP Prio Thresholds configuration.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] numLsb                - the number of LSBits to be randomized.
*                                      (APPLICABLE RANGES: 0..15)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_OUT_OF_RANGE          - on out of range numLsb
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wr_cpssDxChPortPipGlobalThresholdsRandomizationNumLsbSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS  result;
    GT_U8      devNum;
    GT_U32    numLsb;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    numLsb = inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortPipGlobalThresholdsRandomizationNumLsbSet(devNum, numLsb);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet function
* @endinternal
*
* @brief   Get the number of randomized least significant PIP FIFO fill level
*          bits that are compared with PIP Prio Thresholds configuration.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[out] numLsbPtr             - (pointer to) the number of LSBits to be randomized.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wr_cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS  result;
    GT_U8      devNum;
    GT_U32     numLsb;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet(devNum, &numLsb);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", numLsb);
    return CMD_OK;
}

/**
* @internal cpssDxChPortPipGlobalThresholdSet function
* @endinternal
*
* @brief   Set Threshold for specific PIP Threshold type.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp         - bitmap of port groups
*                                      (APPLICABLE DEVICES: Falcon)
*                                      NOTEs:
*                                      1. for non multi port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi port groups (or pipes) device:
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] thresholdType            - the type of threshold
* @param[in] threshold                - the  value. (number of packets)
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portGroupsBmp or thresholdType
* @retval GT_OUT_OF_RANGE          - on out of range threshold
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wr_cpssDxChPortPipGlobalThresholdSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS  result;
    GT_U8      devNum;
    GT_PORT_GROUPS_BMP   portGroupsBmp;
    CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPES_ENT thresholdType;
    GT_U32           threshold;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    portGroupsBmp = inArgs[ii++];
    thresholdType = (CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPES_ENT)inArgs[ii++];
    threshold     = inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortPipGlobalThresholdSet(devNum, portGroupsBmp , thresholdType , threshold);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal cpssDxChPortPipGlobalThresholdGet function
* @endinternal
*
* @brief   Get Threshold for specific PIP Threshold type.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portGroupsBmp         - bitmap of port groups
*                                      (APPLICABLE DEVICES: Falcon)
*                                      NOTEs:
*                                      1. for non multi port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi port groups (or pipes) device:
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] thresholdType            - the type of threshold
* @param[out] thresholdPtr            - (pointer to)the  value. (number of packets)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or portGroupsBmp or thresholdType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wr_cpssDxChPortPipGlobalThresholdGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS  result;
    GT_U8      devNum;
    GT_PORT_GROUPS_BMP   portGroupsBmp;
    CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPES_ENT thresholdType;
    GT_U32           threshold;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    portGroupsBmp = inArgs[ii++];
    thresholdType = (CPSS_DXCH_PORT_PIP_GLOBAL_THRESHOLD_TYPES_ENT)inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortPipGlobalThresholdGet(devNum, portGroupsBmp , thresholdType , &threshold);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", threshold);
    return CMD_OK;
}

/**
* @internal cpssDxChPortPipGlobalPfcTcVectorSet function
* @endinternal
*
* @brief   Set 'TC vector' for PFC generation for specific PIP congestion type.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] vectorType            - the type of TC vector
* @param[in] tcVector              - the TC vector. (bitmap of TCs : 8 bits)
*                                      (APPLICABLE RANGES: 0..0xFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or vectorType
* @retval GT_OUT_OF_RANGE          - on out of range tcVector
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wr_cpssDxChPortPipGlobalPfcTcVectorSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS  result;
    GT_U8      devNum;
    CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPES_ENT vectorType;
    GT_U32               tcVector;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    vectorType = (CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPES_ENT)inArgs[ii++];
    tcVector   = inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortPipGlobalPfcTcVectorSet(devNum, vectorType , tcVector);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal cpssDxChPortPipGlobalPfcTcVectorGet function
* @endinternal
*
* @brief   Get 'TC vector' for PFC generation for specific PIP congestion type.
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] vectorType            - the type of TC vector
* @param[out] tcVectorPtr           - (pointer to)the TC vector. (bitmap of TCs : 8 bits)
*                                      (APPLICABLE RANGES: 0..0xFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or vectorType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wr_cpssDxChPortPipGlobalPfcTcVectorGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS  result;
    GT_U8      devNum;
    CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPES_ENT vectorType;
    GT_U32               tcVector;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    vectorType = (CPSS_DXCH_PORT_PIP_GLOBAL_PFC_TC_VECTOR_TYPES_ENT)inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortPipGlobalPfcTcVectorGet(devNum, vectorType , &tcVector);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", tcVector);
    return CMD_OK;
}

/**
* @internal cpssDxChPortRxDmaGlobalDropCounterModeSet function
* @endinternal
*
* @brief   Set the RXDMA drop counter mode info (what drops to count)
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] counterModeInfoPtr    - (pointer to) counter mode info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or counterModeInfoPtr->modesBmp
* @retval GT_OUT_OF_RANGE          - on out of range
*                                       counterModeInfoPtr->portNumPattern  or
*                                       counterModeInfoPtr->portNumMask     or
*                                       counterModeInfoPtr->priorityPattern or
*                                       counterModeInfoPtr->priorityMask
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wr_cpssDxChPortRxDmaGlobalDropCounterModeSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS  result;
    GT_U8      devNum;
    CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_INFO_STC  counterModeInfo;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    cpssOsMemSet(&counterModeInfo,0,sizeof(counterModeInfo));

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];
    counterModeInfo.modesBmp        = inArgs[ii++];
    counterModeInfo.portNumPattern  = inArgs[ii++];
    counterModeInfo.portNumMask     = inArgs[ii++];
    counterModeInfo.priorityPattern = inArgs[ii++];
    counterModeInfo.priorityMask    = inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortRxDmaGlobalDropCounterModeSet(devNum, &counterModeInfo);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal cpssDxChPortRxDmaGlobalDropCounterModeGet function
* @endinternal
*
* @brief   Get the RXDMA drop counter mode info (what drops to count)
*         (Global to the device)
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[out] counterModeInfoPtr   - (pointer to) counter mode info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on unexpected mode/value read from the HW
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wr_cpssDxChPortRxDmaGlobalDropCounterModeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS  result;
    GT_U8      devNum;
    CPSS_DXCH_PORT_RXDMA_DROP_COUNTER_MODE_INFO_STC  counterModeInfo;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortRxDmaGlobalDropCounterModeGet(devNum, &counterModeInfo);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d",
         counterModeInfo.modesBmp
        ,counterModeInfo.portNumPattern
        ,counterModeInfo.portNumMask
        ,counterModeInfo.priorityPattern
        ,counterModeInfo.priorityMask);
    return CMD_OK;
}

/**
* @internal cpssDxChPortRxDmaGlobalDropCounterGet function
* @endinternal
*
* @brief  Get the RXDMA drop counter value.
*         NOTE: the counters are 'clear on read'
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[out] counterPtr           - (pointer to) the counter value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wr_cpssDxChPortRxDmaGlobalDropCounterGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS  result;
    GT_U8      devNum;
    GT_U64     counter;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortRxDmaGlobalDropCounterGet(devNum, CPSS_DATA_PATH_UNAWARE_MODE_CNS, &counter);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d",
         counter.l[0]
        ,counter.l[1]);
    return CMD_OK;
}

/**
* @internal cpssDxChPortParserGlobalTpidSet function
* @endinternal
*
* @brief   Set the TPID and it's size to allow the 'Rx parser' to 'jump over' up
*           to 4 such tags in order to recognize upper layer protocol.
*           (Global to the device - not 'per port')
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    - index to one of 4 TPIDs
*                                      (APPLICABLE RANGES: 0..3)
* @param[in] etherType                -  the TPID to recognize the protocols packets
* @param[in] tpidSize                 - the TPID size
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index or tpidSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wr_cpssDxChPortParserGlobalTpidSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS  result;
    GT_U8      devNum;
    GT_U32       index;
    GT_U16       etherType;
    CPSS_BRG_TPID_SIZE_TYPE_ENT  tpidSize;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[ii++];
    index      = inArgs[ii++];
    etherType  = (GT_U16)inArgs[ii++];
    tpidSize   = (CPSS_BRG_TPID_SIZE_TYPE_ENT)inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortParserGlobalTpidSet(devNum, index , etherType , tpidSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal cpssDxChPortParserGlobalTpidGet function
* @endinternal
*
* @brief   Get the TPID and it's size to allow the 'Rx parser' to 'jump over' up
*           to 4 such tags in order to recognize upper layer protocol.
*           (Global to the device - not 'per port')
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] index                    - index to one of 4 TPIDs
*                                      (APPLICABLE RANGES: 0..3)
* @param[out] etherTypePtr            - (pointer to) the TPID to recognize the protocols packets.
* @param[out] tpidSizePtr             - (pointer to) the TPID size.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum or index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static CMD_STATUS wr_cpssDxChPortParserGlobalTpidGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U32   ii = 0;
    GT_STATUS  result;
    GT_U8      devNum;
    GT_U32       index;
    GT_U16       etherType;
    CPSS_BRG_TPID_SIZE_TYPE_ENT  tpidSize;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum     = (GT_U8)inArgs[ii++];
    index      = inArgs[ii++];

    /* call cpss api function */
    result = cpssDxChPortParserGlobalTpidGet(devNum, index , &etherType , &tpidSize);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d",
         etherType
        ,tpidSize);
    return CMD_OK;
}



/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    /*********************/
    /* commands - global */
    /*********************/
    {"cpssDxChPortPipGlobalEnableSet",
        &wrCpssDxChPortPipGlobalEnableSet,
        2, 0},
    {"cpssDxChPortPipGlobalEnableGet",
        &wrCpssDxChPortPipGlobalEnableGet,
        1, 0},

    {"cpssDxChPortPipGlobalVidClassificationSet",
        &wrCpssDxChPortPipGlobalVidClassificationSet,
        3, 0},
    {"cpssDxChPortPipGlobalVidClassificationGet",
        &wrCpssDxChPortPipGlobalVidClassificationGet,
        2, 0},

    {"cpssDxChPortPipGlobalMacDaClassificationEntrySet",
        &wrCpssDxChPortPipGlobalMacDaClassificationEntrySet,
        4, 0},
    {"cpssDxChPortPipGlobalMacDaClassificationEntryGet",
        &wrCpssDxChPortPipGlobalMacDaClassificationEntryGet,
        2, 0},

    {"cpssDxChPortPipGlobalBurstFifoThresholdsSet",
        &wrCpssDxChPortPipGlobalBurstFifoThresholdsSet,
        3, 0},
    {"cpssDxChPortPipGlobalBurstFifoThresholdsGet",
        &wrCpssDxChPortPipGlobalBurstFifoThresholdsGet,
        2, 0},
    {"cpssDxChPortPipGlobalBurstFifoThresholdsSet1",
        &wrCpssDxChPortPipGlobalBurstFifoThresholdsSet1,
        4, 0},
    {"cpssDxChPortPipGlobalBurstFifoThresholdsGet1",
        &wrCpssDxChPortPipGlobalBurstFifoThresholdsGet1,
        3, 0},


    {"cpssDxChPortPipGlobalEtherTypeProtocolSet",
        &wrCpssDxChPortPipGlobalEtherTypeProtocolSet,
        4, 0},
    {"cpssDxChPortPipGlobalEtherTypeProtocolGet",
        &wrCpssDxChPortPipGlobalEtherTypeProtocolGet,
        3, 0},

    {"cpssDxChPortPipGlobalDropCounterGet",
        &wrCpssDxChPortPipGlobalDropCounterGet,
        2, 0},
    {"cpssDxChPortPipGlobalDropCounterGet1",
        &wrCpssDxChPortPipGlobalDropCounterGet1,
        1, 0},


    /***********************/
    /* commands - per port */
    /***********************/
    {"cpssDxChPortPipTrustEnableSet",
        &wrCpssDxChPortPipTrustEnableSet,
        3, 0},
    {"cpssDxChPortPipTrustEnableGet",
        &wrCpssDxChPortPipTrustEnableGet,
        2, 0},

    {"cpssDxChPortPipProfileSet",
        &wrCpssDxChPortPipProfileSet,
        3, 0},
    {"cpssDxChPortPipProfileGet",
        &wrCpssDxChPortPipProfileGet,
        2, 0},

    {"cpssDxChPortPipVidClassificationEnableSet",
        &wrCpssDxChPortPipVidClassificationEnableSet,
        3, 0},
    {"cpssDxChPortPipVidClassificationEnableGet",
        &wrCpssDxChPortPipVidClassificationEnableGet,
        2, 0},

    {"cpssDxChPortPipPrioritySet",
        &wrCpssDxChPortPipPrioritySet,
        4, 0},
    {"cpssDxChPortPipPriorityGet",
        &wrCpssDxChPortPipPriorityGet,
        3, 0},

    /* name shorten from cpssDxChPortPipGlobalThresholdsRandomizationNumLsbSet */
    {"cpssDxChPortPipGlobalThresholdsRandomizNumLsbSet",
    &wr_cpssDxChPortPipGlobalThresholdsRandomizationNumLsbSet,
        2 , 0},

    /* name shorten from cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet */
    {"cpssDxChPortPipGlobalThresholdsRandomizNumLsbGet",
    &wr_cpssDxChPortPipGlobalThresholdsRandomizationNumLsbGet,
        1 , 0},

    {"cpssDxChPortPipGlobalThresholdSet",
    &wr_cpssDxChPortPipGlobalThresholdSet,
        4 , 0},

    {"cpssDxChPortPipGlobalThresholdGet",
    &wr_cpssDxChPortPipGlobalThresholdGet,
        3 , 0},

    {"cpssDxChPortPipGlobalPfcTcVectorSet",
    &wr_cpssDxChPortPipGlobalPfcTcVectorSet,
        3,0},

    {"cpssDxChPortPipGlobalPfcTcVectorGet",
    &wr_cpssDxChPortPipGlobalPfcTcVectorGet,
        2,0},

    {"cpssDxChPortRxDmaGlobalDropCounterModeSet",
    &wr_cpssDxChPortRxDmaGlobalDropCounterModeSet,
        6,0},
    {"cpssDxChPortRxDmaGlobalDropCounterModeGet",
    &wr_cpssDxChPortRxDmaGlobalDropCounterModeGet,
        1,0},
    {"cpssDxChPortRxDmaGlobalDropCounterGet",
    &wr_cpssDxChPortRxDmaGlobalDropCounterGet,
        1,0},

    {"cpssDxChPortParserGlobalTpidSet",
    &wr_cpssDxChPortParserGlobalTpidSet,
        4,0},

    {"cpssDxChPortParserGlobalTpidGet",
    &wr_cpssDxChPortParserGlobalTpidGet,
        2,0},


    /**********/
    /* tables */
    /**********/

    /* table : cpssDxChPortPipGlobalProfilePrioritySet */
            {"cpssDxChPortPipGlobalProfilePrioritySet",
                &wrCpssDxChPortPipGlobalProfilePrioritySet,
                3, /*dev,profile,type*/
                2},/*fieldIndex, priority*/

            {"cpssDxChPortPipGlobalProfilePriorityGetFirst",
                &wrCpssDxChPortPipGlobalProfilePriorityGetFirst,
                3, /*dev,profile,type*/
                0},/*fieldIndex, priority*/

            {"cpssDxChPortPipGlobalProfilePriorityGetNext",
                &wrCpssDxChPortPipGlobalProfilePriorityGetNext,
                3, /*dev,profile,type*/
                0},/*fieldIndex, priority*/

    /* table : cpssDxChPortPipDropCounter */
            {"cpssDxChPortPipDropCounterGetFirst",
                &wrCpssDxChPortPipDropCounterGetFirst,
                1, /*dev*/
                0},/*port , counter*/

            {"cpssDxChPortPipDropCounterGetNext",
                &wrCpssDxChPortPipDropCounterGetNext,
                1, /*dev*/
                0},/*port , counter*/

};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChPortPip function
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
GT_STATUS cmdLibInitCpssDxChPortPip
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


