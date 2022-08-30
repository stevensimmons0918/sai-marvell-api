/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChBrgFdbManagerLog.c
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
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/cpssDxChBrgFdbManagerTypes.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManagerLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChConfigLog.h>
#include <cpss/dxCh/dxChxGen/log/private/prvCpssDxChLog.h>
#include <cpss/generic/bridge/private/prvCpssGenBrgLog.h>
#include <cpss/generic/ip/private/prvCpssGenIpLog.h>
#include <cpss/generic/log/prvCpssGenLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT[]  =
{
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_TYPE_COUNTERS_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_SW_BANK_COUNTERS_MISMATCH_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_1ST_PTR_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_LAST_PTR_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_TRANS_SCAN_PTR_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_DELETE_SCAN_PTR_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRY_PREV_PTR_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_ENTRY_BIN_INDEX_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_VALID_EXIST_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_HW_FREE_ENTRY_SP_EXIST_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_GLOBAL_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_FID_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_GLOBAL_EPORT_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_TRUNK_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_PORT_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_LEARN_LIMIT_SRC_INTERFACES_CURRENT_INVALID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_ENT[]  =
{
    "CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_ALL_ZERO_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_USE_FID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_USE_MAC_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_ENT[]  =
{
    "CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_FID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_TRUNK_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_GLOBAL_EPORT_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_PORT_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE__LAST__E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_ENT[]  =
{
    "CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_9_SRC_ID_8_UDB_TAG1_VID_DISABLE_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_12_SRC_ID_5_UDB_TAG1_VID_DISABLE_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_8_UDB_TAG1_VID_ENABLE_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_6_SRC_ID_5_UDB_TAG1_VID_ENABLE_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE__MUST_BE_LAST__E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT[]  =
{
    "CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_MAC_ADDR_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_MC_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_MC_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV6_UC_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE__LAST__E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_ENT[]  =
{
    "CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NEW_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_MOVED_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NO_SPACE_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_AGED_OUT_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_AGED_DELETED_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_DELETED_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_TRANSPLANTED_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_MOVED_REACH_LIMIT_AND_DELETED_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_NEW_REACH_LIMIT_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_ENT[]  =
{
    "CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_SRC_ID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_UDB_4_SRC_ID_3_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_UDB_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE__MUST_BE_LAST__E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_ENT[]  =
{
    "CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_TAG1_VID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_SRC_ID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_UDB_10_SRC_ID_1_DA_ACCESS_LEVEL_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_UDB_8_SRC_ID_3_DA_ACCESS_LEVEL_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE__MUST_BE_LAST__E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_ENT[]  =
{
    "CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_ONLY_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_MAC_AND_FID_AND_VID1_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT[]  =
{
    "CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP6_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP5_SIP6_HYBRID_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ENT[]  =
{
    "CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_DISABLED_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_OR_DA_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_DA_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ON_SA_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ENT[]  =
{
    "CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_TUNNEL_START_PTR_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_NAT_PTR_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_POINTER_TO_ROUTER_E",
    "CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ECMP_POINTER_TO_ROUTER_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, checkAgeMacUcEntries);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, checkAgeMacMcEntries);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, checkAgeIpUcEntries);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, checkAgeIpMcEntries);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, fid);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, fidMask);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ePortTrunkNum);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, ePortTrunkNumMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isTrunk);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isTrunkMask);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vid1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vid1Mask);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, userDefined);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, userDefinedMask);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, hwDevNum);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, hwDevNumMask_ePort);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, hwDevNumMask_trunk);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, deleteAgeoutMacUcEportEntries);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, deleteAgeoutMacUcTrunkEntries);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, deleteAgeoutMacMcEntries);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, deleteAgeoutIpUcEntries);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, deleteAgeoutIpMcEntries);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, destinationUcRefreshEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, destinationMcRefreshEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ipUcRefreshEnable);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, hwCapacity, CPSS_DXCH_BRG_FDB_MANAGER_HW_CAPACITY_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxTotalEntries);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxEntriesPerAgingScan);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxEntriesPerDeleteScan);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxEntriesPerTransplantScan);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxEntriesPerLearningScan);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, enableLearningLimits);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxDynamicUcMacGlobalLimit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxDynamicUcMacFidLimit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxDynamicUcMacGlobalEportLimit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxDynamicUcMacTrunkLimit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxDynamicUcMacPortLimit);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, destinationUcRefreshEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, destinationMcRefreshEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ipUcRefreshEnable);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, saDropCommand, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, daDropCommand, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ipNhPacketcommand, CPSS_PACKET_CMD_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, macRoutedLearningEnable);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, macUnicastDynamic);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, macUnicastStatic);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, macMulticastDynamic);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, macMulticastStatic);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ipv4Multicast);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ipv6Multicast);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ipv4UnicastRoute);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ipv6UnicastRouteKey);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ipv6UnicastRouteData);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, usedEntries);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, freeEntries);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, bankCounters, CPSS_DXCH_BRG_FDB_MANAGER_MAX_NUM_BANKS_CNS, GT_U32);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, globalCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, countersCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, dbFreeListCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, dbUsedListCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, dbIndexPointerCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, dbAgingBinCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, dbAgingBinUsageMatrixCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, hwUsedListCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, hwFreeListCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, hwFreeListWithSpCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, learnLimitGlobalCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, learnLimitFidCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, learnLimitGlobalEportCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, learnLimitTrunkCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, learnLimitPortCheckEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, learnLimitSrcInterfacesCheckEnable);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, deleteMacUcEntries);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, deleteMacMcEntries);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, deleteIpUcEntries);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, deleteIpMcEntries);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, deleteStaticEntries);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, fid);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, fidMask);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, hwDevNum);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, hwDevNumMask);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ePortTrunkNum);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, ePortTrunkNumMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isTrunk);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isTrunkMask);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vid1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vid1Mask);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, userDefined);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, userDefinedMask);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, onlyAboveLearnLimit);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, limitType, CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_TYPE_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, limitIndex, CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_UNT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, tempEntryExist);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tempEntryOffset);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, rehashEnable);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, macEntryMuxingMode, CPSS_DXCH_BRG_FDB_MANAGER_MAC_ENTRY_MUXING_MODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ipmcEntryMuxingMode, CPSS_DXCH_BRG_FDB_MANAGER_IPMC_ENTRY_MUXING_MODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, entryMuxingMode, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MUXING_MODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, saDropCommand, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, daDropCommand, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ipNhPacketcommand, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, shadowType, CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, globalEportInfo, CPSS_DXCH_CFG_GLOBAL_EPORT_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, fdbEntryType, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, format, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_FORMAT_UNT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, updateType, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_TYPE_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, updateInfo, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_INFO_UNT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, entry, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, updateOnlySrcInterface);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, addNewMacUcEntries);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, addWithRehashEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, updateMovedMacUcEntries);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, macNoSpaceUpdatesEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, macRoutedLearningEnable);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, macVlanLookupMode, CPSS_DXCH_BRG_FDB_MANAGER_MAC_VLAN_LOOKUP_MODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, macNoSpaceUpdatesEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, thresholdB);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, thresholdC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, currentUsedEntries);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, noSpaceCache, CPSS_DXCH_BRG_FDB_MANAGER_NO_SPACE_CACHE_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, auMsgRate);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, auMsgRateLimitEn);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, hashTooLongEnabled);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, crcHashUpperBitsMode, CPSS_DXCH_BRG_FDB_MANAGER_CRC_HASH_UPPER_BITS_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ipv4PrefixLength);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ipv6PrefixLength);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, naMsgToCpuEnable);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, unkSrcAddrCmd, CPSS_PORT_LOCK_CMD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, learnPriority, CPSS_DXCH_FDB_LEARN_PRIORITY_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, userGroup);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, movedMacSaCmd, CPSS_PACKET_CMD_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, unicastIpv6RoutingEn);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, unicastIpv4RoutingEn);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddOkRehashingStage0);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddOkRehashingStage1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddOkRehashingStage2);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddOkRehashingStage3);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddOkRehashingStage4);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddOkRehashingStageMoreThanFour);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddErrorInputInvalid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddErrorBadState);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddErrorEntryExist);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddErrorTableFull);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddErrorHwUpdate);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddErrorPortLimit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddErrorTrunkLimit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddErrorGlobalEPortLimit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddErrorFidLimit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryAddErrorGlobalLimit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryUpdateOk);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryUpdateErrorInputInvalid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryUpdateErrorNotFound);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryUpdateErrorBadState);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryUpdateErrorHwUpdate);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryUpdateErrorPortLimit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryUpdateErrorTrunkLimit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryUpdateErrorGlobalEPortLimit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryUpdateErrorFidLimit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryDeleteOk);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryDeleteErrorInputInvalid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryDeleteErrorNotfound);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryDeleteErrorHwUpdate);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryDeleteErrorBadState);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryTempDeleteOk);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entryTempDeleteErrorInputInvalid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanTransplantOk);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanTransplantOkNoMore);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanTransplantErrorInputInvalid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanTransplantErrorFailedTableUpdate);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanTransplantTotalTransplantedEntries);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanTransplantTotalTransplantReachLimitDeletedEntries);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanDeleteOk);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanDeleteOkNoMore);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanDeleteErrorFailedTableUpdate);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanDeleteErrorInputInvalid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanDeleteTotalDeletedEntries);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanAgingOk);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanAgingErrorInputInvalid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanAgingErrorFailedTableUpdate);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanAgingTotalAgedOutEntries);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanAgingTotalAgedOutDeleteEntries);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanLearningOk);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanLearningOkNoMore);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanLearningErrorInputInvalid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanLearningErrorFailedTableUpdate);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanLearningTotalHwEntryNewMessages);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanLearningTotalHwEntryMovedMessages);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanLearningTotalHwEntryNoSpaceMessages);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanLearningTotalHwEntryNoSpaceFilteredMessages);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, haSyncTotalValid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, haSyncTotalInvalid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, haSyncTotalSPEntry);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, haSyncTotalIPv6DataInvalid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entriesRewriteOk);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entriesRewriteErrorInputInvalid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, entriesRewriteTotalRewrite);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, limitSetOkPortLimit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, limitSetOkTrunkLimit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, limitSetOkGlobalEPortLimit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, limitSetOkFidLimit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, limitSetOkGlobalLimit);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, limitSetErrorInput);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_TRANSPLANT_SCAN_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_TRANSPLANT_SCAN_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, transplantMacUcEntries);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, transplantIpUcEntries);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, transplantStaticEntries);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, fid);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, fidMask);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, oldInterface, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, newInterface, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, globalEport);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, trunkId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, fid);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, devPort, CPSS_INTERFACE_DEV_PORT_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_FORMAT_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_FORMAT_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, fdbEntryMacAddrFormat, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, fdbEntryIpv4McFormat, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, fdbEntryIpv6McFormat, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, fdbEntryIpv4UcFormat, CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, fdbEntryIpv6UcFormat, CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, type, CPSS_INTERFACE_TYPE_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, interfaceInfo, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_UNT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_INFO_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_INFO_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, newEntryFormat, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_NEW_FORMAT_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, movedEntryFormat, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_MOVED_FORMAT_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, movedReachLimitEntryFormat, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_MOVED_REACH_LIMIT_AND_DELETED_FORMAT_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_HW_CAPACITY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_HW_CAPACITY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfHwIndexes);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfHashes);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, mode, CPSS_MAC_HASH_FUNC_MODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_NO_SPACE_CACHE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_NO_SPACE_CACHE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, noSpaceEntryCache, 4, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, devPort, CPSS_INTERFACE_DEV_PORT_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, trunkId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vidx);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_IP_MC_FORMAT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, sipAddr, 4, GT_U8);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, dipAddr, 4, GT_U8);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, fid);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, dstInterface, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, daSecurityLevel);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, sourceID);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, userDefined);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, daCommand, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isStatic);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, age);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, daRoute);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, appSpecificCpuCode);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, mirrorToAnalyzerPort, CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, daQoSParameterSetIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, saQoSParameterSetIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vid1);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_MAC_ADDR_FORMAT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ETH_MAC(valPtr, macAddr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, fid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vid1);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, dstInterface, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, daSecurityLevel);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, saSecurityLevel);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, sourceID);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, userDefined);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, daCommand, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, saCommand, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isStatic);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, age);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, daRoute);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, appSpecificCpuCode);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, mirrorToAnalyzerPort, CPSS_DXCH_BRG_FDB_MANAGER_MIRROR_TO_ANALYZER_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, daQoSParameterSetIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, saQoSParameterSetIndex);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_MOVED_FORMAT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_MOVED_FORMAT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, oldInterface, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, oldsrcId);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_MOVED_REACH_LIMIT_AND_DELETED_FORMAT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_MOVED_REACH_LIMIT_AND_DELETED_FORMAT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, limitInfo, CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, limitValue);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_NEW_FORMAT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_NEW_FORMAT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tempEntryOffset);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_IPV4_MAC(valPtr, ipv4Addr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vrfId);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, age);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ucRouteType, CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ucRouteInfo, CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_UNT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ucCommonInfo, CPSS_DXCH_BRG_FDB_MANAGER_IP_UC_COMMON_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_IPV6_UC_ROUTING_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_IPV6_MAC(valPtr, ipv6Addr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vrfId);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, age);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ucRouteType, CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ucRouteInfo, CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_UNT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ucCommonInfo, CPSS_DXCH_BRG_FDB_MANAGER_IP_UC_COMMON_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_IP_UC_COMMON_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_IP_UC_COMMON_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, qosProfileMarkingEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, qosProfileIndex);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, qosProfilePrecedence, CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, modifyUp, CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, modifyDscp, CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, arpBcTrapMirrorEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dipAccessLevel);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ingressMirrorToAnalyzerEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ingressMirrorToAnalyzerIndex);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, fullFdbInfo, CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nextHopPointerToRouter);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ecmpPointerToRouter);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nextHopVlanId);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ttlHopLimitDecEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ttlHopLimDecOptionsExtChkByPass);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, countSet, CPSS_IP_CNT_SET_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ICMPRedirectEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, mtuProfileIndex);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, dstInterface, CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_INTERFACE_INFO_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, pointer, CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FIELD_POINTER_UNT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, ipv6ExtInfo, CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_IPV6_EXT_INFO_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_IPV6_EXT_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_IPV6_EXT_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, siteId, CPSS_IP_SITE_ID_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, scopeCheckingEnable);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FIELD_POINTER_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FIELD_POINTER_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tunnelStartPointer);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, natPointer);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, arpPointer);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_BRG_FDB_MANAGER_TRANSPLANT_SCAN_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_BRG_FDB_MANAGER_TRANSPLANT_SCAN_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_BRG_FDB_MANAGER_TRANSPLANT_SCAN_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC_PTR_agingPtr = {
     "agingPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC_PTR_capacityPtr = {
     "capacityPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC_PTR_agingPtr = {
     "agingPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC_PTR_entryAttrPtr = {
     "entryAttrPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC_PTR_learningPtr = {
     "learningPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC_PTR_checksPtr = {
     "checksPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC_PTR_limitInfoPtr = {
     "limitInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC_PTR_entryAttrPtr = {
     "entryAttrPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC_PTR_learningPtr = {
     "learningPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC_PTR_lookupPtr = {
     "lookupPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC_PTR_learningPtr = {
     "learningPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC_PTR_routingPtr = {
     "routingPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_TRANSPLANT_SCAN_PARAMS_STC_PTR_paramsPtr = {
     "paramsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_TRANSPLANT_SCAN_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_scanStart = {
     "scanStart", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_fdbManagerId = {
     "fdbManagerId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_limitValue = {
     "limitValue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_tempEntryOffset = {
     "tempEntryOffset", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC_PTR_agingPtr = {
     "agingPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC_PTR_capacityPtr = {
     "capacityPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC_PTR_countersPtr = {
     "countersPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT_PTR_resultArray = {
     "resultArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC_PTR_entryAttrPtr = {
     "entryAttrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC_PTR_entriesAgedoutArray = {
     "entriesAgedoutArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC_PTR_entriesDeletedArray = {
     "entriesDeletedArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC_PTR_entriesLearningArray = {
     "entriesLearningArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC_PTR_entriesTransplantedArray = {
     "entriesTransplantedArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC_PTR_learningPtr = {
     "learningPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC_PTR_dataPtr = {
     "dataPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC_PTR_lookupPtr = {
     "lookupPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC_PTR_learningPtr = {
     "learningPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC_PTR_routingPtr = {
     "routingPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC_PTR_statisticsPtr = {
     "statisticsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_currentValuePtr = {
     "currentValuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_entriesAgedoutNumPtr = {
     "entriesAgedoutNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_entriesDeletedNumPtr = {
     "entriesDeletedNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_entriesLearningNumPtr = {
     "entriesLearningNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_entriesTransplantedNumPtr = {
     "entriesTransplantedNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_errorNumPtr = {
     "errorNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_limitValuePtr = {
     "limitValuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_deviceListArray = {
     "deviceListArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerDelete_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerAgingScan_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_AGING_SCAN_PARAMS_STC_PTR_paramsPtr,
    &DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC_PTR_entriesAgedoutArray,
    &DX_OUT_GT_U32_PTR_entriesAgedoutNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerCreate_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC_PTR_capacityPtr,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC_PTR_entryAttrPtr,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC_PTR_learningPtr,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC_PTR_lookupPtr,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC_PTR_agingPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerConfigUpdate_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_ENTRY_ATTRIBUTES_STC_PTR_entryAttrPtr,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_LEARNING_STC_PTR_learningPtr,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_CONFIG_UPDATE_AGING_STC_PTR_agingPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerDatabaseCheck_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC_PTR_checksPtr,
    &DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT_PTR_resultArray,
    &DX_OUT_GT_U32_PTR_errorNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerLimitSet_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC_PTR_limitInfoPtr,
    &DX_IN_GT_U32_limitValue,
    &DX_OUT_GT_U32_PTR_currentValuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerLimitGet_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_STC_PTR_limitInfoPtr,
    &DX_OUT_GT_U32_PTR_limitValuePtr,
    &DX_OUT_GT_U32_PTR_currentValuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerEntryDelete_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerEntryAdd_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC_PTR_entryPtr,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC_PTR_paramsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerEntryUpdate_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC_PTR_entryPtr,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_PARAMS_STC_PTR_paramsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerEntryTempDelete_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC_PTR_entryPtr,
    &DX_IN_GT_U32_tempEntryOffset
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerLearningScan_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_SCAN_PARAMS_STC_PTR_paramsPtr,
    &DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC_PTR_entriesLearningArray,
    &DX_OUT_GT_U32_PTR_entriesLearningNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerEntryGetNext_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_IN_GT_BOOL_getFirst,
    &DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerDeleteScan_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_IN_GT_BOOL_scanStart,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_DELETE_SCAN_PARAMS_STC_PTR_paramsPtr,
    &DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC_PTR_entriesDeletedArray,
    &DX_OUT_GT_U32_PTR_entriesDeletedNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerTransplantScan_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_IN_GT_BOOL_scanStart,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_TRANSPLANT_SCAN_PARAMS_STC_PTR_paramsPtr,
    &DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_UPDATE_EVENT_STC_PTR_entriesTransplantedArray,
    &DX_OUT_GT_U32_PTR_entriesTransplantedNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerEntryRewrite_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_IN_GT_U32_PTR_entriesIndexesArray,
    &DX_IN_GT_U32_entriesIndexesNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerDevListAdd_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_IN_GT_U8_PTR_devListArr,
    &DX_IN_GT_U32_numOfDevs
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerEntryGet_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_INOUT_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerDevListGet_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_INOUT_GT_U32_PTR_numOfDevicesPtr,
    &DX_OUT_GT_U8_PTR_deviceListArray
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerConfigGet_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_CAPACITY_STC_PTR_capacityPtr,
    &DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ATTRIBUTES_STC_PTR_entryAttrPtr,
    &DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_STC_PTR_learningPtr,
    &DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_LOOKUP_STC_PTR_lookupPtr,
    &DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_AGING_STC_PTR_agingPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerCountersGet_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_COUNTERS_STC_PTR_countersPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerLearningThrottlingGet_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_LEARNING_THROTTLING_STC_PTR_dataPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerStatisticsGet_PARAMS[] =  {
    &DX_IN_GT_U32_fdbManagerId,
    &DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC_PTR_statisticsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerPortLearningSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC_PTR_learningPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerPortRoutingSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC_PTR_routingPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerPortLearningGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_PORT_LEARNING_STC_PTR_learningPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChBrgFdbManagerPortRoutingGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_BRG_FDB_MANAGER_PORT_ROUTING_STC_PTR_routingPtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChBridgeFdbManagerLogLibDb[] = {
    {"cpssDxChBrgFdbManagerCreate", 6, cpssDxChBrgFdbManagerCreate_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerDelete", 1, cpssDxChBrgFdbManagerDelete_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerEntryAdd", 3, cpssDxChBrgFdbManagerEntryAdd_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerEntryDelete", 2, cpssDxChBrgFdbManagerEntryDelete_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerEntryTempDelete", 3, cpssDxChBrgFdbManagerEntryTempDelete_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerDevListAdd", 3, cpssDxChBrgFdbManagerDevListAdd_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerDevListRemove", 3, cpssDxChBrgFdbManagerDevListAdd_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerEntryGet", 2, cpssDxChBrgFdbManagerEntryGet_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerEntryGetNext", 3, cpssDxChBrgFdbManagerEntryGetNext_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerEntryUpdate", 3, cpssDxChBrgFdbManagerEntryUpdate_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerConfigGet", 6, cpssDxChBrgFdbManagerConfigGet_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerDevListGet", 3, cpssDxChBrgFdbManagerDevListGet_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerPortLearningSet", 3, cpssDxChBrgFdbManagerPortLearningSet_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerPortLearningGet", 3, cpssDxChBrgFdbManagerPortLearningGet_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerPortRoutingSet", 3, cpssDxChBrgFdbManagerPortRoutingSet_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerPortRoutingGet", 3, cpssDxChBrgFdbManagerPortRoutingGet_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerLearningScan", 4, cpssDxChBrgFdbManagerLearningScan_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerLearningThrottlingGet", 2, cpssDxChBrgFdbManagerLearningThrottlingGet_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerCountersGet", 2, cpssDxChBrgFdbManagerCountersGet_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerStatisticsGet", 2, cpssDxChBrgFdbManagerStatisticsGet_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerStatisticsClear", 1, cpssDxChBrgFdbManagerDelete_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerAgingScan", 4, cpssDxChBrgFdbManagerAgingScan_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerTransplantScan", 5, cpssDxChBrgFdbManagerTransplantScan_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerDeleteScan", 5, cpssDxChBrgFdbManagerDeleteScan_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerDatabaseCheck", 4, cpssDxChBrgFdbManagerDatabaseCheck_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerHighAvailabilityEntriesSync", 1, cpssDxChBrgFdbManagerDelete_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerEntryRewrite", 3, cpssDxChBrgFdbManagerEntryRewrite_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerLimitSet", 4, cpssDxChBrgFdbManagerLimitSet_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerLimitGet", 4, cpssDxChBrgFdbManagerLimitGet_PARAMS, NULL},
    {"cpssDxChBrgFdbManagerConfigUpdate", 4, cpssDxChBrgFdbManagerConfigUpdate_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_BRIDGE_FDB_MANAGER(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChBridgeFdbManagerLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChBridgeFdbManagerLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

