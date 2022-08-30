/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChConfigLog.c
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
#include <cpss/common/cpssHwInit/private/prvCpssCommonCpssHwInitLog.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChConfigLog.h>
#include <cpss/generic/extMac/private/prvCpssGenExtMacLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT[]  =
{
    "CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_6_LSB_E",
    "CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_6_MSB_E",
    "CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_12_BITS_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT[]  =
{
    "CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ALL_E",
    "CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_VLAN_E",
    "CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_PORT_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT[]  =
{
    "CPSS_DXCH_CFG_QCN_REPLICATION_E",
    "CPSS_DXCH_CFG_SNIFF_REPLICATION_E",
    "CPSS_DXCH_CFG_TRAP_REPLICATION_E",
    "CPSS_DXCH_CFG_MIRROR_REPLICATION_E",
    "CPSS_DXCH_CFG_OUTGOING_FORWARD_E",
    "CPSS_DXCH_CFG_QCN_FIFO_DROP_E",
    "CPSS_DXCH_CFG_SNIFF_FIFO_DROP_E",
    "CPSS_DXCH_CFG_TRAP_FIFO_DROP_E",
    "CPSS_DXCH_CFG_MIRROR_FIFO_DROP_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_CFG_TABLES_ENT[]  =
{
    "CPSS_DXCH_CFG_TABLE_VLAN_E",
    "CPSS_DXCH_CFG_TABLE_FDB_E",
    "CPSS_DXCH_CFG_TABLE_PCL_ACTION_E",
    "CPSS_DXCH_CFG_TABLE_PCL_TCAM_E",
    "CPSS_DXCH_CFG_TABLE_ROUTER_NEXT_HOP_E",
    "CPSS_DXCH_CFG_TABLE_ROUTER_LTT_E",
    "CPSS_DXCH_CFG_TABLE_ROUTER_TCAM_E",
    "CPSS_DXCH_CFG_TABLE_ROUTER_ECMP_QOS_E",
    "CPSS_DXCH_CFG_TABLE_TTI_TCAM_E",
    "CPSS_DXCH_CFG_TABLE_MLL_PAIR_E",
    "CPSS_DXCH_CFG_TABLE_POLICER_METERS_E",
    "CPSS_DXCH_CFG_TABLE_POLICER_BILLING_COUNTERS_E",
    "CPSS_DXCH_CFG_TABLE_VIDX_E",
    "CPSS_DXCH_CFG_TABLE_ARP_E",
    "CPSS_DXCH_CFG_TABLE_TUNNEL_START_E",
    "CPSS_DXCH_CFG_TABLE_STG_E",
    "CPSS_DXCH_CFG_TABLE_QOS_PROFILE_E",
    "CPSS_DXCH_CFG_TABLE_MAC_TO_ME_E",
    "CPSS_DXCH_CFG_TABLE_CNC_E",
    "CPSS_DXCH_CFG_TABLE_CNC_BLOCK_E",
    "CPSS_DXCH_CFG_TABLE_TRUNK_E",
    "CPSS_DXCH_CFG_TABLE_LPM_RAM_E",
    "CPSS_DXCH_CFG_TABLE_ROUTER_ECMP_E",
    "CPSS_DXCH_CFG_TABLE_L2_MLL_LTT_E",
    "CPSS_DXCH_CFG_TABLE_EPORT_E",
    "CPSS_DXCH_CFG_TABLE_DEFAULT_EPORT_E",
    "CPSS_DXCH_CFG_TABLE_PHYSICAL_PORT_E",
    "CPSS_DXCH_CFG_TABLE_EXACT_MATCH_E",
    "CPSS_DXCH_CFG_TABLE_SOURCE_ID_E",
    "CPSS_DXCH_CFG_TABLE_OAM_E",
    "CPSS_DXCH_CFG_TABLE_DDE_PARTITION_E",
    "CPSS_DXCH_CFG_TABLE_FDB_WITH_DDE_PARTITION_E",
    "CPSS_DXCH_CFG_TABLE_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CFG_TABLES_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_LPM_RAM_MEM_MODE_ENT[]  =
{
    "CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E",
    "CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_LPM_RAM_MEM_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENT[]  =
{
    "CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_DISABLE_E",
    "CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_PATTERN_MASK_E",
    "CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E",
    "CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_ENT[]  =
{
    "CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_88E1690_E",
    "CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE__LAST___E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_CFG_ROUTING_MODE_ENT[]  =
{
    "CPSS_DXCH_POLICY_BASED_ROUTING_ONLY_E",
    "CPSS_DXCH_TCAM_ROUTER_BASED_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CFG_ROUTING_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_ENT[]  =
{
    "CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_LPM_E",
    "CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_FDB_E",
    "CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_EM_E",
    "CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_ARP_E",
    "CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_LPM_AGING_E",
    "CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS___LAST___E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT[]  =
{
    "CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E",
    "CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E",
    "CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E",
    "CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E",
    "CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_LOW_L2_MAX_EM_E",
    "CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_NO_EM_E",
    "CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_L2_MID_EM_E",
    "CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_LOW_L2_MID_EM_MAX_ARP_E",
    "CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MID_L2_MAX_EM_E",
    "CPSS_DXCH_CFG_SHARED_TABLE_MODE_LOW_MAX_L3_MID_LOW_L2_NO_EM_E",
    "CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_HIGH_L3_MID_LOW_L2_NO_EM_MAX_ARP_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_CFG_DEV_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_CFG_DEV_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, genDevInfo, CPSS_GEN_CFG_DEV_INFO_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_CFG_GLOBAL_EPORT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_CFG_GLOBAL_EPORT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, enable, CPSS_DXCH_CFG_GLOBAL_EPORT_CONFG_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, pattern);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, mask);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, minValue);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxValue);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_CFG_HW_ACCESS_OBJ_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_CFG_HW_ACCESS_OBJ_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hwAccessTableEntryReadFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hwAccessTableEntryWriteFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hwAccessTableEntryFieldReadFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hwAccessTableEntryFieldWriteFunc);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_CFG_PROBE_PACKET_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_CFG_PROBE_PACKET_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, bitLocation);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, enable);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, clientId, CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, portGroupId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, bankIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, lineIndex);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PP_CONFIG_INIT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PP_CONFIG_INIT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, routingMode, CPSS_DXCH_CFG_ROUTING_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxNumOfPbrEntries);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, lpmMemoryMode, CPSS_DXCH_LPM_RAM_MEM_MODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, sharedTableMode, CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_CFG_REMOTE_PHY_MAC_PORT_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_CFG_REMOTE_PHY_MAC_PORT_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, remotePhysicalPortNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, remoteMacPortNum);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_LPM_RAM_CONFIG_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_LPM_RAM_CONFIG_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, devType);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, sharedMemCnfg, CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_CFG_DEV_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_CFG_DEV_INFO_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_CFG_DEV_INFO_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CFG_GLOBAL_EPORT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_CFG_GLOBAL_EPORT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_CFG_GLOBAL_EPORT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_CFG_HW_ACCESS_OBJ_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_CFG_HW_ACCESS_OBJ_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_CFG_HW_ACCESS_OBJ_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CFG_PROBE_PACKET_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_CFG_PROBE_PACKET_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_CFG_PROBE_PACKET_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_CFG_TABLES_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_CFG_TABLES_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_CFG_TABLES_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_LPM_RAM_MEM_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_LPM_RAM_MEM_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_LPM_RAM_MEM_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PP_CONFIG_INIT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PP_CONFIG_INIT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PP_CONFIG_INIT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_entryIndexPtr = {
     "entryIndexPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT_selectionMode = {
     "selectionMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CFG_GLOBAL_EPORT_STC_PTR_globalPtr = {
     "globalPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CFG_GLOBAL_EPORT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CFG_GLOBAL_EPORT_STC_PTR_l2DlbPtr = {
     "l2DlbPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CFG_GLOBAL_EPORT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CFG_GLOBAL_EPORT_STC_PTR_l2EcmpPtr = {
     "l2EcmpPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CFG_GLOBAL_EPORT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CFG_HW_ACCESS_OBJ_STC_PTR_cfgAccessObjPtr = {
     "cfgAccessObjPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CFG_HW_ACCESS_OBJ_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CFG_PROBE_PACKET_STC_PTR_probeCfgPtr = {
     "probeCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CFG_PROBE_PACKET_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC_PTR_infoPtr = {
     "infoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT_counterType = {
     "counterType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CFG_TABLES_ENT_table = {
     "table", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_CFG_TABLES_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PP_CONFIG_INIT_STC_PTR_ppConfigPtr = {
     "ppConfigPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PP_CONFIG_INIT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_MACDRV_OBJ_STC_PTR_macDrvObjPtr = {
     "macDrvObjPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_MACDRV_OBJ_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_PTR_modifedDsaSrcDevPtr = {
     "modifedDsaSrcDevPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_bind = {
     "bind", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_modifedDsaSrcDev = {
     "modifedDsaSrcDev", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_modifyEnable = {
     "modifyEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_pktDropEnable = {
     "pktDropEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PORT_NUM_ePort = {
     "ePort", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_newDevNum = {
     "newDevNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_oldDevNum = {
     "oldDevNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_AUQ_CFG_STC_PTR_PTR_prevCpssInit_auqCfgPtrPtr = {
     "prevCpssInit_auqCfgPtrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_PTR_MAC(CPSS_AUQ_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_AUQ_CFG_STC_PTR_PTR_prevCpssInit_fuqCfgPtrPtr = {
     "prevCpssInit_fuqCfgPtrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_PTR_MAC(CPSS_AUQ_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CFG_DEV_INFO_STC_PTR_devInfoPtr = {
     "devInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CFG_DEV_INFO_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT_PTR_selectionModePtr = {
     "selectionModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CFG_GLOBAL_EPORT_STC_PTR_globalPtr = {
     "globalPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CFG_GLOBAL_EPORT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CFG_GLOBAL_EPORT_STC_PTR_l2DlbPtr = {
     "l2DlbPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CFG_GLOBAL_EPORT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CFG_GLOBAL_EPORT_STC_PTR_l2EcmpPtr = {
     "l2EcmpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CFG_GLOBAL_EPORT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CFG_PROBE_PACKET_STC_PTR_probeCfgPtr = {
     "probeCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CFG_PROBE_PACKET_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_STC_PTR_clientInfoPtr = {
     "clientInfoPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_MULTI_NET_IF_CFG_STC_PTR_PTR_prevCpssInit_multiNetIfCfgPtrPtr = {
     "prevCpssInit_multiNetIfCfgPtrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_PTR_MAC(CPSS_MULTI_NET_IF_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_NET_IF_CFG_STC_PTR_PTR_prevCpssInit_netIfCfgPtrPtr = {
     "prevCpssInit_netIfCfgPtrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_PTR_MAC(CPSS_NET_IF_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_PTR_prevCpssInit_fuqUseSeparatePtrPtr = {
     "prevCpssInit_fuqUseSeparatePtrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_PTR_prevCpssInit_useMultiNetIfSdmaPtrPtr = {
     "prevCpssInit_useMultiNetIfSdmaPtrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_modifyEnablePtr = {
     "modifyEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_pktDropEnablePtr = {
     "pktDropEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_PORT_NUM_PTR_ePortPtr = {
     "ePortPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_PORT_NUM_PTR_portNumPtr = {
     "portNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_nextDevNumPtr = {
     "nextDevNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgHwAccessObjectBind_PARAMS[] =  {
    &DX_IN_CPSS_DXCH_CFG_HW_ACCESS_OBJ_STC_PTR_cfgAccessObjPtr,
    &DX_IN_GT_BOOL_bind
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgProbePacketDropCodeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DIRECTION_ENT_direction,
    &DX_OUT_CPSS_NET_RX_CPU_CODE_ENT_PTR_dropCodePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgEgressHashSelectionModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT_selectionMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgGlobalEportSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CFG_GLOBAL_EPORT_STC_PTR_globalPtr,
    &DX_IN_CPSS_DXCH_CFG_GLOBAL_EPORT_STC_PTR_l2EcmpPtr,
    &DX_IN_CPSS_DXCH_CFG_GLOBAL_EPORT_STC_PTR_l2DlbPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgIngressDropCntrModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT_mode,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_GT_U16_vlan
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgProbePacketConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CFG_PROBE_PACKET_STC_PTR_probeCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgReplicationCountersGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CFG_REPLICATION_COUNTERS_TYPE_ENT_counterType,
    &DX_OUT_GT_U32_PTR_counterPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChTableValidIndexGetNext_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CFG_TABLES_ENT_table,
    &DX_INOUT_GT_U32_PTR_entryIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgTableNumEntriesGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_CFG_TABLES_ENT_table,
    &DX_OUT_GT_U32_PTR_numEntriesPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgRemoteFcModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PORT_HOL_FC_ENT_modeFcHol,
    &DX_IN_CPSS_PORT_TX_DROP_PROFILE_SET_ENT_profileSet,
    &DX_IN_GT_U32_tcBitmap
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgPpLogicalInit_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PP_CONFIG_INIT_STC_PTR_ppConfigPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgDsaTagSrcDevPortModifyGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_PTR_modifedDsaSrcDevPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgDsaTagSrcDevPortModifySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_modifedDsaSrcDev
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgIngressDropEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_pktDropEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgPortRemotePhyMacBind_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC_PTR_infoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgBindPortPhymacObject_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_MACDRV_OBJ_STC_PTR_macDrvObjPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_BOOL_modifyEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgPortDefaultSourceEportNumberSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_PORT_NUM_ePort
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_BOOL_PTR_modifyEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgPortDefaultSourceEportNumberGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_PORT_NUM_PTR_ePortPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgDevInfoGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_CFG_DEV_INFO_STC_PTR_devInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgEgressHashSelectionModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_CFG_EGRESS_HASH_SELECTION_FROM_INGRESS_MODE_ENT_PTR_selectionModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgGlobalEportGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_CFG_GLOBAL_EPORT_STC_PTR_globalPtr,
    &DX_OUT_CPSS_DXCH_CFG_GLOBAL_EPORT_STC_PTR_l2EcmpPtr,
    &DX_OUT_CPSS_DXCH_CFG_GLOBAL_EPORT_STC_PTR_l2DlbPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgIngressDropCntrModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_CFG_INGRESS_DROP_COUNTER_MODE_ENT_PTR_modePtr,
    &DX_OUT_GT_PORT_NUM_PTR_portNumPtr,
    &DX_OUT_GT_U16_PTR_vlanPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgProbePacketConfigGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_CFG_PROBE_PACKET_STC_PTR_probeCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgSharedTablesIllegalClientAccessInfoGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_CFG_SHARED_TABLE_ILLEGAL_CLIENT_ACCESS_STC_PTR_clientInfoPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgRemoteFcModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_PORT_HOL_FC_ENT_PTR_modeFcHolPtr,
    &DX_OUT_CPSS_PORT_TX_DROP_PROFILE_SET_ENT_PTR_profileSetPtr,
    &DX_OUT_GT_U32_PTR_tcBitmapPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgDevDbInfoGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_NET_IF_CFG_STC_PTR_PTR_prevCpssInit_netIfCfgPtrPtr,
    &DX_OUT_CPSS_AUQ_CFG_STC_PTR_PTR_prevCpssInit_auqCfgPtrPtr,
    &DX_OUT_GT_BOOL_PTR_PTR_prevCpssInit_fuqUseSeparatePtrPtr,
    &DX_OUT_CPSS_AUQ_CFG_STC_PTR_PTR_prevCpssInit_fuqCfgPtrPtr,
    &DX_OUT_GT_BOOL_PTR_PTR_prevCpssInit_useMultiNetIfSdmaPtrPtr,
    &DX_OUT_CPSS_MULTI_NET_IF_CFG_STC_PTR_PTR_prevCpssInit_multiNetIfCfgPtrPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgIngressDropEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_BOOL_PTR_pktDropEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgNextDevGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U8_PTR_nextDevNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChCfgReNumberDevNum_PARAMS[] =  {
    &DX_IN_GT_U8_oldDevNum,
    &DX_IN_GT_U8_newDevNum
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChConfigLogLibDb[] = {
    {"cpssDxChCfgDevRemove", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssDxChCfgDevEnable", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChCfgDevEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChCfgNextDevGet", 2, cpssDxChCfgNextDevGet_PARAMS, NULL},
    {"cpssDxChCfgPpLogicalInit", 2, cpssDxChCfgPpLogicalInit_PARAMS, NULL},
    {"cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet", 3, cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifySet_PARAMS, NULL},
    {"cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet", 3, cpssDxChCfgDsaTagSrcDevPortRoutedPcktModifyGet_PARAMS, NULL},
    {"cpssDxChCfgDsaTagSrcDevPortModifySet", 2, cpssDxChCfgDsaTagSrcDevPortModifySet_PARAMS, NULL},
    {"cpssDxChCfgDsaTagSrcDevPortModifyGet", 2, cpssDxChCfgDsaTagSrcDevPortModifyGet_PARAMS, NULL},
    {"cpssDxChCfgTableNumEntriesGet", 3, cpssDxChCfgTableNumEntriesGet_PARAMS, NULL},
    {"cpssDxChTableValidIndexGetNext", 3, cpssDxChTableValidIndexGetNext_PARAMS, NULL},
    {"cpssDxChCfgReNumberDevNum", 2, cpssDxChCfgReNumberDevNum_PARAMS, NULL},
    {"cpssDxChCfgDevInfoGet", 2, cpssDxChCfgDevInfoGet_PARAMS, NULL},
    {"cpssDxChCfgBindPortPhymacObject", 3, cpssDxChCfgBindPortPhymacObject_PARAMS, NULL},
    {"cpssDxChCfgPortDefaultSourceEportNumberSet", 3, cpssDxChCfgPortDefaultSourceEportNumberSet_PARAMS, NULL},
    {"cpssDxChCfgPortDefaultSourceEportNumberGet", 3, cpssDxChCfgPortDefaultSourceEportNumberGet_PARAMS, NULL},
    {"cpssDxChCfgEgressHashSelectionModeSet", 2, cpssDxChCfgEgressHashSelectionModeSet_PARAMS, NULL},
    {"cpssDxChCfgEgressHashSelectionModeGet", 2, cpssDxChCfgEgressHashSelectionModeGet_PARAMS, NULL},
    {"cpssDxChCfgIngressDropCntrModeSet", 4, cpssDxChCfgIngressDropCntrModeSet_PARAMS, NULL},
    {"cpssDxChCfgIngressDropCntrModeGet", 4, cpssDxChCfgIngressDropCntrModeGet_PARAMS, NULL},
    {"cpssDxChCfgIngressDropCntrSet", 2, prvCpssLogGenDevNumCounter_PARAMS, NULL},
    {"cpssDxChCfgIngressDropCntrGet", 2, prvCpssLogGenDevNumCounterPtr_PARAMS, NULL},
    {"cpssDxChCfgGlobalEportSet", 4, cpssDxChCfgGlobalEportSet_PARAMS, NULL},
    {"cpssDxChCfgGlobalEportGet", 4, cpssDxChCfgGlobalEportGet_PARAMS, NULL},
    {"cpssDxChCfgHwAccessObjectBind", 2, cpssDxChCfgHwAccessObjectBind_PARAMS, NULL},
    {"cpssDxChCfgHitlessWriteMethodEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChCfgPortRemotePhyMacBind", 3, cpssDxChCfgPortRemotePhyMacBind_PARAMS, NULL},
    {"cpssDxChCfgRemoteFcModeSet", 4, cpssDxChCfgRemoteFcModeSet_PARAMS, NULL},
    {"cpssDxChCfgRemoteFcModeGet", 4, cpssDxChCfgRemoteFcModeGet_PARAMS, NULL},
    {"cpssDxChCfgSharedTablesIllegalClientAccessInfoGet", 2, cpssDxChCfgSharedTablesIllegalClientAccessInfoGet_PARAMS, NULL},
    {"cpssDxChCfgReplicationCountersGet", 3, cpssDxChCfgReplicationCountersGet_PARAMS, NULL},
    {"cpssDxChCfgProbePacketConfigSet", 2, cpssDxChCfgProbePacketConfigSet_PARAMS, NULL},
    {"cpssDxChCfgProbePacketConfigGet", 2, cpssDxChCfgProbePacketConfigGet_PARAMS, NULL},
    {"cpssDxChCfgProbePacketDropCodeGet", 3, cpssDxChCfgProbePacketDropCodeGet_PARAMS, NULL},
    {"cpssDxChCfgDevDbInfoGet", 7, cpssDxChCfgDevDbInfoGet_PARAMS, NULL},
    {"cpssDxChCfgIngressDropEnableSet", 2, cpssDxChCfgIngressDropEnableSet_PARAMS, NULL},
    {"cpssDxChCfgIngressDropEnableGet", 2, cpssDxChCfgIngressDropEnableGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_CONFIG(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChConfigLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChConfigLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

