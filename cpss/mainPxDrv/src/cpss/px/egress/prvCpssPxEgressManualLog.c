/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxEgressManualLog.c
*       Manually implemented CPSS Log type wrappers
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/generic/log/prvCpssPxGenDbLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/px/egress/cpssPxEgress.h>
#include <cpss/px/egress/private/prvCpssPxEgressLog.h>

/********* structure fields log functions *********/
void prvCpssLogParamFuncStc_CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   operationType;
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT *, valPtr);
    operationType = (CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT)inOutParamInfoPtr->paramKey.paramKeyArr[4];

    if(inOutParamInfoPtr->paramKey.paramKeyArr[5] == 0)
    {
        operationType = (CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT)inOutParamInfoPtr->paramKey.paramKeyArr[4];
    }
    else
    {
        if(NULL != (CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT *)inOutParamInfoPtr->paramKey.paramKeyArr[4])
        {
            operationType = *(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT *)inOutParamInfoPtr->paramKey.paramKeyArr[4];
        }
        else
        {
            prvCpssLogStcLogEnd(contextLib, logType);
            return;
        }
    }

    switch(operationType)
    {
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_E:
            PRV_CPSS_LOG_STC_STC_MAC(valPtr,info_802_1br_E2U, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC);
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E:
            PRV_CPSS_LOG_STC_STC_MAC(valPtr, info_dsa_ET2U, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC);
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E:
            PRV_CPSS_LOG_STC_STC_MAC(valPtr, info_dsa_EU2U, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC);
            break;
        case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_E:
            PRV_CPSS_LOG_STC_STC_MAC(valPtr, info_dsa_QCN, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC);
            break;
        default:
            break;
    }

    prvCpssLogStcLogEnd(contextLib, logType);
}

void prvCpssLogParamFuncStc_CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT   entryType;
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT *, valPtr);
    entryType = (CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT)inOutParamInfoPtr->paramKey.paramKeyArr[4];

    if(inOutParamInfoPtr->paramKey.paramKeyArr[5] == 0)
    {
        entryType = (CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT)inOutParamInfoPtr->paramKey.paramKeyArr[4];
    }
    else
    {
        if(NULL != (CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT *)inOutParamInfoPtr->paramKey.paramKeyArr[4])
        {
            entryType = *(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT *)inOutParamInfoPtr->paramKey.paramKeyArr[4];
        }
        else
        {
            prvCpssLogStcLogEnd(contextLib, logType);
            return;
        }
    }

    switch(entryType)
    {
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_802_1BR_E:
            PRV_CPSS_LOG_STC_STC_MAC(valPtr, info_802_1br, CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC);
            break;
        case CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_DSA_E:
            PRV_CPSS_LOG_STC_STC_MAC(valPtr, info_dsa, CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC);
            break;
        default:
            break;
    }

    prvCpssLogStcLogEnd(contextLib, logType);
}

void prvCpssLogParamFuncStc_CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT   entryType;
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT *, valPtr);
    entryType = (CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT)inOutParamInfoPtr->paramKey.paramKeyArr[4];

    if(inOutParamInfoPtr->paramKey.paramKeyArr[5] == 0)
    {
        entryType = (CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT)inOutParamInfoPtr->paramKey.paramKeyArr[4];
    }
    else
    {
        if(NULL != (CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT *)inOutParamInfoPtr->paramKey.paramKeyArr[4])
        {
            entryType = *(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT *)inOutParamInfoPtr->paramKey.paramKeyArr[4];
        }
        else
        {
            prvCpssLogStcLogEnd(contextLib, logType);
            return;
        }
    }

    switch(entryType)
    {
        case CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_COMMON_E:
            PRV_CPSS_LOG_STC_STC_MAC(valPtr, info_common, CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC);
            break;
        default:
            break;
    }

    prvCpssLogStcLogEnd(contextLib, logType);
}

/********* api pre-log functions *********/
void cpssPxEgressHeaderAlterationEntrySet_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    /* devNum */
    va_arg(args, GT_U32);
    /* portNum */
    va_arg(args, GT_U32);
    /* packetType */
    va_arg(args, GT_U32);
    /* operationType */
    paramDataPtr->paramKey.paramKeyArr[4] = (GT_UINTPTR)va_arg(args, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT);
    paramDataPtr->paramKey.paramKeyArr[5] = 0;
}

void cpssPxEgressHeaderAlterationEntryGet_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    /* devNum */
    va_arg(args, GT_U32);
    /* portNum */
    va_arg(args, GT_U32);
    /* packetType */
    va_arg(args, GT_U32);
    /* operationType */
    paramDataPtr->paramKey.paramKeyArr[4] = (GT_UINTPTR)va_arg(args, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT *);
    paramDataPtr->paramKey.paramKeyArr[5] = 1;
}

void cpssPxEgressSourcePortEntrySet_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    /* devNum */
    va_arg(args, GT_U32);
    /* portNum */
    va_arg(args, GT_U32);
    /* infoType */
    paramDataPtr->paramKey.paramKeyArr[4] = (GT_UINTPTR)va_arg(args, CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT);
    paramDataPtr->paramKey.paramKeyArr[5] = 0;
}

void cpssPxEgressSourcePortEntryGet_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    /* devNum */
    va_arg(args, GT_U32);
    /* portNum */
    va_arg(args, GT_U32);
    /* infoType */
    paramDataPtr->paramKey.paramKeyArr[4] = (GT_UINTPTR)va_arg(args, CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT *);
    paramDataPtr->paramKey.paramKeyArr[5] = 1;
}

void cpssPxEgressTargetPortEntrySet_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    /* devNum */
    va_arg(args, GT_U32);
    /* portNum */
    va_arg(args, GT_U32);
    /* infoType */
    paramDataPtr->paramKey.paramKeyArr[4] = (GT_UINTPTR)va_arg(args, CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT);
    paramDataPtr->paramKey.paramKeyArr[5] = 0;
}

void cpssPxEgressTargetPortEntryGet_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    /* devNum */
    va_arg(args, GT_U32);
    /* portNum */
    va_arg(args, GT_U32);
    /* infoType */
    paramDataPtr->paramKey.paramKeyArr[4] = (GT_UINTPTR)va_arg(args, CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT *);
    paramDataPtr->paramKey.paramKeyArr[5] = 1;
}
