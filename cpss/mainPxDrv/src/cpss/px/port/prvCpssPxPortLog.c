/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxPortLog.c
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
#include <cpss/px/log/private/prvCpssPxLog.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/cpssPxPortDynamicPAUnitBW.h>
#include <cpss/px/port/PizzaArbiter/cpssPxPortPizzaArbiter.h>
#include <cpss/px/port/cpssPxPortAp.h>
#include <cpss/px/port/cpssPxPortCn.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/port/cpssPxPortEcn.h>
#include <cpss/px/port/cpssPxPortMapping.h>
#include <cpss/px/port/cpssPxPortPfc.h>
#include <cpss/px/port/cpssPxPortStat.h>
#include <cpss/px/port/cpssPxPortSyncEther.h>
#include <cpss/px/port/cpssPxPortTxScheduler.h>
#include <cpss/px/port/cpssPxPortTxShaper.h>
#include <cpss/px/port/cpssPxPortTxTailDrop.h>
#include <cpss/px/port/private/prvCpssPxPortLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_PX_MIN_SPEED_ENT[]  =
{
    "CPSS_PX_MIN_SPEED_INVALID_E",
    "CPSS_PX_MIN_SPEED_500_Mbps_E",
    "CPSS_PX_MIN_SPEED_1000_Mbps_E",
    "CPSS_PX_MIN_SPEED_2000_Mbps_E",
    "CPSS_PX_MIN_SPEED_5000_Mbps_E",
    "CPSS_PX_MIN_SPEED_10000_Mbps_E",
    "CPSS_PX_MIN_SPEED_MAX"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_MIN_SPEED_ENT);
const PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_PX_PA_UNIT_ENT[]  =
{PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_PA_UNIT_UNDEFINED_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_PA_UNIT_RXDMA_0_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_PA_UNIT_TXQ_0_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_PA_UNIT_TXQ_1_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_PA_UNIT_TXDMA_0_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_PA_UNIT_TX_FIFO_0_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_PA_UNIT_MAX_E)
};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(CPSS_PX_PA_UNIT_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT[]  =
{
    "CPSS_PX_PORT_CN_MESSAGE_TYPE_QCN_E",
    "CPSS_PX_PORT_CN_MESSAGE_TYPE_CCFC_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_CN_PACKET_LENGTH_ENT[]  =
{
    "CPSS_PX_PORT_CN_LENGTH_ORIG_PACKET_E",
    "CPSS_PX_PORT_CN_LENGTH_1_5_KB_E",
    "CPSS_PX_PORT_CN_LENGTH_2_KB_E",
    "CPSS_PX_PORT_CN_LENGTH_10_KB_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_CN_PACKET_LENGTH_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT[]  =
{
    "CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_25M_E",
    "CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_156M_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT[]  =
{
    "CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_78M_E",
    "CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_156M_E",
    "CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_200M_E",
    "CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_312M_E",
    "CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_161M_E",
    "CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_164M_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_CTLE_BIAS_MODE_ENT[]  =
{
    "CPSS_PX_PORT_CTLE_BIAS_NORMAL_E",
    "CPSS_PX_PORT_CTLE_BIAS_HIGH_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_CTLE_BIAS_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_FC_MODE_ENT[]  =
{
    "CPSS_PX_PORT_FC_MODE_802_3X_E",
    "CPSS_PX_PORT_FC_MODE_PFC_E",
    "CPSS_PX_PORT_FC_MODE_LL_FC_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_FC_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_MAC_COUNTER_ENT[]  =
{
    "CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_RECEIVED_E",
    "CPSS_PX_PORT_MAC_COUNTER_BAD_OCTETS_RECEIVED_E",
    "CPSS_PX_PORT_MAC_COUNTER_CRC_ERRORS_SENT_E",
    "CPSS_PX_PORT_MAC_COUNTER_GOOD_UNICAST_FRAMES_RECEIVED_E",
    "CPSS_PX_PORT_MAC_COUNTER_BROADCAST_FRAMES_RECEIVED_E",
    "CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_RECEIVED_E",
    "CPSS_PX_PORT_MAC_COUNTER_FRAMES_64_OCTETS_E",
    "CPSS_PX_PORT_MAC_COUNTER_FRAMES_65_TO_127_OCTETS_E",
    "CPSS_PX_PORT_MAC_COUNTER_FRAMES_128_TO_255_OCTETS_E",
    "CPSS_PX_PORT_MAC_COUNTER_FRAMES_256_TO_511_OCTETS_E",
    "CPSS_PX_PORT_MAC_COUNTER_FRAMES_512_TO_1023_OCTETS_E",
    "CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_MAX_OCTETS_E",
    "CPSS_PX_PORT_MAC_COUNTER_FRAMES_1024_TO_1518_OCTETS_E",
    "CPSS_PX_PORT_MAC_COUNTER_GOOD_OCTETS_SENT_E",
    "CPSS_PX_PORT_MAC_COUNTER_UNICAST_FRAME_SENT_E",
    "CPSS_PX_PORT_MAC_COUNTER_MULTICAST_FRAMES_SENT_E",
    "CPSS_PX_PORT_MAC_COUNTER_BROADCAST_FRAMES_SENT_E",
    "CPSS_PX_PORT_MAC_COUNTER_FC_SENT_E",
    "CPSS_PX_PORT_MAC_COUNTER_FC_RECEIVED_E",
    "CPSS_PX_PORT_MAC_COUNTER_RECEIVED_FIFO_OVERRUN_E",
    "CPSS_PX_PORT_MAC_COUNTER_UNDERSIZE_E",
    "CPSS_PX_PORT_MAC_COUNTER_FRAGMENTS_E",
    "CPSS_PX_PORT_MAC_COUNTER_OVERSIZE_E",
    "CPSS_PX_PORT_MAC_COUNTER_JABBER_E",
    "CPSS_PX_PORT_MAC_COUNTER_RX_ERROR_FRAME_RECEIVED_E",
    "CPSS_PX_PORT_MAC_COUNTER_BAD_CRC_E",
    "CPSS_PX_PORT_MAC_COUNTER_COLLISION_E",
    "CPSS_PX_PORT_MAC_COUNTER_LATE_COLLISION_E",
    "CPSS_PX_PORT_MAC_COUNTER____LAST____E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_MAC_COUNTER_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT[]  =
{
    "CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E",
    "CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E",
    "CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1522_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT);
const PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_PX_PORT_MAPPING_TYPE_ENT[]  =
{PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_PORT_MAPPING_TYPE_MAX_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_PX_PORT_MAPPING_TYPE_INVALID_E)
};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(CPSS_PX_PORT_MAPPING_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT[]  =
{
    "CPSS_PX_PORT_PCS_LOOPBACK_DISABLE_E",
    "CPSS_PX_PORT_PCS_LOOPBACK_TX2RX_E",
    "CPSS_PX_PORT_PCS_LOOPBACK_RX2TX_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_PFC_COUNT_MODE_ENT[]  =
{
    "CPSS_PX_PORT_PFC_COUNT_BUFFERS_MODE_E",
    "CPSS_PX_PORT_PFC_COUNT_PACKETS_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_PFC_COUNT_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_PFC_ENABLE_ENT[]  =
{
    "CPSS_PX_PORT_PFC_ENABLE_TRIGGERING_ONLY_E",
    "CPSS_PX_PORT_PFC_ENABLE_TRIGGERING_AND_RESPONSE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_PFC_ENABLE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT[]  =
{
    "CPSS_PX_PORT_SERDES_LOOPBACK_DISABLE_E",
    "CPSS_PX_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E",
    "CPSS_PX_PORT_SERDES_LOOPBACK_DIGITAL_TX2RX_E",
    "CPSS_PX_PORT_SERDES_LOOPBACK_DIGITAL_RX2TX_E",
    "CPSS_PX_PORT_SERDES_LOOPBACK_MAX_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT[]  =
{
    "CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_1_E",
    "CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_E",
    "CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_3_E",
    "CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_4_E",
    "CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_5_E",
    "CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_8_E",
    "CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_16_E",
    "CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_5_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT[]  =
{
    "CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK0_E",
    "CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK1_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT[]  =
{
    "CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E",
    "CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_1_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT[]  =
{
    "CPSS_PX_PORT_TX_SCHEDULER_WRR_ARB_GROUP_0_E",
    "CPSS_PX_PORT_TX_SCHEDULER_WRR_ARB_GROUP_1_E",
    "CPSS_PX_PORT_TX_SCHEDULER_SP_ARB_GROUP_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT[]  =
{
    "CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_DISABLE_ALL_E",
    "CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_SHAPER_ONLY_E",
    "CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_SCHEDULER_ONLY_E",
    "CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_SHAPER_AND_SCHEDULER_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT[]  =
{
    "CPSS_PX_PORT_TX_SCHEDULER_PROFILE_0_E",
    "CPSS_PX_PORT_TX_SCHEDULER_PROFILE_1_E",
    "CPSS_PX_PORT_TX_SCHEDULER_PROFILE_2_E",
    "CPSS_PX_PORT_TX_SCHEDULER_PROFILE_3_E",
    "CPSS_PX_PORT_TX_SCHEDULER_PROFILE_4_E",
    "CPSS_PX_PORT_TX_SCHEDULER_PROFILE_5_E",
    "CPSS_PX_PORT_TX_SCHEDULER_PROFILE_6_E",
    "CPSS_PX_PORT_TX_SCHEDULER_PROFILE_7_E",
    "CPSS_PX_PORT_TX_SCHEDULER_PROFILE_8_E",
    "CPSS_PX_PORT_TX_SCHEDULER_PROFILE_9_E",
    "CPSS_PX_PORT_TX_SCHEDULER_PROFILE_10_E",
    "CPSS_PX_PORT_TX_SCHEDULER_PROFILE_11_E",
    "CPSS_PX_PORT_TX_SCHEDULER_PROFILE_12_E",
    "CPSS_PX_PORT_TX_SCHEDULER_PROFILE_13_E",
    "CPSS_PX_PORT_TX_SCHEDULER_PROFILE_14_E",
    "CPSS_PX_PORT_TX_SCHEDULER_PROFILE_15_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT[]  =
{
    "CPSS_PX_PORT_TX_SCHEDULER_WRR_PACKET_MODE_E",
    "CPSS_PX_PORT_TX_SCHEDULER_WRR_BYTE_MODE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT[]  =
{
    "CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_64_E",
    "CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_128_E",
    "CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_256_E",
    "CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_512_E",
    "CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_1K_E",
    "CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_2K_E",
    "CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_4K_E",
    "CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_8K_E",
    "CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_16K_E",
    "CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_32K_E",
    "CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_64K_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_TX_SHAPER_MODE_ENT[]  =
{
    "CPSS_PX_PORT_TX_SHAPER_BYTE_MODE_E",
    "CPSS_PX_PORT_TX_SHAPER_PACKET_MODE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_TX_SHAPER_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT[]  =
{
    "CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_BYTE_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_PACKET_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT[]  =
{
    "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT[]  =
{
    "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_0_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_1_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_2_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_3_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_4_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_5_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_6_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_7_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_8_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_9_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_10_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_11_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_12_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_13_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_14_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_15_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT[]  =
{
    "CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DISABLE_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ALL_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DP0_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_DP0_DP1_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT[]  =
{
    "CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_UNCONSTRAINED_E",
    "CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_CONSTRAINED_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_AP_FLOW_CONTROL_ENT[]  =
{
    "CPSS_PX_PORT_AP_FLOW_CONTROL_SYMMETRIC_E",
    "CPSS_PX_PORT_AP_FLOW_CONTROL_ASYMMETRIC_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_AP_FLOW_CONTROL_ENT);
const char * const prvCpssLogEnum_CPSS_PX_PORT_TX_SHAPER_GRANULARITY_ENT[]  =
{
    "CPSS_PX_PORT_TX_SHAPER_GRANULARITY_64_CORE_CLOCKS_E",
    "CPSS_PX_PORT_TX_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E",
    "CPSS_PX_PORT_TX_SHAPER_GRANULARITY_128_CORE_CLOCKS_E",
    "CPSS_PX_PORT_TX_SHAPER_GRANULARITY_256_CORE_CLOCKS_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PX_PORT_TX_SHAPER_GRANULARITY_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_PX_DETAILED_PORT_MAP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_DETAILED_PORT_MAP_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, valid);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, portMap, CPSS_PX_SHADOW_PORT_MAP_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, devType, CPSS_PP_FAMILY_TYPE_ENT);
    prvCpssLogStcLogStart(contextLib,  logType, "devState");
    PRV_CPSS_LOG_STC_STC_MAC((&valPtr->devState), pipe, CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PORT_AP_INTROP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PORT_AP_INTROP_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, attrBitMask);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, txDisDuration);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, abilityDuration);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, abilityMaxInterval);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, abilityFailMaxInterval);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, apLinkDuration);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, apLinkMaxInterval);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, pdLinkDuration);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, pdLinkMaxInterval);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PORT_AP_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PORT_AP_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, fcPause);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, fcAsmDir, CPSS_PX_PORT_AP_FLOW_CONTROL_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, fecSupported);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, fecRequired);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, noneceDisable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, laneNum);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, modesAdvertiseArr, 10, CPSS_PORT_MODE_SPEED_STC);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, fecAbilityArr, 10, CPSS_PORT_FEC_MODE_ENT);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, fecRequestedArr, 10, CPSS_PORT_FEC_MODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PORT_AP_STATS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PORT_AP_STATS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, txDisCnt);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, abilityCnt);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, abilitySuccessCnt);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, linkFailCnt);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, linkSuccessCnt);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, hcdResoultionTime);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, linkUpTime);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PORT_AP_STATUS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PORT_AP_STATUS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, postApPortNum);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, portMode, CPSS_PORT_MODE_SPEED_STC);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, hcdFound);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, fecEnabled);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, fcRxPauseEn);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, fcTxPauseEn);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, fecType, CPSS_PORT_FEC_MODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, link);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, speed, CPSS_PORT_SPEED_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, duplex, CPSS_PORT_DUPLEX_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, packetType);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, priority);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, dropPrecedence, CPSS_DP_LEVEL_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tc4pfc);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, localGeneratedPacketTypeAssignmentEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, localGeneratedPacketType);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, wExp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, fbLsb);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, deltaEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, fbMin);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, fbMax);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PORT_CN_PROFILE_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PORT_CN_PROFILE_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, cnAware);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, threshold);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, alpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, interval);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, randBitmap);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PORT_ECN_ENABLERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PORT_ECN_ENABLERS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, tcDpLimit);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, portLimit);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, tcLimit);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, sharedPoolLimit);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PORT_EGRESS_CNTR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PORT_EGRESS_CNTR_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, outFrames);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, txqFilterDisc);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PORT_MAC_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PORT_MAC_COUNTERS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, mibCounter, CPSS_PX_PORT_MAC_COUNTER____LAST____E, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PORT_MAP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PORT_MAP_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, physicalPortNumber);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, mappingType, CPSS_PX_PORT_MAPPING_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, interfaceNum);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, xonThreshold);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, xoffThreshold);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, xonAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, xoffAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PORT_SERDES_EYE_INPUT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PORT_SERDES_EYE_INPUT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, min_dwell_bits);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, max_dwell_bits);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PORT_SERDES_EYE_RESULT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PORT_SERDES_EYE_RESULT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, matrixPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, x_points);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, y_points);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, vbtcPtr);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hbtcPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, height_mv);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, width_mui);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, globalSerdesNum);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, egrMirrorDropCntr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, egrStcDropCntr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, egrQcnDropCntr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dropPfcEventsCntr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, clearPacketsDroppedCounter);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PORT_TX_SHAPER_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PORT_TX_SHAPER_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tokensRate);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, slowRateRatio);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, tokensRateGran, CPSS_PX_PORT_TX_SHAPER_GRANULARITY_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, portsPacketLength);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cpuPacketLength);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dp0MaxBuffNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dp0MaxDescNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dp1MaxBuffNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dp1MaxDescNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dp2MaxBuffNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dp2MaxDescNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tcMaxBuffNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tcMaxDescNum);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, dp0QueueAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, dp1QueueAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, dp2QueueAlpha, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, tcDpLimit);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, portLimit);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, tcLimit);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, sharedPoolLimit);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tcDp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, port);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, tc);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, pool);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, unitList, CPSS_PX_PA_UNIT_MAX_E+1, CPSS_PX_PA_UNIT_ENT);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, unitState, CPSS_PX_PA_UNIT_MAX_E, CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_SHADOW_PORT_MAP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_SHADOW_PORT_MAP_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, mappingType, CPSS_PX_PORT_MAPPING_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, macNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dmaNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, txqNum);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_PX_PA_UNIT_ENT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_FIELD_VAL_PTR_MAC(CPSS_PX_PA_UNIT_ENT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_ENUM_MAP_MAC(namePtr, *valPtr, CPSS_PX_PA_UNIT_ENT);
}
void prvCpssLogParamFuncStc_CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, totalConfiguredSlices);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, totalSlicesOnUnit);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, workConservingBit);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, pizzaMapLoadEnBit);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, slice_enable, 340, GT_BOOL);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, slice_occupied_by, 340, GT_U32);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_PX_DETAILED_PORT_MAP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_DETAILED_PORT_MAP_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_DETAILED_PORT_MAP_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_MIN_SPEED_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_MIN_SPEED_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_MIN_SPEED_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_MIN_SPEED_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_MIN_SPEED_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_MIN_SPEED_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PA_UNIT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PA_UNIT_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAP_MAC(namePtr, paramVal, CPSS_PX_PA_UNIT_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_AP_INTROP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PORT_AP_INTROP_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PORT_AP_INTROP_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_AP_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PORT_AP_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PORT_AP_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_AP_STATS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PORT_AP_STATS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PORT_AP_STATS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_AP_STATUS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PORT_AP_STATUS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PORT_AP_STATUS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_CN_PACKET_LENGTH_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_CN_PACKET_LENGTH_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_CN_PACKET_LENGTH_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_CN_PACKET_LENGTH_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_CN_PACKET_LENGTH_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PORT_CN_PACKET_LENGTH_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_CN_PROFILE_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PORT_CN_PROFILE_CONFIG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PORT_CN_PROFILE_CONFIG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_CTLE_BIAS_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_CTLE_BIAS_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_CTLE_BIAS_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_CTLE_BIAS_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_CTLE_BIAS_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PORT_CTLE_BIAS_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_ECN_ENABLERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PORT_ECN_ENABLERS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PORT_ECN_ENABLERS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_EGRESS_CNTR_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PORT_EGRESS_CNTR_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PORT_EGRESS_CNTR_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_FC_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_FC_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_FC_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_FC_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_FC_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PORT_FC_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_MAC_COUNTERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PORT_MAC_COUNTERS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PORT_MAC_COUNTERS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_MAC_COUNTER_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_MAC_COUNTER_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_MAC_COUNTER_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_MAPPING_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_MAPPING_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAP_MAC(namePtr, paramVal, CPSS_PX_PORT_MAPPING_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_MAP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PORT_MAP_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PORT_MAP_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_PFC_COUNT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_PFC_COUNT_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_PFC_COUNT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_PFC_COUNT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_PFC_COUNT_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PORT_PFC_COUNT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_PFC_ENABLE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_PFC_ENABLE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_PFC_ENABLE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_PFC_ENABLE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_PFC_ENABLE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PORT_PFC_ENABLE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_SERDES_EYE_INPUT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PORT_SERDES_EYE_INPUT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PORT_SERDES_EYE_INPUT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_SERDES_EYE_RESULT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PORT_SERDES_EYE_RESULT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PORT_SERDES_EYE_RESULT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SHAPER_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PORT_TX_SHAPER_CONFIG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PORT_TX_SHAPER_CONFIG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SHAPER_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_SHAPER_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_TX_SHAPER_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_SHAPER_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_SHAPER_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PORT_TX_SHAPER_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC PX_INOUT_CPSS_PM_PORT_PARAMS_STC_PTR_portParamsStcPtr = {
     "portParamsStcPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PM_PORT_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_INOUT_GT_U32_PTR_maxRatePtr = {
     "maxRatePtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_ADJUST_OPERATION_ENT_bcOp = {
     "bcOp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_ADJUST_OPERATION_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_DP_LEVEL_ENT_dpLevel = {
     "dpLevel", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DP_LEVEL_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_IP_PROTOCOL_STACK_ENT_protocolStack = {
     "protocolStack", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_IP_PROTOCOL_STACK_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PM_PORT_PARAMS_STC_PTR_portParamsStcPtr = {
     "portParamsStcPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PM_PORT_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORTS_BMP_STC_PTR_portsBmpPtr = {
     "portsBmpPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORTS_BMP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_AP_SERDES_RX_CONFIG_STC_PTR_rxOverrideParamsPtr = {
     "rxOverrideParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_AP_SERDES_RX_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_AP_SERDES_TX_OFFSETS_STC_PTR_serdesTxOffsetsPtr = {
     "serdesTxOffsetsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_AP_SERDES_TX_OFFSETS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_DIRECTION_ENT_direction = {
     "direction", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PORT_DIRECTION_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_DIRECTION_ENT_portDirection = {
     "portDirection", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PORT_DIRECTION_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_DUPLEX_ENT_dMode = {
     "dMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PORT_DUPLEX_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_EGRESS_CNT_MODE_ENT_setModeBmp = {
     "setModeBmp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PORT_EGRESS_CNT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_FEC_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PORT_FEC_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_FLOW_CONTROL_ENT_state = {
     "state", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PORT_FLOW_CONTROL_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_INTERFACE_MODE_ENT_ifMode = {
     "ifMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PORT_INTERFACE_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC_PTR_globalParamsStcPtr = {
     "globalParamsStcPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_MANAGER_PORT_TYPE_ENT_portType = {
     "portType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PORT_MANAGER_PORT_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_MANAGER_STC_PTR_portEventStcPtr = {
     "portEventStcPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_MANAGER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_MANAGER_UPDATE_PARAMS_STC_PTR_updateParamsStcPtr = {
     "updateParamsStcPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_MANAGER_UPDATE_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_PCS_RESET_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PORT_PCS_RESET_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT_enable = {
     "enable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_REF_CLOCK_SOURCE_ENT_refClockSource = {
     "refClockSource", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PORT_REF_CLOCK_SOURCE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_RX_FC_PROFILE_SET_ENT_profileSet = {
     "profileSet", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PORT_RX_FC_PROFILE_SET_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT_portTuningMode = {
     "portTuningMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_SERDES_RX_CONFIG_STC_PTR_serdesRxCfgPtr = {
     "serdesRxCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_SERDES_RX_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_SERDES_SPEED_ENT_serdesFrequency = {
     "serdesFrequency", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PORT_SERDES_SPEED_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_SERDES_TUNE_STC_PTR_tuneValuesPtr = {
     "tuneValuesPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_SERDES_TUNE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_SERDES_TX_CONFIG_STC_PTR_serdesTxCfgPtr = {
     "serdesTxCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_SERDES_TX_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_SPEED_ENT_speed = {
     "speed", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PORT_SPEED_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_XGMII_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PORT_XGMII_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PORT_XG_FIXED_IPG_ENT_ipgBase = {
     "ipgBase", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PORT_XG_FIXED_IPG_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_MIN_SPEED_ENT_minimalPortSpeedMBps = {
     "minimalPortSpeedMBps", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_MIN_SPEED_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PA_UNIT_ENT_unit = {
     "unit", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PA_UNIT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_AP_INTROP_STC_PTR_apIntropPtr = {
     "apIntropPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_AP_INTROP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_AP_PARAMS_STC_PTR_apParamsPtr = {
     "apParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_AP_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC_PTR_portAnAdvertismentPtr = {
     "portAnAdvertismentPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC_PTR_cnmGenerationCfgPtr = {
     "cnmGenerationCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC_PTR_fbCalcCfgPtr = {
     "fbCalcCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT_mType = {
     "mType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_CN_PACKET_LENGTH_ENT_packetLength = {
     "packetLength", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_CN_PACKET_LENGTH_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_CN_PROFILE_CONFIG_STC_PTR_cnProfileCfgPtr = {
     "cnProfileCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_CN_PROFILE_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT_inputFreq = {
     "inputFreq", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT_outputFreq = {
     "outputFreq", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_CTLE_BIAS_MODE_ENT_PTR_ctleBiasValuePtr = {
     "ctleBiasValuePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_CTLE_BIAS_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_CTLE_BIAS_MODE_ENT_ctleBiasValue = {
     "ctleBiasValue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_CTLE_BIAS_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_ECN_ENABLERS_STC_PTR_enablersPtr = {
     "enablersPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_ECN_ENABLERS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_FC_MODE_ENT_fcMode = {
     "fcMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_FC_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_MAC_COUNTER_ENT_cntrName = {
     "cntrName", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_MAC_COUNTER_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT_counterMode = {
     "counterMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_MAPPING_TYPE_ENT_origPortType = {
     "origPortType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_MAPPING_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_MAP_STC_PTR_portMapArrayPtr = {
     "portMapArrayPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_MAP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_PFC_COUNT_MODE_ENT_pfcCountMode = {
     "pfcCountMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_PFC_COUNT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_PFC_ENABLE_ENT_pfcEnable = {
     "pfcEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_PFC_ENABLE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC_PTR_pfcProfileCfgPtr = {
     "pfcProfileCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_SERDES_EYE_INPUT_STC_PTR_eye_inputPtr = {
     "eye_inputPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_SERDES_EYE_INPUT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT_value = {
     "value", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT_recoveryClkType = {
     "recoveryClkType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT_clockSelect = {
     "clockSelect", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT_arbGroup = {
     "arbGroup", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT_bcMode = {
     "bcMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_profile = {
     "profile", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_profileSet = {
     "profileSet", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT_wrrMode = {
     "wrrMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT_wrrMtu = {
     "wrrMtu", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_SHAPER_CONFIG_STC_PTR_configsPtr = {
     "configsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_TX_SHAPER_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_SHAPER_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_TX_SHAPER_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT_portAlpha = {
     "portAlpha", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_profile = {
     "profile", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_profileSet = {
     "profileSet", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC_PTR_tailDropProfileParamsPtr = {
     "tailDropProfileParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT_enableMode = {
     "enableMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT_policy = {
     "policy", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC_PTR_enablerPtr = {
     "enablerPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC_PTR_maskLsbPtr = {
     "maskLsbPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_PTR_overrideEnablePtr = {
     "overrideEnablePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_apEnable = {
     "apEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_avbModeEnable = {
     "avbModeEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_invertRx = {
     "invertRx", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_invertTx = {
     "invertTx", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_overrideEnable = {
     "overrideEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_pauseAdvertise = {
     "pauseAdvertise", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_powerUp = {
     "powerUp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_send = {
     "send", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_srvCpuEnable = {
     "srvCpuEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_state = {
     "state", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_status = {
     "status", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_BOOL_vosOverride = {
     "vosOverride", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_ETHERADDR_PTR_macPtr = {
     "macPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_ETHERADDR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_PHYSICAL_PORT_NUM_firstPhysicalPortNumber = {
     "firstPhysicalPortNumber", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_PHYSICAL_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_PORT_NUM_targetPort = {
     "targetPort", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_PTR_rxSerdesLaneArr = {
     "rxSerdesLaneArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_PTR_serdesOptAlgBmpPtr = {
     "serdesOptAlgBmpPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_PTR_txSerdesLaneArr = {
     "txSerdesLaneArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_almostFullThreshold = {
     "almostFullThreshold", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_availableBuff = {
     "availableBuff", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_baseline = {
     "baseline", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_bcValue = {
     "bcValue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_buffsNum = {
     "buffsNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_burstSize = {
     "burstSize", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_cntrIdx = {
     "cntrIdx", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_cntrNum = {
     "cntrNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_cntrSetNum = {
     "cntrSetNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_cpllNum = {
     "cpllNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_dp = {
     "dp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_dropThreshold = {
     "dropThreshold", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_fullThreshold = {
     "fullThreshold", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_ipg = {
     "ipg", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_laneBmp = {
     "laneBmp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_length = {
     "length", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_limit = {
     "limit", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_maxBuffNum = {
     "maxBuffNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_maxDescNum = {
     "maxDescNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_maxSharedBufferLimit = {
     "maxSharedBufferLimit", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_mcastMaxBuffNum = {
     "mcastMaxBuffNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_mcastMaxDescNum = {
     "mcastMaxDescNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_mruSize = {
     "mruSize", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_numCrcBytes = {
     "numCrcBytes", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_numOfBits = {
     "numOfBits", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_origPortNum = {
     "origPortNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_pfcCounterNum = {
     "pfcCounterNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_pfcTimer = {
     "pfcTimer", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_pipeBandwithInGbps = {
     "pipeBandwithInGbps", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_poolNum = {
     "poolNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_portGroup = {
     "portGroup", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_portGroupId = {
     "portGroupId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_portGroupNum = {
     "portGroupNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_portMapArraySize = {
     "portMapArraySize", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_portMaxBuffLimit = {
     "portMaxBuffLimit", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_portMaxDescLimit = {
     "portMaxDescLimit", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_profileIndex = {
     "profileIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_qcnSampleInterval = {
     "qcnSampleInterval", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_qlenOld = {
     "qlenOld", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_rxBufLimit = {
     "rxBufLimit", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_serdesNum = {
     "serdesNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_serdesOptAlgBmp = {
     "serdesOptAlgBmp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_shaperToPortRateRatio = {
     "shaperToPortRateRatio", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_squelch = {
     "squelch", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_tailDropDumpBmp = {
     "tailDropDumpBmp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_tc = {
     "tc", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_tcQueue = {
     "tcQueue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_threshold = {
     "threshold", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_timer = {
     "timer", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_udbpIndex = {
     "udbpIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_value = {
     "value", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_wrrWeight = {
     "wrrWeight", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_xoffLimit = {
     "xoffLimit", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_xoffThreshold = {
     "xoffThreshold", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_xonLimit = {
     "xonLimit", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U32_xonThreshold = {
     "xonThreshold", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U8_entryIndex = {
     "entryIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U8_macSaLsb = {
     "macSaLsb", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U8_max_LF = {
     "max_LF", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U8_min_LF = {
     "min_LF", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U8_number = {
     "number", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U8_serdesLane = {
     "serdesLane", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_GT_U8_tcQueue = {
     "tcQueue", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_IN_MV_HWS_AP_DEBUG_LOG_OUTPUT_ENT_output = {
     "output", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(MV_HWS_AP_DEBUG_LOG_OUTPUT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_ADJUST_OPERATION_ENT_PTR_bcOpPtr = {
     "bcOpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_ADJUST_OPERATION_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_DP_LEVEL_ENT_PTR_dpLevelPtr = {
     "dpLevelPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DP_LEVEL_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_FCFEC_COUNTERS_STC_PTR_fcfecCountersPtr = {
     "fcfecCountersPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_FCFEC_COUNTERS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PM_PORT_PARAMS_STC_PTR_portParamsStcPtr = {
     "portParamsStcPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PM_PORT_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_ATTRIBUTES_STC_PTR_portAttributSetArrayPtr = {
     "portAttributSetArrayPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_ATTRIBUTES_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_DUPLEX_ENT_PTR_dModePtr = {
     "dModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_DUPLEX_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_EGRESS_CNT_MODE_ENT_PTR_setModeBmpPtr = {
     "setModeBmpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_EGRESS_CNT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_FEC_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_FEC_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_FLOW_CONTROL_ENT_PTR_statePtr = {
     "statePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_FLOW_CONTROL_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_INTERFACE_MODE_ENT_PTR_ifModePtr = {
     "ifModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_INTERFACE_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_MAC_CG_COUNTER_SET_STC_PTR_cgMibStcPtr = {
     "cgMibStcPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_MAC_CG_COUNTER_SET_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_MAC_STATUS_STC_PTR_portMacStatusPtr = {
     "portMacStatusPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_MAC_STATUS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_MAC_TYPE_ENT_PTR_portMacTypePtr = {
     "portMacTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_MAC_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_MANAGER_STATISTICS_STC_PTR_portStatStcPtr = {
     "portStatStcPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_MANAGER_STATISTICS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_MANAGER_STATUS_STC_PTR_portStagePtr = {
     "portStagePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_MANAGER_STATUS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT_PTR_enablePtr = {
     "enablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_REF_CLOCK_SOURCE_ENT_PTR_refClockSourcePtr = {
     "refClockSourcePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_REF_CLOCK_SOURCE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_RX_FC_PROFILE_SET_ENT_PTR_profileSetPtr = {
     "profileSetPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_RX_FC_PROFILE_SET_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT_PTR_rxTuneStatusPtr = {
     "rxTuneStatusPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT_PTR_txTuneStatusPtr = {
     "txTuneStatusPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_SERDES_RX_CONFIG_STC_PTR_serdesRxCfgPtr = {
     "serdesRxCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_SERDES_RX_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_SERDES_TUNE_STC_PTR_serdesTunePtr = {
     "serdesTunePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_SERDES_TUNE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_SERDES_TUNE_STC_PTR_tuneValuesPtr = {
     "tuneValuesPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_SERDES_TUNE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_SERDES_TX_CONFIG_STC_PTR_serdesTxCfgPtr = {
     "serdesTxCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_SERDES_TX_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_SPEED_ENT_PTR_speedPtr = {
     "speedPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_SPEED_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_XGMII_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_XGMII_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PORT_XG_FIXED_IPG_ENT_PTR_ipgBasePtr = {
     "ipgBasePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PORT_XG_FIXED_IPG_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_DETAILED_PORT_MAP_STC_PTR_portMapShadowPtr = {
     "portMapShadowPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_DETAILED_PORT_MAP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC_PTR_pizzaDeviceStatePtr = {
     "pizzaDeviceStatePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_MIN_SPEED_ENT_PTR_minimalPortSpeedResolutionInMBpsPtr = {
     "minimalPortSpeedResolutionInMBpsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_MIN_SPEED_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PACKET_TYPE_PTR_packetTypePtr = {
     "packetTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PACKET_TYPE)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_AP_INTROP_STC_PTR_apIntropPtr = {
     "apIntropPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_AP_INTROP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_AP_PARAMS_STC_PTR_apParamsPtr = {
     "apParamsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_AP_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_AP_STATS_STC_PTR_apStatsPtr = {
     "apStatsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_AP_STATS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_AP_STATUS_STC_PTR_apStatusPtr = {
     "apStatusPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_AP_STATUS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC_PTR_portAnAdvertismentPtr = {
     "portAnAdvertismentPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC_PTR_cnmGenerationCfgPtr = {
     "cnmGenerationCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC_PTR_fbCalcCfgPtr = {
     "fbCalcCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT_PTR_mTypePtr = {
     "mTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_CN_PACKET_LENGTH_ENT_PTR_packetLengthPtr = {
     "packetLengthPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_CN_PACKET_LENGTH_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_CN_PROFILE_CONFIG_STC_PTR_cnProfileCfgPtr = {
     "cnProfileCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_CN_PROFILE_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_ECN_ENABLERS_STC_PTR_enablersPtr = {
     "enablersPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_ECN_ENABLERS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_EGRESS_CNTR_STC_PTR_egrCntrPtr = {
     "egrCntrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_EGRESS_CNTR_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_FC_MODE_ENT_PTR_fcModePtr = {
     "fcModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_FC_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_MAC_COUNTERS_STC_PTR_countersPtr = {
     "countersPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_MAC_COUNTERS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_MAC_COUNTERS_STC_PTR_portMacCounterSetArrayPtr = {
     "portMacCounterSetArrayPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_MAC_COUNTERS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT_PTR_counterModePtr = {
     "counterModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_MAP_STC_PTR_portMapArrayPtr = {
     "portMapArrayPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_MAP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_PFC_COUNT_MODE_ENT_PTR_pfcCountModePtr = {
     "pfcCountModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_PFC_COUNT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_PFC_ENABLE_ENT_PTR_pfcEnablePtr = {
     "pfcEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_PFC_ENABLE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC_PTR_pfcProfileCfgPtr = {
     "pfcProfileCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_SERDES_EYE_RESULT_STC_PTR_eye_resultsPtr = {
     "eye_resultsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_SERDES_EYE_RESULT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC_PTR_dropCntrStcPtr = {
     "dropCntrStcPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT_PTR_valuePtr = {
     "valuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT_PTR_arbGroupPtr = {
     "arbGroupPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT_PTR_bcModePtr = {
     "bcModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_PTR_profileSetPtr = {
     "profileSetPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT_PTR_wrrModePtr = {
     "wrrModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT_PTR_wrrMtuPtr = {
     "wrrMtuPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_SHAPER_CONFIG_STC_PTR_configsPtr = {
     "configsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_TX_SHAPER_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_SHAPER_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_TX_SHAPER_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT_PTR_portAlphaPtr = {
     "portAlphaPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_PTR_profileSetPtr = {
     "profileSetPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC_PTR_tailDropProfileParamsPtr = {
     "tailDropProfileParamsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT_PTR_enableModePtr = {
     "enableModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT_PTR_policyPtr = {
     "policyPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC_PTR_enablerPtr = {
     "enablerPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC_PTR_maskLsbPtr = {
     "maskLsbPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_CPSS_RSFEC_COUNTERS_STC_PTR_rsfecCountersPtr = {
     "rsfecCountersPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_RSFEC_COUNTERS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_apEnablePtr = {
     "apEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_avbModeEnablePtr = {
     "avbModeEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_captureIsDonePtr = {
     "captureIsDonePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_cdrLockPtr = {
     "cdrLockPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_enabledPtr = {
     "enabledPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_gbLockPtr = {
     "gbLockPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_invertRx = {
     "invertRx", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_invertRxPtr = {
     "invertRxPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_invertTx = {
     "invertTx", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_invertTxPtr = {
     "invertTxPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_isCpuPtr = {
     "isCpuPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_isLinkUpPtr = {
     "isLinkUpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_isLocalFaultPtr = {
     "isLocalFaultPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_isRemoteFaultPtr = {
     "isRemoteFaultPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_isValidPtr = {
     "isValidPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_overrideEnablePtr = {
     "overrideEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_pauseAdvertisePtr = {
     "pauseAdvertisePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_signalStatePtr = {
     "signalStatePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_srvCpuEnablePtr = {
     "srvCpuEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_statePtr = {
     "statePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_statusPtr = {
     "statusPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_supportedPtr = {
     "supportedPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_syncPtr = {
     "syncPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_BOOL_PTR_vosOverridePtr = {
     "vosOverridePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_ETHERADDR_PTR_macPtr = {
     "macPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_ETHERADDR)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_PHYSICAL_PORT_NUM_PTR_physicalPortNumPtr = {
     "physicalPortNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_PHYSICAL_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_PHYSICAL_PORT_NUM_PTR_portNumPtr = {
     "portNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_PHYSICAL_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_PORT_NUM_PTR_targetPortPtr = {
     "targetPortPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_PORT_NUM)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U16_PTR_intropAbilityMaxIntervalPtr = {
     "intropAbilityMaxIntervalPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U16)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U16_PTR_numberPtr = {
     "numberPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U16)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_almostFullThresholdPtr = {
     "almostFullThresholdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_availableBuffPtr = {
     "availableBuffPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_baselinePtr = {
     "baselinePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_baudRatePtr = {
     "baudRatePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_bcValuePtr = {
     "bcValuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_buffsNumPtr = {
     "buffsNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_burstSizePtr = {
     "burstSizePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_cntrPtr = {
     "cntrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_dfeResPtr = {
     "dfeResPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_dropThresholdPtr = {
     "dropThresholdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_fullThresholdPtr = {
     "fullThresholdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_ipgPtr = {
     "ipgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_laneNumPtr = {
     "laneNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_lengthPtr = {
     "lengthPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_limitPtr = {
     "limitPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_maxBuffNumPtr = {
     "maxBuffNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_maxDescNumPtr = {
     "maxDescNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_maxRatePtr = {
     "maxRatePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_maxSharedBufferLimitPtr = {
     "maxSharedBufferLimitPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_mcCntrPtr = {
     "mcCntrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_mcastMaxBuffNumPtr = {
     "mcastMaxBuffNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_mcastMaxDescNumPtr = {
     "mcastMaxDescNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_mruSizePtr = {
     "mruSizePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_numCrcBytesPtr = {
     "numCrcBytesPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_numOfBuffersPtr = {
     "numOfBuffersPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_numOfPacketsPtr = {
     "numOfPacketsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_numberPtr = {
     "numberPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_pfcCounterNumPtr = {
     "pfcCounterNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_pfcCounterValuePtr = {
     "pfcCounterValuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_pipeBandwithInGbpsPtr = {
     "pipeBandwithInGbpsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_poolNumPtr = {
     "poolNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_portMaxBuffLimitPtr = {
     "portMaxBuffLimitPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_portMaxDescLimitPtr = {
     "portMaxDescLimitPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_profileIndexPtr = {
     "profileIndexPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_qcnSampleIntervalPtr = {
     "qcnSampleIntervalPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_qlenOldPtr = {
     "qlenOldPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_rxBufLimitPtr = {
     "rxBufLimitPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_rxSerdesLaneArr = {
     "rxSerdesLaneArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_shaperToPortRateRatioPtr = {
     "shaperToPortRateRatioPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_tcPtr = {
     "tcPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_tcQueuePtr = {
     "tcQueuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_thresholdPtr = {
     "thresholdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_timerPtr = {
     "timerPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_txSerdesLaneArr = {
     "txSerdesLaneArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_wrrWeightPtr = {
     "wrrWeightPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_xoffLimitPtr = {
     "xoffLimitPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_xoffThresholdPtr = {
     "xoffThresholdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_xonLimitPtr = {
     "xonLimitPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U32_PTR_xonThresholdPtr = {
     "xonThresholdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U64_PTR_cntrValuePtr = {
     "cntrValuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U8_PTR_macSaLsbPtr = {
     "macSaLsbPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC PX_OUT_GT_U8_PTR_numberPtr = {
     "numberPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortManagerPortParamsStructInit_PARAMS[] =  {
    &PX_IN_CPSS_PORT_MANAGER_PORT_TYPE_ENT_portType,
    &PX_INOUT_CPSS_PM_PORT_PARAMS_STC_PTR_portParamsStcPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortEcnMarkingEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_IP_PROTOCOL_STACK_ENT_protocolStack,
    &PX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortEcnMarkingEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_IP_PROTOCOL_STACK_ENT_protocolStack,
    &PX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortModeSpeedSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PORTS_BMP_STC_PTR_portsBmpPtr,
    &PX_IN_GT_BOOL_powerUp,
    &PX_IN_CPSS_PORT_INTERFACE_MODE_ENT_ifMode,
    &PX_IN_CPSS_PORT_SPEED_ENT_speed
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortBufMgProfileRxBufLimitSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PORT_RX_FC_PROFILE_SET_ENT_profileSet,
    &PX_IN_GT_U32_rxBufLimit
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortBufMgProfileXoffLimitSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PORT_RX_FC_PROFILE_SET_ENT_profileSet,
    &PX_IN_GT_U32_xoffLimit
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortBufMgProfileXonLimitSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PORT_RX_FC_PROFILE_SET_ENT_profileSet,
    &PX_IN_GT_U32_xonLimit
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortBufMgProfileRxBufLimitGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PORT_RX_FC_PROFILE_SET_ENT_profileSet,
    &PX_OUT_GT_U32_PTR_rxBufLimitPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortBufMgProfileXoffLimitGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PORT_RX_FC_PROFILE_SET_ENT_profileSet,
    &PX_OUT_GT_U32_PTR_xoffLimitPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortBufMgProfileXonLimitGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PORT_RX_FC_PROFILE_SET_ENT_profileSet,
    &PX_OUT_GT_U32_PTR_xonLimitPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcPacketClassificationEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PACKET_TYPE_packetType,
    &PX_IN_GT_U32_udbpIndex,
    &PX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortDynamicPizzaArbiterIfWorkConservingModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PA_UNIT_ENT_unit,
    &PX_IN_GT_BOOL_status
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortDynamicPizzaArbiterIfWorkConservingModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PA_UNIT_ENT_unit,
    &PX_OUT_GT_BOOL_PTR_statusPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnMessageGenerationConfigSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC_PTR_cnmGenerationCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnFbCalcConfigSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC_PTR_fbCalcCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnMessageTypeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT_mType
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnPacketLengthSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_CN_PACKET_LENGTH_ENT_packetLength
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPhysicalPortMapReverseMappingGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_MAPPING_TYPE_ENT_origPortType,
    &PX_IN_GT_U32_origPortNum,
    &PX_OUT_GT_PHYSICAL_PORT_NUM_PTR_physicalPortNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcCountingModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_PFC_COUNT_MODE_ENT_pfcCountMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_PFC_ENABLE_ENT_pfcEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSyncEtherRecoveryClkConfigSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT_recoveryClkType,
    &PX_IN_GT_BOOL_enable,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_laneNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSyncEtherRecoveryClkConfigGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT_recoveryClkType,
    &PX_OUT_GT_BOOL_PTR_enablePtr,
    &PX_OUT_GT_PHYSICAL_PORT_NUM_PTR_portNumPtr,
    &PX_OUT_GT_U32_PTR_laneNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxSchedulerProfileByteCountChangeEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_profile,
    &PX_IN_CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT_bcMode,
    &PX_IN_CPSS_ADJUST_OPERATION_ENT_bcOp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxSchedulerProfileCountModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_profile,
    &PX_IN_CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT_wrrMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxSchedulerProfileByteCountChangeEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_profile,
    &PX_OUT_CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT_PTR_bcModePtr,
    &PX_OUT_CPSS_ADJUST_OPERATION_ENT_PTR_bcOpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxSchedulerProfileCountModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_profile,
    &PX_OUT_CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT_PTR_wrrModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcTimerMapEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_profileSet,
    &PX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxSchedulerArbitrationGroupSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_profileSet,
    &PX_IN_GT_U32_tcQueue,
    &PX_IN_CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT_arbGroup
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcShaperToPortRateRatioSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_profileSet,
    &PX_IN_GT_U32_tcQueue,
    &PX_IN_GT_U32_shaperToPortRateRatio
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxSchedulerWrrProfileSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_profileSet,
    &PX_IN_GT_U32_tcQueue,
    &PX_IN_GT_U32_wrrWeight
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxSchedulerArbitrationGroupGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_profileSet,
    &PX_IN_GT_U32_tcQueue,
    &PX_OUT_CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT_PTR_arbGroupPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcShaperToPortRateRatioGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_profileSet,
    &PX_IN_GT_U32_tcQueue,
    &PX_OUT_GT_U32_PTR_shaperToPortRateRatioPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxSchedulerWrrProfileGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_profileSet,
    &PX_IN_GT_U32_tcQueue,
    &PX_OUT_GT_U32_PTR_wrrWeightPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcTimerMapEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_profileSet,
    &PX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxSchedulerWrrMtuSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT_wrrMtu
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxShaperConfigurationSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_SHAPER_CONFIG_STC_PTR_configsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropProfileBufferConsumptionModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_profile,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_profile,
    &PX_IN_GT_U32_dp,
    &PX_IN_GT_U32_tc,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC_PTR_enablerPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_profile,
    &PX_IN_GT_U32_dp,
    &PX_IN_GT_U32_tc,
    &PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_ENABLER_STC_PTR_enablerPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropProfileTcSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_profile,
    &PX_IN_GT_U32_tc,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC_PTR_tailDropProfileParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropProfileTcSharingSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_profile,
    &PX_IN_GT_U32_tc,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT_enableMode,
    &PX_IN_GT_U32_poolNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropProfileTcGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_profile,
    &PX_IN_GT_U32_tc,
    &PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_TC_STC_PTR_tailDropProfileParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropProfileTcSharingGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_profile,
    &PX_IN_GT_U32_tc,
    &PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_DP_MODE_ENT_PTR_enableModePtr,
    &PX_OUT_GT_U32_PTR_poolNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropProfileBufferConsumptionModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_profile,
    &PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortEcnMarkingTailDropProfileEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_profileSet,
    &PX_IN_CPSS_PX_PORT_ECN_ENABLERS_STC_PTR_enablersPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropProfileSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_profileSet,
    &PX_IN_GT_U32_portMaxBuffLimit,
    &PX_IN_GT_U32_portMaxDescLimit,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT_portAlpha
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnProfileQueueConfigSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_profileSet,
    &PX_IN_GT_U32_tcQueue,
    &PX_IN_CPSS_PX_PORT_CN_PROFILE_CONFIG_STC_PTR_cnProfileCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnProfileQueueConfigGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_profileSet,
    &PX_IN_GT_U32_tcQueue,
    &PX_OUT_CPSS_PX_PORT_CN_PROFILE_CONFIG_STC_PTR_cnProfileCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortEcnMarkingTailDropProfileEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_profileSet,
    &PX_OUT_CPSS_PX_PORT_ECN_ENABLERS_STC_PTR_enablersPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropProfileGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_profileSet,
    &PX_OUT_GT_U32_PTR_portMaxBuffLimitPtr,
    &PX_OUT_GT_U32_PTR_portMaxDescLimitPtr,
    &PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT_PTR_portAlphaPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropSharedPolicySet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT_policy
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropWrtdMasksSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC_PTR_maskLsbPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnQueueStatusModeEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_BOOL_enable,
    &PX_IN_GT_PORT_NUM_targetPort
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortApPortEnableCtrlSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_BOOL_srvCpuEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortVosOverrideControlModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_BOOL_vosOverride
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortMacSaBaseSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_ETHERADDR_PTR_macPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPhysicalPortMapGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_firstPhysicalPortNumber,
    &PX_IN_GT_U32_portMapArraySize,
    &PX_OUT_CPSS_PX_PORT_MAP_STC_PTR_portMapArrayPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortManagerPortParamsSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PM_PORT_PARAMS_STC_PTR_portParamsStcPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPreambleLengthSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PORT_DIRECTION_ENT_direction,
    &PX_IN_GT_U32_length
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPreambleLengthGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PORT_DIRECTION_ENT_direction,
    &PX_OUT_GT_U32_PTR_lengthPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCrcNumBytesSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PORT_DIRECTION_ENT_portDirection,
    &PX_IN_GT_U32_numCrcBytes
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCrcNumBytesGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PORT_DIRECTION_ENT_portDirection,
    &PX_OUT_GT_U32_PTR_numCrcBytesPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortDuplexModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PORT_DUPLEX_ENT_dMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortFecModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PORT_FEC_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortFlowControlEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PORT_FLOW_CONTROL_ENT_state
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortFaultSendSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PORT_INTERFACE_MODE_ENT_ifMode,
    &PX_IN_CPSS_PORT_SPEED_ENT_speed,
    &PX_IN_GT_BOOL_send
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortInterfaceSpeedSupportGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PORT_INTERFACE_MODE_ENT_ifMode,
    &PX_IN_CPSS_PORT_SPEED_ENT_speed,
    &PX_OUT_GT_BOOL_PTR_supportedPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortManagerEventSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PORT_MANAGER_STC_PTR_portEventStcPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPcsResetSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PORT_PCS_RESET_MODE_ENT_mode,
    &PX_IN_GT_BOOL_state
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPeriodicFcEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortBufMgRxProfileSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PORT_RX_FC_PROFILE_SET_ENT_profileSet
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesAutoTune_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT_portTuningMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesAutoTuneExt_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT_portTuningMode,
    &PX_IN_GT_U32_serdesOptAlgBmp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSpeedSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PORT_SPEED_ENT_speed
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortXGmiiModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PORT_XGMII_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortIpgBaseSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PORT_XG_FIXED_IPG_ENT_ipgBase
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnMessagePortMapEntrySet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_PORTS_BMP_portsBmp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortApIntropSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_PORT_AP_INTROP_STC_PTR_apIntropPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortAutoNegAdvertismentConfigSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC_PTR_portAnAdvertismentPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortFlowControlModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_PORT_FC_MODE_ENT_fcMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortMacCounterGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_PORT_MAC_COUNTER_ENT_cntrName,
    &PX_OUT_GT_U64_PTR_cntrValuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortMacOversizedPacketsCounterModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT_counterMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPcsLoopbackModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesLoopbackModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxSchedulerProfileIdSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_profileSet
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxShaperModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_PORT_TX_SHAPER_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropProfileIdSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_profileSet
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCtleBiasOverrideEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_BOOL_PTR_overrideEnablePtr,
    &PX_IN_CPSS_PX_PORT_CTLE_BIAS_MODE_ENT_PTR_ctleBiasValuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortApPortConfigSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_BOOL_apEnable,
    &PX_IN_CPSS_PX_PORT_AP_PARAMS_STC_PTR_apParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxShaperEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_BOOL_enable,
    &PX_IN_GT_BOOL_avbModeEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortRefClockSourceOverrideEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_BOOL_overrideEnable,
    &PX_IN_CPSS_PORT_REF_CLOCK_SOURCE_ENT_refClockSource
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesPowerStatusSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_BOOL_powerUp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortForceLinkPassEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_BOOL_state
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortFlowCntrlAutoNegEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_BOOL_state,
    &PX_IN_GT_BOOL_pauseAdvertise
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesAutoTuneOptAlgGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_PTR_serdesOptAlgBmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortBufMgTxDmaBurstLimitThresholdsSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_almostFullThreshold,
    &PX_IN_GT_U32_fullThreshold
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxSchedulerShaperByteCountChangeValueSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_bcValue
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxShaperProfileSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_burstSize,
    &PX_INOUT_GT_U32_PTR_maxRatePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnFcTimerSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_index,
    &PX_IN_GT_U32_timer
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnFcTimerGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_index,
    &PX_OUT_GT_U32_PTR_timerPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortIpgSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_ipg
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesTuningSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_laneBmp,
    &PX_IN_CPSS_PORT_SERDES_SPEED_ENT_serdesFrequency,
    &PX_IN_CPSS_PORT_SERDES_TUNE_STC_PTR_tuneValuesPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesPolaritySet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_laneBmp,
    &PX_IN_GT_BOOL_invertTx,
    &PX_IN_GT_BOOL_invertRx
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesManualRxConfigSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_laneNum,
    &PX_IN_CPSS_PORT_SERDES_RX_CONFIG_STC_PTR_serdesRxCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesTuningGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_laneNum,
    &PX_IN_CPSS_PORT_SERDES_SPEED_ENT_serdesFrequency,
    &PX_OUT_CPSS_PORT_SERDES_TUNE_STC_PTR_tuneValuesPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesManualTxConfigSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_laneNum,
    &PX_IN_CPSS_PORT_SERDES_TX_CONFIG_STC_PTR_serdesTxCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSyncEtherRecoveryClkDividerValueSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_laneNum,
    &PX_IN_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT_clockSelect,
    &PX_IN_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT_value
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSyncEtherRecoveryClkDividerValueGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_laneNum,
    &PX_IN_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT_clockSelect,
    &PX_OUT_CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT_PTR_valuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesManualRxConfigGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_laneNum,
    &PX_OUT_CPSS_PORT_SERDES_RX_CONFIG_STC_PTR_serdesRxCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesAutoTuneResultsGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_laneNum,
    &PX_OUT_CPSS_PORT_SERDES_TUNE_STC_PTR_serdesTunePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesManualTxConfigGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_laneNum,
    &PX_OUT_CPSS_PORT_SERDES_TX_CONFIG_STC_PTR_serdesTxCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesLoopbackModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_laneNum,
    &PX_OUT_CPSS_PX_PORT_SERDES_LOOPBACK_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesPolarityGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_laneNum,
    &PX_OUT_GT_BOOL_PTR_invertTxPtr,
    &PX_OUT_GT_BOOL_PTR_invertRxPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortMruSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_mruSize
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcSourcePortToPfcCounterSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_pfcCounterNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcProfileIndexSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_profileIndex
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortXgPscLanesSwapSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_PTR_rxSerdesLaneArr,
    &PX_IN_GT_U32_PTR_txSerdesLaneArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesEyeMatrixGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_serdesNum,
    &PX_IN_CPSS_PX_PORT_SERDES_EYE_INPUT_STC_PTR_eye_inputPtr,
    &PX_OUT_CPSS_PX_PORT_SERDES_EYE_RESULT_STC_PTR_eye_resultsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortEomBaudRateGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_serdesNum,
    &PX_OUT_GT_U32_PTR_baudRatePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortEomDfeResGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_serdesNum,
    &PX_OUT_GT_U32_PTR_dfeResPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesAutoTuneOptAlgSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_serdesOptAlgBmp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesSquelchSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_squelch
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropTcBuffNumberGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_tc,
    &PX_OUT_GT_U32_PTR_numberPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxDebugQueueingEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_tcQueue,
    &PX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxShaperQueueEnableSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_tcQueue,
    &PX_IN_GT_BOOL_enable,
    &PX_IN_GT_BOOL_avbModeEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxShaperQueueProfileSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_tcQueue,
    &PX_IN_GT_U32_burstSize,
    &PX_INOUT_GT_U32_PTR_maxRatePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxDebugQueueingEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_tcQueue,
    &PX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxShaperQueueEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_tcQueue,
    &PX_OUT_GT_BOOL_PTR_enablePtr,
    &PX_OUT_GT_BOOL_PTR_avbModeEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxShaperQueueProfileGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_tcQueue,
    &PX_OUT_GT_U32_PTR_burstSizePtr,
    &PX_OUT_GT_U32_PTR_maxRatePtr,
    &PX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPeriodicFlowControlCounterSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_value
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortMacSaLsbSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U8_macSaLsb
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesEnhancedAutoTune_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U8_min_LF,
    &PX_IN_GT_U8_max_LF
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortExtraIpgSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U8_number
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnMessageTriggeringStateSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U8_tcQueue,
    &PX_IN_GT_U32_qcnSampleInterval,
    &PX_IN_GT_U32_qlenOld
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnMessageTriggeringStateGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U8_tcQueue,
    &PX_OUT_GT_U32_PTR_qcnSampleIntervalPtr,
    &PX_OUT_GT_U32_PTR_qlenOldPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortApDebugInfoGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_MV_HWS_AP_DEBUG_LOG_OUTPUT_ENT_output
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortManagerPortParamsGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PM_PORT_PARAMS_STC_PTR_portParamsStcPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortAttributesOnPortGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PORT_ATTRIBUTES_STC_PTR_portAttributSetArrayPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortDuplexModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PORT_DUPLEX_ENT_PTR_dModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortFecModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PORT_FEC_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortFlowControlEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PORT_FLOW_CONTROL_ENT_PTR_statePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortInterfaceModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PORT_INTERFACE_MODE_ENT_PTR_ifModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortMacCountersOnCgPortGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PORT_MAC_CG_COUNTER_SET_STC_PTR_cgMibStcPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortMacStatusGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PORT_MAC_STATUS_STC_PTR_portMacStatusPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortMacTypeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PORT_MAC_TYPE_ENT_PTR_portMacTypePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortManagerStatusGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PORT_MANAGER_STATUS_STC_PTR_portStagePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPeriodicFcEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortBufMgRxProfileGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PORT_RX_FC_PROFILE_SET_ENT_PTR_profileSetPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesAutoTuneStatusGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT_PTR_rxTuneStatusPtr,
    &PX_OUT_CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT_PTR_txTuneStatusPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSpeedGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PORT_SPEED_ENT_PTR_speedPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortXGmiiModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PORT_XGMII_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortIpgBaseGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PORT_XG_FIXED_IPG_ENT_PTR_ipgBasePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPhysicalPortDetailedMapGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_DETAILED_PORT_MAP_STC_PTR_portMapShadowPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnMessagePortMapEntryGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_PORTS_BMP_PTR_portsBmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortApIntropGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_PORT_AP_INTROP_STC_PTR_apIntropPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortApStatsGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_PORT_AP_STATS_STC_PTR_apStatsPtr,
    &PX_OUT_GT_U16_PTR_intropAbilityMaxIntervalPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortApPortStatusGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_PORT_AP_STATUS_STC_PTR_apStatusPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortAutoNegAdvertismentConfigGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_PORT_AUTONEG_ADVERTISMENT_STC_PTR_portAnAdvertismentPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortFlowControlModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_PORT_FC_MODE_ENT_PTR_fcModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortMacCountersOnPortGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_PORT_MAC_COUNTERS_STC_PTR_countersPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortMacCountersCaptureOnPortGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_PORT_MAC_COUNTERS_STC_PTR_portMacCounterSetArrayPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortMacOversizedPacketsCounterModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT_PTR_counterModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPcsLoopbackModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_PORT_PCS_LOOPBACK_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxSchedulerProfileIdGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT_PTR_profileSetPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxShaperModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_PORT_TX_SHAPER_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropProfileIdGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT_PTR_profileSetPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortApPortEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_BOOL_PTR_apEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortApPortConfigGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_BOOL_PTR_apEnablePtr,
    &PX_OUT_CPSS_PX_PORT_AP_PARAMS_STC_PTR_apParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortMacCountersCaptureTriggerGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_BOOL_PTR_captureIsDonePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesCDRLockStatusGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_BOOL_PTR_cdrLockPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxShaperEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_BOOL_PTR_enablePtr,
    &PX_OUT_GT_BOOL_PTR_avbModeEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPcsGearBoxStatusGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_BOOL_PTR_gbLockPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPhysicalPortMapIsCpuGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_BOOL_PTR_isCpuPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortLinkStatusGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_BOOL_PTR_isLinkUpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortXgmiiLocalFaultGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_BOOL_PTR_isLocalFaultPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortXgmiiRemoteFaultGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_BOOL_PTR_isRemoteFaultPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPhysicalPortMapIsValidGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_BOOL_PTR_isValidPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortRefClockSourceOverrideEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_BOOL_PTR_overrideEnablePtr,
    &PX_OUT_CPSS_PORT_REF_CLOCK_SOURCE_ENT_PTR_refClockSourcePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPcsSyncStableStatusGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_BOOL_PTR_signalStatePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortForceLinkPassEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_BOOL_PTR_statePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortFlowCntrlAutoNegEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_BOOL_PTR_statePtr,
    &PX_OUT_GT_BOOL_PTR_pauseAdvertisePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPcsSyncStatusGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_BOOL_PTR_syncPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxDebugDescNumberGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_U16_PTR_numberPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortBufMgTxDmaBurstLimitThresholdsGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_U32_PTR_almostFullThresholdPtr,
    &PX_OUT_GT_U32_PTR_fullThresholdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxSchedulerShaperByteCountChangeValueGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_U32_PTR_bcValuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxShaperProfileGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_U32_PTR_burstSizePtr,
    &PX_OUT_GT_U32_PTR_maxRatePtr,
    &PX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortIpgGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_U32_PTR_ipgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortMruGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_U32_PTR_mruSizePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortBufMgRxBufNumberGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_U32_PTR_numOfBuffersPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropDescNumberGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_U32_PTR_numberPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcSourcePortToPfcCounterGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_U32_PTR_pfcCounterNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPeriodicFlowControlCounterGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_U32_PTR_valuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortXgPscLanesSwapGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_U32_PTR_rxSerdesLaneArr,
    &PX_OUT_GT_U32_PTR_txSerdesLaneArr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortMacSaLsbGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_U8_PTR_macSaLsbPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortExtraIpgGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_U8_PTR_numberPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnAvailableBuffSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_availableBuff
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxShaperBaselineSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_baseline
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcDbaAvailableBuffersSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_buffsNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortBufMgRxMcCntrGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_cntrIdx,
    &PX_OUT_GT_U32_PTR_mcCntrPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxDebugResourceHistogramThresholdSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_cntrNum,
    &PX_IN_GT_U32_threshold
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxDebugResourceHistogramCounterGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_cntrNum,
    &PX_OUT_GT_U32_PTR_cntrPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxDebugResourceHistogramThresholdGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_cntrNum,
    &PX_OUT_GT_U32_PTR_thresholdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortEgressCntrModeSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_cntrSetNum,
    &PX_IN_CPSS_PORT_EGRESS_CNT_MODE_ENT_setModeBmp,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U32_tc,
    &PX_IN_CPSS_DP_LEVEL_ENT_dpLevel
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortEgressCntrModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_cntrSetNum,
    &PX_OUT_CPSS_PORT_EGRESS_CNT_MODE_ENT_PTR_setModeBmpPtr,
    &PX_OUT_GT_PHYSICAL_PORT_NUM_PTR_portNumPtr,
    &PX_OUT_GT_U32_PTR_tcPtr,
    &PX_OUT_CPSS_DP_LEVEL_ENT_PTR_dpLevelPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortEgressCntrsGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_cntrSetNum,
    &PX_OUT_CPSS_PX_PORT_EGRESS_CNTR_STC_PTR_egrCntrPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCpllCfgInit_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_cpllNum,
    &PX_IN_CPSS_PX_PORT_CPLL_INPUT_FREQUENCY_ENT_inputFreq,
    &PX_IN_CPSS_PX_PORT_CPLL_OUTPUT_FREQUENCY_ENT_outputFreq
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropPacketModeLengthSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_length
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxDebugGlobalDescLimitSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_limit
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropSharedBuffMaxLimitSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_maxSharedBufferLimit
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropMcastBuffersLimitSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_mcastMaxBuffNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropMcastPcktDescLimitSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_mcastMaxDescNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcTimerToQueueMapSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_pfcTimer,
    &PX_IN_GT_U32_tcQueue
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcTimerToQueueMapGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_pfcTimer,
    &PX_OUT_GT_U32_PTR_tcQueuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_pipeBandwithInGbps,
    &PX_IN_CPSS_PX_MIN_SPEED_ENT_minimalPortSpeedMBps
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropSharedPoolLimitsSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_poolNum,
    &PX_IN_GT_U32_maxBuffNum,
    &PX_IN_GT_U32_maxDescNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropSharedPoolLimitsGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_poolNum,
    &PX_OUT_GT_U32_PTR_maxBuffNumPtr,
    &PX_OUT_GT_U32_PTR_maxDescNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesLaneSignalDetectGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_portGroupId,
    &PX_IN_GT_U32_laneNum,
    &PX_OUT_GT_BOOL_PTR_signalStatePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesLaneTuningSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_portGroupNum,
    &PX_IN_GT_U32_laneNum,
    &PX_IN_CPSS_PORT_SERDES_SPEED_ENT_serdesFrequency,
    &PX_IN_CPSS_PORT_SERDES_TUNE_STC_PTR_tuneValuesPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesLaneTuningGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_portGroupNum,
    &PX_IN_GT_U32_laneNum,
    &PX_IN_CPSS_PORT_SERDES_SPEED_ENT_serdesFrequency,
    &PX_OUT_CPSS_PORT_SERDES_TUNE_STC_PTR_tuneValuesPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesLanePolaritySet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_portGroupNum,
    &PX_IN_GT_U32_laneNum,
    &PX_IN_GT_BOOL_invertTx,
    &PX_IN_GT_BOOL_invertRx
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesLanePolarityGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_portGroupNum,
    &PX_IN_GT_U32_laneNum,
    &PX_OUT_GT_BOOL_PTR_invertTx,
    &PX_OUT_GT_BOOL_PTR_invertRx
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPhysicalPortMapSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_portMapArraySize,
    &PX_IN_CPSS_PX_PORT_MAP_STC_PTR_portMapArrayPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcProfileQueueConfigSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_profileIndex,
    &PX_IN_GT_U32_tcQueue,
    &PX_IN_CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC_PTR_pfcProfileCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcProfileQueueConfigGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_profileIndex,
    &PX_IN_GT_U32_tcQueue,
    &PX_OUT_CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC_PTR_pfcProfileCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxDebugQueueDumpAll_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_tailDropDumpBmp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcCounterGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_tcQueue,
    &PX_IN_GT_U32_pfcCounterNum,
    &PX_OUT_GT_U32_PTR_pfcCounterValuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcGlobalQueueConfigSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_tcQueue,
    &PX_IN_GT_U32_xoffThreshold,
    &PX_IN_GT_U32_dropThreshold,
    &PX_IN_GT_U32_xonThreshold
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcQueueCounterGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_tcQueue,
    &PX_OUT_GT_U32_PTR_cntPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropSharedResourceDescNumberGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_tcQueue,
    &PX_OUT_GT_U32_PTR_numberPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcGlobalQueueConfigGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_tcQueue,
    &PX_OUT_GT_U32_PTR_xoffThresholdPtr,
    &PX_OUT_GT_U32_PTR_dropThresholdPtr,
    &PX_OUT_GT_U32_PTR_xonThresholdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortBufMgGlobalXoffLimitSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_xoffLimit
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortBufMgGlobalXonLimitSet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U32_xonLimit
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnSampleEntrySet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U8_entryIndex,
    &PX_IN_CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnSampleEntryGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_IN_GT_U8_entryIndex,
    &PX_OUT_CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPizzaArbiterDevStateGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC_PTR_pizzaDeviceStatePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcPacketTypeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_PACKET_TYPE_PTR_packetTypePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnMessageGenerationConfigGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC_PTR_cnmGenerationCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnFbCalcConfigGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC_PTR_fbCalcCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnMessageTypeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT_PTR_mTypePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnPacketLengthGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_PORT_CN_PACKET_LENGTH_ENT_PTR_packetLengthPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcCountingModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_PORT_PFC_COUNT_MODE_ENT_PTR_pfcCountModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_PORT_PFC_ENABLE_ENT_PTR_pfcEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortStatTxDebugCountersGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_PORT_STAT_TX_DROP_COUNTERS_STC_PTR_dropCntrStcPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxSchedulerWrrMtuGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT_PTR_wrrMtuPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxShaperConfigurationGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_PORT_TX_SHAPER_CONFIG_STC_PTR_configsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropSharedPolicyGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_SHARED_POLICY_ENT_PTR_policyPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropWrtdMasksGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_CPSS_PX_PORT_TX_TAIL_DROP_WRTD_MASK_LSB_STC_PTR_maskLsbPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnQueueStatusModeEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_BOOL_PTR_enablePtr,
    &PX_OUT_GT_PORT_NUM_PTR_targetPortPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortApEnableGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_BOOL_PTR_enabledPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortApPortEnableCtrlGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_BOOL_PTR_srvCpuEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortVosOverrideControlModeGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_BOOL_PTR_vosOverridePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortMacSaBaseGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_ETHERADDR_PTR_macPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCnAvailableBuffGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_availableBuffPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxShaperBaselineGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_baselinePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcDbaAvailableBuffersGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_buffsNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropPacketModeLengthGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_lengthPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxDebugGlobalDescLimitGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_limitPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropSharedBuffMaxLimitGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_maxSharedBufferLimitPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropMcastBuffersLimitGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_mcastMaxBuffNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropMcastPcktDescLimitGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_mcastMaxDescNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortBufMgGlobalRxBufNumberGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_numOfBuffersPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortBufMgGlobalPacketNumberGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_numOfPacketsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortTxTailDropMcastDescNumberGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_numberPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_pipeBandwithInGbpsPtr,
    &PX_OUT_CPSS_PX_MIN_SPEED_ENT_PTR_minimalPortSpeedResolutionInMBpsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortBufMgGlobalXoffLimitGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_xoffLimitPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortBufMgGlobalXonLimitGet_PARAMS[] =  {
    &PX_IN_GT_SW_DEV_NUM_devNum,
    &PX_OUT_GT_U32_PTR_xonLimitPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortManagerGlobalParamsOverride_PARAMS[] =  {
    &PX_IN_GT_U8_devNum,
    &PX_IN_CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC_PTR_globalParamsStcPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortManagerStatClear_PARAMS[] =  {
    &PX_IN_GT_U8_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortManagerPortParamsUpdate_PARAMS[] =  {
    &PX_IN_GT_U8_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_CPSS_PORT_MANAGER_UPDATE_PARAMS_STC_PTR_updateParamsStcPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortCtleBiasOverrideEnableSet_PARAMS[] =  {
    &PX_IN_GT_U8_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_BOOL_overrideEnable,
    &PX_IN_CPSS_PX_PORT_CTLE_BIAS_MODE_ENT_ctleBiasValue
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortApSerdesRxParametersManualSet_PARAMS[] =  {
    &PX_IN_GT_U8_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U8_serdesLane,
    &PX_IN_CPSS_PORT_AP_SERDES_RX_CONFIG_STC_PTR_rxOverrideParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortApSerdesTxParametersOffsetSet_PARAMS[] =  {
    &PX_IN_GT_U8_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_IN_GT_U8_serdesLane,
    &PX_IN_CPSS_PORT_AP_SERDES_TX_OFFSETS_STC_PTR_serdesTxOffsetsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxFcFecCounterGet_PARAMS[] =  {
    &PX_IN_GT_U8_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_FCFEC_COUNTERS_STC_PTR_fcfecCountersPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortManagerStatGet_PARAMS[] =  {
    &PX_IN_GT_U8_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_PORT_MANAGER_STATISTICS_STC_PTR_portStatStcPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxRsFecCounterGet_PARAMS[] =  {
    &PX_IN_GT_U8_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_CPSS_RSFEC_COUNTERS_STC_PTR_rsfecCountersPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortPfcProfileIndexGet_PARAMS[] =  {
    &PX_IN_GT_U8_devNum,
    &PX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &PX_OUT_GT_U32_PTR_profileIndexPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortSerdesErrorInject_PARAMS[] =  {
    &PX_IN_GT_U8_devNum,
    &PX_IN_GT_U32_portGroup,
    &PX_IN_GT_U32_serdesNum,
    &PX_IN_GT_U32_numOfBits,
    &PX_IN_CPSS_PORT_DIRECTION_ENT_direction
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPxPortManagerEnableGet_PARAMS[] =  {
    &PX_IN_GT_U8_devNum,
    &PX_OUT_GT_BOOL_PTR_enablePtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssPxPortLogLibDb[] = {
    {"cpssPxPortApEnableSet", 2, prvCpssLogGenDevNumEnable2_PARAMS, NULL},
    {"cpssPxPortApEnableGet", 2, cpssPxPortApEnableGet_PARAMS, NULL},
    {"cpssPxPortApPortConfigSet", 4, cpssPxPortApPortConfigSet_PARAMS, NULL},
    {"cpssPxPortApPortConfigGet", 4, cpssPxPortApPortConfigGet_PARAMS, NULL},
    {"cpssPxPortApPortEnableGet", 3, cpssPxPortApPortEnableGet_PARAMS, NULL},
    {"cpssPxPortApPortStatusGet", 3, cpssPxPortApPortStatusGet_PARAMS, NULL},
    {"cpssPxPortApStatsGet", 4, cpssPxPortApStatsGet_PARAMS, NULL},
    {"cpssPxPortApStatsReset", 2, prvCpssLogGenDevNumPortNum2_PARAMS, NULL},
    {"cpssPxPortApIntropSet", 3, cpssPxPortApIntropSet_PARAMS, NULL},
    {"cpssPxPortApIntropGet", 3, cpssPxPortApIntropGet_PARAMS, NULL},
    {"cpssPxPortApDebugInfoGet", 3, cpssPxPortApDebugInfoGet_PARAMS, NULL},
    {"cpssPxPortApPortEnableCtrlSet", 2, cpssPxPortApPortEnableCtrlSet_PARAMS, NULL},
    {"cpssPxPortApPortEnableCtrlGet", 2, cpssPxPortApPortEnableCtrlGet_PARAMS, NULL},
    {"cpssPxPortApSerdesRxParametersManualSet", 4, cpssPxPortApSerdesRxParametersManualSet_PARAMS, NULL},
    {"cpssPxPortApSerdesTxParametersOffsetSet", 4, cpssPxPortApSerdesTxParametersOffsetSet_PARAMS, NULL},
    {"cpssPxPortAutoNegAdvertismentConfigGet", 3, cpssPxPortAutoNegAdvertismentConfigGet_PARAMS, NULL},
    {"cpssPxPortAutoNegAdvertismentConfigSet", 3, cpssPxPortAutoNegAdvertismentConfigSet_PARAMS, NULL},
    {"cpssPxPortAutoNegMasterModeEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxPortAutoNegMasterModeEnableSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxPortInbandAutoNegRestart", 2, prvCpssLogGenDevNumPortNum2_PARAMS, NULL},
    {"cpssPxPortBufMgGlobalXonLimitSet", 2, cpssPxPortBufMgGlobalXonLimitSet_PARAMS, NULL},
    {"cpssPxPortBufMgGlobalXonLimitGet", 2, cpssPxPortBufMgGlobalXonLimitGet_PARAMS, NULL},
    {"cpssPxPortBufMgGlobalXoffLimitSet", 2, cpssPxPortBufMgGlobalXoffLimitSet_PARAMS, NULL},
    {"cpssPxPortBufMgGlobalXoffLimitGet", 2, cpssPxPortBufMgGlobalXoffLimitGet_PARAMS, NULL},
    {"cpssPxPortBufMgRxProfileSet", 3, cpssPxPortBufMgRxProfileSet_PARAMS, NULL},
    {"cpssPxPortBufMgRxProfileGet", 3, cpssPxPortBufMgRxProfileGet_PARAMS, NULL},
    {"cpssPxPortBufMgProfileXonLimitSet", 3, cpssPxPortBufMgProfileXonLimitSet_PARAMS, NULL},
    {"cpssPxPortBufMgProfileXonLimitGet", 3, cpssPxPortBufMgProfileXonLimitGet_PARAMS, NULL},
    {"cpssPxPortBufMgProfileXoffLimitSet", 3, cpssPxPortBufMgProfileXoffLimitSet_PARAMS, NULL},
    {"cpssPxPortBufMgProfileXoffLimitGet", 3, cpssPxPortBufMgProfileXoffLimitGet_PARAMS, NULL},
    {"cpssPxPortBufMgProfileRxBufLimitSet", 3, cpssPxPortBufMgProfileRxBufLimitSet_PARAMS, NULL},
    {"cpssPxPortBufMgProfileRxBufLimitGet", 3, cpssPxPortBufMgProfileRxBufLimitGet_PARAMS, NULL},
    {"cpssPxPortBufMgGlobalRxBufNumberGet", 2, cpssPxPortBufMgGlobalRxBufNumberGet_PARAMS, NULL},
    {"cpssPxPortBufMgRxBufNumberGet", 3, cpssPxPortBufMgRxBufNumberGet_PARAMS, NULL},
    {"cpssPxPortBufMgGlobalPacketNumberGet", 2, cpssPxPortBufMgGlobalPacketNumberGet_PARAMS, NULL},
    {"cpssPxPortBufMgRxMcCntrGet", 3, cpssPxPortBufMgRxMcCntrGet_PARAMS, NULL},
    {"cpssPxPortBufMgTxDmaBurstLimitEnableSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxPortBufMgTxDmaBurstLimitEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxPortBufMgTxDmaBurstLimitThresholdsSet", 4, cpssPxPortBufMgTxDmaBurstLimitThresholdsSet_PARAMS, NULL},
    {"cpssPxPortBufMgTxDmaBurstLimitThresholdsGet", 4, cpssPxPortBufMgTxDmaBurstLimitThresholdsGet_PARAMS, NULL},
    {"cpssPxPortCnModeEnableSet", 2, prvCpssLogGenDevNumEnable2_PARAMS, NULL},
    {"cpssPxPortCnModeEnableGet", 2, prvCpssLogGenDevNumEnablePtr2_PARAMS, NULL},
    {"cpssPxPortCnPacketLengthSet", 2, cpssPxPortCnPacketLengthSet_PARAMS, NULL},
    {"cpssPxPortCnPacketLengthGet", 2, cpssPxPortCnPacketLengthGet_PARAMS, NULL},
    {"cpssPxPortCnSampleEntrySet", 3, cpssPxPortCnSampleEntrySet_PARAMS, NULL},
    {"cpssPxPortCnSampleEntryGet", 3, cpssPxPortCnSampleEntryGet_PARAMS, NULL},
    {"cpssPxPortCnFbCalcConfigSet", 2, cpssPxPortCnFbCalcConfigSet_PARAMS, NULL},
    {"cpssPxPortCnFbCalcConfigGet", 2, cpssPxPortCnFbCalcConfigGet_PARAMS, NULL},
    {"cpssPxPortCnMessageTriggeringStateSet", 5, cpssPxPortCnMessageTriggeringStateSet_PARAMS, NULL},
    {"cpssPxPortCnMessageTriggeringStateGet", 5, cpssPxPortCnMessageTriggeringStateGet_PARAMS, NULL},
    {"cpssPxPortCnMessageGenerationConfigSet", 2, cpssPxPortCnMessageGenerationConfigSet_PARAMS, NULL},
    {"cpssPxPortCnMessageGenerationConfigGet", 2, cpssPxPortCnMessageGenerationConfigGet_PARAMS, NULL},
    {"cpssPxPortCnMessagePortMapEntrySet", 3, cpssPxPortCnMessagePortMapEntrySet_PARAMS, NULL},
    {"cpssPxPortCnMessagePortMapEntryGet", 3, cpssPxPortCnMessagePortMapEntryGet_PARAMS, NULL},
    {"cpssPxPortCnTerminationEnableSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxPortCnTerminationEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxPortCnFcEnableSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxPortCnFcEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxPortCnFcTimerSet", 4, cpssPxPortCnFcTimerSet_PARAMS, NULL},
    {"cpssPxPortCnFcTimerGet", 4, cpssPxPortCnFcTimerGet_PARAMS, NULL},
    {"cpssPxPortCnQueueStatusModeEnableSet", 3, cpssPxPortCnQueueStatusModeEnableSet_PARAMS, NULL},
    {"cpssPxPortCnQueueStatusModeEnableGet", 3, cpssPxPortCnQueueStatusModeEnableGet_PARAMS, NULL},
    {"cpssPxPortCnProfileQueueConfigSet", 4, cpssPxPortCnProfileQueueConfigSet_PARAMS, NULL},
    {"cpssPxPortCnProfileQueueConfigGet", 4, cpssPxPortCnProfileQueueConfigGet_PARAMS, NULL},
    {"cpssPxPortCnAvailableBuffSet", 2, cpssPxPortCnAvailableBuffSet_PARAMS, NULL},
    {"cpssPxPortCnAvailableBuffGet", 2, cpssPxPortCnAvailableBuffGet_PARAMS, NULL},
    {"cpssPxPortCnMessageTypeSet", 2, cpssPxPortCnMessageTypeSet_PARAMS, NULL},
    {"cpssPxPortCnMessageTypeGet", 2, cpssPxPortCnMessageTypeGet_PARAMS, NULL},
    {"cpssPxPortForceLinkPassEnableSet", 3, cpssPxPortForceLinkPassEnableSet_PARAMS, NULL},
    {"cpssPxPortForceLinkPassEnableGet", 3, cpssPxPortForceLinkPassEnableGet_PARAMS, NULL},
    {"cpssPxPortForceLinkDownEnableSet", 3, cpssPxPortForceLinkPassEnableSet_PARAMS, NULL},
    {"cpssPxPortForceLinkDownEnableGet", 3, cpssPxPortForceLinkPassEnableGet_PARAMS, NULL},
    {"cpssPxPortMruSet", 3, cpssPxPortMruSet_PARAMS, NULL},
    {"cpssPxPortMruGet", 3, cpssPxPortMruGet_PARAMS, NULL},
    {"cpssPxPortLinkStatusGet", 3, cpssPxPortLinkStatusGet_PARAMS, NULL},
    {"cpssPxPortInternalLoopbackEnableSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxPortInternalLoopbackEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxPortEnableSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxPortEnableGet", 3, cpssPxPortForceLinkPassEnableGet_PARAMS, NULL},
    {"cpssPxPortCpllCfgInit", 4, cpssPxPortCpllCfgInit_PARAMS, NULL},
    {"cpssPxPortFaultSendSet", 5, cpssPxPortFaultSendSet_PARAMS, NULL},
    {"cpssPxPortDuplexAutoNegEnableSet", 3, cpssPxPortForceLinkPassEnableSet_PARAMS, NULL},
    {"cpssPxPortDuplexAutoNegEnableGet", 3, cpssPxPortForceLinkPassEnableGet_PARAMS, NULL},
    {"cpssPxPortFlowCntrlAutoNegEnableSet", 4, cpssPxPortFlowCntrlAutoNegEnableSet_PARAMS, NULL},
    {"cpssPxPortFlowCntrlAutoNegEnableGet", 4, cpssPxPortFlowCntrlAutoNegEnableGet_PARAMS, NULL},
    {"cpssPxPortFlowControlEnableSet", 3, cpssPxPortFlowControlEnableSet_PARAMS, NULL},
    {"cpssPxPortFlowControlEnableGet", 3, cpssPxPortFlowControlEnableGet_PARAMS, NULL},
    {"cpssPxPortFlowControlModeSet", 3, cpssPxPortFlowControlModeSet_PARAMS, NULL},
    {"cpssPxPortFlowControlModeGet", 3, cpssPxPortFlowControlModeGet_PARAMS, NULL},
    {"cpssPxPortDuplexModeSet", 3, cpssPxPortDuplexModeSet_PARAMS, NULL},
    {"cpssPxPortDuplexModeGet", 3, cpssPxPortDuplexModeGet_PARAMS, NULL},
    {"cpssPxPortAttributesOnPortGet", 3, cpssPxPortAttributesOnPortGet_PARAMS, NULL},
    {"cpssPxPortSpeedAutoNegEnableSet", 3, cpssPxPortForceLinkPassEnableSet_PARAMS, NULL},
    {"cpssPxPortSpeedAutoNegEnableGet", 3, cpssPxPortForceLinkPassEnableGet_PARAMS, NULL},
    {"cpssPxPortBackPressureEnableSet", 3, cpssPxPortForceLinkPassEnableSet_PARAMS, NULL},
    {"cpssPxPortBackPressureEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxPortCrcCheckEnableSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxPortCrcCheckEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxPortExcessiveCollisionDropEnableSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxPortExcessiveCollisionDropEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxPortInBandAutoNegBypassEnableSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxPortInBandAutoNegBypassEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxPortMacResetStateSet", 3, cpssPxPortForceLinkPassEnableSet_PARAMS, NULL},
    {"cpssPxPortForward802_3xEnableSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxPortForward802_3xEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxPortForwardUnknownMacControlFramesEnableSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxPortForwardUnknownMacControlFramesEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxPortInbandAutoNegEnableSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxPortInbandAutoNegEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxPortMacTypeGet", 3, cpssPxPortMacTypeGet_PARAMS, NULL},
    {"cpssPxPortIpgSet", 3, cpssPxPortIpgSet_PARAMS, NULL},
    {"cpssPxPortIpgGet", 3, cpssPxPortIpgGet_PARAMS, NULL},
    {"cpssPxPortIpgBaseSet", 3, cpssPxPortIpgBaseSet_PARAMS, NULL},
    {"cpssPxPortIpgBaseGet", 3, cpssPxPortIpgBaseGet_PARAMS, NULL},
    {"cpssPxPortPreambleLengthSet", 4, cpssPxPortPreambleLengthSet_PARAMS, NULL},
    {"cpssPxPortPreambleLengthGet", 4, cpssPxPortPreambleLengthGet_PARAMS, NULL},
    {"cpssPxPortPeriodicFcEnableSet", 3, cpssPxPortPeriodicFcEnableSet_PARAMS, NULL},
    {"cpssPxPortPeriodicFcEnableGet", 3, cpssPxPortPeriodicFcEnableGet_PARAMS, NULL},
    {"cpssPxPortPeriodicFlowControlCounterSet", 3, cpssPxPortPeriodicFlowControlCounterSet_PARAMS, NULL},
    {"cpssPxPortPeriodicFlowControlCounterGet", 3, cpssPxPortPeriodicFlowControlCounterGet_PARAMS, NULL},
    {"cpssPxPortMacSaLsbSet", 3, cpssPxPortMacSaLsbSet_PARAMS, NULL},
    {"cpssPxPortMacSaLsbGet", 3, cpssPxPortMacSaLsbGet_PARAMS, NULL},
    {"cpssPxPortMacSaBaseSet", 2, cpssPxPortMacSaBaseSet_PARAMS, NULL},
    {"cpssPxPortMacSaBaseGet", 2, cpssPxPortMacSaBaseGet_PARAMS, NULL},
    {"cpssPxPortXGmiiModeSet", 3, cpssPxPortXGmiiModeSet_PARAMS, NULL},
    {"cpssPxPortXGmiiModeGet", 3, cpssPxPortXGmiiModeGet_PARAMS, NULL},
    {"cpssPxPortExtraIpgSet", 3, cpssPxPortExtraIpgSet_PARAMS, NULL},
    {"cpssPxPortExtraIpgGet", 3, cpssPxPortExtraIpgGet_PARAMS, NULL},
    {"cpssPxPortXgmiiLocalFaultGet", 3, cpssPxPortXgmiiLocalFaultGet_PARAMS, NULL},
    {"cpssPxPortXgmiiRemoteFaultGet", 3, cpssPxPortXgmiiRemoteFaultGet_PARAMS, NULL},
    {"cpssPxPortMacStatusGet", 3, cpssPxPortMacStatusGet_PARAMS, NULL},
    {"cpssPxPortPaddingEnableSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxPortPaddingEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxPortCtleBiasOverrideEnableSet", 4, cpssPxPortCtleBiasOverrideEnableSet_PARAMS, NULL},
    {"cpssPxPortCtleBiasOverrideEnableGet", 4, cpssPxPortCtleBiasOverrideEnableGet_PARAMS, NULL},
    {"cpssPxPortVosOverrideControlModeSet", 2, cpssPxPortVosOverrideControlModeSet_PARAMS, NULL},
    {"cpssPxPortVosOverrideControlModeGet", 2, cpssPxPortVosOverrideControlModeGet_PARAMS, NULL},
    {"cpssPxPortXlgReduceAverageIPGSet", 3, cpssPxPortPeriodicFlowControlCounterSet_PARAMS, NULL},
    {"cpssPxPortXlgReduceAverageIPGGet", 3, cpssPxPortPeriodicFlowControlCounterGet_PARAMS, NULL},
    {"cpssPxPortCrcNumBytesSet", 4, cpssPxPortCrcNumBytesSet_PARAMS, NULL},
    {"cpssPxPortCrcNumBytesGet", 4, cpssPxPortCrcNumBytesGet_PARAMS, NULL},
    {"cpssPxFcFecCounterGet", 3, cpssPxFcFecCounterGet_PARAMS, NULL},
    {"cpssPxRsFecCounterGet", 3, cpssPxRsFecCounterGet_PARAMS, NULL},
    {"cpssPxPortEcnMarkingEnableSet", 3, cpssPxPortEcnMarkingEnableSet_PARAMS, NULL},
    {"cpssPxPortEcnMarkingEnableGet", 3, cpssPxPortEcnMarkingEnableGet_PARAMS, NULL},
    {"cpssPxPortEcnMarkingTailDropProfileEnableSet", 3, cpssPxPortEcnMarkingTailDropProfileEnableSet_PARAMS, NULL},
    {"cpssPxPortEcnMarkingTailDropProfileEnableGet", 3, cpssPxPortEcnMarkingTailDropProfileEnableGet_PARAMS, NULL},
    {"cpssPxPortInterfaceModeGet", 3, cpssPxPortInterfaceModeGet_PARAMS, NULL},
    {"cpssPxPortInterfaceSpeedSupportGet", 5, cpssPxPortInterfaceSpeedSupportGet_PARAMS, NULL},
    {"cpssPxPortFecModeSet", 3, cpssPxPortFecModeSet_PARAMS, NULL},
    {"cpssPxPortFecModeGet", 3, cpssPxPortFecModeGet_PARAMS, NULL},
    {"cpssPxPortRefClockSourceOverrideEnableSet", 4, cpssPxPortRefClockSourceOverrideEnableSet_PARAMS, NULL},
    {"cpssPxPortRefClockSourceOverrideEnableGet", 4, cpssPxPortRefClockSourceOverrideEnableGet_PARAMS, NULL},
    {"cpssPxPortModeSpeedSet", 5, cpssPxPortModeSpeedSet_PARAMS, NULL},
    {"cpssPxPortManagerEventSet", 3, cpssPxPortManagerEventSet_PARAMS, NULL},
    {"cpssPxPortManagerStatusGet", 3, cpssPxPortManagerStatusGet_PARAMS, NULL},
    {"cpssPxPortManagerPortParamsSet", 3, cpssPxPortManagerPortParamsSet_PARAMS, NULL},
    {"cpssPxPortManagerPortParamsGet", 3, cpssPxPortManagerPortParamsGet_PARAMS, NULL},
    {"cpssPxPortManagerPortParamsStructInit", 2, cpssPxPortManagerPortParamsStructInit_PARAMS, NULL},
    {"cpssPxPortManagerGlobalParamsOverride", 2, cpssPxPortManagerGlobalParamsOverride_PARAMS, NULL},
    {"cpssPxPortManagerPortParamsUpdate", 3, cpssPxPortManagerPortParamsUpdate_PARAMS, NULL},
    {"cpssPxPortManagerInit", 1, prvCpssLogGenDevNum2_PARAMS, NULL},
    {"cpssPxPortManagerEnableGet", 2, cpssPxPortManagerEnableGet_PARAMS, NULL},
    {"cpssPxPortManagerStatGet", 3, cpssPxPortManagerStatGet_PARAMS, NULL},
    {"cpssPxPortManagerStatClear", 2, cpssPxPortManagerStatClear_PARAMS, NULL},
    {"cpssPxPortPhysicalPortMapSet", 3, cpssPxPortPhysicalPortMapSet_PARAMS, NULL},
    {"cpssPxPortPhysicalPortMapGet", 4, cpssPxPortPhysicalPortMapGet_PARAMS, NULL},
    {"cpssPxPortPhysicalPortMapIsValidGet", 3, cpssPxPortPhysicalPortMapIsValidGet_PARAMS, NULL},
    {"cpssPxPortPhysicalPortMapReverseMappingGet", 4, cpssPxPortPhysicalPortMapReverseMappingGet_PARAMS, NULL},
    {"cpssPxPortPhysicalPortDetailedMapGet", 3, cpssPxPortPhysicalPortDetailedMapGet_PARAMS, NULL},
    {"cpssPxPortPhysicalPortMapIsCpuGet", 3, cpssPxPortPhysicalPortMapIsCpuGet_PARAMS, NULL},
    {"cpssPxPortPcsLoopbackModeSet", 3, cpssPxPortPcsLoopbackModeSet_PARAMS, NULL},
    {"cpssPxPortPcsLoopbackModeGet", 3, cpssPxPortPcsLoopbackModeGet_PARAMS, NULL},
    {"cpssPxPortPcsResetSet", 4, cpssPxPortPcsResetSet_PARAMS, NULL},
    {"cpssPxPortPcsGearBoxStatusGet", 3, cpssPxPortPcsGearBoxStatusGet_PARAMS, NULL},
    {"cpssPxPortPcsSyncStatusGet", 3, cpssPxPortPcsSyncStatusGet_PARAMS, NULL},
    {"cpssPxPortPcsSyncStableStatusGet", 3, cpssPxPortPcsSyncStableStatusGet_PARAMS, NULL},
    {"cpssPxPortXgLanesSwapEnableSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxPortXgLanesSwapEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxPortXgPscLanesSwapSet", 4, cpssPxPortXgPscLanesSwapSet_PARAMS, NULL},
    {"cpssPxPortXgPscLanesSwapGet", 4, cpssPxPortXgPscLanesSwapGet_PARAMS, NULL},
    {"cpssPxPortPfcEnableSet", 2, cpssPxPortPfcEnableSet_PARAMS, NULL},
    {"cpssPxPortPfcEnableGet", 2, cpssPxPortPfcEnableGet_PARAMS, NULL},
    {"cpssPxPortPfcProfileIndexSet", 3, cpssPxPortPfcProfileIndexSet_PARAMS, NULL},
    {"cpssPxPortPfcProfileIndexGet", 3, cpssPxPortPfcProfileIndexGet_PARAMS, NULL},
    {"cpssPxPortPfcDbaAvailableBuffersSet", 2, cpssPxPortPfcDbaAvailableBuffersSet_PARAMS, NULL},
    {"cpssPxPortPfcDbaAvailableBuffersGet", 2, cpssPxPortPfcDbaAvailableBuffersGet_PARAMS, NULL},
    {"cpssPxPortPfcProfileQueueConfigSet", 4, cpssPxPortPfcProfileQueueConfigSet_PARAMS, NULL},
    {"cpssPxPortPfcProfileQueueConfigGet", 4, cpssPxPortPfcProfileQueueConfigGet_PARAMS, NULL},
    {"cpssPxPortPfcCountingModeSet", 2, cpssPxPortPfcCountingModeSet_PARAMS, NULL},
    {"cpssPxPortPfcCountingModeGet", 2, cpssPxPortPfcCountingModeGet_PARAMS, NULL},
    {"cpssPxPortPfcGlobalDropEnableSet", 2, prvCpssLogGenDevNumEnable2_PARAMS, NULL},
    {"cpssPxPortPfcGlobalDropEnableGet", 2, prvCpssLogGenDevNumEnablePtr2_PARAMS, NULL},
    {"cpssPxPortPfcGlobalQueueConfigSet", 5, cpssPxPortPfcGlobalQueueConfigSet_PARAMS, NULL},
    {"cpssPxPortPfcGlobalQueueConfigGet", 5, cpssPxPortPfcGlobalQueueConfigGet_PARAMS, NULL},
    {"cpssPxPortPfcSourcePortToPfcCounterSet", 3, cpssPxPortPfcSourcePortToPfcCounterSet_PARAMS, NULL},
    {"cpssPxPortPfcSourcePortToPfcCounterGet", 3, cpssPxPortPfcSourcePortToPfcCounterGet_PARAMS, NULL},
    {"cpssPxPortPfcCounterGet", 4, cpssPxPortPfcCounterGet_PARAMS, NULL},
    {"cpssPxPortPfcTimerMapEnableSet", 3, cpssPxPortPfcTimerMapEnableSet_PARAMS, NULL},
    {"cpssPxPortPfcTimerMapEnableGet", 3, cpssPxPortPfcTimerMapEnableGet_PARAMS, NULL},
    {"cpssPxPortPfcTimerToQueueMapSet", 3, cpssPxPortPfcTimerToQueueMapSet_PARAMS, NULL},
    {"cpssPxPortPfcTimerToQueueMapGet", 3, cpssPxPortPfcTimerToQueueMapGet_PARAMS, NULL},
    {"cpssPxPortPfcShaperToPortRateRatioSet", 4, cpssPxPortPfcShaperToPortRateRatioSet_PARAMS, NULL},
    {"cpssPxPortPfcShaperToPortRateRatioGet", 4, cpssPxPortPfcShaperToPortRateRatioGet_PARAMS, NULL},
    {"cpssPxPortPfcForwardEnableSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxPortPfcForwardEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxPortPfcPacketClassificationEnableSet", 4, cpssPxPortPfcPacketClassificationEnableSet_PARAMS, NULL},
    {"cpssPxPortPfcPacketTypeGet", 2, cpssPxPortPfcPacketTypeGet_PARAMS, NULL},
    {"cpssPxPortPfcQueueCounterGet", 3, cpssPxPortPfcQueueCounterGet_PARAMS, NULL},
    {"cpssPxPortPfcXonMessageFilterEnableSet", 2, prvCpssLogGenDevNumEnable2_PARAMS, NULL},
    {"cpssPxPortPfcXonMessageFilterEnableGet", 2, prvCpssLogGenDevNumEnablePtr2_PARAMS, NULL},
    {"cpssPxPortSerdesPowerStatusSet", 3, cpssPxPortSerdesPowerStatusSet_PARAMS, NULL},
    {"cpssPxPortSerdesTuningSet", 5, cpssPxPortSerdesTuningSet_PARAMS, NULL},
    {"cpssPxPortSerdesTuningGet", 5, cpssPxPortSerdesTuningGet_PARAMS, NULL},
    {"cpssPxPortSerdesLaneTuningSet", 5, cpssPxPortSerdesLaneTuningSet_PARAMS, NULL},
    {"cpssPxPortSerdesLaneTuningGet", 5, cpssPxPortSerdesLaneTuningGet_PARAMS, NULL},
    {"cpssPxPortSerdesResetStateSet", 3, cpssPxPortForceLinkPassEnableSet_PARAMS, NULL},
    {"cpssPxPortSerdesAutoTune", 3, cpssPxPortSerdesAutoTune_PARAMS, NULL},
    {"cpssPxPortSerdesAutoTuneExt", 4, cpssPxPortSerdesAutoTuneExt_PARAMS, NULL},
    {"cpssPxPortSerdesErrorInject", 5, cpssPxPortSerdesErrorInject_PARAMS, NULL},
    {"cpssPxPortSerdesAutoTuneStatusGet", 4, cpssPxPortSerdesAutoTuneStatusGet_PARAMS, NULL},
    {"cpssPxPortSerdesLanePolarityGet", 5, cpssPxPortSerdesLanePolarityGet_PARAMS, NULL},
    {"cpssPxPortSerdesLanePolaritySet", 5, cpssPxPortSerdesLanePolaritySet_PARAMS, NULL},
    {"cpssPxPortSerdesPolaritySet", 5, cpssPxPortSerdesPolaritySet_PARAMS, NULL},
    {"cpssPxPortSerdesPolarityGet", 5, cpssPxPortSerdesPolarityGet_PARAMS, NULL},
    {"cpssPxPortSerdesLoopbackModeSet", 3, cpssPxPortSerdesLoopbackModeSet_PARAMS, NULL},
    {"cpssPxPortSerdesLoopbackModeGet", 4, cpssPxPortSerdesLoopbackModeGet_PARAMS, NULL},
    {"cpssPxPortEomDfeResGet", 4, cpssPxPortEomDfeResGet_PARAMS, NULL},
    {"cpssPxPortSerdesEyeMatrixGet", 5, cpssPxPortSerdesEyeMatrixGet_PARAMS, NULL},
    {"cpssPxPortEomBaudRateGet", 4, cpssPxPortEomBaudRateGet_PARAMS, NULL},
    {"cpssPxPortSerdesTxEnableSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxPortSerdesTxEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxPortSerdesManualTxConfigSet", 4, cpssPxPortSerdesManualTxConfigSet_PARAMS, NULL},
    {"cpssPxPortSerdesManualTxConfigGet", 4, cpssPxPortSerdesManualTxConfigGet_PARAMS, NULL},
    {"cpssPxPortSerdesManualRxConfigSet", 4, cpssPxPortSerdesManualRxConfigSet_PARAMS, NULL},
    {"cpssPxPortSerdesManualRxConfigGet", 4, cpssPxPortSerdesManualRxConfigGet_PARAMS, NULL},
    {"cpssPxPortSerdesSquelchSet", 3, cpssPxPortSerdesSquelchSet_PARAMS, NULL},
    {"cpssPxPortSerdesSignalDetectGet", 3, cpssPxPortPcsSyncStableStatusGet_PARAMS, NULL},
    {"cpssPxPortSerdesStableSignalDetectGet", 3, cpssPxPortPcsSyncStableStatusGet_PARAMS, NULL},
    {"cpssPxPortSerdesLaneSignalDetectGet", 4, cpssPxPortSerdesLaneSignalDetectGet_PARAMS, NULL},
    {"cpssPxPortSerdesCDRLockStatusGet", 3, cpssPxPortSerdesCDRLockStatusGet_PARAMS, NULL},
    {"cpssPxPortSerdesAutoTuneOptAlgSet", 3, cpssPxPortSerdesAutoTuneOptAlgSet_PARAMS, NULL},
    {"cpssPxPortSerdesAutoTuneOptAlgGet", 3, cpssPxPortSerdesAutoTuneOptAlgGet_PARAMS, NULL},
    {"cpssPxPortSerdesAutoTuneResultsGet", 4, cpssPxPortSerdesAutoTuneResultsGet_PARAMS, NULL},
    {"cpssPxPortSerdesEnhancedAutoTune", 4, cpssPxPortSerdesEnhancedAutoTune_PARAMS, NULL},
    {"cpssPxPortSpeedSet", 3, cpssPxPortSpeedSet_PARAMS, NULL},
    {"cpssPxPortSpeedGet", 3, cpssPxPortSpeedGet_PARAMS, NULL},
    {"cpssPxPortMacCounterGet", 4, cpssPxPortMacCounterGet_PARAMS, NULL},
    {"cpssPxPortMacCountersOnPortGet", 3, cpssPxPortMacCountersOnPortGet_PARAMS, NULL},
    {"cpssPxPortMacCountersClearOnReadSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxPortMacCountersClearOnReadGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxPortMacOversizedPacketsCounterModeSet", 3, cpssPxPortMacOversizedPacketsCounterModeSet_PARAMS, NULL},
    {"cpssPxPortMacOversizedPacketsCounterModeGet", 3, cpssPxPortMacOversizedPacketsCounterModeGet_PARAMS, NULL},
    {"cpssPxPortMacCountersEnableSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxPortMacCountersEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxPortMacCounterCaptureGet", 4, cpssPxPortMacCounterGet_PARAMS, NULL},
    {"cpssPxPortMacCountersCaptureOnPortGet", 3, cpssPxPortMacCountersCaptureOnPortGet_PARAMS, NULL},
    {"cpssPxPortMacCountersCaptureTriggerSet", 2, prvCpssLogGenDevNumPortNum2_PARAMS, NULL},
    {"cpssPxPortMacCountersCaptureTriggerGet", 3, cpssPxPortMacCountersCaptureTriggerGet_PARAMS, NULL},
    {"cpssPxPortMacCountersOnCgPortGet", 3, cpssPxPortMacCountersOnCgPortGet_PARAMS, NULL},
    {"cpssPxPortStatTxDebugCountersGet", 2, cpssPxPortStatTxDebugCountersGet_PARAMS, NULL},
    {"cpssPxPortEgressCntrModeSet", 6, cpssPxPortEgressCntrModeSet_PARAMS, NULL},
    {"cpssPxPortEgressCntrModeGet", 6, cpssPxPortEgressCntrModeGet_PARAMS, NULL},
    {"cpssPxPortEgressCntrsGet", 3, cpssPxPortEgressCntrsGet_PARAMS, NULL},
    {"cpssPxPortMacCountersRxHistogramEnableSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxPortMacCountersTxHistogramEnableSet", 3, prvCpssLogGenDevNumPortNumEnable3_PARAMS, NULL},
    {"cpssPxPortMacCountersRxHistogramEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxPortMacCountersTxHistogramEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr3_PARAMS, NULL},
    {"cpssPxPortSyncEtherRecoveryClkConfigSet", 5, cpssPxPortSyncEtherRecoveryClkConfigSet_PARAMS, NULL},
    {"cpssPxPortSyncEtherRecoveryClkConfigGet", 5, cpssPxPortSyncEtherRecoveryClkConfigGet_PARAMS, NULL},
    {"cpssPxPortSyncEtherRecoveryClkDividerValueSet", 5, cpssPxPortSyncEtherRecoveryClkDividerValueSet_PARAMS, NULL},
    {"cpssPxPortSyncEtherRecoveryClkDividerValueGet", 5, cpssPxPortSyncEtherRecoveryClkDividerValueGet_PARAMS, NULL},
    {"cpssPxPortTxDebugQueueingEnableSet", 4, cpssPxPortTxDebugQueueingEnableSet_PARAMS, NULL},
    {"cpssPxPortTxDebugQueueingEnableGet", 4, cpssPxPortTxDebugQueueingEnableGet_PARAMS, NULL},
    {"cpssPxPortTxDebugQueueTxEnableSet", 4, cpssPxPortTxDebugQueueingEnableSet_PARAMS, NULL},
    {"cpssPxPortTxDebugQueueTxEnableGet", 4, cpssPxPortTxDebugQueueingEnableGet_PARAMS, NULL},
    {"cpssPxPortTxDebugResourceHistogramThresholdSet", 3, cpssPxPortTxDebugResourceHistogramThresholdSet_PARAMS, NULL},
    {"cpssPxPortTxDebugResourceHistogramThresholdGet", 3, cpssPxPortTxDebugResourceHistogramThresholdGet_PARAMS, NULL},
    {"cpssPxPortTxDebugResourceHistogramCounterGet", 3, cpssPxPortTxDebugResourceHistogramCounterGet_PARAMS, NULL},
    {"cpssPxPortTxDebugGlobalDescLimitSet", 2, cpssPxPortTxDebugGlobalDescLimitSet_PARAMS, NULL},
    {"cpssPxPortTxDebugGlobalDescLimitGet", 2, cpssPxPortTxDebugGlobalDescLimitGet_PARAMS, NULL},
    {"cpssPxPortTxDebugGlobalQueueTxEnableSet", 2, prvCpssLogGenDevNumEnable2_PARAMS, NULL},
    {"cpssPxPortTxDebugGlobalQueueTxEnableGet", 2, prvCpssLogGenDevNumEnablePtr2_PARAMS, NULL},
    {"cpssPxPortTxDebugDescNumberGet", 3, cpssPxPortTxDebugDescNumberGet_PARAMS, NULL},
    {"cpssPxPortTxDebugQueueDumpAll", 2, cpssPxPortTxDebugQueueDumpAll_PARAMS, NULL},
    {"cpssPxPortTxSchedulerProfileIdSet", 3, cpssPxPortTxSchedulerProfileIdSet_PARAMS, NULL},
    {"cpssPxPortTxSchedulerProfileIdGet", 3, cpssPxPortTxSchedulerProfileIdGet_PARAMS, NULL},
    {"cpssPxPortTxSchedulerWrrMtuSet", 2, cpssPxPortTxSchedulerWrrMtuSet_PARAMS, NULL},
    {"cpssPxPortTxSchedulerWrrMtuGet", 2, cpssPxPortTxSchedulerWrrMtuGet_PARAMS, NULL},
    {"cpssPxPortTxSchedulerWrrProfileSet", 4, cpssPxPortTxSchedulerWrrProfileSet_PARAMS, NULL},
    {"cpssPxPortTxSchedulerWrrProfileGet", 4, cpssPxPortTxSchedulerWrrProfileGet_PARAMS, NULL},
    {"cpssPxPortTxSchedulerArbitrationGroupSet", 4, cpssPxPortTxSchedulerArbitrationGroupSet_PARAMS, NULL},
    {"cpssPxPortTxSchedulerArbitrationGroupGet", 4, cpssPxPortTxSchedulerArbitrationGroupGet_PARAMS, NULL},
    {"cpssPxPortTxSchedulerProfileByteCountChangeEnableSet", 4, cpssPxPortTxSchedulerProfileByteCountChangeEnableSet_PARAMS, NULL},
    {"cpssPxPortTxSchedulerProfileByteCountChangeEnableGet", 4, cpssPxPortTxSchedulerProfileByteCountChangeEnableGet_PARAMS, NULL},
    {"cpssPxPortTxSchedulerShaperByteCountChangeValueSet", 3, cpssPxPortTxSchedulerShaperByteCountChangeValueSet_PARAMS, NULL},
    {"cpssPxPortTxSchedulerShaperByteCountChangeValueGet", 3, cpssPxPortTxSchedulerShaperByteCountChangeValueGet_PARAMS, NULL},
    {"cpssPxPortTxSchedulerProfileCountModeSet", 3, cpssPxPortTxSchedulerProfileCountModeSet_PARAMS, NULL},
    {"cpssPxPortTxSchedulerProfileCountModeGet", 3, cpssPxPortTxSchedulerProfileCountModeGet_PARAMS, NULL},
    {"cpssPxPortTxShaperEnableSet", 4, cpssPxPortTxShaperEnableSet_PARAMS, NULL},
    {"cpssPxPortTxShaperEnableGet", 4, cpssPxPortTxShaperEnableGet_PARAMS, NULL},
    {"cpssPxPortTxShaperQueueEnableSet", 5, cpssPxPortTxShaperQueueEnableSet_PARAMS, NULL},
    {"cpssPxPortTxShaperQueueEnableGet", 5, cpssPxPortTxShaperQueueEnableGet_PARAMS, NULL},
    {"cpssPxPortTxShaperProfileSet", 4, cpssPxPortTxShaperProfileSet_PARAMS, NULL},
    {"cpssPxPortTxShaperProfileGet", 5, cpssPxPortTxShaperProfileGet_PARAMS, NULL},
    {"cpssPxPortTxShaperModeSet", 3, cpssPxPortTxShaperModeSet_PARAMS, NULL},
    {"cpssPxPortTxShaperModeGet", 3, cpssPxPortTxShaperModeGet_PARAMS, NULL},
    {"cpssPxPortTxShaperBaselineSet", 2, cpssPxPortTxShaperBaselineSet_PARAMS, NULL},
    {"cpssPxPortTxShaperBaselineGet", 2, cpssPxPortTxShaperBaselineGet_PARAMS, NULL},
    {"cpssPxPortTxShaperConfigurationSet", 2, cpssPxPortTxShaperConfigurationSet_PARAMS, NULL},
    {"cpssPxPortTxShaperConfigurationGet", 2, cpssPxPortTxShaperConfigurationGet_PARAMS, NULL},
    {"cpssPxPortTxShaperQueueProfileSet", 5, cpssPxPortTxShaperQueueProfileSet_PARAMS, NULL},
    {"cpssPxPortTxShaperQueueProfileGet", 6, cpssPxPortTxShaperQueueProfileGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropProfileIdSet", 3, cpssPxPortTxTailDropProfileIdSet_PARAMS, NULL},
    {"cpssPxPortTxTailDropProfileIdGet", 3, cpssPxPortTxTailDropProfileIdGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropUcEnableSet", 2, prvCpssLogGenDevNumEnable2_PARAMS, NULL},
    {"cpssPxPortTxTailDropUcEnableGet", 2, prvCpssLogGenDevNumEnablePtr2_PARAMS, NULL},
    {"cpssPxPortTxTailDropProfileSet", 5, cpssPxPortTxTailDropProfileSet_PARAMS, NULL},
    {"cpssPxPortTxTailDropProfileGet", 5, cpssPxPortTxTailDropProfileGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropPacketModeLengthSet", 2, cpssPxPortTxTailDropPacketModeLengthSet_PARAMS, NULL},
    {"cpssPxPortTxTailDropPacketModeLengthGet", 2, cpssPxPortTxTailDropPacketModeLengthGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropProfileBufferConsumptionModeSet", 3, cpssPxPortTxTailDropProfileBufferConsumptionModeSet_PARAMS, NULL},
    {"cpssPxPortTxTailDropProfileBufferConsumptionModeGet", 3, cpssPxPortTxTailDropProfileBufferConsumptionModeGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropMaskSharedBuffEnableSet", 2, prvCpssLogGenDevNumEnable2_PARAMS, NULL},
    {"cpssPxPortTxTailDropMaskSharedBuffEnableGet", 2, prvCpssLogGenDevNumEnablePtr2_PARAMS, NULL},
    {"cpssPxPortTxTailDropSharedBuffMaxLimitSet", 2, cpssPxPortTxTailDropSharedBuffMaxLimitSet_PARAMS, NULL},
    {"cpssPxPortTxTailDropSharedBuffMaxLimitGet", 2, cpssPxPortTxTailDropSharedBuffMaxLimitGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet", 5, cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableSet_PARAMS, NULL},
    {"cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet", 5, cpssPxPortTxTailDropProfileWeightedRandomTailDropEnableGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropWrtdMasksSet", 2, cpssPxPortTxTailDropWrtdMasksSet_PARAMS, NULL},
    {"cpssPxPortTxTailDropWrtdMasksGet", 2, cpssPxPortTxTailDropWrtdMasksGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropProfileTcSharingSet", 5, cpssPxPortTxTailDropProfileTcSharingSet_PARAMS, NULL},
    {"cpssPxPortTxTailDropProfileTcSharingGet", 5, cpssPxPortTxTailDropProfileTcSharingGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropProfileTcSet", 4, cpssPxPortTxTailDropProfileTcSet_PARAMS, NULL},
    {"cpssPxPortTxTailDropProfileTcGet", 4, cpssPxPortTxTailDropProfileTcGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropTcBuffNumberGet", 4, cpssPxPortTxTailDropTcBuffNumberGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropMcastPcktDescLimitSet", 2, cpssPxPortTxTailDropMcastPcktDescLimitSet_PARAMS, NULL},
    {"cpssPxPortTxTailDropMcastPcktDescLimitGet", 2, cpssPxPortTxTailDropMcastPcktDescLimitGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropMcastBuffersLimitSet", 2, cpssPxPortTxTailDropMcastBuffersLimitSet_PARAMS, NULL},
    {"cpssPxPortTxTailDropMcastBuffersLimitGet", 2, cpssPxPortTxTailDropMcastBuffersLimitGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropMcastDescNumberGet", 2, cpssPxPortTxTailDropMcastDescNumberGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropMcastBuffNumberGet", 2, cpssPxPortTxTailDropMcastDescNumberGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropSharedPolicySet", 2, cpssPxPortTxTailDropSharedPolicySet_PARAMS, NULL},
    {"cpssPxPortTxTailDropSharedPolicyGet", 2, cpssPxPortTxTailDropSharedPolicyGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropSharedPoolLimitsSet", 4, cpssPxPortTxTailDropSharedPoolLimitsSet_PARAMS, NULL},
    {"cpssPxPortTxTailDropSharedPoolLimitsGet", 4, cpssPxPortTxTailDropSharedPoolLimitsGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropSharedResourceDescNumberGet", 3, cpssPxPortTxTailDropSharedResourceDescNumberGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropSharedResourceBuffNumberGet", 3, cpssPxPortTxTailDropSharedResourceDescNumberGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropGlobalDescNumberGet", 2, cpssPxPortTxTailDropMcastDescNumberGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropGlobalBuffNumberGet", 2, cpssPxPortTxTailDropMcastDescNumberGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropDescNumberGet", 3, cpssPxPortTxTailDropDescNumberGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropBuffNumberGet", 3, cpssPxPortTxTailDropDescNumberGet_PARAMS, NULL},
    {"cpssPxPortTxTailDropDbaModeEnableSet", 2, prvCpssLogGenDevNumEnable2_PARAMS, NULL},
    {"cpssPxPortTxTailDropDbaModeEnableGet", 2, prvCpssLogGenDevNumEnablePtr2_PARAMS, NULL},
    {"cpssPxPortTxTailDropDbaAvailableBuffSet", 2, cpssPxPortCnAvailableBuffSet_PARAMS, NULL},
    {"cpssPxPortTxTailDropDbaAvailableBuffGet", 2, cpssPxPortCnAvailableBuffGet_PARAMS, NULL},
    {"cpssPxPortPizzaArbiterDevStateGet", 2, cpssPxPortPizzaArbiterDevStateGet_PARAMS, NULL},
    {"cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionSet", 3, cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionSet_PARAMS, NULL},
    {"cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionGet", 3, cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionGet_PARAMS, NULL},
    {"cpssPxPortDynamicPizzaArbiterIfWorkConservingModeSet", 3, cpssPxPortDynamicPizzaArbiterIfWorkConservingModeSet_PARAMS, NULL},
    {"cpssPxPortDynamicPizzaArbiterIfWorkConservingModeGet", 3, cpssPxPortDynamicPizzaArbiterIfWorkConservingModeGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_PX_CPSS_LOG_LIB_PORT(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssPxPortLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssPxPortLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

