/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChCscdLog.c
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
#include <cpss/common/config/private/prvCpssCommonConfigLog.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/cscd/private/prvCpssDxChCscdLog.h>
#include <cpss/dxCh/dxChxGen/log/private/prvCpssDxChLog.h>
#include <cpss/dxCh/dxChxGen/networkIf/private/prvCpssDxChNetworkIfLog.h>
#include <cpss/generic/cscd/private/prvCpssGenCscdLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>
#include <cpss/generic/port/private/prvCpssGenPortLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT[]  =
{
    "CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DISABLED_E",
    "CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_INTERLAKEN_E",
    "CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DSA_E",
    "CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_E_TAG_E",
    "CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_VLAN_TAG_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT[]  =
{
    "CPSS_DXCH_CSCD_PORT_CRC_ONE_BYTE_E",
    "CPSS_DXCH_CSCD_PORT_CRC_TWO_BYTES_E",
    "CPSS_DXCH_CSCD_PORT_CRC_THREE_BYTES_E",
    "CPSS_DXCH_CSCD_PORT_CRC_FOUR_BYTES_E",
    "CPSS_DXCH_CSCD_PORT_CRC_ZERO_BYTES_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT[]  =
{
    "CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_GLOBAL_E",
    "CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_EXTENDED_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_CSCD_QOS_PRIO_TABLE_ACCESS_MODE_ENT[]  =
{
    "CPSS_DXCH_CSCD_QOS_2B_SRC_2B_TRG_E",
    "CPSS_DXCH_CSCD_QOS_4B_TRG_E",
    "CPSS_DXCH_CSCD_QOS_1B_SRC_3B_TRG_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CSCD_QOS_PRIO_TABLE_ACCESS_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT[]  =
{
    "CPSS_DXCH_CSCD_QOS_REMAP_DISABLED_E",
    "CPSS_DXCH_CSCD_QOS_REMAP_CNTRL_ONLY_E",
    "CPSS_DXCH_CSCD_QOS_REMAP_DATA_ONLY_E",
    "CPSS_DXCH_CSCD_QOS_REMAP_ALL_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT[]  =
{
    "CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_INGR_PIPE_HASH_E",
    "CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_SRC_PORT_E",
    "CPSS_DXCH_CSCD_TRUNK_LINK_HASH_IS_DST_PORT_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT[]  =
{
    "CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_E",
    "CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_TRG_PORT_E",
    "CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_DEV_E",
    "CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_SRC_PORT_E",
    "CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_1BIT_SRC_DEV_SRC_PORT_E",
    "CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_2BITS_SRC_DEV_SRC_PORT_E",
    "CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_3BITS_SRC_DEV_SRC_PORT_E",
    "CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_4BITS_SRC_DEV_SRC_PORT_E",
    "CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_5BITS_SRC_DEV_SRC_PORT_E",
    "CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_PORT_E",
    "CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_PHYSICAL_SRC_PORT_E",
    "CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_LSB_HASH_E",
    "CPSS_DXCH_DEV_MAP_LOOKUP_MODE_TRG_DEV_MSB_HASH_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tc);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, dp, CPSS_DP_LEVEL_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, isStack, CPSS_DXCH_PORT_PROFILE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, dsaTagCmd, CPSS_DXCH_NET_DSA_CMD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, targetPortTcProfile, CPSS_DXCH_PORT_PROFILE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, packetIsMultiDestination);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_CSCD_QOS_TC_REMAP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_CSCD_QOS_TC_REMAP_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, forwardLocalTc);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, forwardStackTc);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, toAnalyzerLocalTc);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, toAnalyzerStackTc);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, toCpuLocalTc);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, toCpuStackTc);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, fromCpuLocalTc);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, fromCpuStackTc);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CSCD_QOS_PRIO_TABLE_ACCESS_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CSCD_QOS_PRIO_TABLE_ACCESS_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_CSCD_QOS_PRIO_TABLE_ACCESS_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CSCD_QOS_PRIO_TABLE_ACCESS_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CSCD_QOS_PRIO_TABLE_ACCESS_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_CSCD_QOS_PRIO_TABLE_ACCESS_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_CSCD_QOS_TC_REMAP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_CSCD_QOS_TC_REMAP_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_CSCD_QOS_TC_REMAP_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_CSCD_LINK_TYPE_STC_PTR_cascadeLinkPtr = {
     "cascadeLinkPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_CSCD_LINK_TYPE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_CSCD_PORT_TYPE_ENT_portType = {
     "portType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_CSCD_PORT_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DIRECTION_ENT_portDirection = {
     "portDirection", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DIRECTION_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DP_FOR_RX_ENT_dpForRx = {
     "dpForRx", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DP_FOR_RX_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DP_LEVEL_ENT_cpuToCpuDp = {
     "cpuToCpuDp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DP_LEVEL_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DP_LEVEL_ENT_ctrlDp = {
     "ctrlDp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DP_LEVEL_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DP_LEVEL_ENT_newDp = {
     "newDp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DP_LEVEL_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DP_LEVEL_ENT_remapDp = {
     "remapDp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DP_LEVEL_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT_assignmentMode = {
     "assignmentMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT_crcMode = {
     "crcMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT_portQosDsaMode = {
     "portQosDsaMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CSCD_QOS_PRIO_TABLE_ACCESS_MODE_ENT_prioMode = {
     "prioMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CSCD_QOS_PRIO_TABLE_ACCESS_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT_remapType = {
     "remapType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC_PTR_tcDpRemappingPtr = {
     "tcDpRemappingPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CSCD_QOS_TC_REMAP_STC_PTR_tcMappingsPtr = {
     "tcMappingsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CSCD_QOS_TC_REMAP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT_srcPortTrunkHashEn = {
     "srcPortTrunkHashEn", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PORT_PROFILE_ENT_portProfile = {
     "portProfile", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PORT_PROFILE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT_hwDevMode = {
     "hwDevMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PORT_DIRECTION_ENT_portDirection = {
     "portDirection", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PORT_DIRECTION_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_acceptSkipSaLookup = {
     "acceptSkipSaLookup", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_egressAttributesLocallyEn = {
     "egressAttributesLocallyEn", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_enableOwnDevFltr = {
     "enableOwnDevFltr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_isLooped = {
     "isLooped", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_preemptiveTc = {
     "preemptiveTc", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_HW_DEV_NUM_aggHwDevNum = {
     "aggHwDevNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_HW_DEV_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_HW_DEV_NUM_remotePhysicalHwDevNum = {
     "remotePhysicalHwDevNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_HW_DEV_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_HW_DEV_NUM_reservedHwDevNum = {
     "reservedHwDevNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_HW_DEV_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_HW_DEV_NUM_sourceHwDevNum = {
     "sourceHwDevNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_HW_DEV_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_HW_DEV_NUM_targetHwDevNum = {
     "targetHwDevNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_HW_DEV_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PHYSICAL_PORT_NUM_remotePhysicalPortNum = {
     "remotePhysicalPortNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_PHYSICAL_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PHYSICAL_PORT_NUM_secondaryTargetPort = {
     "secondaryTargetPort", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_PHYSICAL_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PORT_NUM_aggPortNum = {
     "aggPortNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PORT_NUM_portBaseEport = {
     "portBaseEport", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PORT_NUM_trunkBaseEport = {
     "trunkBaseEport", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_aggSrcId = {
     "aggSrcId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_hash = {
     "hash", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_newPfcTc = {
     "newPfcTc", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_newTc = {
     "newTc", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_physicalPortBase = {
     "physicalPortBase", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcDevLsbAmount = {
     "srcDevLsbAmount", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcId = {
     "srcId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcPortLsbAmount = {
     "srcPortLsbAmount", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_srcTrunkLsbAmount = {
     "srcTrunkLsbAmount", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_vlanTagTpid = {
     "vlanTagTpid", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_ctrlTc = {
     "ctrlTc", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_remapTc = {
     "remapTc", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_CSCD_LINK_TYPE_STC_PTR_cascadeLinkPtr = {
     "cascadeLinkPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_CSCD_LINK_TYPE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_CSCD_PORT_TYPE_ENT_PTR_portTypePtr = {
     "portTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_CSCD_PORT_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DP_FOR_RX_ENT_PTR_remappedDpForRxPtr = {
     "remappedDpForRxPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DP_FOR_RX_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DP_LEVEL_ENT_PTR_cpuToCpuDpPtr = {
     "cpuToCpuDpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DP_LEVEL_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DP_LEVEL_ENT_PTR_ctrlDpPtr = {
     "ctrlDpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DP_LEVEL_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DP_LEVEL_ENT_PTR_remapDpPtr = {
     "remapDpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DP_LEVEL_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DP_LEVEL_ENT_PTR_remappedDpPtr = {
     "remappedDpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DP_LEVEL_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT_PTR_assignmentModePtr = {
     "assignmentModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT_PTR_crcModePtr = {
     "crcModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT_PTR_portQosDsaTrustModePtr = {
     "portQosDsaTrustModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CSCD_QOS_PRIO_TABLE_ACCESS_MODE_ENT_PTR_prioModePtr = {
     "prioModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CSCD_QOS_PRIO_TABLE_ACCESS_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT_PTR_remapTypePtr = {
     "remapTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CSCD_QOS_TC_REMAP_STC_PTR_tcMappingsPtr = {
     "tcMappingsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CSCD_QOS_TC_REMAP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT_PTR_srcPortTrunkHashEnPtr = {
     "srcPortTrunkHashEnPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PORT_PROFILE_ENT_PTR_portProfilePtr = {
     "portProfilePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PORT_PROFILE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT_PTR_hwDevModePtr = {
     "hwDevModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT_PTR_remappedMcastPriorityPtr = {
     "remappedMcastPriorityPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_acceptSkipSaLookupPtr = {
     "acceptSkipSaLookupPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_egressAttributesLocallyEnPtr = {
     "egressAttributesLocallyEnPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_enableOwnDevFltrPtr = {
     "enableOwnDevFltrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_isLoopedPtr = {
     "isLoopedPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_preemptiveTcPtr = {
     "preemptiveTcPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_HW_DEV_NUM_PTR_aggHwDevNumPtr = {
     "aggHwDevNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_HW_DEV_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_HW_DEV_NUM_PTR_remotePhysicalHwDevNumPtr = {
     "remotePhysicalHwDevNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_HW_DEV_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_HW_DEV_NUM_PTR_reservedHwDevNumPtr = {
     "reservedHwDevNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_HW_DEV_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_PHYSICAL_PORT_NUM_PTR_remotePhysicalPortNumPtr = {
     "remotePhysicalPortNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_PHYSICAL_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_PHYSICAL_PORT_NUM_PTR_secondaryTargetPortPtr = {
     "secondaryTargetPortPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_PHYSICAL_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_PORT_NUM_PTR_aggPortNumPtr = {
     "aggPortNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_PORT_NUM_PTR_portBaseEportPtr = {
     "portBaseEportPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_PORT_NUM_PTR_trunkBaseEportPtr = {
     "trunkBaseEportPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_aggSrcIdPtr = {
     "aggSrcIdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_physicalPortBasePtr = {
     "physicalPortBasePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_remappedPfcTcPtr = {
     "remappedPfcTcPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_remappedTcPtr = {
     "remappedTcPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_srcDevLsbAmountPtr = {
     "srcDevLsbAmountPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_srcIdPtr = {
     "srcIdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_srcPortLsbAmountPtr = {
     "srcPortLsbAmountPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_srcTrunkLsbAmountPtr = {
     "srcTrunkLsbAmountPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_vlanTagTpidPtr = {
     "vlanTagTpidPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_ctrlTcPtr = {
     "ctrlTcPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_remapTcPtr = {
     "remapTcPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdDbRemoteHwDevNumModeSet_PARAMS[] =  {
    &DX_IN_GT_HW_DEV_NUM_hwDevNum,
    &DX_IN_CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT_hwDevMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdDbRemoteHwDevNumModeGet_PARAMS[] =  {
    &DX_IN_GT_HW_DEV_NUM_hwDevNum,
    &DX_OUT_CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT_PTR_hwDevModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdQosTcDpRemapTableAccessModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CSCD_QOS_PRIO_TABLE_ACCESS_MODE_ENT_prioMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdQosTcDpRemapTableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC_PTR_tcDpRemappingPtr,
    &DX_IN_GT_U32_newTc,
    &DX_IN_CPSS_DP_LEVEL_ENT_newDp,
    &DX_IN_GT_U32_newPfcTc,
    &DX_IN_CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT_mcastPriority,
    &DX_IN_CPSS_DP_FOR_RX_ENT_dpForRx,
    &DX_IN_GT_BOOL_preemptiveTc
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdQosTcDpRemapTableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC_PTR_tcDpRemappingPtr,
    &DX_OUT_GT_U32_PTR_remappedTcPtr,
    &DX_OUT_CPSS_DP_LEVEL_ENT_PTR_remappedDpPtr,
    &DX_OUT_GT_U32_PTR_remappedPfcTcPtr,
    &DX_OUT_CPSS_PORT_TX_DROP_MCAST_PRIORITY_ENT_PTR_remappedMcastPriorityPtr,
    &DX_OUT_CPSS_DP_FOR_RX_ENT_PTR_remappedDpForRxPtr,
    &DX_OUT_GT_BOOL_PTR_preemptiveTcPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdDevMapLookupModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdDsaSrcDevFilterSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_enableOwnDevFltr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdDevMapTableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_HW_DEV_NUM_targetHwDevNum,
    &DX_IN_GT_HW_DEV_NUM_sourceHwDevNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_GT_U32_hash,
    &DX_IN_CPSS_CSCD_LINK_TYPE_STC_PTR_cascadeLinkPtr,
    &DX_IN_CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT_srcPortTrunkHashEn,
    &DX_IN_GT_BOOL_egressAttributesLocallyEn
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdDevMapTableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_HW_DEV_NUM_targetHwDevNum,
    &DX_IN_GT_HW_DEV_NUM_sourceHwDevNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_GT_U32_hash,
    &DX_OUT_CPSS_CSCD_LINK_TYPE_STC_PTR_cascadeLinkPtr,
    &DX_OUT_CPSS_DXCH_CSCD_TRUNK_LINK_HASH_ENT_PTR_srcPortTrunkHashEnPtr,
    &DX_OUT_GT_BOOL_PTR_egressAttributesLocallyEnPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdPortLocalDevMapLookupEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DIRECTION_ENT_portDirection,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdPortLocalDevMapLookupEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DIRECTION_ENT_portDirection,
    &DX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT_assignmentMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdPortQosDsaModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT_portQosDsaMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdRemapQosModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT_remapType
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdPortTypeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_PORT_DIRECTION_ENT_portDirection,
    &DX_IN_CPSS_CSCD_PORT_TYPE_ENT_portType
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdHyperGPortCrcModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_PORT_DIRECTION_ENT_portDirection,
    &DX_IN_CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT_crcMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdPortTcProfiletSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_PORT_DIRECTION_ENT_portDirection,
    &DX_IN_CPSS_DXCH_PORT_PROFILE_ENT_portProfile
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdPortTypeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_PORT_DIRECTION_ENT_portDirection,
    &DX_OUT_CPSS_CSCD_PORT_TYPE_ENT_PTR_portTypePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdHyperGPortCrcModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_PORT_DIRECTION_ENT_portDirection,
    &DX_OUT_CPSS_DXCH_CSCD_PORT_CRC_MODE_ENT_PTR_crcModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdPortTcProfiletGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_PORT_DIRECTION_ENT_portDirection,
    &DX_OUT_CPSS_DXCH_PORT_PROFILE_ENT_PTR_portProfilePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_BOOL_acceptSkipSaLookup
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdFastFailoverPortIsLoopedSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_BOOL_isLooped
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdPortStackAggregationConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_HW_DEV_NUM_aggHwDevNum,
    &DX_IN_GT_PORT_NUM_aggPortNum,
    &DX_IN_GT_U32_aggSrcId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_HW_DEV_NUM_remotePhysicalHwDevNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_remotePhysicalPortNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdCentralizedChassisReservedDevNumSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_HW_DEV_NUM_reservedHwDevNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdFastFailoverSecondaryTargetPortMapSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_secondaryTargetPort
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_PORT_NUM_portBaseEport,
    &DX_IN_GT_PORT_NUM_trunkBaseEport
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_physicalPortBase
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdCentralizedChassisSrcIdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_srcId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT_PTR_assignmentModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdPortQosDsaModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_CSCD_PORT_QOS_DSA_TRUST_MODE_ENT_PTR_portQosDsaTrustModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdRemapQosModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_CSCD_QOS_REMAP_TYPE_ENT_PTR_remapTypePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_BOOL_PTR_acceptSkipSaLookupPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdFastFailoverPortIsLoopedGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_BOOL_PTR_isLoopedPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdPortStackAggregationConfigGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_HW_DEV_NUM_PTR_aggHwDevNumPtr,
    &DX_OUT_GT_PORT_NUM_PTR_aggPortNumPtr,
    &DX_OUT_GT_U32_PTR_aggSrcIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_HW_DEV_NUM_PTR_remotePhysicalHwDevNumPtr,
    &DX_OUT_GT_PHYSICAL_PORT_NUM_PTR_remotePhysicalPortNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdCentralizedChassisReservedDevNumGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_HW_DEV_NUM_PTR_reservedHwDevNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdFastFailoverSecondaryTargetPortMapGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_PHYSICAL_PORT_NUM_PTR_secondaryTargetPortPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_PORT_NUM_PTR_portBaseEportPtr,
    &DX_OUT_GT_PORT_NUM_PTR_trunkBaseEportPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_U32_PTR_physicalPortBasePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdCentralizedChassisSrcIdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_U32_PTR_srcIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_srcDevLsbAmount,
    &DX_IN_GT_U32_srcPortLsbAmount
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_srcTrunkLsbAmount,
    &DX_IN_GT_U32_srcPortLsbAmount,
    &DX_IN_GT_U32_srcDevLsbAmount
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdQosTcRemapTableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_tc,
    &DX_IN_CPSS_DXCH_CSCD_QOS_TC_REMAP_STC_PTR_tcMappingsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdQosTcRemapTableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_tc,
    &DX_OUT_CPSS_DXCH_CSCD_QOS_TC_REMAP_STC_PTR_tcMappingsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_vlanTagTpid
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdCtrlQosSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_ctrlTc,
    &DX_IN_CPSS_DP_LEVEL_ENT_ctrlDp,
    &DX_IN_CPSS_DP_LEVEL_ENT_cpuToCpuDp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdRemapDataQosTblSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_tc,
    &DX_IN_CPSS_DP_LEVEL_ENT_dp,
    &DX_IN_GT_U8_remapTc,
    &DX_IN_CPSS_DP_LEVEL_ENT_remapDp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdRemapDataQosTblGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U8_tc,
    &DX_IN_CPSS_DP_LEVEL_ENT_dp,
    &DX_OUT_GT_U8_PTR_remapTcPtr,
    &DX_OUT_CPSS_DP_LEVEL_ENT_PTR_remapDpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdQosTcDpRemapTableAccessModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_CSCD_QOS_PRIO_TABLE_ACCESS_MODE_ENT_PTR_prioModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdDevMapLookupModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_DEV_MAP_LOOKUP_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdDsaSrcDevFilterGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_BOOL_PTR_enableOwnDevFltrPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_srcDevLsbAmountPtr,
    &DX_OUT_GT_U32_PTR_srcPortLsbAmountPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_srcTrunkLsbAmountPtr,
    &DX_OUT_GT_U32_PTR_srcPortLsbAmountPtr,
    &DX_OUT_GT_U32_PTR_srcDevLsbAmountPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_vlanTagTpidPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCscdCtrlQosGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U8_PTR_ctrlTcPtr,
    &DX_OUT_CPSS_DP_LEVEL_ENT_PTR_ctrlDpPtr,
    &DX_OUT_CPSS_DP_LEVEL_ENT_PTR_cpuToCpuDpPtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChCscdLogLibDb[] = {
    {"cpssDxChCscdPortTypeSet", 4, cpssDxChCscdPortTypeSet_PARAMS, NULL},
    {"cpssDxChCscdPortTypeGet", 4, cpssDxChCscdPortTypeGet_PARAMS, NULL},
    {"cpssDxChCscdDevMapTableSet", 8, cpssDxChCscdDevMapTableSet_PARAMS, NULL},
    {"cpssDxChCscdDevMapTableGet", 8, cpssDxChCscdDevMapTableGet_PARAMS, NULL},
    {"cpssDxChCscdRemapQosModeSet", 3, cpssDxChCscdRemapQosModeSet_PARAMS, NULL},
    {"cpssDxChCscdRemapQosModeGet", 3, cpssDxChCscdRemapQosModeGet_PARAMS, NULL},
    {"cpssDxChCscdCtrlQosSet", 4, cpssDxChCscdCtrlQosSet_PARAMS, NULL},
    {"cpssDxChCscdCtrlQosGet", 4, cpssDxChCscdCtrlQosGet_PARAMS, NULL},
    {"cpssDxChCscdRemapDataQosTblSet", 5, cpssDxChCscdRemapDataQosTblSet_PARAMS, NULL},
    {"cpssDxChCscdRemapDataQosTblGet", 5, cpssDxChCscdRemapDataQosTblGet_PARAMS, NULL},
    {"cpssDxChCscdDsaSrcDevFilterSet", 2, cpssDxChCscdDsaSrcDevFilterSet_PARAMS, NULL},
    {"cpssDxChCscdDsaSrcDevFilterGet", 2, cpssDxChCscdDsaSrcDevFilterGet_PARAMS, NULL},
    {"cpssDxChCscdHyperGPortCrcModeSet", 4, cpssDxChCscdHyperGPortCrcModeSet_PARAMS, NULL},
    {"cpssDxChCscdHyperGPortCrcModeGet", 4, cpssDxChCscdHyperGPortCrcModeGet_PARAMS, NULL},
    {"cpssDxChCscdFastFailoverFastStackRecoveryEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChCscdFastFailoverFastStackRecoveryEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChCscdFastFailoverSecondaryTargetPortMapSet", 3, cpssDxChCscdFastFailoverSecondaryTargetPortMapSet_PARAMS, NULL},
    {"cpssDxChCscdFastFailoverSecondaryTargetPortMapGet", 3, cpssDxChCscdFastFailoverSecondaryTargetPortMapGet_PARAMS, NULL},
    {"cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChCscdFastFailoverTerminateLocalLoopbackEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChCscdFastFailoverPortIsLoopedSet", 3, cpssDxChCscdFastFailoverPortIsLoopedSet_PARAMS, NULL},
    {"cpssDxChCscdFastFailoverPortIsLoopedGet", 3, cpssDxChCscdFastFailoverPortIsLoopedGet_PARAMS, NULL},
    {"cpssDxChCscdQosPortTcRemapEnableSet", 3, prvCpssLogGenDevNumPortNumEnable_PARAMS, NULL},
    {"cpssDxChCscdQosPortTcRemapEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr_PARAMS, NULL},
    {"cpssDxChCscdQosTcRemapTableSet", 3, cpssDxChCscdQosTcRemapTableSet_PARAMS, NULL},
    {"cpssDxChCscdQosTcDpRemapTableAccessModeSet", 2, cpssDxChCscdQosTcDpRemapTableAccessModeSet_PARAMS, NULL},
    {"cpssDxChCscdQosTcDpRemapTableAccessModeGet", 2, cpssDxChCscdQosTcDpRemapTableAccessModeGet_PARAMS, NULL},
    {"cpssDxChCscdQosTcRemapTableGet", 3, cpssDxChCscdQosTcRemapTableGet_PARAMS, NULL},
    {"cpssDxChCscdQosTcDpRemapTableSet", 8, cpssDxChCscdQosTcDpRemapTableSet_PARAMS, NULL},
    {"cpssDxChCscdQosTcDpRemapTableGet", 8, cpssDxChCscdQosTcDpRemapTableGet_PARAMS, NULL},
    {"cpssDxChCscdPortBridgeBypassEnableSet", 3, prvCpssLogGenDevNumPortNumEnable2_PARAMS, NULL},
    {"cpssDxChCscdPortBridgeBypassEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr2_PARAMS, NULL},
    {"cpssDxChCscdOrigSrcPortFilterEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChCscdOrigSrcPortFilterEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChCscdDevMapLookupModeSet", 2, cpssDxChCscdDevMapLookupModeSet_PARAMS, NULL},
    {"cpssDxChCscdDevMapLookupModeGet", 2, cpssDxChCscdDevMapLookupModeGet_PARAMS, NULL},
    {"cpssDxChCscdPortLocalDevMapLookupEnableSet", 4, cpssDxChCscdPortLocalDevMapLookupEnableSet_PARAMS, NULL},
    {"cpssDxChCscdPortLocalDevMapLookupEnableGet", 4, cpssDxChCscdPortLocalDevMapLookupEnableGet_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisModeEnableSet", 3, prvCpssLogGenDevNumPortNumEnable_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisModeEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisReservedDevNumSet", 3, cpssDxChCscdCentralizedChassisReservedDevNumSet_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisReservedDevNumGet", 3, cpssDxChCscdCentralizedChassisReservedDevNumGet_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisSrcIdSet", 3, cpssDxChCscdCentralizedChassisSrcIdSet_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisSrcIdGet", 3, cpssDxChCscdCentralizedChassisSrcIdGet_PARAMS, NULL},
    {"cpssDxChCscdDbRemoteHwDevNumModeSet", 2, cpssDxChCscdDbRemoteHwDevNumModeSet_PARAMS, NULL},
    {"cpssDxChCscdDbRemoteHwDevNumModeGet", 2, cpssDxChCscdDbRemoteHwDevNumModeGet_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet", 4, cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseSet_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet", 4, cpssDxChCscdCentralizedChassisLineCardDefaultEportBaseGet_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet", 4, cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountSet_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet", 4, cpssDxChCscdCentralizedChassisMappingDsaSrcLsbAmountGet_PARAMS, NULL},
    {"cpssDxChCscdPortMruCheckOnCascadeEnableSet", 3, prvCpssLogGenDevNumPortNumEnable_PARAMS, NULL},
    {"cpssDxChCscdPortMruCheckOnCascadeEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet", 3, cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet", 3, cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountGet_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet", 2, cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet", 2, cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidGet_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet", 3, cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet", 3, cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeGet_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet", 3, cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet", 3, cpssDxChCscdCentralizedChassisMyPhysicalPortBaseGet_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet", 3, prvCpssLogGenDevNumPortNumEnable_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr_PARAMS, NULL},
    {"cpssDxChCscdPortStackAggregationEnableSet", 3, prvCpssLogGenDevNumPortNumEnable_PARAMS, NULL},
    {"cpssDxChCscdPortStackAggregationEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr_PARAMS, NULL},
    {"cpssDxChCscdPortStackAggregationConfigSet", 5, cpssDxChCscdPortStackAggregationConfigSet_PARAMS, NULL},
    {"cpssDxChCscdPortStackAggregationConfigGet", 5, cpssDxChCscdPortStackAggregationConfigGet_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet", 3, prvCpssLogGenDevNumPortNumEnable_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet", 4, cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet_PARAMS, NULL},
    {"cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet", 4, cpssDxChCscdCentralizedChassisRemotePhysicalPortMapGet_PARAMS, NULL},
    {"cpssDxChCscdPortQosDsaModeSet", 3, cpssDxChCscdPortQosDsaModeSet_PARAMS, NULL},
    {"cpssDxChCscdPortQosDsaModeGet", 3, cpssDxChCscdPortQosDsaModeGet_PARAMS, NULL},
    {"cpssDxChCscdHashInDsaEnableSet", 4, prvCpssLogGenDevNumPortNumDirectionEnable_PARAMS, NULL},
    {"cpssDxChCscdHashInDsaEnableGet", 4, prvCpssLogGenDevNumPortNumDirectionEnablePtr_PARAMS, NULL},
    {"cpssDxChCscdPortTcProfiletSet", 4, cpssDxChCscdPortTcProfiletSet_PARAMS, NULL},
    {"cpssDxChCscdPortTcProfiletGet", 4, cpssDxChCscdPortTcProfiletGet_PARAMS, NULL},
    {"cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet", 3, cpssDxChCscdPortAcceptFdbSaLookupSkipEnableSet_PARAMS, NULL},
    {"cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet", 3, cpssDxChCscdPortAcceptFdbSaLookupSkipEnableGet_PARAMS, NULL},
    {"cpssDxChCscdPortForce4BfromCpuDsaEnableSet", 3, prvCpssLogGenDevNumPortNumEnable_PARAMS, NULL},
    {"cpssDxChCscdPortForce4BfromCpuDsaEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_CSCD(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChCscdLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChCscdLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

