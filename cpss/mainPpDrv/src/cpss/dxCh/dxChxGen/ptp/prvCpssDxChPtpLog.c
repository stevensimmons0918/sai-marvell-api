/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChPtpLog.c
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
#include <cpss/dxCh/dxChxGen/ptp/cpssDxChPtp.h>
#include <cpss/dxCh/dxChxGen/ptp/private/prvCpssDxChPtpLog.h>
#include <cpss/generic/log/prvCpssGenLog.h>
#include <cpss/generic/networkIf/private/prvCpssGenNetworkIfLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT[]  =
{
    "CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_BASIC_E",
    "CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_TRANS_CLK_E",
    "CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_PIGGY_BACK_E",
    "CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_BOUNDRY_CLK_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT[]  =
{
    "CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_DISABLE_E",
    "CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V1_E",
    "CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT[]  =
{
    "CPSS_DXCH_PTP_REF_CLOCK_SELECT_MAIN_25_MHZ_E",
    "CPSS_DXCH_PTP_REF_CLOCK_SELECT_DEDICATED_PTP_REF_CLK_E",
    "CPSS_DXCH_PTP_REF_CLOCK_SELECT_SD_DIFFRENTIAL_REF_CLK_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TAI_CLOCK_FREQ_ENT[]  =
{
    "CPSS_DXCH_PTP_10_FREQ_E",
    "CPSS_DXCH_PTP_20_FREQ_E",
    "CPSS_DXCH_PTP_25_FREQ_E",
    "CPSS_DXCH_PTP_156_25_FREQ_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TAI_CLOCK_FREQ_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT[]  =
{
    "CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_DISABLED_E",
    "CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_CLOCK_GEN_E",
    "CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_CLOCK_RECEPTION_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT[]  =
{
    "CPSS_DXCH_PTP_TAI_CLOCK_MODE_DISABLED_E",
    "CPSS_DXCH_PTP_TAI_CLOCK_MODE_OUTPUT_E",
    "CPSS_DXCH_PTP_TAI_CLOCK_MODE_INPUT_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT[]  =
{
    "CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_DISABLED_E",
    "CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_CLOCK_RECEPTION_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT[]  =
{
    "CPSS_DXCH_PTP_TAI_CLOCK_SELECT_PTP_PLL_E",
    "CPSS_DXCH_PTP_TAI_CLOCK_SELECT_CORE_PLL_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT[]  =
{
    "CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_CLOCK_E",
    "CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_PULSE_E",
    "CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_PCLK_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TAI_NUMBER_ENT[]  =
{
    "CPSS_DXCH_PTP_TAI_NUMBER_0_E",
    "CPSS_DXCH_PTP_TAI_NUMBER_1_E",
    "CPSS_DXCH_PTP_TAI_NUMBER_2_E",
    "CPSS_DXCH_PTP_TAI_NUMBER_3_E",
    "CPSS_DXCH_PTP_TAI_NUMBER_4_E",
    "CPSS_DXCH_PTP_TAI_NUMBER_ALL_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TAI_NUMBER_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT[]  =
{
    "CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_PCLK_E",
    "CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_RCVR_CLK_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT[]  =
{
    "CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_DISABLED_E",
    "CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_PCLK_E",
    "CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_CLOCK_GEN_E",
    "CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_CLOCK_RECEPTION_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT[]  =
{
    "CPSS_DXCH_PTP_TAI_PIN_SELECT_A_E",
    "CPSS_DXCH_PTP_TAI_PIN_SELECT_B_E",
    "CPSS_DXCH_PTP_TAI_PIN_SELECT_C_E",
    "CPSS_DXCH_PTP_TAI_PIN_SELECT_NONE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT[]  =
{
    "CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E",
    "CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E",
    "CPSS_DXCH_PTP_TAI_PULSE_IN_DISABLED_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TAI_SELECT_UNIT_ENT[]  =
{
    "CPSS_DXCH_PTP_TAI_SELECT_UNIT_CTSU_E",
    "CPSS_DXCH_PTP_TAI_SELECT_UNIT_ERMRK_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TAI_SELECT_UNIT_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT[]  =
{
    "CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E",
    "CPSS_DXCH_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E",
    "CPSS_DXCH_PTP_TAI_TOD_TYPE_LOAD_VALUE_E",
    "CPSS_DXCH_PTP_TAI_TOD_TYPE_CAPTURE_VALUE0_E",
    "CPSS_DXCH_PTP_TAI_TOD_TYPE_CAPTURE_VALUE1_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT[]  =
{
    "CPSS_DXCH_PTP_TOD_COUNTER_FUNC_UPDATE_E",
    "CPSS_DXCH_PTP_TOD_COUNTER_FUNC_INCREMENT_E",
    "CPSS_DXCH_PTP_TOD_COUNTER_FUNC_CAPTURE_E",
    "CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GENERATE_E",
    "CPSS_DXCH_PTP_TOD_COUNTER_FUNC_DECREMENT_E",
    "CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E",
    "CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E",
    "CPSS_DXCH_PTP_TOD_COUNTER_FUNC_FREQUENCE_UPDATE_E",
    "CPSS_DXCH_PTP_TOD_COUNTER_FUNC_NOP_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ENT[]  =
{
    "CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_TOTAL_E",
    "CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V1_E",
    "CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V2_E",
    "CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_Y1731_E",
    "CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_NTP_TS_E",
    "CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_NTP_RX_E",
    "CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_NTP_TX_E",
    "CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_WAMP_E",
    "CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_NONE_ACTION_E",
    "CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_FORWARD_E",
    "CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_DROP_E",
    "CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_E",
    "CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ADD_TIME_E",
    "CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ADD_CORR_TIME_E",
    "CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_ADD_TIME_E",
    "CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_ADD_CORR_TIME_E",
    "CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ADD_INGRESS_TIME_E",
    "CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_ADD_INGRESS_TIME_E",
    "CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_CAPTURE_INGRESS_TIME_E",
    "CPSS_DXCH_PTP_TSU_PACKET_COUNTER_INDEX_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TS_ACTION_ENT[]  =
{
    "CPSS_DXCH_PTP_TS_ACTION_NONE_E",
    "CPSS_DXCH_PTP_TS_ACTION_DROP_E",
    "CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E",
    "CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E",
    "CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E",
    "CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_TIME_E",
    "CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_CORRECTED_TIME_E",
    "CPSS_DXCH_PTP_TS_ACTION_ADD_INGRESS_TIME_E",
    "CPSS_DXCH_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E",
    "CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_INGRESS_TIME_E",
    "CPSS_DXCH_PTP_TS_ACTION_ADD_INGRESS_EGRESS_TIME_E",
    "CPSS_DXCH_PTP_TS_ACTION_ALL_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TS_ACTION_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT[]  =
{
    "CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_INGRESS_E",
    "CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_U_BIT_MODE_ENT[]  =
{
    "CPSS_DXCH_PTP_TAI_U_BIT_MODE_ASSIGNED_E",
    "CPSS_DXCH_PTP_TAI_U_BIT_MODE_COUNTER_E",
    "CPSS_DXCH_PTP_TAI_U_BIT_MODE_PROGRAMMED_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_U_BIT_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TAI_INSTANCE_ENT[]  =
{
    "CPSS_DXCH_PTP_TAI_INSTANCE_GLOBAL_E",
    "CPSS_DXCH_PTP_TAI_INSTANCE_PORT_E",
    "CPSS_DXCH_PTP_TAI_INSTANCE_ALL_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TAI_INSTANCE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TRANSPORT_TYPE_ENT[]  =
{
    "CPSS_DXCH_PTP_TRANSPORT_TYPE_ETHERNET_E",
    "CPSS_DXCH_PTP_TRANSPORT_TYPE_UDP_IPV4_E",
    "CPSS_DXCH_PTP_TRANSPORT_TYPE_UDP_IPV6_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TRANSPORT_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TS_PACKET_TYPE_ENT[]  =
{
    "CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V1_E",
    "CPSS_DXCH_PTP_TS_PACKET_TYPE_PTP_V2_E",
    "CPSS_DXCH_PTP_TS_PACKET_TYPE_Y1731_E",
    "CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TS_E",
    "CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_RX_E",
    "CPSS_DXCH_PTP_TS_PACKET_TYPE_NTP_TX_E",
    "CPSS_DXCH_PTP_TS_PACKET_TYPE_WAMP_E",
    "CPSS_DXCH_PTP_TS_PACKET_TYPE_ALL_E",
    "CPSS_DXCH_PTP_TS_PACKET_TYPE_RESERVED_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TS_PACKET_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TS_TAG_MODE_ENT[]  =
{
    "CPSS_DXCH_PTP_TS_TAG_MODE_NONE_E",
    "CPSS_DXCH_PTP_TS_TAG_MODE_ALL_NON_EXTENDED_E",
    "CPSS_DXCH_PTP_TS_TAG_MODE_ALL_EXTENDED_E",
    "CPSS_DXCH_PTP_TS_TAG_MODE_NON_EXTENDED_E",
    "CPSS_DXCH_PTP_TS_TAG_MODE_EXTENDED_E",
    "CPSS_DXCH_PTP_TS_TAG_MODE_PIGGYBACK_E",
    "CPSS_DXCH_PTP_TS_TAG_MODE_HYBRID_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TS_TAG_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_ENT[]  =
{
    "CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E",
    "CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_MODIFY_TAG_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT[]  =
{
    "CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_CLEAR_E",
    "CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_RECALCULATE_E",
    "CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_KEEP_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ptpOverEhernetTsEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ptpOverUdpIpv4TsEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ptpOverUdpIpv6TsEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, messageTypeTsEnableBmp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, transportSpecificTsEnableBmp);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, invalidPtpPktCmd, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, invalidPtpCpuCode, CPSS_NET_RX_CPU_CODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, invalidOutPiggybackPktCmd, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, invalidOutPiggybackCpuCode, CPSS_NET_RX_CPU_CODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, invalidInPiggybackPktCmd, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, invalidInPiggybackCpuCode, CPSS_NET_RX_CPU_CODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, invalidTsPktCmd, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, invalidTsCpuCode, CPSS_NET_RX_CPU_CODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_EGRESS_EXCEPTION_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_EGRESS_EXCEPTION_COUNTERS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, invalidPtpPktCnt);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, invalidOutPiggybackPktCnt);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, invalidInPiggybackPktCnt);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, invalidTsPktCnt);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ptpExceptionCommandEnable);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ptpExceptionCommand, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, ptpExceptionCpuCode, CPSS_NET_RX_CPU_CODE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ptpVersionCheckEnable);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TAI_ID_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_TAI_ID_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, taiInstance, CPSS_DXCH_PTP_TAI_INSTANCE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, taiNumber, CPSS_DXCH_PTP_TAI_NUMBER_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, portNum);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TAI_TOD_STEP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_TAI_TOD_STEP_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nanoSeconds);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, fracNanoSeconds);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TOD_COUNT_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_TOD_COUNT_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nanoSeconds);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, seconds, GT_U64);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, fracNanoSeconds);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TSU_CONTROL_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_TSU_CONTROL_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, unitEnable);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, taiNumber, CPSS_DXCH_PTP_TAI_NUMBER_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, enable);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, action, CPSS_DXCH_PTP_TS_ACTION_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, format, CPSS_DXCH_PTP_TS_PACKET_TYPE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfBits);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfLsbits);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfShifts);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, entryValid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, queueEntryId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, taiSelect);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, todUpdateFlag);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, timestamp);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_CFG_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_TS_CFG_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, tsMode, CPSS_DXCH_PTP_TS_TIMESTAMPING_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, offsetProfile);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, OE);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, tsAction, CPSS_DXCH_PTP_TS_ACTION_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, packetFormat, CPSS_DXCH_PTP_TS_PACKET_TYPE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ptpTransport, CPSS_DXCH_PTP_TRANSPORT_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, offset);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, offset2);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ptpMessageType);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, domain);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ingrLinkDelayEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, packetDispatchingEnable);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, egressPipeDelayEncrypted);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, egressPipeDelayFsuOffset);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, egressPipeDelay);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, entryValid);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isPtpExeption);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, packetFormat, CPSS_DXCH_PTP_TS_PACKET_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, messageType);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, domainNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, sequenceId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, queueEntryId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, portNum);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, entryValid);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isPtpExeption);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, packetFormat, CPSS_DXCH_PTP_TS_PACKET_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, taiSelect);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, todUpdateFlag);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, messageType);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, domainNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, sequenceId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, timestamp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, portNum);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, tsAction, CPSS_DXCH_PTP_TS_ACTION_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ingrLinkDelayEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, packetDispatchingEnable);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, tsTagParseEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, hybridTsTagParseEnable);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, tsTagEtherType);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, hybridTsTagEtherType);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, tsReceptionEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, tsPiggyBackEnable);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, tsTagMode, CPSS_DXCH_PTP_TS_TAG_MODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ptpIpv4Mode, CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ptpIpv6Mode, CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ntpIpv4Mode, CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ntpIpv6Mode, CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, wampIpv4Mode, CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, wampIpv6Mode, CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_EGRESS_EXCEPTION_COUNTERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_EGRESS_EXCEPTION_COUNTERS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_EGRESS_EXCEPTION_COUNTERS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_CLOCK_FREQ_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TAI_CLOCK_FREQ_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_TAI_CLOCK_FREQ_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_ID_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_TAI_ID_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TAI_ID_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_NUMBER_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TAI_NUMBER_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_TAI_NUMBER_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_NUMBER_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TAI_NUMBER_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PTP_TAI_NUMBER_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_SELECT_UNIT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TAI_SELECT_UNIT_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_TAI_SELECT_UNIT_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_TOD_STEP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_TAI_TOD_STEP_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TAI_TOD_STEP_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TOD_COUNT_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_TOD_COUNT_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TOD_COUNT_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TSU_CONTROL_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_TSU_CONTROL_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TSU_CONTROL_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_ACTION_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TS_ACTION_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_TS_ACTION_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_CFG_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_TS_CFG_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_CFG_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_U_BIT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_U_BIT_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_PTP_U_BIT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_PTP_U_BIT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_PTP_U_BIT_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_PTP_U_BIT_MODE_ENT);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DIRECTION_ENT_clockInterfaceDirection = {
     "clockInterfaceDirection", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DIRECTION_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DIRECTION_ENT_pulseInterfaceDirection = {
     "pulseInterfaceDirection", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DIRECTION_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC_PTR_egrExceptionCfgPtr = {
     "egrExceptionCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT_checkingMode = {
     "checkingMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT_domainMode = {
     "domainMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC_PTR_ingrExceptionCfgPtr = {
     "ingrExceptionCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT_refClockSelect = {
     "refClockSelect", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_CLOCK_FREQ_ENT_clockFrequency = {
     "clockFrequency", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_TAI_CLOCK_FREQ_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT_clockGenerationMode = {
     "clockGenerationMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT_clockMode = {
     "clockMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT_clockReceptionMode = {
     "clockReceptionMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT_clockSelect = {
     "clockSelect", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr = {
     "taiIdPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TAI_ID_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT_interfaceType = {
     "interfaceType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT_pclkRcvrClkMode = {
     "pclkRcvrClkMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT_pClkSelMode = {
     "pClkSelMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT_pinId = {
     "pinId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT_pulseMode = {
     "pulseMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_SELECT_UNIT_ENT_taiUnit = {
     "taiUnit", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_TAI_SELECT_UNIT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_TOD_STEP_STC_PTR_todStepPtr = {
     "todStepPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TAI_TOD_STEP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT_todValueType = {
     "todValueType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TSU_CONTROL_STC_PTR_controlPtr = {
     "controlPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TSU_CONTROL_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC_PTR_frameCounterConfigPtr = {
     "frameCounterConfigPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ENT_counterType = {
     "counterType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC_PTR_maskingProfilePtr = {
     "maskingProfilePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TS_CFG_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TS_CFG_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT_egrTimeCorrTaiSelMode = {
     "egrTimeCorrTaiSelMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC_PTR_egrDelayValPtr = {
     "egrDelayValPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC_PTR_tsTagGlobalCfgPtr = {
     "tsTagGlobalCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC_PTR_udpCsUpdModePtr = {
     "udpCsUpdModePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PTP_U_BIT_MODE_ENT_uBitMode = {
     "uBitMode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_PTP_U_BIT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_32_egrAsymmetryCorr = {
     "egrAsymmetryCorr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_32_fracNanoSecond = {
     "fracNanoSecond", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_32_ingressDelay = {
     "ingressDelay", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_32_ingressDelayCorr = {
     "ingressDelayCorr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_egrCorrFldPBEnable = {
     "egrCorrFldPBEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_ingrCorrFldPBEnable = {
     "ingrCorrFldPBEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_resetRxUnit = {
     "resetRxUnit", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_resetTxUnit = {
     "resetTxUnit", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_tsuBypassEnable = {
     "tsuBypassEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_tsuEnable = {
     "tsuEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_TAI_SELECT_BMP_taiSelectBmp = {
     "taiSelectBmp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_TAI_SELECT_BMP)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_HEX_etherTypeIndex = {
     "etherTypeIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_PTR_domainIdArr = {
     "domainIdArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_captureIndex = {
     "captureIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_delay = {
     "delay", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_domainId = {
     "domainId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_domainProfile = {
     "domainProfile", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_edge = {
     "edge", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_egrTsTaiNum = {
     "egrTsTaiNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_frameCounter = {
     "frameCounter", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_idMapBmp = {
     "idMapBmp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_messageType = {
     "messageType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ntpTimeOffset = {
     "ntpTimeOffset", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_queueSize = {
     "queueSize", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_rxTaiSelect = {
     "rxTaiSelect", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_taiSelect = {
     "taiSelect", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_udpPortIndex = {
     "udpPortIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_udpPortNum = {
     "udpPortNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DIRECTION_ENT_PTR_clockInterfaceDirectionPtr = {
     "clockInterfaceDirectionPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DIRECTION_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DIRECTION_ENT_PTR_pulseInterfaceDirectionPtr = {
     "pulseInterfaceDirectionPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DIRECTION_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC_PTR_egrExceptionCfgPtr = {
     "egrExceptionCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_EGRESS_EXCEPTION_COUNTERS_STC_PTR_egrExceptionCntPtr = {
     "egrExceptionCntPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_EGRESS_EXCEPTION_COUNTERS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT_PTR_checkingModePtr = {
     "checkingModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT_PTR_domainModePtr = {
     "domainModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC_PTR_ingrExceptionCfgPtr = {
     "ingrExceptionCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT_PTR_refClockSelectPtr = {
     "refClockSelectPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT_PTR_clockGenerationModePtr = {
     "clockGenerationModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT_PTR_clockModePtr = {
     "clockModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT_PTR_clockReceptionModePtr = {
     "clockReceptionModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT_PTR_clockSelectPtr = {
     "clockSelectPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TAI_NUMBER_ENT_PTR_taiNumberPtr = {
     "taiNumberPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TAI_NUMBER_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT_PTR_pclkRcvrClkModePtr = {
     "pclkRcvrClkModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT_PTR_pClkSelModePtr = {
     "pClkSelModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT_PTR_pulseModePtr = {
     "pulseModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TAI_TOD_STEP_STC_PTR_todStepPtr = {
     "todStepPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TAI_TOD_STEP_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT_PTR_functionPtr = {
     "functionPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TOD_COUNT_STC_PTR_todValuePtr = {
     "todValuePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TOD_COUNT_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TSU_CONTROL_STC_PTR_controlPtr = {
     "controlPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TSU_CONTROL_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC_PTR_frameCounterConfigPtr = {
     "frameCounterConfigPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC_PTR_maskingProfilePtr = {
     "maskingProfilePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TS_CFG_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TS_CFG_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT_PTR_egrTimeCorrTaiSelModePtr = {
     "egrTimeCorrTaiSelModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC_PTR_egrDelayValPtr = {
     "egrDelayValPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC_PTR_tsQueueEntryPtr = {
     "tsQueueEntryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC_PTR_tsQueueEntryPtr = {
     "tsQueueEntryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC_PTR_entryPtr = {
     "entryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC_PTR_tsTagGlobalCfgPtr = {
     "tsTagGlobalCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC_PTR_tsTagPortCfgPtr = {
     "tsTagPortCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC_PTR_udpCsUpdModePtr = {
     "udpCsUpdModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PTP_U_BIT_MODE_ENT_PTR_uBitModePtr = {
     "uBitModePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PTP_U_BIT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_32_PTR_egrAsymmetryCorrPtr = {
     "egrAsymmetryCorrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_32_PTR_fracNanoSecondPtr = {
     "fracNanoSecondPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_32_PTR_ingressDelayCorrPtr = {
     "ingressDelayCorrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_32_PTR_ingressDelayPtr = {
     "ingressDelayPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_egrCorrFldPBEnablePtr = {
     "egrCorrFldPBEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_finishPtr = {
     "finishPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_ingrCorrFldPBEnablePtr = {
     "ingrCorrFldPBEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_resetRxUnitPtr = {
     "resetRxUnitPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_resetTxUnitPtr = {
     "resetTxUnitPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_tsuBypassEnablePtr = {
     "tsuBypassEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_tsuEnablePtr = {
     "tsuEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_TAI_SELECT_BMP_PTR_taiSelectBmpPtr = {
     "taiSelectBmpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_TAI_SELECT_BMP)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_delayPtr = {
     "delayPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_domainIdArr = {
     "domainIdArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_domainIdPtr = {
     "domainIdPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_edgePtr = {
     "edgePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_egrTsTaiNumPtr = {
     "egrTsTaiNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_extPulseWidthPtr = {
     "extPulseWidthPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_fracNanoSecondsPtr = {
     "fracNanoSecondsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_gracefulStepPtr = {
     "gracefulStepPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_idMapBmpPtr = {
     "idMapBmpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_nanoSecondsPtr = {
     "nanoSecondsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_ntpTimeOffsetPtr = {
     "ntpTimeOffsetPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_queueEntryId0Ptr = {
     "queueEntryId0Ptr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_queueEntryId1Ptr = {
     "queueEntryId1Ptr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_queueSizePtr = {
     "queueSizePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_rxTaiSelectPtr = {
     "rxTaiSelectPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_secondsPtr = {
     "secondsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_taiSelectPtr = {
     "taiSelectPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_udpPortNumPtr = {
     "udpPortNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiPtpPulseInterfaceSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DIRECTION_ENT_pulseInterfaceDirection,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpEgressExceptionCfgSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC_PTR_egrExceptionCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpIngressExceptionCfgSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC_PTR_ingrExceptionCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpRefClockSourceSelectSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT_refClockSelect
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiClockGenerationModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_IN_CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT_clockGenerationMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiClockModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_IN_CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT_clockMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiClockReceptionModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_IN_CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT_clockReceptionMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiPClkSelectionModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_IN_CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT_pClkSelMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiTodStepSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_IN_CPSS_DXCH_PTP_TAI_TOD_STEP_STC_PTR_todStepPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiTodSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_IN_CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT_todValueType,
    &DX_IN_CPSS_DXCH_PTP_TOD_COUNT_STC_PTR_todValuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiTodGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_IN_CPSS_DXCH_PTP_TAI_TOD_TYPE_ENT_todValueType,
    &DX_OUT_CPSS_DXCH_PTP_TOD_COUNT_STC_PTR_todValuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpUBitModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_IN_CPSS_DXCH_PTP_U_BIT_MODE_ENT_uBitMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiFractionalNanosecondDriftSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_IN_GT_32_fracNanoSecond
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiInternalClockGenerateEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiIncomingClockCounterSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_IN_GT_BOOL_enable,
    &DX_IN_GT_U32_value
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiTodCaptureStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_IN_GT_U32_captureIndex,
    &DX_OUT_GT_BOOL_PTR_validPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiClockGenerationEdgeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_IN_GT_U32_edge
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiExternalPulseWidthSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_IN_GT_U32_extPulseWidth
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiGracefulStepSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_IN_GT_U32_gracefulStep
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiPClkCycleSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_IN_GT_U32_nanoSeconds
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiClockCycleSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_IN_GT_U32_seconds,
    &DX_IN_GT_U32_nanoSeconds
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiIncomingTriggerCounterSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_IN_GT_U32_value
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiClockGenerationModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_OUT_CPSS_DXCH_PTP_TAI_CLOCK_GEN_MODE_ENT_PTR_clockGenerationModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiClockModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_OUT_CPSS_DXCH_PTP_TAI_CLOCK_MODE_ENT_PTR_clockModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiClockReceptionModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_OUT_CPSS_DXCH_PTP_TAI_CLOCK_RECEPTION_MODE_ENT_PTR_clockReceptionModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiPClkSelectionModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_OUT_CPSS_DXCH_PTP_TAI_PCLK_SEL_MODE_ENT_PTR_pClkSelModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiTodStepGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_OUT_CPSS_DXCH_PTP_TAI_TOD_STEP_STC_PTR_todStepPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpUBitModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_OUT_CPSS_DXCH_PTP_U_BIT_MODE_ENT_PTR_uBitModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiFractionalNanosecondDriftGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_OUT_GT_32_PTR_fracNanoSecondPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiInternalClockGenerateEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiIncomingClockCounterGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_OUT_GT_BOOL_PTR_enablePtr,
    &DX_OUT_GT_U32_PTR_valuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTodCounterFunctionTriggerStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_OUT_GT_BOOL_PTR_finishPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiClockGenerationEdgeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_OUT_GT_U32_PTR_edgePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiExternalPulseWidthGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_OUT_GT_U32_PTR_extPulseWidthPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiGracefulStepGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_OUT_GT_U32_PTR_gracefulStepPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiPClkCycleGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_OUT_GT_U32_PTR_nanoSecondsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiClockCycleGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_OUT_GT_U32_PTR_secondsPtr,
    &DX_OUT_GT_U32_PTR_nanoSecondsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiTodUpdateCounterGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_OUT_GT_U32_PTR_valuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiPClkInterfaceSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_IN_CPSS_DIRECTION_ENT_clockInterfaceDirection
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiInputClockSelectSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_IN_CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT_clockSelect,
    &DX_IN_CPSS_DXCH_PTP_TAI_CLOCK_FREQ_ENT_clockFrequency
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiPClkOutputInterfaceSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_IN_CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT_pclkRcvrClkMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiPulseInModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_IN_CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT_pulseMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiPClkInterfaceGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_OUT_CPSS_DIRECTION_ENT_PTR_clockInterfaceDirectionPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiInputClockSelectGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_OUT_CPSS_DXCH_PTP_TAI_CLOCK_SELECT_ENT_PTR_clockSelectPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiPulseInModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_OUT_CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT_PTR_pulseModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiInputInterfaceSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT_pinId,
    &DX_IN_CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT_interfaceType,
    &DX_IN_GT_TAI_SELECT_BMP_taiSelectBmp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiOutputInterfaceSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT_pinId,
    &DX_IN_CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT_interfaceType,
    &DX_IN_GT_U32_taiSelect
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiInputInterfaceGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT_pinId,
    &DX_IN_CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT_interfaceType,
    &DX_OUT_GT_TAI_SELECT_BMP_PTR_taiSelectBmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiOutputInterfaceGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_PIN_SELECT_ENT_pinId,
    &DX_IN_CPSS_DXCH_PTP_TAI_INTERFACE_TYPE_ENT_interfaceType,
    &DX_OUT_GT_U32_PTR_taiSelectPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsTagGlobalCfgSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC_PTR_tsTagGlobalCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsUdpChecksumUpdateModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC_PTR_udpCsUpdModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTodCounterFunctionTriggerSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_PORT_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTodCounterFunctionSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_PORT_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_IN_CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT_function
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTodCounterFunctionGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_PORT_DIRECTION_ENT_direction,
    &DX_IN_CPSS_DXCH_PTP_TAI_ID_STC_PTR_taiIdPtr,
    &DX_OUT_CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT_PTR_functionPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsuBypassEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_BOOL_tsuBypassEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiTodStepDbgSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_IN_CPSS_DXCH_PTP_TAI_TOD_STEP_STC_PTR_todStepPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiSerRxFrameLatencyDbgGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_NUMBER_ENT_taiNumber,
    &DX_OUT_GT_U32_PTR_nanoSecondsPtr,
    &DX_OUT_GT_U32_PTR_fracNanoSecondsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiSelectDbgSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_PTP_TAI_SELECT_UNIT_ENT_taiUnit,
    &DX_IN_GT_U32_taiSelect
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsuControlSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_PTP_TSU_CONTROL_STC_PTR_controlPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsuPacketCounterGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_PTP_TSU_PACKET_COUNTER_TYPE_ENT_counterType,
    &DX_IN_GT_U32_index,
    &DX_OUT_GT_U32_PTR_valuePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT_egrTimeCorrTaiSelMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsDelayEgressPipeDelaySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC_PTR_egrDelayValPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsDelayIngressPortDelaySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_32_ingressDelayCorr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_BOOL_egrCorrFldPBEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_BOOL_ingrCorrFldPBEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpPortUnitResetSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_BOOL_resetTxUnit,
    &DX_IN_GT_BOOL_resetRxUnit
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsuEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_BOOL_tsuEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpPortTxPipeStatusDelaySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_delay
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpEgressDomainTableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_domainIndex,
    &DX_IN_CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpMessageTypeCmdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_domainIndex,
    &DX_IN_GT_U32_messageType,
    &DX_IN_CPSS_PACKET_CMD_ENT_command
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpMessageTypeCmdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_domainIndex,
    &DX_IN_GT_U32_messageType,
    &DX_OUT_CPSS_PACKET_CMD_ENT_PTR_commandPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpEgressDomainTableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_domainIndex,
    &DX_OUT_CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsDelayEgressAsymmetryCorrectionSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_domainProfile,
    &DX_IN_GT_32_egrAsymmetryCorr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsDelayIngressLinkDelaySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_domainProfile,
    &DX_IN_GT_32_ingressDelay
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsLocalActionTableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_domainProfile,
    &DX_IN_GT_U32_messageType,
    &DX_IN_CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsLocalActionTableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_domainProfile,
    &DX_IN_GT_U32_messageType,
    &DX_OUT_CPSS_DXCH_PTP_TS_LOCAL_ACTION_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsDelayEgressAsymmetryCorrectionGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_domainProfile,
    &DX_OUT_GT_32_PTR_egrAsymmetryCorrPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsDelayIngressLinkDelayGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_domainProfile,
    &DX_OUT_GT_32_PTR_ingressDelayPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsDelayEgressTimestampTaiSelectSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_egrTsTaiNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsuTSFrameCounterControlSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC_PTR_frameCounterConfigPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsuTimeStampMaskingProfileSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC_PTR_maskingProfilePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsuTsFrameCounterIndexSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_index,
    &DX_IN_GT_U32_frameCounter
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsuTSFrameCounterControlGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_CPSS_DXCH_PTP_TSU_FRAME_COUNTER_CFG_ENTRY_STC_PTR_frameCounterConfigPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsuTimeStampMaskingProfileGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC_PTR_maskingProfilePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsuTxTimestampQueueRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_queueNum,
    &DX_OUT_CPSS_DXCH_PTP_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpRxTaiSelectSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_rxTaiSelect
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsuControlGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_PTP_TSU_CONTROL_STC_PTR_controlPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT_PTR_egrTimeCorrTaiSelModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsDelayEgressPipeDelayGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_PTP_TS_EGRESS_PIPE_DELAY_VALUES_STC_PTR_egrDelayValPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsTagPortCfgGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_PTP_TS_TAG_PORT_CFG_STC_PTR_tsTagPortCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsDelayIngressPortDelayGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_32_PTR_ingressDelayCorrPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_BOOL_PTR_egrCorrFldPBEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_BOOL_PTR_ingrCorrFldPBEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpPortUnitResetGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_BOOL_PTR_resetTxUnitPtr,
    &DX_OUT_GT_BOOL_PTR_resetRxUnitPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsuEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_BOOL_PTR_tsuEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpPortTxPipeStatusDelayGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_U32_PTR_delayPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsDelayEgressTimestampTaiSelectGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_U32_PTR_egrTsTaiNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsuNtpTimeOffsetGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_U32_PTR_ntpTimeOffsetPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpRxTaiSelectGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_U32_PTR_rxTaiSelectPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiTodCounterFunctionGroupTriggerSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_TAI_SELECT_BMP_taiSelectBmp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpDomainModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_domainIndex,
    &DX_IN_CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT_domainMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpDomainV2IdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_domainIndex,
    &DX_IN_GT_U32_domainId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpIngressPacketCheckingModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_domainIndex,
    &DX_IN_GT_U32_messageType,
    &DX_IN_CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT_checkingMode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpIngressPacketCheckingModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_domainIndex,
    &DX_IN_GT_U32_messageType,
    &DX_OUT_CPSS_DXCH_PTP_INGRESS_CHECKING_MODE_ENT_PTR_checkingModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpDomainModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_domainIndex,
    &DX_OUT_CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_ENT_PTR_domainModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpDomainV2IdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_domainIndex,
    &DX_OUT_GT_U32_PTR_domainIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsCfgTableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_entryIndex,
    &DX_IN_CPSS_DXCH_PTP_TS_CFG_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsCfgTableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_entryIndex,
    &DX_OUT_CPSS_DXCH_PTP_TS_CFG_ENTRY_STC_PTR_entryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsMessageTypeToQueueIdMapSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_idMapBmp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsNtpTimeOffsetSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_ntpTimeOffset
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsEgressTimestampQueueEntryRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_queueNum,
    &DX_OUT_CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC_PTR_tsQueueEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsIngressTimestampQueueEntryRead_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_queueNum,
    &DX_OUT_CPSS_DXCH_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC_PTR_tsQueueEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsQueuesSizeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_queueSize
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpNonPtpPacketTaiSelectSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_taiSelect
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpUdpDestPortsSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_udpPortIndex,
    &DX_IN_GT_U32_udpPortNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpUdpDestPortsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_udpPortIndex,
    &DX_OUT_GT_U32_PTR_udpPortNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpEtherTypeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_HEX_etherTypeIndex,
    &DX_IN_GT_U32_HEX_etherType
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpEtherTypeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_HEX_etherTypeIndex,
    &DX_OUT_GT_U32_HEX_PTR_etherTypePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiPtpPulseInterfaceGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DIRECTION_ENT_PTR_pulseInterfaceDirectionPtr,
    &DX_OUT_CPSS_DXCH_PTP_TAI_NUMBER_ENT_PTR_taiNumberPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpEgressExceptionCfgGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_PTP_EGRESS_EXCEPTION_CFG_STC_PTR_egrExceptionCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpEgressExceptionCountersGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_PTP_EGRESS_EXCEPTION_COUNTERS_STC_PTR_egrExceptionCntPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpIngressExceptionCfgGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_PTP_INGRESS_EXCEPTION_CFG_STC_PTR_ingrExceptionCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpRefClockSourceSelectGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_PTP_REF_CLOCK_SELECT_ENT_PTR_refClockSelectPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiPClkOutputInterfaceGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_PTP_TAI_NUMBER_ENT_PTR_taiNumberPtr,
    &DX_OUT_CPSS_DXCH_PTP_TAI_PCLK_RCVR_CLK_MODE_ENT_PTR_pclkRcvrClkModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsTagGlobalCfgGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC_PTR_tsTagGlobalCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsUdpChecksumUpdateModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_STC_PTR_udpCsUpdModePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsuBypassEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_BOOL_PTR_tsuBypassEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTaiTodCounterFunctionGroupTriggerGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_TAI_SELECT_BMP_PTR_taiSelectBmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsMessageTypeToQueueIdMapGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_idMapBmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsNtpTimeOffsetGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_ntpTimeOffsetPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsDebugQueuesEntryIdsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_queueEntryId0Ptr,
    &DX_OUT_GT_U32_PTR_queueEntryId1Ptr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpTsQueuesSizeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_queueSizePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpNonPtpPacketTaiSelectGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_taiSelectPtr
};
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpDomainV1IdGet_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPtpDomainV1IdSet_PARAMS[];


/********* lib API DB *********/

extern void cpssDxChPtpDomainV1IdSet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChPtpDomainV1IdSet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChPtpLogLibDb[] = {
    {"cpssDxChPtpEtherTypeSet", 3, cpssDxChPtpEtherTypeSet_PARAMS, NULL},
    {"cpssDxChPtpEtherTypeGet", 3, cpssDxChPtpEtherTypeGet_PARAMS, NULL},
    {"cpssDxChPtpUdpDestPortsSet", 3, cpssDxChPtpUdpDestPortsSet_PARAMS, NULL},
    {"cpssDxChPtpUdpDestPortsGet", 3, cpssDxChPtpUdpDestPortsGet_PARAMS, NULL},
    {"cpssDxChPtpMessageTypeCmdSet", 5, cpssDxChPtpMessageTypeCmdSet_PARAMS, NULL},
    {"cpssDxChPtpMessageTypeCmdGet", 5, cpssDxChPtpMessageTypeCmdGet_PARAMS, NULL},
    {"cpssDxChPtpCpuCodeBaseSet", 2, prvCpssLogGenDevNumCpuCode_PARAMS, NULL},
    {"cpssDxChPtpCpuCodeBaseGet", 2, prvCpssLogGenDevNumCpuCodePtr_PARAMS, NULL},
    {"cpssDxChPtpTodCounterFunctionSet", 4, cpssDxChPtpTodCounterFunctionSet_PARAMS, NULL},
    {"cpssDxChPtpTodCounterFunctionGet", 4, cpssDxChPtpTodCounterFunctionGet_PARAMS, NULL},
    {"cpssDxChPtpTodCounterFunctionTriggerSet", 3, cpssDxChPtpTodCounterFunctionTriggerSet_PARAMS, NULL},
    {"cpssDxChPtpTodCounterFunctionTriggerStatusGet", 3, cpssDxChPtpTodCounterFunctionTriggerStatusGet_PARAMS, NULL},
    {"cpssDxChPtpTaiClockModeSet", 3, cpssDxChPtpTaiClockModeSet_PARAMS, NULL},
    {"cpssDxChPtpTaiClockModeGet", 3, cpssDxChPtpTaiClockModeGet_PARAMS, NULL},
    {"cpssDxChPtpTaiInternalClockGenerateEnableSet", 3, cpssDxChPtpTaiInternalClockGenerateEnableSet_PARAMS, NULL},
    {"cpssDxChPtpTaiInternalClockGenerateEnableGet", 3, cpssDxChPtpTaiInternalClockGenerateEnableGet_PARAMS, NULL},
    {"cpssDxChPtpTaiPClkDriftAdjustEnableSet", 3, cpssDxChPtpTaiInternalClockGenerateEnableSet_PARAMS, NULL},
    {"cpssDxChPtpTaiPClkDriftAdjustEnableGet", 3, cpssDxChPtpTaiInternalClockGenerateEnableGet_PARAMS, NULL},
    {"cpssDxChPtpTaiCaptureOverrideEnableSet", 3, cpssDxChPtpTaiInternalClockGenerateEnableSet_PARAMS, NULL},
    {"cpssDxChPtpTaiCaptureOverrideEnableGet", 3, cpssDxChPtpTaiInternalClockGenerateEnableGet_PARAMS, NULL},
    {"cpssDxChPtpTaiInputTriggersCountEnableSet", 3, cpssDxChPtpTaiInternalClockGenerateEnableSet_PARAMS, NULL},
    {"cpssDxChPtpTaiInputTriggersCountEnableGet", 3, cpssDxChPtpTaiInternalClockGenerateEnableGet_PARAMS, NULL},
    {"cpssDxChPtpTaiExternalPulseWidthSet", 3, cpssDxChPtpTaiExternalPulseWidthSet_PARAMS, NULL},
    {"cpssDxChPtpTaiExternalPulseWidthGet", 3, cpssDxChPtpTaiExternalPulseWidthGet_PARAMS, NULL},
    {"cpssDxChPtpTaiTodSet", 4, cpssDxChPtpTaiTodSet_PARAMS, NULL},
    {"cpssDxChPtpTaiTodGet", 4, cpssDxChPtpTaiTodGet_PARAMS, NULL},
    {"cpssDxChPtpTaiOutputTriggerEnableSet", 3, cpssDxChPtpTaiInternalClockGenerateEnableSet_PARAMS, NULL},
    {"cpssDxChPtpTaiOutputTriggerEnableGet", 3, cpssDxChPtpTaiInternalClockGenerateEnableGet_PARAMS, NULL},
    {"cpssDxChPtpTaiTodStepSet", 3, cpssDxChPtpTaiTodStepSet_PARAMS, NULL},
    {"cpssDxChPtpTaiTodStepGet", 3, cpssDxChPtpTaiTodStepGet_PARAMS, NULL},
    {"cpssDxChPtpTaiFractionalNanosecondDriftSet", 3, cpssDxChPtpTaiFractionalNanosecondDriftSet_PARAMS, NULL},
    {"cpssDxChPtpTaiFractionalNanosecondDriftGet", 3, cpssDxChPtpTaiFractionalNanosecondDriftGet_PARAMS, NULL},
    {"cpssDxChPtpTaiPClkCycleSet", 3, cpssDxChPtpTaiPClkCycleSet_PARAMS, NULL},
    {"cpssDxChPtpTaiPClkCycleGet", 3, cpssDxChPtpTaiPClkCycleGet_PARAMS, NULL},
    {"cpssDxChPtpTaiClockCycleSet", 4, cpssDxChPtpTaiClockCycleSet_PARAMS, NULL},
    {"cpssDxChPtpTaiClockCycleGet", 4, cpssDxChPtpTaiClockCycleGet_PARAMS, NULL},
    {"cpssDxChPtpTaiTodCaptureStatusGet", 4, cpssDxChPtpTaiTodCaptureStatusGet_PARAMS, NULL},
    {"cpssDxChPtpTaiTodUpdateCounterGet", 3, cpssDxChPtpTaiTodUpdateCounterGet_PARAMS, NULL},
    {"cpssDxChPtpTaiIncomingTriggerCounterSet", 3, cpssDxChPtpTaiIncomingTriggerCounterSet_PARAMS, NULL},
    {"cpssDxChPtpTaiIncomingTriggerCounterGet", 3, cpssDxChPtpTaiTodUpdateCounterGet_PARAMS, NULL},
    {"cpssDxChPtpTaiIncomingClockCounterSet", 4, cpssDxChPtpTaiIncomingClockCounterSet_PARAMS, NULL},
    {"cpssDxChPtpTaiIncomingClockCounterGet", 4, cpssDxChPtpTaiIncomingClockCounterGet_PARAMS, NULL},
    {"cpssDxChPtpTaiGracefulStepSet", 3, cpssDxChPtpTaiGracefulStepSet_PARAMS, NULL},
    {"cpssDxChPtpTaiGracefulStepGet", 3, cpssDxChPtpTaiGracefulStepGet_PARAMS, NULL},
    {"cpssDxChPtpRxTaiSelectSet", 3, cpssDxChPtpRxTaiSelectSet_PARAMS, NULL},
    {"cpssDxChPtpRxTaiSelectGet", 3, cpssDxChPtpRxTaiSelectGet_PARAMS, NULL},
    {"cpssDxChPtpTsuBypassEnableSet", 2, cpssDxChPtpTsuBypassEnableSet_PARAMS, NULL},
    {"cpssDxChPtpTsuBypassEnableGet", 2, cpssDxChPtpTsuBypassEnableGet_PARAMS, NULL},
    {"cpssDxChPtpTsuEnableSet", 3, cpssDxChPtpTsuEnableSet_PARAMS, NULL},
    {"cpssDxChPtpTsuEnableGet", 3, cpssDxChPtpTsuEnableGet_PARAMS, NULL},
    {"cpssDxChPtpTsuControlSet", 3, cpssDxChPtpTsuControlSet_PARAMS, NULL},
    {"cpssDxChPtpTsuControlGet", 3, cpssDxChPtpTsuControlGet_PARAMS, NULL},
    {"cpssDxChPtpTsuTxTimestampQueueRead", 4, cpssDxChPtpTsuTxTimestampQueueRead_PARAMS, NULL},
    {"cpssDxChPtpTsuCountersClear", 2, prvCpssLogGenDevNumPortNum_PARAMS, NULL},
    {"cpssDxChPtpTsuPacketCounterGet", 5, cpssDxChPtpTsuPacketCounterGet_PARAMS, NULL},
    {"cpssDxChPtpTsuNtpTimeOffsetGet", 3, cpssDxChPtpTsuNtpTimeOffsetGet_PARAMS, NULL},
    {"cpssDxChPtpTsTagGlobalCfgSet", 2, cpssDxChPtpTsTagGlobalCfgSet_PARAMS, NULL},
    {"cpssDxChPtpTsTagGlobalCfgGet", 2, cpssDxChPtpTsTagGlobalCfgGet_PARAMS, NULL},
    {"cpssDxChPtpTsTagPortCfgSet", 3, prvCpssLogGenDevNumPortNumTsTagPortCfgPtr_PARAMS, NULL},
    {"cpssDxChPtpTsTagPortCfgGet", 3, cpssDxChPtpTsTagPortCfgGet_PARAMS, NULL},
    {"cpssDxChPtpTsDelayIngressPortDelaySet", 3, cpssDxChPtpTsDelayIngressPortDelaySet_PARAMS, NULL},
    {"cpssDxChPtpTsDelayIngressPortDelayGet", 3, cpssDxChPtpTsDelayIngressPortDelayGet_PARAMS, NULL},
    {"cpssDxChPtpTsDelayIngressLinkDelaySet", 4, cpssDxChPtpTsDelayIngressLinkDelaySet_PARAMS, NULL},
    {"cpssDxChPtpTsDelayIngressLinkDelayGet", 4, cpssDxChPtpTsDelayIngressLinkDelayGet_PARAMS, NULL},
    {"cpssDxChPtpTsDelayEgressPipeDelaySet", 3, cpssDxChPtpTsDelayEgressPipeDelaySet_PARAMS, NULL},
    {"cpssDxChPtpTsDelayEgressPipeDelayGet", 3, cpssDxChPtpTsDelayEgressPipeDelayGet_PARAMS, NULL},
    {"cpssDxChPtpTsDelayEgressAsymmetryCorrectionSet", 4, cpssDxChPtpTsDelayEgressAsymmetryCorrectionSet_PARAMS, NULL},
    {"cpssDxChPtpTsDelayEgressAsymmetryCorrectionGet", 4, cpssDxChPtpTsDelayEgressAsymmetryCorrectionGet_PARAMS, NULL},
    {"cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableSet", 3, cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableSet_PARAMS, NULL},
    {"cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableGet", 3, cpssDxChPtpTsDelayIngressCorrFieldPiggybackEnableGet_PARAMS, NULL},
    {"cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableSet", 3, cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableSet_PARAMS, NULL},
    {"cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableGet", 3, cpssDxChPtpTsDelayEgressCorrFieldPiggybackEnableGet_PARAMS, NULL},
    {"cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeSet", 3, cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeSet_PARAMS, NULL},
    {"cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeGet", 3, cpssDxChPtpTsDelayEgressTimeCorrTaiSelectModeGet_PARAMS, NULL},
    {"cpssDxChPtpTsDelayEgressTimestampTaiSelectSet", 3, cpssDxChPtpTsDelayEgressTimestampTaiSelectSet_PARAMS, NULL},
    {"cpssDxChPtpTsDelayEgressTimestampTaiSelectGet", 3, cpssDxChPtpTsDelayEgressTimestampTaiSelectGet_PARAMS, NULL},
    {"cpssDxChPtpTsCfgTableSet", 3, cpssDxChPtpTsCfgTableSet_PARAMS, NULL},
    {"cpssDxChPtpTsCfgTableGet", 3, cpssDxChPtpTsCfgTableGet_PARAMS, NULL},
    {"cpssDxChPtpTsLocalActionTableSet", 5, cpssDxChPtpTsLocalActionTableSet_PARAMS, NULL},
    {"cpssDxChPtpTsLocalActionTableGet", 5, cpssDxChPtpTsLocalActionTableGet_PARAMS, NULL},
    {"cpssDxChPtpTsUdpChecksumUpdateModeSet", 2, cpssDxChPtpTsUdpChecksumUpdateModeSet_PARAMS, NULL},
    {"cpssDxChPtpTsUdpChecksumUpdateModeGet", 2, cpssDxChPtpTsUdpChecksumUpdateModeGet_PARAMS, NULL},
    {"cpssDxChPtpOverEthernetEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChPtpOverEthernetEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChPtpOverUdpEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChPtpOverUdpEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChPtpDomainModeSet", 3, cpssDxChPtpDomainModeSet_PARAMS, NULL},
    {"cpssDxChPtpDomainModeGet", 3, cpssDxChPtpDomainModeGet_PARAMS, NULL},
    {"cpssDxChPtpDomainV1IdSet", 3, cpssDxChPtpDomainV1IdSet_PARAMS, cpssDxChPtpDomainV1IdSet_preLogic},
    {"cpssDxChPtpDomainV1IdGet", 3, cpssDxChPtpDomainV1IdGet_PARAMS, cpssDxChPtpDomainV1IdSet_preLogic},
    {"cpssDxChPtpDomainV2IdSet", 3, cpssDxChPtpDomainV2IdSet_PARAMS, NULL},
    {"cpssDxChPtpDomainV2IdGet", 3, cpssDxChPtpDomainV2IdGet_PARAMS, NULL},
    {"cpssDxChPtpEgressDomainTableSet", 4, cpssDxChPtpEgressDomainTableSet_PARAMS, NULL},
    {"cpssDxChPtpEgressDomainTableGet", 4, cpssDxChPtpEgressDomainTableGet_PARAMS, NULL},
    {"cpssDxChPtpIngressExceptionCfgSet", 2, cpssDxChPtpIngressExceptionCfgSet_PARAMS, NULL},
    {"cpssDxChPtpIngressExceptionCfgGet", 2, cpssDxChPtpIngressExceptionCfgGet_PARAMS, NULL},
    {"cpssDxChPtpIngressPacketCheckingModeSet", 4, cpssDxChPtpIngressPacketCheckingModeSet_PARAMS, NULL},
    {"cpssDxChPtpIngressPacketCheckingModeGet", 4, cpssDxChPtpIngressPacketCheckingModeGet_PARAMS, NULL},
    {"cpssDxChPtpIngressExceptionCounterGet", 2, prvCpssLogGenDevNumCounterPtr_PARAMS, NULL},
    {"cpssDxChPtpEgressExceptionCfgSet", 2, cpssDxChPtpEgressExceptionCfgSet_PARAMS, NULL},
    {"cpssDxChPtpEgressExceptionCfgGet", 2, cpssDxChPtpEgressExceptionCfgGet_PARAMS, NULL},
    {"cpssDxChPtpEgressExceptionCountersGet", 2, cpssDxChPtpEgressExceptionCountersGet_PARAMS, NULL},
    {"cpssDxChPtpTsIngressTimestampQueueEntryRead", 3, cpssDxChPtpTsIngressTimestampQueueEntryRead_PARAMS, NULL},
    {"cpssDxChPtpTsEgressTimestampQueueEntryRead", 3, cpssDxChPtpTsEgressTimestampQueueEntryRead_PARAMS, NULL},
    {"cpssDxChPtpTsMessageTypeToQueueIdMapSet", 2, cpssDxChPtpTsMessageTypeToQueueIdMapSet_PARAMS, NULL},
    {"cpssDxChPtpTsMessageTypeToQueueIdMapGet", 2, cpssDxChPtpTsMessageTypeToQueueIdMapGet_PARAMS, NULL},
    {"cpssDxChPtpTsQueuesSizeSet", 2, cpssDxChPtpTsQueuesSizeSet_PARAMS, NULL},
    {"cpssDxChPtpTsQueuesSizeGet", 2, cpssDxChPtpTsQueuesSizeGet_PARAMS, NULL},
    {"cpssDxChPtpTsDebugQueuesEntryIdsClear", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssDxChPtpTsDebugQueuesEntryIdsGet", 3, cpssDxChPtpTsDebugQueuesEntryIdsGet_PARAMS, NULL},
    {"cpssDxChPtpTsNtpTimeOffsetSet", 2, cpssDxChPtpTsNtpTimeOffsetSet_PARAMS, NULL},
    {"cpssDxChPtpTsNtpTimeOffsetGet", 2, cpssDxChPtpTsNtpTimeOffsetGet_PARAMS, NULL},
    {"cpssDxChPtpTaiPulseInModeSet", 3, cpssDxChPtpTaiPulseInModeSet_PARAMS, NULL},
    {"cpssDxChPtpTaiPulseInModeGet", 3, cpssDxChPtpTaiPulseInModeGet_PARAMS, NULL},
    {"cpssDxChPtpTaiPtpPulseInterfaceSet", 3, cpssDxChPtpTaiPtpPulseInterfaceSet_PARAMS, NULL},
    {"cpssDxChPtpTaiPtpPulseInterfaceGet", 3, cpssDxChPtpTaiPtpPulseInterfaceGet_PARAMS, NULL},
    {"cpssDxChPtpTaiPClkInterfaceSet", 3, cpssDxChPtpTaiPClkInterfaceSet_PARAMS, NULL},
    {"cpssDxChPtpTaiPClkInterfaceGet", 3, cpssDxChPtpTaiPClkInterfaceGet_PARAMS, NULL},
    {"cpssDxChPtpTaiPClkOutputInterfaceSet", 3, cpssDxChPtpTaiPClkOutputInterfaceSet_PARAMS, NULL},
    {"cpssDxChPtpTaiPClkOutputInterfaceGet", 3, cpssDxChPtpTaiPClkOutputInterfaceGet_PARAMS, NULL},
    {"cpssDxChPtpTaiInputClockSelectSet", 4, cpssDxChPtpTaiInputClockSelectSet_PARAMS, NULL},
    {"cpssDxChPtpTaiInputClockSelectGet", 3, cpssDxChPtpTaiInputClockSelectGet_PARAMS, NULL},
    {"cpssDxChPtpTaiTodCounterFunctionAllTriggerSet", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssDxChPtpPortUnitResetSet", 4, cpssDxChPtpPortUnitResetSet_PARAMS, NULL},
    {"cpssDxChPtpPortUnitResetGet", 4, cpssDxChPtpPortUnitResetGet_PARAMS, NULL},
    {"cpssDxChPtpPortTxPipeStatusDelaySet", 3, cpssDxChPtpPortTxPipeStatusDelaySet_PARAMS, NULL},
    {"cpssDxChPtpPortTxPipeStatusDelayGet", 3, cpssDxChPtpPortTxPipeStatusDelayGet_PARAMS, NULL},
    {"cpssDxChPtpTaiCapturePreviousModeEnableSet", 3, cpssDxChPtpTaiInternalClockGenerateEnableSet_PARAMS, NULL},
    {"cpssDxChPtpTaiCapturePreviousModeEnableGet", 3, cpssDxChPtpTaiInternalClockGenerateEnableGet_PARAMS, NULL},
    {"cpssDxChPtpTaiPClkSelectionModeSet", 3, cpssDxChPtpTaiPClkSelectionModeSet_PARAMS, NULL},
    {"cpssDxChPtpTaiPClkSelectionModeGet", 3, cpssDxChPtpTaiPClkSelectionModeGet_PARAMS, NULL},
    {"cpssDxChPtpTaiClockGenerationModeSet", 3, cpssDxChPtpTaiClockGenerationModeSet_PARAMS, NULL},
    {"cpssDxChPtpTaiClockGenerationModeGet", 3, cpssDxChPtpTaiClockGenerationModeGet_PARAMS, NULL},
    {"cpssDxChPtpTaiClockReceptionModeSet", 3, cpssDxChPtpTaiClockReceptionModeSet_PARAMS, NULL},
    {"cpssDxChPtpTaiClockReceptionModeGet", 3, cpssDxChPtpTaiClockReceptionModeGet_PARAMS, NULL},
    {"cpssDxChPtpTaiClockGenerationEdgeSet", 3, cpssDxChPtpTaiClockGenerationEdgeSet_PARAMS, NULL},
    {"cpssDxChPtpTaiClockGenerationEdgeGet", 3, cpssDxChPtpTaiClockGenerationEdgeGet_PARAMS, NULL},
    {"cpssDxChPtpTaiClockReceptionEdgeSet", 3, cpssDxChPtpTaiClockGenerationEdgeSet_PARAMS, NULL},
    {"cpssDxChPtpTaiClockReceptionEdgeGet", 3, cpssDxChPtpTaiClockGenerationEdgeGet_PARAMS, NULL},
    {"cpssDxChPtpTsuTimeStampMaskingProfileSet", 4, cpssDxChPtpTsuTimeStampMaskingProfileSet_PARAMS, NULL},
    {"cpssDxChPtpTsuTimeStampMaskingProfileGet", 4, cpssDxChPtpTsuTimeStampMaskingProfileGet_PARAMS, NULL},
    {"cpssDxChPtpTsuTSFrameCounterControlSet", 4, cpssDxChPtpTsuTSFrameCounterControlSet_PARAMS, NULL},
    {"cpssDxChPtpTsuTSFrameCounterControlGet", 4, cpssDxChPtpTsuTSFrameCounterControlGet_PARAMS, NULL},
    {"cpssDxChPtpTsuTsFrameCounterIndexSet", 4, cpssDxChPtpTsuTsFrameCounterIndexSet_PARAMS, NULL},
    {"cpssDxChPtpUBitModeSet", 3, cpssDxChPtpUBitModeSet_PARAMS, NULL},
    {"cpssDxChPtpUBitModeGet", 3, cpssDxChPtpUBitModeGet_PARAMS, NULL},
    {"cpssDxChPtpPLLBypassEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChPtpPLLBypassEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChPtpRefClockSourceSelectSet", 2, cpssDxChPtpRefClockSourceSelectSet_PARAMS, NULL},
    {"cpssDxChPtpRefClockSourceSelectGet", 2, cpssDxChPtpRefClockSourceSelectGet_PARAMS, NULL},
    {"cpssDxChPtpNonPtpPacketTaiSelectSet", 2, cpssDxChPtpNonPtpPacketTaiSelectSet_PARAMS, NULL},
    {"cpssDxChPtpNonPtpPacketTaiSelectGet", 2, cpssDxChPtpNonPtpPacketTaiSelectGet_PARAMS, NULL},
    {"cpssDxChPtpTaiOutputInterfaceSet", 4, cpssDxChPtpTaiOutputInterfaceSet_PARAMS, NULL},
    {"cpssDxChPtpTaiOutputInterfaceGet", 4, cpssDxChPtpTaiOutputInterfaceGet_PARAMS, NULL},
    {"cpssDxChPtpTaiInputInterfaceSet", 4, cpssDxChPtpTaiInputInterfaceSet_PARAMS, NULL},
    {"cpssDxChPtpTaiInputInterfaceGet", 4, cpssDxChPtpTaiInputInterfaceGet_PARAMS, NULL},
    {"cpssDxChPtpTaiTodCounterFunctionGroupTriggerSet", 2, cpssDxChPtpTaiTodCounterFunctionGroupTriggerSet_PARAMS, NULL},
    {"cpssDxChPtpTaiTodCounterFunctionGroupTriggerGet", 2, cpssDxChPtpTaiTodCounterFunctionGroupTriggerGet_PARAMS, NULL},
    {"cpssDxChPtpTaiSerRxFrameLatencyDbgGet", 5, cpssDxChPtpTaiSerRxFrameLatencyDbgGet_PARAMS, NULL},
    {"cpssDxChPtpTaiTodStepDbgSet", 4, cpssDxChPtpTaiTodStepDbgSet_PARAMS, NULL},
    {"cpssDxChPtpTaiSelectDbgSet", 4, cpssDxChPtpTaiSelectDbgSet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_PTP(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChPtpLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChPtpLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

