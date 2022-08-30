/*************************************************************************
* /local/store/cpss_regression/tmp/all_common_dxpx_lua_c_type_wrappers.c
*
* DESCRIPTION:
*       A lua type wrapper
*       It implements support for the following types:
*           struct  GT_FCID
*           union   GT_IP_ADDR_TYPE_UNT
*           enum    MV_HWS_REF_CLOCK_SOURCE
*           enum    CPSS_LOG_TYPE_ENT
*           struct  MV_HWS_SERDES_CONFIG_STC
*           struct  MV_HWS_ANP_PORT_SM
*           struct  CPSS_HW_ADDR_STC
*           enum    MV_HWS_PPM_VALUE
*           enum    MV_HWS_CONNECTION_TYPE
*           enum    MV_HWS_MIF_REGISTERS
*           enum    CPSS_PORT_MANAGER_PARALLEL_DETECT_STATUS_ENT
*           struct  MV_HWS_SERDES_INFO_STC
*           struct  CPSS_802_1BR_ETAG_STC
*           struct  MV_HWS_FW_LOG
*           enum    CPSS_HW_PP_INIT_STAGE_ENT
*           struct  MV_HWS_IPC_PORT_INIT_DATA_STRUCT
*           struct  CPSS_PORT_MANAGER_SIGNAL_DETECT_CONFIG_STC
*           struct  MV_HWS_AVAGO_TXRX_TUNE_PARAMS
*           enum    CPSS_SERDES_TERMINATION
*           enum    MV_HWS_PORT_FEC_MODE
*           enum    CPSS_PORT_REF_CLOCK_SOURCE_ENT
*           struct  MV_HWS_COMPHY_C56G_AUTO_TUNE_RESULTS
*           enum    CPSS_DIAG_TEST_PROFILE_ENT
*           struct  MV_HWS_IPC_PORT_FEC_CONFIG_DATA_STRUCT
*           enum    CPSS_LED_CLOCK_OUT_FREQUENCY_ENT
*           struct  CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC
*           enum    MV_HWS_AVAGO_SERDES_HAL_FUNC_TYPES
*           enum    CPSS_FORMAT_CONVERT_FIELD_CONVERT_CMD_ENT
*           enum    CPSS_UNI_EV_CAUSE_ENT
*           enum    MV_HWS_PORT_TEST_GEN_ACTION
*           struct  MV_HWS_ANP_PORT_DATA_STC
*           struct  MV_HWS_CALIBRATION_RESULTS
*           struct  MV_HWS_AP_PORT_STATUS
*           struct  CPSS_PORT_MANAGER_SGMII_AUTO_NEGOTIATION_STC
*           enum    CPSS_PORT_MAC_MTI_BR_STAT_COUNTER_ENT
*           struct  CPSS_PORT_COMPHY_C28GP4_RX_CONFIG_STC
*           enum    CPSS_PORT_FLOW_CONTROL_ENT
*           struct  MV_HWS_IPC_PORT_PARMAS_DATA_STRUCT
*           struct  MV_HWS_MAN_TUNE_COMPHY_C12G_TX_CONFIG_DATA
*           struct  MV_HWS_MAN_TUNE_AVAGO_16NM_RX_CONFIG_DATA
*           enum    CPSS_LOG_LIB_ENT
*           struct  MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE
*           struct  CPSS_PORT_MANAGER_UPDATE_PARAMS_STC
*           struct  MV_HWS_SAMPLER_RESULTS
*           struct  MV_HWS_COMPHY_C112G_AUTO_TUNE_RESULTS
*           union   CPSS_PORT_SERDES_RX_CONFIG_UNT
*           struct  CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC
*           struct  MV_HWS_IPC_VOS_OVERRIDE_MODE_STRUCT
*           enum    CPSS_DRV_HW_ACCESS_STAGE_ENT
*           enum    CPSS_PORT_PCS_LOOPBACK_MODE_ENT
*           enum    CPSS_PORT_DIRECTION_ENT
*           struct  MV_HWS_D2D_FRAME_GEN_CONFIG_STC
*           struct  MV_HWS_TEST_GEN_STATUS
*           struct  CPSS_PORT_MANAGER_CALIBRATION_MODE_STC
*           struct  MV_HWS_RSFEC_STATUS_STC
*           enum    CPSS_PORT_RX_FC_PROFILE_SET_ENT
*           struct  MV_HWS_TX_TUNE_PARAMS
*           struct  CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC
*           enum    CPSS_PORT_MAC_COUNTERS_ENT
*           enum    CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT
*           enum    CPSS_PORT_MANAGER_FAILURE_ENT
*           struct  CPSS_PORT_AP_SERDES_RX_CONFIG_STC
*           struct  CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_STC
*           struct  MV_HWS_IPC_PORT_LOOPBACK_SET_DATA_STRUCT
*           enum    CPSS_LED_BLINK_SELECT_ENT
*           union   CPSS_PM_PORT_PARAMS_TYPE_UNT
*           enum    CPSS_PORT_DUPLEX_ENT
*           enum    CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT
*           struct  CPSS_ANP_INFO_STC
*           enum    CPSS_PM_MAC_PREEMPTION_TYPE_ENT
*           enum    MV_HWS_REF_CLOCK_SRC
*           enum    MV_HWS_DEV_TYPE
*           struct  CPSS_PORT_EGRESS_CNTR_STC
*           struct  CPSS_SYSTEM_RECOVERY_ERROR_INJECTION_STC
*           enum    MV_HWS_PORT_TEST_GEN_PATTERN
*           struct  MV_HWS_COMPHY_C12GP41P2V_AUTO_TUNE_RESULTS
*           enum    MV_HWS_PORT_AUTO_TUNE_MODE
*           enum    MV_HWS_SERDES_ENCODING_TYPE
*           union   MV_HWS_SERDES_RX_OVERRIDE_CONFIG_DATA_UNT
*           struct  MV_HWS_SERDES_RX_DATAPATH_CONFIG_STC
*           struct  MV_HWS_COMPHY_C28GP4_PRESET_OVERRIDE
*           struct  MV_HWS_AP_PORT_STATS
*           enum    CPSS_PORT_PA_BW_MODE_ENT
*           enum    CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT
*           struct  CPSS_FORMAT_CONVERT_FIELD_VAL_CONVERT_STC
*           enum    CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT
*           enum    CPSS_DIAG_PG_IF_SIZE_ENT
*           enum    MV_HWS_PORT_CRC_MODE
*           enum    CPSS_PORT_SPEED_ENT
*           enum    MV_HWS_PORT_LB_TYPE
*           enum    MV_HWS_CPLL_OUTPUT_FREQUENCY
*           enum    MV_HWS_D2D_DIRECTION_MODE_ENT
*           enum    MV_HWS_UNITS_ID
*           enum    MV_HWS_REF_CLOCK_SUP_VAL
*           union   MV_HWS_SERDES_PRESET_OVERRIDE_UNT
*           enum    CPSS_PHY_XSMI_INTERFACE_ENT
*           enum    MV_HWS_PORT_SPEED_ENT
*           struct  CPSS_PORT_MAC_MTI_RX_COUNTER_SET_STC
*           struct  CPSS_802_1Q_VLAN_TAG_STC
*           enum    MV_HWS_SERDES_MEDIA
*           struct  MV_HWS_MAN_TUNE_COMPHY_H_TX_CONFIG_DATA
*           enum    MV_HWS_AP_DEBUG_LOG_OUTPUT_ENT
*           struct  CPSS_CYCLIC_LOGGER_LOG_ENTRY
*           struct  MV_HWS_IPC_PORT_GENERAL_STRUCT
*           enum    CPSS_DP_FOR_RX_ENT
*           struct  MV_HWS_PORT_CTRL_LOG_ENTRY
*           struct  CPSS_PORT_AVAGO_SERDES_TX_CONFIG_STC
*           enum    CPSS_TX_SDMA_QUEUE_MODE_ENT
*           struct  CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC
*           enum    MV_HWS_SERDES_TEST_GEN_MODE
*           enum    CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_ENT
*           struct  CPSS_PORT_ATTRIBUTES_STC
*           enum    CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT
*           struct  MV_HWS_LOG_ENTRY
*           struct  CPSS_PORTS_BMP_STC
*           struct  CPSS_PM_AP_MODE_LANE_PARAM_STC
*           struct  MV_HWS_PORT_STATUS_STC
*           struct  MV_HWS_D2D_PRBS_CONFIG_STC
*           struct  CPSS_PORT_MAC_MTI_TX_COUNTER_SET_STC
*           struct  CPSS_APP_PLATFORM_BOARD_PROFILE_STC
*           struct  MV_HWS_MAN_TUNE_COMPHY_C28G_TX_CONFIG_DATA
*           enum    MV_HWS_SERDES_DUMP_TYPE_ENT
*           struct  MV_HWS_IPC_LINK_BINDING_STRUCT
*           enum    MV_HWS_PORT_MAN_TUNE_MODE
*           enum    CPSS_PP_INTERFACE_CHANNEL_ENT
*           struct  CPSS_PORT_SERDES_TX_CONFIG_STC
*           union   MV_HWS_SERDES_TX_OVERRIDE_CONFIG_DATA_UNT
*           struct  CPSS_TRUNK_MEMBER_STC
*           struct  MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS
*           struct  CPSS_PORT_AP_SERDES_TX_OFFSETS_STC
*           struct  CPSS_PORT_COMPHY_SERDES_RX_CONFIG_STC
*           enum    MV_HWS_PORT_PRESET_CMD
*           enum    MV_HWS_PORT_FLOW_CONTROL_ENT
*           enum    CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_ENT
*           enum    CPSS_EVENT_GPP_ID_ENT
*           struct  CPSS_LED_GROUP_CONF_STC
*           enum    CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_ENT
*           struct  MV_HWS_HCD_INFO
*           struct  CPSS_PM_MAC_PREEMPTION_PARAMS_STC
*           enum    CPSS_LED_PULSE_STRETCH_ENT
*           struct  CPSS_HW_DRIVER_SPLIT_ADDR_SPACE_RANGE_STC
*           enum    CPSS_DRV_HW_TRACE_TYPE_ENT
*           enum    CPSS_PP_FAMILY_BIT_ENT
*           enum    CPSS_PORT_AP_FLOW_CONTROL_ENT
*           enum    CPSS_PP_SUB_FAMILY_TYPE_ENT
*           enum    CPSS_HW_DRIVER_TYPE_ENT
*           struct  CPSS_PORT_MAC_CG_COUNTER_SET_STC
*           enum    CPSS_FORMAT_CONVERT_FIELD_CHECK_ENT
*           struct  MV_HWS_IPC_PORT_INFO_STRUCT
*           enum    CPSS_RX_BUFF_ALLOC_METHOD_ENT
*           struct  CPSS_PORT_COMPHY_C112G_TX_CONFIG_STC
*           struct  MV_HWS_SERDES_TUNING_PARAMS
*           enum    MV_HWS_PORT_ACTION
*           enum    CPSS_PORT_MAC_TYPE_ENT
*           struct  MV_HWS_IPC_PORT_LOOPBACK_GET_DATA_STRUCT
*           struct  MV_HWS_ETL_CONFIG_DATA
*           enum    CPSS_PORT_INTERFACE_MODE_ENT
*           struct  MV_HWS_ANP_INFO_STC
*           struct  MV_HWS_AP_PORT_INTROP
*           struct  CPSS_PORT_MAC_STATUS_STC
*           struct  CPSS_PORT_MANAGER_STATISTICS_STC
*           struct  MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC
*           enum    MV_HWS_PORT_MII_TYPE_E
*           struct  MV_HWS_PORT_SERDES_TO_MAC_MUX
*           enum    CPSS_HW_INFO_BUS_TYPE_ENT
*           struct  CPSS_PORT_MODE_SPEED_STC
*           enum    CPSS_PHY_SMI_INTERFACE_ENT
*           enum    MV_HWS_SERDES_TYPE
*           enum    MV_HWS_SERDES_TX_PATTERN
*           struct  CPSS_PORT_MAC_COUNTER_SET_STC
*           enum    MV_HWS_COM_PHY_H_SUB_SEQ
*           struct  CPSS_PORT_COMPHY_C28G_TX_CONFIG_STC
*           struct  MV_HWS_MAN_TUNE_COMPHY_H_RX_CONFIG_DATA
*           struct  MV_HWS_AVAGO_AUTO_TUNE_RESULTS
*           struct  MV_HWS_AVAGO_TXRX_OVERRIDE_CONFIG_DATA
*           enum    CPSS_PORT_SERDES_LOOPBACK_MODE_ENT
*           struct  CPSS_PORT_MANAGER_LKB_CONFIG_STC
*           enum    CPSS_NET_CPU_PORT_MODE_ENT
*           enum    CPSS_FORMAT_CONVERT_FIELD_TYPE_ENT
*           struct  MV_HWS_IPC_PORT_AP_INTROP_STRUCT
*           enum    CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_ENT
*           struct  MV_HWS_PORT_INIT_INPUT_PARAMS
*           enum    CPSS_SYSTEM_RECOVERY_HA_STATE_ENT
*           struct  MV_HWS_PER_SERDES_INFO_STC
*           struct  CPSS_SYSTEM_RECOVERY_INFO_STC
*           union   MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT
*           enum    CPSS_PORT_MANAGER_EVENT_ENT
*           enum    MV_HWS_UNIT_NUM
*           enum    CPSS_PM_MAC_PREEMPTION_METHOD_ENT
*           enum    CPSS_PORT_MANAGER_PORT_TYPE_ENT
*           enum    MV_HWS_TX_TRAINING_MODES
*           enum    CPSS_LED_ORDER_MODE_ENT
*           enum    CPSS_LED_BLINK_DURATION_ENT
*           enum    CPSS_PORT_XG_FIXED_IPG_ENT
*           enum    CPSS_SYS_HA_MODE_ENT
*           struct  MV_HWS_IPC_PORT_AP_DATA_STRUCT
*           struct  CPSS_PORT_COMPHY_C12G_TX_CONFIG_STC
*           struct  CPSS_SYSTEM_RECOVERY_MODE_STC
*           struct  CPSS_PORT_MANAGER_STC
*           enum    CPSS_FORMAT_CONVERT_FIELD_PTR_CMD_ENT
*           union   MV_HWS_SERDES_TX_CONFIG_DATA_UNT
*           struct  CPSS_FORMAT_CONVERT_FIELD_CONDITION_STC
*           enum    CPSS_DIAG_PG_PACKET_LENGTH_TYPE_ENT
*           enum    CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_ENT
*           struct  MV_HWS_ANP_PORT_COUNTERS
*           struct  MV_HWS_IPC_PORT_AUTO_TUNE_STOP_DATA_STRUCT
*           enum    CPSS_EVENT_MASK_SET_ENT
*           struct  MV_HWS_COMPHY_H_AUTO_TUNE_RESULTS
*           enum    CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT
*           enum    CPSS_DRV_HW_RESOURCE_TYPE_ENT
*           struct  CPSS_PORT_COMPHY_C56G_RX_CONFIG_STC
*           struct  MV_HWS_IPC_PORT_PPM_SET_DATA_STRUCT
*           enum    CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT
*           enum    CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_ENT
*           struct  CPSS_PM_MODE_LANE_PARAM_STC
*           enum    CPSS_PORT_PCS_RESET_MODE_ENT
*           enum    CPSS_INTERFACE_TYPE_ENT
*           enum    CPSS_UNICAST_MULTICAST_ENT
*           enum    CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT
*           struct  MV_HWS_IPC_PORT_SERDES_TX_OFFSETS_STC
*           struct  CPSS_PM_AP_PORT_ADV_STC
*           struct  MV_HWS_AUTO_TUNE_STATUS_RES
*           enum    MV_HWS_DFE_MODE
*           enum    CPSS_PORT_EGRESS_CNT_MODE_ENT
*           struct  CPSS_PORT_COMPHY_C12GP41P2V_RX_CONFIG_STC
*           enum    CPSS_PORT_FEC_MODE_ENT
*           struct  CPSS_PM_PORT_ATTR_STC
*           enum    CPSS_BM_POOL_ALIGNMENT_ENT
*           struct  CPSS_GEN_CFG_DEV_INFO_STC
*           enum    CPSS_TX_BUFF_ALLOC_METHOD_ENT
*           struct  MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_DATA_STRUCT
*           struct  MV_HWS_SERDES_TXRX_TUNE_PARAMS
*           enum    MV_HWS_INPUT_SRC_CLOCK
*           struct  MV_HWS_D2D_FRAME_CHECKER_CONFIG_STC
*           struct  MV_HWS_ENHANCE_TUNE_LITE_PHASE_PARAMS
*           enum    CPSS_LOG_POINTER_FORMAT_ENT
*           struct  MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA
*           struct  MV_HWS_D2D_FRAME_TABLE_CONFIG_STC
*           enum    MV_HWS_PORT_SERDES_OPERATION_MODE_ENT
*           struct  MV_HWS_COMPHY_REG_FIELD_STC
*           struct  MV_HWS_IPC_VOS_OVERRIDE_PARAMS
*           enum    CPSS_SYSTEM_RECOVERY_MANAGER_ENT
*           enum    CPSS_LOG_API_FORMAT_ENT
*           enum    MV_HWS_CPLL_INPUT_FREQUENCY
*           struct  MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA
*           struct  MV_HWS_ANP_MISC_PARAM_STC
*           enum    CPSS_PORT_SERDES_SPEED_ENT
*           struct  MV_HWS_IPC_PORT_RESET_DATA_STRUCT
*           enum    MV_HWS_RESET
*           struct  CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC
*           struct  CPSS_PORT_MANAGER_STATUS_STC
*           struct  MV_HWS_SERDES_TEST_GEN_STATUS
*           enum    MV_HWS_PORT_STANDARD
*           enum    MV_HWS_SERDES_BUS_WIDTH_ENT
*           enum    MV_HWS_RX_TRAINING_MODES
*           struct  MV_HWS_AP_CFG
*           enum    CPSS_PORT_MANAGER_AN_RESOLUTION_BIT_ENT
*           struct  CPSS_PORT_SERDES_RX_CONFIG_STC
*           enum    MV_HWS_AVAGO_SERDES_HAL_TYPES
*           struct  MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA
*           enum    CPSS_HW_PP_RESET_SKIP_TYPE_ENT
*           enum    CPSS_DIAG_PP_REG_TYPE_ENT
*           enum    CPSS_PORT_MANAGER_STATE_ENT
*           struct  MV_HWS_AVAGO_16NM_TXRX_TUNE_PARAMS
*           enum    CPSS_SYSTEM_RECOVERY_STATE_ENT
*           enum    CPSS_MEMORY_DUMP_TYPE_ENT
*           enum    MV_HWS_REF_CLOCK
*           struct  MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC
*           struct  MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_RX_CONFIG_DATA
*           struct  MV_HWS_MAN_TUNE_COMPHY_C28GP4_RX_CONFIG_DATA
*           enum    CPSS_PORT_SERDES_TYPE_ENT
*           enum    CPSS_SYSTEM_RECOVERY_PROCESS_ENT
*           struct  MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA
*           enum    CPSS_PORT_XGMII_MODE_ENT
*           struct  GT_VERSION
*           enum    MV_HWS_CPLL_NUMBER
*           enum    MV_HWS_SERDES_DIRECTION
*           struct  MV_HWS_COMPHY_C28GP4_AUTO_TUNE_RESULTS
*           struct  MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_DATA_STRUCT
*           struct  CPSS_PM_ET_OVERRIDE_CFG_STC
*           enum    CPSS_DIAG_PG_TRANSMIT_MODE_ENT
*           struct  CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC
*           enum    MV_HWS_PORT_MAC_TYPE
*           enum    CPSS_PORT_MANAGER_PROGRESS_TYPE_ENT
*           struct  MV_HWS_FCFEC_COUNTERS_STC
*           enum    CPSS_GEN_SIP_ENT
*           struct  CPSS_PORT_AP_STATUS_STC
*           struct  MV_HWS_ELEMENTS_ARRAY
*           enum    CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT
*           enum    CPSS_LED_BLINK_DUTY_CYCLE_ENT
*           enum    CPSS_PP_FAMILY_TYPE_ENT
*           enum    CPSS_PORT_INTERLAKEN_COUNTER_ID_ENT
*           struct  MV_HWS_COMPHY_SERDES_DATA
*           enum    CPSS_DP_LEVEL_ENT
*           enum    CPSS_DIAG_PG_PACKET_PAYLOAD_TYPE_ENT
*           struct  CPSS_PM_AP_PORT_PARAMS_STC
*           enum    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT
*           struct  CPSS_HW_INFO_RESOURCE_MAPPING_STC
*           struct  MV_HWS_PORT_INIT_PARAMS
*           enum    CPSS_PM_AP_LANE_SERDES_SPEED
*           enum    CPSS_LOG_TIME_FORMAT_ENT
*           enum    CPSS_FORMAT_CONVERT_FIELD_CONTEXT_ENT
*           struct  CPSS_PORT_COMPHY_SERDES_TX_CONFIG_STC
*           struct  MV_HWS_IPC_PORT_POLARITY_SET_DATA_STRUCT
*           struct  MV_HWS_ALIGN90_PARAMS
*           struct  MV_HWS_RSFEC_COUNTERS_STC
*           union   CPSS_PORT_SERDES_TX_CONFIG_UNT
*           struct  CPSS_PORT_AVAGO_SERDES_RX_CONFIG_STC
*           struct  MV_HWS_MAN_TUNE_COMPHY_TX_CONFIG_DATA
*           struct  MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA
*           enum    CPSS_IP_PROTOCOL_STACK_ENT
*           enum    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT
*           struct  CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC
*           struct  MV_HWS_AVG_BLOCK
*           struct  CPSS_PORT_COMPHY_C112G_RX_CONFIG_STC
*           struct  MV_HWS_AVAGO_RX_OVERRIDE_CONFIG_DATA
*           struct  CPSS_PORT_MAC_MTI_COUNTER_SET_STC
*           struct  CPSS_RSFEC_COUNTERS_STC
*           struct  CPSS_PORT_AP_PARAMS_STC
*           enum    CPSS_PORT_MANAGER_CALIBRATION_TYPE_ENT
*           struct  CPSS_PM_PORT_REG_PARAMS_STC
*           enum    MV_HWS_SERDES_SPEED
*           struct  MV_HWS_ANP_SD_MUX_STC
*           struct  CPSS_PORT_COMPHY_C56G_TX_CONFIG_STC
*           struct  MV_HWS_REF_CLOCK_CFG
*           struct  CPSS_PORT_MAC_TO_SERDES_STC
*           struct  CPSS_INTERFACE_DEV_PORT_STC
*           enum    MV_HWS_AUTO_TUNE_STATUS
*           enum    MV_HWS_SERDES_LB
*           enum    CPSS_DIRECTION_ENT
*           enum    MV_HWS_PORT_PCS_TYPE
*           struct  CPSS_FCFEC_COUNTERS_STC
*           struct  CPSS_PORT_MANAGER_AP_PARALLEL_DETECT_STC
*           union   MV_HWS_SERDES_RX_CONFIG_DATA_UNT
*           enum    MV_HWS_UNIT
*
* DEPENDENCIES:
*
* COMMENTS:
*       Generated at Sun Jan  2 21:31:05 2022
*       /local/store/cpss_regression/cpss/mainLuaWrapper/scripts/make_type_wrapper.py -i /local/store/cpss_regression/tmp/type_info_file_dxpx -p /local/store/cpss_regression/cpss/mainLuaWrapper/scripts/already_implemented.lst -X /local/store/cpss_regression/cpss/mainLuaWrapper/scripts/excludelist -C /local/store/cpss_regression/cpss -M /local/store/cpss_regression/cpss/mainLuaWrapper/scripts/lua_C_mapping -N -F NONE -I common -I mainPxDrv -I mainPpDrv -o /local/store/cpss_regression/tmp/all_common_dxpx_lua_c_type_wrappers.c
*
* FILE REVISION NUMBER:
*       $Revision: 15 $
**************************************************************************/
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <cpss/generic/log/cpssLog.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/port/cpssPortManager.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>
#include <cpss/common/port/cpssPortCtrl.h>
#include <cpss/common/diag/cpssCommonDiag.h>
#include <cpss/common/networkIf/cpssGenNetIfTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortApInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortAnp.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcDefs.h>
#include <cpss/common/cpssHwInit/cpssCommonLedCtrl.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecovery.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesIf.h>
#include <cpssCommon/cpssFormatConvert.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/generic/cpssHwInfo.h>
#include <cpss/common/labServices/port/gop/port/serdes/avago/mvAvagoIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsMifRegDb.h>
#include <cpss/common/config/cpssGenCfg.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvComphyIf.h>
#include <cpss/common/diag/cpssDiagPacketGenerator.h>
#include <cpss/extServices/os/gtOs/gtOs.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/port/cpssPortStat.h>
#include <cpss/generic/events/cpssGenEventUnifyTypes.h>
#include <cpss/generic/events/cpssGenEventRequests.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpssCommon/cpssBuffManagerPool.h>
#include <cpss/common/labServices/port/gop/port/mac/d2dMac/mvHwsD2dMacIf.h>
#include <cpssDriver/pp/config/generic/cpssDrvPpGenDump.h>
#include <cpss/generic/version/gtVersion.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpssAppPlatform/profiles/cpssAppPlatformRunTimeProfile.h>
#include <cpss/common/phy/cpssGenPhySmi.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApLogDefs.h>
#include <cpss/generic/hwDriver/cpssHwDriverAPI.h>
#include <cpss/common/private/prvCpssCyclicLogger.h>
#include <cpss/common/cos/cpssCosTypes.h>
#include <cpssAppPlatform/profiles/cpssAppPlatformBoardProfile.h>
#include <cpss/generic/events/cpssGenEventCtrl.h>

/***** declarations ********/

use_prv_struct(CPSS_INTERFACE_INFO_STC);
use_prv_struct(MV_HWS_PORT_SERDES_TO_MAC_MUX);
use_prv_struct(MV_HWS_SERDES_RX_DATAPATH_CONFIG_STC);
use_prv_struct(CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC);
use_prv_struct(MV_HWS_AP_PORT_STATUS);
use_prv_struct(MV_HWS_ANP_PORT_SM);
use_prv_struct(MV_HWS_RSFEC_STATUS_STC);
use_prv_struct(MV_HWS_TX_TUNE_PARAMS);
use_prv_struct(MV_HWS_IPC_VOS_OVERRIDE_MODE_STRUCT);
use_prv_struct(CPSS_PM_PORT_PARAMS_TYPE_UNT);
use_prv_struct(CPSS_PORT_SERDES_TX_CONFIG_STC);
use_prv_struct(CPSS_SYSTEM_RECOVERY_ERROR_INJECTION_STC);
use_prv_struct(MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS);
use_prv_struct(MV_HWS_IPC_PORT_FEC_CONFIG_DATA_STRUCT);
use_prv_struct(CPSS_PORTS_BMP_STC);
use_prv_struct(MV_HWS_D2D_PRBS_CONFIG_STC);
use_prv_struct(CPSS_FORMAT_CONVERT_FIELD_VAL_CONVERT_STC);
use_prv_struct(MV_HWS_MAN_TUNE_COMPHY_C28G_TX_CONFIG_DATA);
use_prv_struct(MV_HWS_AVAGO_TXRX_OVERRIDE_CONFIG_DATA);
use_prv_struct(CPSS_LED_GROUP_CONF_STC);
use_prv_struct(MV_HWS_IPC_PORT_PPM_SET_DATA_STRUCT);
use_prv_struct(MV_HWS_ETL_CONFIG_DATA);
use_prv_struct(MV_HWS_ANP_INFO_STC);
use_prv_struct(CPSS_PORT_MAC_STATUS_STC);
use_prv_struct(CPSS_PM_PORT_PARAMS_STC);
use_prv_struct(MV_HWS_SERDES_TX_OVERRIDE_CONFIG_DATA_UNT);
use_prv_struct(MV_HWS_IPC_PORT_AUTO_TUNE_STOP_DATA_STRUCT);
use_prv_struct(GT_ETHERADDR);
use_prv_struct(MV_HWS_AUTO_TUNE_STATUS_RES);
use_prv_struct(CPSS_PM_PORT_ATTR_STC);
use_prv_struct(CPSS_GEN_CFG_DEV_INFO_STC);
use_prv_struct(MV_HWS_SERDES_TXRX_TUNE_PARAMS);
use_prv_struct(MV_HWS_COMPHY_REG_FIELD_STC);
use_prv_struct(MV_HWS_IPC_PORT_RESET_DATA_STRUCT);
use_prv_struct(MV_HWS_MAN_TUNE_AVAGO_RX_CONFIG_DATA);
use_prv_struct(MV_HWS_MAN_TUNE_COMPHY_C28GP4_RX_CONFIG_DATA);
use_prv_struct(CPSS_PM_AP_PORT_ADV_STC);
use_prv_struct(MV_HWS_AVAGO_TXRX_TUNE_PARAMS);
use_prv_struct(MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA);
use_prv_struct(CPSS_HW_INFO_RESOURCE_MAPPING_STC);
use_prv_struct(CPSS_PORT_ATTRIBUTES_STC);
use_prv_struct(CPSS_PORT_COMPHY_SERDES_TX_CONFIG_STC);
use_prv_struct(MV_HWS_RSFEC_COUNTERS_STC);
use_prv_struct(MV_HWS_MAN_TUNE_COMPHY_TX_CONFIG_DATA);
use_prv_struct(CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC);
use_prv_struct(CPSS_PORT_COMPHY_C112G_RX_CONFIG_STC);
use_prv_struct(CPSS_RSFEC_COUNTERS_STC);
use_prv_struct(MV_HWS_AVG_BLOCK);
use_prv_struct(GT_U64);
use_prv_struct(CPSS_INTERFACE_DEV_PORT_STC);
use_prv_struct(CPSS_FCFEC_COUNTERS_STC);
use_prv_struct(MV_HWS_SERDES_CONFIG_STC);
use_prv_struct(MV_HWS_IPC_LINK_BINDING_STRUCT);
use_prv_struct(MV_HWS_FW_LOG);
use_prv_struct(MV_HWS_COMPHY_C56G_AUTO_TUNE_RESULTS);
use_prv_struct(CPSS_PORT_MAC_MTI_TX_COUNTER_SET_STC);
use_prv_struct(MV_HWS_MAN_TUNE_AVAGO_16NM_RX_CONFIG_DATA);
use_prv_struct(GT_IPV6ADDR);
use_prv_struct(CPSS_PORT_MANAGER_SGMII_AUTO_NEGOTIATION_STC);
use_prv_struct(CPSS_PORT_AP_STATUS_STC);
use_prv_struct(MV_HWS_IPC_PORT_PARMAS_DATA_STRUCT);
use_prv_struct(MV_HWS_MAN_TUNE_COMPHY_C12G_TX_CONFIG_DATA);
use_prv_struct(CPSS_PORT_MANAGER_CALIBRATION_MODE_STC);
use_prv_struct(CPSS_PORT_AP_SERDES_RX_CONFIG_STC);
use_prv_struct(CPSS_ANP_INFO_STC);
use_prv_struct(MV_HWS_AVAGO_TX_OVERRIDE_CONFIG_DATA);
use_prv_struct(MV_HWS_COMPHY_C12GP41P2V_AUTO_TUNE_RESULTS);
use_prv_struct(MV_HWS_SERDES_PRESET_OVERRIDE_UNT);
use_prv_struct(CPSS_PORT_MAC_MTI_RX_COUNTER_SET_STC);
use_prv_struct(CPSS_PORT_AVAGO_SERDES_RX_CONFIG_STC);
use_prv_struct(MV_HWS_PORT_STATUS_STC);
use_prv_struct(MV_HWS_COMPHY_C28GP4_PRESET_OVERRIDE);
use_prv_struct(CPSS_PORT_MAC_COUNTER_SET_STC);
use_prv_struct(MV_HWS_HCD_INFO);
use_prv_struct(CPSS_HW_DRIVER_SPLIT_ADDR_SPACE_RANGE_STC);
use_prv_struct(CPSS_PORT_COMPHY_C112G_TX_CONFIG_STC);
use_prv_struct(CPSS_PORT_MODE_SPEED_STC);
use_prv_struct(MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE);
use_prv_struct(MV_HWS_SERDES_TX_CONFIG_DATA_UNT);
use_prv_struct(CPSS_PORT_MANAGER_LKB_CONFIG_STC);
use_prv_struct(MV_HWS_ANP_PORT_COUNTERS);
use_prv_struct(CPSS_FORMAT_CONVERT_FIELD_CONDITION_STC);
use_prv_struct(CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC);
use_prv_struct(MV_HWS_COMPHY_H_AUTO_TUNE_RESULTS);
use_prv_struct(MV_HWS_IPC_PORT_SERDES_TX_OFFSETS_STC);
use_prv_struct(CPSS_PORT_COMPHY_C28GP4_RX_CONFIG_STC);
use_prv_struct(MV_HWS_D2D_FRAME_TABLE_CONFIG_STC);
use_prv_struct(MV_HWS_IPC_VOS_OVERRIDE_PARAMS);
use_prv_struct(MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA);
use_prv_struct(MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC);
use_prv_struct(CPSS_PM_AP_PORT_ATTR_STC);
use_prv_struct(GT_VERSION);
use_prv_struct(MV_HWS_IPC_PORT_AP_INTROP_STRUCT);
use_prv_struct(MV_HWS_FCFEC_COUNTERS_STC);
use_prv_struct(MV_HWS_ELEMENTS_ARRAY);
use_prv_struct(CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC);
use_prv_struct(CPSS_PM_AP_PORT_PARAMS_STC);
use_prv_struct(MV_HWS_IPC_PORT_POLARITY_SET_DATA_STRUCT);
use_prv_struct(MV_HWS_SERDES_INFO_STC);
use_prv_struct(MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA);
use_prv_struct(CPSS_PORT_COMPHY_C12GP41P2V_RX_CONFIG_STC);
use_prv_struct(CPSS_PORT_AP_PARAMS_STC);
use_prv_struct(MV_HWS_ANP_SD_MUX_STC);
use_prv_struct(GT_FCID);
use_prv_struct(MV_HWS_ANP_PORT_DATA_STC);
use_prv_struct(CPSS_HW_ADDR_STC);
use_prv_struct(CPSS_PORT_MANAGER_SIGNAL_DETECT_CONFIG_STC);
use_prv_struct(CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC);
use_prv_struct(MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA);
use_prv_struct(MV_HWS_REF_CLOCK_CFG);
use_prv_struct(MV_HWS_D2D_FRAME_GEN_CONFIG_STC);
use_prv_struct(MV_HWS_COMPHY_C112G_AUTO_TUNE_RESULTS);
use_prv_struct(CPSS_PM_AP_MODE_LANE_PARAM_STC);
use_prv_struct(CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_STC);
use_prv_struct(MV_HWS_AP_PORT_STATS);
use_prv_struct(CPSS_SYSTEM_RECOVERY_MODE_STC);
use_prv_struct(MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_DATA_STRUCT);
use_prv_struct(CPSS_802_1Q_VLAN_TAG_STC);
use_prv_struct(MV_HWS_PORT_CTRL_LOG_ENTRY);
use_prv_struct(CPSS_PORT_AVAGO_SERDES_TX_CONFIG_STC);
use_prv_struct(MV_HWS_LOG_ENTRY);
use_prv_struct(CPSS_TRUNK_MEMBER_STC);
use_prv_struct(CPSS_PORT_AP_SERDES_TX_OFFSETS_STC);
use_prv_struct(CPSS_PORT_COMPHY_SERDES_RX_CONFIG_STC);
use_prv_struct(MV_HWS_IPC_PORT_LOOPBACK_SET_DATA_STRUCT);
use_prv_struct(MV_HWS_MAN_TUNE_COMPHY_H_RX_CONFIG_DATA);
use_prv_struct(MV_HWS_AVAGO_AUTO_TUNE_RESULTS);
use_prv_struct(MV_HWS_IPC_PORT_INIT_DATA_STRUCT);
use_prv_struct(CPSS_SYSTEM_RECOVERY_INFO_STC);
use_prv_struct(MV_HWS_IPC_PORT_AP_DATA_STRUCT);
use_prv_struct(CPSS_PORT_COMPHY_C12G_TX_CONFIG_STC);
use_prv_struct(CPSS_PORT_COMPHY_C56G_RX_CONFIG_STC);
use_prv_struct(CPSS_PM_MODE_LANE_PARAM_STC);
use_prv_struct(MV_HWS_CALIBRATION_RESULTS);
use_prv_struct(GT_IPADDR);
use_prv_struct(CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC);
use_prv_struct(MV_HWS_AVAGO_16NM_TXRX_TUNE_PARAMS);
use_prv_struct(MV_HWS_PORT_INIT_INPUT_PARAMS);
use_prv_struct(MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_RX_CONFIG_DATA);
use_prv_struct(MV_HWS_COMPHY_SERDES_DATA);
use_prv_struct(MV_HWS_SERDES_RX_CONFIG_DATA_UNT);
use_prv_struct(GT_IP_ADDR_TYPE_UNT);
use_prv_struct(CPSS_PORT_SERDES_TX_CONFIG_UNT);
use_prv_struct(MV_HWS_SERDES_TEST_GEN_STATUS);
use_prv_struct(MV_HWS_ENHANCE_TUNE_LITE_PHASE_PARAMS);
use_prv_struct(CPSS_PORT_COMPHY_C56G_TX_CONFIG_STC);
use_prv_struct(CPSS_PORT_MAC_TO_SERDES_STC);
use_prv_struct(CPSS_PORT_MANAGER_STATISTICS_STC);
use_prv_struct(CPSS_802_1BR_ETAG_STC);
use_prv_struct(CPSS_PORT_MANAGER_UPDATE_PARAMS_STC);
use_prv_struct(MV_HWS_SAMPLER_RESULTS);
use_prv_struct(CPSS_PORT_SERDES_RX_CONFIG_UNT);
use_prv_struct(CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC);
use_prv_struct(CPSS_PORT_EGRESS_CNTR_STC);
use_prv_struct(MV_HWS_SERDES_TUNING_PARAMS);
use_prv_struct(MV_HWS_SERDES_RX_OVERRIDE_CONFIG_DATA_UNT);
use_prv_struct(MV_HWS_MAN_TUNE_COMPHY_H_TX_CONFIG_DATA);
use_prv_struct(CPSS_CYCLIC_LOGGER_LOG_ENTRY);
use_prv_struct(MV_HWS_AVAGO_RX_OVERRIDE_CONFIG_DATA);
use_prv_struct(CPSS_APP_PLATFORM_BOARD_PROFILE_STC);
use_prv_struct(MV_HWS_IPC_PORT_GENERAL_STRUCT);
use_prv_struct(CPSS_PM_MAC_PREEMPTION_PARAMS_STC);
use_prv_struct(CPSS_PORT_MAC_CG_COUNTER_SET_STC);
use_prv_struct(MV_HWS_TEST_GEN_STATUS);
use_prv_struct(MV_HWS_IPC_PORT_LOOPBACK_GET_DATA_STRUCT);
use_prv_struct(MV_HWS_AP_PORT_INTROP);
use_prv_struct(MV_HWS_ALIGN90_PARAMS);
use_prv_struct(CPSS_PORT_SERDES_TUNE_STC);
use_prv_struct(CPSS_PORT_COMPHY_C28G_TX_CONFIG_STC);
use_prv_struct(MV_HWS_PER_SERDES_INFO_STC);
use_prv_struct(MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT);
use_prv_struct(MV_HWS_IPC_PORT_INFO_STRUCT);
use_prv_struct(CPSS_PORT_SERDES_RX_CONFIG_STC);
use_prv_struct(CPSS_PORT_MANAGER_STC);
use_prv_struct(MV_HWS_D2D_FRAME_CHECKER_CONFIG_STC);
use_prv_struct(MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA);
use_prv_struct(CPSS_PORT_MANAGER_STATUS_STC);
use_prv_struct(MV_HWS_AP_CFG);
use_prv_struct(MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA);
use_prv_struct(MV_HWS_COMPHY_C28GP4_AUTO_TUNE_RESULTS);
use_prv_struct(MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_DATA_STRUCT);
use_prv_struct(CPSS_PM_ET_OVERRIDE_CFG_STC);
use_prv_struct(MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC);
use_prv_struct(CPSS_PORT_MAC_MTI_COUNTER_SET_STC);
use_prv_struct(MV_HWS_ANP_MISC_PARAM_STC);
use_prv_struct(CPSS_PM_PORT_REG_PARAMS_STC);
use_prv_struct(MV_HWS_PORT_INIT_PARAMS);
use_prv_struct(CPSS_PORT_MANAGER_AP_PARALLEL_DETECT_STC);

/***** declarations ********/

void prv_lua_to_c_GT_FCID(
    lua_State *L,
    GT_FCID *val
)
{
    F_ARRAY_START(val, -1, fcid);
    {
        int idx;
        for (idx = 0; idx < 3; idx++) {
            F_ARRAY_NUMBER(val, fcid, idx, GT_U8);
        }
    }
    F_ARRAY_END(val, -1, fcid);
}

void prv_c_to_lua_GT_FCID(
    lua_State *L,
    GT_FCID *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, fcid);
    {
        int idx;
        for (idx = 0; idx < 3; idx++) {
            FO_ARRAY_NUMBER(val, fcid, idx, GT_U8);
        }
    }
    FO_ARRAY_END(val, t, fcid);
    lua_settop(L, t);
}
add_mgm_type(GT_FCID);

void prv_lua_to_c_GT_IP_ADDR_TYPE_UNT(
    lua_State *L,
    GT_IP_ADDR_TYPE_UNT *val
)
{
    F_STRUCT_CUSTOM(val, -1, ipv6Addr, GT_IPV6ADDR);
    F_STRUCT_CUSTOM(val, -1, ipv4Addr, GT_IPADDR);
}

void prv_c_to_lua_GT_IP_ADDR_TYPE_UNT(
    lua_State *L,
    GT_IP_ADDR_TYPE_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, ipv6Addr, GT_IPV6ADDR);
    FO_STRUCT(val, t, ipv4Addr, GT_IPADDR);
    lua_settop(L, t);
}
add_mgm_type(GT_IP_ADDR_TYPE_UNT);
add_mgm_union(GT_IP_ADDR_TYPE_UNT);

add_mgm_enum(MV_HWS_REF_CLOCK_SOURCE);

add_mgm_enum(CPSS_LOG_TYPE_ENT);

void prv_lua_to_c_MV_HWS_SERDES_CONFIG_STC(
    lua_State *L,
    MV_HWS_SERDES_CONFIG_STC *val
)
{
    F_ENUM(val, -1, serdesType, MV_HWS_SERDES_TYPE);
    F_ENUM(val, -1, baudRate, MV_HWS_SERDES_SPEED);
    F_ENUM(val, -1, refClock, MV_HWS_REF_CLOCK);
    F_ENUM(val, -1, refClockSource, MV_HWS_REF_CLOCK_SRC);
    F_ENUM(val, -1, media, MV_HWS_SERDES_MEDIA);
    F_ENUM(val, -1, busWidth, MV_HWS_SERDES_BUS_WIDTH_ENT);
    F_ENUM(val, -1, encoding, MV_HWS_SERDES_ENCODING_TYPE);
    F_BOOL(val, -1, opticalMode);
}

void prv_c_to_lua_MV_HWS_SERDES_CONFIG_STC(
    lua_State *L,
    MV_HWS_SERDES_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, serdesType, MV_HWS_SERDES_TYPE);
    FO_ENUM(val, t, baudRate, MV_HWS_SERDES_SPEED);
    FO_ENUM(val, t, refClock, MV_HWS_REF_CLOCK);
    FO_ENUM(val, t, refClockSource, MV_HWS_REF_CLOCK_SRC);
    FO_ENUM(val, t, media, MV_HWS_SERDES_MEDIA);
    FO_ENUM(val, t, busWidth, MV_HWS_SERDES_BUS_WIDTH_ENT);
    FO_ENUM(val, t, encoding, MV_HWS_SERDES_ENCODING_TYPE);
    FO_BOOL(val, t, opticalMode);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_SERDES_CONFIG_STC);

void prv_lua_to_c_MV_HWS_ANP_PORT_SM(
    lua_State *L,
    MV_HWS_ANP_PORT_SM *val
)
{
    F_NUMBER(val, -1, chSmState, GT_U32);
    F_NUMBER(val, -1, txSmState, GT_U32);
    F_NUMBER(val, -1, rxSmState, GT_U32);
}

void prv_c_to_lua_MV_HWS_ANP_PORT_SM(
    lua_State *L,
    MV_HWS_ANP_PORT_SM *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, chSmState, GT_U32);
    FO_NUMBER(val, t, txSmState, GT_U32);
    FO_NUMBER(val, t, rxSmState, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_ANP_PORT_SM);

void prv_lua_to_c_CPSS_HW_ADDR_STC(
    lua_State *L,
    CPSS_HW_ADDR_STC *val
)
{
    F_NUMBER(val, -1, busNo, GT_U32);
    F_NUMBER(val, -1, devSel, GT_U32);
    F_NUMBER(val, -1, funcNo, GT_U32);
}

void prv_c_to_lua_CPSS_HW_ADDR_STC(
    lua_State *L,
    CPSS_HW_ADDR_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, busNo, GT_U32);
    FO_NUMBER(val, t, devSel, GT_U32);
    FO_NUMBER(val, t, funcNo, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_HW_ADDR_STC);

add_mgm_enum(MV_HWS_PPM_VALUE);

add_mgm_enum(MV_HWS_CONNECTION_TYPE);

add_mgm_enum(MV_HWS_MIF_REGISTERS);

add_mgm_enum(CPSS_PORT_MANAGER_PARALLEL_DETECT_STATUS_ENT);

void prv_lua_to_c_MV_HWS_SERDES_INFO_STC(
    lua_State *L,
    MV_HWS_SERDES_INFO_STC *val
)
{
    F_NUMBER(val, -1, numOfSerdeses, GT_U32);
    F_STRUCT_PTR(val, -1, serdesesDb, serdesesDb, MV_HWS_PER_SERDES_INFO_STC);
}

void prv_c_to_lua_MV_HWS_SERDES_INFO_STC(
    lua_State *L,
    MV_HWS_SERDES_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, numOfSerdeses, GT_U32);
    FO_STRUCT_PTR(val, t, serdesesDb, serdesesDb, MV_HWS_PER_SERDES_INFO_STC);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_SERDES_INFO_STC);

void prv_lua_to_c_CPSS_802_1BR_ETAG_STC(
    lua_State *L,
    CPSS_802_1BR_ETAG_STC *val
)
{
    F_NUMBER(val, -1, TPID, GT_U32);
    F_NUMBER(val, -1, E_PCP, GT_U32);
    F_NUMBER(val, -1, E_DEI, GT_U32);
    F_NUMBER(val, -1, Ingress_E_CID_base, GT_U32);
    F_NUMBER(val, -1, Direction, GT_U32);
    F_NUMBER(val, -1, Upstream_Specific, GT_U32);
    F_NUMBER(val, -1, GRP, GT_U32);
    F_NUMBER(val, -1, E_CID_base, GT_U32);
    F_NUMBER(val, -1, Ingress_E_CID_ext, GT_U32);
    F_NUMBER(val, -1, E_CID_ext, GT_U32);
}

void prv_c_to_lua_CPSS_802_1BR_ETAG_STC(
    lua_State *L,
    CPSS_802_1BR_ETAG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, TPID, GT_U32);
    FO_NUMBER(val, t, E_PCP, GT_U32);
    FO_NUMBER(val, t, E_DEI, GT_U32);
    FO_NUMBER(val, t, Ingress_E_CID_base, GT_U32);
    FO_NUMBER(val, t, Direction, GT_U32);
    FO_NUMBER(val, t, Upstream_Specific, GT_U32);
    FO_NUMBER(val, t, GRP, GT_U32);
    FO_NUMBER(val, t, E_CID_base, GT_U32);
    FO_NUMBER(val, t, Ingress_E_CID_ext, GT_U32);
    FO_NUMBER(val, t, E_CID_ext, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_802_1BR_ETAG_STC);

void prv_lua_to_c_MV_HWS_FW_LOG(
    lua_State *L,
    MV_HWS_FW_LOG *val
)
{
    F_NUMBER(val, -1, fwBaseAddr, GT_U32);
    F_NUMBER(val, -1, fwLogBaseAddr, GT_U32);
    F_NUMBER(val, -1, fwLogCountAddr, GT_U32);
    F_NUMBER(val, -1, fwLogPointerAddr, GT_U32);
    F_NUMBER(val, -1, fwLogResetAddr, GT_U32);
}

void prv_c_to_lua_MV_HWS_FW_LOG(
    lua_State *L,
    MV_HWS_FW_LOG *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, fwBaseAddr, GT_U32);
    FO_NUMBER(val, t, fwLogBaseAddr, GT_U32);
    FO_NUMBER(val, t, fwLogCountAddr, GT_U32);
    FO_NUMBER(val, t, fwLogPointerAddr, GT_U32);
    FO_NUMBER(val, t, fwLogResetAddr, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_FW_LOG);

add_mgm_enum(CPSS_HW_PP_INIT_STAGE_ENT);

void prv_lua_to_c_MV_HWS_IPC_PORT_INIT_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_INIT_DATA_STRUCT *val
)
{
    F_NUMBER(val, -1, portGroup, GT_U16);
    F_NUMBER(val, -1, phyPortNum, GT_U16);
    F_NUMBER(val, -1, portMode, GT_U32);
    F_NUMBER(val, -1, lbPort, GT_U32);
    F_NUMBER(val, -1, refClock, GT_U32);
    F_NUMBER(val, -1, refClockSource, GT_U32);
}

void prv_c_to_lua_MV_HWS_IPC_PORT_INIT_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_INIT_DATA_STRUCT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, portGroup, GT_U16);
    FO_NUMBER(val, t, phyPortNum, GT_U16);
    FO_NUMBER(val, t, portMode, GT_U32);
    FO_NUMBER(val, t, lbPort, GT_U32);
    FO_NUMBER(val, t, refClock, GT_U32);
    FO_NUMBER(val, t, refClockSource, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_IPC_PORT_INIT_DATA_STRUCT);

void prv_lua_to_c_CPSS_PORT_MANAGER_SIGNAL_DETECT_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_SIGNAL_DETECT_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, sdChangeInterval, GT_U32);
    F_NUMBER(val, -1, sdChangeMinWindowSize, GT_U32);
    F_NUMBER(val, -1, sdCheckTimeExpired, GT_U32);
}

void prv_c_to_lua_CPSS_PORT_MANAGER_SIGNAL_DETECT_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_SIGNAL_DETECT_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, sdChangeInterval, GT_U32);
    FO_NUMBER(val, t, sdChangeMinWindowSize, GT_U32);
    FO_NUMBER(val, t, sdCheckTimeExpired, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_MANAGER_SIGNAL_DETECT_CONFIG_STC);

void prv_lua_to_c_MV_HWS_AVAGO_TXRX_TUNE_PARAMS(
    lua_State *L,
    MV_HWS_AVAGO_TXRX_TUNE_PARAMS *val
)
{
    F_ENUM(val, -1, serdesSpeed, MV_HWS_SERDES_SPEED);
    F_STRUCT(val, -1, txParams, MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA);
    F_STRUCT(val, -1, rxParams, MV_HWS_MAN_TUNE_AVAGO_RX_CONFIG_DATA);
}

void prv_c_to_lua_MV_HWS_AVAGO_TXRX_TUNE_PARAMS(
    lua_State *L,
    MV_HWS_AVAGO_TXRX_TUNE_PARAMS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, serdesSpeed, MV_HWS_SERDES_SPEED);
    FO_STRUCT(val, t, txParams, MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA);
    FO_STRUCT(val, t, rxParams, MV_HWS_MAN_TUNE_AVAGO_RX_CONFIG_DATA);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_AVAGO_TXRX_TUNE_PARAMS);

add_mgm_enum(CPSS_SERDES_TERMINATION);

add_mgm_enum(MV_HWS_PORT_FEC_MODE);

add_mgm_enum(CPSS_PORT_REF_CLOCK_SOURCE_ENT);

void prv_lua_to_c_MV_HWS_COMPHY_C56G_AUTO_TUNE_RESULTS(
    lua_State *L,
    MV_HWS_COMPHY_C56G_AUTO_TUNE_RESULTS *val
)
{
    F_STRUCT(val, -1, txComphyC56G, MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA);
    F_STRUCT(val, -1, rxComphyC56G, MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA);
    F_NUMBER(val, -1, eo, GT_U8);
    F_NUMBER(val, -1, align90, GT_U16);
    F_NUMBER(val, -1, sampler, GT_U16);
    F_NUMBER(val, -1, slewRateCtrl0, GT_U32);
    F_NUMBER(val, -1, slewRateCtrl1, GT_U32);
    F_ARRAY_START(val, -1, dfe);
    {
        int idx;
        for (idx = 0; idx < 26; idx++) {
            F_ARRAY_NUMBER(val, dfe, idx, GT_32);
        }
    }
    F_ARRAY_END(val, -1, dfe);
}

void prv_c_to_lua_MV_HWS_COMPHY_C56G_AUTO_TUNE_RESULTS(
    lua_State *L,
    MV_HWS_COMPHY_C56G_AUTO_TUNE_RESULTS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, txComphyC56G, MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA);
    FO_STRUCT(val, t, rxComphyC56G, MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA);
    FO_NUMBER(val, t, eo, GT_U8);
    FO_NUMBER(val, t, align90, GT_U16);
    FO_NUMBER(val, t, sampler, GT_U16);
    FO_NUMBER(val, t, slewRateCtrl0, GT_U32);
    FO_NUMBER(val, t, slewRateCtrl1, GT_U32);
    FO_ARRAY_START(val, t, dfe);
    {
        int idx;
        for (idx = 0; idx < 26; idx++) {
            FO_ARRAY_NUMBER(val, dfe, idx, GT_32);
        }
    }
    FO_ARRAY_END(val, t, dfe);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_COMPHY_C56G_AUTO_TUNE_RESULTS);

add_mgm_enum(CPSS_DIAG_TEST_PROFILE_ENT);

void prv_lua_to_c_MV_HWS_IPC_PORT_FEC_CONFIG_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_FEC_CONFIG_DATA_STRUCT *val
)
{
    F_NUMBER(val, -1, portGroup, GT_U16);
    F_NUMBER(val, -1, phyPortNum, GT_U16);
    F_NUMBER(val, -1, portMode, GT_U32);
    F_NUMBER(val, -1, portFecType, GT_U32);
}

void prv_c_to_lua_MV_HWS_IPC_PORT_FEC_CONFIG_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_FEC_CONFIG_DATA_STRUCT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, portGroup, GT_U16);
    FO_NUMBER(val, t, phyPortNum, GT_U16);
    FO_NUMBER(val, t, portMode, GT_U32);
    FO_NUMBER(val, t, portFecType, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_IPC_PORT_FEC_CONFIG_DATA_STRUCT);

add_mgm_enum(CPSS_LED_CLOCK_OUT_FREQUENCY_ENT);

void prv_lua_to_c_CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC(
    lua_State *L,
    CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC *val
)
{
    F_ARRAY_START(val, -1, counterArray);
    {
        int idx;
        for (idx = 0; idx < 128; idx++) {
            F_ARRAY_NUMBER(val, counterArray, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, counterArray);
}

void prv_c_to_lua_CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC(
    lua_State *L,
    CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, counterArray);
    {
        int idx;
        for (idx = 0; idx < 128; idx++) {
            FO_ARRAY_NUMBER(val, counterArray, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, counterArray);
    lua_settop(L, t);
}
add_mgm_type(CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC);

add_mgm_enum(MV_HWS_AVAGO_SERDES_HAL_FUNC_TYPES);

add_mgm_enum(CPSS_FORMAT_CONVERT_FIELD_CONVERT_CMD_ENT);

add_mgm_enum(CPSS_UNI_EV_CAUSE_ENT);

add_mgm_enum(MV_HWS_PORT_TEST_GEN_ACTION);

void prv_lua_to_c_MV_HWS_ANP_PORT_DATA_STC(
    lua_State *L,
    MV_HWS_ANP_PORT_DATA_STC *val
)
{
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* unhandled line
MV_HWS_ANP_SD_MUX_STC anpSdMuxDb[4][14]; */
#endif
    F_ARRAY_START(val, -1, anpPortParamPtr);
    F_ARRAY_END(val, -1, anpPortParamPtr);
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* can't handle STRUCT_PTR for array
MV_HWS_AP_CFG *anpPortParamPtr[128]; */
#endif
    F_ARRAY_START(val, -1, anpSkipResetPtr);
    F_ARRAY_END(val, -1, anpSkipResetPtr);
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* can't handle BOOL_PTR for array
GT_BOOL *anpSkipResetPtr[128]; */
#endif
    F_ARRAY_START(val, -1, anpCmdIntLineIdxPtr);
    F_ARRAY_END(val, -1, anpCmdIntLineIdxPtr);
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* can't handle NUMBER_PTR for array
GT_U32 *anpCmdIntLineIdxPtr[128]; */
#endif
    F_ARRAY_START(val, -1, anpPortMiscParamDbPtr);
    F_ARRAY_END(val, -1, anpPortMiscParamDbPtr);
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* can't handle STRUCT_PTR for array
MV_HWS_ANP_MISC_PARAM_STC *anpPortMiscParamDbPtr[128]; */
#endif
}

void prv_c_to_lua_MV_HWS_ANP_PORT_DATA_STC(
    lua_State *L,
    MV_HWS_ANP_PORT_DATA_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* unhandled line
MV_HWS_ANP_SD_MUX_STC anpSdMuxDb[4][14]; */
#endif
    FO_ARRAY_START(val, t, anpPortParamPtr);
    FO_ARRAY_END(val, t, anpPortParamPtr);
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* can't handle STRUCT_PTR for array
MV_HWS_AP_CFG *anpPortParamPtr[128]; */
#endif
    FO_ARRAY_START(val, t, anpSkipResetPtr);
    FO_ARRAY_END(val, t, anpSkipResetPtr);
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* can't handle BOOL_PTR for array
GT_BOOL *anpSkipResetPtr[128]; */
#endif
    FO_ARRAY_START(val, t, anpCmdIntLineIdxPtr);
    FO_ARRAY_END(val, t, anpCmdIntLineIdxPtr);
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* can't handle NUMBER_PTR for array
GT_U32 *anpCmdIntLineIdxPtr[128]; */
#endif
    FO_ARRAY_START(val, t, anpPortMiscParamDbPtr);
    FO_ARRAY_END(val, t, anpPortMiscParamDbPtr);
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* can't handle STRUCT_PTR for array
MV_HWS_ANP_MISC_PARAM_STC *anpPortMiscParamDbPtr[128]; */
#endif
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_ANP_PORT_DATA_STC);

void prv_lua_to_c_MV_HWS_CALIBRATION_RESULTS(
    lua_State *L,
    MV_HWS_CALIBRATION_RESULTS *val
)
{
    F_BOOL(val, -1, calDone);
    F_NUMBER(val, -1, txImpCal, GT_U32);
    F_NUMBER(val, -1, rxImpCal, GT_U32);
    F_NUMBER(val, -1, ProcessCal, GT_U32);
    F_NUMBER(val, -1, speedPll, GT_U32);
    F_NUMBER(val, -1, sellvTxClk, GT_U32);
    F_NUMBER(val, -1, sellvTxData, GT_U32);
    F_NUMBER(val, -1, sellvTxIntp, GT_U32);
    F_NUMBER(val, -1, sellvTxDrv, GT_U32);
    F_NUMBER(val, -1, sellvTxDig, GT_U32);
    F_NUMBER(val, -1, sellvRxSample, GT_U32);
    F_BOOL(val, -1, ffeDone);
    F_ARRAY_START(val, -1, ffeCal);
    {
        int idx;
        for (idx = 0; idx < 8; idx++) {
            F_ARRAY_NUMBER(val, ffeCal, idx, GT_32);
        }
    }
    F_ARRAY_END(val, -1, ffeCal);
}

void prv_c_to_lua_MV_HWS_CALIBRATION_RESULTS(
    lua_State *L,
    MV_HWS_CALIBRATION_RESULTS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, calDone);
    FO_NUMBER(val, t, txImpCal, GT_U32);
    FO_NUMBER(val, t, rxImpCal, GT_U32);
    FO_NUMBER(val, t, ProcessCal, GT_U32);
    FO_NUMBER(val, t, speedPll, GT_U32);
    FO_NUMBER(val, t, sellvTxClk, GT_U32);
    FO_NUMBER(val, t, sellvTxData, GT_U32);
    FO_NUMBER(val, t, sellvTxIntp, GT_U32);
    FO_NUMBER(val, t, sellvTxDrv, GT_U32);
    FO_NUMBER(val, t, sellvTxDig, GT_U32);
    FO_NUMBER(val, t, sellvRxSample, GT_U32);
    FO_BOOL(val, t, ffeDone);
    FO_ARRAY_START(val, t, ffeCal);
    {
        int idx;
        for (idx = 0; idx < 8; idx++) {
            FO_ARRAY_NUMBER(val, ffeCal, idx, GT_32);
        }
    }
    FO_ARRAY_END(val, t, ffeCal);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_CALIBRATION_RESULTS);

void prv_lua_to_c_MV_HWS_AP_PORT_STATUS(
    lua_State *L,
    MV_HWS_AP_PORT_STATUS *val
)
{
    F_NUMBER(val, -1, preApPortNum, GT_U32);
    F_NUMBER(val, -1, apLaneNum, GT_U32);
    F_NUMBER(val, -1, postApPortNum, GT_U32);
    F_ENUM(val, -1, postApPortMode, MV_HWS_PORT_STANDARD);
    F_STRUCT(val, -1, hcdResult, MV_HWS_HCD_INFO);
    F_NUMBER(val, -1, smState, GT_U32);
    F_NUMBER(val, -1, smStatus, GT_U32);
    F_NUMBER(val, -1, arbStatus, GT_U32);
}

void prv_c_to_lua_MV_HWS_AP_PORT_STATUS(
    lua_State *L,
    MV_HWS_AP_PORT_STATUS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, preApPortNum, GT_U32);
    FO_NUMBER(val, t, apLaneNum, GT_U32);
    FO_NUMBER(val, t, postApPortNum, GT_U32);
    FO_ENUM(val, t, postApPortMode, MV_HWS_PORT_STANDARD);
    FO_STRUCT(val, t, hcdResult, MV_HWS_HCD_INFO);
    FO_NUMBER(val, t, smState, GT_U32);
    FO_NUMBER(val, t, smStatus, GT_U32);
    FO_NUMBER(val, t, arbStatus, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_AP_PORT_STATUS);

void prv_lua_to_c_CPSS_PORT_MANAGER_SGMII_AUTO_NEGOTIATION_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_SGMII_AUTO_NEGOTIATION_STC *val
)
{
    F_BOOL(val, -1, inbandEnable);
    F_BOOL(val, -1, duplexEnable);
    F_BOOL(val, -1, speedEnable);
    F_BOOL(val, -1, byPassEnable);
    F_BOOL(val, -1, flowCtrlEnable);
    F_BOOL(val, -1, flowCtrlPauseAdvertiseEnable);
    F_BOOL(val, -1, flowCtrlAsmAdvertiseEnable);
}

void prv_c_to_lua_CPSS_PORT_MANAGER_SGMII_AUTO_NEGOTIATION_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_SGMII_AUTO_NEGOTIATION_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, inbandEnable);
    FO_BOOL(val, t, duplexEnable);
    FO_BOOL(val, t, speedEnable);
    FO_BOOL(val, t, byPassEnable);
    FO_BOOL(val, t, flowCtrlEnable);
    FO_BOOL(val, t, flowCtrlPauseAdvertiseEnable);
    FO_BOOL(val, t, flowCtrlAsmAdvertiseEnable);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_MANAGER_SGMII_AUTO_NEGOTIATION_STC);

add_mgm_enum(CPSS_PORT_MAC_MTI_BR_STAT_COUNTER_ENT);

void prv_lua_to_c_CPSS_PORT_COMPHY_C28GP4_RX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_COMPHY_C28GP4_RX_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, dataRate, GT_U32);
    F_NUMBER(val, -1, res1Sel, GT_U32);
    F_NUMBER(val, -1, res2Sel, GT_U32);
    F_NUMBER(val, -1, cap1Sel, GT_U32);
    F_NUMBER(val, -1, cap2Sel, GT_U32);
    F_NUMBER(val, -1, selmufi, GT_U32);
    F_NUMBER(val, -1, selmuff, GT_U32);
    F_NUMBER(val, -1, selmupi, GT_U32);
    F_NUMBER(val, -1, selmupf, GT_U32);
    F_NUMBER(val, -1, midpointLargeThresKLane, GT_U32);
    F_NUMBER(val, -1, midpointSmallThresKLane, GT_U32);
    F_NUMBER(val, -1, midpointLargeThresCLane, GT_U32);
    F_NUMBER(val, -1, midpointSmallThresCLane, GT_U32);
    F_NUMBER(val, -1, dfeResF0aHighThresInitLane, GT_U32);
    F_NUMBER(val, -1, dfeResF0aHighThresEndLane, GT_U32);
    F_NUMBER(val, -1, squelch, GT_32);
    F_NUMBER(val, -1, align90, GT_U32);
    F_NUMBER(val, -1, sampler, GT_U32);
    F_NUMBER(val, -1, slewRateCtrl0, GT_U32);
    F_NUMBER(val, -1, slewRateCtrl1, GT_U32);
    F_ARRAY_START(val, -1, dfe);
    {
        int idx;
        for (idx = 0; idx < 25; idx++) {
            F_ARRAY_NUMBER(val, dfe, idx, GT_32);
        }
    }
    F_ARRAY_END(val, -1, dfe);
    F_NUMBER(val, -1, EO, GT_U32);
}

void prv_c_to_lua_CPSS_PORT_COMPHY_C28GP4_RX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_COMPHY_C28GP4_RX_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, dataRate, GT_U32);
    FO_NUMBER(val, t, res1Sel, GT_U32);
    FO_NUMBER(val, t, res2Sel, GT_U32);
    FO_NUMBER(val, t, cap1Sel, GT_U32);
    FO_NUMBER(val, t, cap2Sel, GT_U32);
    FO_NUMBER(val, t, selmufi, GT_U32);
    FO_NUMBER(val, t, selmuff, GT_U32);
    FO_NUMBER(val, t, selmupi, GT_U32);
    FO_NUMBER(val, t, selmupf, GT_U32);
    FO_NUMBER(val, t, midpointLargeThresKLane, GT_U32);
    FO_NUMBER(val, t, midpointSmallThresKLane, GT_U32);
    FO_NUMBER(val, t, midpointLargeThresCLane, GT_U32);
    FO_NUMBER(val, t, midpointSmallThresCLane, GT_U32);
    FO_NUMBER(val, t, dfeResF0aHighThresInitLane, GT_U32);
    FO_NUMBER(val, t, dfeResF0aHighThresEndLane, GT_U32);
    FO_NUMBER(val, t, squelch, GT_32);
    FO_NUMBER(val, t, align90, GT_U32);
    FO_NUMBER(val, t, sampler, GT_U32);
    FO_NUMBER(val, t, slewRateCtrl0, GT_U32);
    FO_NUMBER(val, t, slewRateCtrl1, GT_U32);
    FO_ARRAY_START(val, t, dfe);
    {
        int idx;
        for (idx = 0; idx < 25; idx++) {
            FO_ARRAY_NUMBER(val, dfe, idx, GT_32);
        }
    }
    FO_ARRAY_END(val, t, dfe);
    FO_NUMBER(val, t, EO, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_COMPHY_C28GP4_RX_CONFIG_STC);

add_mgm_enum(CPSS_PORT_FLOW_CONTROL_ENT);

void prv_lua_to_c_MV_HWS_IPC_PORT_PARMAS_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_PARMAS_DATA_STRUCT *val
)
{
    F_NUMBER(val, -1, portGroup, GT_U16);
    F_NUMBER(val, -1, phyPortNum, GT_U16);
    F_NUMBER(val, -1, portStandard, GT_U32);
    F_NUMBER(val, -1, portMacType, GT_U32);
    F_NUMBER(val, -1, portMacNumber, GT_U32);
    F_NUMBER(val, -1, portPcsType, GT_U32);
    F_NUMBER(val, -1, portPcsNumber, GT_U32);
    F_NUMBER(val, -1, portFecMode, GT_U32);
    F_NUMBER(val, -1, serdesSpeed, GT_U32);
    F_NUMBER(val, -1, firstLaneNum, GT_U8);
    F_NUMBER(val, -1, numOfActLanes, GT_U8);
    F_ARRAY_START(val, -1, activeLanesList);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            F_ARRAY_NUMBER(val, activeLanesList, idx, GT_U8);
        }
    }
    F_ARRAY_END(val, -1, activeLanesList);
    F_NUMBER(val, -1, serdesMediaType, GT_U32);
    F_NUMBER(val, -1, serdes10BitStatus, GT_U32);
}

void prv_c_to_lua_MV_HWS_IPC_PORT_PARMAS_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_PARMAS_DATA_STRUCT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, portGroup, GT_U16);
    FO_NUMBER(val, t, phyPortNum, GT_U16);
    FO_NUMBER(val, t, portStandard, GT_U32);
    FO_NUMBER(val, t, portMacType, GT_U32);
    FO_NUMBER(val, t, portMacNumber, GT_U32);
    FO_NUMBER(val, t, portPcsType, GT_U32);
    FO_NUMBER(val, t, portPcsNumber, GT_U32);
    FO_NUMBER(val, t, portFecMode, GT_U32);
    FO_NUMBER(val, t, serdesSpeed, GT_U32);
    FO_NUMBER(val, t, firstLaneNum, GT_U8);
    FO_NUMBER(val, t, numOfActLanes, GT_U8);
    FO_ARRAY_START(val, t, activeLanesList);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            FO_ARRAY_NUMBER(val, activeLanesList, idx, GT_U8);
        }
    }
    FO_ARRAY_END(val, t, activeLanesList);
    FO_NUMBER(val, t, serdesMediaType, GT_U32);
    FO_NUMBER(val, t, serdes10BitStatus, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_IPC_PORT_PARMAS_DATA_STRUCT);

void prv_lua_to_c_MV_HWS_MAN_TUNE_COMPHY_C12G_TX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_C12G_TX_CONFIG_DATA *val
)
{
    F_NUMBER(val, -1, pre, GT_U8);
    F_NUMBER(val, -1, peak, GT_U8);
    F_NUMBER(val, -1, post, GT_U8);
}

void prv_c_to_lua_MV_HWS_MAN_TUNE_COMPHY_C12G_TX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_C12G_TX_CONFIG_DATA *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, pre, GT_U8);
    FO_NUMBER(val, t, peak, GT_U8);
    FO_NUMBER(val, t, post, GT_U8);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_MAN_TUNE_COMPHY_C12G_TX_CONFIG_DATA);

void prv_lua_to_c_MV_HWS_MAN_TUNE_AVAGO_16NM_RX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_AVAGO_16NM_RX_CONFIG_DATA *val
)
{
    F_NUMBER(val, -1, lowFrequency, GT_U8);
    F_NUMBER(val, -1, highFrequency, GT_U8);
    F_NUMBER(val, -1, bandWidth, GT_U8);
    F_NUMBER(val, -1, dcGain, GT_U8);
    F_NUMBER(val, -1, gainShape1, GT_U8);
    F_NUMBER(val, -1, gainShape2, GT_U8);
    F_NUMBER(val, -1, ffeFix, GT_U8);
    F_NUMBER(val, -1, shortChannelEn, GT_U8);
    F_NUMBER(val, -1, bfLf, GT_8);
    F_NUMBER(val, -1, bfHf, GT_8);
    F_NUMBER(val, -1, minLf, GT_U8);
    F_NUMBER(val, -1, maxLf, GT_U8);
    F_NUMBER(val, -1, minHf, GT_U8);
    F_NUMBER(val, -1, maxHf, GT_U8);
    F_NUMBER(val, -1, minPre1, GT_8);
    F_NUMBER(val, -1, maxPre1, GT_8);
    F_NUMBER(val, -1, minPre2, GT_8);
    F_NUMBER(val, -1, maxPre2, GT_8);
    F_NUMBER(val, -1, minPost, GT_8);
    F_NUMBER(val, -1, maxPost, GT_8);
    F_NUMBER(val, -1, squelch, GT_U16);
    F_NUMBER(val, -1, iCalEffort, GT_U8);
    F_NUMBER(val, -1, pCalEffort, GT_U8);
    F_NUMBER(val, -1, dfeDataRate, GT_U8);
    F_NUMBER(val, -1, dfeCommon, GT_U8);
    F_NUMBER(val, -1, int11d, GT_U16);
    F_NUMBER(val, -1, int17d, GT_U16);
    F_NUMBER(val, -1, termination, GT_U8);
    F_NUMBER(val, -1, pre1PosGradient, GT_16);
    F_NUMBER(val, -1, pre1NegGradient, GT_16);
    F_NUMBER(val, -1, pre2PosGradient, GT_16);
    F_NUMBER(val, -1, pre2NegGradient, GT_16);
    F_NUMBER(val, -1, hfPosGradient, GT_16);
    F_NUMBER(val, -1, hfNegGradient, GT_16);
    F_NUMBER(val, -1, agcTargetLow, GT_U8);
    F_NUMBER(val, -1, agcTargetHigh, GT_U8);
    F_NUMBER(val, -1, coldEnvelope, GT_U8);
    F_NUMBER(val, -1, hotEnvelope, GT_U8);
}

void prv_c_to_lua_MV_HWS_MAN_TUNE_AVAGO_16NM_RX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_AVAGO_16NM_RX_CONFIG_DATA *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, lowFrequency, GT_U8);
    FO_NUMBER(val, t, highFrequency, GT_U8);
    FO_NUMBER(val, t, bandWidth, GT_U8);
    FO_NUMBER(val, t, dcGain, GT_U8);
    FO_NUMBER(val, t, gainShape1, GT_U8);
    FO_NUMBER(val, t, gainShape2, GT_U8);
    FO_NUMBER(val, t, ffeFix, GT_U8);
    FO_NUMBER(val, t, shortChannelEn, GT_U8);
    FO_NUMBER(val, t, bfLf, GT_8);
    FO_NUMBER(val, t, bfHf, GT_8);
    FO_NUMBER(val, t, minLf, GT_U8);
    FO_NUMBER(val, t, maxLf, GT_U8);
    FO_NUMBER(val, t, minHf, GT_U8);
    FO_NUMBER(val, t, maxHf, GT_U8);
    FO_NUMBER(val, t, minPre1, GT_8);
    FO_NUMBER(val, t, maxPre1, GT_8);
    FO_NUMBER(val, t, minPre2, GT_8);
    FO_NUMBER(val, t, maxPre2, GT_8);
    FO_NUMBER(val, t, minPost, GT_8);
    FO_NUMBER(val, t, maxPost, GT_8);
    FO_NUMBER(val, t, squelch, GT_U16);
    FO_NUMBER(val, t, iCalEffort, GT_U8);
    FO_NUMBER(val, t, pCalEffort, GT_U8);
    FO_NUMBER(val, t, dfeDataRate, GT_U8);
    FO_NUMBER(val, t, dfeCommon, GT_U8);
    FO_NUMBER(val, t, int11d, GT_U16);
    FO_NUMBER(val, t, int17d, GT_U16);
    FO_NUMBER(val, t, termination, GT_U8);
    FO_NUMBER(val, t, pre1PosGradient, GT_16);
    FO_NUMBER(val, t, pre1NegGradient, GT_16);
    FO_NUMBER(val, t, pre2PosGradient, GT_16);
    FO_NUMBER(val, t, pre2NegGradient, GT_16);
    FO_NUMBER(val, t, hfPosGradient, GT_16);
    FO_NUMBER(val, t, hfNegGradient, GT_16);
    FO_NUMBER(val, t, agcTargetLow, GT_U8);
    FO_NUMBER(val, t, agcTargetHigh, GT_U8);
    FO_NUMBER(val, t, coldEnvelope, GT_U8);
    FO_NUMBER(val, t, hotEnvelope, GT_U8);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_MAN_TUNE_AVAGO_16NM_RX_CONFIG_DATA);

add_mgm_enum(CPSS_LOG_LIB_ENT);

void prv_lua_to_c_MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE(
    lua_State *L,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE *val
)
{
    F_NUMBER(val, -1, serdesSpeed, GT_U32);
    F_STRUCT(val, -1, ctleParams, MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA);
    F_STRUCT(val, -1, etlParams, MV_HWS_ETL_CONFIG_DATA);
    F_NUMBER(val, -1, fieldOverrideBmp, GT_U16);
}

void prv_c_to_lua_MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE(
    lua_State *L,
    MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, serdesSpeed, GT_U32);
    FO_STRUCT(val, t, ctleParams, MV_HWS_MAN_TUNE_CTLE_CONFIG_DATA);
    FO_STRUCT(val, t, etlParams, MV_HWS_ETL_CONFIG_DATA);
    FO_NUMBER(val, t, fieldOverrideBmp, GT_U16);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE);

void prv_lua_to_c_CPSS_PORT_MANAGER_UPDATE_PARAMS_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_UPDATE_PARAMS_STC *val
)
{
    F_NUMBER(val, -1, updateParamsBitmapType, GT_U32);
    F_ENUM(val, -1, bwMode, CPSS_PORT_PA_BW_MODE_ENT);
}

void prv_c_to_lua_CPSS_PORT_MANAGER_UPDATE_PARAMS_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_UPDATE_PARAMS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, updateParamsBitmapType, GT_U32);
    FO_ENUM(val, t, bwMode, CPSS_PORT_PA_BW_MODE_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_MANAGER_UPDATE_PARAMS_STC);

void prv_lua_to_c_MV_HWS_SAMPLER_RESULTS(
    lua_State *L,
    MV_HWS_SAMPLER_RESULTS *val
)
{
    F_NUMBER(val, -1, bestVal, GT_U32);
    F_NUMBER(val, -1, positiveVal, GT_U32);
    F_NUMBER(val, -1, negativeVal, GT_U32);
}

void prv_c_to_lua_MV_HWS_SAMPLER_RESULTS(
    lua_State *L,
    MV_HWS_SAMPLER_RESULTS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, bestVal, GT_U32);
    FO_NUMBER(val, t, positiveVal, GT_U32);
    FO_NUMBER(val, t, negativeVal, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_SAMPLER_RESULTS);

void prv_lua_to_c_MV_HWS_COMPHY_C112G_AUTO_TUNE_RESULTS(
    lua_State *L,
    MV_HWS_COMPHY_C112G_AUTO_TUNE_RESULTS *val
)
{
    F_STRUCT(val, -1, txComphyC112G, MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA);
    F_STRUCT(val, -1, rxComphyC112G, MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA);
    F_NUMBER(val, -1, eo, GT_U8);
    F_NUMBER(val, -1, align90AnaReg, GT_U32);
    F_NUMBER(val, -1, align90, GT_16);
    F_NUMBER(val, -1, sampler, GT_U16);
    F_NUMBER(val, -1, slewRateCtrl0, GT_U32);
    F_NUMBER(val, -1, slewRateCtrl1, GT_U32);
    F_ARRAY_START(val, -1, dfe);
    {
        int idx;
        for (idx = 0; idx < 40; idx++) {
            F_ARRAY_NUMBER(val, dfe, idx, GT_32);
        }
    }
    F_ARRAY_END(val, -1, dfe);
}

void prv_c_to_lua_MV_HWS_COMPHY_C112G_AUTO_TUNE_RESULTS(
    lua_State *L,
    MV_HWS_COMPHY_C112G_AUTO_TUNE_RESULTS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, txComphyC112G, MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA);
    FO_STRUCT(val, t, rxComphyC112G, MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA);
    FO_NUMBER(val, t, eo, GT_U8);
    FO_NUMBER(val, t, align90AnaReg, GT_U32);
    FO_NUMBER(val, t, align90, GT_16);
    FO_NUMBER(val, t, sampler, GT_U16);
    FO_NUMBER(val, t, slewRateCtrl0, GT_U32);
    FO_NUMBER(val, t, slewRateCtrl1, GT_U32);
    FO_ARRAY_START(val, t, dfe);
    {
        int idx;
        for (idx = 0; idx < 40; idx++) {
            FO_ARRAY_NUMBER(val, dfe, idx, GT_32);
        }
    }
    FO_ARRAY_END(val, t, dfe);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_COMPHY_C112G_AUTO_TUNE_RESULTS);

void prv_lua_to_c_CPSS_PORT_SERDES_RX_CONFIG_UNT(
    lua_State *L,
    CPSS_PORT_SERDES_RX_CONFIG_UNT *val
)
{
    F_UNION_MEMBER_STRUCT(val, -1, comphy, CPSS_PORT_COMPHY_SERDES_RX_CONFIG_STC);
    F_UNION_MEMBER_STRUCT(val, -1, comphy_C12G, CPSS_PORT_COMPHY_C12GP41P2V_RX_CONFIG_STC);
    F_UNION_MEMBER_STRUCT(val, -1, comphy_C28G, CPSS_PORT_COMPHY_C28GP4_RX_CONFIG_STC);
    F_UNION_MEMBER_STRUCT(val, -1, comphy_C56G, CPSS_PORT_COMPHY_C56G_RX_CONFIG_STC);
    F_UNION_MEMBER_STRUCT(val, -1, comphy_C112G, CPSS_PORT_COMPHY_C112G_RX_CONFIG_STC);
    F_UNION_MEMBER_STRUCT(val, -1, avago, CPSS_PORT_AVAGO_SERDES_RX_CONFIG_STC);
}

void prv_c_to_lua_CPSS_PORT_SERDES_RX_CONFIG_UNT(
    lua_State *L,
    CPSS_PORT_SERDES_RX_CONFIG_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, comphy, CPSS_PORT_COMPHY_SERDES_RX_CONFIG_STC);
    FO_STRUCT(val, t, comphy_C12G, CPSS_PORT_COMPHY_C12GP41P2V_RX_CONFIG_STC);
    FO_STRUCT(val, t, comphy_C28G, CPSS_PORT_COMPHY_C28GP4_RX_CONFIG_STC);
    FO_STRUCT(val, t, comphy_C56G, CPSS_PORT_COMPHY_C56G_RX_CONFIG_STC);
    FO_STRUCT(val, t, comphy_C112G, CPSS_PORT_COMPHY_C112G_RX_CONFIG_STC);
    FO_STRUCT(val, t, avago, CPSS_PORT_AVAGO_SERDES_RX_CONFIG_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_SERDES_RX_CONFIG_UNT);
add_mgm_union(CPSS_PORT_SERDES_RX_CONFIG_UNT);

void prv_lua_to_c_CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC(
    lua_State *L,
    CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC *val
)
{
    F_NUMBER(val, -1, portGroupId, GT_U32);
    F_NUMBER(val, -1, mppmId, GT_U32);
    F_NUMBER(val, -1, bankId, GT_U32);
}

void prv_c_to_lua_CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC(
    lua_State *L,
    CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, portGroupId, GT_U32);
    FO_NUMBER(val, t, mppmId, GT_U32);
    FO_NUMBER(val, t, bankId, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC);

void prv_lua_to_c_MV_HWS_IPC_VOS_OVERRIDE_MODE_STRUCT(
    lua_State *L,
    MV_HWS_IPC_VOS_OVERRIDE_MODE_STRUCT *val
)
{
    F_NUMBER(val, -1, portGroup, GT_U16);
    F_NUMBER(val, -1, phyPortNum, GT_U16);
    F_NUMBER(val, -1, portMode, GT_U32);
    F_NUMBER(val, -1, vosOverrideMode, GT_U32);
}

void prv_c_to_lua_MV_HWS_IPC_VOS_OVERRIDE_MODE_STRUCT(
    lua_State *L,
    MV_HWS_IPC_VOS_OVERRIDE_MODE_STRUCT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, portGroup, GT_U16);
    FO_NUMBER(val, t, phyPortNum, GT_U16);
    FO_NUMBER(val, t, portMode, GT_U32);
    FO_NUMBER(val, t, vosOverrideMode, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_IPC_VOS_OVERRIDE_MODE_STRUCT);

add_mgm_enum(CPSS_DRV_HW_ACCESS_STAGE_ENT);

add_mgm_enum(CPSS_PORT_PCS_LOOPBACK_MODE_ENT);

add_mgm_enum(CPSS_PORT_DIRECTION_ENT);

void prv_lua_to_c_MV_HWS_D2D_FRAME_GEN_CONFIG_STC(
    lua_State *L,
    MV_HWS_D2D_FRAME_GEN_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, txFrameRepeatCount, GT_U32);
    F_NUMBER(val, -1, txFrameTableCount, GT_U32);
    F_NUMBER(val, -1, txFrameTableStartPointer, GT_U32);
    F_NUMBER(val, -1, txPayloadType, GT_U32);
}

void prv_c_to_lua_MV_HWS_D2D_FRAME_GEN_CONFIG_STC(
    lua_State *L,
    MV_HWS_D2D_FRAME_GEN_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, txFrameRepeatCount, GT_U32);
    FO_NUMBER(val, t, txFrameTableCount, GT_U32);
    FO_NUMBER(val, t, txFrameTableStartPointer, GT_U32);
    FO_NUMBER(val, t, txPayloadType, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_D2D_FRAME_GEN_CONFIG_STC);

void prv_lua_to_c_MV_HWS_TEST_GEN_STATUS(
    lua_State *L,
    MV_HWS_TEST_GEN_STATUS *val
)
{
    F_NUMBER(val, -1, totalErrors, GT_U32);
    F_NUMBER(val, -1, goodFrames, GT_U32);
    F_NUMBER(val, -1, checkerLock, GT_U32);
}

void prv_c_to_lua_MV_HWS_TEST_GEN_STATUS(
    lua_State *L,
    MV_HWS_TEST_GEN_STATUS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, totalErrors, GT_U32);
    FO_NUMBER(val, t, goodFrames, GT_U32);
    FO_NUMBER(val, t, checkerLock, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_TEST_GEN_STATUS);

void prv_lua_to_c_CPSS_PORT_MANAGER_CALIBRATION_MODE_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_CALIBRATION_MODE_STC *val
)
{
    F_ENUM(val, -1, calibrationType, CPSS_PORT_MANAGER_CALIBRATION_TYPE_ENT);
    F_STRUCT(val, -1, confidenceCfg, CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC);
}

void prv_c_to_lua_CPSS_PORT_MANAGER_CALIBRATION_MODE_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_CALIBRATION_MODE_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, calibrationType, CPSS_PORT_MANAGER_CALIBRATION_TYPE_ENT);
    FO_STRUCT(val, t, confidenceCfg, CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_MANAGER_CALIBRATION_MODE_STC);

void prv_lua_to_c_MV_HWS_RSFEC_STATUS_STC(
    lua_State *L,
    MV_HWS_RSFEC_STATUS_STC *val
)
{
    F_NUMBER(val, -1, fecAlignStatus, GT_U32);
    F_NUMBER(val, -1, ampsLock, GT_U32);
    F_NUMBER(val, -1, highSer, GT_U32);
    F_ARRAY_START(val, -1, symbolError);
    {
        int idx;
        for (idx = 0; idx < 16; idx++) {
            F_ARRAY_NUMBER(val, symbolError, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, symbolError);
}

void prv_c_to_lua_MV_HWS_RSFEC_STATUS_STC(
    lua_State *L,
    MV_HWS_RSFEC_STATUS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, fecAlignStatus, GT_U32);
    FO_NUMBER(val, t, ampsLock, GT_U32);
    FO_NUMBER(val, t, highSer, GT_U32);
    FO_ARRAY_START(val, t, symbolError);
    {
        int idx;
        for (idx = 0; idx < 16; idx++) {
            FO_ARRAY_NUMBER(val, symbolError, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, symbolError);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_RSFEC_STATUS_STC);

add_mgm_enum(CPSS_PORT_RX_FC_PROFILE_SET_ENT);

void prv_lua_to_c_MV_HWS_TX_TUNE_PARAMS(
    lua_State *L,
    MV_HWS_TX_TUNE_PARAMS *val
)
{
    F_NUMBER(val, -1, txAmp, GT_U32);
    F_NUMBER(val, -1, txEmph0, GT_U32);
    F_NUMBER(val, -1, txEmph1, GT_U32);
}

void prv_c_to_lua_MV_HWS_TX_TUNE_PARAMS(
    lua_State *L,
    MV_HWS_TX_TUNE_PARAMS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, txAmp, GT_U32);
    FO_NUMBER(val, t, txEmph0, GT_U32);
    FO_NUMBER(val, t, txEmph1, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_TX_TUNE_PARAMS);

void prv_lua_to_c_CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC *val
)
{
    F_NUMBER(val, -1, globalParamsBitmapType, GT_U32);
    F_STRUCT(val, -1, signalDetectDbCfg, CPSS_PORT_MANAGER_SIGNAL_DETECT_CONFIG_STC);
    F_BOOL(val, -1, propHighSpeedPortEnabled);
    F_ENUM(val, -1, rxTermination, CPSS_SERDES_TERMINATION);
    F_ENUM(val, -1, anResolutionBit_40G, CPSS_PORT_MANAGER_AN_RESOLUTION_BIT_ENT);
}

void prv_c_to_lua_CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, globalParamsBitmapType, GT_U32);
    FO_STRUCT(val, t, signalDetectDbCfg, CPSS_PORT_MANAGER_SIGNAL_DETECT_CONFIG_STC);
    FO_BOOL(val, t, propHighSpeedPortEnabled);
    FO_ENUM(val, t, rxTermination, CPSS_SERDES_TERMINATION);
    FO_ENUM(val, t, anResolutionBit_40G, CPSS_PORT_MANAGER_AN_RESOLUTION_BIT_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC);

add_mgm_enum(CPSS_PORT_MAC_COUNTERS_ENT);

add_mgm_enum(CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT);

add_mgm_enum(CPSS_PORT_MANAGER_FAILURE_ENT);

void prv_lua_to_c_CPSS_PORT_AP_SERDES_RX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_AP_SERDES_RX_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, squelch, GT_U16);
    F_NUMBER(val, -1, lowFrequency, GT_U8);
    F_NUMBER(val, -1, highFrequency, GT_U8);
    F_NUMBER(val, -1, dcGain, GT_U8);
    F_NUMBER(val, -1, bandWidth, GT_U8);
    F_NUMBER(val, -1, etlMinDelay, GT_U8);
    F_NUMBER(val, -1, etlMaxDelay, GT_U8);
    F_BOOL(val, -1, etlEnable);
    F_NUMBER(val, -1, fieldOverrideBmp, GT_U16);
    F_ENUM(val, -1, serdesSpeed, CPSS_PORT_SERDES_SPEED_ENT);
}

void prv_c_to_lua_CPSS_PORT_AP_SERDES_RX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_AP_SERDES_RX_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, squelch, GT_U16);
    FO_NUMBER(val, t, lowFrequency, GT_U8);
    FO_NUMBER(val, t, highFrequency, GT_U8);
    FO_NUMBER(val, t, dcGain, GT_U8);
    FO_NUMBER(val, t, bandWidth, GT_U8);
    FO_NUMBER(val, t, etlMinDelay, GT_U8);
    FO_NUMBER(val, t, etlMaxDelay, GT_U8);
    FO_BOOL(val, t, etlEnable);
    FO_NUMBER(val, t, fieldOverrideBmp, GT_U16);
    FO_ENUM(val, t, serdesSpeed, CPSS_PORT_SERDES_SPEED_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_AP_SERDES_RX_CONFIG_STC);

void prv_lua_to_c_CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_STC(
    lua_State *L,
    CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_STC *val
)
{
    F_ENUM(val, -1, dfxInstanceType, CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT);
    F_NUMBER(val, -1, dfxInstanceIndex, GT_U32);
}

void prv_c_to_lua_CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_STC(
    lua_State *L,
    CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, dfxInstanceType, CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT);
    FO_NUMBER(val, t, dfxInstanceIndex, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_STC);

void prv_lua_to_c_MV_HWS_IPC_PORT_LOOPBACK_SET_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_LOOPBACK_SET_DATA_STRUCT *val
)
{
    F_NUMBER(val, -1, portGroup, GT_U16);
    F_NUMBER(val, -1, phyPortNum, GT_U16);
    F_NUMBER(val, -1, portMode, GT_U32);
    F_NUMBER(val, -1, lpPlace, GT_U32);
    F_NUMBER(val, -1, lbType, GT_U32);
}

void prv_c_to_lua_MV_HWS_IPC_PORT_LOOPBACK_SET_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_LOOPBACK_SET_DATA_STRUCT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, portGroup, GT_U16);
    FO_NUMBER(val, t, phyPortNum, GT_U16);
    FO_NUMBER(val, t, portMode, GT_U32);
    FO_NUMBER(val, t, lpPlace, GT_U32);
    FO_NUMBER(val, t, lbType, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_IPC_PORT_LOOPBACK_SET_DATA_STRUCT);

add_mgm_enum(CPSS_LED_BLINK_SELECT_ENT);

void prv_lua_to_c_CPSS_PM_PORT_PARAMS_TYPE_UNT(
    lua_State *L,
    CPSS_PM_PORT_PARAMS_TYPE_UNT *val
)
{
    F_UNION_MEMBER_STRUCT(val, -1, regPort, CPSS_PM_PORT_REG_PARAMS_STC);
    F_UNION_MEMBER_STRUCT(val, -1, apPort, CPSS_PM_AP_PORT_PARAMS_STC);
}

void prv_c_to_lua_CPSS_PM_PORT_PARAMS_TYPE_UNT(
    lua_State *L,
    CPSS_PM_PORT_PARAMS_TYPE_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, regPort, CPSS_PM_PORT_REG_PARAMS_STC);
    FO_STRUCT(val, t, apPort, CPSS_PM_AP_PORT_PARAMS_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PM_PORT_PARAMS_TYPE_UNT);
add_mgm_union(CPSS_PM_PORT_PARAMS_TYPE_UNT);

add_mgm_enum(CPSS_PORT_DUPLEX_ENT);

add_mgm_enum(CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT);

void prv_lua_to_c_CPSS_ANP_INFO_STC(
    lua_State *L,
    CPSS_ANP_INFO_STC *val
)
{
    F_NUMBER(val, -1, anRestartCounter, GT_U32);
    F_NUMBER(val, -1, dspLockFailCounter, GT_U32);
    F_NUMBER(val, -1, linkFailCounter, GT_U32);
    F_NUMBER(val, -1, txTrainDuration, GT_U32);
    F_NUMBER(val, -1, txTrainFailCounter, GT_U32);
    F_NUMBER(val, -1, txTrainTimeoutCounter, GT_U32);
    F_NUMBER(val, -1, txTrainOkCounter, GT_U32);
    F_NUMBER(val, -1, rxInitOk, GT_U32);
    F_NUMBER(val, -1, rxInitTimeOut, GT_U32);
    F_NUMBER(val, -1, rxTrainFailed, GT_U32);
    F_NUMBER(val, -1, rxTrainOk, GT_U32);
    F_NUMBER(val, -1, rxTrainTimeOut, GT_U32);
    F_NUMBER(val, -1, chSmState, GT_U32);
    F_NUMBER(val, -1, txSmState, GT_U32);
    F_NUMBER(val, -1, rxSmState, GT_U32);
}

void prv_c_to_lua_CPSS_ANP_INFO_STC(
    lua_State *L,
    CPSS_ANP_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, anRestartCounter, GT_U32);
    FO_NUMBER(val, t, dspLockFailCounter, GT_U32);
    FO_NUMBER(val, t, linkFailCounter, GT_U32);
    FO_NUMBER(val, t, txTrainDuration, GT_U32);
    FO_NUMBER(val, t, txTrainFailCounter, GT_U32);
    FO_NUMBER(val, t, txTrainTimeoutCounter, GT_U32);
    FO_NUMBER(val, t, txTrainOkCounter, GT_U32);
    FO_NUMBER(val, t, rxInitOk, GT_U32);
    FO_NUMBER(val, t, rxInitTimeOut, GT_U32);
    FO_NUMBER(val, t, rxTrainFailed, GT_U32);
    FO_NUMBER(val, t, rxTrainOk, GT_U32);
    FO_NUMBER(val, t, rxTrainTimeOut, GT_U32);
    FO_NUMBER(val, t, chSmState, GT_U32);
    FO_NUMBER(val, t, txSmState, GT_U32);
    FO_NUMBER(val, t, rxSmState, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_ANP_INFO_STC);

add_mgm_enum(CPSS_PM_MAC_PREEMPTION_TYPE_ENT);

add_mgm_enum(MV_HWS_REF_CLOCK_SRC);

add_mgm_enum(MV_HWS_DEV_TYPE);

void prv_lua_to_c_CPSS_PORT_EGRESS_CNTR_STC(
    lua_State *L,
    CPSS_PORT_EGRESS_CNTR_STC *val
)
{
    F_NUMBER(val, -1, outUcFrames, GT_U32);
    F_NUMBER(val, -1, outMcFrames, GT_U32);
    F_NUMBER(val, -1, outBcFrames, GT_U32);
    F_NUMBER(val, -1, brgEgrFilterDisc, GT_U32);
    F_NUMBER(val, -1, txqFilterDisc, GT_U32);
    F_NUMBER(val, -1, outCtrlFrames, GT_U32);
    F_NUMBER(val, -1, egrFrwDropFrames, GT_U32);
    F_NUMBER(val, -1, mcFifo3_0DropPkts, GT_U32);
    F_NUMBER(val, -1, mcFifo7_4DropPkts, GT_U32);
    F_NUMBER(val, -1, mcFifoDropPkts, GT_U32);
    F_NUMBER(val, -1, mcFilterDropPkts, GT_U32);
}

void prv_c_to_lua_CPSS_PORT_EGRESS_CNTR_STC(
    lua_State *L,
    CPSS_PORT_EGRESS_CNTR_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, outUcFrames, GT_U32);
    FO_NUMBER(val, t, outMcFrames, GT_U32);
    FO_NUMBER(val, t, outBcFrames, GT_U32);
    FO_NUMBER(val, t, brgEgrFilterDisc, GT_U32);
    FO_NUMBER(val, t, txqFilterDisc, GT_U32);
    FO_NUMBER(val, t, outCtrlFrames, GT_U32);
    FO_NUMBER(val, t, egrFrwDropFrames, GT_U32);
    FO_NUMBER(val, t, mcFifo3_0DropPkts, GT_U32);
    FO_NUMBER(val, t, mcFifo7_4DropPkts, GT_U32);
    FO_NUMBER(val, t, mcFifoDropPkts, GT_U32);
    FO_NUMBER(val, t, mcFilterDropPkts, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_EGRESS_CNTR_STC);

void prv_lua_to_c_CPSS_SYSTEM_RECOVERY_ERROR_INJECTION_STC(
    lua_State *L,
    CPSS_SYSTEM_RECOVERY_ERROR_INJECTION_STC *val
)
{
    F_BOOL(val, -1, errorInjectionEnable);
    F_ARRAY_START(val, -1, errorBmp);
    {
        int idx;
        for (idx = 0; idx < 3; idx++) {
            F_ARRAY_NUMBER(val, errorBmp, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, errorBmp);
}

void prv_c_to_lua_CPSS_SYSTEM_RECOVERY_ERROR_INJECTION_STC(
    lua_State *L,
    CPSS_SYSTEM_RECOVERY_ERROR_INJECTION_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, errorInjectionEnable);
    FO_ARRAY_START(val, t, errorBmp);
    {
        int idx;
        for (idx = 0; idx < 3; idx++) {
            FO_ARRAY_NUMBER(val, errorBmp, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, errorBmp);
    lua_settop(L, t);
}
add_mgm_type(CPSS_SYSTEM_RECOVERY_ERROR_INJECTION_STC);

add_mgm_enum(MV_HWS_PORT_TEST_GEN_PATTERN);

void prv_lua_to_c_MV_HWS_COMPHY_C12GP41P2V_AUTO_TUNE_RESULTS(
    lua_State *L,
    MV_HWS_COMPHY_C12GP41P2V_AUTO_TUNE_RESULTS *val
)
{
    F_STRUCT(val, -1, txComphyC12GP41P2V, MV_HWS_MAN_TUNE_COMPHY_C12G_TX_CONFIG_DATA);
    F_STRUCT(val, -1, rxComphyC12GP41P2V, MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_RX_CONFIG_DATA);
    F_NUMBER(val, -1, eo, GT_U8);
    F_NUMBER(val, -1, align90, GT_U16);
    F_NUMBER(val, -1, sampler, GT_U16);
    F_NUMBER(val, -1, slewRateCtrl0, GT_U32);
    F_NUMBER(val, -1, slewRateCtrl1, GT_U32);
    F_ARRAY_START(val, -1, dfe);
    {
        int idx;
        for (idx = 0; idx < 12; idx++) {
            F_ARRAY_NUMBER(val, dfe, idx, GT_32);
        }
    }
    F_ARRAY_END(val, -1, dfe);
    F_NUMBER(val, -1, ffeSettingForce, GT_U8);
    F_NUMBER(val, -1, adaptedResSel, GT_U8);
    F_NUMBER(val, -1, adaptedCapSel, GT_U8);
    F_NUMBER(val, -1, analogPre, GT_U8);
    F_NUMBER(val, -1, analogPeak, GT_U8);
    F_NUMBER(val, -1, analogPost, GT_U8);
}

void prv_c_to_lua_MV_HWS_COMPHY_C12GP41P2V_AUTO_TUNE_RESULTS(
    lua_State *L,
    MV_HWS_COMPHY_C12GP41P2V_AUTO_TUNE_RESULTS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, txComphyC12GP41P2V, MV_HWS_MAN_TUNE_COMPHY_C12G_TX_CONFIG_DATA);
    FO_STRUCT(val, t, rxComphyC12GP41P2V, MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_RX_CONFIG_DATA);
    FO_NUMBER(val, t, eo, GT_U8);
    FO_NUMBER(val, t, align90, GT_U16);
    FO_NUMBER(val, t, sampler, GT_U16);
    FO_NUMBER(val, t, slewRateCtrl0, GT_U32);
    FO_NUMBER(val, t, slewRateCtrl1, GT_U32);
    FO_ARRAY_START(val, t, dfe);
    {
        int idx;
        for (idx = 0; idx < 12; idx++) {
            FO_ARRAY_NUMBER(val, dfe, idx, GT_32);
        }
    }
    FO_ARRAY_END(val, t, dfe);
    FO_NUMBER(val, t, ffeSettingForce, GT_U8);
    FO_NUMBER(val, t, adaptedResSel, GT_U8);
    FO_NUMBER(val, t, adaptedCapSel, GT_U8);
    FO_NUMBER(val, t, analogPre, GT_U8);
    FO_NUMBER(val, t, analogPeak, GT_U8);
    FO_NUMBER(val, t, analogPost, GT_U8);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_COMPHY_C12GP41P2V_AUTO_TUNE_RESULTS);

add_mgm_enum(MV_HWS_PORT_AUTO_TUNE_MODE);

add_mgm_enum(MV_HWS_SERDES_ENCODING_TYPE);

void prv_lua_to_c_MV_HWS_SERDES_RX_OVERRIDE_CONFIG_DATA_UNT(
    lua_State *L,
    MV_HWS_SERDES_RX_OVERRIDE_CONFIG_DATA_UNT *val
)
{
    F_UNION_MEMBER_STRUCT(val, -1, rxAvago, MV_HWS_AVAGO_RX_OVERRIDE_CONFIG_DATA);
    F_UNION_MEMBER_STRUCT(val, -1, rxComphyC12GP41P2V, MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_RX_CONFIG_DATA);
}

void prv_c_to_lua_MV_HWS_SERDES_RX_OVERRIDE_CONFIG_DATA_UNT(
    lua_State *L,
    MV_HWS_SERDES_RX_OVERRIDE_CONFIG_DATA_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, rxAvago, MV_HWS_AVAGO_RX_OVERRIDE_CONFIG_DATA);
    FO_STRUCT(val, t, rxComphyC12GP41P2V, MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_RX_CONFIG_DATA);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_SERDES_RX_OVERRIDE_CONFIG_DATA_UNT);
add_mgm_union(MV_HWS_SERDES_RX_OVERRIDE_CONFIG_DATA_UNT);

void prv_lua_to_c_MV_HWS_SERDES_RX_DATAPATH_CONFIG_STC(
    lua_State *L,
    MV_HWS_SERDES_RX_DATAPATH_CONFIG_STC *val
)
{
    F_BOOL(val, -1, polarityInvert);
    F_BOOL(val, -1, grayEnable);
    F_BOOL(val, -1, precodeEnable);
    F_BOOL(val, -1, swizzleEnable);
}

void prv_c_to_lua_MV_HWS_SERDES_RX_DATAPATH_CONFIG_STC(
    lua_State *L,
    MV_HWS_SERDES_RX_DATAPATH_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, polarityInvert);
    FO_BOOL(val, t, grayEnable);
    FO_BOOL(val, t, precodeEnable);
    FO_BOOL(val, t, swizzleEnable);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_SERDES_RX_DATAPATH_CONFIG_STC);

void prv_lua_to_c_MV_HWS_COMPHY_C28GP4_PRESET_OVERRIDE(
    lua_State *L,
    MV_HWS_COMPHY_C28GP4_PRESET_OVERRIDE *val
)
{
    F_NUMBER(val, -1, localCtlePresetDefault, GT_U32);
    F_NUMBER(val, -1, localCtlePresetRes1, GT_U32);
    F_NUMBER(val, -1, localCtlePresetCap1, GT_U32);
    F_NUMBER(val, -1, localCtlePresetRes2_O, GT_U32);
    F_NUMBER(val, -1, localCtlePresetRes2_E, GT_U32);
    F_NUMBER(val, -1, localCtlePresetCap2_O, GT_U32);
    F_NUMBER(val, -1, localCtlePresetCap2_E, GT_U32);
    F_NUMBER(val, -1, localCtlePresetPhase, GT_U32);
    F_NUMBER(val, -1, txLocalPreset, GT_U32);
    F_NUMBER(val, -1, remotePresetRequestType, GT_U32);
}

void prv_c_to_lua_MV_HWS_COMPHY_C28GP4_PRESET_OVERRIDE(
    lua_State *L,
    MV_HWS_COMPHY_C28GP4_PRESET_OVERRIDE *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, localCtlePresetDefault, GT_U32);
    FO_NUMBER(val, t, localCtlePresetRes1, GT_U32);
    FO_NUMBER(val, t, localCtlePresetCap1, GT_U32);
    FO_NUMBER(val, t, localCtlePresetRes2_O, GT_U32);
    FO_NUMBER(val, t, localCtlePresetRes2_E, GT_U32);
    FO_NUMBER(val, t, localCtlePresetCap2_O, GT_U32);
    FO_NUMBER(val, t, localCtlePresetCap2_E, GT_U32);
    FO_NUMBER(val, t, localCtlePresetPhase, GT_U32);
    FO_NUMBER(val, t, txLocalPreset, GT_U32);
    FO_NUMBER(val, t, remotePresetRequestType, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_COMPHY_C28GP4_PRESET_OVERRIDE);

void prv_lua_to_c_MV_HWS_AP_PORT_STATS(
    lua_State *L,
    MV_HWS_AP_PORT_STATS *val
)
{
    F_NUMBER(val, -1, txDisCnt, GT_U16);
    F_NUMBER(val, -1, abilityCnt, GT_U16);
    F_NUMBER(val, -1, abilitySuccessCnt, GT_U16);
    F_NUMBER(val, -1, linkFailCnt, GT_U16);
    F_NUMBER(val, -1, linkSuccessCnt, GT_U16);
    F_NUMBER(val, -1, hcdResoultionTime, GT_U32);
    F_NUMBER(val, -1, linkUpTime, GT_U32);
    F_NUMBER(val, -1, rxTrainDuration, GT_U32);
}

void prv_c_to_lua_MV_HWS_AP_PORT_STATS(
    lua_State *L,
    MV_HWS_AP_PORT_STATS *val
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
    FO_NUMBER(val, t, rxTrainDuration, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_AP_PORT_STATS);

add_mgm_enum(CPSS_PORT_PA_BW_MODE_ENT);

add_mgm_enum(CPSS_PORT_PERIODIC_FLOW_CONTROL_TYPE_ENT);

void prv_lua_to_c_CPSS_FORMAT_CONVERT_FIELD_VAL_CONVERT_STC(
    lua_State *L,
    CPSS_FORMAT_CONVERT_FIELD_VAL_CONVERT_STC *val
)
{
    F_NUMBER(val, -1, stcValue, GT_U32);
    F_NUMBER(val, -1, rawValue, GT_U32);
}

void prv_c_to_lua_CPSS_FORMAT_CONVERT_FIELD_VAL_CONVERT_STC(
    lua_State *L,
    CPSS_FORMAT_CONVERT_FIELD_VAL_CONVERT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, stcValue, GT_U32);
    FO_NUMBER(val, t, rawValue, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_FORMAT_CONVERT_FIELD_VAL_CONVERT_STC);

add_mgm_enum(CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT);

add_mgm_enum(CPSS_DIAG_PG_IF_SIZE_ENT);

add_mgm_enum(MV_HWS_PORT_CRC_MODE);

add_mgm_enum(CPSS_PORT_SPEED_ENT);

add_mgm_enum(MV_HWS_PORT_LB_TYPE);

add_mgm_enum(MV_HWS_CPLL_OUTPUT_FREQUENCY);

add_mgm_enum(MV_HWS_D2D_DIRECTION_MODE_ENT);

add_mgm_enum(MV_HWS_UNITS_ID);

add_mgm_enum(MV_HWS_REF_CLOCK_SUP_VAL);

void prv_lua_to_c_MV_HWS_SERDES_PRESET_OVERRIDE_UNT(
    lua_State *L,
    MV_HWS_SERDES_PRESET_OVERRIDE_UNT *val
)
{
    F_UNION_MEMBER_STRUCT(val, -1, comphyC28GP4PresetOverride, MV_HWS_COMPHY_C28GP4_PRESET_OVERRIDE);
}

void prv_c_to_lua_MV_HWS_SERDES_PRESET_OVERRIDE_UNT(
    lua_State *L,
    MV_HWS_SERDES_PRESET_OVERRIDE_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, comphyC28GP4PresetOverride, MV_HWS_COMPHY_C28GP4_PRESET_OVERRIDE);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_SERDES_PRESET_OVERRIDE_UNT);
add_mgm_union(MV_HWS_SERDES_PRESET_OVERRIDE_UNT);

add_mgm_enum(CPSS_PHY_XSMI_INTERFACE_ENT);

add_mgm_enum(MV_HWS_PORT_SPEED_ENT);

void prv_lua_to_c_CPSS_PORT_MAC_MTI_RX_COUNTER_SET_STC(
    lua_State *L,
    CPSS_PORT_MAC_MTI_RX_COUNTER_SET_STC *val
)
{
    F_STRUCT(val, -1, etherStatsOctets, GT_U64);
    F_STRUCT(val, -1, ifInOctets, GT_U64);
    F_STRUCT(val, -1, aAlignmentErrors, GT_U64);
    F_STRUCT(val, -1, aMACControlFramesReceived, GT_U64);
    F_STRUCT(val, -1, aFrameTooLongErrors, GT_U64);
    F_STRUCT(val, -1, aInRangeLengthErrors, GT_U64);
    F_STRUCT(val, -1, aFramesReceivedOK, GT_U64);
    F_STRUCT(val, -1, aFrameCheckSequenceErrors, GT_U64);
    F_STRUCT(val, -1, VLANRecievedOK, GT_U64);
    F_STRUCT(val, -1, ifInErrors, GT_U64);
    F_STRUCT(val, -1, ifInUcastPkts, GT_U64);
    F_STRUCT(val, -1, ifInMulticastPkts, GT_U64);
    F_STRUCT(val, -1, ifInBroadcastPkts, GT_U64);
    F_STRUCT(val, -1, etherStatsDropEvents, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts, GT_U64);
    F_STRUCT(val, -1, etherStatsUndersizePkts, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts64Octets, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts65to127Octets, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts128to255Octets, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts256to511Octets, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts512to1023Octets, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts1024to1518Octets, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts1519toMaxOctets, GT_U64);
    F_STRUCT(val, -1, etherStatsOversizePkts, GT_U64);
    F_STRUCT(val, -1, etherStatsJabbers, GT_U64);
    F_STRUCT(val, -1, etherStatsFragments, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_0, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_1, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_2, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_3, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_4, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_5, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_6, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_7, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_8, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_9, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_10, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_11, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_12, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_13, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_14, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_15, GT_U64);
    F_STRUCT(val, -1, aPAUSEMACCtrlFramesReceived, GT_U64);
}

void prv_c_to_lua_CPSS_PORT_MAC_MTI_RX_COUNTER_SET_STC(
    lua_State *L,
    CPSS_PORT_MAC_MTI_RX_COUNTER_SET_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, etherStatsOctets, GT_U64);
    FO_STRUCT(val, t, ifInOctets, GT_U64);
    FO_STRUCT(val, t, aAlignmentErrors, GT_U64);
    FO_STRUCT(val, t, aMACControlFramesReceived, GT_U64);
    FO_STRUCT(val, t, aFrameTooLongErrors, GT_U64);
    FO_STRUCT(val, t, aInRangeLengthErrors, GT_U64);
    FO_STRUCT(val, t, aFramesReceivedOK, GT_U64);
    FO_STRUCT(val, t, aFrameCheckSequenceErrors, GT_U64);
    FO_STRUCT(val, t, VLANRecievedOK, GT_U64);
    FO_STRUCT(val, t, ifInErrors, GT_U64);
    FO_STRUCT(val, t, ifInUcastPkts, GT_U64);
    FO_STRUCT(val, t, ifInMulticastPkts, GT_U64);
    FO_STRUCT(val, t, ifInBroadcastPkts, GT_U64);
    FO_STRUCT(val, t, etherStatsDropEvents, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts, GT_U64);
    FO_STRUCT(val, t, etherStatsUndersizePkts, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts64Octets, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts65to127Octets, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts128to255Octets, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts256to511Octets, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts512to1023Octets, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts1024to1518Octets, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts1519toMaxOctets, GT_U64);
    FO_STRUCT(val, t, etherStatsOversizePkts, GT_U64);
    FO_STRUCT(val, t, etherStatsJabbers, GT_U64);
    FO_STRUCT(val, t, etherStatsFragments, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_0, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_1, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_2, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_3, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_4, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_5, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_6, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_7, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_8, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_9, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_10, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_11, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_12, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_13, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_14, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_15, GT_U64);
    FO_STRUCT(val, t, aPAUSEMACCtrlFramesReceived, GT_U64);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_MAC_MTI_RX_COUNTER_SET_STC);

void prv_lua_to_c_CPSS_802_1Q_VLAN_TAG_STC(
    lua_State *L,
    CPSS_802_1Q_VLAN_TAG_STC *val
)
{
    F_NUMBER(val, -1, TPID, GT_U32);
    F_NUMBER(val, -1, PCP, GT_U32);
    F_NUMBER(val, -1, DEI, GT_U32);
    F_NUMBER(val, -1, VID, GT_U32);
}

void prv_c_to_lua_CPSS_802_1Q_VLAN_TAG_STC(
    lua_State *L,
    CPSS_802_1Q_VLAN_TAG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, TPID, GT_U32);
    FO_NUMBER(val, t, PCP, GT_U32);
    FO_NUMBER(val, t, DEI, GT_U32);
    FO_NUMBER(val, t, VID, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_802_1Q_VLAN_TAG_STC);

add_mgm_enum(MV_HWS_SERDES_MEDIA);

void prv_lua_to_c_MV_HWS_MAN_TUNE_COMPHY_H_TX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_H_TX_CONFIG_DATA *val
)
{
    F_NUMBER(val, -1, txAmp, GT_U32);
    F_NUMBER(val, -1, txAmpAdj, GT_U32);
    F_NUMBER(val, -1, txEmph0, GT_32);
    F_NUMBER(val, -1, txEmph1, GT_32);
    F_NUMBER(val, -1, txAmpShft, GT_U32);
    F_NUMBER(val, -1, txEmphEn, GT_U32);
    F_NUMBER(val, -1, slewRate, GT_U32);
    F_NUMBER(val, -1, slewCtrlEn, GT_U32);
}

void prv_c_to_lua_MV_HWS_MAN_TUNE_COMPHY_H_TX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_H_TX_CONFIG_DATA *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, txAmp, GT_U32);
    FO_NUMBER(val, t, txAmpAdj, GT_U32);
    FO_NUMBER(val, t, txEmph0, GT_32);
    FO_NUMBER(val, t, txEmph1, GT_32);
    FO_NUMBER(val, t, txAmpShft, GT_U32);
    FO_NUMBER(val, t, txEmphEn, GT_U32);
    FO_NUMBER(val, t, slewRate, GT_U32);
    FO_NUMBER(val, t, slewCtrlEn, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_MAN_TUNE_COMPHY_H_TX_CONFIG_DATA);

add_mgm_enum(MV_HWS_AP_DEBUG_LOG_OUTPUT_ENT);

void prv_lua_to_c_CPSS_CYCLIC_LOGGER_LOG_ENTRY(
    lua_State *L,
    CPSS_CYCLIC_LOGGER_LOG_ENTRY *val
)
{
    F_NUMBER(val, -1, psudoTimeStamp, GT_U32);
    F_ARRAY_START(val, -1, timeStamp);
    {
        int idx;
        for (idx = 0; idx < 3; idx++) {
            F_ARRAY_NUMBER(val, timeStamp, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, timeStamp);
    F_ARRAY_START(val, -1, info);
    {
        int idx;
        for (idx = 0; idx < 256; idx++) {
            F_ARRAY_NUMBER(val, info, idx, GT_U8);
        }
    }
    F_ARRAY_END(val, -1, info);
    F_NUMBER(val, -1, line, GT_U32);
    F_NUMBER(val, -1, devNum, GT_SW_DEV_NUM);
    F_NUMBER(val, -1, port, GT_U32);
}

void prv_c_to_lua_CPSS_CYCLIC_LOGGER_LOG_ENTRY(
    lua_State *L,
    CPSS_CYCLIC_LOGGER_LOG_ENTRY *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, psudoTimeStamp, GT_U32);
    FO_ARRAY_START(val, t, timeStamp);
    {
        int idx;
        for (idx = 0; idx < 3; idx++) {
            FO_ARRAY_NUMBER(val, timeStamp, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, timeStamp);
    FO_ARRAY_START(val, t, info);
    {
        int idx;
        for (idx = 0; idx < 256; idx++) {
            FO_ARRAY_NUMBER(val, info, idx, GT_U8);
        }
    }
    FO_ARRAY_END(val, t, info);
    FO_NUMBER(val, t, line, GT_U32);
    FO_NUMBER(val, t, devNum, GT_SW_DEV_NUM);
    FO_NUMBER(val, t, port, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_CYCLIC_LOGGER_LOG_ENTRY);

void prv_lua_to_c_MV_HWS_IPC_PORT_GENERAL_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_GENERAL_STRUCT *val
)
{
    F_NUMBER(val, -1, portGroup, GT_U16);
    F_NUMBER(val, -1, phyPortNum, GT_U16);
    F_NUMBER(val, -1, portMode, GT_U32);
    F_ARRAY_START(val, -1, portMiscData);
    {
        int idx;
        for (idx = 0; idx < 40; idx++) {
            F_ARRAY_NUMBER(val, portMiscData, idx, GT_U8);
        }
    }
    F_ARRAY_END(val, -1, portMiscData);
}

void prv_c_to_lua_MV_HWS_IPC_PORT_GENERAL_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_GENERAL_STRUCT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, portGroup, GT_U16);
    FO_NUMBER(val, t, phyPortNum, GT_U16);
    FO_NUMBER(val, t, portMode, GT_U32);
    FO_ARRAY_START(val, t, portMiscData);
    {
        int idx;
        for (idx = 0; idx < 40; idx++) {
            FO_ARRAY_NUMBER(val, portMiscData, idx, GT_U8);
        }
    }
    FO_ARRAY_END(val, t, portMiscData);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_IPC_PORT_GENERAL_STRUCT);

add_mgm_enum(CPSS_DP_FOR_RX_ENT);

void prv_lua_to_c_MV_HWS_PORT_CTRL_LOG_ENTRY(
    lua_State *L,
    MV_HWS_PORT_CTRL_LOG_ENTRY *val
)
{
    F_NUMBER(val, -1, timestamp, GT_U32);
    F_NUMBER(val, -1, info, GT_U32);
}

void prv_c_to_lua_MV_HWS_PORT_CTRL_LOG_ENTRY(
    lua_State *L,
    MV_HWS_PORT_CTRL_LOG_ENTRY *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, timestamp, GT_U32);
    FO_NUMBER(val, t, info, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_PORT_CTRL_LOG_ENTRY);

void prv_lua_to_c_CPSS_PORT_AVAGO_SERDES_TX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_AVAGO_SERDES_TX_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, atten, GT_U32);
    F_NUMBER(val, -1, post, GT_32);
    F_NUMBER(val, -1, pre, GT_32);
    F_NUMBER(val, -1, pre2, GT_32);
    F_NUMBER(val, -1, pre3, GT_32);
}

void prv_c_to_lua_CPSS_PORT_AVAGO_SERDES_TX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_AVAGO_SERDES_TX_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, atten, GT_U32);
    FO_NUMBER(val, t, post, GT_32);
    FO_NUMBER(val, t, pre, GT_32);
    FO_NUMBER(val, t, pre2, GT_32);
    FO_NUMBER(val, t, pre3, GT_32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_AVAGO_SERDES_TX_CONFIG_STC);

add_mgm_enum(CPSS_TX_SDMA_QUEUE_MODE_ENT);

void prv_lua_to_c_CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC(
    lua_State *L,
    CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC *val
)
{
    F_NUMBER(val, -1, devNum, GT_U32);
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* cannot identify type classUNKNOWN_PTR
CPSS_APP_PLATFORM_PORT_CONFIG_STC   *portTypeListPtr; */
#endif
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* cannot identify type classUNKNOWN_PTR
CPSS_APP_PLATFORM_EXT_PHY_PORT_MAP  *phyMapListPtr; */
#endif
    F_NUMBER(val, -1, phyMapListPtrSize, GT_U32);
}

void prv_c_to_lua_CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC(
    lua_State *L,
    CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, devNum, GT_U32);
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* cannot identify type classUNKNOWN_PTR
CPSS_APP_PLATFORM_PORT_CONFIG_STC   *portTypeListPtr; */
#endif
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* cannot identify type classUNKNOWN_PTR
CPSS_APP_PLATFORM_EXT_PHY_PORT_MAP  *phyMapListPtr; */
#endif
    FO_NUMBER(val, t, phyMapListPtrSize, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC);

add_mgm_enum(MV_HWS_SERDES_TEST_GEN_MODE);

add_mgm_enum(CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_ENT);

void prv_lua_to_c_CPSS_PORT_ATTRIBUTES_STC(
    lua_State *L,
    CPSS_PORT_ATTRIBUTES_STC *val
)
{
    F_BOOL(val, -1, portLinkUp);
    F_ENUM(val, -1, portSpeed, CPSS_PORT_SPEED_ENT);
    F_ENUM(val, -1, portDuplexity, CPSS_PORT_DUPLEX_ENT);
}

void prv_c_to_lua_CPSS_PORT_ATTRIBUTES_STC(
    lua_State *L,
    CPSS_PORT_ATTRIBUTES_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, portLinkUp);
    FO_ENUM(val, t, portSpeed, CPSS_PORT_SPEED_ENT);
    FO_ENUM(val, t, portDuplexity, CPSS_PORT_DUPLEX_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_ATTRIBUTES_STC);

add_mgm_enum(CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT);

void prv_lua_to_c_MV_HWS_LOG_ENTRY(
    lua_State *L,
    MV_HWS_LOG_ENTRY *val
)
{
    F_NUMBER(val, -1, timestamp, GT_U32);
    F_ARRAY_START(val, -1, info);
    {
        int idx;
        for (idx = 0; idx < 60; idx++) {
            F_ARRAY_NUMBER(val, info, idx, char);
        }
    }
    F_ARRAY_END(val, -1, info);
}

void prv_c_to_lua_MV_HWS_LOG_ENTRY(
    lua_State *L,
    MV_HWS_LOG_ENTRY *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, timestamp, GT_U32);
    FO_ARRAY_START(val, t, info);
    {
        int idx;
        for (idx = 0; idx < 60; idx++) {
            FO_ARRAY_NUMBER(val, info, idx, char);
        }
    }
    FO_ARRAY_END(val, t, info);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_LOG_ENTRY);

void prv_lua_to_c_CPSS_PORTS_BMP_STC(
    lua_State *L,
    CPSS_PORTS_BMP_STC *val
)
{
    F_ARRAY_START(val, -1, ports);
    {
        int idx;
        for (idx = 0; idx < 32; idx++) {
            F_ARRAY_NUMBER(val, ports, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, ports);
}

void prv_c_to_lua_CPSS_PORTS_BMP_STC(
    lua_State *L,
    CPSS_PORTS_BMP_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, ports);
    {
        int idx;
        for (idx = 0; idx < 32; idx++) {
            FO_ARRAY_NUMBER(val, ports, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, ports);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORTS_BMP_STC);

void prv_lua_to_c_CPSS_PM_AP_MODE_LANE_PARAM_STC(
    lua_State *L,
    CPSS_PM_AP_MODE_LANE_PARAM_STC *val
)
{
    F_NUMBER(val, -1, validApLaneParamsBitMask, GT_U32);
    F_STRUCT(val, -1, txParams, CPSS_PORT_SERDES_TX_CONFIG_STC);
    F_STRUCT(val, -1, rxParams, CPSS_PORT_SERDES_RX_CONFIG_STC);
    F_NUMBER(val, -1, etlMinDelay, GT_U8);
    F_NUMBER(val, -1, etlMaxDelay, GT_U8);
}

void prv_c_to_lua_CPSS_PM_AP_MODE_LANE_PARAM_STC(
    lua_State *L,
    CPSS_PM_AP_MODE_LANE_PARAM_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, validApLaneParamsBitMask, GT_U32);
    FO_STRUCT(val, t, txParams, CPSS_PORT_SERDES_TX_CONFIG_STC);
    FO_STRUCT(val, t, rxParams, CPSS_PORT_SERDES_RX_CONFIG_STC);
    FO_NUMBER(val, t, etlMinDelay, GT_U8);
    FO_NUMBER(val, t, etlMaxDelay, GT_U8);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PM_AP_MODE_LANE_PARAM_STC);

void prv_lua_to_c_MV_HWS_PORT_STATUS_STC(
    lua_State *L,
    MV_HWS_PORT_STATUS_STC *val
)
{
    F_NUMBER(val, -1, rsfecAligned, GT_U32);
    F_NUMBER(val, -1, hiBer, GT_U32);
    F_NUMBER(val, -1, alignDone, GT_U32);
    F_NUMBER(val, -1, lpcsAnDone, GT_U32);
    F_NUMBER(val, -1, lpcsRxSync, GT_U32);
    F_NUMBER(val, -1, lpcsLinkStatus, GT_U32);
    F_NUMBER(val, -1, hiSer, GT_U32);
    F_NUMBER(val, -1, degradeSer, GT_U32);
    F_NUMBER(val, -1, alignLock, GT_U32);
    F_NUMBER(val, -1, linkStatus, GT_U32);
    F_NUMBER(val, -1, linkOk, GT_U32);
    F_NUMBER(val, -1, phyLos, GT_U32);
    F_NUMBER(val, -1, rxRemFault, GT_U32);
    F_NUMBER(val, -1, rxLocFault, GT_U32);
    F_NUMBER(val, -1, fault, GT_U32);
    F_NUMBER(val, -1, pcsReceiveLink, GT_U32);
    F_NUMBER(val, -1, devicePresent, GT_U32);
    F_NUMBER(val, -1, transmitFault, GT_U32);
    F_NUMBER(val, -1, receiveFault, GT_U32);
    F_NUMBER(val, -1, berCounter, GT_U32);
    F_NUMBER(val, -1, errorBlockCounter, GT_U32);
}

void prv_c_to_lua_MV_HWS_PORT_STATUS_STC(
    lua_State *L,
    MV_HWS_PORT_STATUS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, rsfecAligned, GT_U32);
    FO_NUMBER(val, t, hiBer, GT_U32);
    FO_NUMBER(val, t, alignDone, GT_U32);
    FO_NUMBER(val, t, lpcsAnDone, GT_U32);
    FO_NUMBER(val, t, lpcsRxSync, GT_U32);
    FO_NUMBER(val, t, lpcsLinkStatus, GT_U32);
    FO_NUMBER(val, t, hiSer, GT_U32);
    FO_NUMBER(val, t, degradeSer, GT_U32);
    FO_NUMBER(val, t, alignLock, GT_U32);
    FO_NUMBER(val, t, linkStatus, GT_U32);
    FO_NUMBER(val, t, linkOk, GT_U32);
    FO_NUMBER(val, t, phyLos, GT_U32);
    FO_NUMBER(val, t, rxRemFault, GT_U32);
    FO_NUMBER(val, t, rxLocFault, GT_U32);
    FO_NUMBER(val, t, fault, GT_U32);
    FO_NUMBER(val, t, pcsReceiveLink, GT_U32);
    FO_NUMBER(val, t, devicePresent, GT_U32);
    FO_NUMBER(val, t, transmitFault, GT_U32);
    FO_NUMBER(val, t, receiveFault, GT_U32);
    FO_NUMBER(val, t, berCounter, GT_U32);
    FO_NUMBER(val, t, errorBlockCounter, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_PORT_STATUS_STC);

void prv_lua_to_c_MV_HWS_D2D_PRBS_CONFIG_STC(
    lua_State *L,
    MV_HWS_D2D_PRBS_CONFIG_STC *val
)
{
    F_ENUM(val, -1, direction, MV_HWS_D2D_DIRECTION_MODE_ENT);
    F_ENUM(val, -1, fecMode, MV_HWS_PORT_FEC_MODE);
    F_BOOL(val, -1, userDefPatternEnabled);
    F_ARRAY_START(val, -1, userDefPattern);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            F_ARRAY_NUMBER(val, userDefPattern, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, userDefPattern);
    F_ENUM(val, -1, serdesPattern, MV_HWS_SERDES_TX_PATTERN);
    F_ENUM(val, -1, mode, MV_HWS_SERDES_TEST_GEN_MODE);
    F_ENUM(val, -1, portPattern, MV_HWS_PORT_TEST_GEN_PATTERN);
}

void prv_c_to_lua_MV_HWS_D2D_PRBS_CONFIG_STC(
    lua_State *L,
    MV_HWS_D2D_PRBS_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, direction, MV_HWS_D2D_DIRECTION_MODE_ENT);
    FO_ENUM(val, t, fecMode, MV_HWS_PORT_FEC_MODE);
    FO_BOOL(val, t, userDefPatternEnabled);
    FO_ARRAY_START(val, t, userDefPattern);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            FO_ARRAY_NUMBER(val, userDefPattern, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, userDefPattern);
    FO_ENUM(val, t, serdesPattern, MV_HWS_SERDES_TX_PATTERN);
    FO_ENUM(val, t, mode, MV_HWS_SERDES_TEST_GEN_MODE);
    FO_ENUM(val, t, portPattern, MV_HWS_PORT_TEST_GEN_PATTERN);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_D2D_PRBS_CONFIG_STC);

void prv_lua_to_c_CPSS_PORT_MAC_MTI_TX_COUNTER_SET_STC(
    lua_State *L,
    CPSS_PORT_MAC_MTI_TX_COUNTER_SET_STC *val
)
{
    F_STRUCT(val, -1, ifOutOctets, GT_U64);
    F_STRUCT(val, -1, octetsTransmittedOk, GT_U64);
    F_STRUCT(val, -1, aPAUSEMACCtrlFramesTransmitted, GT_U64);
    F_STRUCT(val, -1, aFramesTransmittedOK, GT_U64);
    F_STRUCT(val, -1, VLANTransmittedOK, GT_U64);
    F_STRUCT(val, -1, ifOutErrors, GT_U64);
    F_STRUCT(val, -1, ifOutUcastPkts, GT_U64);
    F_STRUCT(val, -1, ifOutMulticastPkts, GT_U64);
    F_STRUCT(val, -1, ifOutBroadcastPkts, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts64Octets, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts65to127Octets, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts128to255Octets, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts256to511Octets, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts512to1023Octets, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts1024to1518Octets, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts1519toMaxOctets, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_0, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_1, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_2, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_3, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_4, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_5, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_6, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_7, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_8, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_9, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_10, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_11, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_12, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_13, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_14, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_15, GT_U64);
    F_STRUCT(val, -1, aMACControlFramesTransmitted, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts, GT_U64);
}

void prv_c_to_lua_CPSS_PORT_MAC_MTI_TX_COUNTER_SET_STC(
    lua_State *L,
    CPSS_PORT_MAC_MTI_TX_COUNTER_SET_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, ifOutOctets, GT_U64);
    FO_STRUCT(val, t, octetsTransmittedOk, GT_U64);
    FO_STRUCT(val, t, aPAUSEMACCtrlFramesTransmitted, GT_U64);
    FO_STRUCT(val, t, aFramesTransmittedOK, GT_U64);
    FO_STRUCT(val, t, VLANTransmittedOK, GT_U64);
    FO_STRUCT(val, t, ifOutErrors, GT_U64);
    FO_STRUCT(val, t, ifOutUcastPkts, GT_U64);
    FO_STRUCT(val, t, ifOutMulticastPkts, GT_U64);
    FO_STRUCT(val, t, ifOutBroadcastPkts, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts64Octets, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts65to127Octets, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts128to255Octets, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts256to511Octets, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts512to1023Octets, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts1024to1518Octets, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts1519toMaxOctets, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_0, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_1, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_2, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_3, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_4, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_5, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_6, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_7, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_8, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_9, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_10, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_11, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_12, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_13, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_14, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_15, GT_U64);
    FO_STRUCT(val, t, aMACControlFramesTransmitted, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts, GT_U64);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_MAC_MTI_TX_COUNTER_SET_STC);

void prv_lua_to_c_CPSS_APP_PLATFORM_BOARD_PROFILE_STC(
    lua_State *L,
    CPSS_APP_PLATFORM_BOARD_PROFILE_STC *val
)
{
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* cannot identify type classUNKNOWN
CPSS_APP_PLATFORM_BOARD_PARAM_ENT boardInfoType; */
#endif
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* cannot identify type classUNKNOWN
CPSS_APP_PLATFORM_BOARD_PARAM_UNT boardParam; */
#endif
}

void prv_c_to_lua_CPSS_APP_PLATFORM_BOARD_PROFILE_STC(
    lua_State *L,
    CPSS_APP_PLATFORM_BOARD_PROFILE_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* cannot identify type classUNKNOWN
CPSS_APP_PLATFORM_BOARD_PARAM_ENT boardInfoType; */
#endif
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* cannot identify type classUNKNOWN
CPSS_APP_PLATFORM_BOARD_PARAM_UNT boardParam; */
#endif
    lua_settop(L, t);
}
add_mgm_type(CPSS_APP_PLATFORM_BOARD_PROFILE_STC);

void prv_lua_to_c_MV_HWS_MAN_TUNE_COMPHY_C28G_TX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_C28G_TX_CONFIG_DATA *val
)
{
    F_NUMBER(val, -1, pre, GT_U8);
    F_NUMBER(val, -1, peak, GT_U8);
    F_NUMBER(val, -1, post, GT_U8);
}

void prv_c_to_lua_MV_HWS_MAN_TUNE_COMPHY_C28G_TX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_C28G_TX_CONFIG_DATA *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, pre, GT_U8);
    FO_NUMBER(val, t, peak, GT_U8);
    FO_NUMBER(val, t, post, GT_U8);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_MAN_TUNE_COMPHY_C28G_TX_CONFIG_DATA);

add_mgm_enum(MV_HWS_SERDES_DUMP_TYPE_ENT);

void prv_lua_to_c_MV_HWS_IPC_LINK_BINDING_STRUCT(
    lua_State *L,
    MV_HWS_IPC_LINK_BINDING_STRUCT *val
)
{
    F_NUMBER(val, -1, portGroup, GT_U16);
    F_NUMBER(val, -1, phyPortNum, GT_U16);
    F_NUMBER(val, -1, phyPairNum, GT_U16);
    F_NUMBER(val, -1, enable, GT_8);
    F_NUMBER(val, -1, portMode, GT_U8);
}

void prv_c_to_lua_MV_HWS_IPC_LINK_BINDING_STRUCT(
    lua_State *L,
    MV_HWS_IPC_LINK_BINDING_STRUCT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, portGroup, GT_U16);
    FO_NUMBER(val, t, phyPortNum, GT_U16);
    FO_NUMBER(val, t, phyPairNum, GT_U16);
    FO_NUMBER(val, t, enable, GT_8);
    FO_NUMBER(val, t, portMode, GT_U8);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_IPC_LINK_BINDING_STRUCT);

add_mgm_enum(MV_HWS_PORT_MAN_TUNE_MODE);

add_mgm_enum(CPSS_PP_INTERFACE_CHANNEL_ENT);

void prv_lua_to_c_CPSS_PORT_SERDES_TX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_SERDES_TX_CONFIG_STC *val
)
{
    F_ENUM(val, -1, type, CPSS_PORT_SERDES_TYPE_ENT);
    F_UNION(val, -1, txTune, CPSS_PORT_SERDES_TX_CONFIG_UNT);
}

void prv_c_to_lua_CPSS_PORT_SERDES_TX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_SERDES_TX_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, type, CPSS_PORT_SERDES_TYPE_ENT);
    FO_UNION(val, t, txTune, CPSS_PORT_SERDES_TX_CONFIG_UNT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_SERDES_TX_CONFIG_STC);

void prv_lua_to_c_MV_HWS_SERDES_TX_OVERRIDE_CONFIG_DATA_UNT(
    lua_State *L,
    MV_HWS_SERDES_TX_OVERRIDE_CONFIG_DATA_UNT *val
)
{
    F_UNION_MEMBER_STRUCT(val, -1, txAvago, MV_HWS_AVAGO_TX_OVERRIDE_CONFIG_DATA);
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* cannot identify type classUNKNOWN
MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_TX_CONFIG_DATA txComphyC12GP41P2V; */
#endif
}

void prv_c_to_lua_MV_HWS_SERDES_TX_OVERRIDE_CONFIG_DATA_UNT(
    lua_State *L,
    MV_HWS_SERDES_TX_OVERRIDE_CONFIG_DATA_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, txAvago, MV_HWS_AVAGO_TX_OVERRIDE_CONFIG_DATA);
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* cannot identify type classUNKNOWN
MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_TX_CONFIG_DATA txComphyC12GP41P2V; */
#endif
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_SERDES_TX_OVERRIDE_CONFIG_DATA_UNT);
add_mgm_union(MV_HWS_SERDES_TX_OVERRIDE_CONFIG_DATA_UNT);

void prv_lua_to_c_CPSS_TRUNK_MEMBER_STC(
    lua_State *L,
    CPSS_TRUNK_MEMBER_STC *val
)
{
    F_NUMBER(val, -1, port, GT_PHYSICAL_PORT_NUM);
    F_NUMBER_N(val, -1, hwDevice, device, GT_HW_DEV_NUM);
}

void prv_c_to_lua_CPSS_TRUNK_MEMBER_STC(
    lua_State *L,
    CPSS_TRUNK_MEMBER_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, port, GT_PHYSICAL_PORT_NUM);
    FO_NUMBER_N(val, t, hwDevice, device, GT_HW_DEV_NUM);
    lua_settop(L, t);
}
add_mgm_type(CPSS_TRUNK_MEMBER_STC);

void prv_lua_to_c_MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS(
    lua_State *L,
    MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *val
)
{
    F_BOOL(val, -1, preemptionEnable);
    F_ENUM(val, -1, portSpeed, MV_HWS_PORT_SPEED_ENT);
}

void prv_c_to_lua_MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS(
    lua_State *L,
    MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, preemptionEnable);
    FO_ENUM(val, t, portSpeed, MV_HWS_PORT_SPEED_ENT);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS);

void prv_lua_to_c_CPSS_PORT_AP_SERDES_TX_OFFSETS_STC(
    lua_State *L,
    CPSS_PORT_AP_SERDES_TX_OFFSETS_STC *val
)
{
    F_NUMBER(val, -1, txAmpOffset, GT_8);
    F_NUMBER(val, -1, txEmph0Offset, GT_8);
    F_NUMBER(val, -1, txEmph1Offset, GT_8);
    F_BOOL(val, -1, reqInit);
    F_ENUM(val, -1, ifMode, CPSS_PORT_INTERFACE_MODE_ENT);
    F_ENUM(val, -1, speed, CPSS_PORT_SPEED_ENT);
}

void prv_c_to_lua_CPSS_PORT_AP_SERDES_TX_OFFSETS_STC(
    lua_State *L,
    CPSS_PORT_AP_SERDES_TX_OFFSETS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, txAmpOffset, GT_8);
    FO_NUMBER(val, t, txEmph0Offset, GT_8);
    FO_NUMBER(val, t, txEmph1Offset, GT_8);
    FO_BOOL(val, t, reqInit);
    FO_ENUM(val, t, ifMode, CPSS_PORT_INTERFACE_MODE_ENT);
    FO_ENUM(val, t, speed, CPSS_PORT_SPEED_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_AP_SERDES_TX_OFFSETS_STC);

void prv_lua_to_c_CPSS_PORT_COMPHY_SERDES_RX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_COMPHY_SERDES_RX_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, dcGain, GT_U32);
    F_NUMBER(val, -1, bandWidth, GT_U32);
    F_NUMBER(val, -1, dfe, GT_U32);
    F_NUMBER(val, -1, ffeR, GT_U32);
    F_NUMBER(val, -1, ffeC, GT_U32);
    F_NUMBER(val, -1, sampler, GT_U32);
    F_NUMBER(val, -1, sqlch, GT_U32);
    F_NUMBER(val, -1, align90, GT_U32);
    F_NUMBER(val, -1, ffeS, GT_U32);
    F_ARRAY_START(val, -1, dfeValsArray);
    {
        int idx;
        for (idx = 0; idx < 6; idx++) {
            F_ARRAY_NUMBER(val, dfeValsArray, idx, GT_32);
        }
    }
    F_ARRAY_END(val, -1, dfeValsArray);
}

void prv_c_to_lua_CPSS_PORT_COMPHY_SERDES_RX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_COMPHY_SERDES_RX_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, dcGain, GT_U32);
    FO_NUMBER(val, t, bandWidth, GT_U32);
    FO_NUMBER(val, t, dfe, GT_U32);
    FO_NUMBER(val, t, ffeR, GT_U32);
    FO_NUMBER(val, t, ffeC, GT_U32);
    FO_NUMBER(val, t, sampler, GT_U32);
    FO_NUMBER(val, t, sqlch, GT_U32);
    FO_NUMBER(val, t, align90, GT_U32);
    FO_NUMBER(val, t, ffeS, GT_U32);
    FO_ARRAY_START(val, t, dfeValsArray);
    {
        int idx;
        for (idx = 0; idx < 6; idx++) {
            FO_ARRAY_NUMBER(val, dfeValsArray, idx, GT_32);
        }
    }
    FO_ARRAY_END(val, t, dfeValsArray);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_COMPHY_SERDES_RX_CONFIG_STC);

add_mgm_enum(MV_HWS_PORT_PRESET_CMD);

add_mgm_enum(MV_HWS_PORT_FLOW_CONTROL_ENT);

add_mgm_enum(CPSS_DIAG_PG_PACKET_COUNT_MULTIPLIER_ENT);

add_mgm_enum(CPSS_EVENT_GPP_ID_ENT);

void prv_lua_to_c_CPSS_LED_GROUP_CONF_STC(
    lua_State *L,
    CPSS_LED_GROUP_CONF_STC *val
)
{
    F_NUMBER(val, -1, classA, GT_U32);
    F_NUMBER(val, -1, classB, GT_U32);
    F_NUMBER(val, -1, classC, GT_U32);
    F_NUMBER(val, -1, classD, GT_U32);
}

void prv_c_to_lua_CPSS_LED_GROUP_CONF_STC(
    lua_State *L,
    CPSS_LED_GROUP_CONF_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, classA, GT_U32);
    FO_NUMBER(val, t, classB, GT_U32);
    FO_NUMBER(val, t, classC, GT_U32);
    FO_NUMBER(val, t, classD, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_LED_GROUP_CONF_STC);

add_mgm_enum(CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_ENT);

void prv_lua_to_c_MV_HWS_HCD_INFO(
    lua_State *L,
    MV_HWS_HCD_INFO *val
)
{
    F_BOOL(val, -1, hcdFound);
    F_NUMBER(val, -1, hcdLinkStatus, GT_U32);
    F_NUMBER(val, -1, hcdResult, GT_U32);
    F_NUMBER(val, -1, hcdPcsLockStatus, GT_U32);
    F_BOOL(val, -1, hcdFecEn);
    F_BOOL(val, -1, hcdFcRxPauseEn);
    F_BOOL(val, -1, hcdFcTxPauseEn);
    F_NUMBER(val, -1, hcdFecType, GT_U32);
    F_NUMBER(val, -1, hcdMisc, GT_U32);
}

void prv_c_to_lua_MV_HWS_HCD_INFO(
    lua_State *L,
    MV_HWS_HCD_INFO *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, hcdFound);
    FO_NUMBER(val, t, hcdLinkStatus, GT_U32);
    FO_NUMBER(val, t, hcdResult, GT_U32);
    FO_NUMBER(val, t, hcdPcsLockStatus, GT_U32);
    FO_BOOL(val, t, hcdFecEn);
    FO_BOOL(val, t, hcdFcRxPauseEn);
    FO_BOOL(val, t, hcdFcTxPauseEn);
    FO_NUMBER(val, t, hcdFecType, GT_U32);
    FO_NUMBER(val, t, hcdMisc, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_HCD_INFO);

void prv_lua_to_c_CPSS_PM_MAC_PREEMPTION_PARAMS_STC(
    lua_State *L,
    CPSS_PM_MAC_PREEMPTION_PARAMS_STC *val
)
{
    F_ENUM(val, -1, type, CPSS_PM_MAC_PREEMPTION_TYPE_ENT);
    F_ENUM(val, -1, minFragSize, CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_ENT);
    F_ENUM(val, -1, preemptionMethod, CPSS_PM_MAC_PREEMPTION_METHOD_ENT);
}

void prv_c_to_lua_CPSS_PM_MAC_PREEMPTION_PARAMS_STC(
    lua_State *L,
    CPSS_PM_MAC_PREEMPTION_PARAMS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, type, CPSS_PM_MAC_PREEMPTION_TYPE_ENT);
    FO_ENUM(val, t, minFragSize, CPSS_PM_MAC_PREEMPTION_MIN_FRAG_SIZE_ENT);
    FO_ENUM(val, t, preemptionMethod, CPSS_PM_MAC_PREEMPTION_METHOD_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PM_MAC_PREEMPTION_PARAMS_STC);

add_mgm_enum(CPSS_LED_PULSE_STRETCH_ENT);

void prv_lua_to_c_CPSS_HW_DRIVER_SPLIT_ADDR_SPACE_RANGE_STC(
    lua_State *L,
    CPSS_HW_DRIVER_SPLIT_ADDR_SPACE_RANGE_STC *val
)
{
    F_NUMBER(val, -1, start, GT_UINTPTR);
    F_NUMBER(val, -1, size, GT_UINTPTR);
    F_NUMBER(val, -1, relativeOffset, GT_U32);
}

void prv_c_to_lua_CPSS_HW_DRIVER_SPLIT_ADDR_SPACE_RANGE_STC(
    lua_State *L,
    CPSS_HW_DRIVER_SPLIT_ADDR_SPACE_RANGE_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, start, GT_UINTPTR);
    FO_NUMBER(val, t, size, GT_UINTPTR);
    FO_NUMBER(val, t, relativeOffset, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_HW_DRIVER_SPLIT_ADDR_SPACE_RANGE_STC);

add_mgm_enum(CPSS_DRV_HW_TRACE_TYPE_ENT);

add_mgm_enum(CPSS_PP_FAMILY_BIT_ENT);

add_mgm_enum(CPSS_PORT_AP_FLOW_CONTROL_ENT);

add_mgm_enum(CPSS_PP_SUB_FAMILY_TYPE_ENT);

add_mgm_enum(CPSS_HW_DRIVER_TYPE_ENT);

void prv_lua_to_c_CPSS_PORT_MAC_CG_COUNTER_SET_STC(
    lua_State *L,
    CPSS_PORT_MAC_CG_COUNTER_SET_STC *val
)
{
    F_STRUCT(val, -1, ifOutOctets, GT_U64);
    F_STRUCT(val, -1, ifOutMulticastPkts, GT_U64);
    F_STRUCT(val, -1, ifOutErrors, GT_U64);
    F_STRUCT(val, -1, ifOutUcastPkts, GT_U64);
    F_STRUCT(val, -1, ifOutBroadcastPkts, GT_U64);
    F_STRUCT(val, -1, ifInOctets, GT_U64);
    F_STRUCT(val, -1, ifInErrors, GT_U64);
    F_STRUCT(val, -1, ifInBroadcastPkts, GT_U64);
    F_STRUCT(val, -1, ifInMulticastPkts, GT_U64);
    F_STRUCT(val, -1, ifInUcastPkts, GT_U64);
    F_STRUCT(val, -1, aFramesTransmittedOK, GT_U64);
    F_STRUCT(val, -1, aFramesReceivedOK, GT_U64);
    F_STRUCT(val, -1, aFrameCheckSequenceErrors, GT_U64);
    F_STRUCT(val, -1, aMACControlFramesTransmitted, GT_U64);
    F_STRUCT(val, -1, aMACControlFramesReceived, GT_U64);
    F_STRUCT(val, -1, aPAUSEMACCtrlFramesTransmitted, GT_U64);
    F_STRUCT(val, -1, aPAUSEMACCtrlFramesReceived, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_0, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_1, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_2, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_3, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_4, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_5, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_6, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesTransmitted_7, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_0, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_1, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_2, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_3, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_4, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_5, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_6, GT_U64);
    F_STRUCT(val, -1, aCBFCPAUSEFramesReceived_7, GT_U64);
    F_STRUCT(val, -1, etherStatsOctets, GT_U64);
    F_STRUCT(val, -1, etherStatsUndersizePkts, GT_U64);
    F_STRUCT(val, -1, etherStatsOversizePkts, GT_U64);
    F_STRUCT(val, -1, etherStatsJabbers, GT_U64);
    F_STRUCT(val, -1, etherStatsDropEvents, GT_U64);
    F_STRUCT(val, -1, etherStatsFragments, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts64Octets, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts65to127Octets, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts128to255Octets, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts256to511Octets, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts512to1023Octets, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts1024to1518Octets, GT_U64);
    F_STRUCT(val, -1, etherStatsPkts1519toMaxOctets, GT_U64);
    F_STRUCT(val, -1, aAlignmentErrors, GT_U64);
    F_STRUCT(val, -1, aFrameTooLongErrors, GT_U64);
    F_STRUCT(val, -1, aInRangeLengthErrors, GT_U64);
    F_STRUCT(val, -1, VLANTransmittedOK, GT_U64);
    F_STRUCT(val, -1, VLANRecievedOK, GT_U64);
}

void prv_c_to_lua_CPSS_PORT_MAC_CG_COUNTER_SET_STC(
    lua_State *L,
    CPSS_PORT_MAC_CG_COUNTER_SET_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, ifOutOctets, GT_U64);
    FO_STRUCT(val, t, ifOutMulticastPkts, GT_U64);
    FO_STRUCT(val, t, ifOutErrors, GT_U64);
    FO_STRUCT(val, t, ifOutUcastPkts, GT_U64);
    FO_STRUCT(val, t, ifOutBroadcastPkts, GT_U64);
    FO_STRUCT(val, t, ifInOctets, GT_U64);
    FO_STRUCT(val, t, ifInErrors, GT_U64);
    FO_STRUCT(val, t, ifInBroadcastPkts, GT_U64);
    FO_STRUCT(val, t, ifInMulticastPkts, GT_U64);
    FO_STRUCT(val, t, ifInUcastPkts, GT_U64);
    FO_STRUCT(val, t, aFramesTransmittedOK, GT_U64);
    FO_STRUCT(val, t, aFramesReceivedOK, GT_U64);
    FO_STRUCT(val, t, aFrameCheckSequenceErrors, GT_U64);
    FO_STRUCT(val, t, aMACControlFramesTransmitted, GT_U64);
    FO_STRUCT(val, t, aMACControlFramesReceived, GT_U64);
    FO_STRUCT(val, t, aPAUSEMACCtrlFramesTransmitted, GT_U64);
    FO_STRUCT(val, t, aPAUSEMACCtrlFramesReceived, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_0, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_1, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_2, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_3, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_4, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_5, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_6, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesTransmitted_7, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_0, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_1, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_2, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_3, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_4, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_5, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_6, GT_U64);
    FO_STRUCT(val, t, aCBFCPAUSEFramesReceived_7, GT_U64);
    FO_STRUCT(val, t, etherStatsOctets, GT_U64);
    FO_STRUCT(val, t, etherStatsUndersizePkts, GT_U64);
    FO_STRUCT(val, t, etherStatsOversizePkts, GT_U64);
    FO_STRUCT(val, t, etherStatsJabbers, GT_U64);
    FO_STRUCT(val, t, etherStatsDropEvents, GT_U64);
    FO_STRUCT(val, t, etherStatsFragments, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts64Octets, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts65to127Octets, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts128to255Octets, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts256to511Octets, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts512to1023Octets, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts1024to1518Octets, GT_U64);
    FO_STRUCT(val, t, etherStatsPkts1519toMaxOctets, GT_U64);
    FO_STRUCT(val, t, aAlignmentErrors, GT_U64);
    FO_STRUCT(val, t, aFrameTooLongErrors, GT_U64);
    FO_STRUCT(val, t, aInRangeLengthErrors, GT_U64);
    FO_STRUCT(val, t, VLANTransmittedOK, GT_U64);
    FO_STRUCT(val, t, VLANRecievedOK, GT_U64);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_MAC_CG_COUNTER_SET_STC);

add_mgm_enum(CPSS_FORMAT_CONVERT_FIELD_CHECK_ENT);

void prv_lua_to_c_MV_HWS_IPC_PORT_INFO_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_INFO_STRUCT *val
)
{
    F_NUMBER(val, -1, portGroup, GT_U16);
    F_NUMBER(val, -1, phyPortNum, GT_U16);
    F_NUMBER(val, -1, portMode, GT_U32);
}

void prv_c_to_lua_MV_HWS_IPC_PORT_INFO_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_INFO_STRUCT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, portGroup, GT_U16);
    FO_NUMBER(val, t, phyPortNum, GT_U16);
    FO_NUMBER(val, t, portMode, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_IPC_PORT_INFO_STRUCT);

add_mgm_enum(CPSS_RX_BUFF_ALLOC_METHOD_ENT);

void prv_lua_to_c_CPSS_PORT_COMPHY_C112G_TX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_COMPHY_C112G_TX_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, pre2, GT_32);
    F_NUMBER(val, -1, pre, GT_32);
    F_NUMBER(val, -1, main, GT_32);
    F_NUMBER(val, -1, post, GT_32);
}

void prv_c_to_lua_CPSS_PORT_COMPHY_C112G_TX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_COMPHY_C112G_TX_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, pre2, GT_32);
    FO_NUMBER(val, t, pre, GT_32);
    FO_NUMBER(val, t, main, GT_32);
    FO_NUMBER(val, t, post, GT_32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_COMPHY_C112G_TX_CONFIG_STC);

void prv_lua_to_c_MV_HWS_SERDES_TUNING_PARAMS(
    lua_State *L,
    MV_HWS_SERDES_TUNING_PARAMS *val
)
{
    F_ENUM(val, -1, presetCmdVal, MV_HWS_PORT_PRESET_CMD);
    F_BOOL(val, -1, eyeCheckEnVal);
    F_NUMBER(val, -1, gen1TrainAmp, GT_U32);
    F_NUMBER(val, -1, gen1TrainEmph0, GT_U32);
    F_NUMBER(val, -1, gen1TrainEmph1, GT_U32);
    F_BOOL(val, -1, gen1AmpAdj);
    F_NUMBER(val, -1, initialAmp, GT_U32);
    F_NUMBER(val, -1, initialEmph0, GT_U32);
    F_NUMBER(val, -1, initialEmph1, GT_U32);
    F_NUMBER(val, -1, presetAmp, GT_U32);
    F_NUMBER(val, -1, presetEmph0, GT_U32);
    F_NUMBER(val, -1, presetEmph1, GT_U32);
}

void prv_c_to_lua_MV_HWS_SERDES_TUNING_PARAMS(
    lua_State *L,
    MV_HWS_SERDES_TUNING_PARAMS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, presetCmdVal, MV_HWS_PORT_PRESET_CMD);
    FO_BOOL(val, t, eyeCheckEnVal);
    FO_NUMBER(val, t, gen1TrainAmp, GT_U32);
    FO_NUMBER(val, t, gen1TrainEmph0, GT_U32);
    FO_NUMBER(val, t, gen1TrainEmph1, GT_U32);
    FO_BOOL(val, t, gen1AmpAdj);
    FO_NUMBER(val, t, initialAmp, GT_U32);
    FO_NUMBER(val, t, initialEmph0, GT_U32);
    FO_NUMBER(val, t, initialEmph1, GT_U32);
    FO_NUMBER(val, t, presetAmp, GT_U32);
    FO_NUMBER(val, t, presetEmph0, GT_U32);
    FO_NUMBER(val, t, presetEmph1, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_SERDES_TUNING_PARAMS);

add_mgm_enum(MV_HWS_PORT_ACTION);

add_mgm_enum(CPSS_PORT_MAC_TYPE_ENT);

void prv_lua_to_c_MV_HWS_IPC_PORT_LOOPBACK_GET_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_LOOPBACK_GET_DATA_STRUCT *val
)
{
    F_NUMBER(val, -1, portGroup, GT_U16);
    F_NUMBER(val, -1, phyPortNum, GT_U16);
    F_NUMBER(val, -1, portMode, GT_U32);
    F_NUMBER(val, -1, lpPlace, GT_U32);
}

void prv_c_to_lua_MV_HWS_IPC_PORT_LOOPBACK_GET_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_LOOPBACK_GET_DATA_STRUCT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, portGroup, GT_U16);
    FO_NUMBER(val, t, phyPortNum, GT_U16);
    FO_NUMBER(val, t, portMode, GT_U32);
    FO_NUMBER(val, t, lpPlace, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_IPC_PORT_LOOPBACK_GET_DATA_STRUCT);

void prv_lua_to_c_MV_HWS_ETL_CONFIG_DATA(
    lua_State *L,
    MV_HWS_ETL_CONFIG_DATA *val
)
{
    F_NUMBER(val, -1, etlMinDelay, GT_U8);
    F_NUMBER(val, -1, etlMaxDelay, GT_U8);
    F_NUMBER(val, -1, etlEnableOverride, GT_U8);
}

void prv_c_to_lua_MV_HWS_ETL_CONFIG_DATA(
    lua_State *L,
    MV_HWS_ETL_CONFIG_DATA *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, etlMinDelay, GT_U8);
    FO_NUMBER(val, t, etlMaxDelay, GT_U8);
    FO_NUMBER(val, t, etlEnableOverride, GT_U8);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_ETL_CONFIG_DATA);

add_mgm_enum(CPSS_PORT_INTERFACE_MODE_ENT);

void prv_lua_to_c_MV_HWS_ANP_INFO_STC(
    lua_State *L,
    MV_HWS_ANP_INFO_STC *val
)
{
    F_STRUCT(val, -1, portConters, MV_HWS_ANP_PORT_COUNTERS);
    F_STRUCT(val, -1, portSm, MV_HWS_ANP_PORT_SM);
}

void prv_c_to_lua_MV_HWS_ANP_INFO_STC(
    lua_State *L,
    MV_HWS_ANP_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, portConters, MV_HWS_ANP_PORT_COUNTERS);
    FO_STRUCT(val, t, portSm, MV_HWS_ANP_PORT_SM);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_ANP_INFO_STC);

void prv_lua_to_c_MV_HWS_AP_PORT_INTROP(
    lua_State *L,
    MV_HWS_AP_PORT_INTROP *val
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
    F_NUMBER(val, -1, anPam4LinkMaxInterval, GT_U16);
}

void prv_c_to_lua_MV_HWS_AP_PORT_INTROP(
    lua_State *L,
    MV_HWS_AP_PORT_INTROP *val
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
    FO_NUMBER(val, t, anPam4LinkMaxInterval, GT_U16);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_AP_PORT_INTROP);

void prv_lua_to_c_CPSS_PORT_MAC_STATUS_STC(
    lua_State *L,
    CPSS_PORT_MAC_STATUS_STC *val
)
{
    F_BOOL(val, -1, isPortRxPause);
    F_BOOL(val, -1, isPortTxPause);
    F_BOOL(val, -1, isPortBackPres);
    F_BOOL(val, -1, isPortBufFull);
    F_BOOL(val, -1, isPortSyncFail);
    F_BOOL(val, -1, isPortHiErrorRate);
    F_BOOL(val, -1, isPortAnDone);
    F_BOOL(val, -1, isPortFatalError);
}

void prv_c_to_lua_CPSS_PORT_MAC_STATUS_STC(
    lua_State *L,
    CPSS_PORT_MAC_STATUS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, isPortRxPause);
    FO_BOOL(val, t, isPortTxPause);
    FO_BOOL(val, t, isPortBackPres);
    FO_BOOL(val, t, isPortBufFull);
    FO_BOOL(val, t, isPortSyncFail);
    FO_BOOL(val, t, isPortHiErrorRate);
    FO_BOOL(val, t, isPortAnDone);
    FO_BOOL(val, t, isPortFatalError);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_MAC_STATUS_STC);

void prv_lua_to_c_CPSS_PORT_MANAGER_STATISTICS_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_STATISTICS_STC *val
)
{
    F_NUMBER(val, -1, rxTrainingTime, GT_U32);
    F_NUMBER(val, -1, alignLockTime, GT_U32);
    F_NUMBER(val, -1, linkUpTime, GT_U32);
    F_NUMBER(val, -1, linkFailToSignalTime, GT_U32);
    F_NUMBER(val, -1, linkFailCnt, GT_U32);
    F_NUMBER(val, -1, linkOkCnt, GT_U32);
    F_NUMBER(val, -1, alignLockFailCnt, GT_U32);
    F_NUMBER(val, -1, trainingFailCnt, GT_U32);
}

void prv_c_to_lua_CPSS_PORT_MANAGER_STATISTICS_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_STATISTICS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, rxTrainingTime, GT_U32);
    FO_NUMBER(val, t, alignLockTime, GT_U32);
    FO_NUMBER(val, t, linkUpTime, GT_U32);
    FO_NUMBER(val, t, linkFailToSignalTime, GT_U32);
    FO_NUMBER(val, t, linkFailCnt, GT_U32);
    FO_NUMBER(val, t, linkOkCnt, GT_U32);
    FO_NUMBER(val, t, alignLockFailCnt, GT_U32);
    FO_NUMBER(val, t, trainingFailCnt, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_MANAGER_STATISTICS_STC);

void prv_lua_to_c_MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC(
    lua_State *L,
    MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC *val
)
{
    F_ARRAY_START(val, -1, phase_tvalBefore_tvSec);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            F_ARRAY_NUMBER(val, phase_tvalBefore_tvSec, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, phase_tvalBefore_tvSec);
    F_ARRAY_START(val, -1, phase_tvalBefore_tvUsec);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            F_ARRAY_NUMBER(val, phase_tvalBefore_tvUsec, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, phase_tvalBefore_tvUsec);
    F_NUMBER(val, -1, phase_contextSwitchedBack, GT_U8);
    F_NUMBER(val, -1, phase_edgeDetectin_Sec, GT_U32);
    F_NUMBER(val, -1, phase_edgeDetectin_Usec, GT_U32);
    F_NUMBER(val, -1, phase_edgeDetectin_Flag, GT_U8);
    F_NUMBER(val, -1, phase_tmpTimeout, GT_32);
    F_ARRAY_START(val, -1, phase_firstEO);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            F_ARRAY_NUMBER(val, phase_firstEO, idx, GT_32);
        }
    }
    F_ARRAY_END(val, -1, phase_firstEO);
    F_ARRAY_START(val, -1, phase_pCalTimeNano);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            F_ARRAY_NUMBER(val, phase_pCalTimeNano, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, phase_pCalTimeNano);
    F_ARRAY_START(val, -1, phase_pCalTimeSec);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            F_ARRAY_NUMBER(val, phase_pCalTimeSec, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, phase_pCalTimeSec);
    F_ARRAY_START(val, -1, phase_enable);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            F_ARRAY_BOOL(val, phase_enable, idx, GT_BOOL);
        }
    }
    F_ARRAY_END(val, -1, phase_enable);
    F_ARRAY_START(val, -1, phase_breakLoop);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            F_ARRAY_BOOL(val, phase_breakLoop, idx, GT_BOOL);
        }
    }
    F_ARRAY_END(val, -1, phase_breakLoop);
    F_BOOL(val, -1, phase_breakLoopTotal);
    F_BOOL(val, -1, phase_breakLoopInd);
    F_ARRAY_START(val, -1, phase_tuneRes);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            F_ARRAY_UNION(val, phase_tuneRes, idx, MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT);
        }
    }
    F_ARRAY_END(val, -1, phase_tuneRes);
    F_BOOL(val, -1, initFlag);
}

void prv_c_to_lua_MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC(
    lua_State *L,
    MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, phase_tvalBefore_tvSec);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            FO_ARRAY_NUMBER(val, phase_tvalBefore_tvSec, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, phase_tvalBefore_tvSec);
    FO_ARRAY_START(val, t, phase_tvalBefore_tvUsec);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            FO_ARRAY_NUMBER(val, phase_tvalBefore_tvUsec, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, phase_tvalBefore_tvUsec);
    FO_NUMBER(val, t, phase_contextSwitchedBack, GT_U8);
    FO_NUMBER(val, t, phase_edgeDetectin_Sec, GT_U32);
    FO_NUMBER(val, t, phase_edgeDetectin_Usec, GT_U32);
    FO_NUMBER(val, t, phase_edgeDetectin_Flag, GT_U8);
    FO_NUMBER(val, t, phase_tmpTimeout, GT_32);
    FO_ARRAY_START(val, t, phase_firstEO);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            FO_ARRAY_NUMBER(val, phase_firstEO, idx, GT_32);
        }
    }
    FO_ARRAY_END(val, t, phase_firstEO);
    FO_ARRAY_START(val, t, phase_pCalTimeNano);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            FO_ARRAY_NUMBER(val, phase_pCalTimeNano, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, phase_pCalTimeNano);
    FO_ARRAY_START(val, t, phase_pCalTimeSec);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            FO_ARRAY_NUMBER(val, phase_pCalTimeSec, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, phase_pCalTimeSec);
    FO_ARRAY_START(val, t, phase_enable);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            FO_ARRAY_BOOL(val, phase_enable, idx, GT_BOOL);
        }
    }
    FO_ARRAY_END(val, t, phase_enable);
    FO_ARRAY_START(val, t, phase_breakLoop);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            FO_ARRAY_BOOL(val, phase_breakLoop, idx, GT_BOOL);
        }
    }
    FO_ARRAY_END(val, t, phase_breakLoop);
    FO_BOOL(val, t, phase_breakLoopTotal);
    FO_BOOL(val, t, phase_breakLoopInd);
    FO_ARRAY_START(val, t, phase_tuneRes);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            FO_ARRAY_UNION(val, phase_tuneRes, idx, MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT);
        }
    }
    FO_ARRAY_END(val, t, phase_tuneRes);
    FO_BOOL(val, t, initFlag);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_AVAGO_PORT_MANAGER_EDGE_DETECT_STC);

add_mgm_enum(MV_HWS_PORT_MII_TYPE_E);

void prv_lua_to_c_MV_HWS_PORT_SERDES_TO_MAC_MUX(
    lua_State *L,
    MV_HWS_PORT_SERDES_TO_MAC_MUX *val
)
{
    F_BOOL(val, -1, enableSerdesMuxing);
    F_ARRAY_START(val, -1, serdesLanes);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            F_ARRAY_NUMBER(val, serdesLanes, idx, GT_U8);
        }
    }
    F_ARRAY_END(val, -1, serdesLanes);
}

void prv_c_to_lua_MV_HWS_PORT_SERDES_TO_MAC_MUX(
    lua_State *L,
    MV_HWS_PORT_SERDES_TO_MAC_MUX *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, enableSerdesMuxing);
    FO_ARRAY_START(val, t, serdesLanes);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            FO_ARRAY_NUMBER(val, serdesLanes, idx, GT_U8);
        }
    }
    FO_ARRAY_END(val, t, serdesLanes);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_PORT_SERDES_TO_MAC_MUX);

add_mgm_enum(CPSS_HW_INFO_BUS_TYPE_ENT);

void prv_lua_to_c_CPSS_PORT_MODE_SPEED_STC(
    lua_State *L,
    CPSS_PORT_MODE_SPEED_STC *val
)
{
    F_ENUM(val, -1, ifMode, CPSS_PORT_INTERFACE_MODE_ENT);
    F_ENUM(val, -1, speed, CPSS_PORT_SPEED_ENT);
}

void prv_c_to_lua_CPSS_PORT_MODE_SPEED_STC(
    lua_State *L,
    CPSS_PORT_MODE_SPEED_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, ifMode, CPSS_PORT_INTERFACE_MODE_ENT);
    FO_ENUM(val, t, speed, CPSS_PORT_SPEED_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_MODE_SPEED_STC);

add_mgm_enum(CPSS_PHY_SMI_INTERFACE_ENT);

add_mgm_enum(MV_HWS_SERDES_TYPE);

add_mgm_enum(MV_HWS_SERDES_TX_PATTERN);

void prv_lua_to_c_CPSS_PORT_MAC_COUNTER_SET_STC(
    lua_State *L,
    CPSS_PORT_MAC_COUNTER_SET_STC *val
)
{
    F_STRUCT(val, -1, goodOctetsRcv, GT_U64);
    F_STRUCT(val, -1, badOctetsRcv, GT_U64);
    F_STRUCT(val, -1, macTransmitErr, GT_U64);
    F_STRUCT(val, -1, goodPktsRcv, GT_U64);
    F_STRUCT(val, -1, badPktsRcv, GT_U64);
    F_STRUCT(val, -1, brdcPktsRcv, GT_U64);
    F_STRUCT(val, -1, mcPktsRcv, GT_U64);
    F_STRUCT(val, -1, pkts64Octets, GT_U64);
    F_STRUCT(val, -1, pkts65to127Octets, GT_U64);
    F_STRUCT(val, -1, pkts128to255Octets, GT_U64);
    F_STRUCT(val, -1, pkts256to511Octets, GT_U64);
    F_STRUCT(val, -1, pkts512to1023Octets, GT_U64);
    F_STRUCT(val, -1, pkts1024tomaxOoctets, GT_U64);
    F_STRUCT(val, -1, goodOctetsSent, GT_U64);
    F_STRUCT(val, -1, goodPktsSent, GT_U64);
    F_STRUCT(val, -1, excessiveCollisions, GT_U64);
    F_STRUCT(val, -1, mcPktsSent, GT_U64);
    F_STRUCT(val, -1, brdcPktsSent, GT_U64);
    F_STRUCT(val, -1, unrecogMacCntrRcv, GT_U64);
    F_STRUCT(val, -1, fcSent, GT_U64);
    F_STRUCT(val, -1, goodFcRcv, GT_U64);
    F_STRUCT(val, -1, dropEvents, GT_U64);
    F_STRUCT(val, -1, undersizePkts, GT_U64);
    F_STRUCT(val, -1, fragmentsPkts, GT_U64);
    F_STRUCT(val, -1, oversizePkts, GT_U64);
    F_STRUCT(val, -1, jabberPkts, GT_U64);
    F_STRUCT(val, -1, macRcvError, GT_U64);
    F_STRUCT(val, -1, badCrc, GT_U64);
    F_STRUCT(val, -1, collisions, GT_U64);
    F_STRUCT(val, -1, lateCollisions, GT_U64);
    F_STRUCT(val, -1, badFcRcv, GT_U64);
    F_STRUCT(val, -1, ucPktsRcv, GT_U64);
    F_STRUCT(val, -1, ucPktsSent, GT_U64);
    F_STRUCT(val, -1, multiplePktsSent, GT_U64);
    F_STRUCT(val, -1, deferredPktsSent, GT_U64);
    F_STRUCT(val, -1, pkts1024to1518Octets, GT_U64);
    F_STRUCT(val, -1, pkts1519toMaxOctets, GT_U64);
}

void prv_c_to_lua_CPSS_PORT_MAC_COUNTER_SET_STC(
    lua_State *L,
    CPSS_PORT_MAC_COUNTER_SET_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, goodOctetsRcv, GT_U64);
    FO_STRUCT(val, t, badOctetsRcv, GT_U64);
    FO_STRUCT(val, t, macTransmitErr, GT_U64);
    FO_STRUCT(val, t, goodPktsRcv, GT_U64);
    FO_STRUCT(val, t, badPktsRcv, GT_U64);
    FO_STRUCT(val, t, brdcPktsRcv, GT_U64);
    FO_STRUCT(val, t, mcPktsRcv, GT_U64);
    FO_STRUCT(val, t, pkts64Octets, GT_U64);
    FO_STRUCT(val, t, pkts65to127Octets, GT_U64);
    FO_STRUCT(val, t, pkts128to255Octets, GT_U64);
    FO_STRUCT(val, t, pkts256to511Octets, GT_U64);
    FO_STRUCT(val, t, pkts512to1023Octets, GT_U64);
    FO_STRUCT(val, t, pkts1024tomaxOoctets, GT_U64);
    FO_STRUCT(val, t, goodOctetsSent, GT_U64);
    FO_STRUCT(val, t, goodPktsSent, GT_U64);
    FO_STRUCT(val, t, excessiveCollisions, GT_U64);
    FO_STRUCT(val, t, mcPktsSent, GT_U64);
    FO_STRUCT(val, t, brdcPktsSent, GT_U64);
    FO_STRUCT(val, t, unrecogMacCntrRcv, GT_U64);
    FO_STRUCT(val, t, fcSent, GT_U64);
    FO_STRUCT(val, t, goodFcRcv, GT_U64);
    FO_STRUCT(val, t, dropEvents, GT_U64);
    FO_STRUCT(val, t, undersizePkts, GT_U64);
    FO_STRUCT(val, t, fragmentsPkts, GT_U64);
    FO_STRUCT(val, t, oversizePkts, GT_U64);
    FO_STRUCT(val, t, jabberPkts, GT_U64);
    FO_STRUCT(val, t, macRcvError, GT_U64);
    FO_STRUCT(val, t, badCrc, GT_U64);
    FO_STRUCT(val, t, collisions, GT_U64);
    FO_STRUCT(val, t, lateCollisions, GT_U64);
    FO_STRUCT(val, t, badFcRcv, GT_U64);
    FO_STRUCT(val, t, ucPktsRcv, GT_U64);
    FO_STRUCT(val, t, ucPktsSent, GT_U64);
    FO_STRUCT(val, t, multiplePktsSent, GT_U64);
    FO_STRUCT(val, t, deferredPktsSent, GT_U64);
    FO_STRUCT(val, t, pkts1024to1518Octets, GT_U64);
    FO_STRUCT(val, t, pkts1519toMaxOctets, GT_U64);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_MAC_COUNTER_SET_STC);

add_mgm_enum(MV_HWS_COM_PHY_H_SUB_SEQ);

void prv_lua_to_c_CPSS_PORT_COMPHY_C28G_TX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_COMPHY_C28G_TX_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, pre, GT_U32);
    F_NUMBER(val, -1, peak, GT_U32);
    F_NUMBER(val, -1, post, GT_U32);
}

void prv_c_to_lua_CPSS_PORT_COMPHY_C28G_TX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_COMPHY_C28G_TX_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, pre, GT_U32);
    FO_NUMBER(val, t, peak, GT_U32);
    FO_NUMBER(val, t, post, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_COMPHY_C28G_TX_CONFIG_STC);

void prv_lua_to_c_MV_HWS_MAN_TUNE_COMPHY_H_RX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_H_RX_CONFIG_DATA *val
)
{
    F_NUMBER(val, -1, sqlch, GT_U32);
    F_NUMBER(val, -1, ffeRes, GT_U32);
    F_NUMBER(val, -1, ffeCap, GT_U32);
    F_BOOL(val, -1, dfeEn);
    F_NUMBER(val, -1, alig, GT_U32);
    F_ENUM(val, -1, portTuningMode, MV_HWS_PORT_MAN_TUNE_MODE);
}

void prv_c_to_lua_MV_HWS_MAN_TUNE_COMPHY_H_RX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_H_RX_CONFIG_DATA *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, sqlch, GT_U32);
    FO_NUMBER(val, t, ffeRes, GT_U32);
    FO_NUMBER(val, t, ffeCap, GT_U32);
    FO_BOOL(val, t, dfeEn);
    FO_NUMBER(val, t, alig, GT_U32);
    FO_ENUM(val, t, portTuningMode, MV_HWS_PORT_MAN_TUNE_MODE);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_MAN_TUNE_COMPHY_H_RX_CONFIG_DATA);

void prv_lua_to_c_MV_HWS_AVAGO_AUTO_TUNE_RESULTS(
    lua_State *L,
    MV_HWS_AVAGO_AUTO_TUNE_RESULTS *val
)
{
    F_NUMBER(val, -1, sqleuch, GT_U32);
    F_NUMBER(val, -1, DC, GT_U32);
    F_NUMBER(val, -1, LF, GT_U32);
    F_NUMBER(val, -1, HF, GT_U32);
    F_NUMBER(val, -1, BW, GT_U32);
    F_NUMBER(val, -1, EO, GT_U32);
    F_ARRAY_START(val, -1, DFE);
    {
        int idx;
        for (idx = 0; idx < 13; idx++) {
            F_ARRAY_NUMBER(val, DFE, idx, GT_32);
        }
    }
    F_ARRAY_END(val, -1, DFE);
    F_NUMBER(val, -1, gainshape1, GT_U8);
    F_NUMBER(val, -1, gainshape2, GT_U8);
    F_BOOL(val, -1, shortChannelEn);
    F_NUMBER(val, -1, dfeGAIN, GT_U8);
    F_NUMBER(val, -1, dfeGAIN2, GT_U8);
    F_STRUCT(val, -1, avagoStc, MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA);
    F_NUMBER(val, -1, rxffe_pre2, GT_32);
    F_NUMBER(val, -1, rxffe_pre1, GT_32);
    F_NUMBER(val, -1, rxffe_post1, GT_32);
    F_NUMBER(val, -1, rxffe_bflf, GT_32);
    F_NUMBER(val, -1, rxffe_bfhf, GT_32);
    F_NUMBER(val, -1, rxffe_datarate, GT_32);
    F_NUMBER(val, -1, rxffe_minPre1, GT_32);
    F_NUMBER(val, -1, rxffe_maxPre1, GT_32);
    F_NUMBER(val, -1, rxffe_minPre2, GT_32);
    F_NUMBER(val, -1, rxffe_maxPre2, GT_32);
    F_NUMBER(val, -1, minLf, GT_U32);
    F_NUMBER(val, -1, maxLf, GT_U32);
    F_NUMBER(val, -1, minHf, GT_U32);
    F_NUMBER(val, -1, maxHf, GT_U32);
    F_NUMBER(val, -1, vernier_upper_odd_dly, GT_U32);
    F_NUMBER(val, -1, vernier_upper_even_dly, GT_U32);
    F_NUMBER(val, -1, vernier_middle_odd_dly, GT_U32);
    F_NUMBER(val, -1, vernier_middle_even_dly, GT_U32);
    F_NUMBER(val, -1, vernier_lower_odd_dly, GT_U32);
    F_NUMBER(val, -1, vernier_lower_even_dly, GT_U32);
    F_NUMBER(val, -1, vernier_test_odd_dly, GT_U32);
    F_NUMBER(val, -1, vernier_test_even_dly, GT_U32);
    F_NUMBER(val, -1, vernier_edge_odd_dly, GT_U32);
    F_NUMBER(val, -1, vernier_edge_even_dly, GT_U32);
    F_NUMBER(val, -1, vernier_tap_dly, GT_U32);
    F_ARRAY_START(val, -1, pam4EyesArr);
    {
        int idx;
        for (idx = 0; idx < 6; idx++) {
            F_ARRAY_NUMBER(val, pam4EyesArr, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, pam4EyesArr);
    F_NUMBER(val, -1, termination, GT_U8);
    F_NUMBER(val, -1, rxffe_minPost, GT_32);
    F_NUMBER(val, -1, rxffe_maxPost, GT_32);
    F_NUMBER(val, -1, coldEnvelope, GT_U32);
    F_NUMBER(val, -1, hotEnvelope, GT_U32);
}

void prv_c_to_lua_MV_HWS_AVAGO_AUTO_TUNE_RESULTS(
    lua_State *L,
    MV_HWS_AVAGO_AUTO_TUNE_RESULTS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, sqleuch, GT_U32);
    FO_NUMBER(val, t, DC, GT_U32);
    FO_NUMBER(val, t, LF, GT_U32);
    FO_NUMBER(val, t, HF, GT_U32);
    FO_NUMBER(val, t, BW, GT_U32);
    FO_NUMBER(val, t, EO, GT_U32);
    FO_ARRAY_START(val, t, DFE);
    {
        int idx;
        for (idx = 0; idx < 13; idx++) {
            FO_ARRAY_NUMBER(val, DFE, idx, GT_32);
        }
    }
    FO_ARRAY_END(val, t, DFE);
    FO_NUMBER(val, t, gainshape1, GT_U8);
    FO_NUMBER(val, t, gainshape2, GT_U8);
    FO_BOOL(val, t, shortChannelEn);
    FO_NUMBER(val, t, dfeGAIN, GT_U8);
    FO_NUMBER(val, t, dfeGAIN2, GT_U8);
    FO_STRUCT(val, t, avagoStc, MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA);
    FO_NUMBER(val, t, rxffe_pre2, GT_32);
    FO_NUMBER(val, t, rxffe_pre1, GT_32);
    FO_NUMBER(val, t, rxffe_post1, GT_32);
    FO_NUMBER(val, t, rxffe_bflf, GT_32);
    FO_NUMBER(val, t, rxffe_bfhf, GT_32);
    FO_NUMBER(val, t, rxffe_datarate, GT_32);
    FO_NUMBER(val, t, rxffe_minPre1, GT_32);
    FO_NUMBER(val, t, rxffe_maxPre1, GT_32);
    FO_NUMBER(val, t, rxffe_minPre2, GT_32);
    FO_NUMBER(val, t, rxffe_maxPre2, GT_32);
    FO_NUMBER(val, t, minLf, GT_U32);
    FO_NUMBER(val, t, maxLf, GT_U32);
    FO_NUMBER(val, t, minHf, GT_U32);
    FO_NUMBER(val, t, maxHf, GT_U32);
    FO_NUMBER(val, t, vernier_upper_odd_dly, GT_U32);
    FO_NUMBER(val, t, vernier_upper_even_dly, GT_U32);
    FO_NUMBER(val, t, vernier_middle_odd_dly, GT_U32);
    FO_NUMBER(val, t, vernier_middle_even_dly, GT_U32);
    FO_NUMBER(val, t, vernier_lower_odd_dly, GT_U32);
    FO_NUMBER(val, t, vernier_lower_even_dly, GT_U32);
    FO_NUMBER(val, t, vernier_test_odd_dly, GT_U32);
    FO_NUMBER(val, t, vernier_test_even_dly, GT_U32);
    FO_NUMBER(val, t, vernier_edge_odd_dly, GT_U32);
    FO_NUMBER(val, t, vernier_edge_even_dly, GT_U32);
    FO_NUMBER(val, t, vernier_tap_dly, GT_U32);
    FO_ARRAY_START(val, t, pam4EyesArr);
    {
        int idx;
        for (idx = 0; idx < 6; idx++) {
            FO_ARRAY_NUMBER(val, pam4EyesArr, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, pam4EyesArr);
    FO_NUMBER(val, t, termination, GT_U8);
    FO_NUMBER(val, t, rxffe_minPost, GT_32);
    FO_NUMBER(val, t, rxffe_maxPost, GT_32);
    FO_NUMBER(val, t, coldEnvelope, GT_U32);
    FO_NUMBER(val, t, hotEnvelope, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_AVAGO_AUTO_TUNE_RESULTS);

void prv_lua_to_c_MV_HWS_AVAGO_TXRX_OVERRIDE_CONFIG_DATA(
    lua_State *L,
    MV_HWS_AVAGO_TXRX_OVERRIDE_CONFIG_DATA *val
)
{
    F_NUMBER(val, -1, valid, GT_U8);
    F_NUMBER(val, -1, interconnectProfile, GT_U8);
    F_STRUCT(val, -1, txData, MV_HWS_AVAGO_TX_OVERRIDE_CONFIG_DATA);
    F_STRUCT(val, -1, rxData, MV_HWS_AVAGO_RX_OVERRIDE_CONFIG_DATA);
}

void prv_c_to_lua_MV_HWS_AVAGO_TXRX_OVERRIDE_CONFIG_DATA(
    lua_State *L,
    MV_HWS_AVAGO_TXRX_OVERRIDE_CONFIG_DATA *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, valid, GT_U8);
    FO_NUMBER(val, t, interconnectProfile, GT_U8);
    FO_STRUCT(val, t, txData, MV_HWS_AVAGO_TX_OVERRIDE_CONFIG_DATA);
    FO_STRUCT(val, t, rxData, MV_HWS_AVAGO_RX_OVERRIDE_CONFIG_DATA);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_AVAGO_TXRX_OVERRIDE_CONFIG_DATA);

add_mgm_enum(CPSS_PORT_SERDES_LOOPBACK_MODE_ENT);

void prv_lua_to_c_CPSS_PORT_MANAGER_LKB_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_LKB_CONFIG_STC *val
)
{
    F_BOOL(val, -1, enabled);
    F_NUMBER(val, -1, pairPortNum, GT_PHYSICAL_PORT_NUM);
}

void prv_c_to_lua_CPSS_PORT_MANAGER_LKB_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_LKB_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, enabled);
    FO_NUMBER(val, t, pairPortNum, GT_PHYSICAL_PORT_NUM);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_MANAGER_LKB_CONFIG_STC);

add_mgm_enum(CPSS_NET_CPU_PORT_MODE_ENT);

add_mgm_enum(CPSS_FORMAT_CONVERT_FIELD_TYPE_ENT);

void prv_lua_to_c_MV_HWS_IPC_PORT_AP_INTROP_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_AP_INTROP_STRUCT *val
)
{
    F_NUMBER(val, -1, portGroup, GT_U16);
    F_NUMBER(val, -1, phyPortNum, GT_U16);
    F_NUMBER(val, -1, portMode, GT_U32);
    F_NUMBER(val, -1, attrBitMask, GT_U16);
    F_NUMBER(val, -1, txDisDuration, GT_U16);
    F_NUMBER(val, -1, abilityDuration, GT_U16);
    F_NUMBER(val, -1, abilityMaxInterval, GT_U16);
    F_NUMBER(val, -1, abilityFailMaxInterval, GT_U16);
    F_NUMBER(val, -1, apLinkDuration, GT_U16);
    F_NUMBER(val, -1, apLinkMaxInterval, GT_U16);
    F_NUMBER(val, -1, pdLinkDuration, GT_U16);
    F_NUMBER(val, -1, pdLinkMaxInterval, GT_U16);
    F_NUMBER(val, -1, anPam4LinkMaxInterval, GT_U16);
}

void prv_c_to_lua_MV_HWS_IPC_PORT_AP_INTROP_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_AP_INTROP_STRUCT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, portGroup, GT_U16);
    FO_NUMBER(val, t, phyPortNum, GT_U16);
    FO_NUMBER(val, t, portMode, GT_U32);
    FO_NUMBER(val, t, attrBitMask, GT_U16);
    FO_NUMBER(val, t, txDisDuration, GT_U16);
    FO_NUMBER(val, t, abilityDuration, GT_U16);
    FO_NUMBER(val, t, abilityMaxInterval, GT_U16);
    FO_NUMBER(val, t, abilityFailMaxInterval, GT_U16);
    FO_NUMBER(val, t, apLinkDuration, GT_U16);
    FO_NUMBER(val, t, apLinkMaxInterval, GT_U16);
    FO_NUMBER(val, t, pdLinkDuration, GT_U16);
    FO_NUMBER(val, t, pdLinkMaxInterval, GT_U16);
    FO_NUMBER(val, t, anPam4LinkMaxInterval, GT_U16);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_IPC_PORT_AP_INTROP_STRUCT);

add_mgm_enum(CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_ENT);

void prv_lua_to_c_MV_HWS_PORT_INIT_INPUT_PARAMS(
    lua_State *L,
    MV_HWS_PORT_INIT_INPUT_PARAMS *val
)
{
    F_BOOL(val, -1, lbPort);
    F_ENUM(val, -1, refClock, MV_HWS_REF_CLOCK_SUP_VAL);
    F_ENUM(val, -1, refClockSource, MV_HWS_REF_CLOCK_SOURCE);
    F_ENUM(val, -1, portFecMode, MV_HWS_PORT_FEC_MODE);
    F_ENUM(val, -1, portSpeed, MV_HWS_PORT_SPEED_ENT);
    F_BOOL(val, -1, isPreemptionEnabled);
    F_BOOL(val, -1, apEnabled);
    F_BOOL(val, -1, autoNegEnabled);
}

void prv_c_to_lua_MV_HWS_PORT_INIT_INPUT_PARAMS(
    lua_State *L,
    MV_HWS_PORT_INIT_INPUT_PARAMS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, lbPort);
    FO_ENUM(val, t, refClock, MV_HWS_REF_CLOCK_SUP_VAL);
    FO_ENUM(val, t, refClockSource, MV_HWS_REF_CLOCK_SOURCE);
    FO_ENUM(val, t, portFecMode, MV_HWS_PORT_FEC_MODE);
    FO_ENUM(val, t, portSpeed, MV_HWS_PORT_SPEED_ENT);
    FO_BOOL(val, t, isPreemptionEnabled);
    FO_BOOL(val, t, apEnabled);
    FO_BOOL(val, t, autoNegEnabled);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_PORT_INIT_INPUT_PARAMS);

add_mgm_enum(CPSS_SYSTEM_RECOVERY_HA_STATE_ENT);

void prv_lua_to_c_MV_HWS_PER_SERDES_INFO_STC(
    lua_State *L,
    MV_HWS_PER_SERDES_INFO_STC *val
)
{
}

void prv_c_to_lua_MV_HWS_PER_SERDES_INFO_STC(
    lua_State *L,
    MV_HWS_PER_SERDES_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_PER_SERDES_INFO_STC);

void prv_lua_to_c_CPSS_SYSTEM_RECOVERY_INFO_STC(
    lua_State *L,
    CPSS_SYSTEM_RECOVERY_INFO_STC *val
)
{
    F_ENUM(val, -1, systemRecoveryState, CPSS_SYSTEM_RECOVERY_STATE_ENT);
    F_STRUCT(val, -1, systemRecoveryMode, CPSS_SYSTEM_RECOVERY_MODE_STC);
    F_ENUM(val, -1, systemRecoveryProcess, CPSS_SYSTEM_RECOVERY_PROCESS_ENT);
}

void prv_c_to_lua_CPSS_SYSTEM_RECOVERY_INFO_STC(
    lua_State *L,
    CPSS_SYSTEM_RECOVERY_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, systemRecoveryState, CPSS_SYSTEM_RECOVERY_STATE_ENT);
    FO_STRUCT(val, t, systemRecoveryMode, CPSS_SYSTEM_RECOVERY_MODE_STC);
    FO_ENUM(val, t, systemRecoveryProcess, CPSS_SYSTEM_RECOVERY_PROCESS_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_SYSTEM_RECOVERY_INFO_STC);

void prv_lua_to_c_MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT(
    lua_State *L,
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT *val
)
{
    F_UNION_MEMBER_STRUCT(val, -1, comphyHResults, MV_HWS_COMPHY_H_AUTO_TUNE_RESULTS);
    F_UNION_MEMBER_STRUCT(val, -1, avagoResults, MV_HWS_AVAGO_AUTO_TUNE_RESULTS);
    F_UNION_MEMBER_STRUCT(val, -1, comphyC12GP41P2VResults, MV_HWS_COMPHY_C12GP41P2V_AUTO_TUNE_RESULTS);
    F_UNION_MEMBER_STRUCT(val, -1, comphyC28GP4Results, MV_HWS_COMPHY_C28GP4_AUTO_TUNE_RESULTS);
    F_UNION_MEMBER_STRUCT(val, -1, comphyC112GResults, MV_HWS_COMPHY_C112G_AUTO_TUNE_RESULTS);
    F_UNION_MEMBER_STRUCT(val, -1, comphyC56GResults, MV_HWS_COMPHY_C56G_AUTO_TUNE_RESULTS);
}

void prv_c_to_lua_MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT(
    lua_State *L,
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, comphyHResults, MV_HWS_COMPHY_H_AUTO_TUNE_RESULTS);
    FO_STRUCT(val, t, avagoResults, MV_HWS_AVAGO_AUTO_TUNE_RESULTS);
    FO_STRUCT(val, t, comphyC12GP41P2VResults, MV_HWS_COMPHY_C12GP41P2V_AUTO_TUNE_RESULTS);
    FO_STRUCT(val, t, comphyC28GP4Results, MV_HWS_COMPHY_C28GP4_AUTO_TUNE_RESULTS);
    FO_STRUCT(val, t, comphyC112GResults, MV_HWS_COMPHY_C112G_AUTO_TUNE_RESULTS);
    FO_STRUCT(val, t, comphyC56GResults, MV_HWS_COMPHY_C56G_AUTO_TUNE_RESULTS);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT);
add_mgm_union(MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT);

add_mgm_enum(CPSS_PORT_MANAGER_EVENT_ENT);

add_mgm_enum(MV_HWS_UNIT_NUM);

add_mgm_enum(CPSS_PM_MAC_PREEMPTION_METHOD_ENT);

add_mgm_enum(CPSS_PORT_MANAGER_PORT_TYPE_ENT);

add_mgm_enum(MV_HWS_TX_TRAINING_MODES);

add_mgm_enum(CPSS_LED_ORDER_MODE_ENT);

add_mgm_enum(CPSS_LED_BLINK_DURATION_ENT);

add_mgm_enum(CPSS_PORT_XG_FIXED_IPG_ENT);

add_mgm_enum(CPSS_SYS_HA_MODE_ENT);

void prv_lua_to_c_MV_HWS_IPC_PORT_AP_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_AP_DATA_STRUCT *val
)
{
    F_NUMBER(val, -1, portGroup, GT_U16);
    F_NUMBER(val, -1, phyPortNum, GT_U16);
    F_NUMBER(val, -1, advMode, GT_U32);
    F_NUMBER(val, -1, options, GT_U32);
    F_NUMBER(val, -1, polarityVector, GT_U16);
    F_NUMBER(val, -1, laneNum, GT_U8);
    F_NUMBER(val, -1, pcsNum, GT_U8);
    F_NUMBER(val, -1, macNum, GT_U8);
    F_NUMBER(val, -1, refClockSrcParams, GT_U8);
}

void prv_c_to_lua_MV_HWS_IPC_PORT_AP_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_AP_DATA_STRUCT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, portGroup, GT_U16);
    FO_NUMBER(val, t, phyPortNum, GT_U16);
    FO_NUMBER(val, t, advMode, GT_U32);
    FO_NUMBER(val, t, options, GT_U32);
    FO_NUMBER(val, t, polarityVector, GT_U16);
    FO_NUMBER(val, t, laneNum, GT_U8);
    FO_NUMBER(val, t, pcsNum, GT_U8);
    FO_NUMBER(val, t, macNum, GT_U8);
    FO_NUMBER(val, t, refClockSrcParams, GT_U8);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_IPC_PORT_AP_DATA_STRUCT);

void prv_lua_to_c_CPSS_PORT_COMPHY_C12G_TX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_COMPHY_C12G_TX_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, pre, GT_U32);
    F_NUMBER(val, -1, peak, GT_U32);
    F_NUMBER(val, -1, post, GT_U32);
}

void prv_c_to_lua_CPSS_PORT_COMPHY_C12G_TX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_COMPHY_C12G_TX_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, pre, GT_U32);
    FO_NUMBER(val, t, peak, GT_U32);
    FO_NUMBER(val, t, post, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_COMPHY_C12G_TX_CONFIG_STC);

void prv_lua_to_c_CPSS_SYSTEM_RECOVERY_MODE_STC(
    lua_State *L,
    CPSS_SYSTEM_RECOVERY_MODE_STC *val
)
{
    F_BOOL(val, -1, continuousRx);
    F_BOOL(val, -1, continuousTx);
    F_BOOL(val, -1, continuousAuMessages);
    F_BOOL(val, -1, continuousFuMessages);
    F_BOOL(val, -1, haCpuMemoryAccessBlocked);
    F_ENUM(val, -1, ha2phasesInitPhase, CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_ENT);
    F_ENUM(val, -1, haReadWriteState, CPSS_SYSTEM_RECOVERY_HA_STATE_ENT);
}

void prv_c_to_lua_CPSS_SYSTEM_RECOVERY_MODE_STC(
    lua_State *L,
    CPSS_SYSTEM_RECOVERY_MODE_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, continuousRx);
    FO_BOOL(val, t, continuousTx);
    FO_BOOL(val, t, continuousAuMessages);
    FO_BOOL(val, t, continuousFuMessages);
    FO_BOOL(val, t, haCpuMemoryAccessBlocked);
    FO_ENUM(val, t, ha2phasesInitPhase, CPSS_SYSTEM_RECOVERY_HA_2_PHASES_INIT_ENT);
    FO_ENUM(val, t, haReadWriteState, CPSS_SYSTEM_RECOVERY_HA_STATE_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_SYSTEM_RECOVERY_MODE_STC);

void prv_lua_to_c_CPSS_PORT_MANAGER_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_STC *val
)
{
    F_ENUM(val, -1, portEvent, CPSS_PORT_MANAGER_EVENT_ENT);
}

void prv_c_to_lua_CPSS_PORT_MANAGER_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, portEvent, CPSS_PORT_MANAGER_EVENT_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_MANAGER_STC);

add_mgm_enum(CPSS_FORMAT_CONVERT_FIELD_PTR_CMD_ENT);

void prv_lua_to_c_MV_HWS_SERDES_TX_CONFIG_DATA_UNT(
    lua_State *L,
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT *val
)
{
    F_UNION_MEMBER_STRUCT(val, -1, txComphyH, MV_HWS_MAN_TUNE_COMPHY_H_TX_CONFIG_DATA);
    F_UNION_MEMBER_STRUCT(val, -1, txAvago, MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA);
    F_UNION_MEMBER_STRUCT(val, -1, txComphyC12GP41P2V, MV_HWS_MAN_TUNE_COMPHY_C12G_TX_CONFIG_DATA);
    F_UNION_MEMBER_STRUCT(val, -1, txComphyC28GP4, MV_HWS_MAN_TUNE_COMPHY_C28G_TX_CONFIG_DATA);
    F_UNION_MEMBER_STRUCT(val, -1, txComphyC112G, MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA);
    F_UNION_MEMBER_STRUCT(val, -1, txComphyC56G, MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA);
}

void prv_c_to_lua_MV_HWS_SERDES_TX_CONFIG_DATA_UNT(
    lua_State *L,
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, txComphyH, MV_HWS_MAN_TUNE_COMPHY_H_TX_CONFIG_DATA);
    FO_STRUCT(val, t, txAvago, MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA);
    FO_STRUCT(val, t, txComphyC12GP41P2V, MV_HWS_MAN_TUNE_COMPHY_C12G_TX_CONFIG_DATA);
    FO_STRUCT(val, t, txComphyC28GP4, MV_HWS_MAN_TUNE_COMPHY_C28G_TX_CONFIG_DATA);
    FO_STRUCT(val, t, txComphyC112G, MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA);
    FO_STRUCT(val, t, txComphyC56G, MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_SERDES_TX_CONFIG_DATA_UNT);
add_mgm_union(MV_HWS_SERDES_TX_CONFIG_DATA_UNT);

void prv_lua_to_c_CPSS_FORMAT_CONVERT_FIELD_CONDITION_STC(
    lua_State *L,
    CPSS_FORMAT_CONVERT_FIELD_CONDITION_STC *val
)
{
    F_ENUM(val, -1, fieldTypeOrSizeOf, CPSS_FORMAT_CONVERT_FIELD_TYPE_ENT);
    F_NUMBER(val, -1, stcOrExtMemByteOffset, GT_U16);
    F_NUMBER(val, -1, mask, GT_U32);
    F_NUMBER(val, -1, pattern, GT_U32);
}

void prv_c_to_lua_CPSS_FORMAT_CONVERT_FIELD_CONDITION_STC(
    lua_State *L,
    CPSS_FORMAT_CONVERT_FIELD_CONDITION_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, fieldTypeOrSizeOf, CPSS_FORMAT_CONVERT_FIELD_TYPE_ENT);
    FO_NUMBER(val, t, stcOrExtMemByteOffset, GT_U16);
    FO_NUMBER(val, t, mask, GT_U32);
    FO_NUMBER(val, t, pattern, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_FORMAT_CONVERT_FIELD_CONDITION_STC);

add_mgm_enum(CPSS_DIAG_PG_PACKET_LENGTH_TYPE_ENT);

add_mgm_enum(CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_ENT);

void prv_lua_to_c_MV_HWS_ANP_PORT_COUNTERS(
    lua_State *L,
    MV_HWS_ANP_PORT_COUNTERS *val
)
{
    F_NUMBER(val, -1, anRestartCounter, GT_U32);
    F_NUMBER(val, -1, dspLockFailCounter, GT_U32);
    F_NUMBER(val, -1, linkFailCounter, GT_U32);
    F_NUMBER(val, -1, txTrainDuration, GT_U32);
    F_NUMBER(val, -1, txTrainFailCounter, GT_U32);
    F_NUMBER(val, -1, txTrainTimeoutCounter, GT_U32);
    F_NUMBER(val, -1, txTrainOkCounter, GT_U32);
    F_NUMBER(val, -1, rxInitOk, GT_U32);
    F_NUMBER(val, -1, rxInitTimeOut, GT_U32);
    F_NUMBER(val, -1, rxTrainFailed, GT_U32);
    F_NUMBER(val, -1, rxTrainOk, GT_U32);
    F_NUMBER(val, -1, rxTrainTimeOut, GT_U32);
    F_NUMBER(val, -1, rxTrainDuration, GT_U32);
}

void prv_c_to_lua_MV_HWS_ANP_PORT_COUNTERS(
    lua_State *L,
    MV_HWS_ANP_PORT_COUNTERS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, anRestartCounter, GT_U32);
    FO_NUMBER(val, t, dspLockFailCounter, GT_U32);
    FO_NUMBER(val, t, linkFailCounter, GT_U32);
    FO_NUMBER(val, t, txTrainDuration, GT_U32);
    FO_NUMBER(val, t, txTrainFailCounter, GT_U32);
    FO_NUMBER(val, t, txTrainTimeoutCounter, GT_U32);
    FO_NUMBER(val, t, txTrainOkCounter, GT_U32);
    FO_NUMBER(val, t, rxInitOk, GT_U32);
    FO_NUMBER(val, t, rxInitTimeOut, GT_U32);
    FO_NUMBER(val, t, rxTrainFailed, GT_U32);
    FO_NUMBER(val, t, rxTrainOk, GT_U32);
    FO_NUMBER(val, t, rxTrainTimeOut, GT_U32);
    FO_NUMBER(val, t, rxTrainDuration, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_ANP_PORT_COUNTERS);

void prv_lua_to_c_MV_HWS_IPC_PORT_AUTO_TUNE_STOP_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_AUTO_TUNE_STOP_DATA_STRUCT *val
)
{
    F_NUMBER(val, -1, portGroup, GT_U16);
    F_NUMBER(val, -1, phyPortNum, GT_U16);
    F_NUMBER(val, -1, portMode, GT_U32);
    F_NUMBER(val, -1, stopRx, GT_U32);
    F_NUMBER(val, -1, stopTx, GT_U32);
}

void prv_c_to_lua_MV_HWS_IPC_PORT_AUTO_TUNE_STOP_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_AUTO_TUNE_STOP_DATA_STRUCT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, portGroup, GT_U16);
    FO_NUMBER(val, t, phyPortNum, GT_U16);
    FO_NUMBER(val, t, portMode, GT_U32);
    FO_NUMBER(val, t, stopRx, GT_U32);
    FO_NUMBER(val, t, stopTx, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_IPC_PORT_AUTO_TUNE_STOP_DATA_STRUCT);

add_mgm_enum(CPSS_EVENT_MASK_SET_ENT);

void prv_lua_to_c_MV_HWS_COMPHY_H_AUTO_TUNE_RESULTS(
    lua_State *L,
    MV_HWS_COMPHY_H_AUTO_TUNE_RESULTS *val
)
{
    F_NUMBER(val, -1, ffeR, GT_U32);
    F_NUMBER(val, -1, ffeC, GT_U32);
    F_NUMBER(val, -1, sampler, GT_U32);
    F_NUMBER(val, -1, sqleuch, GT_U32);
    F_NUMBER(val, -1, txAmp, GT_U32);
    F_NUMBER(val, -1, txEmph0, GT_32);
    F_NUMBER(val, -1, txEmph1, GT_32);
    F_NUMBER(val, -1, align90, GT_U32);
    F_ARRAY_START(val, -1, dfeVals);
    {
        int idx;
        for (idx = 0; idx < 6; idx++) {
            F_ARRAY_NUMBER(val, dfeVals, idx, GT_32);
        }
    }
    F_ARRAY_END(val, -1, dfeVals);
    F_NUMBER(val, -1, txAmpAdj, GT_U32);
    F_NUMBER(val, -1, txAmpShft, GT_U32);
    F_NUMBER(val, -1, txEmph0En, GT_U32);
    F_NUMBER(val, -1, txEmph1En, GT_U32);
    F_NUMBER(val, -1, slewRate, GT_U32);
    F_BOOL(val, -1, slewCtrlEn);
}

void prv_c_to_lua_MV_HWS_COMPHY_H_AUTO_TUNE_RESULTS(
    lua_State *L,
    MV_HWS_COMPHY_H_AUTO_TUNE_RESULTS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, ffeR, GT_U32);
    FO_NUMBER(val, t, ffeC, GT_U32);
    FO_NUMBER(val, t, sampler, GT_U32);
    FO_NUMBER(val, t, sqleuch, GT_U32);
    FO_NUMBER(val, t, txAmp, GT_U32);
    FO_NUMBER(val, t, txEmph0, GT_32);
    FO_NUMBER(val, t, txEmph1, GT_32);
    FO_NUMBER(val, t, align90, GT_U32);
    FO_ARRAY_START(val, t, dfeVals);
    {
        int idx;
        for (idx = 0; idx < 6; idx++) {
            FO_ARRAY_NUMBER(val, dfeVals, idx, GT_32);
        }
    }
    FO_ARRAY_END(val, t, dfeVals);
    FO_NUMBER(val, t, txAmpAdj, GT_U32);
    FO_NUMBER(val, t, txAmpShft, GT_U32);
    FO_NUMBER(val, t, txEmph0En, GT_U32);
    FO_NUMBER(val, t, txEmph1En, GT_U32);
    FO_NUMBER(val, t, slewRate, GT_U32);
    FO_BOOL(val, t, slewCtrlEn);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_COMPHY_H_AUTO_TUNE_RESULTS);

add_mgm_enum(CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_ENT);

add_mgm_enum(CPSS_DRV_HW_RESOURCE_TYPE_ENT);

void prv_lua_to_c_CPSS_PORT_COMPHY_C56G_RX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_COMPHY_C56G_RX_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, cur1Sel, GT_U32);
    F_NUMBER(val, -1, rl1Sel, GT_U32);
    F_NUMBER(val, -1, rl1Extra, GT_U32);
    F_NUMBER(val, -1, res1Sel, GT_U32);
    F_NUMBER(val, -1, cap1Sel, GT_U32);
    F_NUMBER(val, -1, enMidfreq, GT_U32);
    F_NUMBER(val, -1, cs1Mid, GT_U32);
    F_NUMBER(val, -1, rs1Mid, GT_U32);
    F_NUMBER(val, -1, cur2Sel, GT_U32);
    F_NUMBER(val, -1, rl2Sel, GT_U32);
    F_NUMBER(val, -1, rl2TuneG, GT_U32);
    F_NUMBER(val, -1, res2Sel, GT_U32);
    F_NUMBER(val, -1, cap2Sel, GT_U32);
    F_NUMBER(val, -1, selmufi, GT_U32);
    F_NUMBER(val, -1, selmuff, GT_U32);
    F_NUMBER(val, -1, selmupi, GT_U32);
    F_NUMBER(val, -1, selmupf, GT_U32);
    F_NUMBER(val, -1, squelch, GT_32);
    F_ARRAY_START(val, -1, dfe);
    {
        int idx;
        for (idx = 0; idx < 26; idx++) {
            F_ARRAY_NUMBER(val, dfe, idx, GT_32);
        }
    }
    F_ARRAY_END(val, -1, dfe);
}

void prv_c_to_lua_CPSS_PORT_COMPHY_C56G_RX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_COMPHY_C56G_RX_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, cur1Sel, GT_U32);
    FO_NUMBER(val, t, rl1Sel, GT_U32);
    FO_NUMBER(val, t, rl1Extra, GT_U32);
    FO_NUMBER(val, t, res1Sel, GT_U32);
    FO_NUMBER(val, t, cap1Sel, GT_U32);
    FO_NUMBER(val, t, enMidfreq, GT_U32);
    FO_NUMBER(val, t, cs1Mid, GT_U32);
    FO_NUMBER(val, t, rs1Mid, GT_U32);
    FO_NUMBER(val, t, cur2Sel, GT_U32);
    FO_NUMBER(val, t, rl2Sel, GT_U32);
    FO_NUMBER(val, t, rl2TuneG, GT_U32);
    FO_NUMBER(val, t, res2Sel, GT_U32);
    FO_NUMBER(val, t, cap2Sel, GT_U32);
    FO_NUMBER(val, t, selmufi, GT_U32);
    FO_NUMBER(val, t, selmuff, GT_U32);
    FO_NUMBER(val, t, selmupi, GT_U32);
    FO_NUMBER(val, t, selmupf, GT_U32);
    FO_NUMBER(val, t, squelch, GT_32);
    FO_ARRAY_START(val, t, dfe);
    {
        int idx;
        for (idx = 0; idx < 26; idx++) {
            FO_ARRAY_NUMBER(val, dfe, idx, GT_32);
        }
    }
    FO_ARRAY_END(val, t, dfe);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_COMPHY_C56G_RX_CONFIG_STC);

void prv_lua_to_c_MV_HWS_IPC_PORT_PPM_SET_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_PPM_SET_DATA_STRUCT *val
)
{
    F_NUMBER(val, -1, portGroup, GT_U16);
    F_NUMBER(val, -1, phyPortNum, GT_U16);
    F_NUMBER(val, -1, portMode, GT_U32);
    F_NUMBER(val, -1, portPPM, GT_U32);
}

void prv_c_to_lua_MV_HWS_IPC_PORT_PPM_SET_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_PPM_SET_DATA_STRUCT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, portGroup, GT_U16);
    FO_NUMBER(val, t, phyPortNum, GT_U16);
    FO_NUMBER(val, t, portMode, GT_U32);
    FO_NUMBER(val, t, portPPM, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_IPC_PORT_PPM_SET_DATA_STRUCT);

add_mgm_enum(CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT);

add_mgm_enum(CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_ENT);

void prv_lua_to_c_CPSS_PM_MODE_LANE_PARAM_STC(
    lua_State *L,
    CPSS_PM_MODE_LANE_PARAM_STC *val
)
{
    F_NUMBER(val, -1, validLaneParamsBitMask, GT_U32);
    F_NUMBER(val, -1, globalLaneNum, GT_U16);
    F_STRUCT(val, -1, txParams, CPSS_PORT_SERDES_TX_CONFIG_STC);
    F_STRUCT(val, -1, rxParams, CPSS_PORT_SERDES_RX_CONFIG_STC);
}

void prv_c_to_lua_CPSS_PM_MODE_LANE_PARAM_STC(
    lua_State *L,
    CPSS_PM_MODE_LANE_PARAM_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, validLaneParamsBitMask, GT_U32);
    FO_NUMBER(val, t, globalLaneNum, GT_U16);
    FO_STRUCT(val, t, txParams, CPSS_PORT_SERDES_TX_CONFIG_STC);
    FO_STRUCT(val, t, rxParams, CPSS_PORT_SERDES_RX_CONFIG_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PM_MODE_LANE_PARAM_STC);

add_mgm_enum(CPSS_PORT_PCS_RESET_MODE_ENT);

add_mgm_enum(CPSS_INTERFACE_TYPE_ENT);

add_mgm_enum(CPSS_UNICAST_MULTICAST_ENT);

add_mgm_enum(CPSS_PORT_SERDES_AUTO_TUNE_STATUS_ENT);

void prv_lua_to_c_MV_HWS_IPC_PORT_SERDES_TX_OFFSETS_STC(
    lua_State *L,
    MV_HWS_IPC_PORT_SERDES_TX_OFFSETS_STC *val
)
{
    F_NUMBER(val, -1, portGroup, GT_U16);
    F_NUMBER(val, -1, phyPortNum, GT_U16);
    F_NUMBER(val, -1, serdesSpeed, GT_U32);
    F_NUMBER(val, -1, serdesLane, GT_U8);
    F_NUMBER(val, -1, offsets, GT_U16);
}

void prv_c_to_lua_MV_HWS_IPC_PORT_SERDES_TX_OFFSETS_STC(
    lua_State *L,
    MV_HWS_IPC_PORT_SERDES_TX_OFFSETS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, portGroup, GT_U16);
    FO_NUMBER(val, t, phyPortNum, GT_U16);
    FO_NUMBER(val, t, serdesSpeed, GT_U32);
    FO_NUMBER(val, t, serdesLane, GT_U8);
    FO_NUMBER(val, t, offsets, GT_U16);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_IPC_PORT_SERDES_TX_OFFSETS_STC);

void prv_lua_to_c_CPSS_PM_AP_PORT_ADV_STC(
    lua_State *L,
    CPSS_PM_AP_PORT_ADV_STC *val
)
{
    F_ENUM(val, -1, ifMode, CPSS_PORT_INTERFACE_MODE_ENT);
    F_ENUM(val, -1, speed, CPSS_PORT_SPEED_ENT);
    F_ENUM(val, -1, fecSupported, CPSS_PORT_FEC_MODE_ENT);
    F_ENUM(val, -1, fecRequested, CPSS_PORT_FEC_MODE_ENT);
}

void prv_c_to_lua_CPSS_PM_AP_PORT_ADV_STC(
    lua_State *L,
    CPSS_PM_AP_PORT_ADV_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, ifMode, CPSS_PORT_INTERFACE_MODE_ENT);
    FO_ENUM(val, t, speed, CPSS_PORT_SPEED_ENT);
    FO_ENUM(val, t, fecSupported, CPSS_PORT_FEC_MODE_ENT);
    FO_ENUM(val, t, fecRequested, CPSS_PORT_FEC_MODE_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PM_AP_PORT_ADV_STC);

void prv_lua_to_c_MV_HWS_AUTO_TUNE_STATUS_RES(
    lua_State *L,
    MV_HWS_AUTO_TUNE_STATUS_RES *val
)
{
    F_ENUM(val, -1, rxTune, MV_HWS_AUTO_TUNE_STATUS);
    F_ENUM(val, -1, txTune, MV_HWS_AUTO_TUNE_STATUS);
}

void prv_c_to_lua_MV_HWS_AUTO_TUNE_STATUS_RES(
    lua_State *L,
    MV_HWS_AUTO_TUNE_STATUS_RES *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, rxTune, MV_HWS_AUTO_TUNE_STATUS);
    FO_ENUM(val, t, txTune, MV_HWS_AUTO_TUNE_STATUS);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_AUTO_TUNE_STATUS_RES);

add_mgm_enum(MV_HWS_DFE_MODE);

add_mgm_enum(CPSS_PORT_EGRESS_CNT_MODE_ENT);

void prv_lua_to_c_CPSS_PORT_COMPHY_C12GP41P2V_RX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_COMPHY_C12GP41P2V_RX_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, resSel, GT_U32);
    F_NUMBER(val, -1, resShift, GT_U32);
    F_NUMBER(val, -1, capSel, GT_U32);
    F_NUMBER(val, -1, ffeSettingForce, GT_U8);
    F_NUMBER(val, -1, adaptedResSel, GT_U8);
    F_NUMBER(val, -1, adaptedCapSel, GT_U8);
    F_NUMBER(val, -1, selmufi, GT_U32);
    F_NUMBER(val, -1, selmuff, GT_U32);
    F_NUMBER(val, -1, selmupi, GT_U32);
    F_NUMBER(val, -1, selmupf, GT_U32);
    F_NUMBER(val, -1, squelch, GT_32);
    F_NUMBER(val, -1, align90, GT_U32);
    F_NUMBER(val, -1, sampler, GT_U32);
    F_NUMBER(val, -1, slewRateCtrl0, GT_U32);
    F_NUMBER(val, -1, slewRateCtrl1, GT_U32);
    F_ARRAY_START(val, -1, dfe);
    {
        int idx;
        for (idx = 0; idx < 12; idx++) {
            F_ARRAY_NUMBER(val, dfe, idx, GT_32);
        }
    }
    F_ARRAY_END(val, -1, dfe);
    F_NUMBER(val, -1, EO, GT_U32);
}

void prv_c_to_lua_CPSS_PORT_COMPHY_C12GP41P2V_RX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_COMPHY_C12GP41P2V_RX_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, resSel, GT_U32);
    FO_NUMBER(val, t, resShift, GT_U32);
    FO_NUMBER(val, t, capSel, GT_U32);
    FO_NUMBER(val, t, ffeSettingForce, GT_U8);
    FO_NUMBER(val, t, adaptedResSel, GT_U8);
    FO_NUMBER(val, t, adaptedCapSel, GT_U8);
    FO_NUMBER(val, t, selmufi, GT_U32);
    FO_NUMBER(val, t, selmuff, GT_U32);
    FO_NUMBER(val, t, selmupi, GT_U32);
    FO_NUMBER(val, t, selmupf, GT_U32);
    FO_NUMBER(val, t, squelch, GT_32);
    FO_NUMBER(val, t, align90, GT_U32);
    FO_NUMBER(val, t, sampler, GT_U32);
    FO_NUMBER(val, t, slewRateCtrl0, GT_U32);
    FO_NUMBER(val, t, slewRateCtrl1, GT_U32);
    FO_ARRAY_START(val, t, dfe);
    {
        int idx;
        for (idx = 0; idx < 12; idx++) {
            FO_ARRAY_NUMBER(val, dfe, idx, GT_32);
        }
    }
    FO_ARRAY_END(val, t, dfe);
    FO_NUMBER(val, t, EO, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_COMPHY_C12GP41P2V_RX_CONFIG_STC);

add_mgm_enum(CPSS_PORT_FEC_MODE_ENT);

void prv_lua_to_c_CPSS_PM_PORT_ATTR_STC(
    lua_State *L,
    CPSS_PM_PORT_ATTR_STC *val
)
{
    F_NUMBER(val, -1, validAttrsBitMask, GT_U32);
    F_ENUM(val, -1, trainMode, CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT);
    F_BOOL(val, -1, adaptRxTrainSupp);
    F_BOOL(val, -1, edgeDetectSupported);
    F_STRUCT(val, -1, loopback, CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC);
    F_STRUCT(val, -1, etOverride, CPSS_PM_ET_OVERRIDE_CFG_STC);
    F_ENUM(val, -1, fecMode, CPSS_PORT_FEC_MODE_ENT);
    F_STRUCT(val, -1, calibrationMode, CPSS_PORT_MANAGER_CALIBRATION_MODE_STC);
    F_ENUM(val, -1, unMaskEventsMode, CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT);
    F_ENUM(val, -1, bwMode, CPSS_PORT_PA_BW_MODE_ENT);
    F_STRUCT(val, -1, linkBinding, CPSS_PORT_MANAGER_LKB_CONFIG_STC);
    F_NUMBER(val, -1, portExtraOperation, GT_U32);
    F_ENUM(val, -1, interconnectProfile, CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_ENT);
    F_STRUCT(val, -1, preemptionParams, CPSS_PM_MAC_PREEMPTION_PARAMS_STC);
    F_STRUCT(val, -1, autoNegotiation, CPSS_PORT_MANAGER_SGMII_AUTO_NEGOTIATION_STC);
    F_BOOL(val, -1, pmOverFw);
}

void prv_c_to_lua_CPSS_PM_PORT_ATTR_STC(
    lua_State *L,
    CPSS_PM_PORT_ATTR_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, validAttrsBitMask, GT_U32);
    FO_ENUM(val, t, trainMode, CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT);
    FO_BOOL(val, t, adaptRxTrainSupp);
    FO_BOOL(val, t, edgeDetectSupported);
    FO_STRUCT(val, t, loopback, CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC);
    FO_STRUCT(val, t, etOverride, CPSS_PM_ET_OVERRIDE_CFG_STC);
    FO_ENUM(val, t, fecMode, CPSS_PORT_FEC_MODE_ENT);
    FO_STRUCT(val, t, calibrationMode, CPSS_PORT_MANAGER_CALIBRATION_MODE_STC);
    FO_ENUM(val, t, unMaskEventsMode, CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT);
    FO_ENUM(val, t, bwMode, CPSS_PORT_PA_BW_MODE_ENT);
    FO_STRUCT(val, t, linkBinding, CPSS_PORT_MANAGER_LKB_CONFIG_STC);
    FO_NUMBER(val, t, portExtraOperation, GT_U32);
    FO_ENUM(val, t, interconnectProfile, CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_ENT);
    FO_STRUCT(val, t, preemptionParams, CPSS_PM_MAC_PREEMPTION_PARAMS_STC);
    FO_STRUCT(val, t, autoNegotiation, CPSS_PORT_MANAGER_SGMII_AUTO_NEGOTIATION_STC);
    FO_BOOL(val, t, pmOverFw);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PM_PORT_ATTR_STC);

add_mgm_enum(CPSS_BM_POOL_ALIGNMENT_ENT);

void prv_lua_to_c_CPSS_GEN_CFG_DEV_INFO_STC(
    lua_State *L,
    CPSS_GEN_CFG_DEV_INFO_STC *val
)
{
    F_NUMBER(val, -1, devType, CPSS_PP_DEVICE_TYPE);
    F_NUMBER(val, -1, revision, GT_U8);
    F_ENUM(val, -1, devFamily, CPSS_PP_FAMILY_TYPE_ENT);
    F_ENUM(val, -1, devSubFamily, CPSS_PP_SUB_FAMILY_TYPE_ENT);
    F_NUMBER(val, -1, maxPortNum, GT_U32);
    F_NUMBER(val, -1, numOfVirtPorts, GT_U32);
    F_STRUCT(val, -1, existingPorts, CPSS_PORTS_BMP_STC);
    F_ENUM(val, -1, hwDevNumMode, CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT);
    F_ENUM(val, -1, cpuPortMode, CPSS_NET_CPU_PORT_MODE_ENT);
    F_NUMBER(val, -1, numOfLedInfPerPortGroup, GT_U32);
    F_ARRAY_START(val, -1, supportedSipBmp);
    {
        int idx;
        for (idx = 0; idx < 1; idx++) {
            F_ARRAY_NUMBER(val, supportedSipBmp, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, supportedSipBmp);
}

void prv_c_to_lua_CPSS_GEN_CFG_DEV_INFO_STC(
    lua_State *L,
    CPSS_GEN_CFG_DEV_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, devType, CPSS_PP_DEVICE_TYPE);
    FO_NUMBER(val, t, revision, GT_U8);
    FO_ENUM(val, t, devFamily, CPSS_PP_FAMILY_TYPE_ENT);
    FO_ENUM(val, t, devSubFamily, CPSS_PP_SUB_FAMILY_TYPE_ENT);
    FO_NUMBER(val, t, maxPortNum, GT_U32);
    FO_NUMBER(val, t, numOfVirtPorts, GT_U32);
    FO_STRUCT(val, t, existingPorts, CPSS_PORTS_BMP_STC);
    FO_ENUM(val, t, hwDevNumMode, CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT);
    FO_ENUM(val, t, cpuPortMode, CPSS_NET_CPU_PORT_MODE_ENT);
    FO_NUMBER(val, t, numOfLedInfPerPortGroup, GT_U32);
    FO_ARRAY_START(val, t, supportedSipBmp);
    {
        int idx;
        for (idx = 0; idx < 1; idx++) {
            FO_ARRAY_NUMBER(val, supportedSipBmp, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, supportedSipBmp);
    lua_settop(L, t);
}
add_mgm_type(CPSS_GEN_CFG_DEV_INFO_STC);

add_mgm_enum(CPSS_TX_BUFF_ALLOC_METHOD_ENT);

void prv_lua_to_c_MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_DATA_STRUCT *val
)
{
    F_NUMBER(val, -1, portGroup, GT_U16);
    F_NUMBER(val, -1, phyPortNum, GT_U16);
    F_NUMBER(val, -1, portMode, GT_U32);
}

void prv_c_to_lua_MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_DATA_STRUCT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, portGroup, GT_U16);
    FO_NUMBER(val, t, phyPortNum, GT_U16);
    FO_NUMBER(val, t, portMode, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_IPC_PORT_PCS_ACTIVE_STATUS_GET_DATA_STRUCT);

void prv_lua_to_c_MV_HWS_SERDES_TXRX_TUNE_PARAMS(
    lua_State *L,
    MV_HWS_SERDES_TXRX_TUNE_PARAMS *val
)
{
    F_ENUM(val, -1, serdesSpeed, MV_HWS_SERDES_SPEED);
    F_UNION(val, -1, txParams, MV_HWS_SERDES_TX_CONFIG_DATA_UNT);
    F_UNION(val, -1, rxParams, MV_HWS_SERDES_RX_CONFIG_DATA_UNT);
}

void prv_c_to_lua_MV_HWS_SERDES_TXRX_TUNE_PARAMS(
    lua_State *L,
    MV_HWS_SERDES_TXRX_TUNE_PARAMS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, serdesSpeed, MV_HWS_SERDES_SPEED);
    FO_UNION(val, t, txParams, MV_HWS_SERDES_TX_CONFIG_DATA_UNT);
    FO_UNION(val, t, rxParams, MV_HWS_SERDES_RX_CONFIG_DATA_UNT);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_SERDES_TXRX_TUNE_PARAMS);

add_mgm_enum(MV_HWS_INPUT_SRC_CLOCK);

void prv_lua_to_c_MV_HWS_D2D_FRAME_CHECKER_CONFIG_STC(
    lua_State *L,
    MV_HWS_D2D_FRAME_CHECKER_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, rxPayloadType, GT_U32);
    F_NUMBER(val, -1, rxBadCntMask, GT_U32);
}

void prv_c_to_lua_MV_HWS_D2D_FRAME_CHECKER_CONFIG_STC(
    lua_State *L,
    MV_HWS_D2D_FRAME_CHECKER_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, rxPayloadType, GT_U32);
    FO_NUMBER(val, t, rxBadCntMask, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_D2D_FRAME_CHECKER_CONFIG_STC);

void prv_lua_to_c_MV_HWS_ENHANCE_TUNE_LITE_PHASE_PARAMS(
    lua_State *L,
    MV_HWS_ENHANCE_TUNE_LITE_PHASE_PARAMS *val
)
{
    F_NUMBER(val, -1, phase, GT_U8);
    F_NUMBER(val, -1, phaseFinished, GT_U8);
}

void prv_c_to_lua_MV_HWS_ENHANCE_TUNE_LITE_PHASE_PARAMS(
    lua_State *L,
    MV_HWS_ENHANCE_TUNE_LITE_PHASE_PARAMS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, phase, GT_U8);
    FO_NUMBER(val, t, phaseFinished, GT_U8);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_ENHANCE_TUNE_LITE_PHASE_PARAMS);

add_mgm_enum(CPSS_LOG_POINTER_FORMAT_ENT);

void prv_lua_to_c_MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA *val
)
{
    F_NUMBER(val, -1, pre2, GT_8);
    F_NUMBER(val, -1, pre, GT_8);
    F_NUMBER(val, -1, main, GT_8);
    F_NUMBER(val, -1, post, GT_8);
}

void prv_c_to_lua_MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, pre2, GT_8);
    FO_NUMBER(val, t, pre, GT_8);
    FO_NUMBER(val, t, main, GT_8);
    FO_NUMBER(val, t, post, GT_8);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA);

void prv_lua_to_c_MV_HWS_D2D_FRAME_TABLE_CONFIG_STC(
    lua_State *L,
    MV_HWS_D2D_FRAME_TABLE_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, frameRepeatCnt, GT_U32);
    F_NUMBER(val, -1, idleWord, GT_U32);
    F_NUMBER(val, -1, frameCommand, GT_U32);
    F_NUMBER(val, -1, frameLengthMax, GT_U32);
    F_NUMBER(val, -1, frameLengthMin, GT_U32);
    F_NUMBER(val, -1, framePayload, GT_U32);
    F_ARRAY_START(val, -1, frameHeader);
    {
        int idx;
        for (idx = 0; idx < 7; idx++) {
            F_ARRAY_NUMBER(val, frameHeader, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, frameHeader);
}

void prv_c_to_lua_MV_HWS_D2D_FRAME_TABLE_CONFIG_STC(
    lua_State *L,
    MV_HWS_D2D_FRAME_TABLE_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, frameRepeatCnt, GT_U32);
    FO_NUMBER(val, t, idleWord, GT_U32);
    FO_NUMBER(val, t, frameCommand, GT_U32);
    FO_NUMBER(val, t, frameLengthMax, GT_U32);
    FO_NUMBER(val, t, frameLengthMin, GT_U32);
    FO_NUMBER(val, t, framePayload, GT_U32);
    FO_ARRAY_START(val, t, frameHeader);
    {
        int idx;
        for (idx = 0; idx < 7; idx++) {
            FO_ARRAY_NUMBER(val, frameHeader, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, frameHeader);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_D2D_FRAME_TABLE_CONFIG_STC);

add_mgm_enum(MV_HWS_PORT_SERDES_OPERATION_MODE_ENT);

void prv_lua_to_c_MV_HWS_COMPHY_REG_FIELD_STC(
    lua_State *L,
    MV_HWS_COMPHY_REG_FIELD_STC *val
)
{
    F_NUMBER(val, -1, regOffset, GT_U32);
    F_NUMBER(val, -1, fieldStart, GT_U8);
    F_NUMBER(val, -1, fieldLen, GT_U8);
}

void prv_c_to_lua_MV_HWS_COMPHY_REG_FIELD_STC(
    lua_State *L,
    MV_HWS_COMPHY_REG_FIELD_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, regOffset, GT_U32);
    FO_NUMBER(val, t, fieldStart, GT_U8);
    FO_NUMBER(val, t, fieldLen, GT_U8);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_COMPHY_REG_FIELD_STC);

void prv_lua_to_c_MV_HWS_IPC_VOS_OVERRIDE_PARAMS(
    lua_State *L,
    MV_HWS_IPC_VOS_OVERRIDE_PARAMS *val
)
{
    F_NUMBER(val, -1, portGroup, GT_U16);
    F_NUMBER(val, -1, phyPortNum, GT_U16);
    F_NUMBER(val, -1, portMode, GT_U32);
    F_ARRAY_START(val, -1, vosOverrideParamsBufferPtr);
    {
        int idx;
        for (idx = 0; idx < 8; idx++) {
            F_ARRAY_NUMBER(val, vosOverrideParamsBufferPtr, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, vosOverrideParamsBufferPtr);
    F_NUMBER(val, -1, vosOverrideParamsBufferIdx, GT_U32);
    F_NUMBER(val, -1, vosOverrideParamsBufferLength, GT_U32);
}

void prv_c_to_lua_MV_HWS_IPC_VOS_OVERRIDE_PARAMS(
    lua_State *L,
    MV_HWS_IPC_VOS_OVERRIDE_PARAMS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, portGroup, GT_U16);
    FO_NUMBER(val, t, phyPortNum, GT_U16);
    FO_NUMBER(val, t, portMode, GT_U32);
    FO_ARRAY_START(val, t, vosOverrideParamsBufferPtr);
    {
        int idx;
        for (idx = 0; idx < 8; idx++) {
            FO_ARRAY_NUMBER(val, vosOverrideParamsBufferPtr, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, vosOverrideParamsBufferPtr);
    FO_NUMBER(val, t, vosOverrideParamsBufferIdx, GT_U32);
    FO_NUMBER(val, t, vosOverrideParamsBufferLength, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_IPC_VOS_OVERRIDE_PARAMS);

add_mgm_enum(CPSS_SYSTEM_RECOVERY_MANAGER_ENT);

add_mgm_enum(CPSS_LOG_API_FORMAT_ENT);

add_mgm_enum(MV_HWS_CPLL_INPUT_FREQUENCY);

void prv_lua_to_c_MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA *val
)
{
    F_NUMBER(val, -1, pre2, GT_8);
    F_NUMBER(val, -1, pre, GT_8);
    F_NUMBER(val, -1, main, GT_8);
    F_NUMBER(val, -1, post, GT_8);
    F_NUMBER(val, -1, usr, GT_8);
}

void prv_c_to_lua_MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, pre2, GT_8);
    FO_NUMBER(val, t, pre, GT_8);
    FO_NUMBER(val, t, main, GT_8);
    FO_NUMBER(val, t, post, GT_8);
    FO_NUMBER(val, t, usr, GT_8);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA);

void prv_lua_to_c_MV_HWS_ANP_MISC_PARAM_STC(
    lua_State *L,
    MV_HWS_ANP_MISC_PARAM_STC *val
)
{
    F_NUMBER(val, -1, interopLinkTimer, GT_U8);
}

void prv_c_to_lua_MV_HWS_ANP_MISC_PARAM_STC(
    lua_State *L,
    MV_HWS_ANP_MISC_PARAM_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, interopLinkTimer, GT_U8);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_ANP_MISC_PARAM_STC);

add_mgm_enum(CPSS_PORT_SERDES_SPEED_ENT);

void prv_lua_to_c_MV_HWS_IPC_PORT_RESET_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_RESET_DATA_STRUCT *val
)
{
    F_NUMBER(val, -1, portGroup, GT_U16);
    F_NUMBER(val, -1, phyPortNum, GT_U16);
    F_NUMBER(val, -1, portMode, GT_U32);
    F_NUMBER(val, -1, action, GT_U32);
}

void prv_c_to_lua_MV_HWS_IPC_PORT_RESET_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_RESET_DATA_STRUCT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, portGroup, GT_U16);
    FO_NUMBER(val, t, phyPortNum, GT_U16);
    FO_NUMBER(val, t, portMode, GT_U32);
    FO_NUMBER(val, t, action, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_IPC_PORT_RESET_DATA_STRUCT);

add_mgm_enum(MV_HWS_RESET);

void prv_lua_to_c_CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC(
    lua_State *L,
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC *val
)
{
    F_NUMBER(val, -1, dfxPipeId, GT_U32);
    F_NUMBER(val, -1, dfxClientId, GT_U32);
    F_NUMBER(val, -1, dfxMemoryId, GT_U32);
    F_STRUCT(val, -1, dfxInstance, CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_STC);
}

void prv_c_to_lua_CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC(
    lua_State *L,
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, dfxPipeId, GT_U32);
    FO_NUMBER(val, t, dfxClientId, GT_U32);
    FO_NUMBER(val, t, dfxMemoryId, GT_U32);
    FO_STRUCT(val, t, dfxInstance, CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC);

void prv_lua_to_c_CPSS_PORT_MANAGER_STATUS_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_STATUS_STC *val
)
{
    F_ENUM(val, -1, portState, CPSS_PORT_MANAGER_STATE_ENT);
    F_BOOL(val, -1, portUnderOperDisable);
    F_ENUM(val, -1, failure, CPSS_PORT_MANAGER_FAILURE_ENT);
    F_ENUM(val, -1, ifMode, CPSS_PORT_INTERFACE_MODE_ENT);
    F_ENUM(val, -1, speed, CPSS_PORT_SPEED_ENT);
    F_ENUM(val, -1, fecType, CPSS_PORT_FEC_MODE_ENT);
    F_BOOL(val, -1, remoteFaultConfig);
}

void prv_c_to_lua_CPSS_PORT_MANAGER_STATUS_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_STATUS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, portState, CPSS_PORT_MANAGER_STATE_ENT);
    FO_BOOL(val, t, portUnderOperDisable);
    FO_ENUM(val, t, failure, CPSS_PORT_MANAGER_FAILURE_ENT);
    FO_ENUM(val, t, ifMode, CPSS_PORT_INTERFACE_MODE_ENT);
    FO_ENUM(val, t, speed, CPSS_PORT_SPEED_ENT);
    FO_ENUM(val, t, fecType, CPSS_PORT_FEC_MODE_ENT);
    FO_BOOL(val, t, remoteFaultConfig);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_MANAGER_STATUS_STC);

void prv_lua_to_c_MV_HWS_SERDES_TEST_GEN_STATUS(
    lua_State *L,
    MV_HWS_SERDES_TEST_GEN_STATUS *val
)
{
    F_NUMBER(val, -1, errorsCntr, GT_U32);
    F_STRUCT(val, -1, txFramesCntr, GT_U64);
    F_NUMBER(val, -1, lockStatus, GT_U32);
}

void prv_c_to_lua_MV_HWS_SERDES_TEST_GEN_STATUS(
    lua_State *L,
    MV_HWS_SERDES_TEST_GEN_STATUS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, errorsCntr, GT_U32);
    FO_STRUCT(val, t, txFramesCntr, GT_U64);
    FO_NUMBER(val, t, lockStatus, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_SERDES_TEST_GEN_STATUS);

add_mgm_enum(MV_HWS_PORT_STANDARD);

add_mgm_enum(MV_HWS_SERDES_BUS_WIDTH_ENT);

add_mgm_enum(MV_HWS_RX_TRAINING_MODES);

void prv_lua_to_c_MV_HWS_AP_CFG(
    lua_State *L,
    MV_HWS_AP_CFG *val
)
{
    F_NUMBER(val, -1, apLaneNum, GT_U32);
    F_NUMBER(val, -1, modesVector, GT_U32);
    F_BOOL(val, -1, fcPause);
    F_BOOL(val, -1, fcAsmDir);
    F_BOOL(val, -1, fecSup);
    F_BOOL(val, -1, fecReq);
    F_BOOL(val, -1, nonceDis);
    F_STRUCT(val, -1, refClockCfg, MV_HWS_REF_CLOCK_CFG);
    F_NUMBER(val, -1, polarityVector, GT_U16);
    F_NUMBER(val, -1, fecAdvanceAbil, GT_U32);
    F_NUMBER(val, -1, fecAdvanceReq, GT_U32);
    F_NUMBER(val, -1, ctleBiasValue, GT_U32);
    F_NUMBER(val, -1, specialSpeeds, GT_U8);
    F_NUMBER(val, -1, extraOperation, GT_U32);
    F_BOOL(val, -1, skipRes);
}

void prv_c_to_lua_MV_HWS_AP_CFG(
    lua_State *L,
    MV_HWS_AP_CFG *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, apLaneNum, GT_U32);
    FO_NUMBER(val, t, modesVector, GT_U32);
    FO_BOOL(val, t, fcPause);
    FO_BOOL(val, t, fcAsmDir);
    FO_BOOL(val, t, fecSup);
    FO_BOOL(val, t, fecReq);
    FO_BOOL(val, t, nonceDis);
    FO_STRUCT(val, t, refClockCfg, MV_HWS_REF_CLOCK_CFG);
    FO_NUMBER(val, t, polarityVector, GT_U16);
    FO_NUMBER(val, t, fecAdvanceAbil, GT_U32);
    FO_NUMBER(val, t, fecAdvanceReq, GT_U32);
    FO_NUMBER(val, t, ctleBiasValue, GT_U32);
    FO_NUMBER(val, t, specialSpeeds, GT_U8);
    FO_NUMBER(val, t, extraOperation, GT_U32);
    FO_BOOL(val, t, skipRes);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_AP_CFG);

add_mgm_enum(CPSS_PORT_MANAGER_AN_RESOLUTION_BIT_ENT);

void prv_lua_to_c_CPSS_PORT_SERDES_RX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_SERDES_RX_CONFIG_STC *val
)
{
    F_ENUM(val, -1, type, CPSS_PORT_SERDES_TYPE_ENT);
    F_UNION(val, -1, rxTune, CPSS_PORT_SERDES_RX_CONFIG_UNT);
}

void prv_c_to_lua_CPSS_PORT_SERDES_RX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_SERDES_RX_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, type, CPSS_PORT_SERDES_TYPE_ENT);
    FO_UNION(val, t, rxTune, CPSS_PORT_SERDES_RX_CONFIG_UNT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_SERDES_RX_CONFIG_STC);

add_mgm_enum(MV_HWS_AVAGO_SERDES_HAL_TYPES);

void prv_lua_to_c_MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA *val
)
{
    F_NUMBER(val, -1, atten, GT_U8);
    F_NUMBER(val, -1, post, GT_8);
    F_NUMBER(val, -1, pre, GT_8);
    F_NUMBER(val, -1, pre2, GT_8);
    F_NUMBER(val, -1, pre3, GT_8);
}

void prv_c_to_lua_MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, atten, GT_U8);
    FO_NUMBER(val, t, post, GT_8);
    FO_NUMBER(val, t, pre, GT_8);
    FO_NUMBER(val, t, pre2, GT_8);
    FO_NUMBER(val, t, pre3, GT_8);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_MAN_TUNE_AVAGO_TX_CONFIG_DATA);

add_mgm_enum(CPSS_HW_PP_RESET_SKIP_TYPE_ENT);

add_mgm_enum(CPSS_DIAG_PP_REG_TYPE_ENT);

add_mgm_enum(CPSS_PORT_MANAGER_STATE_ENT);

void prv_lua_to_c_MV_HWS_AVAGO_16NM_TXRX_TUNE_PARAMS(
    lua_State *L,
    MV_HWS_AVAGO_16NM_TXRX_TUNE_PARAMS *val
)
{
    F_ENUM(val, -1, serdesSpeed, MV_HWS_SERDES_SPEED);
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* cannot identify type classUNKNOWN
MV_HWS_MAN_TUNE_AVAGO_16NM_TX_CONFIG_DATA txTuneData; */
#endif
    F_STRUCT(val, -1, rxTuneData, MV_HWS_MAN_TUNE_AVAGO_16NM_RX_CONFIG_DATA);
}

void prv_c_to_lua_MV_HWS_AVAGO_16NM_TXRX_TUNE_PARAMS(
    lua_State *L,
    MV_HWS_AVAGO_16NM_TXRX_TUNE_PARAMS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, serdesSpeed, MV_HWS_SERDES_SPEED);
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* cannot identify type classUNKNOWN
MV_HWS_MAN_TUNE_AVAGO_16NM_TX_CONFIG_DATA txTuneData; */
#endif
    FO_STRUCT(val, t, rxTuneData, MV_HWS_MAN_TUNE_AVAGO_16NM_RX_CONFIG_DATA);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_AVAGO_16NM_TXRX_TUNE_PARAMS);

add_mgm_enum(CPSS_SYSTEM_RECOVERY_STATE_ENT);

add_mgm_enum(CPSS_MEMORY_DUMP_TYPE_ENT);

add_mgm_enum(MV_HWS_REF_CLOCK);

void prv_lua_to_c_MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC(
    lua_State *L,
    MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC *val
)
{
    F_ARRAY_START(val, -1, coreStatusReady);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            F_ARRAY_BOOL(val, coreStatusReady, idx, GT_BOOL);
        }
    }
    F_ARRAY_END(val, -1, coreStatusReady);
    F_BOOL(val, -1, parametersInitialized);
}

void prv_c_to_lua_MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC(
    lua_State *L,
    MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, coreStatusReady);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            FO_ARRAY_BOOL(val, coreStatusReady, idx, GT_BOOL);
        }
    }
    FO_ARRAY_END(val, t, coreStatusReady);
    FO_BOOL(val, t, parametersInitialized);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_AVAGO_PORT_MANAGER_ONE_SHOT_TUNE_STC);

void prv_lua_to_c_MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_RX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_RX_CONFIG_DATA *val
)
{
    F_NUMBER(val, -1, resSel, GT_U8);
    F_NUMBER(val, -1, resShift, GT_U8);
    F_NUMBER(val, -1, capSel, GT_U8);
    F_NUMBER(val, -1, selmufi, GT_U8);
    F_NUMBER(val, -1, selmuff, GT_U8);
    F_NUMBER(val, -1, selmupi, GT_U8);
    F_NUMBER(val, -1, selmupf, GT_U8);
    F_NUMBER(val, -1, squelch, GT_16);
}

void prv_c_to_lua_MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_RX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_RX_CONFIG_DATA *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, resSel, GT_U8);
    FO_NUMBER(val, t, resShift, GT_U8);
    FO_NUMBER(val, t, capSel, GT_U8);
    FO_NUMBER(val, t, selmufi, GT_U8);
    FO_NUMBER(val, t, selmuff, GT_U8);
    FO_NUMBER(val, t, selmupi, GT_U8);
    FO_NUMBER(val, t, selmupf, GT_U8);
    FO_NUMBER(val, t, squelch, GT_16);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_RX_CONFIG_DATA);

void prv_lua_to_c_MV_HWS_MAN_TUNE_COMPHY_C28GP4_RX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_C28GP4_RX_CONFIG_DATA *val
)
{
    F_NUMBER(val, -1, dataRate, GT_U32);
    F_NUMBER(val, -1, res1Sel, GT_U32);
    F_NUMBER(val, -1, res2Sel, GT_U32);
    F_NUMBER(val, -1, cap1Sel, GT_U32);
    F_NUMBER(val, -1, cap2Sel, GT_U32);
    F_NUMBER(val, -1, selmufi, GT_U32);
    F_NUMBER(val, -1, selmuff, GT_U32);
    F_NUMBER(val, -1, selmupi, GT_U32);
    F_NUMBER(val, -1, selmupf, GT_U32);
    F_NUMBER(val, -1, midpointLargeThresKLane, GT_U32);
    F_NUMBER(val, -1, midpointSmallThresKLane, GT_U32);
    F_NUMBER(val, -1, midpointLargeThresCLane, GT_U32);
    F_NUMBER(val, -1, midpointSmallThresCLane, GT_U32);
    F_NUMBER(val, -1, dfeResF0aHighThresInitLane, GT_U32);
    F_NUMBER(val, -1, dfeResF0aHighThresEndLane, GT_U32);
    F_NUMBER(val, -1, squelch, GT_16);
}

void prv_c_to_lua_MV_HWS_MAN_TUNE_COMPHY_C28GP4_RX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_C28GP4_RX_CONFIG_DATA *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, dataRate, GT_U32);
    FO_NUMBER(val, t, res1Sel, GT_U32);
    FO_NUMBER(val, t, res2Sel, GT_U32);
    FO_NUMBER(val, t, cap1Sel, GT_U32);
    FO_NUMBER(val, t, cap2Sel, GT_U32);
    FO_NUMBER(val, t, selmufi, GT_U32);
    FO_NUMBER(val, t, selmuff, GT_U32);
    FO_NUMBER(val, t, selmupi, GT_U32);
    FO_NUMBER(val, t, selmupf, GT_U32);
    FO_NUMBER(val, t, midpointLargeThresKLane, GT_U32);
    FO_NUMBER(val, t, midpointSmallThresKLane, GT_U32);
    FO_NUMBER(val, t, midpointLargeThresCLane, GT_U32);
    FO_NUMBER(val, t, midpointSmallThresCLane, GT_U32);
    FO_NUMBER(val, t, dfeResF0aHighThresInitLane, GT_U32);
    FO_NUMBER(val, t, dfeResF0aHighThresEndLane, GT_U32);
    FO_NUMBER(val, t, squelch, GT_16);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_MAN_TUNE_COMPHY_C28GP4_RX_CONFIG_DATA);

add_mgm_enum(CPSS_PORT_SERDES_TYPE_ENT);

add_mgm_enum(CPSS_SYSTEM_RECOVERY_PROCESS_ENT);

void prv_lua_to_c_MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA *val
)
{
    F_NUMBER(val, -1, cur1Sel, GT_U32);
    F_NUMBER(val, -1, rl1Sel, GT_U32);
    F_NUMBER(val, -1, rl1Extra, GT_U32);
    F_NUMBER(val, -1, res1Sel, GT_U32);
    F_NUMBER(val, -1, cap1Sel, GT_U32);
    F_NUMBER(val, -1, enMidfreq, GT_U32);
    F_NUMBER(val, -1, cs1Mid, GT_U32);
    F_NUMBER(val, -1, rs1Mid, GT_U32);
    F_NUMBER(val, -1, cur2Sel, GT_U32);
    F_NUMBER(val, -1, rl2Sel, GT_U32);
    F_NUMBER(val, -1, rl2TuneG, GT_U32);
    F_NUMBER(val, -1, res2Sel, GT_U32);
    F_NUMBER(val, -1, cap2Sel, GT_U32);
    F_NUMBER(val, -1, selmufi, GT_U32);
    F_NUMBER(val, -1, selmuff, GT_U32);
    F_NUMBER(val, -1, selmupi, GT_U32);
    F_NUMBER(val, -1, selmupf, GT_U32);
    F_NUMBER(val, -1, squelch, GT_16);
}

void prv_c_to_lua_MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, cur1Sel, GT_U32);
    FO_NUMBER(val, t, rl1Sel, GT_U32);
    FO_NUMBER(val, t, rl1Extra, GT_U32);
    FO_NUMBER(val, t, res1Sel, GT_U32);
    FO_NUMBER(val, t, cap1Sel, GT_U32);
    FO_NUMBER(val, t, enMidfreq, GT_U32);
    FO_NUMBER(val, t, cs1Mid, GT_U32);
    FO_NUMBER(val, t, rs1Mid, GT_U32);
    FO_NUMBER(val, t, cur2Sel, GT_U32);
    FO_NUMBER(val, t, rl2Sel, GT_U32);
    FO_NUMBER(val, t, rl2TuneG, GT_U32);
    FO_NUMBER(val, t, res2Sel, GT_U32);
    FO_NUMBER(val, t, cap2Sel, GT_U32);
    FO_NUMBER(val, t, selmufi, GT_U32);
    FO_NUMBER(val, t, selmuff, GT_U32);
    FO_NUMBER(val, t, selmupi, GT_U32);
    FO_NUMBER(val, t, selmupf, GT_U32);
    FO_NUMBER(val, t, squelch, GT_16);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA);

add_mgm_enum(CPSS_PORT_XGMII_MODE_ENT);

void prv_lua_to_c_GT_VERSION(
    lua_State *L,
    GT_VERSION *val
)
{
    F_ARRAY_START(val, -1, version);
    {
        int idx;
        for (idx = 0; idx < 30; idx++) {
            F_ARRAY_NUMBER(val, version, idx, GT_U8);
        }
    }
    F_ARRAY_END(val, -1, version);
}

void prv_c_to_lua_GT_VERSION(
    lua_State *L,
    GT_VERSION *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, version);
    {
        int idx;
        for (idx = 0; idx < 30; idx++) {
            FO_ARRAY_NUMBER(val, version, idx, GT_U8);
        }
    }
    FO_ARRAY_END(val, t, version);
    lua_settop(L, t);
}
add_mgm_type(GT_VERSION);

add_mgm_enum(MV_HWS_CPLL_NUMBER);

add_mgm_enum(MV_HWS_SERDES_DIRECTION);

void prv_lua_to_c_MV_HWS_COMPHY_C28GP4_AUTO_TUNE_RESULTS(
    lua_State *L,
    MV_HWS_COMPHY_C28GP4_AUTO_TUNE_RESULTS *val
)
{
    F_STRUCT(val, -1, txComphyC28GP4, MV_HWS_MAN_TUNE_COMPHY_C28G_TX_CONFIG_DATA);
    F_STRUCT(val, -1, rxComphyC28GP4, MV_HWS_MAN_TUNE_COMPHY_C28GP4_RX_CONFIG_DATA);
    F_NUMBER(val, -1, eo, GT_U8);
    F_NUMBER(val, -1, align90, GT_U16);
    F_NUMBER(val, -1, sampler, GT_U16);
    F_NUMBER(val, -1, slewRateCtrl0, GT_U32);
    F_NUMBER(val, -1, slewRateCtrl1, GT_U32);
    F_ARRAY_START(val, -1, dfe);
    {
        int idx;
        for (idx = 0; idx < 25; idx++) {
            F_ARRAY_NUMBER(val, dfe, idx, GT_32);
        }
    }
    F_ARRAY_END(val, -1, dfe);
}

void prv_c_to_lua_MV_HWS_COMPHY_C28GP4_AUTO_TUNE_RESULTS(
    lua_State *L,
    MV_HWS_COMPHY_C28GP4_AUTO_TUNE_RESULTS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, txComphyC28GP4, MV_HWS_MAN_TUNE_COMPHY_C28G_TX_CONFIG_DATA);
    FO_STRUCT(val, t, rxComphyC28GP4, MV_HWS_MAN_TUNE_COMPHY_C28GP4_RX_CONFIG_DATA);
    FO_NUMBER(val, t, eo, GT_U8);
    FO_NUMBER(val, t, align90, GT_U16);
    FO_NUMBER(val, t, sampler, GT_U16);
    FO_NUMBER(val, t, slewRateCtrl0, GT_U32);
    FO_NUMBER(val, t, slewRateCtrl1, GT_U32);
    FO_ARRAY_START(val, t, dfe);
    {
        int idx;
        for (idx = 0; idx < 25; idx++) {
            FO_ARRAY_NUMBER(val, dfe, idx, GT_32);
        }
    }
    FO_ARRAY_END(val, t, dfe);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_COMPHY_C28GP4_AUTO_TUNE_RESULTS);

void prv_lua_to_c_MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_DATA_STRUCT *val
)
{
    F_NUMBER(val, -1, portGroup, GT_U16);
    F_NUMBER(val, -1, phyPortNum, GT_U16);
    F_NUMBER(val, -1, portMode, GT_U32);
    F_NUMBER(val, -1, portTuningMode, GT_U32);
    F_NUMBER(val, -1, optAlgoMask, GT_U32);
}

void prv_c_to_lua_MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_DATA_STRUCT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, portGroup, GT_U16);
    FO_NUMBER(val, t, phyPortNum, GT_U16);
    FO_NUMBER(val, t, portMode, GT_U32);
    FO_NUMBER(val, t, portTuningMode, GT_U32);
    FO_NUMBER(val, t, optAlgoMask, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_IPC_PORT_AUTO_TUNE_SET_EXT_DATA_STRUCT);

void prv_lua_to_c_CPSS_PM_ET_OVERRIDE_CFG_STC(
    lua_State *L,
    CPSS_PM_ET_OVERRIDE_CFG_STC *val
)
{
    F_NUMBER(val, -1, minLF, GT_U8);
    F_NUMBER(val, -1, maxLF, GT_U8);
}

void prv_c_to_lua_CPSS_PM_ET_OVERRIDE_CFG_STC(
    lua_State *L,
    CPSS_PM_ET_OVERRIDE_CFG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, minLF, GT_U8);
    FO_NUMBER(val, t, maxLF, GT_U8);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PM_ET_OVERRIDE_CFG_STC);

add_mgm_enum(CPSS_DIAG_PG_TRANSMIT_MODE_ENT);

void prv_lua_to_c_CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, lfMinThreshold, GT_U32);
    F_NUMBER(val, -1, lfMaxThreshold, GT_U32);
    F_NUMBER(val, -1, hfMinThreshold, GT_U32);
    F_NUMBER(val, -1, hfMaxThreshold, GT_U32);
    F_NUMBER(val, -1, eoMinThreshold, GT_U32);
    F_NUMBER(val, -1, eoMaxThreshold, GT_U32);
    F_NUMBER(val, -1, confidenceEnableBitMap, GT_U32);
    F_NUMBER(val, -1, calibrationTimeOutSec, GT_U32);
}

void prv_c_to_lua_CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, lfMinThreshold, GT_U32);
    FO_NUMBER(val, t, lfMaxThreshold, GT_U32);
    FO_NUMBER(val, t, hfMinThreshold, GT_U32);
    FO_NUMBER(val, t, hfMaxThreshold, GT_U32);
    FO_NUMBER(val, t, eoMinThreshold, GT_U32);
    FO_NUMBER(val, t, eoMaxThreshold, GT_U32);
    FO_NUMBER(val, t, confidenceEnableBitMap, GT_U32);
    FO_NUMBER(val, t, calibrationTimeOutSec, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_MANAGER_CONFIDENCE_DATA_CONFIG_STC);

add_mgm_enum(MV_HWS_PORT_MAC_TYPE);

add_mgm_enum(CPSS_PORT_MANAGER_PROGRESS_TYPE_ENT);

void prv_lua_to_c_MV_HWS_FCFEC_COUNTERS_STC(
    lua_State *L,
    MV_HWS_FCFEC_COUNTERS_STC *val
)
{
    F_NUMBER(val, -1, numReceivedBlocks, GT_U32);
    F_NUMBER(val, -1, numReceivedBlocksNoError, GT_U32);
    F_NUMBER(val, -1, numReceivedBlocksCorrectedError, GT_U32);
    F_NUMBER(val, -1, numReceivedBlocksUncorrectedError, GT_U32);
    F_NUMBER(val, -1, numReceivedCorrectedErrorBits, GT_U32);
    F_NUMBER(val, -1, numReceivedUncorrectedErrorBits, GT_U32);
    F_ARRAY_START(val, -1, blocksCorrectedError);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            F_ARRAY_NUMBER(val, blocksCorrectedError, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, blocksCorrectedError);
    F_ARRAY_START(val, -1, blocksUncorrectedError);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            F_ARRAY_NUMBER(val, blocksUncorrectedError, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, blocksUncorrectedError);
}

void prv_c_to_lua_MV_HWS_FCFEC_COUNTERS_STC(
    lua_State *L,
    MV_HWS_FCFEC_COUNTERS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, numReceivedBlocks, GT_U32);
    FO_NUMBER(val, t, numReceivedBlocksNoError, GT_U32);
    FO_NUMBER(val, t, numReceivedBlocksCorrectedError, GT_U32);
    FO_NUMBER(val, t, numReceivedBlocksUncorrectedError, GT_U32);
    FO_NUMBER(val, t, numReceivedCorrectedErrorBits, GT_U32);
    FO_NUMBER(val, t, numReceivedUncorrectedErrorBits, GT_U32);
    FO_ARRAY_START(val, t, blocksCorrectedError);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            FO_ARRAY_NUMBER(val, blocksCorrectedError, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, blocksCorrectedError);
    FO_ARRAY_START(val, t, blocksUncorrectedError);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            FO_ARRAY_NUMBER(val, blocksUncorrectedError, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, blocksUncorrectedError);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_FCFEC_COUNTERS_STC);

add_mgm_enum(CPSS_GEN_SIP_ENT);

void prv_lua_to_c_CPSS_PORT_AP_STATUS_STC(
    lua_State *L,
    CPSS_PORT_AP_STATUS_STC *val
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

void prv_c_to_lua_CPSS_PORT_AP_STATUS_STC(
    lua_State *L,
    CPSS_PORT_AP_STATUS_STC *val
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
add_mgm_type(CPSS_PORT_AP_STATUS_STC);

void prv_lua_to_c_MV_HWS_ELEMENTS_ARRAY(
    lua_State *L,
    MV_HWS_ELEMENTS_ARRAY *val
)
{
    F_NUMBER(val, -1, devNum, GT_U8);
    F_NUMBER(val, -1, portGroup, GT_U32);
    F_NUMBER(val, -1, elementNum, GT_U32);
}

void prv_c_to_lua_MV_HWS_ELEMENTS_ARRAY(
    lua_State *L,
    MV_HWS_ELEMENTS_ARRAY *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, devNum, GT_U8);
    FO_NUMBER(val, t, portGroup, GT_U32);
    FO_NUMBER(val, t, elementNum, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_ELEMENTS_ARRAY);

add_mgm_enum(CPSS_GEN_CFG_HW_DEV_NUM_MODE_ENT);

add_mgm_enum(CPSS_LED_BLINK_DUTY_CYCLE_ENT);

add_mgm_enum(CPSS_PP_FAMILY_TYPE_ENT);

add_mgm_enum(CPSS_PORT_INTERLAKEN_COUNTER_ID_ENT);

void prv_lua_to_c_MV_HWS_COMPHY_SERDES_DATA(
    lua_State *L,
    MV_HWS_COMPHY_SERDES_DATA *val
)
{
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* unhandled line
const MV_HWS_COMPHY_REG_FIELD_STC         *pinToRegMap; */
#endif
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* cannot identify type classUNKNOWN
MCESD_DEV_PTR                             sDev; */
#endif
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* unhandled line
const MV_HWS_SERDES_TXRX_TUNE_PARAMS      *tuneParams; */
#endif
    F_STRUCT_PTR(val, -1, tuneOverrideParams, tuneOverrideParams, MV_HWS_SERDES_TXRX_TUNE_PARAMS);
}

void prv_c_to_lua_MV_HWS_COMPHY_SERDES_DATA(
    lua_State *L,
    MV_HWS_COMPHY_SERDES_DATA *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* unhandled line
const MV_HWS_COMPHY_REG_FIELD_STC         *pinToRegMap; */
#endif
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* cannot identify type classUNKNOWN
MCESD_DEV_PTR                             sDev; */
#endif
#ifndef _VISUALC
# warning "TBD manually"
#else
# pragma message ( "TBD manually" )
#endif
#if 0
    /* unhandled line
const MV_HWS_SERDES_TXRX_TUNE_PARAMS      *tuneParams; */
#endif
    FO_STRUCT_PTR(val, t, tuneOverrideParams, tuneOverrideParams, MV_HWS_SERDES_TXRX_TUNE_PARAMS);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_COMPHY_SERDES_DATA);

add_mgm_enum(CPSS_DP_LEVEL_ENT);

add_mgm_enum(CPSS_DIAG_PG_PACKET_PAYLOAD_TYPE_ENT);

void prv_lua_to_c_CPSS_PM_AP_PORT_PARAMS_STC(
    lua_State *L,
    CPSS_PM_AP_PORT_PARAMS_STC *val
)
{
    F_NUMBER(val, -1, numOfModes, GT_U32);
    F_ARRAY_START(val, -1, modesArr);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            F_ARRAY_STRUCT(val, modesArr, idx, CPSS_PM_AP_PORT_ADV_STC);
        }
    }
    F_ARRAY_END(val, -1, modesArr);
    F_STRUCT(val, -1, apAttrs, CPSS_PM_AP_PORT_ATTR_STC);
}

void prv_c_to_lua_CPSS_PM_AP_PORT_PARAMS_STC(
    lua_State *L,
    CPSS_PM_AP_PORT_PARAMS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, numOfModes, GT_U32);
    FO_ARRAY_START(val, t, modesArr);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            FO_ARRAY_STRUCT(val, modesArr, idx, CPSS_PM_AP_PORT_ADV_STC);
        }
    }
    FO_ARRAY_END(val, t, modesArr);
    FO_STRUCT(val, t, apAttrs, CPSS_PM_AP_PORT_ATTR_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PM_AP_PORT_PARAMS_STC);

add_mgm_enum(CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT);

void prv_lua_to_c_CPSS_HW_INFO_RESOURCE_MAPPING_STC(
    lua_State *L,
    CPSS_HW_INFO_RESOURCE_MAPPING_STC *val
)
{
    F_NUMBER(val, -1, start, GT_UINTPTR);
    F_NUMBER(val, -1, size, GT_UINTPTR);
    F_NUMBER(val, -1, phys, GT_PHYSICAL_ADDR);
}

void prv_c_to_lua_CPSS_HW_INFO_RESOURCE_MAPPING_STC(
    lua_State *L,
    CPSS_HW_INFO_RESOURCE_MAPPING_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, start, GT_UINTPTR);
    FO_NUMBER(val, t, size, GT_UINTPTR);
    FO_NUMBER(val, t, phys, GT_PHYSICAL_ADDR);
    lua_settop(L, t);
}
add_mgm_type(CPSS_HW_INFO_RESOURCE_MAPPING_STC);

void prv_lua_to_c_MV_HWS_PORT_INIT_PARAMS(
    lua_State *L,
    MV_HWS_PORT_INIT_PARAMS *val
)
{
    F_ENUM(val, -1, portStandard, MV_HWS_PORT_STANDARD);
    F_ENUM(val, -1, portMacType, MV_HWS_PORT_MAC_TYPE);
    F_NUMBER(val, -1, portMacNumber, GT_U16);
    F_ENUM(val, -1, portPcsType, MV_HWS_PORT_PCS_TYPE);
    F_NUMBER(val, -1, portPcsNumber, GT_U16);
    F_ENUM(val, -1, portFecMode, MV_HWS_PORT_FEC_MODE);
    F_ENUM(val, -1, serdesSpeed, MV_HWS_SERDES_SPEED);
    F_NUMBER(val, -1, firstLaneNum, GT_U16);
    F_ARRAY_START(val, -1, activeLanesList);
    {
        int idx;
        for (idx = 0; idx < MV_HWS_MAX_LANES_NUM_PER_PORT; idx++) {
            F_ARRAY_NUMBER(val, activeLanesList, idx, GT_U16);
        }
    }
    F_ARRAY_END(val, -1, activeLanesList);
    F_NUMBER(val, -1, numOfActLanes, GT_U8);
    F_ENUM(val, -1, serdesMediaType, MV_HWS_SERDES_MEDIA);
    F_ENUM(val, -1, serdes10BitStatus, MV_HWS_SERDES_BUS_WIDTH_ENT);
    F_NUMBER(val, -1, fecSupportedModesBmp, GT_U8);
}

void prv_c_to_lua_MV_HWS_PORT_INIT_PARAMS(
    lua_State *L,
    MV_HWS_PORT_INIT_PARAMS *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, portStandard, MV_HWS_PORT_STANDARD);
    FO_ENUM(val, t, portMacType, MV_HWS_PORT_MAC_TYPE);
    FO_NUMBER(val, t, portMacNumber, GT_U16);
    FO_ENUM(val, t, portPcsType, MV_HWS_PORT_PCS_TYPE);
    FO_NUMBER(val, t, portPcsNumber, GT_U16);
    FO_ENUM(val, t, portFecMode, MV_HWS_PORT_FEC_MODE);
    FO_ENUM(val, t, serdesSpeed, MV_HWS_SERDES_SPEED);
    FO_NUMBER(val, t, firstLaneNum, GT_U16);
    FO_ARRAY_START(val, t, activeLanesList);
    {
        int idx;
        for (idx = 0; idx < MV_HWS_MAX_LANES_NUM_PER_PORT; idx++) {
            FO_ARRAY_NUMBER(val, activeLanesList, idx, GT_U16);
        }
    }
    FO_ARRAY_END(val, t, activeLanesList);
    FO_NUMBER(val, t, numOfActLanes, GT_U8);
    FO_ENUM(val, t, serdesMediaType, MV_HWS_SERDES_MEDIA);
    FO_ENUM(val, t, serdes10BitStatus, MV_HWS_SERDES_BUS_WIDTH_ENT);
    FO_NUMBER(val, t, fecSupportedModesBmp, GT_U8);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_PORT_INIT_PARAMS);

add_mgm_enum(CPSS_PM_AP_LANE_SERDES_SPEED);

add_mgm_enum(CPSS_LOG_TIME_FORMAT_ENT);

add_mgm_enum(CPSS_FORMAT_CONVERT_FIELD_CONTEXT_ENT);

void prv_lua_to_c_CPSS_PORT_COMPHY_SERDES_TX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_COMPHY_SERDES_TX_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, txAmp, GT_U32);
    F_BOOL(val, -1, txAmpAdjEn);
    F_NUMBER(val, -1, emph0, GT_32);
    F_NUMBER(val, -1, emph1, GT_32);
    F_BOOL(val, -1, txAmpShft);
    F_BOOL(val, -1, txEmphEn);
    F_BOOL(val, -1, txEmphEn1);
    F_NUMBER(val, -1, txAmpAdj, GT_U32);
    F_BOOL(val, -1, slewCtrlEn);
    F_NUMBER(val, -1, slewRate, GT_U32);
}

void prv_c_to_lua_CPSS_PORT_COMPHY_SERDES_TX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_COMPHY_SERDES_TX_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, txAmp, GT_U32);
    FO_BOOL(val, t, txAmpAdjEn);
    FO_NUMBER(val, t, emph0, GT_32);
    FO_NUMBER(val, t, emph1, GT_32);
    FO_BOOL(val, t, txAmpShft);
    FO_BOOL(val, t, txEmphEn);
    FO_BOOL(val, t, txEmphEn1);
    FO_NUMBER(val, t, txAmpAdj, GT_U32);
    FO_BOOL(val, t, slewCtrlEn);
    FO_NUMBER(val, t, slewRate, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_COMPHY_SERDES_TX_CONFIG_STC);

void prv_lua_to_c_MV_HWS_IPC_PORT_POLARITY_SET_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_POLARITY_SET_DATA_STRUCT *val
)
{
    F_NUMBER(val, -1, portGroup, GT_U16);
    F_NUMBER(val, -1, phyPortNum, GT_U16);
    F_NUMBER(val, -1, portMode, GT_U32);
    F_NUMBER(val, -1, txInvMask, GT_32);
    F_NUMBER(val, -1, rxInvMask, GT_32);
}

void prv_c_to_lua_MV_HWS_IPC_PORT_POLARITY_SET_DATA_STRUCT(
    lua_State *L,
    MV_HWS_IPC_PORT_POLARITY_SET_DATA_STRUCT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, portGroup, GT_U16);
    FO_NUMBER(val, t, phyPortNum, GT_U16);
    FO_NUMBER(val, t, portMode, GT_U32);
    FO_NUMBER(val, t, txInvMask, GT_32);
    FO_NUMBER(val, t, rxInvMask, GT_32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_IPC_PORT_POLARITY_SET_DATA_STRUCT);

void prv_lua_to_c_MV_HWS_ALIGN90_PARAMS(
    lua_State *L,
    MV_HWS_ALIGN90_PARAMS *val
)
{
    F_NUMBER(val, -1, startAlign90, GT_U32);
    F_NUMBER(val, -1, rxTrainingCfg, GT_U32);
    F_NUMBER(val, -1, osDeltaMax, GT_U32);
    F_NUMBER(val, -1, adaptedFfeR, GT_U32);
    F_NUMBER(val, -1, adaptedFfeC, GT_U32);
}

void prv_c_to_lua_MV_HWS_ALIGN90_PARAMS(
    lua_State *L,
    MV_HWS_ALIGN90_PARAMS *val
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
add_mgm_type(MV_HWS_ALIGN90_PARAMS);

void prv_lua_to_c_MV_HWS_RSFEC_COUNTERS_STC(
    lua_State *L,
    MV_HWS_RSFEC_COUNTERS_STC *val
)
{
    F_STRUCT(val, -1, correctedFecCodeword, GT_U64);
    F_STRUCT(val, -1, uncorrectedFecCodeword, GT_U64);
    F_STRUCT(val, -1, correctFecCodeword, GT_U64);
    F_ARRAY_START(val, -1, symbolError);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            F_ARRAY_NUMBER(val, symbolError, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, symbolError);
    F_STRUCT(val, -1, total_cw_received, GT_U64);
}

void prv_c_to_lua_MV_HWS_RSFEC_COUNTERS_STC(
    lua_State *L,
    MV_HWS_RSFEC_COUNTERS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, correctedFecCodeword, GT_U64);
    FO_STRUCT(val, t, uncorrectedFecCodeword, GT_U64);
    FO_STRUCT(val, t, correctFecCodeword, GT_U64);
    FO_ARRAY_START(val, t, symbolError);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            FO_ARRAY_NUMBER(val, symbolError, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, symbolError);
    FO_STRUCT(val, t, total_cw_received, GT_U64);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_RSFEC_COUNTERS_STC);

void prv_lua_to_c_CPSS_PORT_SERDES_TX_CONFIG_UNT(
    lua_State *L,
    CPSS_PORT_SERDES_TX_CONFIG_UNT *val
)
{
    F_UNION_MEMBER_STRUCT(val, -1, comphy, CPSS_PORT_COMPHY_SERDES_TX_CONFIG_STC);
    F_UNION_MEMBER_STRUCT(val, -1, comphy_C12G, CPSS_PORT_COMPHY_C12G_TX_CONFIG_STC);
    F_UNION_MEMBER_STRUCT(val, -1, comphy_C28G, CPSS_PORT_COMPHY_C28G_TX_CONFIG_STC);
    F_UNION_MEMBER_STRUCT(val, -1, comphy_C56G, CPSS_PORT_COMPHY_C56G_TX_CONFIG_STC);
    F_UNION_MEMBER_STRUCT(val, -1, comphy_C112G, CPSS_PORT_COMPHY_C112G_TX_CONFIG_STC);
    F_UNION_MEMBER_STRUCT(val, -1, avago, CPSS_PORT_AVAGO_SERDES_TX_CONFIG_STC);
}

void prv_c_to_lua_CPSS_PORT_SERDES_TX_CONFIG_UNT(
    lua_State *L,
    CPSS_PORT_SERDES_TX_CONFIG_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, comphy, CPSS_PORT_COMPHY_SERDES_TX_CONFIG_STC);
    FO_STRUCT(val, t, comphy_C12G, CPSS_PORT_COMPHY_C12G_TX_CONFIG_STC);
    FO_STRUCT(val, t, comphy_C28G, CPSS_PORT_COMPHY_C28G_TX_CONFIG_STC);
    FO_STRUCT(val, t, comphy_C56G, CPSS_PORT_COMPHY_C56G_TX_CONFIG_STC);
    FO_STRUCT(val, t, comphy_C112G, CPSS_PORT_COMPHY_C112G_TX_CONFIG_STC);
    FO_STRUCT(val, t, avago, CPSS_PORT_AVAGO_SERDES_TX_CONFIG_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_SERDES_TX_CONFIG_UNT);
add_mgm_union(CPSS_PORT_SERDES_TX_CONFIG_UNT);

void prv_lua_to_c_CPSS_PORT_AVAGO_SERDES_RX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_AVAGO_SERDES_RX_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, sqlch, GT_U32);
    F_NUMBER(val, -1, DC, GT_U32);
    F_NUMBER(val, -1, LF, GT_U32);
    F_NUMBER(val, -1, HF, GT_U32);
    F_NUMBER(val, -1, BW, GT_U32);
    F_NUMBER(val, -1, EO, GT_U32);
    F_ARRAY_START(val, -1, DFE);
    {
        int idx;
        for (idx = 0; idx < 13; idx++) {
            F_ARRAY_NUMBER(val, DFE, idx, GT_32);
        }
    }
    F_ARRAY_END(val, -1, DFE);
    F_NUMBER(val, -1, gainshape1, GT_U32);
    F_NUMBER(val, -1, gainshape2, GT_U32);
    F_NUMBER(val, -1, shortChannelEn, GT_U32);
    F_NUMBER(val, -1, dfeGAIN, GT_U32);
    F_NUMBER(val, -1, dfeGAIN2, GT_U32);
    F_NUMBER(val, -1, BFLF, GT_32);
    F_NUMBER(val, -1, BFHF, GT_32);
    F_NUMBER(val, -1, minLf, GT_U32);
    F_NUMBER(val, -1, maxLf, GT_U32);
    F_NUMBER(val, -1, minHf, GT_U32);
    F_NUMBER(val, -1, maxHf, GT_U32);
    F_NUMBER(val, -1, minPre1, GT_32);
    F_NUMBER(val, -1, maxPre1, GT_32);
    F_NUMBER(val, -1, minPre2, GT_32);
    F_NUMBER(val, -1, maxPre2, GT_32);
    F_NUMBER(val, -1, minPost, GT_32);
    F_NUMBER(val, -1, maxPost, GT_32);
    F_NUMBER(val, -1, pre1, GT_32);
    F_NUMBER(val, -1, pre2, GT_32);
    F_NUMBER(val, -1, post1, GT_32);
    F_ARRAY_START(val, -1, pam4EyesArr);
    {
        int idx;
        for (idx = 0; idx < 6; idx++) {
            F_ARRAY_NUMBER(val, pam4EyesArr, idx, GT_32);
        }
    }
    F_ARRAY_END(val, -1, pam4EyesArr);
    F_ENUM(val, -1, termination, CPSS_SERDES_TERMINATION);
    F_NUMBER(val, -1, coldEnvelope, GT_U32);
    F_NUMBER(val, -1, hotEnvelope, GT_U32);
}

void prv_c_to_lua_CPSS_PORT_AVAGO_SERDES_RX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_AVAGO_SERDES_RX_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, sqlch, GT_U32);
    FO_NUMBER(val, t, DC, GT_U32);
    FO_NUMBER(val, t, LF, GT_U32);
    FO_NUMBER(val, t, HF, GT_U32);
    FO_NUMBER(val, t, BW, GT_U32);
    FO_NUMBER(val, t, EO, GT_U32);
    FO_ARRAY_START(val, t, DFE);
    {
        int idx;
        for (idx = 0; idx < 13; idx++) {
            FO_ARRAY_NUMBER(val, DFE, idx, GT_32);
        }
    }
    FO_ARRAY_END(val, t, DFE);
    FO_NUMBER(val, t, gainshape1, GT_U32);
    FO_NUMBER(val, t, gainshape2, GT_U32);
    FO_NUMBER(val, t, shortChannelEn, GT_U32);
    FO_NUMBER(val, t, dfeGAIN, GT_U32);
    FO_NUMBER(val, t, dfeGAIN2, GT_U32);
    FO_NUMBER(val, t, BFLF, GT_32);
    FO_NUMBER(val, t, BFHF, GT_32);
    FO_NUMBER(val, t, minLf, GT_U32);
    FO_NUMBER(val, t, maxLf, GT_U32);
    FO_NUMBER(val, t, minHf, GT_U32);
    FO_NUMBER(val, t, maxHf, GT_U32);
    FO_NUMBER(val, t, minPre1, GT_32);
    FO_NUMBER(val, t, maxPre1, GT_32);
    FO_NUMBER(val, t, minPre2, GT_32);
    FO_NUMBER(val, t, maxPre2, GT_32);
    FO_NUMBER(val, t, minPost, GT_32);
    FO_NUMBER(val, t, maxPost, GT_32);
    FO_NUMBER(val, t, pre1, GT_32);
    FO_NUMBER(val, t, pre2, GT_32);
    FO_NUMBER(val, t, post1, GT_32);
    FO_ARRAY_START(val, t, pam4EyesArr);
    {
        int idx;
        for (idx = 0; idx < 6; idx++) {
            FO_ARRAY_NUMBER(val, pam4EyesArr, idx, GT_32);
        }
    }
    FO_ARRAY_END(val, t, pam4EyesArr);
    FO_ENUM(val, t, termination, CPSS_SERDES_TERMINATION);
    FO_NUMBER(val, t, coldEnvelope, GT_U32);
    FO_NUMBER(val, t, hotEnvelope, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_AVAGO_SERDES_RX_CONFIG_STC);

void prv_lua_to_c_MV_HWS_MAN_TUNE_COMPHY_TX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_TX_CONFIG_DATA *val
)
{
    F_NUMBER(val, -1, pre, GT_U8);
    F_NUMBER(val, -1, peak, GT_U8);
    F_NUMBER(val, -1, post, GT_U8);
}

void prv_c_to_lua_MV_HWS_MAN_TUNE_COMPHY_TX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_TX_CONFIG_DATA *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, pre, GT_U8);
    FO_NUMBER(val, t, peak, GT_U8);
    FO_NUMBER(val, t, post, GT_U8);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_MAN_TUNE_COMPHY_TX_CONFIG_DATA);

void prv_lua_to_c_MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA *val
)
{
    F_NUMBER(val, -1, current1Sel, GT_U32);
    F_NUMBER(val, -1, rl1Sel, GT_U32);
    F_NUMBER(val, -1, rl1Extra, GT_U32);
    F_NUMBER(val, -1, res1Sel, GT_U32);
    F_NUMBER(val, -1, cap1Sel, GT_U32);
    F_NUMBER(val, -1, cl1Ctrl, GT_U32);
    F_NUMBER(val, -1, enMidFreq, GT_U32);
    F_NUMBER(val, -1, cs1Mid, GT_U32);
    F_NUMBER(val, -1, rs1Mid, GT_U32);
    F_NUMBER(val, -1, rfCtrl, GT_U32);
    F_NUMBER(val, -1, rl1TiaSel, GT_U32);
    F_NUMBER(val, -1, rl1TiaExtra, GT_U32);
    F_NUMBER(val, -1, hpfRSel1st, GT_U32);
    F_NUMBER(val, -1, current1TiaSel, GT_U32);
    F_NUMBER(val, -1, rl2Tune, GT_U32);
    F_NUMBER(val, -1, rl2Sel, GT_U32);
    F_NUMBER(val, -1, rs2Sel, GT_U32);
    F_NUMBER(val, -1, current2Sel, GT_U32);
    F_NUMBER(val, -1, cap2Sel, GT_U32);
    F_NUMBER(val, -1, hpfRsel2nd, GT_U32);
    F_NUMBER(val, -1, selmufi, GT_U32);
    F_NUMBER(val, -1, selmuff, GT_U32);
    F_NUMBER(val, -1, selmupi, GT_U32);
    F_NUMBER(val, -1, selmupf, GT_U32);
    F_NUMBER(val, -1, squelch, GT_U8);
}

void prv_c_to_lua_MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA(
    lua_State *L,
    MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, current1Sel, GT_U32);
    FO_NUMBER(val, t, rl1Sel, GT_U32);
    FO_NUMBER(val, t, rl1Extra, GT_U32);
    FO_NUMBER(val, t, res1Sel, GT_U32);
    FO_NUMBER(val, t, cap1Sel, GT_U32);
    FO_NUMBER(val, t, cl1Ctrl, GT_U32);
    FO_NUMBER(val, t, enMidFreq, GT_U32);
    FO_NUMBER(val, t, cs1Mid, GT_U32);
    FO_NUMBER(val, t, rs1Mid, GT_U32);
    FO_NUMBER(val, t, rfCtrl, GT_U32);
    FO_NUMBER(val, t, rl1TiaSel, GT_U32);
    FO_NUMBER(val, t, rl1TiaExtra, GT_U32);
    FO_NUMBER(val, t, hpfRSel1st, GT_U32);
    FO_NUMBER(val, t, current1TiaSel, GT_U32);
    FO_NUMBER(val, t, rl2Tune, GT_U32);
    FO_NUMBER(val, t, rl2Sel, GT_U32);
    FO_NUMBER(val, t, rs2Sel, GT_U32);
    FO_NUMBER(val, t, current2Sel, GT_U32);
    FO_NUMBER(val, t, cap2Sel, GT_U32);
    FO_NUMBER(val, t, hpfRsel2nd, GT_U32);
    FO_NUMBER(val, t, selmufi, GT_U32);
    FO_NUMBER(val, t, selmuff, GT_U32);
    FO_NUMBER(val, t, selmupi, GT_U32);
    FO_NUMBER(val, t, selmupf, GT_U32);
    FO_NUMBER(val, t, squelch, GT_U8);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA);

add_mgm_enum(CPSS_IP_PROTOCOL_STACK_ENT);

add_mgm_enum(CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT);

void prv_lua_to_c_CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC *val
)
{
    F_ENUM(val, -1, loopbackType, CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT);
    lua_getfield(L, -1, "loopbackMode");
    if (lua_istable(L, -1))
    {
        F_BOOL(&(val->loopbackMode), -1, macLoopbackEnable);
        F_ENUM(&(val->loopbackMode), -1, pcsLoopbackMode, CPSS_PORT_PCS_LOOPBACK_MODE_ENT);
        F_ENUM(&(val->loopbackMode), -1, serdesLoopbackMode, CPSS_PORT_SERDES_LOOPBACK_MODE_ENT);
    }
    lua_pop(L, 1);
    F_BOOL(val, -1, enableRegularTrafic);
}

void prv_c_to_lua_CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, loopbackType, CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT);
    {
        int t1;
        lua_newtable(L);
        t1 = lua_gettop(L);
            FO_BOOL(&(val->loopbackMode), t1, macLoopbackEnable);
            FO_ENUM(&(val->loopbackMode), t1, pcsLoopbackMode, CPSS_PORT_PCS_LOOPBACK_MODE_ENT);
            FO_ENUM(&(val->loopbackMode), t1, serdesLoopbackMode, CPSS_PORT_SERDES_LOOPBACK_MODE_ENT);
        lua_setfield(L, t, "loopbackMode");
    }
    FO_BOOL(val, t, enableRegularTrafic);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC);

void prv_lua_to_c_MV_HWS_AVG_BLOCK(
    lua_State *L,
    MV_HWS_AVG_BLOCK *val
)
{
    F_NUMBER(val, -1, revision, GT_U8);
    F_NUMBER(val, -1, avgBlockLines, GT_U8);
    F_NUMBER(val, -1, minVal, GT_U8);
    F_NUMBER(val, -1, baseVal, GT_U8);
    F_NUMBER(val, -1, bitsPerSds, GT_U8);
}

void prv_c_to_lua_MV_HWS_AVG_BLOCK(
    lua_State *L,
    MV_HWS_AVG_BLOCK *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, revision, GT_U8);
    FO_NUMBER(val, t, avgBlockLines, GT_U8);
    FO_NUMBER(val, t, minVal, GT_U8);
    FO_NUMBER(val, t, baseVal, GT_U8);
    FO_NUMBER(val, t, bitsPerSds, GT_U8);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_AVG_BLOCK);

void prv_lua_to_c_CPSS_PORT_COMPHY_C112G_RX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_COMPHY_C112G_RX_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, current1Sel, GT_U32);
    F_NUMBER(val, -1, rl1Sel, GT_U32);
    F_NUMBER(val, -1, rl1Extra, GT_U32);
    F_NUMBER(val, -1, res1Sel, GT_U32);
    F_NUMBER(val, -1, cap1Sel, GT_U32);
    F_NUMBER(val, -1, cl1Ctrl, GT_U32);
    F_NUMBER(val, -1, enMidFreq, GT_U32);
    F_NUMBER(val, -1, cs1Mid, GT_U32);
    F_NUMBER(val, -1, rs1Mid, GT_U32);
    F_NUMBER(val, -1, rfCtrl, GT_U32);
    F_NUMBER(val, -1, rl1TiaSel, GT_U32);
    F_NUMBER(val, -1, rl1TiaExtra, GT_U32);
    F_NUMBER(val, -1, hpfRSel1st, GT_U32);
    F_NUMBER(val, -1, current1TiaSel, GT_U32);
    F_NUMBER(val, -1, rl2Tune, GT_U32);
    F_NUMBER(val, -1, rl2Sel, GT_U32);
    F_NUMBER(val, -1, rs2Sel, GT_U32);
    F_NUMBER(val, -1, current2Sel, GT_U32);
    F_NUMBER(val, -1, cap2Sel, GT_U32);
    F_NUMBER(val, -1, hpfRsel2nd, GT_U32);
    F_NUMBER(val, -1, selmufi, GT_U32);
    F_NUMBER(val, -1, selmuff, GT_U32);
    F_NUMBER(val, -1, selmupi, GT_U32);
    F_NUMBER(val, -1, selmupf, GT_U32);
    F_NUMBER(val, -1, squelch, GT_U32);
    F_NUMBER(val, -1, align90AnaReg, GT_U32);
    F_NUMBER(val, -1, align90, GT_32);
    F_NUMBER(val, -1, sampler, GT_U32);
    F_NUMBER(val, -1, slewRateCtrl0, GT_U32);
    F_NUMBER(val, -1, slewRateCtrl1, GT_U32);
    F_ARRAY_START(val, -1, dfe);
    {
        int idx;
        for (idx = 0; idx < 40; idx++) {
            F_ARRAY_NUMBER(val, dfe, idx, GT_32);
        }
    }
    F_ARRAY_END(val, -1, dfe);
    F_NUMBER(val, -1, EO, GT_U32);
}

void prv_c_to_lua_CPSS_PORT_COMPHY_C112G_RX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_COMPHY_C112G_RX_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, current1Sel, GT_U32);
    FO_NUMBER(val, t, rl1Sel, GT_U32);
    FO_NUMBER(val, t, rl1Extra, GT_U32);
    FO_NUMBER(val, t, res1Sel, GT_U32);
    FO_NUMBER(val, t, cap1Sel, GT_U32);
    FO_NUMBER(val, t, cl1Ctrl, GT_U32);
    FO_NUMBER(val, t, enMidFreq, GT_U32);
    FO_NUMBER(val, t, cs1Mid, GT_U32);
    FO_NUMBER(val, t, rs1Mid, GT_U32);
    FO_NUMBER(val, t, rfCtrl, GT_U32);
    FO_NUMBER(val, t, rl1TiaSel, GT_U32);
    FO_NUMBER(val, t, rl1TiaExtra, GT_U32);
    FO_NUMBER(val, t, hpfRSel1st, GT_U32);
    FO_NUMBER(val, t, current1TiaSel, GT_U32);
    FO_NUMBER(val, t, rl2Tune, GT_U32);
    FO_NUMBER(val, t, rl2Sel, GT_U32);
    FO_NUMBER(val, t, rs2Sel, GT_U32);
    FO_NUMBER(val, t, current2Sel, GT_U32);
    FO_NUMBER(val, t, cap2Sel, GT_U32);
    FO_NUMBER(val, t, hpfRsel2nd, GT_U32);
    FO_NUMBER(val, t, selmufi, GT_U32);
    FO_NUMBER(val, t, selmuff, GT_U32);
    FO_NUMBER(val, t, selmupi, GT_U32);
    FO_NUMBER(val, t, selmupf, GT_U32);
    FO_NUMBER(val, t, squelch, GT_U32);
    FO_NUMBER(val, t, align90AnaReg, GT_U32);
    FO_NUMBER(val, t, align90, GT_32);
    FO_NUMBER(val, t, sampler, GT_U32);
    FO_NUMBER(val, t, slewRateCtrl0, GT_U32);
    FO_NUMBER(val, t, slewRateCtrl1, GT_U32);
    FO_ARRAY_START(val, t, dfe);
    {
        int idx;
        for (idx = 0; idx < 40; idx++) {
            FO_ARRAY_NUMBER(val, dfe, idx, GT_32);
        }
    }
    FO_ARRAY_END(val, t, dfe);
    FO_NUMBER(val, t, EO, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_COMPHY_C112G_RX_CONFIG_STC);

void prv_lua_to_c_MV_HWS_AVAGO_RX_OVERRIDE_CONFIG_DATA(
    lua_State *L,
    MV_HWS_AVAGO_RX_OVERRIDE_CONFIG_DATA *val
)
{
    F_NUMBER(val, -1, dcGain, GT_U8);
    F_NUMBER(val, -1, lowFrequency, GT_U8);
    F_NUMBER(val, -1, highFrequency, GT_U8);
    F_NUMBER(val, -1, bandWidth, GT_U8);
    F_NUMBER(val, -1, gainShape1, GT_U8);
    F_NUMBER(val, -1, gainShape2, GT_U8);
    F_NUMBER(val, -1, minLf, GT_U8);
    F_NUMBER(val, -1, maxLf, GT_U8);
    F_NUMBER(val, -1, minHf, GT_U8);
    F_NUMBER(val, -1, maxHf, GT_U8);
    F_NUMBER(val, -1, bfLf, GT_8);
    F_NUMBER(val, -1, bfHf, GT_8);
    F_NUMBER(val, -1, minPre1, GT_8);
    F_NUMBER(val, -1, maxPre1, GT_8);
    F_NUMBER(val, -1, minPre2, GT_8);
    F_NUMBER(val, -1, maxPre2, GT_8);
    F_NUMBER(val, -1, minPost, GT_8);
    F_NUMBER(val, -1, maxPost, GT_8);
    F_NUMBER(val, -1, squelch, GT_U16);
    F_NUMBER(val, -1, shortChannelEn, GT_U8);
    F_NUMBER(val, -1, termination, GT_U8);
    F_NUMBER(val, -1, coldEnvelope, GT_U8);
    F_NUMBER(val, -1, hotEnvelope, GT_U8);
}

void prv_c_to_lua_MV_HWS_AVAGO_RX_OVERRIDE_CONFIG_DATA(
    lua_State *L,
    MV_HWS_AVAGO_RX_OVERRIDE_CONFIG_DATA *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, dcGain, GT_U8);
    FO_NUMBER(val, t, lowFrequency, GT_U8);
    FO_NUMBER(val, t, highFrequency, GT_U8);
    FO_NUMBER(val, t, bandWidth, GT_U8);
    FO_NUMBER(val, t, gainShape1, GT_U8);
    FO_NUMBER(val, t, gainShape2, GT_U8);
    FO_NUMBER(val, t, minLf, GT_U8);
    FO_NUMBER(val, t, maxLf, GT_U8);
    FO_NUMBER(val, t, minHf, GT_U8);
    FO_NUMBER(val, t, maxHf, GT_U8);
    FO_NUMBER(val, t, bfLf, GT_8);
    FO_NUMBER(val, t, bfHf, GT_8);
    FO_NUMBER(val, t, minPre1, GT_8);
    FO_NUMBER(val, t, maxPre1, GT_8);
    FO_NUMBER(val, t, minPre2, GT_8);
    FO_NUMBER(val, t, maxPre2, GT_8);
    FO_NUMBER(val, t, minPost, GT_8);
    FO_NUMBER(val, t, maxPost, GT_8);
    FO_NUMBER(val, t, squelch, GT_U16);
    FO_NUMBER(val, t, shortChannelEn, GT_U8);
    FO_NUMBER(val, t, termination, GT_U8);
    FO_NUMBER(val, t, coldEnvelope, GT_U8);
    FO_NUMBER(val, t, hotEnvelope, GT_U8);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_AVAGO_RX_OVERRIDE_CONFIG_DATA);

void prv_lua_to_c_CPSS_PORT_MAC_MTI_COUNTER_SET_STC(
    lua_State *L,
    CPSS_PORT_MAC_MTI_COUNTER_SET_STC *val
)
{
    F_STRUCT(val, -1, rxCounterStc, CPSS_PORT_MAC_MTI_RX_COUNTER_SET_STC);
    F_STRUCT(val, -1, txCounterStc, CPSS_PORT_MAC_MTI_TX_COUNTER_SET_STC);
}

void prv_c_to_lua_CPSS_PORT_MAC_MTI_COUNTER_SET_STC(
    lua_State *L,
    CPSS_PORT_MAC_MTI_COUNTER_SET_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, rxCounterStc, CPSS_PORT_MAC_MTI_RX_COUNTER_SET_STC);
    FO_STRUCT(val, t, txCounterStc, CPSS_PORT_MAC_MTI_TX_COUNTER_SET_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_MAC_MTI_COUNTER_SET_STC);

void prv_lua_to_c_CPSS_RSFEC_COUNTERS_STC(
    lua_State *L,
    CPSS_RSFEC_COUNTERS_STC *val
)
{
    F_STRUCT(val, -1, correctedFecCodeword, GT_U64);
    F_STRUCT(val, -1, uncorrectedFecCodeword, GT_U64);
    F_STRUCT(val, -1, correctFecCodeword, GT_U64);
    F_ARRAY_START(val, -1, symbolError);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            F_ARRAY_NUMBER(val, symbolError, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, symbolError);
    F_STRUCT(val, -1, total_cw_received, GT_U64);
}

void prv_c_to_lua_CPSS_RSFEC_COUNTERS_STC(
    lua_State *L,
    CPSS_RSFEC_COUNTERS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, correctedFecCodeword, GT_U64);
    FO_STRUCT(val, t, uncorrectedFecCodeword, GT_U64);
    FO_STRUCT(val, t, correctFecCodeword, GT_U64);
    FO_ARRAY_START(val, t, symbolError);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            FO_ARRAY_NUMBER(val, symbolError, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, symbolError);
    FO_STRUCT(val, t, total_cw_received, GT_U64);
    lua_settop(L, t);
}
add_mgm_type(CPSS_RSFEC_COUNTERS_STC);

void prv_lua_to_c_CPSS_PORT_AP_PARAMS_STC(
    lua_State *L,
    CPSS_PORT_AP_PARAMS_STC *val
)
{
    F_BOOL(val, -1, fcPause);
    F_ENUM(val, -1, fcAsmDir, CPSS_PORT_AP_FLOW_CONTROL_ENT);
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

void prv_c_to_lua_CPSS_PORT_AP_PARAMS_STC(
    lua_State *L,
    CPSS_PORT_AP_PARAMS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, fcPause);
    FO_ENUM(val, t, fcAsmDir, CPSS_PORT_AP_FLOW_CONTROL_ENT);
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
add_mgm_type(CPSS_PORT_AP_PARAMS_STC);

add_mgm_enum(CPSS_PORT_MANAGER_CALIBRATION_TYPE_ENT);

void prv_lua_to_c_CPSS_PM_PORT_REG_PARAMS_STC(
    lua_State *L,
    CPSS_PM_PORT_REG_PARAMS_STC *val
)
{
    F_ENUM(val, -1, ifMode, CPSS_PORT_INTERFACE_MODE_ENT);
    F_ENUM(val, -1, speed, CPSS_PORT_SPEED_ENT);
    F_ARRAY_START(val, -1, laneParams);
    {
        int idx;
        for (idx = 0; idx < 8; idx++) {
            F_ARRAY_STRUCT(val, laneParams, idx, CPSS_PM_MODE_LANE_PARAM_STC);
        }
    }
    F_ARRAY_END(val, -1, laneParams);
    F_STRUCT(val, -1, portAttributes, CPSS_PM_PORT_ATTR_STC);
}

void prv_c_to_lua_CPSS_PM_PORT_REG_PARAMS_STC(
    lua_State *L,
    CPSS_PM_PORT_REG_PARAMS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, ifMode, CPSS_PORT_INTERFACE_MODE_ENT);
    FO_ENUM(val, t, speed, CPSS_PORT_SPEED_ENT);
    FO_ARRAY_START(val, t, laneParams);
    {
        int idx;
        for (idx = 0; idx < 8; idx++) {
            FO_ARRAY_STRUCT(val, laneParams, idx, CPSS_PM_MODE_LANE_PARAM_STC);
        }
    }
    FO_ARRAY_END(val, t, laneParams);
    FO_STRUCT(val, t, portAttributes, CPSS_PM_PORT_ATTR_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PM_PORT_REG_PARAMS_STC);

add_mgm_enum(MV_HWS_SERDES_SPEED);

void prv_lua_to_c_MV_HWS_ANP_SD_MUX_STC(
    lua_State *L,
    MV_HWS_ANP_SD_MUX_STC *val
)
{
    F_NUMBER(val, -1, hwSerdesIdx, GT_U32);
    F_NUMBER(val, -1, outSerdesIdx, GT_U32);
}

void prv_c_to_lua_MV_HWS_ANP_SD_MUX_STC(
    lua_State *L,
    MV_HWS_ANP_SD_MUX_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, hwSerdesIdx, GT_U32);
    FO_NUMBER(val, t, outSerdesIdx, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_ANP_SD_MUX_STC);

void prv_lua_to_c_CPSS_PORT_COMPHY_C56G_TX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_COMPHY_C56G_TX_CONFIG_STC *val
)
{
    F_NUMBER(val, -1, pre2, GT_32);
    F_NUMBER(val, -1, pre, GT_32);
    F_NUMBER(val, -1, main, GT_32);
    F_NUMBER(val, -1, post, GT_32);
    F_BOOL(val, -1, usr);
}

void prv_c_to_lua_CPSS_PORT_COMPHY_C56G_TX_CONFIG_STC(
    lua_State *L,
    CPSS_PORT_COMPHY_C56G_TX_CONFIG_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, pre2, GT_32);
    FO_NUMBER(val, t, pre, GT_32);
    FO_NUMBER(val, t, main, GT_32);
    FO_NUMBER(val, t, post, GT_32);
    FO_BOOL(val, t, usr);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_COMPHY_C56G_TX_CONFIG_STC);

void prv_lua_to_c_MV_HWS_REF_CLOCK_CFG(
    lua_State *L,
    MV_HWS_REF_CLOCK_CFG *val
)
{
    F_ENUM(val, -1, refClockFreq, MV_HWS_REF_CLOCK_SUP_VAL);
    F_ENUM(val, -1, refClockSource, MV_HWS_REF_CLOCK_SOURCE);
    F_NUMBER(val, -1, cpllOutFreq, GT_U8);
    F_BOOL(val, -1, isValid);
}

void prv_c_to_lua_MV_HWS_REF_CLOCK_CFG(
    lua_State *L,
    MV_HWS_REF_CLOCK_CFG *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, refClockFreq, MV_HWS_REF_CLOCK_SUP_VAL);
    FO_ENUM(val, t, refClockSource, MV_HWS_REF_CLOCK_SOURCE);
    FO_NUMBER(val, t, cpllOutFreq, GT_U8);
    FO_BOOL(val, t, isValid);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_REF_CLOCK_CFG);

void prv_lua_to_c_CPSS_PORT_MAC_TO_SERDES_STC(
    lua_State *L,
    CPSS_PORT_MAC_TO_SERDES_STC *val
)
{
    F_ARRAY_START(val, -1, serdesLanes);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            F_ARRAY_NUMBER(val, serdesLanes, idx, GT_U8);
        }
    }
    F_ARRAY_END(val, -1, serdesLanes);
}

void prv_c_to_lua_CPSS_PORT_MAC_TO_SERDES_STC(
    lua_State *L,
    CPSS_PORT_MAC_TO_SERDES_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, serdesLanes);
    {
        int idx;
        for (idx = 0; idx < 10; idx++) {
            FO_ARRAY_NUMBER(val, serdesLanes, idx, GT_U8);
        }
    }
    FO_ARRAY_END(val, t, serdesLanes);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_MAC_TO_SERDES_STC);

void prv_lua_to_c_CPSS_INTERFACE_DEV_PORT_STC(
    lua_State *L,
    CPSS_INTERFACE_DEV_PORT_STC *val
)
{
    F_NUMBER(val, -1, hwDevNum, GT_HW_DEV_NUM);
    F_NUMBER(val, -1, portNum, GT_PORT_NUM);
}

void prv_c_to_lua_CPSS_INTERFACE_DEV_PORT_STC(
    lua_State *L,
    CPSS_INTERFACE_DEV_PORT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, hwDevNum, GT_HW_DEV_NUM);
    FO_NUMBER(val, t, portNum, GT_PORT_NUM);
    lua_settop(L, t);
}
add_mgm_type(CPSS_INTERFACE_DEV_PORT_STC);

add_mgm_enum(MV_HWS_AUTO_TUNE_STATUS);

add_mgm_enum(MV_HWS_SERDES_LB);

add_mgm_enum(CPSS_DIRECTION_ENT);

add_mgm_enum(MV_HWS_PORT_PCS_TYPE);

void prv_lua_to_c_CPSS_FCFEC_COUNTERS_STC(
    lua_State *L,
    CPSS_FCFEC_COUNTERS_STC *val
)
{
    F_NUMBER(val, -1, numReceivedBlocks, GT_U32);
    F_NUMBER(val, -1, numReceivedBlocksNoError, GT_U32);
    F_NUMBER(val, -1, numReceivedBlocksCorrectedError, GT_U32);
    F_NUMBER(val, -1, numReceivedBlocksUncorrectedError, GT_U32);
    F_NUMBER(val, -1, numReceivedCorrectedErrorBits, GT_U32);
    F_NUMBER(val, -1, numReceivedUncorrectedErrorBits, GT_U32);
    F_ARRAY_START(val, -1, blocksCorrectedError);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            F_ARRAY_NUMBER(val, blocksCorrectedError, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, blocksCorrectedError);
    F_ARRAY_START(val, -1, blocksUncorrectedError);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            F_ARRAY_NUMBER(val, blocksUncorrectedError, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, blocksUncorrectedError);
}

void prv_c_to_lua_CPSS_FCFEC_COUNTERS_STC(
    lua_State *L,
    CPSS_FCFEC_COUNTERS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, numReceivedBlocks, GT_U32);
    FO_NUMBER(val, t, numReceivedBlocksNoError, GT_U32);
    FO_NUMBER(val, t, numReceivedBlocksCorrectedError, GT_U32);
    FO_NUMBER(val, t, numReceivedBlocksUncorrectedError, GT_U32);
    FO_NUMBER(val, t, numReceivedCorrectedErrorBits, GT_U32);
    FO_NUMBER(val, t, numReceivedUncorrectedErrorBits, GT_U32);
    FO_ARRAY_START(val, t, blocksCorrectedError);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            FO_ARRAY_NUMBER(val, blocksCorrectedError, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, blocksCorrectedError);
    FO_ARRAY_START(val, t, blocksUncorrectedError);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            FO_ARRAY_NUMBER(val, blocksUncorrectedError, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, blocksUncorrectedError);
    lua_settop(L, t);
}
add_mgm_type(CPSS_FCFEC_COUNTERS_STC);

void prv_lua_to_c_CPSS_PORT_MANAGER_AP_PARALLEL_DETECT_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_AP_PARALLEL_DETECT_STC *val
)
{
    F_NUMBER(val, -1, restartMaxNum, GT_U8);
    F_BOOL(val, -1, enable);
    F_NUMBER(val, -1, restartCnt, GT_U8);
    F_ENUM(val, -1, status, CPSS_PORT_MANAGER_PARALLEL_DETECT_STATUS_ENT);
}

void prv_c_to_lua_CPSS_PORT_MANAGER_AP_PARALLEL_DETECT_STC(
    lua_State *L,
    CPSS_PORT_MANAGER_AP_PARALLEL_DETECT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, restartMaxNum, GT_U8);
    FO_BOOL(val, t, enable);
    FO_NUMBER(val, t, restartCnt, GT_U8);
    FO_ENUM(val, t, status, CPSS_PORT_MANAGER_PARALLEL_DETECT_STATUS_ENT);
    lua_settop(L, t);
}
add_mgm_type(CPSS_PORT_MANAGER_AP_PARALLEL_DETECT_STC);

void prv_lua_to_c_MV_HWS_SERDES_RX_CONFIG_DATA_UNT(
    lua_State *L,
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT *val
)
{
    F_UNION_MEMBER_STRUCT(val, -1, rxComphyH, MV_HWS_MAN_TUNE_COMPHY_H_RX_CONFIG_DATA);
    F_UNION_MEMBER_STRUCT(val, -1, rxAvago, MV_HWS_MAN_TUNE_AVAGO_RX_CONFIG_DATA);
    F_UNION_MEMBER_STRUCT(val, -1, rxAvago16nm, MV_HWS_MAN_TUNE_AVAGO_16NM_RX_CONFIG_DATA);
    F_UNION_MEMBER_STRUCT(val, -1, rxComphyC12GP41P2V, MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_RX_CONFIG_DATA);
    F_UNION_MEMBER_STRUCT(val, -1, rxComphyC28GP4, MV_HWS_MAN_TUNE_COMPHY_C28GP4_RX_CONFIG_DATA);
    F_UNION_MEMBER_STRUCT(val, -1, rxComphyC112G, MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA);
    F_UNION_MEMBER_STRUCT(val, -1, rxComphyC56G, MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA);
}

void prv_c_to_lua_MV_HWS_SERDES_RX_CONFIG_DATA_UNT(
    lua_State *L,
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, rxComphyH, MV_HWS_MAN_TUNE_COMPHY_H_RX_CONFIG_DATA);
    FO_STRUCT(val, t, rxAvago, MV_HWS_MAN_TUNE_AVAGO_RX_CONFIG_DATA);
    FO_STRUCT(val, t, rxAvago16nm, MV_HWS_MAN_TUNE_AVAGO_16NM_RX_CONFIG_DATA);
    FO_STRUCT(val, t, rxComphyC12GP41P2V, MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_RX_CONFIG_DATA);
    FO_STRUCT(val, t, rxComphyC28GP4, MV_HWS_MAN_TUNE_COMPHY_C28GP4_RX_CONFIG_DATA);
    FO_STRUCT(val, t, rxComphyC112G, MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA);
    FO_STRUCT(val, t, rxComphyC56G, MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA);
    lua_settop(L, t);
}
add_mgm_type(MV_HWS_SERDES_RX_CONFIG_DATA_UNT);
add_mgm_union(MV_HWS_SERDES_RX_CONFIG_DATA_UNT);

add_mgm_enum(MV_HWS_UNIT);

