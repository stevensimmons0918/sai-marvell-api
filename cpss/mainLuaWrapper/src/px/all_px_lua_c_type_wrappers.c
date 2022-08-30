/*************************************************************************
* /local/store/cpss_regression/tmp/all_px_lua_c_type_wrappers.c
*
* DESCRIPTION:
*       A lua type wrapper
*       It implements support for the following types:
*           struct  CPSS_PX_PTP_TYPE_KEY_FORMAT_STC
*           enum    CPSS_PX_PORT_MAC_COUNTER_ENT
*           struct  CPSS_PX_PORT_AP_STATS_STC
*           struct  CPSS_PX_LOGICAL_TABLE_INFO_STC
*           struct  CPSS_PX_EXTENDED_DSA_FORWARD_STC
*           struct  CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC
*           struct  CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC
*           struct  CPSS_PX_UDB_PAIR_DATA_STC
*           enum    CPSS_PX_PTP_TAI_TOD_TYPE_ENT
*           struct  CPSS_PX_PORT_CN_PROFILE_CONFIG_STC
*           enum    CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT
*           struct  CPSS_PX_PORT_TX_SHAPER_CONFIG_STC
*           enum    CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT
*           enum    CPSS_PX_PORT_AP_FLOW_CONTROL_ENT
*           enum    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT
*           enum    CPSS_PX_PORT_PFC_ENABLE_ENT
*           enum    CPSS_PX_PORT_MAPPING_TYPE_ENT
*           enum    CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT
*           enum    CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT
*           struct  CPSS_PX_DIAG_BIST_RESULT_STC
*           struct  CPSS_PX_LOGICAL_TABLES_SHADOW_STC
*           struct  CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC
*           struct  CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC
*           enum    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT
*           enum    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT
*           struct  CPSS_PX_INGRESS_HASH_UDBP_STC
*           enum    CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT
*           union   CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT
*           struct  CPSS_PX_PORT_ECN_ENABLERS_STC
*           struct  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC
*           enum    CPSS_PX_PP_SERDES_REF_CLOCK_ENT
*           struct  CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC
*           struct  CPSS_PX_UDB_KEY_STC
*           enum    CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT
*           struct  CPSS_PX_PORT_AP_INTROP_STC
*           enum    CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT
*           enum    CPSS_PX_PTP_TAI_CLOCK_MODE_ENT
*           struct  CPSS_PX_LOCATION_SPECIFIC_INFO_STC
*           enum    CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT
*           struct  CPSS_PX_LED_CLASS_MANIPULATION_STC
*           struct  CPSS_PX_PORT_MAP_STC
*           struct  CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC
*           struct  CPSS_PX_PTP_TYPE_KEY_STC
*           enum    CPSS_PX_CNC_COUNTER_FORMAT_ENT
*           enum    CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT
*           struct  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC
*           enum    CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT
*           struct  CPSS_PX_HW_INDEX_INFO_STC
*           enum    CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT
*           struct  CPSS_PX_DETAILED_PORT_MAP_STC
*           enum    CPSS_PX_PORT_CN_PACKET_LENGTH_ENT
*           enum    CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT
*           struct  CPSS_PX_COS_FORMAT_ENTRY_STC
*           union   CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT
*           struct  CPSS_PX_UDB_PAIR_KEY_STC
*           struct  CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC
*           struct  CPSS_PX_REGULAR_DSA_FORWARD_STC
*           enum    CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT
*           struct  CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC
*           struct  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_PTP_DATA_STC
*           enum    CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT
*           union   CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT
*           enum    CPSS_PX_CNC_BYTE_COUNT_MODE_ENT
*           enum    CPSS_PX_COS_MODE_ENT
*           struct  CPSS_PX_SHADOW_PORT_MAP_STC
*           struct  CPSS_PX_LOGICAL_TABLE_SHADOW_INFO_STC
*           enum    CPSS_PX_INGRESS_ETHERTYPE_ENT
*           struct  CPSS_PX_PTP_TAI_TOD_STEP_STC
*           enum    CPSS_PX_PORT_FC_MODE_ENT
*           enum    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT
*           enum    CPSS_PX_LOCATION_ENT
*           enum    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT
*           struct  CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC
*           enum    CPSS_PX_PORT_TX_SHAPER_GRANULARITY_ENT
*           enum    CPSS_PX_DIAG_TRANSMIT_MODE_ENT
*           struct  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC
*           union   CPSS_PX_LOCATION_UNT
*           struct  CPSS_PX_PORT_EGRESS_CNTR_STC
*           struct  CPSS_PX_LED_CONF_STC
*           struct  CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC
*           enum    CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT
*           enum    CPSS_PX_CNC_CLIENT_ENT
*           struct  CPSS_PX_CNC_COUNTER_STC
*           enum    CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT
*           struct  CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC
*           struct  CPSS_PX_PTP_TSU_CONTROL_STC
*           enum    CPSS_PX_DIAG_BIST_STATUS_ENT
*           struct  CPSS_PX_INGRESS_TPID_ENTRY_STC
*           struct  CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC
*           struct  CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC
*           struct  CPSS_PX_DIAG_PG_CONFIGURATIONS_STC
*           enum    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT
*           struct  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC
*           struct  CPSS_PX_EDSA_FORWARD_STC
*           enum    CPSS_PX_INGRESS_HASH_MODE_ENT
*           struct  CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC
*           struct  CPSS_PX_PTP_TAI_TOD_COUNT_STC
*           struct  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_STC
*           enum    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT
*           enum    CPSS_PX_PA_UNIT_ENT
*           struct  CPSS_PX_COS_ATTRIBUTES_STC
*           struct  CPSS_PX_EGRESS_SRC_PORT_INFO_STC
*           enum    CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT
*           enum    CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT
*           struct  CPSS_PX_PORT_SERDES_EYE_INPUT_STC
*           struct  CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC
*           struct  CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC
*           enum    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT
*           struct  CPSS_PX_CFG_DEV_INFO_STC
*           struct  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC
*           enum    CPSS_PX_PTP_TAI_CLOCK_FREQ_ENT
*           struct  CPSS_PX_VLAN_TAG_STC
*           enum    CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT
*           struct  CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC
*           struct  CPSS_PX_PORT_ALIGN90_PARAMS_STC
*           struct  CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC
*           enum    CPSS_PX_MIN_SPEED_ENT
*           struct  CPSS_PX_NET_TX_PARAMS_STC
*           enum    CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT
*           struct  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC
*           enum    CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT
*           enum    CPSS_PX_SHADOW_TYPE_ENT
*           enum    CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT
*           enum    CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_ENT
*           enum    CPSS_PX_PORT_TX_SHAPER_MODE_ENT
*           enum    CPSS_PX_POWER_SUPPLIES_NUMBER_ENT
*           enum    CPSS_PX_PHA_FIRMWARE_TYPE_ENT
*           enum    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT
*           enum    CPSS_PX_PORT_PFC_COUNT_MODE_ENT
*           struct  CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC
*           enum    CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT
*           struct  CPSS_PX_EGRESS_SOURCE_PORT_PRE_DA_STC
*           enum    CPSS_PX_PORT_CTLE_BIAS_MODE_ENT
*           struct  CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TO_UPSTREAM_PORT_STC
*           struct  CPSS_PX_LOCATION_FULL_INFO_STC
*           struct  CPSS_PX_RAM_INDEX_INFO_STC
*           struct  CPSS_PX_NET_SDMA_RX_COUNTERS_STC
*           enum    CPSS_PX_CFG_CNTR_ENT
*           union   CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT
*           enum    CPSS_PX_LOGICAL_TABLE_ENT
*           struct  CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC
*           struct  CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC
*           enum    CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT
*           struct  CPSS_PX_PORT_AP_PARAMS_STC
*           struct  CPSS_PX_PORT_AP_STATUS_STC
*           enum    CPSS_PX_TABLE_ENT
*           enum    CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT
*           struct  CPSS_PX_PORT_MAC_COUNTERS_STC
*           enum    CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT
*
* DEPENDENCIES:
*
* COMMENTS:
*       Generated at Wed Dec  1 21:26:01 2021
*       /local/store/cpss_regression/cpss/mainLuaWrapper/scripts/make_type_wrapper.py -i /local/store/cpss_regression/tmp/type_info_file_px -p /local/store/cpss_regression/cpss/mainLuaWrapper/scripts/already_implemented.lst -X /local/store/cpss_regression/cpss/mainLuaWrapper/scripts/excludelist -C /local/store/cpss_regression/cpss -M /local/store/cpss_regression/cpss/mainLuaWrapper/scripts/lua_C_mapping -N -F px -o /local/store/cpss_regression/tmp/all_px_lua_c_type_wrappers.c
*
* FILE REVISION NUMBER:
*       $Revision: 15 $
**************************************************************************/
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <cpss/px/ptp/cpssPxPtp.h>
#include <cpss/px/port/cpssPxPortStat.h>
#include <cpss/px/port/cpssPxPortAp.h>
#include <cpss/px/diag/cpssPxDiagDataIntegrity.h>
#include <cpss/px/egress/cpssPxEgress.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/px/ingress/cpssPxIngress.h>
#include <cpss/px/port/cpssPxPortCn.h>
#include <cpss/px/diag/cpssPxDiag.h>
#include <cpss/common/cpssHwInit/cpssCommonLedCtrl.h>
#include <cpss/common/port/cpssPortCtrl.h>
#include <cpss/px/port/cpssPxPortTxShaper.h>
#include <cpss/px/ingress/cpssPxIngressHash.h>
#include <cpss/px/port/cpssPxPortPfc.h>
#include <cpss/px/port/cpssPxPortMapping.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/cpssHwInit/cpssPxTables.h>
#include <cpss/px/port/PizzaArbiter/cpssPxPortPizzaArbiter.h>
#include <cpss/px/port/cpssPxPortTxTailDrop.h>
#include <cpss/common/diag/cpssCommonDiag.h>
#include <cpss/px/port/cpssPxPortTxScheduler.h>
#include <cpss/px/port/cpssPxPortEcn.h>
#include <cpss/px/cnc/cpssPxCnc.h>
#include <cpss/px/cpssHwInit/cpssPxHwInitLedCtrl.h>
#include <cpss/common/cos/cpssCosTypes.h>
#include <cpss/common/diag/cpssDiagPacketGenerator.h>
#include <cpss/px/cos/cpssPxCos.h>
#include <cpss/px/diag/cpssPxDiagDataIntegrityTables.h>
#include <cpss/px/port/cpssPxPortSyncEther.h>
#include <cpss/px/diag/cpssPxDiagPacketGenerator.h>
#include <cpss/common/config/cpssGenCfg.h>
#include <cpss/px/config/cpssPxCfgInit.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/cpssPxPortDynamicPAUnitBW.h>
#include <cpss/px/networkIf/cpssPxNetIfTypes.h>
#include <cpss/px/cpssHwInit/cpssPxHwInit.h>
#include <cpss/extServices/os/gtOs/gtOs.h>

/***** declarations ********/

use_prv_struct(CPSS_PX_PTP_TYPE_KEY_FORMAT_STC);
use_prv_struct(CPSS_PX_PORT_AP_STATS_STC);
use_prv_struct(CPSS_PX_LOGICAL_TABLE_INFO_STC);
use_prv_struct(CPSS_PX_EXTENDED_DSA_FORWARD_STC);
use_prv_struct(CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC);
use_prv_struct(CPSS_802_1BR_ETAG_STC);
use_prv_struct(CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC);
use_prv_struct(CPSS_PX_UDB_PAIR_DATA_STC);
use_prv_struct(CPSS_PX_PORT_CN_PROFILE_CONFIG_STC);
use_prv_struct(CPSS_PORT_MODE_SPEED_STC);
use_prv_struct(CPSS_PX_PORT_TX_SHAPER_CONFIG_STC);
use_prv_struct(CPSS_PX_INGRESS_IP2ME_ENTRY_STC);
use_prv_struct(CPSS_PX_DIAG_BIST_RESULT_STC);
use_prv_struct(CPSS_PX_LOGICAL_TABLES_SHADOW_STC);
use_prv_struct(CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC);
use_prv_struct(CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC);
use_prv_struct(CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC);
use_prv_struct(CPSS_PX_INGRESS_HASH_UDBP_STC);
use_prv_struct(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT);
use_prv_struct(CPSS_PX_PORT_ECN_ENABLERS_STC);
use_prv_struct(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC);
use_prv_struct(CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC);
use_prv_struct(CPSS_PX_UDB_KEY_STC);
use_prv_struct(CPSS_PX_PORT_AP_INTROP_STC);
use_prv_struct(CPSS_PX_LOCATION_SPECIFIC_INFO_STC);
use_prv_struct(CPSS_PX_LED_CLASS_MANIPULATION_STC);
use_prv_struct(CPSS_PX_PORT_MAP_STC);
use_prv_struct(CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC);
use_prv_struct(CPSS_PX_PTP_TYPE_KEY_STC);
use_prv_struct(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC);
use_prv_struct(CPSS_PX_HW_INDEX_INFO_STC);
use_prv_struct(CPSS_PX_DETAILED_PORT_MAP_STC);
use_prv_struct(CPSS_PX_PORT_SERDES_EYE_RESULT_STC);
use_prv_struct(CPSS_PX_COS_FORMAT_ENTRY_STC);
use_prv_struct(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT);
use_prv_struct(CPSS_PX_UDB_PAIR_KEY_STC);
use_prv_struct(CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC);
use_prv_struct(CPSS_PX_REGULAR_DSA_FORWARD_STC);
use_prv_struct(CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC);
use_prv_struct(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_PTP_DATA_STC);
use_prv_struct(CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT);
use_prv_struct(CPSS_PX_SHADOW_PORT_MAP_STC);
use_prv_struct(CPSS_PX_LOGICAL_TABLE_SHADOW_INFO_STC);
use_prv_struct(CPSS_PX_PTP_TAI_TOD_STEP_STC);
use_prv_struct(CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC);
use_prv_struct(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC);
use_prv_struct(CPSS_PX_LOCATION_UNT);
use_prv_struct(CPSS_PX_PORT_EGRESS_CNTR_STC);
use_prv_struct(CPSS_PX_LED_CONF_STC);
use_prv_struct(CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC);
use_prv_struct(CPSS_PX_DIAG_PG_CONFIGURATIONS_STC);
use_prv_struct(CPSS_PX_CNC_COUNTER_STC);
use_prv_struct(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC);
use_prv_struct(CPSS_PX_PTP_TSU_CONTROL_STC);
use_prv_struct(CPSS_PX_INGRESS_TPID_ENTRY_STC);
use_prv_struct(CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC);
use_prv_struct(CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC);
use_prv_struct(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC);
use_prv_struct(CPSS_PX_EDSA_FORWARD_STC);
use_prv_struct(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC);
use_prv_struct(CPSS_PX_PTP_TAI_TOD_COUNT_STC);
use_prv_struct(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_STC);
use_prv_struct(CPSS_PX_COS_ATTRIBUTES_STC);
use_prv_struct(CPSS_PX_EGRESS_SRC_PORT_INFO_STC);
use_prv_struct(CPSS_PX_PORT_SERDES_EYE_INPUT_STC);
use_prv_struct(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC);
use_prv_struct(GT_ETHERADDR);
use_prv_struct(CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC);
use_prv_struct(CPSS_GEN_CFG_DEV_INFO_STC);
use_prv_struct(CPSS_PX_CFG_DEV_INFO_STC);
use_prv_struct(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC);
use_prv_struct(CPSS_PX_VLAN_TAG_STC);
use_prv_struct(CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC);
use_prv_struct(CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC);
use_prv_struct(CPSS_PX_PORT_ALIGN90_PARAMS_STC);
use_prv_struct(CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC);
use_prv_struct(CPSS_PX_NET_TX_PARAMS_STC);
use_prv_struct(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC);
use_prv_struct(CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC);
use_prv_struct(CPSS_PX_EGRESS_SOURCE_PORT_PRE_DA_STC);
use_prv_struct(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TO_UPSTREAM_PORT_STC);
use_prv_struct(CPSS_PX_LOCATION_FULL_INFO_STC);
use_prv_struct(CPSS_PX_RAM_INDEX_INFO_STC);
use_prv_struct(CPSS_PX_NET_SDMA_RX_COUNTERS_STC);
use_prv_struct(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT);
use_prv_struct(CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC);
use_prv_struct(CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC);
use_prv_struct(GT_U64);
use_prv_struct(CPSS_PX_PORT_AP_PARAMS_STC);
use_prv_struct(CPSS_PX_PORT_AP_STATUS_STC);
use_prv_struct(CPSS_PX_PORT_MAC_COUNTERS_STC);

/***** declarations ********/

void prv_lua_to_c_CPSS_PX_PTP_TYPE_KEY_FORMAT_STC(
    lua_State *L,
    CPSS_PX_PTP_TYPE_KEY_FORMAT_STC *val
)
{
    F_BOOL(val, -1, isUdp);
    F_NUMBER(val, -1, srcPortProfile, GT_U32);
    F_ARRAY_START(val, -1, udbPairsArr);
    {
        int idx;
        for (idx = 0; idx < 2; idx++) {
            F_ARRAY_STRUCT(val, udbPairsArr, idx, CPSS_PX_UDB_PAIR_DATA_STC);
        }
    }
    F_ARRAY_END(val, -1, udbPairsArr);
    F_ARRAY_START(val, -1, udbArr);
    {
        int idx;
        for (idx = 0; idx < 6; idx++) {
            F_ARRAY_NUMBER(val, udbArr, idx, GT_U8);
        }
    }
    F_ARRAY_END(val, -1, udbArr);
}

void prv_c_to_lua_CPSS_PX_PTP_TYPE_KEY_FORMAT_STC(
    lua_State *L,
    CPSS_PX_PTP_TYPE_KEY_FORMAT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, isUdp);
    FO_NUMBER(val, t, srcPortProfile, GT_U32);
    FO_ARRAY_START(val, t, udbPairsArr);
    {
        int idx;
        for (idx = 0; idx < 2; idx++) {
            FO_ARRAY_STRUCT(val, udbPairsArr, idx, CPSS_PX_UDB_PAIR_DATA_STC);
        }
    }
    FO_ARRAY_END(val, t, udbPairsArr);
    FO_ARRAY_START(val, t, udbArr);
    {
        int idx;
        for (idx = 0; idx < 6; idx++) {
            FO_ARRAY_NUMBER(val, udbArr, idx, GT_U8);
        }
    }
    FO_ARRAY_END(val, t, udbArr);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PTP_TYPE_KEY_FORMAT_STC);

add_mgm_enum(CPSS_PX_PORT_MAC_COUNTER_ENT);

void prv_lua_to_c_CPSS_PX_PORT_AP_STATS_STC(
    lua_State *L,
    CPSS_PX_PORT_AP_STATS_STC *val
)
{
    F_NUMBER(val, -1, txDisCnt, GT_U16);
    F_NUMBER(val, -1, abilityCnt, GT_U16);
    F_NUMBER(val, -1, abilitySuccessCnt, GT_U16);
    F_NUMBER(val, -1, linkFailCnt, GT_U16);
    F_NUMBER(val, -1, linkSuccessCnt, GT_U16);
    F_NUMBER(val, -1, hcdResoultionTime, GT_U32);
    F_NUMBER(val, -1, linkUpTime, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PORT_AP_STATS_STC(
    lua_State *L,
    CPSS_PX_PORT_AP_STATS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, txDisCnt, GT_U16);
    FO_NUMBER(val, t, abilityCnt, GT_U16);
    FO_NUMBER(val, t, abilitySuccessCnt, GT_U16);
    FO_NUMBER(val, t, linkFailCnt, GT_U16);
    FO_NUMBER(val, t, linkSuccessCnt, GT_U16);
    FO_NUMBER(val, t, hcdResoultionTime, GT_U32);
    FO_NUMBER(val, t, linkUpTime, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_AP_STATS_STC);

void prv_lua_to_c_CPSS_PX_LOGICAL_TABLE_INFO_STC(
    lua_State *L,
    CPSS_PX_LOGICAL_TABLE_INFO_STC *val
)
{
    F_ENUM(val, -1, logicalTableType, CPSS_PX_LOGICAL_TABLE_ENT);
    F_NUMBER(val, -1, logicalTableEntryIndex, GT_U32);
}

void prv_c_to_lua_CPSS_PX_LOGICAL_TABLE_INFO_STC(
    lua_State *L,
    CPSS_PX_LOGICAL_TABLE_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, logicalTableType, CPSS_PX_LOGICAL_TABLE_ENT);
    FO_NUMBER(val, t, logicalTableEntryIndex, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_LOGICAL_TABLE_INFO_STC);

void prv_lua_to_c_CPSS_PX_EXTENDED_DSA_FORWARD_STC(
    lua_State *L,
    CPSS_PX_EXTENDED_DSA_FORWARD_STC *val
)
{
    F_BOOL(val, -1, srcTagged);
    F_NUMBER(val, -1, hwSrcDev, GT_HW_DEV_NUM);
    F_NUMBER(val, -1, srcPortOrTrunk, GT_U32);
    F_BOOL(val, -1, srcIsTrunk);
    F_NUMBER(val, -1, cfi, GT_U32);
    F_NUMBER(val, -1, up, GT_U32);
    F_NUMBER(val, -1, vid, GT_U32);
    F_BOOL(val, -1, egrFilterRegistered);
    F_BOOL(val, -1, dropOnSource);
    F_BOOL(val, -1, packetIsLooped);
    F_BOOL(val, -1, wasRouted);
    F_NUMBER(val, -1, srcId, GT_U32);
    F_NUMBER(val, -1, qosProfileIndex, GT_U32);
    F_BOOL(val, -1, useVidx);
    F_NUMBER(val, -1, trgVidx, GT_U32);
    F_NUMBER(val, -1, trgPort, GT_U32);
    F_NUMBER(val, -1, hwTrgDev, GT_HW_DEV_NUM);
}

void prv_c_to_lua_CPSS_PX_EXTENDED_DSA_FORWARD_STC(
    lua_State *L,
    CPSS_PX_EXTENDED_DSA_FORWARD_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, srcTagged);
    FO_NUMBER(val, t, hwSrcDev, GT_HW_DEV_NUM);
    FO_NUMBER(val, t, srcPortOrTrunk, GT_U32);
    FO_BOOL(val, t, srcIsTrunk);
    FO_NUMBER(val, t, cfi, GT_U32);
    FO_NUMBER(val, t, up, GT_U32);
    FO_NUMBER(val, t, vid, GT_U32);
    FO_BOOL(val, t, egrFilterRegistered);
    FO_BOOL(val, t, dropOnSource);
    FO_BOOL(val, t, packetIsLooped);
    FO_BOOL(val, t, wasRouted);
    FO_NUMBER(val, t, srcId, GT_U32);
    FO_NUMBER(val, t, qosProfileIndex, GT_U32);
    FO_BOOL(val, t, useVidx);
    FO_NUMBER(val, t, trgVidx, GT_U32);
    FO_NUMBER(val, t, trgPort, GT_U32);
    FO_NUMBER(val, t, hwTrgDev, GT_HW_DEV_NUM);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EXTENDED_DSA_FORWARD_STC);

void prv_lua_to_c_CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC(
    lua_State *L,
    CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC *val
)
{
    F_ENUM(val, -1, eventsType, CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT);
    F_STRUCT(val, -1, location, CPSS_PX_LOCATION_FULL_INFO_STC);
    F_ENUM(val, -1, memoryUseType, CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_ENT);
    F_ENUM(val, -1, correctionMethod, CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_ENT);
}

void prv_c_to_lua_CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC(
    lua_State *L,
    CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, eventsType, CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT);
    FO_STRUCT(val, t, location, CPSS_PX_LOCATION_FULL_INFO_STC);
    FO_ENUM(val, t, memoryUseType, CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_ENT);
    FO_ENUM(val, t, correctionMethod, CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC);

void prv_lua_to_c_CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC(
    lua_State *L,
    CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC *val
)
{
    F_NUMBER(val, -1, srcPortNum, GT_U32);
    F_STRUCT(val, -1, ptpPortInfo, CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC);
}

void prv_c_to_lua_CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC(
    lua_State *L,
    CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, srcPortNum, GT_U32);
    FO_STRUCT(val, t, ptpPortInfo, CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC);

void prv_lua_to_c_CPSS_PX_UDB_PAIR_DATA_STC(
    lua_State *L,
    CPSS_PX_UDB_PAIR_DATA_STC *val
)
{
    F_ARRAY_START(val, -1, udb);
    {
        int idx;
        for (idx = 0; idx < 2; idx++) {
            F_ARRAY_NUMBER(val, udb, idx, GT_U8);
        }
    }
    F_ARRAY_END(val, -1, udb);
}

void prv_c_to_lua_CPSS_PX_UDB_PAIR_DATA_STC(
    lua_State *L,
    CPSS_PX_UDB_PAIR_DATA_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, udb);
    {
        int idx;
        for (idx = 0; idx < 2; idx++) {
            FO_ARRAY_NUMBER(val, udb, idx, GT_U8);
        }
    }
    FO_ARRAY_END(val, t, udb);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_UDB_PAIR_DATA_STC);

add_mgm_enum(CPSS_PX_PTP_TAI_TOD_TYPE_ENT);

void prv_lua_to_c_CPSS_PX_PORT_CN_PROFILE_CONFIG_STC(
    lua_State *L,
    CPSS_PX_PORT_CN_PROFILE_CONFIG_STC *val
)
{
    F_BOOL(val, -1, cnAware);
    F_NUMBER(val, -1, threshold, GT_U32);
    F_ENUM(val, -1, alpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
}

void prv_c_to_lua_CPSS_PX_PORT_CN_PROFILE_CONFIG_STC(
    lua_State *L,
    CPSS_PX_PORT_CN_PROFILE_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, cnAware);
    FO_NUMBER(val, t, threshold, GT_U32);
    FO_ENUM(val, t, alpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_CN_PROFILE_CONFIG_STC);

add_mgm_enum(CPSS_PX_DIAG_TEMPERATURE_SENSOR_ENT);

void prv_lua_to_c_CPSS_PX_PORT_TX_SHAPER_CONFIG_STC(
    lua_State *L,
    CPSS_PX_PORT_TX_SHAPER_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, tokensRate, GT_U32);
    F_NUMBER(val, -1, slowRateRatio, GT_U32);
    F_ENUM(val, -1, tokensRateGran, CPSS_PX_PORT_TX_SHAPER_GRANULARITY_ENT);
    F_NUMBER(val, -1, portsPacketLength, GT_U32);
    F_NUMBER(val, -1, cpuPacketLength, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PORT_TX_SHAPER_CONFIG_STC(
    lua_State *L,
    CPSS_PX_PORT_TX_SHAPER_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, tokensRate, GT_U32);
    FO_NUMBER(val, t, slowRateRatio, GT_U32);
    FO_ENUM(val, t, tokensRateGran, CPSS_PX_PORT_TX_SHAPER_GRANULARITY_ENT);
    FO_NUMBER(val, t, portsPacketLength, GT_U32);
    FO_NUMBER(val, t, cpuPacketLength, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_TX_SHAPER_CONFIG_STC);

add_mgm_enum(CPSS_PX_INGRESS_HASH_LAG_TABLE_MODE_ENT);

add_mgm_enum(CPSS_PX_PORT_AP_FLOW_CONTROL_ENT);

add_mgm_enum(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT);

add_mgm_enum(CPSS_PX_PORT_PFC_ENABLE_ENT);

add_mgm_enum(CPSS_PX_PORT_MAPPING_TYPE_ENT);

add_mgm_enum(CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT);

add_mgm_enum(CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT);

void prv_lua_to_c_CPSS_PX_DIAG_BIST_RESULT_STC(
    lua_State *L,
    CPSS_PX_DIAG_BIST_RESULT_STC *val
)
{
    F_ENUM(val, -1, memType, CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT);
    F_STRUCT(val, -1, location, CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC);
}

void prv_c_to_lua_CPSS_PX_DIAG_BIST_RESULT_STC(
    lua_State *L,
    CPSS_PX_DIAG_BIST_RESULT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, memType, CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT);
    FO_STRUCT(val, t, location, CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_DIAG_BIST_RESULT_STC);

void prv_lua_to_c_CPSS_PX_LOGICAL_TABLES_SHADOW_STC(
    lua_State *L,
    CPSS_PX_LOGICAL_TABLES_SHADOW_STC *val
)
{
    F_ENUM(val, -1, logicalTableName, CPSS_PX_LOGICAL_TABLE_ENT);
    F_ENUM(val, -1, shadowType, CPSS_PX_SHADOW_TYPE_ENT);
}

void prv_c_to_lua_CPSS_PX_LOGICAL_TABLES_SHADOW_STC(
    lua_State *L,
    CPSS_PX_LOGICAL_TABLES_SHADOW_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, logicalTableName, CPSS_PX_LOGICAL_TABLE_ENT);
    FO_ENUM(val, t, shadowType, CPSS_PX_SHADOW_TYPE_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_LOGICAL_TABLES_SHADOW_STC);

void prv_lua_to_c_CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC(
    lua_State *L,
    CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC *val
)
{
    F_NUMBER(val, -1, tpid, GT_U16);
    F_NUMBER(val, -1, pcid, GT_U32);
    F_NUMBER(val, -1, egressDelay, GT_U32);
}

void prv_c_to_lua_CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC(
    lua_State *L,
    CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, tpid, GT_U16);
    FO_NUMBER(val, t, pcid, GT_U32);
    FO_NUMBER(val, t, egressDelay, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC);

void prv_lua_to_c_CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC(
    lua_State *L,
    CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC *val
)
{
    F_ENUM(val, -1, devType, CPSS_PP_FAMILY_TYPE_ENT);
    lua_getfield(L, -1, "devState");
    if (lua_istable(L, -1))
    {
        F_STRUCT(&(val->devState), -1, pipe, CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC);
    }
    lua_pop(L, 1);
}

void prv_c_to_lua_CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC(
    lua_State *L,
    CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, devType, CPSS_PP_FAMILY_TYPE_ENT);
    {
        int t1;
        lua_newtable(L);
        t1 = lua_gettop(L);
            FO_STRUCT(&(val->devState), t1, pipe, CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC);
        lua_setfield(L, t, "devState");
    }
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC);

add_mgm_enum(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT);

add_mgm_enum(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT);

void prv_lua_to_c_CPSS_PX_INGRESS_HASH_UDBP_STC(
    lua_State *L,
    CPSS_PX_INGRESS_HASH_UDBP_STC *val
)
{
    F_ENUM(val, -1, anchor, CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT);
    F_NUMBER(val, -1, offset, GT_U32);
    F_ARRAY_START(val, -1, nibbleMaskArr);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            F_ARRAY_BOOL(val, nibbleMaskArr, idx, GT_BOOL);
        }
    }
    F_ARRAY_END(val, -1, nibbleMaskArr);
}

void prv_c_to_lua_CPSS_PX_INGRESS_HASH_UDBP_STC(
    lua_State *L,
    CPSS_PX_INGRESS_HASH_UDBP_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, anchor, CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT);
    FO_NUMBER(val, t, offset, GT_U32);
    FO_ARRAY_START(val, t, nibbleMaskArr);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            FO_ARRAY_BOOL(val, nibbleMaskArr, idx, GT_BOOL);
        }
    }
    FO_ARRAY_END(val, t, nibbleMaskArr);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_INGRESS_HASH_UDBP_STC);

add_mgm_enum(CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT);

void prv_lua_to_c_CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT(
    lua_State *L,
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT *val
)
{
    F_UNION_MEMBER_STRUCT(val, -1, info_common, CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC);
    F_UNION_MEMBER_NUMBER(val, -1, notNeeded, GT_U32);
}

void prv_c_to_lua_CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT(
    lua_State *L,
    CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, info_common, CPSS_PX_EGRESS_TARGET_PORT_COMMON_STC);
    FO_NUMBER(val, t, notNeeded, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT);
add_mgm_union(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_UNT);

void prv_lua_to_c_CPSS_PX_PORT_ECN_ENABLERS_STC(
    lua_State *L,
    CPSS_PX_PORT_ECN_ENABLERS_STC *val
)
{
    F_BOOL(val, -1, tcDpLimit);
    F_BOOL(val, -1, portLimit);
    F_BOOL(val, -1, tcLimit);
    F_BOOL(val, -1, sharedPoolLimit);
}

void prv_c_to_lua_CPSS_PX_PORT_ECN_ENABLERS_STC(
    lua_State *L,
    CPSS_PX_PORT_ECN_ENABLERS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, tcDpLimit);
    FO_BOOL(val, t, portLimit);
    FO_BOOL(val, t, tcLimit);
    FO_BOOL(val, t, sharedPoolLimit);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_ECN_ENABLERS_STC);

void prv_lua_to_c_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC *val
)
{
    F_STRUCT_CUSTOM(val, -1, macSa, GT_ETHERADDR);
    F_STRUCT(val, -1, dsaExtForward, CPSS_PX_EXTENDED_DSA_FORWARD_STC);
    F_NUMBER(val, -1, cnmTpid, GT_U16);
}

void prv_c_to_lua_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, macSa, GT_ETHERADDR);
    FO_STRUCT(val, t, dsaExtForward, CPSS_PX_EXTENDED_DSA_FORWARD_STC);
    FO_NUMBER(val, t, cnmTpid, GT_U16);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC);

add_mgm_enum(CPSS_PX_PP_SERDES_REF_CLOCK_ENT);

void prv_lua_to_c_CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC(
    lua_State *L,
    CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC *val
)
{
    F_NUMBER(val, -1, totalConfiguredSlices, GT_U32);
    F_NUMBER(val, -1, totalSlicesOnUnit, GT_U32);
    F_BOOL(val, -1, workConservingBit);
    F_BOOL(val, -1, pizzaMapLoadEnBit);
    F_ARRAY_START(val, -1, slice_enable);
    {
        int idx;
        for (idx = 0; idx < 340; idx++) {
            F_ARRAY_BOOL(val, slice_enable, idx, GT_BOOL);
        }
    }
    F_ARRAY_END(val, -1, slice_enable);
    F_ARRAY_START(val, -1, slice_occupied_by);
    {
        int idx;
        for (idx = 0; idx < 340; idx++) {
            F_ARRAY_NUMBER(val, slice_occupied_by, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, slice_occupied_by);
}

void prv_c_to_lua_CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC(
    lua_State *L,
    CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, totalConfiguredSlices, GT_U32);
    FO_NUMBER(val, t, totalSlicesOnUnit, GT_U32);
    FO_BOOL(val, t, workConservingBit);
    FO_BOOL(val, t, pizzaMapLoadEnBit);
    FO_ARRAY_START(val, t, slice_enable);
    {
        int idx;
        for (idx = 0; idx < 340; idx++) {
            FO_ARRAY_BOOL(val, slice_enable, idx, GT_BOOL);
        }
    }
    FO_ARRAY_END(val, t, slice_enable);
    FO_ARRAY_START(val, t, slice_occupied_by);
    {
        int idx;
        for (idx = 0; idx < 340; idx++) {
            FO_ARRAY_NUMBER(val, slice_occupied_by, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, slice_occupied_by);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC);

void prv_lua_to_c_CPSS_PX_UDB_KEY_STC(
    lua_State *L,
    CPSS_PX_UDB_KEY_STC *val
)
{
    F_ENUM(val, -1, udbAnchorType, CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT);
    F_NUMBER(val, -1, udbByteOffset, GT_U32);
}

void prv_c_to_lua_CPSS_PX_UDB_KEY_STC(
    lua_State *L,
    CPSS_PX_UDB_KEY_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, udbAnchorType, CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT);
    FO_NUMBER(val, t, udbByteOffset, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_UDB_KEY_STC);

add_mgm_enum(CPSS_PX_INGRESS_HASH_PACKET_TYPE_ENT);

void prv_lua_to_c_CPSS_PX_PORT_AP_INTROP_STC(
    lua_State *L,
    CPSS_PX_PORT_AP_INTROP_STC *val
)
{
    F_NUMBER(val, -1, attrBitMask, GT_U16);
    F_NUMBER(val, -1, txDisDuration, GT_U16);
    F_NUMBER(val, -1, abilityDuration, GT_U16);
    F_NUMBER(val, -1, abilityMaxInterval, GT_U16);
    F_NUMBER(val, -1, abilityFailMaxInterval, GT_U16);
    F_NUMBER(val, -1, apLinkDuration, GT_U16);
    F_NUMBER(val, -1, apLinkMaxInterval, GT_U16);
    F_NUMBER(val, -1, pdLinkDuration, GT_U16);
    F_NUMBER(val, -1, pdLinkMaxInterval, GT_U16);
}

void prv_c_to_lua_CPSS_PX_PORT_AP_INTROP_STC(
    lua_State *L,
    CPSS_PX_PORT_AP_INTROP_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, attrBitMask, GT_U16);
    FO_NUMBER(val, t, txDisDuration, GT_U16);
    FO_NUMBER(val, t, abilityDuration, GT_U16);
    FO_NUMBER(val, t, abilityMaxInterval, GT_U16);
    FO_NUMBER(val, t, abilityFailMaxInterval, GT_U16);
    FO_NUMBER(val, t, apLinkDuration, GT_U16);
    FO_NUMBER(val, t, apLinkMaxInterval, GT_U16);
    FO_NUMBER(val, t, pdLinkDuration, GT_U16);
    FO_NUMBER(val, t, pdLinkMaxInterval, GT_U16);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_AP_INTROP_STC);

add_mgm_enum(CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT);

add_mgm_enum(CPSS_PX_PTP_TAI_CLOCK_MODE_ENT);

void prv_lua_to_c_CPSS_PX_LOCATION_SPECIFIC_INFO_STC(
    lua_State *L,
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC *val
)
{
    F_ENUM(val, -1, type, CPSS_PX_LOCATION_ENT);
    F_UNION(val, -1, info, CPSS_PX_LOCATION_UNT);
}

void prv_c_to_lua_CPSS_PX_LOCATION_SPECIFIC_INFO_STC(
    lua_State *L,
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, type, CPSS_PX_LOCATION_ENT);
    FO_UNION(val, t, info, CPSS_PX_LOCATION_UNT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_LOCATION_SPECIFIC_INFO_STC);

add_mgm_enum(CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT);

void prv_lua_to_c_CPSS_PX_LED_CLASS_MANIPULATION_STC(
    lua_State *L,
    CPSS_PX_LED_CLASS_MANIPULATION_STC *val
)
{
    F_BOOL(val, -1, blinkEnable);
    F_ENUM(val, -1, blinkSelect, CPSS_LED_BLINK_SELECT_ENT);
    F_BOOL(val, -1, forceEnable);
    F_NUMBER(val, -1, forceData, GT_U32);
    F_BOOL(val, -1, pulseStretchEnable);
    F_BOOL(val, -1, disableOnLinkDown);
}

void prv_c_to_lua_CPSS_PX_LED_CLASS_MANIPULATION_STC(
    lua_State *L,
    CPSS_PX_LED_CLASS_MANIPULATION_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, blinkEnable);
    FO_ENUM(val, t, blinkSelect, CPSS_LED_BLINK_SELECT_ENT);
    FO_BOOL(val, t, forceEnable);
    FO_NUMBER(val, t, forceData, GT_U32);
    FO_BOOL(val, t, pulseStretchEnable);
    FO_BOOL(val, t, disableOnLinkDown);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_LED_CLASS_MANIPULATION_STC);

void prv_lua_to_c_CPSS_PX_PORT_MAP_STC(
    lua_State *L,
    CPSS_PX_PORT_MAP_STC *val
)
{
    F_NUMBER(val, -1, physicalPortNumber, GT_PHYSICAL_PORT_NUM);
    F_ENUM(val, -1, mappingType, CPSS_PX_PORT_MAPPING_TYPE_ENT);
    F_NUMBER(val, -1, interfaceNum, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PORT_MAP_STC(
    lua_State *L,
    CPSS_PX_PORT_MAP_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, physicalPortNumber, GT_PHYSICAL_PORT_NUM);
    FO_ENUM(val, t, mappingType, CPSS_PX_PORT_MAPPING_TYPE_ENT);
    FO_NUMBER(val, t, interfaceNum, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_MAP_STC);

void prv_lua_to_c_CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC(
    lua_State *L,
    CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, xonThreshold, GT_U32);
    F_NUMBER(val, -1, xoffThreshold, GT_U32);
    F_ENUM(val, -1, xonAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    F_ENUM(val, -1, xoffAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
}

void prv_c_to_lua_CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC(
    lua_State *L,
    CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, xonThreshold, GT_U32);
    FO_NUMBER(val, t, xoffThreshold, GT_U32);
    FO_ENUM(val, t, xonAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    FO_ENUM(val, t, xoffAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC);

void prv_lua_to_c_CPSS_PX_PTP_TYPE_KEY_STC(
    lua_State *L,
    CPSS_PX_PTP_TYPE_KEY_STC *val
)
{
    F_ARRAY_START(val, -1, portUdbPairArr);
    {
        int idx;
        for (idx = 0; idx < 2; idx++) {
            F_ARRAY_STRUCT(val, portUdbPairArr, idx, CPSS_PX_UDB_PAIR_KEY_STC);
        }
    }
    F_ARRAY_END(val, -1, portUdbPairArr);
    F_ARRAY_START(val, -1, portUdbArr);
    {
        int idx;
        for (idx = 0; idx < 6; idx++) {
            F_ARRAY_STRUCT(val, portUdbArr, idx, CPSS_PX_UDB_KEY_STC);
        }
    }
    F_ARRAY_END(val, -1, portUdbArr);
}

void prv_c_to_lua_CPSS_PX_PTP_TYPE_KEY_STC(
    lua_State *L,
    CPSS_PX_PTP_TYPE_KEY_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, portUdbPairArr);
    {
        int idx;
        for (idx = 0; idx < 2; idx++) {
            FO_ARRAY_STRUCT(val, portUdbPairArr, idx, CPSS_PX_UDB_PAIR_KEY_STC);
        }
    }
    FO_ARRAY_END(val, t, portUdbPairArr);
    FO_ARRAY_START(val, t, portUdbArr);
    {
        int idx;
        for (idx = 0; idx < 6; idx++) {
            FO_ARRAY_STRUCT(val, portUdbArr, idx, CPSS_PX_UDB_KEY_STC);
        }
    }
    FO_ARRAY_END(val, t, portUdbArr);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PTP_TYPE_KEY_STC);

add_mgm_enum(CPSS_PX_CNC_COUNTER_FORMAT_ENT);

add_mgm_enum(CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT);

void prv_lua_to_c_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC *val
)
{
    F_ARRAY_START(val, -1, pcid);
    {
        int idx;
        for (idx = 0; idx < 7; idx++) {
            F_ARRAY_NUMBER(val, pcid, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, pcid);
}

void prv_c_to_lua_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, pcid);
    {
        int idx;
        for (idx = 0; idx < 7; idx++) {
            FO_ARRAY_NUMBER(val, pcid, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, pcid);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC);

add_mgm_enum(CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT);

void prv_lua_to_c_CPSS_PX_HW_INDEX_INFO_STC(
    lua_State *L,
    CPSS_PX_HW_INDEX_INFO_STC *val
)
{
    F_ENUM(val, -1, hwTableType, CPSS_PX_TABLE_ENT);
    F_NUMBER(val, -1, hwTableEntryIndex, GT_U32);
}

void prv_c_to_lua_CPSS_PX_HW_INDEX_INFO_STC(
    lua_State *L,
    CPSS_PX_HW_INDEX_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, hwTableType, CPSS_PX_TABLE_ENT);
    FO_NUMBER(val, t, hwTableEntryIndex, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_HW_INDEX_INFO_STC);

add_mgm_enum(CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT);

void prv_lua_to_c_CPSS_PX_DETAILED_PORT_MAP_STC(
    lua_State *L,
    CPSS_PX_DETAILED_PORT_MAP_STC *val
)
{
    F_BOOL(val, -1, valid);
    F_STRUCT(val, -1, portMap, CPSS_PX_SHADOW_PORT_MAP_STC);
}

void prv_c_to_lua_CPSS_PX_DETAILED_PORT_MAP_STC(
    lua_State *L,
    CPSS_PX_DETAILED_PORT_MAP_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, valid);
    FO_STRUCT(val, t, portMap, CPSS_PX_SHADOW_PORT_MAP_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_DETAILED_PORT_MAP_STC);

add_mgm_enum(CPSS_PX_PORT_CN_PACKET_LENGTH_ENT);

add_mgm_enum(CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT);

void prv_lua_to_c_CPSS_PX_COS_FORMAT_ENTRY_STC(
    lua_State *L,
    CPSS_PX_COS_FORMAT_ENTRY_STC *val
)
{
    F_ENUM(val, -1, cosMode, CPSS_PX_COS_MODE_ENT);
    F_NUMBER(val, -1, cosByteOffset, GT_U32);
    F_NUMBER(val, -1, cosBitOffset, GT_U32);
    F_NUMBER(val, -1, cosNumOfBits, GT_U32);
    F_STRUCT(val, -1, cosAttributes, CPSS_PX_COS_ATTRIBUTES_STC);
}

void prv_c_to_lua_CPSS_PX_COS_FORMAT_ENTRY_STC(
    lua_State *L,
    CPSS_PX_COS_FORMAT_ENTRY_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, cosMode, CPSS_PX_COS_MODE_ENT);
    FO_NUMBER(val, t, cosByteOffset, GT_U32);
    FO_NUMBER(val, t, cosBitOffset, GT_U32);
    FO_NUMBER(val, t, cosNumOfBits, GT_U32);
    FO_STRUCT(val, t, cosAttributes, CPSS_PX_COS_ATTRIBUTES_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_COS_FORMAT_ENTRY_STC);

void prv_lua_to_c_CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT *val
)
{
    F_UNION_MEMBER_STRUCT(val, -1, info_802_1br_E2U, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC);
    F_UNION_MEMBER_STRUCT(val, -1, info_802_1br_U2E_MC, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC);
    F_UNION_MEMBER_STRUCT(val, -1, info_dsa_ET2U, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC);
    F_UNION_MEMBER_STRUCT(val, -1, info_dsa_EU2U, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC);
    F_UNION_MEMBER_STRUCT(val, -1, info_edsa_E2U, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TO_UPSTREAM_PORT_STC);
    F_UNION_MEMBER_STRUCT(val, -1, info_dsa_QCN, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC);
    F_UNION_MEMBER_STRUCT(val, -1, info_evb_QCN, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_STC);
    F_UNION_MEMBER_STRUCT(val, -1, info_pre_da_PTP, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_PTP_DATA_STC);
    F_UNION_MEMBER_NUMBER(val, -1, notNeeded, GT_U32);
}

void prv_c_to_lua_CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, info_802_1br_E2U, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC);
    FO_STRUCT(val, t, info_802_1br_U2E_MC, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_UPSTREAM_PORT_TO_EXTENDED_PORT_MC_STC);
    FO_STRUCT(val, t, info_dsa_ET2U, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC);
    FO_STRUCT(val, t, info_dsa_EU2U, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC);
    FO_STRUCT(val, t, info_edsa_E2U, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TO_UPSTREAM_PORT_STC);
    FO_STRUCT(val, t, info_dsa_QCN, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_QCN_STC);
    FO_STRUCT(val, t, info_evb_QCN, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_STC);
    FO_STRUCT(val, t, info_pre_da_PTP, CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_PTP_DATA_STC);
    FO_NUMBER(val, t, notNeeded, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT);
add_mgm_union(CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT);

void prv_lua_to_c_CPSS_PX_UDB_PAIR_KEY_STC(
    lua_State *L,
    CPSS_PX_UDB_PAIR_KEY_STC *val
)
{
    F_ENUM(val, -1, udbAnchorType, CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT);
    F_NUMBER(val, -1, udbByteOffset, GT_U32);
}

void prv_c_to_lua_CPSS_PX_UDB_PAIR_KEY_STC(
    lua_State *L,
    CPSS_PX_UDB_PAIR_KEY_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, udbAnchorType, CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT);
    FO_NUMBER(val, t, udbByteOffset, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_UDB_PAIR_KEY_STC);

void prv_lua_to_c_CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC(
    lua_State *L,
    CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC *val
)
{
    F_BOOL(val, -1, ptpOverMplsEn);
    F_ENUM(val, -1, ptpPortMode, CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_ENT);
}

void prv_c_to_lua_CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC(
    lua_State *L,
    CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, ptpOverMplsEn);
    FO_ENUM(val, t, ptpPortMode, CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC);

void prv_lua_to_c_CPSS_PX_REGULAR_DSA_FORWARD_STC(
    lua_State *L,
    CPSS_PX_REGULAR_DSA_FORWARD_STC *val
)
{
    F_BOOL(val, -1, srcTagged);
    F_NUMBER(val, -1, hwSrcDev, GT_HW_DEV_NUM);
    F_NUMBER(val, -1, srcPortOrTrunk, GT_U32);
    F_BOOL(val, -1, srcIsTrunk);
    F_NUMBER(val, -1, cfi, GT_U32);
    F_NUMBER(val, -1, up, GT_U32);
    F_NUMBER(val, -1, vid, GT_U32);
}

void prv_c_to_lua_CPSS_PX_REGULAR_DSA_FORWARD_STC(
    lua_State *L,
    CPSS_PX_REGULAR_DSA_FORWARD_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, srcTagged);
    FO_NUMBER(val, t, hwSrcDev, GT_HW_DEV_NUM);
    FO_NUMBER(val, t, srcPortOrTrunk, GT_U32);
    FO_BOOL(val, t, srcIsTrunk);
    FO_NUMBER(val, t, cfi, GT_U32);
    FO_NUMBER(val, t, up, GT_U32);
    FO_NUMBER(val, t, vid, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_REGULAR_DSA_FORWARD_STC);

add_mgm_enum(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT);

void prv_lua_to_c_CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC(
    lua_State *L,
    CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC *val
)
{
    F_ARRAY_START(val, -1, unitList);
    {
        int idx;
        for (idx = 0; idx < 6; idx++) {
            F_ARRAY_ENUM(val, unitList, idx, CPSS_PX_PA_UNIT_ENT);
        }
    }
    F_ARRAY_END(val, -1, unitList);
    F_ARRAY_START(val, -1, unitState);
    {
        int idx;
        for (idx = 0; idx < 5; idx++) {
            F_ARRAY_STRUCT(val, unitState, idx, CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC);
        }
    }
    F_ARRAY_END(val, -1, unitState);
}

void prv_c_to_lua_CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC(
    lua_State *L,
    CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, unitList);
    {
        int idx;
        for (idx = 0; idx < 6; idx++) {
            FO_ARRAY_ENUM(val, unitList, idx, CPSS_PX_PA_UNIT_ENT);
        }
    }
    FO_ARRAY_END(val, t, unitList);
    FO_ARRAY_START(val, t, unitState);
    {
        int idx;
        for (idx = 0; idx < 5; idx++) {
            FO_ARRAY_STRUCT(val, unitState, idx, CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC);
        }
    }
    FO_ARRAY_END(val, t, unitState);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC);

void prv_lua_to_c_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_PTP_DATA_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_PTP_DATA_STC *val
)
{
    F_NUMBER(val, -1, messageType, GT_U32);
    F_NUMBER(val, -1, ptpVersion, GT_U32);
    F_NUMBER(val, -1, domainNumber, GT_U32);
    F_NUMBER(val, -1, udpDestPort, GT_U32);
    F_NUMBER(val, -1, ipv4ProtocolIpv6NextHeader, GT_U32);
}

void prv_c_to_lua_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_PTP_DATA_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_PTP_DATA_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, messageType, GT_U32);
    FO_NUMBER(val, t, ptpVersion, GT_U32);
    FO_NUMBER(val, t, domainNumber, GT_U32);
    FO_NUMBER(val, t, udpDestPort, GT_U32);
    FO_NUMBER(val, t, ipv4ProtocolIpv6NextHeader, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_PRE_DA_PTP_DATA_STC);

add_mgm_enum(CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT);

void prv_lua_to_c_CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT(
    lua_State *L,
    CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT *val
)
{
    F_UNION_MEMBER_STRUCT(val, -1, memLocation, CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC);
    F_UNION_MEMBER_STRUCT(val, -1, mppmMemLocation, CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC);
}

void prv_c_to_lua_CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT(
    lua_State *L,
    CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, memLocation, CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC);
    FO_STRUCT(val, t, mppmMemLocation, CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT);
add_mgm_union(CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT);

add_mgm_enum(CPSS_PX_CNC_BYTE_COUNT_MODE_ENT);

add_mgm_enum(CPSS_PX_COS_MODE_ENT);

void prv_lua_to_c_CPSS_PX_SHADOW_PORT_MAP_STC(
    lua_State *L,
    CPSS_PX_SHADOW_PORT_MAP_STC *val
)
{
    F_ENUM(val, -1, mappingType, CPSS_PX_PORT_MAPPING_TYPE_ENT);
    F_NUMBER(val, -1, macNum, GT_U32);
    F_NUMBER(val, -1, dmaNum, GT_U32);
    F_NUMBER(val, -1, txqNum, GT_U32);
}

void prv_c_to_lua_CPSS_PX_SHADOW_PORT_MAP_STC(
    lua_State *L,
    CPSS_PX_SHADOW_PORT_MAP_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, mappingType, CPSS_PX_PORT_MAPPING_TYPE_ENT);
    FO_NUMBER(val, t, macNum, GT_U32);
    FO_NUMBER(val, t, dmaNum, GT_U32);
    FO_NUMBER(val, t, txqNum, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_SHADOW_PORT_MAP_STC);

void prv_lua_to_c_CPSS_PX_LOGICAL_TABLE_SHADOW_INFO_STC(
    lua_State *L,
    CPSS_PX_LOGICAL_TABLE_SHADOW_INFO_STC *val
)
{
    F_ENUM(val, -1, logicalTableName, CPSS_PX_LOGICAL_TABLE_ENT);
    F_ENUM(val, -1, shadowType, CPSS_PX_SHADOW_TYPE_ENT);
    F_NUMBER(val, -1, numOfBytes, GT_U32);
    F_NUMBER(val, -1, isSupported, GT_STATUS);
}

void prv_c_to_lua_CPSS_PX_LOGICAL_TABLE_SHADOW_INFO_STC(
    lua_State *L,
    CPSS_PX_LOGICAL_TABLE_SHADOW_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, logicalTableName, CPSS_PX_LOGICAL_TABLE_ENT);
    FO_ENUM(val, t, shadowType, CPSS_PX_SHADOW_TYPE_ENT);
    FO_NUMBER(val, t, numOfBytes, GT_U32);
    FO_NUMBER(val, t, isSupported, GT_STATUS);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_LOGICAL_TABLE_SHADOW_INFO_STC);

add_mgm_enum(CPSS_PX_INGRESS_ETHERTYPE_ENT);

void prv_lua_to_c_CPSS_PX_PTP_TAI_TOD_STEP_STC(
    lua_State *L,
    CPSS_PX_PTP_TAI_TOD_STEP_STC *val
)
{
    F_NUMBER(val, -1, nanoSeconds, GT_U32);
    F_NUMBER(val, -1, fracNanoSeconds, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PTP_TAI_TOD_STEP_STC(
    lua_State *L,
    CPSS_PX_PTP_TAI_TOD_STEP_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, nanoSeconds, GT_U32);
    FO_NUMBER(val, t, fracNanoSeconds, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PTP_TAI_TOD_STEP_STC);

add_mgm_enum(CPSS_PX_PORT_FC_MODE_ENT);

add_mgm_enum(CPSS_PX_EGRESS_TARGET_PORT_ENTRY_TYPE_ENT);

add_mgm_enum(CPSS_PX_LOCATION_ENT);

add_mgm_enum(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT);

void prv_lua_to_c_CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC(
    lua_State *L,
    CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC *val
)
{
    F_BOOL(val, -1, link);
    F_ENUM(val, -1, speed, CPSS_PORT_SPEED_ENT);
    F_ENUM(val, -1, duplex, CPSS_PORT_DUPLEX_ENT);
}

void prv_c_to_lua_CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC(
    lua_State *L,
    CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, link);
    FO_ENUM(val, t, speed, CPSS_PORT_SPEED_ENT);
    FO_ENUM(val, t, duplex, CPSS_PORT_DUPLEX_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC);

add_mgm_enum(CPSS_PX_PORT_TX_SHAPER_GRANULARITY_ENT);

add_mgm_enum(CPSS_PX_DIAG_TRANSMIT_MODE_ENT);

void prv_lua_to_c_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC *val
)
{
    F_STRUCT(val, -1, eTag, CPSS_802_1BR_ETAG_STC);
    F_NUMBER(val, -1, vlanTagTpid, GT_U32);
}

void prv_c_to_lua_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, eTag, CPSS_802_1BR_ETAG_STC);
    FO_NUMBER(val, t, vlanTagTpid, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_802_1BR_EXTENDED_PORT_TO_UPSTREAM_PORT_STC);

void prv_lua_to_c_CPSS_PX_LOCATION_UNT(
    lua_State *L,
    CPSS_PX_LOCATION_UNT *val
)
{
    F_UNION_MEMBER_STRUCT(val, -1, logicalEntryInfo, CPSS_PX_LOGICAL_TABLE_INFO_STC);
    F_UNION_MEMBER_STRUCT(val, -1, hwEntryInfo, CPSS_PX_HW_INDEX_INFO_STC);
    F_UNION_MEMBER_STRUCT(val, -1, ramEntryInfo, CPSS_PX_RAM_INDEX_INFO_STC);
}

void prv_c_to_lua_CPSS_PX_LOCATION_UNT(
    lua_State *L,
    CPSS_PX_LOCATION_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, logicalEntryInfo, CPSS_PX_LOGICAL_TABLE_INFO_STC);
    FO_STRUCT(val, t, hwEntryInfo, CPSS_PX_HW_INDEX_INFO_STC);
    FO_STRUCT(val, t, ramEntryInfo, CPSS_PX_RAM_INDEX_INFO_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_LOCATION_UNT);
add_mgm_union(CPSS_PX_LOCATION_UNT);

void prv_lua_to_c_CPSS_PX_PORT_EGRESS_CNTR_STC(
    lua_State *L,
    CPSS_PX_PORT_EGRESS_CNTR_STC *val
)
{
    F_NUMBER(val, -1, outFrames, GT_U32);
    F_NUMBER(val, -1, txqFilterDisc, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PORT_EGRESS_CNTR_STC(
    lua_State *L,
    CPSS_PX_PORT_EGRESS_CNTR_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, outFrames, GT_U32);
    FO_NUMBER(val, t, txqFilterDisc, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_EGRESS_CNTR_STC);

void prv_lua_to_c_CPSS_PX_LED_CONF_STC(
    lua_State *L,
    CPSS_PX_LED_CONF_STC *val
)
{
    F_ENUM(val, -1, ledOrganize, CPSS_LED_ORDER_MODE_ENT);
    F_ENUM(val, -1, blink0DutyCycle, CPSS_LED_BLINK_DUTY_CYCLE_ENT);
    F_ENUM(val, -1, blink0Duration, CPSS_LED_BLINK_DURATION_ENT);
    F_ENUM(val, -1, blink1DutyCycle, CPSS_LED_BLINK_DUTY_CYCLE_ENT);
    F_ENUM(val, -1, blink1Duration, CPSS_LED_BLINK_DURATION_ENT);
    F_ENUM(val, -1, pulseStretch, CPSS_LED_PULSE_STRETCH_ENT);
    F_NUMBER(val, -1, ledStart, GT_U32);
    F_NUMBER(val, -1, ledEnd, GT_U32);
    F_BOOL(val, -1, invertEnable);
    F_ENUM(val, -1, ledClockFrequency, CPSS_LED_CLOCK_OUT_FREQUENCY_ENT);
}

void prv_c_to_lua_CPSS_PX_LED_CONF_STC(
    lua_State *L,
    CPSS_PX_LED_CONF_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, ledOrganize, CPSS_LED_ORDER_MODE_ENT);
    FO_ENUM(val, t, blink0DutyCycle, CPSS_LED_BLINK_DUTY_CYCLE_ENT);
    FO_ENUM(val, t, blink0Duration, CPSS_LED_BLINK_DURATION_ENT);
    FO_ENUM(val, t, blink1DutyCycle, CPSS_LED_BLINK_DUTY_CYCLE_ENT);
    FO_ENUM(val, t, blink1Duration, CPSS_LED_BLINK_DURATION_ENT);
    FO_ENUM(val, t, pulseStretch, CPSS_LED_PULSE_STRETCH_ENT);
    FO_NUMBER(val, t, ledStart, GT_U32);
    FO_NUMBER(val, t, ledEnd, GT_U32);
    FO_BOOL(val, t, invertEnable);
    FO_ENUM(val, t, ledClockFrequency, CPSS_LED_CLOCK_OUT_FREQUENCY_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_LED_CONF_STC);

void prv_lua_to_c_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC(
    lua_State *L,
    CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC *val
)
{
    F_BOOL(val, -1, tcDpLimit);
    F_BOOL(val, -1, portLimit);
    F_BOOL(val, -1, tcLimit);
    F_BOOL(val, -1, sharedPoolLimit);
}

void prv_c_to_lua_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC(
    lua_State *L,
    CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, tcDpLimit);
    FO_BOOL(val, t, portLimit);
    FO_BOOL(val, t, tcLimit);
    FO_BOOL(val, t, sharedPoolLimit);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC);

add_mgm_enum(CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT);

add_mgm_enum(CPSS_PX_CNC_CLIENT_ENT);

void prv_lua_to_c_CPSS_PX_CNC_COUNTER_STC(
    lua_State *L,
    CPSS_PX_CNC_COUNTER_STC *val
)
{
    F_STRUCT(val, -1, byteCount, GT_U64);
    F_STRUCT(val, -1, packetCount, GT_U64);
}

void prv_c_to_lua_CPSS_PX_CNC_COUNTER_STC(
    lua_State *L,
    CPSS_PX_CNC_COUNTER_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, byteCount, GT_U64);
    FO_STRUCT(val, t, packetCount, GT_U64);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_CNC_COUNTER_STC);

add_mgm_enum(CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);

void prv_lua_to_c_CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC(
    lua_State *L,
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC *val
)
{
    F_NUMBER(val, -1, srcPortProfile, GT_U32);
    F_ARRAY_START(val, -1, portUdbPairArr);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            F_ARRAY_STRUCT(val, portUdbPairArr, idx, CPSS_PX_UDB_PAIR_KEY_STC);
        }
    }
    F_ARRAY_END(val, -1, portUdbPairArr);
}

void prv_c_to_lua_CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC(
    lua_State *L,
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, srcPortProfile, GT_U32);
    FO_ARRAY_START(val, t, portUdbPairArr);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            FO_ARRAY_STRUCT(val, portUdbPairArr, idx, CPSS_PX_UDB_PAIR_KEY_STC);
        }
    }
    FO_ARRAY_END(val, t, portUdbPairArr);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC);

void prv_lua_to_c_CPSS_PX_PTP_TSU_CONTROL_STC(
    lua_State *L,
    CPSS_PX_PTP_TSU_CONTROL_STC *val
)
{
    F_BOOL(val, -1, unitEnable);
}

void prv_c_to_lua_CPSS_PX_PTP_TSU_CONTROL_STC(
    lua_State *L,
    CPSS_PX_PTP_TSU_CONTROL_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, unitEnable);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PTP_TSU_CONTROL_STC);

add_mgm_enum(CPSS_PX_DIAG_BIST_STATUS_ENT);

void prv_lua_to_c_CPSS_PX_INGRESS_TPID_ENTRY_STC(
    lua_State *L,
    CPSS_PX_INGRESS_TPID_ENTRY_STC *val
)
{
    F_NUMBER(val, -1, val, GT_U16);
    F_NUMBER(val, -1, size, GT_U32);
    F_BOOL(val, -1, valid);
}

void prv_c_to_lua_CPSS_PX_INGRESS_TPID_ENTRY_STC(
    lua_State *L,
    CPSS_PX_INGRESS_TPID_ENTRY_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, val, GT_U16);
    FO_NUMBER(val, t, size, GT_U32);
    FO_BOOL(val, t, valid);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_INGRESS_TPID_ENTRY_STC);

void prv_lua_to_c_CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC(
    lua_State *L,
    CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC *val
)
{
    F_NUMBER(val, -1, egrMirrorDropCntr, GT_U32);
    F_NUMBER(val, -1, egrStcDropCntr, GT_U32);
    F_NUMBER(val, -1, egrQcnDropCntr, GT_U32);
    F_NUMBER(val, -1, dropPfcEventsCntr, GT_U32);
    F_NUMBER(val, -1, clearPacketsDroppedCounter, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC(
    lua_State *L,
    CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, egrMirrorDropCntr, GT_U32);
    FO_NUMBER(val, t, egrStcDropCntr, GT_U32);
    FO_NUMBER(val, t, egrQcnDropCntr, GT_U32);
    FO_NUMBER(val, t, dropPfcEventsCntr, GT_U32);
    FO_NUMBER(val, t, clearPacketsDroppedCounter, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC);

void prv_lua_to_c_CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC(
    lua_State *L,
    CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC *val
)
{
    F_NUMBER(val, -1, byteOffset, GT_U32);
    F_NUMBER(val, -1, startBit, GT_U32);
    F_NUMBER(val, -1, numBits, GT_U32);
}

void prv_c_to_lua_CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC(
    lua_State *L,
    CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, byteOffset, GT_U32);
    FO_NUMBER(val, t, startBit, GT_U32);
    FO_NUMBER(val, t, numBits, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC);

void prv_lua_to_c_CPSS_PX_DIAG_PG_CONFIGURATIONS_STC(
    lua_State *L,
    CPSS_PX_DIAG_PG_CONFIGURATIONS_STC *val
)
{
    F_STRUCT_CUSTOM(val, -1, macDa, GT_ETHERADDR);
    F_BOOL(val, -1, macDaIncrementEnable);
    F_NUMBER(val, -1, macDaIncrementLimit, GT_U32);
    F_STRUCT_CUSTOM(val, -1, macSa, GT_ETHERADDR);
    F_BOOL(val, -1, vlanTagEnable);
    F_NUMBER(val, -1, vpt, GT_U8);
    F_NUMBER(val, -1, cfi, GT_U8);
    F_NUMBER(val, -1, vid, GT_U16);
    F_NUMBER(val, -1, etherType, GT_U16);
    F_ENUM(val, -1, payloadType, CPSS_DIAG_PG_PACKET_PAYLOAD_TYPE_ENT);
    F_ARRAY_START(val, -1, cyclicPatternArr);
    {
        int idx;
        for (idx = 0; idx < 64; idx++) {
            F_ARRAY_NUMBER(val, cyclicPatternArr, idx, GT_U8);
        }
    }
    F_ARRAY_END(val, -1, cyclicPatternArr);
    F_ENUM(val, -1, packetLengthType, CPSS_DIAG_PG_PACKET_LENGTH_TYPE_ENT);
    F_NUMBER(val, -1, packetLength, GT_U32);
    F_BOOL(val, -1, undersizeEnable);
    F_ENUM(val, -1, transmitMode, CPSS_DIAG_PG_TRANSMIT_MODE_ENT);
    F_NUMBER(val, -1, packetCount, GT_U32);
    F_ENUM(val, -1, packetCountMultiplier, CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_ENT);
    F_NUMBER(val, -1, ifg, GT_U32);
    F_ENUM(val, -1, interfaceSize, CPSS_DIAG_PG_IF_SIZE_ENT);
}

void prv_c_to_lua_CPSS_PX_DIAG_PG_CONFIGURATIONS_STC(
    lua_State *L,
    CPSS_PX_DIAG_PG_CONFIGURATIONS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, macDa, GT_ETHERADDR);
    FO_BOOL(val, t, macDaIncrementEnable);
    FO_NUMBER(val, t, macDaIncrementLimit, GT_U32);
    FO_STRUCT(val, t, macSa, GT_ETHERADDR);
    FO_BOOL(val, t, vlanTagEnable);
    FO_NUMBER(val, t, vpt, GT_U8);
    FO_NUMBER(val, t, cfi, GT_U8);
    FO_NUMBER(val, t, vid, GT_U16);
    FO_NUMBER(val, t, etherType, GT_U16);
    FO_ENUM(val, t, payloadType, CPSS_DIAG_PG_PACKET_PAYLOAD_TYPE_ENT);
    FO_ARRAY_START(val, t, cyclicPatternArr);
    {
        int idx;
        for (idx = 0; idx < 64; idx++) {
            FO_ARRAY_NUMBER(val, cyclicPatternArr, idx, GT_U8);
        }
    }
    FO_ARRAY_END(val, t, cyclicPatternArr);
    FO_ENUM(val, t, packetLengthType, CPSS_DIAG_PG_PACKET_LENGTH_TYPE_ENT);
    FO_NUMBER(val, t, packetLength, GT_U32);
    FO_BOOL(val, t, undersizeEnable);
    FO_ENUM(val, t, transmitMode, CPSS_DIAG_PG_TRANSMIT_MODE_ENT);
    FO_NUMBER(val, t, packetCount, GT_U32);
    FO_ENUM(val, t, packetCountMultiplier, CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_ENT);
    FO_NUMBER(val, t, ifg, GT_U32);
    FO_ENUM(val, t, interfaceSize, CPSS_DIAG_PG_IF_SIZE_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_DIAG_PG_CONFIGURATIONS_STC);

add_mgm_enum(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT);

void prv_lua_to_c_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC *val
)
{
    F_STRUCT(val, -1, dsaForward, CPSS_PX_REGULAR_DSA_FORWARD_STC);
}

void prv_c_to_lua_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, dsaForward, CPSS_PX_REGULAR_DSA_FORWARD_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_STC);

void prv_lua_to_c_CPSS_PX_EDSA_FORWARD_STC(
    lua_State *L,
    CPSS_PX_EDSA_FORWARD_STC *val
)
{
    F_BOOL(val, -1, srcTagged);
    F_NUMBER(val, -1, hwSrcDev, GT_HW_DEV_NUM);
    F_NUMBER(val, -1, tpIdIndex, GT_U32);
    F_BOOL(val, -1, tag1SrcTagged);
}

void prv_c_to_lua_CPSS_PX_EDSA_FORWARD_STC(
    lua_State *L,
    CPSS_PX_EDSA_FORWARD_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, srcTagged);
    FO_NUMBER(val, t, hwSrcDev, GT_HW_DEV_NUM);
    FO_NUMBER(val, t, tpIdIndex, GT_U32);
    FO_BOOL(val, t, tag1SrcTagged);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EDSA_FORWARD_STC);

add_mgm_enum(CPSS_PX_INGRESS_HASH_MODE_ENT);

void prv_lua_to_c_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC(
    lua_State *L,
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC *val
)
{
    F_STRUCT_CUSTOM(val, -1, macDa, GT_ETHERADDR);
    F_NUMBER(val, -1, etherType, GT_U16);
    F_BOOL(val, -1, isLLCNonSnap);
    F_NUMBER(val, -1, profileIndex, GT_U32);
    F_ARRAY_START(val, -1, udbPairsArr);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            F_ARRAY_STRUCT(val, udbPairsArr, idx, CPSS_PX_UDB_PAIR_DATA_STC);
        }
    }
    F_ARRAY_END(val, -1, udbPairsArr);
    F_NUMBER(val, -1, ip2meIndex, GT_U32);
}

void prv_c_to_lua_CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC(
    lua_State *L,
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, macDa, GT_ETHERADDR);
    FO_NUMBER(val, t, etherType, GT_U16);
    FO_BOOL(val, t, isLLCNonSnap);
    FO_NUMBER(val, t, profileIndex, GT_U32);
    FO_ARRAY_START(val, t, udbPairsArr);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            FO_ARRAY_STRUCT(val, udbPairsArr, idx, CPSS_PX_UDB_PAIR_DATA_STC);
        }
    }
    FO_ARRAY_END(val, t, udbPairsArr);
    FO_NUMBER(val, t, ip2meIndex, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC);

void prv_lua_to_c_CPSS_PX_PTP_TAI_TOD_COUNT_STC(
    lua_State *L,
    CPSS_PX_PTP_TAI_TOD_COUNT_STC *val
)
{
    F_NUMBER(val, -1, nanoSeconds, GT_U32);
    F_STRUCT(val, -1, seconds, GT_U64);
    F_NUMBER(val, -1, fracNanoSeconds, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PTP_TAI_TOD_COUNT_STC(
    lua_State *L,
    CPSS_PX_PTP_TAI_TOD_COUNT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, nanoSeconds, GT_U32);
    FO_STRUCT(val, t, seconds, GT_U64);
    FO_NUMBER(val, t, fracNanoSeconds, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PTP_TAI_TOD_COUNT_STC);

void prv_lua_to_c_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_STC *val
)
{
    F_STRUCT_CUSTOM(val, -1, macSa, GT_ETHERADDR);
    F_NUMBER(val, -1, qcnTpid, GT_U16);
}

void prv_c_to_lua_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, macSa, GT_ETHERADDR);
    FO_NUMBER(val, t, qcnTpid, GT_U16);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EVB_QCN_STC);

add_mgm_enum(CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT);

add_mgm_enum(CPSS_PX_PA_UNIT_ENT);

void prv_lua_to_c_CPSS_PX_COS_ATTRIBUTES_STC(
    lua_State *L,
    CPSS_PX_COS_ATTRIBUTES_STC *val
)
{
    F_NUMBER(val, -1, trafficClass, GT_U32);
    F_ENUM(val, -1, dropPrecedence, CPSS_DP_LEVEL_ENT);
    F_NUMBER(val, -1, userPriority, GT_U32);
    F_NUMBER(val, -1, dropEligibilityIndication, GT_U32);
}

void prv_c_to_lua_CPSS_PX_COS_ATTRIBUTES_STC(
    lua_State *L,
    CPSS_PX_COS_ATTRIBUTES_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, trafficClass, GT_U32);
    FO_ENUM(val, t, dropPrecedence, CPSS_DP_LEVEL_ENT);
    FO_NUMBER(val, t, userPriority, GT_U32);
    FO_NUMBER(val, t, dropEligibilityIndication, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_COS_ATTRIBUTES_STC);

void prv_lua_to_c_CPSS_PX_EGRESS_SRC_PORT_INFO_STC(
    lua_State *L,
    CPSS_PX_EGRESS_SRC_PORT_INFO_STC *val
)
{
    F_NUMBER(val, -1, srcFilteringVector, CPSS_PX_PORTS_BMP);
    F_STRUCT(val, -1, vlanTag, CPSS_PX_VLAN_TAG_STC);
}

void prv_c_to_lua_CPSS_PX_EGRESS_SRC_PORT_INFO_STC(
    lua_State *L,
    CPSS_PX_EGRESS_SRC_PORT_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, srcFilteringVector, CPSS_PX_PORTS_BMP);
    FO_STRUCT(val, t, vlanTag, CPSS_PX_VLAN_TAG_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_SRC_PORT_INFO_STC);

add_mgm_enum(CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_ENT);

add_mgm_enum(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT);

void prv_lua_to_c_CPSS_PX_PORT_SERDES_EYE_INPUT_STC(
    lua_State *L,
    CPSS_PX_PORT_SERDES_EYE_INPUT_STC *val
)
{
    F_NUMBER(val, -1, min_dwell_bits, GT_U32);
    F_NUMBER(val, -1, max_dwell_bits, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PORT_SERDES_EYE_INPUT_STC(
    lua_State *L,
    CPSS_PX_PORT_SERDES_EYE_INPUT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, min_dwell_bits, GT_U32);
    FO_NUMBER(val, t, max_dwell_bits, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_SERDES_EYE_INPUT_STC);

void prv_lua_to_c_CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC(
    lua_State *L,
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC *val
)
{
    F_ARRAY_START(val, -1, bitFieldArr);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            F_ARRAY_STRUCT(val, bitFieldArr, idx, CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC);
        }
    }
    F_ARRAY_END(val, -1, bitFieldArr);
    F_NUMBER(val, -1, indexConst, GT_32);
    F_NUMBER(val, -1, indexMax, GT_U32);
}

void prv_c_to_lua_CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC(
    lua_State *L,
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, bitFieldArr);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            FO_ARRAY_STRUCT(val, bitFieldArr, idx, CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC);
        }
    }
    FO_ARRAY_END(val, t, bitFieldArr);
    FO_NUMBER(val, t, indexConst, GT_32);
    FO_NUMBER(val, t, indexMax, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC);

void prv_lua_to_c_CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC(
    lua_State *L,
    CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, packetType, CPSS_PX_PACKET_TYPE);
    F_NUMBER(val, -1, priority, GT_U32);
    F_ENUM(val, -1, dropPrecedence, CPSS_DP_LEVEL_ENT);
    F_NUMBER(val, -1, tc4pfc, GT_U32);
    F_BOOL(val, -1, localGeneratedPacketTypeAssignmentEnable);
    F_NUMBER(val, -1, localGeneratedPacketType, CPSS_PX_PACKET_TYPE);
}

void prv_c_to_lua_CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC(
    lua_State *L,
    CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, packetType, CPSS_PX_PACKET_TYPE);
    FO_NUMBER(val, t, priority, GT_U32);
    FO_ENUM(val, t, dropPrecedence, CPSS_DP_LEVEL_ENT);
    FO_NUMBER(val, t, tc4pfc, GT_U32);
    FO_BOOL(val, t, localGeneratedPacketTypeAssignmentEnable);
    FO_NUMBER(val, t, localGeneratedPacketType, CPSS_PX_PACKET_TYPE);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC);

add_mgm_enum(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_ENT);

void prv_lua_to_c_CPSS_PX_CFG_DEV_INFO_STC(
    lua_State *L,
    CPSS_PX_CFG_DEV_INFO_STC *val
)
{
    F_STRUCT(val, -1, genDevInfo, CPSS_GEN_CFG_DEV_INFO_STC);
}

void prv_c_to_lua_CPSS_PX_CFG_DEV_INFO_STC(
    lua_State *L,
    CPSS_PX_CFG_DEV_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, genDevInfo, CPSS_GEN_CFG_DEV_INFO_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_CFG_DEV_INFO_STC);

void prv_lua_to_c_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC *val
)
{
    F_STRUCT(val, -1, dsaForward, CPSS_PX_REGULAR_DSA_FORWARD_STC);
}

void prv_c_to_lua_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, dsaForward, CPSS_PX_REGULAR_DSA_FORWARD_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_STC);

add_mgm_enum(CPSS_PX_PTP_TAI_CLOCK_FREQ_ENT);

void prv_lua_to_c_CPSS_PX_VLAN_TAG_STC(
    lua_State *L,
    CPSS_PX_VLAN_TAG_STC *val
)
{
    F_NUMBER(val, -1, pcp, GT_U32);
    F_NUMBER(val, -1, dei, GT_U32);
    F_NUMBER(val, -1, vid, GT_U32);
}

void prv_c_to_lua_CPSS_PX_VLAN_TAG_STC(
    lua_State *L,
    CPSS_PX_VLAN_TAG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, pcp, GT_U32);
    FO_NUMBER(val, t, dei, GT_U32);
    FO_NUMBER(val, t, vid, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_VLAN_TAG_STC);

add_mgm_enum(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT);

void prv_lua_to_c_CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC(
    lua_State *L,
    CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *val
)
{
    F_NUMBER(val, -1, interval, GT_U32);
    F_NUMBER(val, -1, randBitmap, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC(
    lua_State *L,
    CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, interval, GT_U32);
    FO_NUMBER(val, t, randBitmap, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC);

void prv_lua_to_c_CPSS_PX_PORT_ALIGN90_PARAMS_STC(
    lua_State *L,
    CPSS_PX_PORT_ALIGN90_PARAMS_STC *val
)
{
    F_NUMBER(val, -1, startAlign90, GT_U32);
    F_NUMBER(val, -1, rxTrainingCfg, GT_U32);
    F_NUMBER(val, -1, osDeltaMax, GT_U32);
    F_NUMBER(val, -1, adaptedFfeR, GT_U32);
    F_NUMBER(val, -1, adaptedFfeC, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PORT_ALIGN90_PARAMS_STC(
    lua_State *L,
    CPSS_PX_PORT_ALIGN90_PARAMS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, startAlign90, GT_U32);
    FO_NUMBER(val, t, rxTrainingCfg, GT_U32);
    FO_NUMBER(val, t, osDeltaMax, GT_U32);
    FO_NUMBER(val, t, adaptedFfeR, GT_U32);
    FO_NUMBER(val, t, adaptedFfeC, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_ALIGN90_PARAMS_STC);

void prv_lua_to_c_CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC(
    lua_State *L,
    CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC *val
)
{
    F_NUMBER(val, -1, vid, GT_U32);
    F_STRUCT(val, -1, ptpPortInfo, CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC);
}

void prv_c_to_lua_CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC(
    lua_State *L,
    CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, vid, GT_U32);
    FO_STRUCT(val, t, ptpPortInfo, CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC);

add_mgm_enum(CPSS_PX_MIN_SPEED_ENT);

void prv_lua_to_c_CPSS_PX_NET_TX_PARAMS_STC(
    lua_State *L,
    CPSS_PX_NET_TX_PARAMS_STC *val
)
{
    F_NUMBER(val, -1, txQueue, GT_U32);
    F_BOOL(val, -1, recalcCrc);
}

void prv_c_to_lua_CPSS_PX_NET_TX_PARAMS_STC(
    lua_State *L,
    CPSS_PX_NET_TX_PARAMS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, txQueue, GT_U32);
    FO_BOOL(val, t, recalcCrc);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_NET_TX_PARAMS_STC);

add_mgm_enum(CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT);

void prv_lua_to_c_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC(
    lua_State *L,
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC *val
)
{
    F_NUMBER(val, -1, dp0MaxBuffNum, GT_U32);
    F_NUMBER(val, -1, dp0MaxDescNum, GT_U32);
    F_NUMBER(val, -1, dp1MaxBuffNum, GT_U32);
    F_NUMBER(val, -1, dp1MaxDescNum, GT_U32);
    F_NUMBER(val, -1, dp2MaxBuffNum, GT_U32);
    F_NUMBER(val, -1, dp2MaxDescNum, GT_U32);
    F_NUMBER(val, -1, tcMaxBuffNum, GT_U32);
    F_NUMBER(val, -1, tcMaxDescNum, GT_U32);
    F_ENUM(val, -1, dp0QueueAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    F_ENUM(val, -1, dp1QueueAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    F_ENUM(val, -1, dp2QueueAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
}

void prv_c_to_lua_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC(
    lua_State *L,
    CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, dp0MaxBuffNum, GT_U32);
    FO_NUMBER(val, t, dp0MaxDescNum, GT_U32);
    FO_NUMBER(val, t, dp1MaxBuffNum, GT_U32);
    FO_NUMBER(val, t, dp1MaxDescNum, GT_U32);
    FO_NUMBER(val, t, dp2MaxBuffNum, GT_U32);
    FO_NUMBER(val, t, dp2MaxDescNum, GT_U32);
    FO_NUMBER(val, t, tcMaxBuffNum, GT_U32);
    FO_NUMBER(val, t, tcMaxDescNum, GT_U32);
    FO_ENUM(val, t, dp0QueueAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    FO_ENUM(val, t, dp1QueueAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    FO_ENUM(val, t, dp2QueueAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC);

add_mgm_enum(CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT);

add_mgm_enum(CPSS_PX_SHADOW_TYPE_ENT);

add_mgm_enum(CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT);

add_mgm_enum(CPSS_PX_EGRESS_SOURCE_PORT_PTP_MODE_ENT);

add_mgm_enum(CPSS_PX_PORT_TX_SHAPER_MODE_ENT);

add_mgm_enum(CPSS_PX_POWER_SUPPLIES_NUMBER_ENT);

add_mgm_enum(CPSS_PX_PHA_FIRMWARE_TYPE_ENT);

add_mgm_enum(CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT);

add_mgm_enum(CPSS_PX_PORT_PFC_COUNT_MODE_ENT);

void prv_lua_to_c_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC(
    lua_State *L,
    CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC *val
)
{
    F_NUMBER(val, -1, tcDp, GT_U32);
    F_NUMBER(val, -1, port, GT_U32);
    F_NUMBER(val, -1, tc, GT_U32);
    F_NUMBER(val, -1, pool, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC(
    lua_State *L,
    CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, tcDp, GT_U32);
    FO_NUMBER(val, t, port, GT_U32);
    FO_NUMBER(val, t, tc, GT_U32);
    FO_NUMBER(val, t, pool, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC);

add_mgm_enum(CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT);

void prv_lua_to_c_CPSS_PX_EGRESS_SOURCE_PORT_PRE_DA_STC(
    lua_State *L,
    CPSS_PX_EGRESS_SOURCE_PORT_PRE_DA_STC *val
)
{
    F_NUMBER(val, -1, port, GT_U32);
    F_STRUCT(val, -1, ptpPortInfo, CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC);
}

void prv_c_to_lua_CPSS_PX_EGRESS_SOURCE_PORT_PRE_DA_STC(
    lua_State *L,
    CPSS_PX_EGRESS_SOURCE_PORT_PRE_DA_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, port, GT_U32);
    FO_STRUCT(val, t, ptpPortInfo, CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_SOURCE_PORT_PRE_DA_STC);

add_mgm_enum(CPSS_PX_PORT_CTLE_BIAS_MODE_ENT);

void prv_lua_to_c_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TO_UPSTREAM_PORT_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TO_UPSTREAM_PORT_STC *val
)
{
    F_STRUCT(val, -1, eDsaForward, CPSS_PX_EDSA_FORWARD_STC);
}

void prv_c_to_lua_CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TO_UPSTREAM_PORT_STC(
    lua_State *L,
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TO_UPSTREAM_PORT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, eDsaForward, CPSS_PX_EDSA_FORWARD_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TO_UPSTREAM_PORT_STC);

void prv_lua_to_c_CPSS_PX_LOCATION_FULL_INFO_STC(
    lua_State *L,
    CPSS_PX_LOCATION_FULL_INFO_STC *val
)
{
    F_STRUCT(val, -1, logicalEntryInfo, CPSS_PX_LOGICAL_TABLE_INFO_STC);
    F_STRUCT(val, -1, hwEntryInfo, CPSS_PX_HW_INDEX_INFO_STC);
    F_STRUCT(val, -1, ramEntryInfo, CPSS_PX_RAM_INDEX_INFO_STC);
    F_BOOL(val, -1, isMppmInfoValid);
    F_STRUCT(val, -1, mppmMemLocation, CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC);
}

void prv_c_to_lua_CPSS_PX_LOCATION_FULL_INFO_STC(
    lua_State *L,
    CPSS_PX_LOCATION_FULL_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, logicalEntryInfo, CPSS_PX_LOGICAL_TABLE_INFO_STC);
    FO_STRUCT(val, t, hwEntryInfo, CPSS_PX_HW_INDEX_INFO_STC);
    FO_STRUCT(val, t, ramEntryInfo, CPSS_PX_RAM_INDEX_INFO_STC);
    FO_BOOL(val, t, isMppmInfoValid);
    FO_STRUCT(val, t, mppmMemLocation, CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_LOCATION_FULL_INFO_STC);

void prv_lua_to_c_CPSS_PX_RAM_INDEX_INFO_STC(
    lua_State *L,
    CPSS_PX_RAM_INDEX_INFO_STC *val
)
{
    F_ENUM(val, -1, memType, CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT);
    F_NUMBER(val, -1, ramRow, GT_U32);
    F_STRUCT(val, -1, memLocation, CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC);
}

void prv_c_to_lua_CPSS_PX_RAM_INDEX_INFO_STC(
    lua_State *L,
    CPSS_PX_RAM_INDEX_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, memType, CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT);
    FO_NUMBER(val, t, ramRow, GT_U32);
    FO_STRUCT(val, t, memLocation, CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_RAM_INDEX_INFO_STC);

void prv_lua_to_c_CPSS_PX_NET_SDMA_RX_COUNTERS_STC(
    lua_State *L,
    CPSS_PX_NET_SDMA_RX_COUNTERS_STC *val
)
{
    F_NUMBER(val, -1, rxInPkts, GT_U32);
    F_NUMBER(val, -1, rxInOctets, GT_U32);
}

void prv_c_to_lua_CPSS_PX_NET_SDMA_RX_COUNTERS_STC(
    lua_State *L,
    CPSS_PX_NET_SDMA_RX_COUNTERS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, rxInPkts, GT_U32);
    FO_NUMBER(val, t, rxInOctets, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_NET_SDMA_RX_COUNTERS_STC);

add_mgm_enum(CPSS_PX_CFG_CNTR_ENT);

void prv_lua_to_c_CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT(
    lua_State *L,
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT *val
)
{
    F_UNION_MEMBER_STRUCT(val, -1, info_802_1br, CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC);
    F_UNION_MEMBER_STRUCT(val, -1, info_dsa, CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC);
    F_UNION_MEMBER_STRUCT(val, -1, info_evb, CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC);
    F_UNION_MEMBER_STRUCT(val, -1, info_pre_da, CPSS_PX_EGRESS_SOURCE_PORT_PRE_DA_STC);
    F_UNION_MEMBER_STRUCT(val, -1, ptpPortInfo, CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC);
}

void prv_c_to_lua_CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT(
    lua_State *L,
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, info_802_1br, CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC);
    FO_STRUCT(val, t, info_dsa, CPSS_PX_EGRESS_SOURCE_PORT_DSA_STC);
    FO_STRUCT(val, t, info_evb, CPSS_PX_EGRESS_SOURCE_PORT_EVB_STC);
    FO_STRUCT(val, t, info_pre_da, CPSS_PX_EGRESS_SOURCE_PORT_PRE_DA_STC);
    FO_STRUCT(val, t, ptpPortInfo, CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT);
add_mgm_union(CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT);

add_mgm_enum(CPSS_PX_LOGICAL_TABLE_ENT);

void prv_lua_to_c_CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC(
    lua_State *L,
    CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC *val
)
{
    F_NUMBER(val, -1, pcid, GT_U32);
    F_STRUCT(val, -1, srcPortInfo, CPSS_PX_EGRESS_SRC_PORT_INFO_STC);
    F_BOOL(val, -1, upstreamPort);
    F_STRUCT(val, -1, ptpPortInfo, CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC);
}

void prv_c_to_lua_CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC(
    lua_State *L,
    CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, pcid, GT_U32);
    FO_STRUCT(val, t, srcPortInfo, CPSS_PX_EGRESS_SRC_PORT_INFO_STC);
    FO_BOOL(val, t, upstreamPort);
    FO_STRUCT(val, t, ptpPortInfo, CPSS_PX_EGRESS_SOURCE_PORT_PTP_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_EGRESS_SOURCE_PORT_802_1BR_STC);

void prv_lua_to_c_CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC(
    lua_State *L,
    CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, wExp, GT_32);
    F_NUMBER(val, -1, fbLsb, GT_U32);
    F_BOOL(val, -1, deltaEnable);
    F_NUMBER(val, -1, fbMin, GT_U32);
    F_NUMBER(val, -1, fbMax, GT_U32);
}

void prv_c_to_lua_CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC(
    lua_State *L,
    CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, wExp, GT_32);
    FO_NUMBER(val, t, fbLsb, GT_U32);
    FO_BOOL(val, t, deltaEnable);
    FO_NUMBER(val, t, fbMin, GT_U32);
    FO_NUMBER(val, t, fbMax, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC);

add_mgm_enum(CPSS_PX_INGRESS_HASH_PORT_INDEX_MODE_ENT);

void prv_lua_to_c_CPSS_PX_PORT_AP_PARAMS_STC(
    lua_State *L,
    CPSS_PX_PORT_AP_PARAMS_STC *val
)
{
    F_BOOL(val, -1, fcPause);
    F_ENUM(val, -1, fcAsmDir, CPSS_PX_PORT_AP_FLOW_CONTROL_ENT);
    F_BOOL(val, -1, fecSupported);
    F_BOOL(val, -1, fecRequired);
    F_BOOL(val, -1, noneceDisable);
    F_NUMBER(val, -1, laneNum, GT_U32);
    F_ARRAY_START(val, -1, modesAdvertiseArr);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            F_ARRAY_STRUCT(val, modesAdvertiseArr, idx, CPSS_PORT_MODE_SPEED_STC);
        }
    }
    F_ARRAY_END(val, -1, modesAdvertiseArr);
    F_ARRAY_START(val, -1, fecAbilityArr);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            F_ARRAY_ENUM(val, fecAbilityArr, idx, CPSS_PORT_FEC_MODE_ENT);
        }
    }
    F_ARRAY_END(val, -1, fecAbilityArr);
    F_ARRAY_START(val, -1, fecRequestedArr);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            F_ARRAY_ENUM(val, fecRequestedArr, idx, CPSS_PORT_FEC_MODE_ENT);
        }
    }
    F_ARRAY_END(val, -1, fecRequestedArr);
}

void prv_c_to_lua_CPSS_PX_PORT_AP_PARAMS_STC(
    lua_State *L,
    CPSS_PX_PORT_AP_PARAMS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, fcPause);
    FO_ENUM(val, t, fcAsmDir, CPSS_PX_PORT_AP_FLOW_CONTROL_ENT);
    FO_BOOL(val, t, fecSupported);
    FO_BOOL(val, t, fecRequired);
    FO_BOOL(val, t, noneceDisable);
    FO_NUMBER(val, t, laneNum, GT_U32);
    FO_ARRAY_START(val, t, modesAdvertiseArr);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            FO_ARRAY_STRUCT(val, modesAdvertiseArr, idx, CPSS_PORT_MODE_SPEED_STC);
        }
    }
    FO_ARRAY_END(val, t, modesAdvertiseArr);
    FO_ARRAY_START(val, t, fecAbilityArr);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            FO_ARRAY_ENUM(val, fecAbilityArr, idx, CPSS_PORT_FEC_MODE_ENT);
        }
    }
    FO_ARRAY_END(val, t, fecAbilityArr);
    FO_ARRAY_START(val, t, fecRequestedArr);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            FO_ARRAY_ENUM(val, fecRequestedArr, idx, CPSS_PORT_FEC_MODE_ENT);
        }
    }
    FO_ARRAY_END(val, t, fecRequestedArr);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_AP_PARAMS_STC);

void prv_lua_to_c_CPSS_PX_PORT_AP_STATUS_STC(
    lua_State *L,
    CPSS_PX_PORT_AP_STATUS_STC *val
)
{
    F_NUMBER(val, -1, postApPortNum, GT_U32);
    F_STRUCT(val, -1, portMode, CPSS_PORT_MODE_SPEED_STC);
    F_BOOL(val, -1, hcdFound);
    F_BOOL(val, -1, fecEnabled);
    F_BOOL(val, -1, fcRxPauseEn);
    F_BOOL(val, -1, fcTxPauseEn);
    F_ENUM(val, -1, fecType, CPSS_PORT_FEC_MODE_ENT);
}

void prv_c_to_lua_CPSS_PX_PORT_AP_STATUS_STC(
    lua_State *L,
    CPSS_PX_PORT_AP_STATUS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, postApPortNum, GT_U32);
    FO_STRUCT(val, t, portMode, CPSS_PORT_MODE_SPEED_STC);
    FO_BOOL(val, t, hcdFound);
    FO_BOOL(val, t, fecEnabled);
    FO_BOOL(val, t, fcRxPauseEn);
    FO_BOOL(val, t, fcTxPauseEn);
    FO_ENUM(val, t, fecType, CPSS_PORT_FEC_MODE_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_AP_STATUS_STC);

add_mgm_enum(CPSS_PX_TABLE_ENT);

add_mgm_enum(CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT);

void prv_lua_to_c_CPSS_PX_PORT_MAC_COUNTERS_STC(
    lua_State *L,
    CPSS_PX_PORT_MAC_COUNTERS_STC *val
)
{
    F_ARRAY_START(val, -1, mibCounter);
    {
        int idx;
        for (idx = 0; idx < 28; idx++) {
            F_ARRAY_STRUCT(val, mibCounter, idx, GT_U64);
        }
    }
    F_ARRAY_END(val, -1, mibCounter);
}

void prv_c_to_lua_CPSS_PX_PORT_MAC_COUNTERS_STC(
    lua_State *L,
    CPSS_PX_PORT_MAC_COUNTERS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, mibCounter);
    {
        int idx;
        for (idx = 0; idx < 28; idx++) {
            FO_ARRAY_STRUCT(val, mibCounter, idx, GT_U64);
        }
    }
    FO_ARRAY_END(val, t, mibCounter);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PX_PORT_MAC_COUNTERS_STC);

add_mgm_enum(CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT);

