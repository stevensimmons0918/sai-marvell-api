/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxCosLog.c
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

/* disable deprecation warnings (if one) */
#ifdef __GNUC__
#if  (__GNUC__*100+__GNUC_MINOR__) >= 406
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#endif

#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssPxGenDbLog.h>
#include <cpss/px/cos/cpssPxCos.h>
#include <cpss/px/cos/private/prvCpssPxCosLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_PX_COS_MODE_ENT[]  =
{
    "CPSS_PX_COS_MODE_PORT_E",
    "CPSS_PX_COS_MODE_PACKET_DSA_E",
    "CPSS_PX_COS_MODE_PACKET_L2_E",
    "CPSS_PX_COS_MODE_PACKET_L3_E",
    "CPSS_PX_COS_MODE_PACKET_MPLS_E",
    "CPSS_PX_COS_MODE_FORMAT_ENTRY_E",
    "CPSS_PX_COS_MODE_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_COS_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_PX_COS_ATTRIBUTES_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_COS_ATTRIBUTES_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, trafficClass);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, dropPrecedence, CPSS_DP_LEVEL_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, userPriority);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dropEligibilityIndication);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_COS_FORMAT_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_COS_FORMAT_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, cosMode, CPSS_PX_COS_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cosByteOffset);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cosBitOffset);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cosNumOfBits);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, cosAttributes, CPSS_PX_COS_ATTRIBUTES_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_PX_COS_ATTRIBUTES_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_COS_ATTRIBUTES_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_COS_ATTRIBUTES_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_COS_FORMAT_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_COS_FORMAT_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_COS_FORMAT_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_COS_ATTRIBUTES_STC_PTR_cosAttributesPtr = {
     "cosAttributesPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_COS_ATTRIBUTES_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_COS_FORMAT_ENTRY_STC_PTR_cosFormatEntryPtr = {
     "cosFormatEntryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_COS_FORMAT_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_dsaIndex = {
     "dsaIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_l2Index = {
     "l2Index", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_l3Index = {
     "l3Index", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_mplsIndex = {
     "mplsIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_COS_ATTRIBUTES_STC_PTR_cosAttributesPtr = {
     "cosAttributesPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_COS_ATTRIBUTES_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_COS_FORMAT_ENTRY_STC_PTR_cosFormatEntryPtr = {
     "cosFormatEntryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_COS_FORMAT_ENTRY_STC)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCosFormatEntrySet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PACKET_TYPE_packetType,
    &PX_IN_CPSS_PX_COS_FORMAT_ENTRY_STC_PTR_cosFormatEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCosFormatEntryGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PACKET_TYPE_packetType,
    &PX_OUT_CPSS_PX_COS_FORMAT_ENTRY_STC_PTR_cosFormatEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCosPortAttributesSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_COS_ATTRIBUTES_STC_PTR_cosAttributesPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCosPortL2MappingSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_l2Index,
    &PX_IN_CPSS_PX_COS_ATTRIBUTES_STC_PTR_cosAttributesPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCosPortL2MappingGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_l2Index,
    &PX_OUT_CPSS_PX_COS_ATTRIBUTES_STC_PTR_cosAttributesPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCosPortAttributesGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_COS_ATTRIBUTES_STC_PTR_cosAttributesPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCosDsaMappingSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_dsaIndex,
    &PX_IN_CPSS_PX_COS_ATTRIBUTES_STC_PTR_cosAttributesPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCosDsaMappingGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_dsaIndex,
    &PX_OUT_CPSS_PX_COS_ATTRIBUTES_STC_PTR_cosAttributesPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCosL3MappingSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_l3Index,
    &PX_IN_CPSS_PX_COS_ATTRIBUTES_STC_PTR_cosAttributesPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCosL3MappingGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_l3Index,
    &PX_OUT_CPSS_PX_COS_ATTRIBUTES_STC_PTR_cosAttributesPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCosMplsMappingSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_mplsIndex,
    &PX_IN_CPSS_PX_COS_ATTRIBUTES_STC_PTR_cosAttributesPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxCosMplsMappingGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_mplsIndex,
    &PX_OUT_CPSS_PX_COS_ATTRIBUTES_STC_PTR_cosAttributesPtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssPxCosLogLibDb[] = {
    {"cpssPxCosFormatEntrySet", 3, cpssPxCosFormatEntrySet_PARAMS, NULL},
    {"cpssPxCosFormatEntryGet", 3, cpssPxCosFormatEntryGet_PARAMS, NULL},
    {"cpssPxCosPortAttributesSet", 3, cpssPxCosPortAttributesSet_PARAMS, NULL},
    {"cpssPxCosPortAttributesGet", 3, cpssPxCosPortAttributesGet_PARAMS, NULL},
    {"cpssPxCosPortL2MappingSet", 4, cpssPxCosPortL2MappingSet_PARAMS, NULL},
    {"cpssPxCosPortL2MappingGet", 4, cpssPxCosPortL2MappingGet_PARAMS, NULL},
    {"cpssPxCosL3MappingSet", 3, cpssPxCosL3MappingSet_PARAMS, NULL},
    {"cpssPxCosL3MappingGet", 3, cpssPxCosL3MappingGet_PARAMS, NULL},
    {"cpssPxCosMplsMappingSet", 3, cpssPxCosMplsMappingSet_PARAMS, NULL},
    {"cpssPxCosMplsMappingGet", 3, cpssPxCosMplsMappingGet_PARAMS, NULL},
    {"cpssPxCosDsaMappingSet", 3, cpssPxCosDsaMappingSet_PARAMS, NULL},
    {"cpssPxCosDsaMappingGet", 3, cpssPxCosDsaMappingGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_PX_CPSS_LOG_LIB_COS(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssPxCosLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssPxCosLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

