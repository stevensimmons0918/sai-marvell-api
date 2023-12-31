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
* @file wrapCpssDxChLpm.c
*
* @brief Wrappers for LPM functions on DX
*
* @version   6
********************************************************************************
*/

/* Common galtis includes */
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cmdShell/cmdDb/cmdBase.h>
#include <cmdShell/common/cmdWrapUtils.h>

/* Feature specific includes. */
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpm.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>

static GT_U8                                            devNum;
static GT_U32                                           lpmLineOffset;
static CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT              nodeType;
static GT_U32                                           numOfRanges;
static CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT           rangeSelectionSection;
static CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC              *nextPointerArrayPtr;
static GT_U32                                           entryIndex;

/**
* @internal wrCpssDxChLpmNodeSetFirst function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Write an LPM Node (Bucket) to the HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
static CMD_STATUS wrCpssDxChLpmNodeSetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                           compressedBitVector[8];
    CPSS_DXCH_LPM_REGULAR_NODE_BIT_VECTOR_ENTRY_STC regularBitVector[11];
    GT_U32                                          i;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    lpmLineOffset = (GT_U32)inArgs[1];
    nodeType = (CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT)inArgs[2];
    numOfRanges = (GT_U32)inArgs[3];
    if (numOfRanges == 0)
    {
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_OK;
    }

    for (i = 0; i < 8; i++)
    {
        compressedBitVector[i] = (GT_U8)inArgs[4+i];
    }
    for (i = 0; i < 11; i++)
    {
        regularBitVector[i].bitMap = (GT_U32)inArgs[12+2*i];
        regularBitVector[i].rangeCounter = (GT_U8)inArgs[13+2*i];
    }

    switch (nodeType)
    {
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
            for (i = 0; i < 11; i++)
            {
                rangeSelectionSection.regularBitVector.bitVectorEntry[i].bitMap =
                    regularBitVector[i].bitMap;
                rangeSelectionSection.regularBitVector.bitVectorEntry[i].rangeCounter =
                    regularBitVector[i].rangeCounter;
            }
            break;

        case CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
            rangeSelectionSection.compressed1BitVector.ranges1_4[0] = compressedBitVector[0];
            rangeSelectionSection.compressed1BitVector.ranges1_4[1] = compressedBitVector[1];
            rangeSelectionSection.compressed1BitVector.ranges1_4[2] = compressedBitVector[2];
            rangeSelectionSection.compressed1BitVector.ranges1_4[3] = compressedBitVector[3];
            break;

        case CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
            rangeSelectionSection.compressed2BitVector.ranges1_4[0] = compressedBitVector[0];
            rangeSelectionSection.compressed2BitVector.ranges1_4[1] = compressedBitVector[1];
            rangeSelectionSection.compressed2BitVector.ranges1_4[2] = compressedBitVector[2];
            rangeSelectionSection.compressed2BitVector.ranges1_4[3] = compressedBitVector[3];
            rangeSelectionSection.compressed2BitVector.ranges6_9[0] = compressedBitVector[4];
            rangeSelectionSection.compressed2BitVector.ranges6_9[1] = compressedBitVector[5];
            rangeSelectionSection.compressed2BitVector.ranges6_9[2] = compressedBitVector[6];
            rangeSelectionSection.compressed2BitVector.ranges6_9[3] = compressedBitVector[7];
            break;

        default:
            break;
    }

    nextPointerArrayPtr = cmdOsMalloc(numOfRanges * sizeof(CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC));
    if (nextPointerArrayPtr == NULL)
    {
        galtisOutput(outArgs, GT_NO_RESOURCE, "");
        return CMD_OK;
    }

    entryIndex = 0;
    nextPointerArrayPtr[entryIndex].pointerType = (CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT)inFields[0];
    switch (nextPointerArrayPtr[entryIndex].pointerType)
    {
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
            nextPointerArrayPtr[entryIndex].pointerData.nextNodePointer.nextPointer =
                (GT_U32)inFields[1];
            nextPointerArrayPtr[entryIndex].pointerData.nextNodePointer.range5Index =
                (GT_U32)inFields[2];
            nextPointerArrayPtr[entryIndex].pointerData.nextNodePointer.pointToSipTree =
                (GT_BOOL)inFields[3];
            break;

        case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
        case CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E:
        case CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E:
            nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.ucRpfCheckEnable =
                (GT_BOOL)inFields[4];
            nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.srcAddrCheckMismatchEnable =
                (GT_BOOL)inFields[5];
            nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.ipv6McGroupScopeLevel =
                (CPSS_IPV6_PREFIX_SCOPE_ENT)inFields[6];
            nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.entryIndex =
                (GT_U32)inFields[8];
            break;

        default:
            break;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChLpmNodeSetNext function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Write an LPM Node (Bucket) to the HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
static CMD_STATUS wrCpssDxChLpmNodeSetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    entryIndex++;
    if (entryIndex >= numOfRanges)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }
    nextPointerArrayPtr[entryIndex].pointerType = (CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT)inFields[0];
    switch (nextPointerArrayPtr[entryIndex].pointerType)
    {
    case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
    case CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
        nextPointerArrayPtr[entryIndex].pointerData.nextNodePointer.nextPointer =
            (GT_U32)inFields[1];
        nextPointerArrayPtr[entryIndex].pointerData.nextNodePointer.range5Index =
            (GT_U32)inFields[2];
        nextPointerArrayPtr[entryIndex].pointerData.nextNodePointer.pointToSipTree =
            (GT_BOOL)inFields[3];
        break;

    case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
    case CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E:
    case CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E:
        nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.ucRpfCheckEnable =
            (GT_BOOL)inFields[4];
        nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.srcAddrCheckMismatchEnable =
            (GT_BOOL)inFields[5];
        nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.ipv6McGroupScopeLevel =
            (CPSS_IPV6_PREFIX_SCOPE_ENT)inFields[6];
        nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.entryIndex =
            (GT_U32)inFields[8];
        break;

        default:
            break;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChLpmNodeEndSet function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Write an LPM Node (Bucket) to the HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
static CMD_STATUS wrCpssDxChLpmNodeEndSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    result = cpssDxChLpmNodeWrite(devNum, lpmLineOffset, nodeType,
                                  &rangeSelectionSection, numOfRanges,
                                  nextPointerArrayPtr);
    cmdOsFree(nextPointerArrayPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChLpmFillBitVector function
* @endinternal
*
* @brief   Fill the regular and compressed bit vector arrays
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @param[out] compressedBitVectorPtr   - (pointer to) compressed bit vector array
* @param[out] regularBitVectorPtr      - (pointer to) regular bit vector array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
static GT_VOID wrCpssDxChLpmFillBitVector
(
    OUT GT_U8                                           *compressedBitVectorPtr,
    OUT CPSS_DXCH_LPM_REGULAR_NODE_BIT_VECTOR_ENTRY_STC *regularBitVectorPtr
)
{
    GT_U32 i, j;

    switch (nodeType)
    {
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
            for (i = 0; i < 11; i++)
            {
                regularBitVectorPtr[i].bitMap =
                    rangeSelectionSection.regularBitVector.bitVectorEntry[i].bitMap;
                regularBitVectorPtr[i].rangeCounter =
                    rangeSelectionSection.regularBitVector.bitVectorEntry[i].rangeCounter;
                for (j = 0; j < 8; j++)
                {
                    compressedBitVectorPtr[j] = 0;
                }
            }
            break;

        case CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
            compressedBitVectorPtr[0] = rangeSelectionSection.compressed1BitVector.ranges1_4[0];
            compressedBitVectorPtr[1] = rangeSelectionSection.compressed1BitVector.ranges1_4[1];
            compressedBitVectorPtr[2] = rangeSelectionSection.compressed1BitVector.ranges1_4[2];
            compressedBitVectorPtr[3] = rangeSelectionSection.compressed1BitVector.ranges1_4[3];
            compressedBitVectorPtr[4] = 0;
            compressedBitVectorPtr[5] = 0;
            compressedBitVectorPtr[6] = 0;
            compressedBitVectorPtr[7] = 0;
            for (j = 0; j < 11; j++)
            {
                regularBitVectorPtr[j].bitMap = 0;
                regularBitVectorPtr[j].rangeCounter = 0;
            }
            break;

        case CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
            compressedBitVectorPtr[0] = rangeSelectionSection.compressed2BitVector.ranges1_4[0];
            compressedBitVectorPtr[1] = rangeSelectionSection.compressed2BitVector.ranges1_4[1];
            compressedBitVectorPtr[2] = rangeSelectionSection.compressed2BitVector.ranges1_4[2];
            compressedBitVectorPtr[3] = rangeSelectionSection.compressed2BitVector.ranges1_4[3];
            compressedBitVectorPtr[4] = rangeSelectionSection.compressed2BitVector.ranges6_9[0];
            compressedBitVectorPtr[5] = rangeSelectionSection.compressed2BitVector.ranges6_9[1];
            compressedBitVectorPtr[6] = rangeSelectionSection.compressed2BitVector.ranges6_9[2];
            compressedBitVectorPtr[7] = rangeSelectionSection.compressed2BitVector.ranges6_9[3];
            for (j = 0; j < 11; j++)
            {
                regularBitVectorPtr[j].bitMap = 0;
                regularBitVectorPtr[j].rangeCounter = 0;
            }
            break;

        default:
            break;
    }
}

/**
* @internal wrCpssDxChLpmNodeGetFirst function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Read an LPM Node (Bucket) from the HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
static CMD_STATUS wrCpssDxChLpmNodeGetFirst
(
 IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
 IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
 IN  GT_32 numFields,
 OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                           compressedBitVector[8];
    CPSS_DXCH_LPM_REGULAR_NODE_BIT_VECTOR_ENTRY_STC regularBitVector[11];
    GT_STATUS                                       result;

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    lpmLineOffset = (GT_U32)inArgs[1];
    nodeType = (CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT)inArgs[2];

    nextPointerArrayPtr = cmdOsMalloc(256 * sizeof(CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC));
    if (nextPointerArrayPtr == NULL)
    {
        galtisOutput(outArgs, GT_NO_RESOURCE, "");
        return CMD_OK;
    }

    /* call cpss api function */
    result = cpssDxChLpmNodeRead(devNum, lpmLineOffset, nodeType,
                                 &rangeSelectionSection, &numOfRanges,
                                 nextPointerArrayPtr);
    if (result != GT_OK)
    {
        galtisOutput(outArgs, result, "");
        return CMD_OK;
    }

    entryIndex = 0;

    inFields[0] = nextPointerArrayPtr[entryIndex].pointerType;
    switch (nextPointerArrayPtr[entryIndex].pointerType)
    {
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
            inFields[1] =
                nextPointerArrayPtr[entryIndex].pointerData.nextNodePointer.nextPointer;
            inFields[2] =
                nextPointerArrayPtr[entryIndex].pointerData.nextNodePointer.range5Index;
            inFields[3] =
                nextPointerArrayPtr[entryIndex].pointerData.nextNodePointer.pointToSipTree;
            inFields[4] = 0;
            inFields[5] = 0;
            inFields[6] = 0;
            inFields[7] = 0;
            inFields[8] = 0;
            break;

        case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
        case CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E:
        case CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E:
            inFields[1] = 0;
            inFields[2] = 0;
            inFields[3] = 0;
            inFields[4] =
                nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.ucRpfCheckEnable;
            inFields[5] =
                nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.srcAddrCheckMismatchEnable;
            inFields[6] =
                nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.ipv6McGroupScopeLevel;
            inFields[7] =
                GT_FALSE;
            inFields[8] =
                nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.entryIndex;
            break;

        default:
            break;
    }

    wrCpssDxChLpmFillBitVector(compressedBitVector, regularBitVector);

    fieldOutput("%d%d%d%d%d%d%d%d%d",
                inFields[0], inFields[1], inFields[2], inFields[3], inFields[4],
                inFields[5], inFields[6], inFields[7], inFields[8]);
    galtisOutput(outArgs, GT_OK,
                 "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%f",
                 numOfRanges,
                 compressedBitVector[0], compressedBitVector[1],
                 compressedBitVector[2], compressedBitVector[3],
                 compressedBitVector[4], compressedBitVector[5],
                 compressedBitVector[6], compressedBitVector[7],
                 regularBitVector[0].bitMap, regularBitVector[0].rangeCounter,
                 regularBitVector[1].bitMap, regularBitVector[1].rangeCounter,
                 regularBitVector[2].bitMap, regularBitVector[2].rangeCounter,
                 regularBitVector[3].bitMap, regularBitVector[3].rangeCounter,
                 regularBitVector[4].bitMap, regularBitVector[4].rangeCounter,
                 regularBitVector[5].bitMap, regularBitVector[5].rangeCounter,
                 regularBitVector[6].bitMap, regularBitVector[6].rangeCounter,
                 regularBitVector[7].bitMap, regularBitVector[7].rangeCounter,
                 regularBitVector[8].bitMap, regularBitVector[8].rangeCounter,
                 regularBitVector[9].bitMap, regularBitVector[9].rangeCounter,
                 regularBitVector[10].bitMap, regularBitVector[10].rangeCounter);

    return CMD_OK;
}

/**
* @internal wrCpssDxChLpmNodeGetNext function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Read an LPM Node (Bucket) from the HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
static CMD_STATUS wrCpssDxChLpmNodeGetNext
(
 IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
 IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
 IN  GT_32 numFields,
 OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_U8                                           compressedBitVector[8];
    CPSS_DXCH_LPM_REGULAR_NODE_BIT_VECTOR_ENTRY_STC regularBitVector[11];

    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    entryIndex++;
    if (entryIndex >= numOfRanges)
    {
        cmdOsFree(nextPointerArrayPtr);
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    inFields[0] = nextPointerArrayPtr[entryIndex].pointerType;
    switch (nextPointerArrayPtr[entryIndex].pointerType)
    {
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
            inFields[1] =
                nextPointerArrayPtr[entryIndex].pointerData.nextNodePointer.nextPointer;
            inFields[2] =
                nextPointerArrayPtr[entryIndex].pointerData.nextNodePointer.range5Index;
            inFields[3] =
                nextPointerArrayPtr[entryIndex].pointerData.nextNodePointer.pointToSipTree;
            inFields[4] = 0;
            inFields[5] = 0;
            inFields[6] = 0;
            inFields[7] = 0;
            inFields[8] = 0;
            break;

        case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
        case CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E:
        case CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E:
            inFields[1] = 0;
            inFields[2] = 0;
            inFields[3] = 0;
            inFields[4] =
                nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.ucRpfCheckEnable;
            inFields[5] =
                nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.srcAddrCheckMismatchEnable;
            inFields[6] =
                nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.ipv6McGroupScopeLevel;
            inFields[7] =
                GT_FALSE;
            inFields[8] =
                nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.entryIndex;
            break;

        default:
            break;
    }

    wrCpssDxChLpmFillBitVector(compressedBitVector, regularBitVector);

    fieldOutput("%d%d%d%d%d%d%d%d%d",
                inFields[0], inFields[1], inFields[2], inFields[3], inFields[4],
                inFields[5], inFields[6], inFields[7], inFields[8]);
    galtisOutput(outArgs, GT_OK,
                 "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%f",
                 numOfRanges,
                 compressedBitVector[0], compressedBitVector[1],
                 compressedBitVector[2], compressedBitVector[3],
                 compressedBitVector[4], compressedBitVector[5],
                 compressedBitVector[6], compressedBitVector[7],
                 regularBitVector[0].bitMap, regularBitVector[0].rangeCounter,
                 regularBitVector[1].bitMap, regularBitVector[1].rangeCounter,
                 regularBitVector[2].bitMap, regularBitVector[2].rangeCounter,
                 regularBitVector[3].bitMap, regularBitVector[3].rangeCounter,
                 regularBitVector[4].bitMap, regularBitVector[4].rangeCounter,
                 regularBitVector[5].bitMap, regularBitVector[5].rangeCounter,
                 regularBitVector[6].bitMap, regularBitVector[6].rangeCounter,
                 regularBitVector[7].bitMap, regularBitVector[7].rangeCounter,
                 regularBitVector[8].bitMap, regularBitVector[8].rangeCounter,
                 regularBitVector[9].bitMap, regularBitVector[9].rangeCounter,
                 regularBitVector[10].bitMap, regularBitVector[10].rangeCounter);

    return CMD_OK;
}

/**
* @internal wrCpssDxChLpmNodeNextPointersSetFirst function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Write an LPM Node's (Bucket) Next Pointer entry (range) to the HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
static CMD_STATUS wrCpssDxChLpmNodeNextPointersSetFirst
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    lpmLineOffset = (GT_U32)inArgs[1];
    numOfRanges = (GT_U32)inArgs[2];
    if (numOfRanges == 0)
    {
        galtisOutput(outArgs, GT_BAD_PARAM, "");
        return CMD_OK;
    }

    nextPointerArrayPtr = cmdOsMalloc(numOfRanges * sizeof(CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC));
    if (nextPointerArrayPtr == NULL)
    {
        galtisOutput(outArgs, GT_NO_RESOURCE, "");
        return CMD_OK;
    }

    entryIndex = 0;
    nextPointerArrayPtr[entryIndex].pointerType = (CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT)inFields[0];
    switch (nextPointerArrayPtr[entryIndex].pointerType)
    {
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
            nextPointerArrayPtr[entryIndex].pointerData.nextNodePointer.nextPointer =
                (GT_U32)inFields[1];
            nextPointerArrayPtr[entryIndex].pointerData.nextNodePointer.range5Index =
                (GT_U32)inFields[2];
            nextPointerArrayPtr[entryIndex].pointerData.nextNodePointer.pointToSipTree =
                (GT_BOOL)inFields[3];
            break;

        case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
        case CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E:
        case CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E:
            nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.ucRpfCheckEnable =
                (GT_BOOL)inFields[4];
            nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.srcAddrCheckMismatchEnable =
                (GT_BOOL)inFields[5];
            nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.ipv6McGroupScopeLevel =
                (CPSS_IPV6_PREFIX_SCOPE_ENT)inFields[6];
            nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.entryIndex =
                (GT_U32)inFields[8];
            break;

        default:
            break;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChLpmNodeNextPointersSetNext function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Write an LPM Node's (Bucket) Next Pointer entry (range) to the HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
static CMD_STATUS wrCpssDxChLpmNodeNextPointersSetNext
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_UNUSED_PARAM(numFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    entryIndex++;
    if (entryIndex >= numOfRanges)
    {
        galtisOutput(outArgs, GT_OK, "%d", -1);
        return CMD_OK;
    }

    nextPointerArrayPtr[entryIndex].pointerType = (CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT)inFields[0];
    switch (nextPointerArrayPtr[entryIndex].pointerType)
    {
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E:
            nextPointerArrayPtr[entryIndex].pointerData.nextNodePointer.nextPointer =
                (GT_U32)inFields[1];
            nextPointerArrayPtr[entryIndex].pointerData.nextNodePointer.range5Index =
                (GT_U32)inFields[2];
            nextPointerArrayPtr[entryIndex].pointerData.nextNodePointer.pointToSipTree =
                (GT_BOOL)inFields[3];
            break;

        case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
        case CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E:
        case CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E:
            nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.ucRpfCheckEnable =
                (GT_BOOL)inFields[4];
            nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.srcAddrCheckMismatchEnable =
                (GT_BOOL)inFields[5];
            nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.ipv6McGroupScopeLevel =
                (CPSS_IPV6_PREFIX_SCOPE_ENT)inFields[6];
            nextPointerArrayPtr[entryIndex].pointerData.nextHopOrEcmpPointer.entryIndex =
                (GT_U32)inFields[8];
            break;

        default:
            break;
    }

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, GT_OK, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChLpmNodeNextPointersEndSet function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Write an LPM Node's (Bucket) Next Pointer entry (range) to the HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
static CMD_STATUS wrCpssDxChLpmNodeNextPointersEndSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS result;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    result = cpssDxChLpmNodeNextPointersWrite(devNum, lpmLineOffset,
                                              numOfRanges, nextPointerArrayPtr);
    cmdOsFree(nextPointerArrayPtr);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChLpmLastLookupStagesBypassEnableSet function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Set the enabling status of bypassing lookup stages 8-31.
*         bypassing the last lookup stages is recommended when IPv6 routing is not
*         used in the system, to decrease the latency caused by the LPM engine.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
static CMD_STATUS wrCpssDxChLpmLastLookupStagesBypassEnableSet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    GT_BOOL                     bypassEnabled;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    bypassEnabled = (GT_BOOL)inArgs[1];

    /* call cpss api function */
    result = cpssDxChLpmLastLookupStagesBypassEnableSet(devNum, bypassEnabled);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChLpmLastLookupStagesBypassEnableGet function
* @endinternal
*
* @brief   Function Relevant mode : Low Level API modes
*         Get the enabling status of bypassing lookup stages 8-31.
*         bypassing the last lookup stages is recommended when IPv6 routing is not
*         used in the system, to decrease the latency caused by the LPM engine.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
static CMD_STATUS wrCpssDxChLpmLastLookupStagesBypassEnableGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                   result;
    GT_U8                       devNum;
    GT_BOOL                     bypassEnabled;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    result = cpssDxChLpmLastLookupStagesBypassEnableGet(devNum, &bypassEnabled);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d", bypassEnabled);
    return CMD_OK;
}

/**
* @internal wrCpssDxChLpmPortSipLookupEnableSet function
* @endinternal
*
* @brief   Enable/Disable SIP lookup for packets received from the given port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on out of range
* @retval GT_BAD_STATE             - on invalid hardware value set
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note When this attribute is set to "False", SIP Lookup is never performed for
*       the source ePort, regardless of other SIP related configurations
*       (i.e SIP/SA check, SIP filtering etc')
*
*/
static CMD_STATUS wrCpssDxChLpmPortSipLookupEnableSet
(
 IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
 IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
 IN  GT_32 numFields,
 OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   status;

    GT_U8                        devNum;
    GT_PORT_NUM                  portNum;
    GT_BOOL                      enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

   /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum      = (GT_U8)inArgs[0];
    portNum     = (GT_PORT_NUM)inArgs[1];
    enable     = (GT_BOOL)inArgs[2];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    status =  cpssDxChLpmPortSipLookupEnableSet(devNum,portNum, enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "");

    return CMD_OK;
}

/**
* @internal wrCpssDxChLpmPortSipLookupEnableGet function
* @endinternal
*
* @brief   Return the SIP Lookup status for packets received from the given port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on out of range
* @retval GT_BAD_STATE             - on invalid hardware value read
* @retval GT_BAD_PARAM             - on wrong device or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
* @note When this attribute is set to "False", SIP Lookup is never performed for
*       the source ePort, regardless of other SIP related configurations
*       (i.e SIP/SA check, SIP filtering etc')
*
*/
static CMD_STATUS wrCpssDxChLpmPortSipLookupEnableGet
(
 IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
 IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
 IN  GT_32 numFields,
 OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   status;

    GT_U8                        devNum;
    GT_PORT_NUM                  portNum;
    GT_BOOL                      enable;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum      = (GT_U8)inArgs[0];
    portNum     = (GT_PORT_NUM)inArgs[1];

    /* Override Device and Port */
    CONVERT_DEV_PORT_U32_MAC(devNum, portNum);

    /* call cpss api function */
    status =  cpssDxChLpmPortSipLookupEnableGet(devNum,portNum, &enable);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", enable);

    return CMD_OK;
}


/**
* @internal wrCpssDxChLpmLeafEntryWrite function
* @endinternal
*
* @brief   Write an LPM leaf entry, which is used for policy based routing, to the
*         HW
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note The PBR base address is set in cpssDxChCfgPpLogicalInit according to the
*       maxNumOfPbrEntries parameter in CPSS_DXCH_PP_CONFIG_INIT_STC.
*       See the datasheet for maximal leafIndex.
*
*/
static CMD_STATUS wrCpssDxChLpmLeafEntryWrite
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_U32                          leafIndex;
    CPSS_DXCH_LPM_LEAF_ENTRY_STC    leaf;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    leafIndex = (GT_U32)inArgs[1];
    leaf.entryType = (CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT)inArgs[2];
    leaf.index = (GT_U32)inArgs[3];
    leaf.ucRPFCheckEnable = (GT_BOOL)inArgs[4];
    leaf.sipSaCheckMismatchEnable = (GT_BOOL)inArgs[5];
    leaf.ipv6MCGroupScopeLevel = (CPSS_IPV6_PREFIX_SCOPE_ENT)inArgs[6];
    leaf.priority = (CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT)inArgs[7];

    /* call cpss api function */
    result = cpssDxChLpmLeafEntryWrite(devNum, leafIndex, &leaf);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "");
    return CMD_OK;
}

/**
* @internal wrCpssDxChLpmLeafEntryRead function
* @endinternal
*
* @brief   Read an LPM leaf entry, which is used for policy based routing, from the
*         HW
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong input parameter
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note The PBR base address is set in cpssDxChCfgPpLogicalInit according to the
*       maxNumOfPbrEntries parameter in CPSS_DXCH_PP_CONFIG_INIT_STC.
*       See the datasheet for maximal leafIndex.
*
*/
static CMD_STATUS wrCpssDxChLpmLeafEntryRead
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS                       result;
    GT_U8                           devNum;
    GT_U32                          leafIndex;
    CPSS_DXCH_LPM_LEAF_ENTRY_STC    leaf;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    /* map input arguments to locals */
    devNum = (GT_U8)inArgs[0];
    leafIndex = (GT_U32)inArgs[1];

    /* call cpss api function */
    result = cpssDxChLpmLeafEntryRead(devNum, leafIndex, &leaf);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, result, "%d%d%d%d%d%d", leaf.entryType, leaf.index,
                 leaf.ucRPFCheckEnable, leaf.sipSaCheckMismatchEnable,leaf.ipv6MCGroupScopeLevel,leaf.priority);
    return CMD_OK;
}

/**
* @internal wrCpssDxChLpmPbrBaseAndSizeGet function
* @endinternal
*
* @brief   Return the base address and size of the policy based routing area within
*         the LPM memory
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
static CMD_STATUS wrCpssDxChLpmPbrBaseAndSizeGet
(
    IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
    IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
    IN  GT_32 numFields,
    OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   status;
    GT_U8       devNum;
    GT_U32      baseOffset;
    GT_U32      size;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum = (GT_U8)inArgs[0];

    /* call cpss api function */
    status = cpssDxChLpmPbrBaseAndSizeGet(devNum, &baseOffset, &size);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d%d", baseOffset, size);

    return CMD_OK;
}

/**
* @internal wrCpssDxChLpmExceptionStatusGet function
* @endinternal
*
* @brief   Return the Lpm Exception Status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_STATE             - wrong value returned from hw
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
static CMD_STATUS wrCpssDxChLpmExceptionStatusGet
(
 IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
 IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
 IN  GT_32 numFields,
 OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   status;

    GT_U8                               devNum;
    CPSS_DXCH_LPM_EXCEPTION_STATUS_ENT  hitException;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum      = (GT_U8)inArgs[0];

    /* call cpss api function */
    status =  cpssDxChLpmExceptionStatusGet(devNum,0,&hitException);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", hitException);

    return CMD_OK;
}


/**
* @internal wrCpssDxChLpmMemoryModeGet function
* @endinternal
*
* @brief   This function get LPM memory mode
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; xCat2; Bobcat2; Caelum; Aldrin;DxCh2; DxCh3; xCat; xCat3; Lion; Lion2.
*
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_NOT_SUPPORTED         - in case DFX unit is not supported
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/

static CMD_STATUS wrCpssDxChLpmMemoryModeGet
(
 IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
 IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
 IN  GT_32 numFields,
 OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   status;

    GT_U8                               devNum;
    CPSS_DXCH_LPM_RAM_MEM_MODE_ENT    lpmMemoryMode;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum      = (GT_U8)inArgs[0];

    /* call cpss api function */
    status =  cpssDxChLpmMemoryModeGet(devNum, &lpmMemoryMode);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", lpmMemoryMode);

    return CMD_OK;
}

/**
* @internal wrCpssDxChLpmExceptionStatusGet_1 function
* @endinternal
*
* @brief   Return the Lpm Exception Status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2; Lion; Lion2.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_STATE             - wrong value returned from hw
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
static CMD_STATUS wrCpssDxChLpmExceptionStatusGet_1
(
 IN  GT_UINTPTR inArgs[CMD_MAX_ARGS],
 IN  GT_UINTPTR inFields[CMD_MAX_FIELDS],
 IN  GT_32 numFields,
 OUT GT_8  outArgs[CMD_MAX_BUFFER]
)
{
    GT_STATUS   status;

    GT_U8                               devNum;
    GT_U32                              lpmEngine;
    CPSS_DXCH_LPM_EXCEPTION_STATUS_ENT  hitException;

    GT_UNUSED_PARAM(numFields);
    GT_UNUSED_PARAM(inFields);

    /* check for valid arguments */
    if(!inArgs || !outArgs)
        return CMD_AGENT_ERROR;

    devNum      = (GT_U8)inArgs[0];
    lpmEngine      = (GT_U32)inArgs[1];

    /* call cpss api function */
    status =  cpssDxChLpmExceptionStatusGet(devNum,lpmEngine,&hitException);

    /* pack output arguments to galtis string */
    galtisOutput(outArgs, status, "%d", hitException);

    return CMD_OK;
}



/**** database initialization **************************************/

static CMD_COMMAND dbCommands[] =
{
    {"cpssDxChLpmNodeSetFirst",
    &wrCpssDxChLpmNodeSetFirst,
    34, 9},

    {"cpssDxChLpmNodeSetNext",
    &wrCpssDxChLpmNodeSetNext,
    34, 9},

    {"cpssDxChLpmNodeEndSet",
    &wrCpssDxChLpmNodeEndSet,
    34, 0},

    {"cpssDxChLpmNodeGetFirst",
    &wrCpssDxChLpmNodeGetFirst,
    3, 0},

    {"cpssDxChLpmNodeGetNext",
    &wrCpssDxChLpmNodeGetNext,
    3, 0},

    {"cpssDxChLpmNodeNextPointersSetFirst",
    &wrCpssDxChLpmNodeNextPointersSetFirst,
    3, 9},

    {"cpssDxChLpmNodeNextPointersSetNext",
    &wrCpssDxChLpmNodeNextPointersSetNext,
    3, 9},

    {"cpssDxChLpmNodeNextPointersEndSet",
    &wrCpssDxChLpmNodeNextPointersEndSet,
    3, 0},

    {"cpssDxChLpmLastLookupStagesBypassEnableSet",
    &wrCpssDxChLpmLastLookupStagesBypassEnableSet,
    2, 0},

    {"cpssDxChLpmLastLookupStagesBypassEnableGet",
    &wrCpssDxChLpmLastLookupStagesBypassEnableGet,
    1, 0},

    {"cpssDxChLpmPortSipLookupEnableSet",
    &wrCpssDxChLpmPortSipLookupEnableSet,
    3, 0},

    {"cpssDxChLpmPortSipLookupEnableGet",
    &wrCpssDxChLpmPortSipLookupEnableGet,
    2, 0},

    {"cpssDxChLpmLeafEntryWrite",
    &wrCpssDxChLpmLeafEntryWrite,
    8, 0},

    {"cpssDxChLpmLeafEntryRead",
    &wrCpssDxChLpmLeafEntryRead,
    2, 0},

    {"cpssDxChLpmPbrBaseAndSizeGet",
    &wrCpssDxChLpmPbrBaseAndSizeGet,
    1, 0},

    {"cpssDxChLpmExceptionStatusGet",
    &wrCpssDxChLpmExceptionStatusGet,
    1, 0},
    {"cpssDxChLpmExceptionStatusGet_1",
    &wrCpssDxChLpmExceptionStatusGet_1,
    2, 0},

    {"cpssDxChLpmMemoryModeGet",
    &wrCpssDxChLpmMemoryModeGet,
    1, 0}
};

#define numCommands (sizeof(dbCommands) / sizeof(CMD_COMMAND))

/**
* @internal cmdLibInitCpssDxChLpm function
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
GT_STATUS cmdLibInitCpssDxChLpm
(
    GT_VOID
)
{
    return cmdInitLibrary(dbCommands, numCommands);
}


