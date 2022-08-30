/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChLpmLog.c
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
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChConfigLog.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpm.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpmTypes.h>
#include <cpss/dxCh/dxChxGen/lpm/private/prvCpssDxChLpmLog.h>
#include <cpss/generic/ip/private/prvCpssGenIpLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_LPM_EXCEPTION_STATUS_ENT[]  =
{
    "CPSS_DXCH_LPM_EXCEPTION_HIT_E",
    "CPSS_DXCH_LPM_EXCEPTION_LPM_ECC_E",
    "CPSS_DXCH_LPM_EXCEPTION_ECMP_ECC_E",
    "CPSS_DXCH_LPM_EXCEPTION_PBR_BUCKET_E",
    "CPSS_DXCH_LPM_EXCEPTION_CONTINUE_TO_LOOKUP_E",
    "CPSS_DXCH_LPM_EXCEPTION_UNICAST_LOOKUP_0_E",
    "CPSS_DXCH_LPM_EXCEPTION_UNICAST_LOOKUP_1_E",
    "CPSS_DXCH_LPM_EXCEPTION_DST_G_IPV4_PACKETS_E",
    "CPSS_DXCH_LPM_EXCEPTION_SRC_G_IPV4_PACKETS_E",
    "CPSS_DXCH_LPM_EXCEPTION_UNICAST_LOOKUP_0_IPV6_PACKETS_E",
    "CPSS_DXCH_LPM_EXCEPTION_UNICAST_LOOKUP_1_IPV6_PACKETS_E",
    "CPSS_DXCH_LPM_EXCEPTION_DST_G_IPV6_PACKETS_E",
    "CPSS_DXCH_LPM_EXCEPTION_SRC_G_IPV6_PACKETS_E",
    "CPSS_DXCH_LPM_EXCEPTION_FCOE_D_ID_LOOKUP_E",
    "CPSS_DXCH_LPM_EXCEPTION_FCOE_S_ID_LOOKUP_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_LPM_EXCEPTION_STATUS_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT[]  =
{
    "CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E",
    "CPSS_DXCH_LPM_COMPRESSED_1_NODE_PTR_TYPE_E",
    "CPSS_DXCH_LPM_COMPRESSED_2_NODE_PTR_TYPE_E",
    "CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E",
    "CPSS_DXCH_LPM_ECMP_ENTRY_PTR_TYPE_E",
    "CPSS_DXCH_LPM_QOS_ENTRY_PTR_TYPE_E",
    "CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E",
    "CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E",
    "CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E",
    "CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E",
    "CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT[]  =
{
    "CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_FDB_E",
    "CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_LPM_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT[]  =
{
    "CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E",
    "CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITH_BLOCK_SHARING_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_LPM_LEAF_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_LPM_LEAF_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, entryType, CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, index);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ucRPFCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, sipSaCheckMismatchEnable);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ipv6MCGroupScopeLevel, CPSS_IPV6_PREFIX_SCOPE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, priority, CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, applyPbr);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_LPM_COMPRESSED_1_BIT_VECTOR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_LPM_COMPRESSED_1_BIT_VECTOR_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, ranges1_4, 4, GT_U8);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_LPM_COMPRESSED_2_BIT_VECTOR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_LPM_COMPRESSED_2_BIT_VECTOR_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, ranges1_4, 4, GT_U8);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, ranges6_9, 4, GT_U8);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_LPM_NODE_POINTER_DATA_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_LPM_NODE_POINTER_DATA_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, nextNodePointer, CPSS_DXCH_LPM_NEXT_NODE_POINTER_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, nextHopOrEcmpPointer, CPSS_DXCH_LPM_NODE_NEXT_HOP_OR_ECMP_POINTER_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_LPM_RAM_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_LPM_RAM_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfBlocks);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, blocksSizeArray, CPSS_DXCH_SIP6_LPM_RAM_NUM_OF_MEMORIES_CNS, GT_U32);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, blocksAllocationMethod, CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, lpmMemMode, CPSS_DXCH_LPM_RAM_MEM_MODE_ENT);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, lpmRamConfigInfo, CPSS_DXCH_CFG_NUM_OF_DEV_TYPES_MANAGED_CNS, CPSS_DXCH_LPM_RAM_CONFIG_INFO_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, lpmRamConfigInfoNumOfElements);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxNumOfPbrEntries);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_LPM_REGULAR_BIT_VECTOR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_LPM_REGULAR_BIT_VECTOR_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, bitVectorEntry, 11, CPSS_DXCH_LPM_REGULAR_NODE_BIT_VECTOR_ENTRY_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_LPM_NEXT_NODE_POINTER_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_LPM_NEXT_NODE_POINTER_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nextPointer);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, range5Index);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, pointToSipTree);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_LPM_NODE_NEXT_HOP_OR_ECMP_POINTER_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_LPM_NODE_NEXT_HOP_OR_ECMP_POINTER_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ucRpfCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, srcAddrCheckMismatchEnable);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ipv6McGroupScopeLevel, CPSS_IPV6_PREFIX_SCOPE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, activityState);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryIndex);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_LPM_REGULAR_NODE_BIT_VECTOR_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_LPM_REGULAR_NODE_BIT_VECTOR_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, bitMap);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, rangeCounter);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_LPM_EXCEPTION_STATUS_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_LPM_EXCEPTION_STATUS_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_LPM_EXCEPTION_STATUS_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_LPM_LEAF_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_LPM_LEAF_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_LPM_LEAF_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LPM_LEAF_ENTRY_STC_PTR_leafPtr = {
     "leafPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_LPM_LEAF_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT_nodeType = {
     "nodeType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC_PTR_nextPointerArrayPtr = {
     "nextPointerArrayPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT_PTR_rangeSelectSecPtr = {
     "rangeSelectSecPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_bypassEnabled = {
     "bypassEnabled", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_leafIndex = {
     "leafIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lpmEngine = {
     "lpmEngine", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_lpmLineOffset = {
     "lpmLineOffset", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfNextPointers = {
     "numOfNextPointers", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfRanges = {
     "numOfRanges", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_LPM_EXCEPTION_STATUS_ENT_PTR_hitExceptionPtr = {
     "hitExceptionPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_LPM_EXCEPTION_STATUS_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_LPM_LEAF_ENTRY_STC_PTR_leafPtr = {
     "leafPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_LPM_LEAF_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC_PTR_nextPointerArrayPtr = {
     "nextPointerArrayPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT_PTR_rangeSelectSecPtr = {
     "rangeSelectSecPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_LPM_RAM_MEM_MODE_ENT_PTR_lpmMemoryModePtr = {
     "lpmMemoryModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_LPM_RAM_MEM_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_bypassEnabledPtr = {
     "bypassEnabledPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_baseOffsetPtr = {
     "baseOffsetPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numOfRangesPtr = {
     "numOfRangesPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLpmLastLookupStagesBypassEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_bypassEnabled
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLpmLeafEntryWrite_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_leafIndex,
    &DX_IN_CPSS_DXCH_LPM_LEAF_ENTRY_STC_PTR_leafPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLpmLeafEntryRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_leafIndex,
    &DX_OUT_CPSS_DXCH_LPM_LEAF_ENTRY_STC_PTR_leafPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLpmExceptionStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_lpmEngine,
    &DX_OUT_CPSS_DXCH_LPM_EXCEPTION_STATUS_ENT_PTR_hitExceptionPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLpmNodeWrite_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_lpmLineOffset,
    &DX_IN_CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT_nodeType,
    &DX_IN_CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT_PTR_rangeSelectSecPtr,
    &DX_IN_GT_U32_numOfRanges,
    &DX_IN_CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC_PTR_nextPointerArrayPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLpmNodeRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_lpmLineOffset,
    &DX_IN_CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT_nodeType,
    &DX_OUT_CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT_PTR_rangeSelectSecPtr,
    &DX_OUT_GT_U32_PTR_numOfRangesPtr,
    &DX_OUT_CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC_PTR_nextPointerArrayPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLpmNodeNextPointersWrite_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_lpmLineOffset,
    &DX_IN_GT_U32_numOfNextPointers,
    &DX_IN_CPSS_DXCH_LPM_NODE_NEXT_POINTER_STC_PTR_nextPointerArrayPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLpmMemoryModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_LPM_RAM_MEM_MODE_ENT_PTR_lpmMemoryModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLpmLastLookupStagesBypassEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_BOOL_PTR_bypassEnabledPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChLpmPbrBaseAndSizeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_baseOffsetPtr,
    &DX_OUT_GT_U32_PTR_sizePtr
};


/********* lib API DB *********/

extern void cpssDxChLpmNodeWrite_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChLpmNodeRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChLpmNodeNextPointersWrite_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChLpmLogLibDb[] = {
    {"cpssDxChLpmNodeWrite", 6, cpssDxChLpmNodeWrite_PARAMS, cpssDxChLpmNodeWrite_preLogic},
    {"cpssDxChLpmNodeRead", 6, cpssDxChLpmNodeRead_PARAMS, cpssDxChLpmNodeRead_preLogic},
    {"cpssDxChLpmNodeNextPointersWrite", 4, cpssDxChLpmNodeNextPointersWrite_PARAMS, cpssDxChLpmNodeNextPointersWrite_preLogic},
    {"cpssDxChLpmLastLookupStagesBypassEnableSet", 2, cpssDxChLpmLastLookupStagesBypassEnableSet_PARAMS, NULL},
    {"cpssDxChLpmLastLookupStagesBypassEnableGet", 2, cpssDxChLpmLastLookupStagesBypassEnableGet_PARAMS, NULL},
    {"cpssDxChLpmPortSipLookupEnableSet", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChLpmPortSipLookupEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChLpmLeafEntryWrite", 3, cpssDxChLpmLeafEntryWrite_PARAMS, NULL},
    {"cpssDxChLpmLeafEntryRead", 3, cpssDxChLpmLeafEntryRead_PARAMS, NULL},
    {"cpssDxChLpmPbrBaseAndSizeGet", 3, cpssDxChLpmPbrBaseAndSizeGet_PARAMS, NULL},
    {"cpssDxChLpmExceptionStatusGet", 3, cpssDxChLpmExceptionStatusGet_PARAMS, NULL},
    {"cpssDxChLpmMemoryModeGet", 2, cpssDxChLpmMemoryModeGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_LPM(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChLpmLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChLpmLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

