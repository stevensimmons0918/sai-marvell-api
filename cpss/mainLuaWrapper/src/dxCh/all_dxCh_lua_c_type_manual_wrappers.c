/*************************************************************************
* all_lua_c_type_wrappers.c
*
* DESCRIPTION:
*       A lua type wrapper
*       It implements support for the following types:
*
*           struct  CPSS_DXCH_NET_DSA_TO_CPU_STC
*           struct  CPSS_DXCH_NET_DSA_FROM_CPU_STC
*           struct  CPSS_DXCH_NET_DSA_TO_ANALYZER_STC
*           struct  CPSS_DXCH_NET_DSA_FORWARD_STC
*           struct  CPSS_DXCH_NET_DSA_PARAMS_STC
*           struct  CPSS_DXCH_NET_TX_PARAMS_STC
*           struct  CPSS_DXCH_TTI_ACTION_STC
*           struct  CPSS_DXCH_PCL_ACTION_STC
*           struct  CPSS_DXCH_PCL_ACTION_REDIRECT_STC
*           struct  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC
*           struct  CPSS_DXCH3_POLICER_METERING_ENTRY_STC
*           struct  CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STC
*           struct  CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC
*           struct  CPSS_DXCH_IP_LTT_ENTRY_STC
*           struct  CPSS_DXCH_PORT_EOM_MATRIX_STC
*           struct  CPSS_DXCH_OAM_SRV_TRANSMIT_PARAMS_STC
*           struct  CPSS_DXCH_PORT_PIZZA_CFG_SLICE_LIST_STC
*           struct  CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC
*           struct  CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC
*           struct  CPSS_DXCH_LOGICAL_INDEX_INFO_STC
*           struct  CPSS_DXCH_EXACT_MATCH_EXPANDED_TTI_ACTION_ORIGIN_STC
*           struct  CPSS_DXCH_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC
*           struct  CPSS_DXCH3_POLICER_METER_TB_SR_TCM_PARAMS_STC
*           struct  CPSS_DXCH3_POLICER_METER_TB_TR_TCM_PARAMS_STC
*           struct  CPSS_DXCH3_POLICER_METER_TB_ENVELOPE_PARAMS_STC
*           struct  CPSS_DXCH_PCL_ACTION_QOS_INGRESS_STC
*           struct  CPSS_DXCH_PCL_ACTION_QOS_EGRESS_STC
*           struct  CPSS_DXCH_PCL_ACTION_VLAN_INGRESS_STC
*           struct  CPSS_DXCH_PCL_ACTION_VLAN_EGRESS_STC
*           struct  CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_EGRESS_SWITCH_IPV6_STC
*           struct  CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC
*           struct  CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC
*           struct  CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC
*           struct  CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC
*           struct  CPSS_DXCH_PORT_SERDES_EYE_RESULT_STC
*           struct  TGF_TRAFFIC_GEN_PARAMS_STC
*           struct  TGF_TRAFFIC_GEN_L3_IP_CUSTOM_TAG_UNT
*           struct  TGF_TRAFFIC_GEN_L4_CUSTOM_TAG_UNT
*           struct  CPSS_DXCH_TTI_DIP_VLAN_STC
*           struct  PRV_CPSS_BOOT_CH_PORT_STATUS_STC
*
*
*       Support for the following unions:
*           union   CPSS_DXCH_PCL_ACTION_QOS_UNT
*           union   CPSS_DXCH_PCL_ACTION_VLAN_UNT
*           union   CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT
*           union   CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT
*           union   CPSS_DXCH_IP_UC_ROUTE_ENTRY_UNT
*           union   CPSS_DXCH_PCL_RULE_FORMAT_UNT
*           union   CPSS_DXCH_TTI_RULE_UNT
*           union   CPSS_DXCH_TUNNEL_START_CONFIG_UNT
*           union   CPSS_DXCH_IP_NAT_ENTRY_UNT
*           union   CPSS_DXCH_EXACT_MATCH_ACTION_UNT
*           union   CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT
*           union   TGF_TRAFFIC_GEN_STREAM_TYPE_ENT
*           union   TGF_TRAFFIC_GEN_PKT_TYPE_ENT
*           union   CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_UNT
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 27 $
**************************************************************************/

#include <extUtils/luaCLI/luaCLIEngine_genWrapper.h>
#include <generic/private/prvWraplGeneral.h>
#include <cpssCommon/wraplCpssDeviceInfo.h>
#include <cpssCommon/wrapCpssDebugInfo.h>
#include <cpss/generic/bridge/cpssGenBrgFdb.h>
#include <cpss/common/config/cpssGenCfg.h>
#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <cpss/generic/trunk/cpssGenTrunkTypes.h>
#include <cpss/generic/diag/cpssDiag.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCn.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDescriptor.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpTypes.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTtiTypes.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpmTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnelTypes.h>
#include <cpss/dxCh/dxChxGen/policer/cpssDxChPolicer.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrity.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpNat.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpmTypes.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOamSrv.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiterProfile.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>
#include <mainLuaWrapper/wraplNetIf.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiter.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/flowManager/cpssDxChFlowManager.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/cpssDxChPacketAnalyzerTypes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/cpssDxChBrgFdbManager.h>
#include <appDemo/sysHwConfig/gtAppDemoPciConfig.h>
#include <extUtils/trafficEngine/tgfTrafficEngine.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/bootChannel/private/prvCpssDxChBootChannelHandler.h>

/* Slice bounds of IP v6 array  */
#define GT_IPV6_BEGIN_OCTET     0
#define GT_IPV6_END_OCTET       3

/***** declarations ********/

use_prv_struct(GT_ETHERADDR)
use_prv_struct(CPSS_PORTS_BMP_STC);
use_prv_struct(GT_IPADDR);
use_prv_struct(GT_IPV6ADDR);
use_prv_struct(CPSS_INTERFACE_INFO_STC);
use_prv_struct(CPSS_DXCH_BRG_VLAN_INFO_STC);
use_prv_struct(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC);
use_prv_struct(CPSS_DXCH_NET_DSA_COMMON_STC);
use_prv_struct(CPSS_DXCH_NET_DSA_TO_CPU_STC);
use_prv_struct(CPSS_DXCH_NET_DSA_FROM_CPU_STC);
use_prv_struct(CPSS_DXCH_NET_DSA_TO_ANALYZER_STC);
use_prv_struct(CPSS_DXCH_NET_DSA_FORWARD_STC);
use_prv_struct(CPSS_DXCH_NET_DSA_PARAMS_STC);
use_prv_struct(CPSS_DXCH_CFG_DEV_INFO_STC);
use_prv_struct(CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC);
use_prv_struct(CPSS_DXCH_PCL_ACTION_REDIRECT_STC);
use_prv_struct(CPSS_DXCH3_POLICER_METERING_ENTRY_STC);
use_prv_struct(CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STC);
use_prv_struct(CPSS_DXCH_IP_UC_ECMP_RPF_FORMAT_STC);
use_prv_struct(CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC);
use_prv_struct(CPSS_DXCH_NET_SDMA_TX_PARAMS_STC);
use_prv_struct(CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC);
use_prv_struct(CPSS_DXCH_PCL_ACTION_MATCH_COUNTER_STC);
use_prv_struct(CPSS_DXCH_PCL_ACTION_MIRROR_STC);
use_prv_struct(CPSS_DXCH_PCL_ACTION_POLICER_STC);
use_prv_struct(CPSS_DXCH_PCL_ACTION_IP_UC_ROUTE_STC);
use_prv_struct(CPSS_DXCH_PCL_ACTION_SOURCE_ID_STC);
use_prv_struct(CPSS_DXCH_PCL_ACTION_SOURCE_PORT_STC);
use_prv_struct(CPSS_DXCH_PCL_ACTION_MIRROR_STC);
use_prv_struct(CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC);
use_prv_struct(CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC);
use_prv_struct(CPSS_DIAG_DATA_INTEGRITY_TCAM_MEMORY_LOCATION_STC);
use_prv_struct(CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC);
use_prv_struct(CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC);
use_prv_struct(CPSS_DXCH_UNIT32_SLICES_PIZZA_ARBITER_STATE_STC);
use_prv_struct(CPSS_DXCH_UNIT_TXQ_SLICES_PIZZA_ARBITER_STATE_STC);
use_prv_struct(CPSS_DXCH_BOBCAT2_SLICES_PIZZA_ARBITER_STATE_STC);
use_prv_struct(CPSS_DXCH_LION2_DEV_PIZZA_ARBITER_STATE_STC);
use_prv_struct(CPSS_DXCH_BOBCAT2_DEV_PIZZA_ARBITER_STATE_STC);
use_prv_struct(CPSS_DXCH_LOGICAL_INDEX_INFO_STC);
use_prv_struct(CPSS_DXCH_LOGICAL_TABLE_INFO_STC);
use_prv_struct(CPSS_DXCH_PCL_ACTION_LATENCY_MONITORING_STC);
use_prv_struct(CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_EGRESS_SWITCH_IPV6_STC);

use_prv_print_struct(CPSS_DXCH_BRG_VLAN_INFO_STC);
use_prv_print_struct(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC);
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC);
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV6_DIP_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC)

use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_COMMON_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV4_PORT_VLAN_QOS_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_ROUTED_ACL_QOS_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC)
use_prv_print_struct(CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC)
use_prv_print_struct(CPSS_DXCH_PCL_ACTION_STC);
use_prv_print_struct(CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC)
use_prv_print_struct(CPSS_DXCH_PCL_ACTION_MIRROR_STC)
use_prv_print_struct(CPSS_DXCH_PCL_ACTION_MATCH_COUNTER_STC)
use_prv_print_struct(CPSS_DXCH_PCL_ACTION_REDIRECT_STC)
use_prv_print_struct(CPSS_DXCH_PCL_ACTION_POLICER_STC)
use_prv_print_struct(CPSS_DXCH_PCL_ACTION_IP_UC_ROUTE_STC)
use_prv_print_struct(CPSS_DXCH_PCL_ACTION_SOURCE_ID_STC)
use_prv_print_struct(CPSS_DXCH_PCL_ACTION_SOURCE_PORT_STC)
use_prv_print_struct(CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STC);
use_prv_print_struct(CPSS_DXCH_IP_UC_ECMP_RPF_FORMAT_STC);
use_prv_print_struct(CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC);
use_prv_print_struct(CPSS_DXCH_IP_LTT_ENTRY_STC);

use_prv_struct(CPSS_DXCH_PCL_ACTION_OAM_STC);
use_prv_struct(CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_STC);
use_prv_print_struct(CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_STC);
use_prv_struct(CPSS_DXCH_FLOW_MANAGER_STC);
use_prv_struct(CPSS_DXCH_FLOW_MANAGER_FW_CFG_STC);
use_prv_struct(CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC);
use_prv_struct(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC);
use_prv_struct(CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC);
use_prv_struct(CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC);

use_prv_struct(CPSS_DXCH_IP_UC_ROUTE_ENTRY_UNT);
use_prv_struct(CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_UNT);
use_prv_struct(CPSS_DXCH_PCL_ACTION_COPY_RESERVED_STC);
use_prv_struct(CPSS_DXCH_PORT_SERDES_EYE_RESULT_STC);
use_prv_struct(CPSS_PP_HA_2_PHASES_INIT_PHASE1_STC);

use_prv_struct(GT_IP_ADDR_TYPE_UNT);
use_prv_struct(TGF_TRAFFIC_GEN_PARAMS_STC);
use_prv_struct(TGF_TRAFFIC_GEN_L4_CUSTOM_TAG_UNT);
use_prv_struct(TGF_TRAFFIC_GEN_L3_IP_CUSTOM_TAG_UNT);

use_prv_struct(PRV_CPSS_BOOT_CH_PORT_STATUS_STC);


/* Externs */
void prv_lua_c_print_CPSS_PORTS_BMP_STC
(
    const GT_CHAR_PTR           name,
    const GT_CHAR_PTR           nameExtension,
    const CPSS_PORTS_BMP_STC    *val
);
void prv_lua_c_print_GT_IPADDR
(
    const GT_CHAR_PTR           name,
    const GT_CHAR_PTR           nameExtension,
    const GT_IPADDR             *val
);
void prv_lua_c_print_GT_IPV6ADDR
(
    const GT_CHAR_PTR                   name,
    const GT_CHAR_PTR                   nameExtension,
    const GT_IPV6ADDR                   *val
);
void prv_lua_c_print_GT_ETHERADDR
(
    const GT_CHAR_PTR           name,
    const GT_CHAR_PTR           nameExtension,
    const GT_ETHERADDR          *val
);
void prv_lua_c_print_CPSS_INTERFACE_INFO_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_INTERFACE_INFO_STC   *val
);

/*************************************************************************
* prv_lua_to_c_mask_CPSS_DXCH_BRG_VLAN_INFO_STC
*
* Description:
*       Convert lua CPSS_DXCH_BRG_VLAN_INFO_STC comparision mask to "C"
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_lua_to_c_mask_CPSS_DXCH_BRG_VLAN_INFO_STC
(
    lua_State                               *L,
    CPSS_DXCH_BRG_VLAN_INFO_STC             *val
)
{
    cpssOsMemSet(val, 0xff, sizeof(*val));
    FM_BOOL(val,    -1, unkSrcAddrSecBreach);
    FM_ENUM(val,    -1, unregNonIpMcastCmd, CPSS_PACKET_CMD_ENT);
    FM_ENUM(val,    -1, unregIpv4McastCmd, CPSS_PACKET_CMD_ENT);
    FM_ENUM(val,    -1, unregIpv6McastCmd, CPSS_PACKET_CMD_ENT);
    FM_ENUM(val,    -1, unkUcastCmd, CPSS_PACKET_CMD_ENT);
    FM_ENUM(val,    -1, unregIpv4BcastCmd, CPSS_PACKET_CMD_ENT);
    FM_ENUM(val,    -1, unregNonIpv4BcastCmd, CPSS_PACKET_CMD_ENT);
    FM_BOOL(val,    -1, ipv4IgmpToCpuEn);
    FM_BOOL(val,    -1, mirrToRxAnalyzerEn);
    FM_BOOL(val,    -1, ipv6IcmpToCpuEn);
    FM_ENUM(val,    -1, ipCtrlToCpuEn, CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT);
    FM_ENUM(val,    -1, ipv4IpmBrgMode, CPSS_BRG_IPM_MODE_ENT);
    FM_ENUM(val,    -1, ipv6IpmBrgMode, CPSS_BRG_IPM_MODE_ENT);
    FM_BOOL(val,    -1, ipv4IpmBrgEn);
    FM_BOOL(val,    -1, ipv6IpmBrgEn);
    FM_ENUM(val,    -1, ipv6SiteIdMode, CPSS_IP_SITE_ID_ENT);
    FM_BOOL(val,    -1, ipv4UcastRouteEn);
    FM_BOOL(val,    -1, ipv4McastRouteEn);
    FM_BOOL(val,    -1, ipv6UcastRouteEn);
    FM_BOOL(val,    -1, ipv6McastRouteEn);
    FM_NUMBER(val,  -1, stgId, GT_U32);
    FM_BOOL(val,    -1, autoLearnDisable);
    FM_BOOL(val,    -1, naMsgToCpuEn);
    FM_NUMBER(val,  -1, mruIdx, GT_U32);
    FM_BOOL(val,    -1, bcastUdpTrapMirrEn);
    FM_NUMBER(val,  -1, vrfId, GT_U32);
    FM_NUMBER(val,  -1, floodVidx, GT_U16);
    FM_ENUM(val,    -1, floodVidxMode, CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ENT);
    FM_ENUM(val,    -1, portIsolationMode,
                                    CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_CMD_ENT);
    FM_BOOL(val,    -1, ucastLocalSwitchingEn);
    FM_BOOL(val,    -1, mcastLocalSwitchingEn);
    FM_NUMBER(val,  -1, mirrToRxAnalyzerIndex, GT_U32);
    FM_BOOL(val,    -1, mirrToTxAnalyzerEn);
    FM_NUMBER(val,  -1, mirrToTxAnalyzerIndex, GT_U32);
    FM_NUMBER(val,  -1, fidValue, GT_U32);
    FM_ENUM(val,    -1, unknownMacSaCmd, CPSS_PACKET_CMD_ENT);
    FM_BOOL(val,    -1, ipv4McBcMirrToAnalyzerEn);
    FM_NUMBER(val,  -1, ipv4McBcMirrToAnalyzerIndex, GT_U32);
    FM_BOOL(val,    -1, ipv6McMirrToAnalyzerEn);
    FM_NUMBER(val,  -1, ipv6McMirrToAnalyzerIndex, GT_U32);
    FM_BOOL(val,    -1, fcoeForwardingEn);
    FM_ENUM(val,    -1, unregIpmEVidxMode, CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_ENT);
    FM_NUMBER(val,  -1, unregIpmEVidx, GT_U32);
    FM_ENUM(val,    -1, fdbLookupKeyMode, CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_ENT);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_BRG_VLAN_INFO_STC
*
* Description:
*       Prints CPSS_DXCH_BRG_VLAN_INFO_STC fields
*
* Parameters:
*       name                          - structure variable name or path
*                                       to structure field
*       nameExtension                 - structure field name (could be
*                                       NULL)
*       val                           - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_BRG_VLAN_INFO_STC
(
    const GT_CHAR_PTR                   name,
    const GT_CHAR_PTR                   nameExtension,
    const CPSS_DXCH_BRG_VLAN_INFO_STC   *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_BOOL(val,    fullName, unkSrcAddrSecBreach);
    P_ENUM(val,    fullName, unregNonIpMcastCmd, CPSS_PACKET_CMD_ENT);
    P_ENUM(val,    fullName, unregIpv4McastCmd, CPSS_PACKET_CMD_ENT);
    P_ENUM(val,    fullName, unregIpv6McastCmd, CPSS_PACKET_CMD_ENT);
    P_ENUM(val,    fullName, unkUcastCmd, CPSS_PACKET_CMD_ENT);
    P_ENUM(val,    fullName, unregIpv4BcastCmd, CPSS_PACKET_CMD_ENT);
    P_ENUM(val,    fullName, unregNonIpv4BcastCmd, CPSS_PACKET_CMD_ENT);
    P_BOOL(val,    fullName, ipv4IgmpToCpuEn);
    P_BOOL(val,    fullName, mirrToRxAnalyzerEn);
    P_BOOL(val,    fullName, ipv6IcmpToCpuEn);
    P_ENUM(val,    fullName, ipCtrlToCpuEn, CPSS_DXCH_BRG_IP_CTRL_TYPE_ENT);
    P_ENUM(val,    fullName, ipv4IpmBrgMode, CPSS_BRG_IPM_MODE_ENT);
    P_ENUM(val,    fullName, ipv6IpmBrgMode, CPSS_BRG_IPM_MODE_ENT);
    P_BOOL(val,    fullName, ipv4IpmBrgEn);
    P_BOOL(val,    fullName, ipv6IpmBrgEn);
    P_ENUM(val,    fullName, ipv6SiteIdMode, CPSS_IP_SITE_ID_ENT);
    P_BOOL(val,    fullName, ipv4UcastRouteEn);
    P_BOOL(val,    fullName, ipv4McastRouteEn);
    P_BOOL(val,    fullName, ipv6UcastRouteEn);
    P_BOOL(val,    fullName, ipv6McastRouteEn);
    P_NUMBER(val,  fullName, stgId, GT_U32);
    P_BOOL(val,    fullName, autoLearnDisable);
    P_BOOL(val,    fullName, naMsgToCpuEn);
    P_NUMBER(val,  fullName, mruIdx, GT_U32);
    P_BOOL(val,    fullName, bcastUdpTrapMirrEn);
    P_NUMBER(val,  fullName, vrfId, GT_U32);
    P_NUMBER(val,  fullName, floodVidx, GT_U16);
    P_ENUM(val,    fullName, floodVidxMode,
                                        CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ENT);
    P_ENUM(val,    fullName, portIsolationMode,
                                    CPSS_DXCH_BRG_VLAN_PORT_ISOLATION_CMD_ENT);
    P_BOOL(val,    fullName, ucastLocalSwitchingEn);
    P_BOOL(val,    fullName, mcastLocalSwitchingEn);
    P_NUMBER(val,  fullName, mirrToRxAnalyzerIndex, GT_U32);
    P_BOOL(val,    fullName, mirrToTxAnalyzerEn);
    P_NUMBER(val,  fullName, mirrToTxAnalyzerIndex, GT_U32);
    P_NUMBER(val,  fullName, fidValue, GT_U32);
    P_ENUM(val,    fullName, unknownMacSaCmd, CPSS_PACKET_CMD_ENT);
    P_BOOL(val,    fullName, ipv4McBcMirrToAnalyzerEn);
    P_NUMBER(val,  fullName, ipv4McBcMirrToAnalyzerIndex, GT_U32);
    P_BOOL(val,    fullName, ipv6McMirrToAnalyzerEn);
    P_NUMBER(val,  fullName, ipv6McMirrToAnalyzerIndex, GT_U32);
    P_BOOL(val,    fullName, fcoeForwardingEn);
    P_ENUM(val,    fullName, unregIpmEVidxMode, CPSS_DXCH_BRG_VLAN_UNREG_IPM_EVIDX_MODE_ENT);
    P_NUMBER(val,  fullName, unregIpmEVidx, GT_U32);
    P_ENUM(val,    fullName, fdbLookupKeyMode, CPSS_DXCH_BRG_FDB_LOOKUP_KEY_MODE_ENT);
}


/*************************************************************************
* prv_lua_to_c_mask_CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC
*
* Description:
*       Convert lua CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC comparision mask
*       to "C"
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_lua_to_c_mask_CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC
(
    lua_State                               *L,
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC    *val
)
{
    cpssOsMemSet(val, 0xff, sizeof(*val));
    FM_ARRAY_START(val, -1, portsCmd);
    {
        int idx;
        for (idx = 0; idx < CPSS_MAX_PORTS_NUM_CNS; idx++)
        {
            FM_ARRAY_ENUM(val, portsCmd, idx,
                          CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT);
        }
    }
    FM_ARRAY_END(val, -1, portsCmd);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC
*
* Description:
*       Prints CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC fields
*
* Parameters:
*       name                                  - structure variable name or
*                                               path to structure field
*       nameExtension                         - structure field name
*                                               ( could be NULL)
*       val                                   - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC
(
    const GT_CHAR_PTR                           name,
    const GT_CHAR_PTR                           nameExtension,
    const CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  *val
)
{
    GT_BOOL taggedPortsPresence = GT_FALSE;

    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_ARRAY_START(val, fullName, portsCmd);
    {
        int idx;
        for (idx = 0; idx < CPSS_MAX_PORTS_NUM_CNS; idx++)
        {
            if (CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E != val->portsCmd[idx])
            {
                taggedPortsPresence = GT_TRUE;
                P_ARRAY_ENUM(val, fullName, portsCmd, idx,
                              CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT);
            }
        }
    }
    P_ARRAY_END(val, fullName, portsCmd);

    P_CONDITIONAL_MESSAGE(GT_FALSE == taggedPortsPresence, "No tagged ports")
}


/*******************************************/
/* CPSS_DXCH_NET_DSA_TO_CPU_STC            */
/*******************************************/
/*************************************************************************
* prv_lua_to_c_CPSS_DXCH_NET_DSA_TO_CPU_STC
*
* Description:
*       Convert to CPSS_DXCH_NET_DSA_TO_CPU_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_CPSS_DXCH_NET_DSA_TO_CPU_STC(
    lua_State *L,
    CPSS_DXCH_NET_DSA_TO_CPU_STC *val
)
{
    F_BOOL(val, -1, isEgressPipe);
    F_BOOL(val, -1, isTagged);
    F_NUMBER(val, -1, hwDevNum, GT_HW_DEV_NUM);
    F_BOOL(val, -1, srcIsTrunk);
    lua_getfield(L, -1, "interface");
    if (lua_istable(L, -1))
    {
        F_NUMBER(&(val->interface), -1, srcTrunkId, GT_TRUNK_ID);
        F_NUMBER(&(val->interface), -1, portNum, GT_PHYSICAL_PORT_NUM);
        F_NUMBER(&(val->interface), -1, ePort, GT_PORT_NUM);

    }
    lua_pop(L, 1);
    F_ENUM(val, -1, cpuCode, CPSS_NET_RX_CPU_CODE_ENT);
    F_BOOL(val, -1, wasTruncated);
    F_NUMBER(val, -1, originByteCount, GT_U32);
    F_NUMBER(val, -1, timestamp, GT_U32);
    F_BOOL(val, -1, packetIsTT);
    lua_getfield(L, -1, "flowIdTtOffset");
    if (lua_istable(L, -1))
    {
        if (GT_TRUE == val->packetIsTT)
        {
            F_NUMBER(&val->flowIdTtOffset,-1, ttOffset, GT_U32);
        }
        else
        {
            F_NUMBER(&val->flowIdTtOffset, -1, flowId, GT_U32);
        }
    }
    lua_pop(L, 1);

    F_NUMBER(val, -1, tag0TpidIndex, GT_U32)
}

/*************************************************************************
* prv_c_to_lua_CPSS_DXCH_NET_DSA_TO_CPU_STC
*
* Description:
*       Convert CPSS_DXCH_NET_DSA_TO_CPU_STC to lua
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_c_to_lua_CPSS_DXCH_NET_DSA_TO_CPU_STC(
    lua_State *L,
    CPSS_DXCH_NET_DSA_TO_CPU_STC *val
)
{
    int t,t1;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, isEgressPipe);
    FO_BOOL(val, t, isTagged);
    FO_NUMBER(val, t, hwDevNum, GT_HW_DEV_NUM);
    FO_BOOL(val, t, srcIsTrunk);

    lua_newtable(L);
    t1 = lua_gettop(L);
    FO_NUMBER(&(val->interface), t1, srcTrunkId, GT_TRUNK_ID);
    FO_NUMBER(&(val->interface), t1, portNum, GT_PHYSICAL_PORT_NUM);
    FO_NUMBER(&(val->interface), t1, ePort, GT_PORT_NUM);
    lua_setfield(L, t, "interface");

    FO_ENUM(val, t, cpuCode, CPSS_NET_RX_CPU_CODE_ENT);
    FO_BOOL(val, t, wasTruncated);
    FO_NUMBER(val, t, originByteCount, GT_U32);
    FO_NUMBER(val, t, timestamp, GT_U32);
    FO_BOOL(val, t,  packetIsTT);
    FO_NUMBER(val, t, tag0TpidIndex, GT_U32);

    /* union flowIdTtOffset */
    lua_newtable(L);
    t1 = lua_gettop(L);
    if (GT_TRUE == val->packetIsTT)
    {
        FO_NUMBER(&(val->flowIdTtOffset), t1, ttOffset, GT_U32);
    }
    else
    {
        FO_NUMBER(&(val->flowIdTtOffset), t1, flowId, GT_U32);
    }
    lua_setfield(L, t, "flowIdTtOffset");

    FO_NUMBER(val, t, tag0TpidIndex, GT_U32)
}

add_mgm_type(CPSS_DXCH_NET_DSA_TO_CPU_STC);


/*******************************************/
/* CPSS_DXCH_NET_DSA_FROM_CPU_STC          */
/*******************************************/
/*************************************************************************
* prv_lua_to_c_CPSS_DXCH_NET_DSA_FROM_CPU_STC
*
* Description:
*       Convert to CPSS_DXCH_NET_DSA_FROM_CPU_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_CPSS_DXCH_NET_DSA_FROM_CPU_STC(
    lua_State *L,
    CPSS_DXCH_NET_DSA_FROM_CPU_STC *val
)
{
    F_STRUCT(val, -1, dstInterface, CPSS_INTERFACE_INFO_STC);
    F_NUMBER(val, -1, tc, GT_U8);
    F_ENUM(val, -1, dp, CPSS_DP_LEVEL_ENT);
    F_BOOL(val, -1, egrFilterEn);
    F_BOOL(val, -1, cascadeControl);
    F_BOOL(val, -1, egrFilterRegistered);
    F_NUMBER(val, -1, srcId, GT_U32);
    F_NUMBER(val, -1, srcHwDev, GT_HW_DEV_NUM);
    lua_getfield(L, -1, "extDestInfo");
    if (lua_type(L, -1) == LUA_TTABLE)
    {
        if (val->dstInterface.type == CPSS_INTERFACE_VIDX_E
            || val->dstInterface.type == CPSS_INTERFACE_VID_E)
        {
            /* multiDest */
            lua_getfield(L, -1, "multiDest");
            if (lua_type(L, -1) == LUA_TTABLE)
            {
                F_BOOL(&(val->extDestInfo.multiDest), -1, excludeInterface);
                F_STRUCT(&(val->extDestInfo.multiDest), -1, excludedInterface, CPSS_INTERFACE_INFO_STC);
                F_BOOL(&(val->extDestInfo.multiDest), -1, mirrorToAllCPUs);
                F_BOOL(&(val->extDestInfo.multiDest), -1, excludedIsPhyPort);
                F_ENUM(&(val->extDestInfo.multiDest), -1, srcIsTagged, CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT);
            }
            lua_pop(L, 1);
        }
        if (val->dstInterface.type == CPSS_INTERFACE_PORT_E)
        {
            /* devPort */
            lua_getfield(L, -1, "devPort");
            if (lua_type(L, -1) == LUA_TTABLE)
            {
                F_BOOL(&(val->extDestInfo.devPort), -1, dstIsTagged);
                F_BOOL(&(val->extDestInfo.devPort), -1, mailBoxToNeighborCPU);
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    F_BOOL(val, -1, isTrgPhyPortValid);
    F_NUMBER(val, -1, dstEport, GT_PORT_NUM);
    F_NUMBER(val, -1, tag0TpidIndex, GT_U32);
}

/*************************************************************************
* prv_c_to_lua_CPSS_DXCH_NET_DSA_FROM_CPU_STC
*
* Description:
*       Convert CPSS_DXCH_NET_DSA_FROM_CPU_STC to lua
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_c_to_lua_CPSS_DXCH_NET_DSA_FROM_CPU_STC(
    lua_State *L,
    CPSS_DXCH_NET_DSA_FROM_CPU_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, dstInterface, CPSS_INTERFACE_INFO_STC);
    FO_NUMBER(val, t, tc, GT_U8);
    FO_ENUM(val, t, dp, CPSS_DP_LEVEL_ENT);
    FO_BOOL(val, t, egrFilterEn);
    FO_BOOL(val, t, cascadeControl);
    FO_BOOL(val, t, egrFilterRegistered);
    FO_NUMBER(val, t, srcId, GT_U32);
    FO_NUMBER(val, t, srcHwDev, GT_HW_DEV_NUM);
    lua_newtable(L);
        if (val->dstInterface.type == CPSS_INTERFACE_VIDX_E
            || val->dstInterface.type == CPSS_INTERFACE_VID_E)
        {
            /* multiDest */
            lua_newtable(L);
                FO_BOOL(&(val->extDestInfo.multiDest), t+2, excludeInterface);
                FO_STRUCT(&(val->extDestInfo.multiDest), t+2, excludedInterface, CPSS_INTERFACE_INFO_STC);
                FO_BOOL(&(val->extDestInfo.multiDest), t+2, mirrorToAllCPUs);
                FO_BOOL(&(val->extDestInfo.multiDest), t+2, excludedIsPhyPort);
                FO_ENUM(&(val->extDestInfo.multiDest), t+2, srcIsTagged, CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT);
            lua_setfield(L, t+1, "multiDest");
        }
        if (val->dstInterface.type == CPSS_INTERFACE_PORT_E)
        {
            /* devPort */
            lua_newtable(L);
                FO_BOOL(&(val->extDestInfo.devPort), t+2, dstIsTagged);
                FO_BOOL(&(val->extDestInfo.devPort), t+2, mailBoxToNeighborCPU);
            lua_setfield(L, t+1, "devPort");
        }
    lua_setfield(L, t, "extDestInfo");
    FO_BOOL(val, t, isTrgPhyPortValid);
    FO_NUMBER(val, t, dstEport, GT_PORT_NUM);
    FO_NUMBER(val, t, tag0TpidIndex, GT_U32);

}

add_mgm_type(CPSS_DXCH_NET_DSA_FROM_CPU_STC);

/*******************************************/
/* CPSS_DXCH_NET_DSA_TO_ANALYZER_STC       */
/*******************************************/
/*************************************************************************
* prv_lua_to_c_CPSS_DXCH_NET_DSA_TO_ANALYZER_STC
*
* Description:
*       Convert to CPSS_DXCH_NET_DSA_TO_ANALYZER_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_CPSS_DXCH_NET_DSA_TO_ANALYZER_STC(
    lua_State *L,
    CPSS_DXCH_NET_DSA_TO_ANALYZER_STC *val
)
{
    F_BOOL(val, -1, rxSniffer);
    F_BOOL(val, -1, isTagged);
    lua_getfield(L, -1, "devPort");
    if (lua_type(L, -1) == LUA_TTABLE)
    {
        F_NUMBER(&(val->devPort), -1, hwDevNum, GT_HW_DEV_NUM);
        F_NUMBER(&(val->devPort), -1, portNum, GT_PHYSICAL_PORT_NUM);
        F_NUMBER(&(val->devPort), -1, ePort, GT_PORT_NUM);
    }
    lua_pop(L, 1);
    F_ENUM(val, -1, analyzerTrgType, CPSS_INTERFACE_TYPE_ENT);

    /* union val->extDestInfo */
    lua_getfield(L, -1, "extDestInfo");
    if (LUA_TTABLE == lua_type(L, -1))
    {
        if (val->analyzerTrgType == CPSS_INTERFACE_PORT_E)
        {
            /* struct extDestInfo.devPort */
            lua_getfield(L, -1, "devPort");
            if (LUA_TTABLE == lua_type(L,-1))
            {
                F_BOOL(&(val->extDestInfo.devPort), -1, analyzerIsTrgPortValid);
                F_NUMBER(&(val->extDestInfo.devPort), -1, analyzerHwTrgDev, GT_HW_DEV_NUM);
                F_NUMBER(&(val->extDestInfo.devPort), -1, analyzerTrgPort, GT_PHYSICAL_PORT_NUM);
                F_NUMBER(&(val->extDestInfo.devPort), -1, analyzerTrgEport, GT_PORT_NUM);
            }
            lua_pop(L, 1);
        }
        else
        {
            /* struct extDestInfo.multiDest */
            lua_getfield(L, -1, "multiDest");
            if (LUA_TTABLE == lua_type(L,-1))
            {
                F_NUMBER(&(val->extDestInfo.multiDest), -1, analyzerEvidx, GT_U16);
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    F_NUMBER(val, -1, tag0TpidIndex, GT_U32);
}

/*************************************************************************
* prv_c_to_lua_CPSS_DXCH_NET_DSA_TO_ANALYZER_STC
*
* Description:
*       Convert CPSS_DXCH_NET_DSA_TO_ANALYZER_STC to lua
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_c_to_lua_CPSS_DXCH_NET_DSA_TO_ANALYZER_STC(
    lua_State *L,
    CPSS_DXCH_NET_DSA_TO_ANALYZER_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, rxSniffer);
    FO_BOOL(val, t, isTagged);
    lua_newtable(L);
        FO_NUMBER(&(val->devPort), t+1, hwDevNum, GT_HW_DEV_NUM);
        FO_NUMBER(&(val->devPort), t+1, portNum, GT_PHYSICAL_PORT_NUM);
        FO_NUMBER(&(val->devPort), t+1, ePort, GT_PORT_NUM);
    lua_setfield(L, t, "devPort");
    FO_ENUM(val, t, analyzerTrgType, CPSS_INTERFACE_TYPE_ENT);

    /* union val->extDestInfo */
    lua_newtable(L);
    if (val->analyzerTrgType == CPSS_INTERFACE_PORT_E)
    {
        /* struct extDestInfo.devPort*/
        lua_newtable(L);

        FO_BOOL(&(val->extDestInfo.devPort), t+2, analyzerIsTrgPortValid);
        FO_NUMBER(&(val->extDestInfo.devPort), t+2, analyzerHwTrgDev, GT_HW_DEV_NUM);
        FO_NUMBER(&(val->extDestInfo.devPort), t+2, analyzerTrgPort, GT_PHYSICAL_PORT_NUM);
        FO_NUMBER(&(val->extDestInfo.devPort), t+2, analyzerTrgEport, GT_PORT_NUM);

        lua_setfield(L, t+1, "devPort");
    }
    else
    {
        lua_newtable(L);
        FO_NUMBER(&(val->extDestInfo.multiDest), t+2, analyzerEvidx, GT_U16);
        lua_setfield(L, t+1, "multiDest");
    }
    lua_setfield(L, t, "extDestInfo");

    FO_NUMBER(val, t, tag0TpidIndex, GT_U32);
}

add_mgm_type(CPSS_DXCH_NET_DSA_TO_ANALYZER_STC);


/*******************************************/
/* CPSS_DXCH_NET_DSA_FORWARD_STC           */
/*******************************************/
/*************************************************************************
* prv_lua_to_c_CPSS_DXCH_NET_DSA_FORWARD_STC
*
* Description:
*       Convert to CPSS_DXCH_NET_DSA_FORWARD_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_CPSS_DXCH_NET_DSA_FORWARD_STC(
    lua_State *L,
    CPSS_DXCH_NET_DSA_FORWARD_STC *val
)
{
    F_ENUM(val, -1, srcIsTagged, CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT);
    F_NUMBER(val, -1, srcHwDev, GT_HW_DEV_NUM);
    F_BOOL(val, -1, srcIsTrunk);
    lua_getfield(L, -1, "source");
    if (lua_type(L, -1) == LUA_TTABLE)
    {
        if (val->srcIsTrunk == GT_TRUE)
        {
            F_NUMBER(&(val->source), -1, trunkId, GT_TRUNK_ID);
        }
        else
        {
            F_NUMBER(&(val->source), -1, portNum, GT_PORT_NUM);
        }
    }
    lua_pop(L, 1);
    F_NUMBER(val, -1, srcId, GT_U32);
    F_BOOL(val, -1, egrFilterRegistered);
    F_BOOL(val, -1, wasRouted);
    F_NUMBER(val, -1, qosProfileIndex, GT_U32);
    F_STRUCT(val, -1, dstInterface, CPSS_INTERFACE_INFO_STC);

    F_BOOL(val, -1, isTrgPhyPortValid);
    F_BOOL(val, -1, skipFdbSaLookup);
    F_NUMBER(val, -1, dstEport, GT_PORT_NUM);
    F_NUMBER(val, -1, tag0TpidIndex, GT_U32);
    F_BOOL(val, -1, origSrcPhyIsTrunk);

    lua_getfield(L, -1, "origSrcPhy"); /* union */
    if (lua_type(L, -1) == LUA_TTABLE)
    {
        if (val->origSrcPhyIsTrunk == GT_TRUE)
        {
            F_NUMBER(&(val->origSrcPhy), -1, trunkId, GT_TRUNK_ID);
        }
        else
        {
            F_NUMBER(&(val->origSrcPhy), -1, portNum, GT_PHYSICAL_PORT_NUM);
        }
    }
    lua_pop(L, 1);

    F_BOOL(val, -1, phySrcMcFilterEnable);
    F_NUMBER(val, -1, hash, GT_U32);
}

/*************************************************************************
* prv_c_to_lua_CPSS_DXCH_NET_DSA_FORWARD_STC
*
* Description:
*       Convert CPSS_DXCH_NET_DSA_FORWARD_STC to lua
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_c_to_lua_CPSS_DXCH_NET_DSA_FORWARD_STC(
    lua_State *L,
    CPSS_DXCH_NET_DSA_FORWARD_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);

    FO_ENUM(val, t, srcIsTagged, CPSS_DXCH_BRG_VLAN_PORT_TAG_CMD_ENT);
    FO_NUMBER(val, t, srcHwDev, GT_HW_DEV_NUM);
    FO_BOOL(val, t, srcIsTrunk);
    lua_newtable(L);
        if (val->srcIsTrunk == GT_TRUE)
        {
            FO_NUMBER(&(val->source), t+1, trunkId, GT_TRUNK_ID);
        }
        else
        {
            FO_NUMBER(&(val->source), t+1, portNum, GT_PORT_NUM);
        }
    lua_setfield(L, t, "source");
    FO_NUMBER(val, t, srcId, GT_U32);
    FO_BOOL(val, t, egrFilterRegistered);
    FO_BOOL(val, t, wasRouted);
    FO_NUMBER(val, t, qosProfileIndex, GT_U32);
    FO_STRUCT(val, t, dstInterface, CPSS_INTERFACE_INFO_STC);

    FO_BOOL(val, t, isTrgPhyPortValid);
    FO_BOOL(val, t, skipFdbSaLookup);
    FO_NUMBER(val, t, dstEport, GT_PORT_NUM);
    FO_NUMBER(val, t, tag0TpidIndex, GT_U32);
    FO_BOOL(val, t, origSrcPhyIsTrunk);

    lua_newtable(L); /* union origSrcPhy*/
    if (val->origSrcPhyIsTrunk == GT_TRUE)
    {
        FO_NUMBER(&val->origSrcPhy, t+1, trunkId, GT_TRUNK_ID);
    }
    else
    {
        FO_NUMBER(&val->origSrcPhy, t+1, portNum, GT_PHYSICAL_PORT_NUM);
    }
    lua_setfield(L, t, "origSrcPhy");
    FO_BOOL(val, t, phySrcMcFilterEnable);
    FO_NUMBER(val, t, hash, GT_U32);
}

add_mgm_type(CPSS_DXCH_NET_DSA_FORWARD_STC);


/*******************************************/
/* CPSS_DXCH_NET_DSA_PARAMS_STC            */
/*******************************************/
/*************************************************************************
* prv_lua_to_c_CPSS_DXCH_NET_DSA_PARAMS_STC
*
* Description:
*       Convert to CPSS_DXCH_NET_DSA_PARAMS_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_CPSS_DXCH_NET_DSA_PARAMS_STC(
    lua_State *L,
    CPSS_DXCH_NET_DSA_PARAMS_STC *val
)
{
    F_STRUCT(val, -1, commonParams, CPSS_DXCH_NET_DSA_COMMON_STC);
    F_ENUM(val, -1, dsaType, CPSS_DXCH_NET_DSA_CMD_ENT);
    lua_getfield(L, -1, "dsaInfo");
    if (lua_type(L, -1) == LUA_TTABLE)
    {
        if (val->dsaType == CPSS_DXCH_NET_DSA_CMD_TO_CPU_E)
        {
            F_STRUCT(&(val->dsaInfo), -1, toCpu, CPSS_DXCH_NET_DSA_TO_CPU_STC);
        }
        if (val->dsaType == CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E)
        {
            F_STRUCT(&(val->dsaInfo), -1, fromCpu, CPSS_DXCH_NET_DSA_FROM_CPU_STC);
        }
        if (val->dsaType == CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E)
        {
            F_STRUCT(&(val->dsaInfo), -1, toAnalyzer, CPSS_DXCH_NET_DSA_TO_ANALYZER_STC);
        }
        if (val->dsaType == CPSS_DXCH_NET_DSA_CMD_FORWARD_E)
        {
            F_STRUCT(&(val->dsaInfo), -1, forward, CPSS_DXCH_NET_DSA_FORWARD_STC);
        }
    }
    lua_pop(L, 1);
}

/*************************************************************************
* prv_c_to_lua_CPSS_DXCH_NET_DSA_PARAMS_STC
*
* Description:
*       Convert CPSS_DXCH_NET_DSA_PARAMS_STC to lua
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_c_to_lua_CPSS_DXCH_NET_DSA_PARAMS_STC(
    lua_State *L,
    CPSS_DXCH_NET_DSA_PARAMS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, commonParams, CPSS_DXCH_NET_DSA_COMMON_STC);
    FO_ENUM(val, t, dsaType, CPSS_DXCH_NET_DSA_CMD_ENT);
    lua_newtable(L);
        if (val->dsaType == CPSS_DXCH_NET_DSA_CMD_TO_CPU_E)
        {
            FO_STRUCT(&(val->dsaInfo), t+1, toCpu, CPSS_DXCH_NET_DSA_TO_CPU_STC);
        }
        if (val->dsaType == CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E)
        {
            FO_STRUCT(&(val->dsaInfo), t+1, fromCpu, CPSS_DXCH_NET_DSA_FROM_CPU_STC);
        }
        if (val->dsaType == CPSS_DXCH_NET_DSA_CMD_TO_ANALYZER_E)
        {
            FO_STRUCT(&(val->dsaInfo), t+1, toAnalyzer, CPSS_DXCH_NET_DSA_TO_ANALYZER_STC);
        }
        if (val->dsaType == CPSS_DXCH_NET_DSA_CMD_FORWARD_E)
        {
            FO_STRUCT(&(val->dsaInfo), t+1, forward, CPSS_DXCH_NET_DSA_FORWARD_STC);
        }
    lua_setfield(L, t, "dsaInfo");
}
add_mgm_type(CPSS_DXCH_NET_DSA_PARAMS_STC);

/*******************************************/
/* CPSS_DXCH_NET_TX_PARAMS_STC             */
/*******************************************/
/*************************************************************************
* prv_lua_to_c_CPSS_DXCH_NET_TX_PARAMS_STC
*
* Description:
*       Convert to CPSS_DXCH_NET_TX_PARAMS_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_CPSS_DXCH_NET_TX_PARAMS_STC(
    lua_State *L,
    CPSS_DXCH_NET_TX_PARAMS_STC *val
)
{
    F_BOOL(val, -1, packetIsTagged);
    F_PTR(val, -1, cookie, GT_PTR);
    F_STRUCT(val, -1, sdmaInfo, CPSS_DXCH_NET_SDMA_TX_PARAMS_STC);
    F_STRUCT(val, -1, dsaParam, CPSS_DXCH_NET_DSA_PARAMS_STC);
}

/*************************************************************************
* prv_c_to_lua_CPSS_DXCH_NET_TX_PARAMS_STC
*
* Description:
*       Convert CPSS_DXCH_NET_TX_PARAMS_STC to lua
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_c_to_lua_CPSS_DXCH_NET_TX_PARAMS_STC(
    lua_State *L,
    CPSS_DXCH_NET_TX_PARAMS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, packetIsTagged);
    FO_PTR(val, t, cookie, GT_PTR);
    FO_STRUCT(val, t, sdmaInfo, CPSS_DXCH_NET_SDMA_TX_PARAMS_STC);
    FO_STRUCT(val, t, dsaParam, CPSS_DXCH_NET_DSA_PARAMS_STC);
}

add_mgm_type(CPSS_DXCH_NET_TX_PARAMS_STC);


void prv_lua_to_c_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC(
    lua_State *L,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC *val
)
{
    F_NUMBER(val, -1, valid, GT_U8);
    F_NUMBER(val, -1, pclId, GT_U32);
    F_NUMBER(val, -1, sourcePort, GT_PHYSICAL_PORT_NUM);
    F_STRUCT(val, -1, portListBmp, CPSS_PORTS_BMP_STC);
    F_NUMBER(val, -1, isTagged, GT_U8);
    F_NUMBER(val, -1, vid, GT_U32);
    F_NUMBER(val, -1, up, GT_U32);
    F_NUMBER(val, -1, isIp, GT_U8);
    F_NUMBER(val, -1, isL2Valid, GT_U8);
    F_NUMBER(val, -1, egrPacketType, GT_U32);
    /* done manually [[[ */
    lua_getfield(L, -1, "toCpu");
    if (lua_istable(L, -1))
    {
        F_NUMBER(&(val->toCpu), -1, cpuCode, GT_U32);
        F_NUMBER(&(val->toCpu), -1, srcTrg, GT_U8);
    }
    lua_pop(L, 1);
    lua_getfield(L, -1, "fromCpu");
    if (lua_istable(L, -1))
    {
        F_NUMBER(&(val->fromCpu), -1, tc, GT_U32);
        F_NUMBER(&(val->fromCpu), -1, dp, GT_U32);
        F_NUMBER(&(val->fromCpu), -1, egrFilterEnable, GT_U8);
    }
    lua_pop(L, 1);
    lua_getfield(L, -1, "toAnalyzer");
    if (lua_istable(L, -1))
    {
        F_NUMBER(&(val->toAnalyzer), -1, rxSniff, GT_U8);
    }
    lua_pop(L, 1);
    lua_getfield(L, -1, "fwdData");
    if (lua_istable(L, -1))
    {
        F_NUMBER(&(val->fwdData), -1, qosProfile, GT_U32);
        F_NUMBER(&(val->fwdData), -1, srcTrunkId, GT_TRUNK_ID);
        F_NUMBER(&(val->fwdData), -1, srcIsTrunk, GT_U8);
        F_NUMBER(&(val->fwdData), -1, isUnknown, GT_U8);
        F_NUMBER(&(val->fwdData), -1, isRouted, GT_U8);
    }
    lua_pop(L, 1);
    /* ]]] done manually */
    F_NUMBER_N(val, -1, srcHwDev, srcDev, GT_HW_DEV_NUM);
    F_NUMBER(val, -1, sourceId, GT_U32);
    F_NUMBER(val, -1, isVidx, GT_U8);
    F_NUMBER(val, -1, tag1Exist, GT_U8);
}


void prv_c_to_lua_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC(
    lua_State *L,
    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC *val
)
{
    int t,t1;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, valid, GT_U8);
    FO_NUMBER(val, t, pclId, GT_U32);
    FO_NUMBER(val, t, sourcePort, GT_PHYSICAL_PORT_NUM);
    FO_STRUCT(val, t, portListBmp, CPSS_PORTS_BMP_STC);
    FO_NUMBER(val, t, isTagged, GT_U8);
    FO_NUMBER(val, t, vid, GT_U32);
    FO_NUMBER(val, t, up, GT_U32);
    FO_NUMBER(val, t, isIp, GT_U8);
    FO_NUMBER(val, t, isL2Valid, GT_U8);
    FO_NUMBER(val, t, egrPacketType, GT_U32);
    /* done manually [[[ */
    lua_newtable(L);
    t1 = lua_gettop(L);
        FO_NUMBER(&(val->toCpu), t1, cpuCode, GT_U32);
        FO_NUMBER(&(val->toCpu), t1, srcTrg, GT_U8);
    lua_setfield(L, t, "toCpu");
    lua_newtable(L);
    t1 = lua_gettop(L);
        FO_NUMBER(&(val->fromCpu), t1, tc, GT_U32);
        FO_NUMBER(&(val->fromCpu), t1, dp, GT_U32);
        FO_NUMBER(&(val->fromCpu), t1, egrFilterEnable, GT_U8);
    lua_setfield(L, t, "fromCpu");
    lua_newtable(L);
    t1 = lua_gettop(L);
        FO_NUMBER(&(val->toAnalyzer), t1, rxSniff, GT_U8);
    lua_setfield(L, t, "toAnalyzer");
    lua_newtable(L);
    t1 = lua_gettop(L);
        FO_NUMBER(&(val->fwdData), t1, qosProfile, GT_U32);
        FO_NUMBER(&(val->fwdData), t1, srcTrunkId, GT_TRUNK_ID);
        FO_NUMBER(&(val->fwdData), t1, srcIsTrunk, GT_U8);
        FO_NUMBER(&(val->fwdData), t1, isUnknown, GT_U8);
        FO_NUMBER(&(val->fwdData), t1, isRouted, GT_U8);
    lua_setfield(L, t, "fwdData");
    /* ]]] done manually */
    FO_NUMBER_N(val, t, srcHwDev, srcDev, GT_HW_DEV_NUM);
    FO_NUMBER(val, t, sourceId, GT_U32);
    FO_NUMBER(val, t, isVidx, GT_U8);
    FO_NUMBER(val, t, tag1Exist, GT_U8);
    lua_settop(L, t);
}
add_mgm_type(CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC);


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC fields
*
* Parameters:
*       name                                         - structure variable
*                                                      name or path to
*                                                      structure field
*       nameExtension                                - structure field
*                                                      name (could be
*                                                      NULL)
*       val                                          - pointer to
*                                                      structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC
(
    const GT_CHAR_PTR                                   name,
    const GT_CHAR_PTR                                   nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC  *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_NUMBER(val,   fullName, valid, GT_U8);
    P_NUMBER(val,   fullName, pclId, GT_U32);
    P_NUMBER(val,   fullName, sourcePort, GT_PHYSICAL_PORT_NUM);
    P_STRUCT(val,   fullName, portListBmp, CPSS_PORTS_BMP_STC);
    P_NUMBER(val,   fullName, isTagged, GT_U8);
    P_NUMBER(val,   fullName, vid, GT_U32);
    P_NUMBER(val,   fullName, up, GT_U32);
    P_NUMBER(val,   fullName, isIp, GT_U8);
    P_NUMBER(val,   fullName, isL2Valid, GT_U8);
    P_NUMBER(val,   fullName, egrPacketType, GT_U32);
    P_NAME_CONVERSIONS_BEGIN(toCpuName, fullName, "toCpu");
        P_NUMBER(&(val->toCpu),         toCpuName, cpuCode, GT_U32);
        P_NUMBER(&(val->toCpu),         toCpuName, srcTrg, GT_U8);
    P_NAME_CONVERSIONS_END(toCpuName, fullName, "toCpu");
    P_NAME_CONVERSIONS_BEGIN(fromCpuName, fullName, "fromCpu");
        P_NUMBER(&(val->fromCpu),       fromCpuName, tc, GT_U32);
        P_NUMBER(&(val->fromCpu),       fromCpuName, dp, GT_U32);
        P_NUMBER(&(val->fromCpu),       fromCpuName, egrFilterEnable, GT_U8);
    P_NAME_CONVERSIONS_END(fromCpuName, fullName, "fromCpu");
    P_NAME_CONVERSIONS_BEGIN(toAnalyzerName, fullName, "toAnalyzer");
        P_NUMBER(&(val->toAnalyzer),    toAnalyzerName, rxSniff, GT_U8);
    P_NAME_CONVERSIONS_END(toAnalyzerName, fullName, "toAnalyzer");
    P_NAME_CONVERSIONS_BEGIN(fwdDataName, fullName, "fwdData");
        P_NUMBER(&(val->fwdData),       fwdDataName, qosProfile, GT_U32);
        P_NUMBER(&(val->fwdData),       fwdDataName, srcTrunkId, GT_TRUNK_ID);
        P_NUMBER(&(val->fwdData),       fwdDataName, srcIsTrunk, GT_U8);
        P_NUMBER(&(val->fwdData),       fwdDataName, isUnknown, GT_U8);
        P_NUMBER(&(val->fwdData),       fwdDataName, isRouted, GT_U8);
    P_NAME_CONVERSIONS_END(fwdDataName, fullName, "fwdData");
    P_NUMBER(val,   fullName, srcHwDev, GT_HW_DEV_NUM);
    P_NUMBER(val,   fullName, sourceId, GT_U32);
    P_NUMBER(val,   fullName, isVidx, GT_U8);
    P_NUMBER(val,   fullName, tag1Exist, GT_U8);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val,   fullName, common,
                                CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC);
    P_STRUCT(val,   fullName, commonExt,
                            CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC);
    P_STRUCT(val,   fullName, sip, GT_IPV6ADDR);
    P_STRUCT(val,   fullName, dip, GT_IPV6ADDR);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val,   fullName, common, CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC);
    P_STRUCT(val,   fullName, commonExt,
                                    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC);


    P_STRUCT(val,   fullName, sip, GT_IPADDR);
    P_STRUCT(val,   fullName, dip, GT_IPADDR);
    P_NUMBER(val,   fullName, etherType, GT_U16);
    P_NUMBER(val,   fullName, l2Encap, GT_U32);
    P_STRUCT(val,   fullName, macDa, GT_ETHERADDR);
    P_STRUCT(val,   fullName, macSa, GT_ETHERADDR);
    P_NUMBER(val,   fullName, ipv4Fragmented, GT_U8);
    P_ARRAY_START(val,  fullName, udb0_5);
    {
        int idx;
        for (idx = 0; idx < 6; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, udb0_5, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, udb0_5);
    P_NUMBER(val,   fullName, tag1Exist, GT_U32);
    P_NUMBER(val,   fullName, vid1, GT_U32);
    P_NUMBER(val,   fullName, up1, GT_U32);
    P_NUMBER(val,   fullName, cfi1, GT_U32);
    P_NUMBER(val,   fullName, vrfId, GT_U32);
    P_NUMBER(val,   fullName, trunkHash, GT_U32);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV6_DIP_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV6_DIP_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV6_DIP_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV6_DIP_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val,   fullName, common, CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC);
    P_STRUCT(val,   fullName, commonStdIp,
                                CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC);


    P_NUMBER(val,   fullName, isArp, GT_U8);
    P_NUMBER(val,   fullName, isIpv6ExtHdrExist, GT_U8);
    P_NUMBER(val,   fullName, isIpv6HopByHop, GT_U8);
    P_STRUCT(val,   fullName, dip, GT_IPV6ADDR);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_NUMBER(val,   fullName, isIpv4, GT_U8);
    P_NUMBER(val,   fullName, ipProtocol, GT_U32);
    P_NUMBER(val,   fullName, dscp, GT_U32);
    P_NUMBER(val,   fullName, isL4Valid, GT_U8);
    P_NUMBER(val,   fullName, l4Byte2, GT_U8);
    P_NUMBER(val,   fullName, l4Byte3, GT_U8);
    P_NUMBER(val,   fullName, ipv4Fragmented, GT_U8);
    P_NUMBER(val,   fullName, egrTcpUdpPortComparator, GT_U32);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val,   fullName, common,
                                CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC);
    P_STRUCT(val,   fullName, commonExt,
                            CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC);
    P_STRUCT(val,   fullName, sip, GT_IPV6ADDR);
    P_NUMBER(val,   fullName, dipBits127to120, GT_U8);
    P_STRUCT(val,   fullName, macDa, GT_ETHERADDR);
    P_STRUCT(val,   fullName, macSa, GT_ETHERADDR);
    P_NUMBER(val,   fullName, vid1, GT_U32);
    P_NUMBER(val,   fullName, up1, GT_U32);
    P_NUMBER(val,   fullName, cfi1, GT_U32);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_NUMBER(val,   fullName, isIpv6, GT_U8);
    P_NUMBER(val,   fullName, ipProtocol, GT_U32);
    P_NUMBER(val,   fullName, dscp, GT_U32);
    P_NUMBER(val,   fullName, isL4Valid, GT_U8);
    P_NUMBER(val,   fullName, l4Byte0, GT_U8);
    P_NUMBER(val,   fullName, l4Byte1, GT_U8);
    P_NUMBER(val,   fullName, l4Byte2, GT_U8);
    P_NUMBER(val,   fullName, l4Byte3, GT_U8);
    P_NUMBER(val,   fullName, l4Byte13, GT_U8);
    P_NUMBER(val,   fullName, ipHeaderOk, GT_U8);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val,   fullName, common, CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC);
    P_STRUCT(val,   fullName, commonExt,
                                    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC);


    P_STRUCT(val,   fullName, sip, GT_IPV6ADDR);
    P_NUMBER(val,   fullName, dipBits127to120, GT_U8);
    P_NUMBER(val,   fullName, isIpv6ExtHdrExist, GT_U8);
    P_NUMBER(val,   fullName, isIpv6HopByHop, GT_U8);
    P_STRUCT(val,   fullName, macDa, GT_ETHERADDR);
    P_STRUCT(val,   fullName, macSa, GT_ETHERADDR);
    P_ARRAY_START(val,  fullName, udb6_11);
    {
        int idx;
        for (idx = 0; idx < 6; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, udb6_11, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, udb6_11);
    P_NUMBER(val, fullName, tag1Exist, GT_U32);
    P_NUMBER(val, fullName, vid1, GT_U32);
    P_NUMBER(val, fullName, up1, GT_U32);
    P_NUMBER(val, fullName, vrfId, GT_U32);
    P_NUMBER(val, fullName, trunkHash, GT_U32);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val, fullName, common, CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC);


    P_NUMBER(val, fullName, isIpv4, GT_U8);
    P_NUMBER(val, fullName, etherType, GT_U16);
    P_NUMBER(val, fullName, isArp, GT_U8);
    P_NUMBER(val, fullName, l2Encap, GT_U32);
    P_STRUCT(val, fullName, macDa, GT_ETHERADDR);
    P_STRUCT(val, fullName, macSa, GT_ETHERADDR);
    P_ARRAY_START(val,  fullName, udb15_17);
    {
        int idx;
        for (idx = 0; idx < 3; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, udb15_17, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, udb15_17);
    P_NUMBER(val,   fullName, vrfId, GT_U32);
    P_NUMBER(val,   fullName, trunkHash, GT_U32);
    P_NUMBER(val,   fullName, tag1Exist, GT_U32);
    P_NUMBER(val,   fullName, vid1, GT_U32);
    P_NUMBER(val,   fullName, up1, GT_U32);
    P_NUMBER(val,   fullName, cfi1, GT_U32);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_NUMBER(val,   fullName, isIpv6, GT_U8);
    P_NUMBER(val,   fullName, ipProtocol, GT_U32);
    P_NUMBER(val,   fullName, dscp, GT_U32);
    P_NUMBER(val,   fullName, isL4Valid, GT_U8);
    P_NUMBER(val,   fullName, l4Byte0, GT_U8);
    P_NUMBER(val,   fullName, l4Byte1, GT_U8);
    P_NUMBER(val,   fullName, l4Byte2, GT_U8);
    P_NUMBER(val,   fullName, l4Byte3, GT_U8);
    P_NUMBER(val,   fullName, l4Byte13, GT_U8);
    P_NUMBER(val,   fullName, egrTcpUdpPortComparator, GT_U32);
}




/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val,   fullName, common, CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC);
    P_STRUCT(val,   fullName, commonStdIp,
                                CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC);


    P_NUMBER(val,   fullName, isArp, GT_U8);
    P_NUMBER(val,   fullName, isBc, GT_U8);
    P_STRUCT(val,   fullName, sip, GT_IPADDR);
    P_STRUCT(val,   fullName, dip, GT_IPADDR);
    P_NUMBER(val,   fullName, l4Byte0, GT_U8);
    P_NUMBER(val,   fullName, l4Byte1, GT_U8);
    P_NUMBER(val,   fullName, l4Byte13, GT_U8);
    P_ARRAY_START(val,  fullName, udb20_22);
    {
        int idx;
        for (idx = 0; idx < 3; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, udb20_22, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, udb20_22);
    P_NUMBER(val,   fullName, vrfId, GT_U32);
    P_NUMBER(val,   fullName, trunkHash, GT_U32);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val,   fullName, common,
                                CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC);
    P_NUMBER(val,   fullName, isIpv4, GT_U8);
    P_NUMBER(val,   fullName, etherType, GT_U16);
    P_NUMBER(val,   fullName, isArp, GT_U8);
    P_NUMBER(val,   fullName, l2Encap, GT_U32);
    P_STRUCT(val,   fullName, macDa, GT_ETHERADDR);
    P_STRUCT(val,   fullName, macSa, GT_ETHERADDR);
    P_NUMBER(val,   fullName, vid1, GT_U32);
    P_NUMBER(val,   fullName, up1, GT_U32);
    P_NUMBER(val,   fullName, cfi1, GT_U32);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val,   fullName, common, CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC);
    P_STRUCT(val,   fullName, commonExt,
                                    CPSS_DXCH_PCL_RULE_FORMAT_COMMON_EXT_STC);


    P_STRUCT(val,   fullName, sip, GT_IPV6ADDR);
    P_STRUCT(val,   fullName, dip, GT_IPV6ADDR);
    P_NUMBER(val,   fullName, isIpv6ExtHdrExist, GT_U8);
    P_NUMBER(val,   fullName, isIpv6HopByHop, GT_U8);
    P_ARRAY_START(val, fullName, udb12_14);
    {
        int idx;
        for (idx = 0; idx < 3; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, udb12_14, idx, GT_U8);
        }
    }
    P_ARRAY_END(val, fullName, udb12_14);
    P_NUMBER(val,   fullName, tag1Exist, GT_U32);
    P_NUMBER(val,   fullName, vid1, GT_U32);
    P_NUMBER(val,   fullName, up1, GT_U32);
    P_NUMBER(val,   fullName, vrfId, GT_U32);
    P_NUMBER(val,   fullName, trunkHash, GT_U32);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC fields
*
* Parameters:
*       name                                  - structure variable name or
*                                               path to structure field
*       nameExtension                         - structure field name
*                                               (could be NULL)
*       val                                   - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC
(
    const GT_CHAR_PTR                           name,
    const GT_CHAR_PTR                           nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC  *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_NUMBER(val,   fullName, pclId, GT_U32);
    P_NUMBER(val,   fullName, macToMe, GT_U8);
    P_NUMBER(val,   fullName, sourcePort, GT_PHYSICAL_PORT_NUM);
    P_STRUCT(val,   fullName, portListBmp, CPSS_PORTS_BMP_STC);
    P_NUMBER(val,   fullName, isTagged, GT_U8);
    P_NUMBER(val,   fullName, vid, GT_U32);
    P_NUMBER(val,   fullName, up, GT_U32);
    P_NUMBER(val,   fullName, qosProfile, GT_U32);
    P_NUMBER(val,   fullName, isIp, GT_U8);
    P_NUMBER(val,   fullName, isL2Valid, GT_U8);
    P_NUMBER(val,   fullName, isUdbValid, GT_U8);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val,   fullName, common, CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STC);
    P_STRUCT(val,   fullName, commonStdIp,
                                CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC);


    P_NUMBER(val,   fullName, isArp, GT_U8);
    P_NUMBER(val,   fullName, isIpv6ExtHdrExist, GT_U8);
    P_NUMBER(val,   fullName, isIpv6HopByHop, GT_U8);
    P_STRUCT(val,   fullName, macDa, GT_ETHERADDR);
    P_STRUCT(val,   fullName, macSa, GT_ETHERADDR);
    P_ARRAY_START(val, fullName, udb18_19);
    {
        int idx;
        for (idx = 0; idx < 2; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, udb18_19, idx, GT_U8);
        }
    }
    P_ARRAY_END(val, fullName, udb18_19);
    P_NUMBER(val,   fullName, vrfId, GT_U32);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_COMMON_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_COMMON_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_COMMON_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_COMMON_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_NUMBER(val,   fullName, pclId, GT_U32);
    P_NUMBER(val,   fullName, macToMe, GT_U8);
    P_NUMBER(val,   fullName, sourcePort, GT_PHYSICAL_PORT_NUM);
    P_STRUCT(val,   fullName, portListBmp, CPSS_PORTS_BMP_STC);
    P_NUMBER(val,   fullName, vid, GT_U32);
    P_NUMBER(val,   fullName, up, GT_U32);
    P_NUMBER(val,   fullName, isIp, GT_U8);
    P_NUMBER(val,   fullName, dscpOrExp, GT_U32);
    P_NUMBER(val,   fullName, isL2Valid, GT_U8);
    P_NUMBER(val,   fullName, isUdbValid, GT_U8);
    P_NUMBER(val,   fullName, pktTagging, GT_U32);
    P_NUMBER(val,   fullName, l3OffsetInvalid, GT_U8);
    P_NUMBER(val,   fullName, l4ProtocolType, GT_U32);
    P_NUMBER(val,   fullName, pktType, GT_U32);
    P_NUMBER(val,   fullName, ipHeaderOk, GT_U8);
    P_NUMBER(val,   fullName, macDaType, GT_U32);
    P_NUMBER(val,   fullName, l4OffsetInvalid, GT_U8);
    P_NUMBER(val,   fullName, l2Encapsulation, GT_U32);
    P_NUMBER(val,   fullName, isIpv6Eh, GT_U8);
    P_NUMBER(val,   fullName, isIpv6HopByHop, GT_U8);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val,   fullName, common,
                                CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC);
    P_STRUCT(val,   fullName, commonStdIp,
                            CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC);
    P_NUMBER(val,   fullName, isArp, GT_U8);
    P_NUMBER(val,   fullName, isBc, GT_U8);
    P_STRUCT(val,   fullName, sip, GT_IPADDR);
    P_STRUCT(val,   fullName, dip, GT_IPADDR);
    P_NUMBER(val,   fullName, l4Byte0, GT_U8);
    P_NUMBER(val,   fullName, l4Byte1, GT_U8);
    P_NUMBER(val,   fullName, l4Byte13, GT_U8);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val,   fullName, common,
                                CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC);
    P_STRUCT(val,   fullName, commonStdIp,
                            CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STD_IP_STC);
    P_NUMBER(val,   fullName, isArp, GT_U8);
    P_ARRAY_START(val,  fullName, dipBits0to31);
    {
        int idx;
        for (idx = 0; idx < 4; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, dipBits0to31, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, dipBits0to31);
    P_NUMBER(val,   fullName, l4Byte13, GT_U8);
    P_STRUCT(val,   fullName, macDa, GT_ETHERADDR);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val,   fullName, commonIngrUdb,
                            CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_COMMON_STC);


    P_NUMBER(val,   fullName, isIpv6, GT_U8);
    P_NUMBER(val,   fullName, ipProtocol, GT_U32);
    P_ARRAY_START(val,  fullName, sipBits31_0);
    {
        int idx;
        for (idx = 0; idx < 4; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, sipBits31_0, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, sipBits31_0);
    P_ARRAY_START(val,  fullName, sipBits79_32orMacSa);
    {
        int idx;
        for (idx = 0; idx < 6; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, sipBits79_32orMacSa, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, sipBits79_32orMacSa);
    P_ARRAY_START(val,  fullName, sipBits127_80orMacDa);
    {
        int idx;
        for (idx = 0; idx < 6; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, sipBits127_80orMacDa, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, sipBits127_80orMacDa);
    P_ARRAY_START(val,  fullName, dipBits127_112);
    {
        int idx;
        for (idx = 0; idx < 2; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, dipBits127_112, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, dipBits127_112);
    P_ARRAY_START(val,  fullName, dipBits31_0);
    {
        int idx;
        for (idx = 0; idx < 4; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, dipBits31_0, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, dipBits31_0);
    P_ARRAY_START(val,  fullName, udb0_15);
    {
        int idx;
        for (idx = 0; idx < 16; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, udb0_15, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, udb0_15);
    P_NUMBER(val,   fullName, tag1Exist, GT_U32);
    P_NUMBER(val,   fullName, vid1, GT_U32);
    P_NUMBER(val,   fullName, up1, GT_U32);
    P_NUMBER(val,   fullName, cfi1, GT_U32);
    P_NUMBER(val,   fullName, vrfId, GT_U32);
    P_NUMBER(val,   fullName, qosProfile, GT_U32);
    P_NUMBER(val,   fullName, trunkHash, GT_U32);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_COMMON_STD_IP_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_NUMBER(val,   fullName, isIpv4, GT_U8);
    P_NUMBER(val,   fullName, ipProtocol, GT_U32);
    P_NUMBER(val,   fullName, dscp, GT_U32);
    P_NUMBER(val,   fullName, isL4Valid, GT_U8);
    P_NUMBER(val,   fullName, l4Byte2, GT_U8);
    P_NUMBER(val,   fullName, l4Byte3, GT_U8);
    P_NUMBER(val,   fullName, ipHeaderOk, GT_U8);
    P_NUMBER(val,   fullName, ipv4Fragmented, GT_U8);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val,   fullName, commonIngrUdb,
                            CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_COMMON_STC);


    P_NUMBER(val,   fullName, isIpv4, GT_U8);
    P_ARRAY_START(val,  fullName, udb0_15);
    {
        int idx;
        for (idx = 0; idx < 16; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, udb0_15, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, udb0_15);
    P_NUMBER(val,   fullName, vrfId, GT_U32);
    P_NUMBER(val,   fullName, qosProfile, GT_U32);
    P_NUMBER(val,   fullName, trunkHash, GT_U32);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val,   fullName, common,
                                CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_STC);
    P_STRUCT(val,   fullName, commonExt,
                            CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_COMMON_EXT_STC);
    P_STRUCT(val,   fullName, sip, GT_IPADDR);
    P_STRUCT(val,   fullName, dip, GT_IPADDR);
    P_NUMBER(val,   fullName, etherType, GT_U16);
    P_NUMBER(val,   fullName, l2Encap, GT_U32);
    P_STRUCT(val,   fullName, macDa, GT_ETHERADDR);
    P_STRUCT(val,   fullName, macSa, GT_ETHERADDR);
    P_NUMBER(val,   fullName, ipv4Fragmented, GT_U8);
    P_NUMBER(val,   fullName, vid1, GT_U32);
    P_NUMBER(val,   fullName, up1, GT_U32);
    P_NUMBER(val,   fullName, cfi1, GT_U32);
    P_NUMBER(val,   fullName, isMpls, GT_U8);
    P_NUMBER(val,   fullName, numOfMplsLabels, GT_U32);
    P_NUMBER(val,   fullName, protocolTypeAfterMpls, GT_U32);
    P_NUMBER(val,   fullName, mplsLabel0, GT_U32);
    P_NUMBER(val,   fullName, mplsExp0, GT_U32);
    P_NUMBER(val,   fullName, mplsLabel1, GT_U32);
    P_NUMBER(val,   fullName, mplsExp1, GT_U32);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC
*       fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val,   fullName, ingressIpCommon,
                                    CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC);
    P_NUMBER(val,   fullName, pktType, GT_U32);
    P_NUMBER(val,   fullName, ipFragmented, GT_U8);
    P_NUMBER(val,   fullName, ipHeaderInfo, GT_U32);
    P_NUMBER(val,   fullName, ipPacketLength, GT_U32);
    P_NUMBER(val,   fullName, ttl, GT_U32);
    P_STRUCT(val,   fullName, sip, GT_IPADDR);
    P_STRUCT(val,   fullName, dip, GT_IPADDR);
    P_NUMBER(val,   fullName, tcpUdpPortComparators, GT_U32);
    P_NUMBER(val,   fullName, udb5, GT_U8);
}

/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV4_PORT_VLAN_QOS_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV4_PORT_VLAN_QOS_STC
*       fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV4_PORT_VLAN_QOS_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV4_PORT_VLAN_QOS_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val,   fullName, ingressIpCommon,
                                    CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC);
    P_NUMBER(val,   fullName, isL2Valid, GT_U8);
    P_NUMBER(val,   fullName, isBc, GT_U8);
    P_NUMBER(val,   fullName, isIp, GT_U8);
    P_NUMBER(val,   fullName, isArp, GT_U8);
    P_NUMBER(val,   fullName, l2Encap, GT_U32);
    P_NUMBER(val,   fullName, etherType, GT_U16);
    P_STRUCT(val,   fullName, macDa, GT_ETHERADDR);
    P_STRUCT(val,   fullName, macSa, GT_ETHERADDR);
    P_NUMBER(val,   fullName, ipFragmented, GT_U8);
    P_NUMBER(val,   fullName, ipHeaderInfo, GT_U32);
    P_STRUCT(val,   fullName, sip, GT_IPADDR);
    P_STRUCT(val,   fullName, dip, GT_IPADDR);
    P_NUMBER(val,   fullName, isUdbValid, GT_U8);
    P_ARRAY_START(val,  fullName, UdbStdIpL2Qos);
    {
        int idx;
        for (idx = 0; idx < 2; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, UdbStdIpL2Qos, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, UdbStdIpL2Qos);
    P_ARRAY_START(val,  fullName, UdbStdIpV4L4);
    {
        int idx;
        for (idx = 0; idx < 3; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, UdbStdIpV4L4, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, UdbStdIpV4L4);
    P_ARRAY_START(val,  fullName, UdbExtIpv6L2);
    {
        int idx;
        for (idx = 0; idx < 4; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, UdbExtIpv6L2, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, UdbExtIpv6L2);
    P_ARRAY_START(val,  fullName, UdbExtIpv6L4);
    {
        int idx;
        for (idx = 0; idx < 3; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, UdbExtIpv6L4, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, UdbExtIpv6L4);
    P_ARRAY_START(val,  fullName, udb5_16);
    {
        int idx;
        for (idx = 0; idx < 12; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, udb5_16, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, udb5_16);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC
*       fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val,   fullName, ingressIpCommon,
                                    CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC);
    P_NUMBER(val,   fullName, isL2Valid, GT_U8);
    P_NUMBER(val,   fullName, isNd, GT_U8);
    P_NUMBER(val,   fullName, isBc, GT_U8);
    P_NUMBER(val,   fullName, isIp, GT_U8);
    P_NUMBER(val,   fullName, l2Encap, GT_U32);
    P_NUMBER(val,   fullName, etherType, GT_U16);
    P_NUMBER(val,   fullName, etherType, GT_U16);
    P_STRUCT(val,   fullName, macDa, GT_ETHERADDR);
    P_STRUCT(val,   fullName, macSa, GT_ETHERADDR);
    P_STRUCT(val,   fullName, sip, GT_IPV6ADDR);
    P_STRUCT(val,   fullName, dip, GT_IPV6ADDR);
    P_NUMBER(val,   fullName, isIpv6ExtHdrExist, GT_U8);
    P_NUMBER(val,   fullName, isIpv6HopByHop, GT_U8);
    P_NUMBER(val,   fullName, ipHeaderOk, GT_U8);
    P_NUMBER(val,   fullName, isUdbValid, GT_U8);
    P_ARRAY_START(val,  fullName, UdbExtNotIpv6);
    {
        int idx;
        for (idx = 0; idx < 4; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, UdbExtNotIpv6, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, UdbExtNotIpv6);
    P_ARRAY_START(val,  fullName, UdbExtIpv6L2);
    {
        int idx;
        for (idx = 0; idx < 5; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, UdbExtIpv6L2, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, UdbExtIpv6L2);
    P_ARRAY_START(val,  fullName, UdbExtIpv6L4);
    {
        int idx;
        for (idx = 0; idx < 3; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, UdbExtIpv6L4, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, UdbExtIpv6L4);
    P_ARRAY_START(val,  fullName, udb0_11);
    {
        int idx;
        for (idx = 0; idx < 12; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, udb0_11, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, udb0_11);
}

/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_ROUTED_ACL_QOS_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_ROUTED_ACL_QOS_STC
*       fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_ROUTED_ACL_QOS_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_ROUTED_ACL_QOS_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val,   fullName, ingressIpCommon,
                                    CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC);
    P_NUMBER(val,   fullName, pktType, GT_U32);
    P_NUMBER(val,   fullName, isNd, GT_U8);
    P_NUMBER(val,   fullName, mplsOuterLabel, GT_U32);
    P_NUMBER(val,   fullName, mplsOuterLabExp, GT_U32);
    P_NUMBER(val,   fullName, mplsOuterLabSBit, GT_U8);
    P_NUMBER(val,   fullName, ipPacketLength, GT_U32);
    P_NUMBER(val,   fullName, ipv6HdrFlowLabel, GT_U32);
    P_NUMBER(val,   fullName, ttl, GT_U32);
    P_STRUCT(val,   fullName, sip, GT_IPV6ADDR);
    P_STRUCT(val,   fullName, dip, GT_IPV6ADDR);
    P_NUMBER(val,   fullName, isIpv6ExtHdrExist, GT_U8);
    P_NUMBER(val,   fullName, isIpv6HopByHop, GT_U8);
    P_NUMBER(val,   fullName, isIpv6LinkLocal, GT_U8);
    P_NUMBER(val,   fullName, isIpv6Mld, GT_U8);
    P_NUMBER(val,   fullName, ipHeaderOk, GT_U8);
    P_NUMBER(val,   fullName, isUdbValid, GT_U8);
    P_ARRAY_START(val,  fullName, UdbStdNotIp);
    {
        int idx;
        for (idx = 0; idx < 2; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, UdbStdNotIp, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, UdbStdNotIp);
    P_ARRAY_START(val,  fullName, UdbStdIpL2Qos);
    {
        int idx;
        for (idx = 0; idx < 3; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, UdbStdIpL2Qos, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, UdbStdIpL2Qos);
    P_ARRAY_START(val,  fullName, UdbStdIpV4L4);
    {
        int idx;
        for (idx = 0; idx < 3; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, UdbStdIpV4L4, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, UdbStdIpV4L4);
    P_ARRAY_START(val,  fullName, UdbExtNotIpv6);
    {
        int idx;
        for (idx = 0; idx < 4; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, UdbExtNotIpv6, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, UdbExtNotIpv6);
    P_ARRAY_START(val,  fullName, UdbExtIpv6L2);
    {
        int idx;
        for (idx = 0; idx < 3; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, UdbExtIpv6L2, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, UdbExtIpv6L2);
    P_ARRAY_START(val,  fullName, UdbExtIpv6L4);
    {
        int idx;
        for (idx = 0; idx < 3; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, UdbExtIpv6L4, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, UdbExtIpv6L4);
    P_ARRAY_START(val,  fullName, udb0_11);
    {
        int idx;
        for (idx = 0; idx < 12; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, udb0_11, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, udb0_11);
    P_ARRAY_START(val,  fullName, udb17_22);
    {
        int idx;
        for (idx = 0; idx < 6; idx++)
        {
            P_ARRAY_NUMBER(val, fullName, udb17_22, idx, GT_U8);
        }
    }
    P_ARRAY_END(val,    fullName, udb17_22);
}

/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_STC
*       fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val,   fullName, egressIpCommon,
                                CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC);
    P_STRUCT(val,   fullName, sip, GT_IPADDR);
    P_STRUCT(val,   fullName, dip, GT_IPADDR);
    P_NUMBER(val,   fullName, ipv4Options, GT_U8);
    P_NUMBER(val,   fullName, isVidx, GT_U8);
}

/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_STC
*       fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val,   fullName, egressIpCommon,
                                CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC);
    P_STRUCT(val,   fullName, sip, GT_IPV6ADDR);
    P_STRUCT(val,   fullName, dip, GT_IPV6ADDR);
    P_NUMBER(val,   fullName, isNd, GT_U8);
    P_NUMBER(val,   fullName, isIpv6ExtHdrExist, GT_U8);
    P_NUMBER(val,   fullName, isIpv6HopByHop, GT_U8);
    P_NUMBER(val,   fullName, isVidx, GT_U8);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC
*       fields
*
* Parameters:
*       name                                      - structure variable
*                                                   name or path to
*                                                   structure field
*       nameExtension                             - structure field name
*                                                   (could be NULL)
*       val                                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_IP_COMMON_STC   *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_NUMBER(val,   fullName, pclId, GT_U32);
    P_NUMBER(val,   fullName, macToMe, GT_U8);
    P_NUMBER(val,   fullName, sourcePort, GT_PHYSICAL_PORT_NUM);
    P_STRUCT(val,   fullName, portListBmp, CPSS_PORTS_BMP_STC);
    P_NUMBER(val,   fullName, isTagged, GT_U8);
    P_NUMBER(val,   fullName, vid, GT_U32);
    P_NUMBER(val,   fullName, up, GT_U32);
    P_NUMBER(val,   fullName, tos, GT_U32);
    P_NUMBER(val,   fullName, ipProtocol, GT_U32);
    P_NUMBER(val,   fullName, isL4Valid, GT_U8);
    P_NUMBER(val,   fullName, l4Byte0, GT_U8);
    P_NUMBER(val,   fullName, l4Byte1, GT_U8);
    P_NUMBER(val,   fullName, l4Byte2, GT_U8);
    P_NUMBER(val,   fullName, l4Byte3, GT_U8);
    P_NUMBER(val,   fullName, l4Byte13, GT_U8);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC
*       fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_IP_COMMON_STC
                                    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_NUMBER(val,   fullName, pclId, GT_U32);
    P_NUMBER(val,   fullName, sourcePort, GT_PHYSICAL_PORT_NUM);
    P_NUMBER(val,   fullName, qosProfile, GT_U32);
    P_NUMBER(val,   fullName, isL2Valid, GT_U8);
    P_NUMBER(val,   fullName, originalVid, GT_U32);
    P_NUMBER(val,   fullName, isSrcTrunk, GT_U8);
    P_NUMBER(val,   fullName, srcDevOrTrunkId, GT_U32);
    P_NUMBER(val,   fullName, isIp, GT_U8);
    P_NUMBER(val,   fullName, isArp, GT_U8);
    P_STRUCT(val,   fullName, macDa, GT_ETHERADDR);
    P_STRUCT(val,   fullName, macSa, GT_ETHERADDR);
    P_NUMBER(val,   fullName, tos, GT_U32);
    P_NUMBER(val,   fullName, ipProtocol, GT_U32);
    P_NUMBER(val,   fullName, ttl, GT_U32);
    P_NUMBER(val,   fullName, isL4Valid, GT_U8);
    P_NUMBER(val,   fullName, l4Byte0, GT_U8);
    P_NUMBER(val,   fullName, l4Byte1, GT_U8);
    P_NUMBER(val,   fullName, l4Byte2, GT_U8);
    P_NUMBER(val,   fullName, l4Byte3, GT_U8);
    P_NUMBER(val,   fullName, l4Byte13, GT_U8);
    P_NUMBER(val,   fullName, tcpUdpPortComparators, GT_U32);
    P_NUMBER(val,   fullName, tc, GT_U32);
    P_NUMBER(val,   fullName, dp, GT_U32);
    P_NUMBER(val,   fullName, egrPacketType, GT_U32);
    P_NUMBER(val,   fullName, srcTrgOrTxMirror, GT_U8);
    P_NUMBER(val,   fullName, assignedUp, GT_U32);
    P_NUMBER(val,   fullName, trgPhysicalPort, GT_PHYSICAL_PORT_NUM);
    P_NUMBER(val,   fullName, rxSniff, GT_U8);
    P_NUMBER(val,   fullName, isRouted, GT_U8);
    P_NUMBER(val,   fullName, isIpv6, GT_U8);
}


void prv_lua_to_c_CPSS_DXCH_PCL_ACTION_STC(
    lua_State *L,
    CPSS_DXCH_PCL_ACTION_STC *val
)
{
    F_ENUM(val, -1, pktCmd, CPSS_PACKET_CMD_ENT);
    F_BOOL(val, -1, actionStop);
    F_BOOL(val, -1, bypassBridge);
    F_BOOL(val, -1, bypassIngressPipe);
    F_BOOL(val, -1, egressPolicy);
    F_STRUCT(val, -1, lookupConfig, CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC);
    F_STRUCT(val, -1, mirror, CPSS_DXCH_PCL_ACTION_MIRROR_STC);
    F_STRUCT(val, -1, matchCounter, CPSS_DXCH_PCL_ACTION_MATCH_COUNTER_STC);
    /* done manually [[[ */
    lua_getfield(L, -1, "qos");
    if (lua_istable(L, -1))
    {
        if (val->egressPolicy == GT_TRUE)
        {
            lua_getfield(L, -1, "egress");
            if (lua_istable(L, -1))
            {
                F_ENUM(&(val->qos.egress), -1, modifyDscp, CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_ENT);
                F_NUMBER(&(val->qos.egress), -1, dscp, GT_U32);
                F_ENUM(&(val->qos.egress), -1, modifyUp, CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT);
                F_NUMBER(&(val->qos.egress), -1, up, GT_U32);
                F_BOOL(&(val->qos.egress), -1, up1ModifyEnable);
                F_NUMBER(&(val->qos.egress), -1, up1, GT_U32);
            }
            lua_pop(L, 1);
        }
        else
        {
            lua_getfield(L, -1, "ingress");
            if (lua_istable(L, -1))
            {
                F_ENUM(&(val->qos.ingress), -1, modifyDscp, CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT);
                F_ENUM(&(val->qos.ingress), -1, modifyUp, CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT);
                F_NUMBER(&(val->qos.ingress), -1, profileIndex, GT_U32);
                F_BOOL(&(val->qos.ingress), -1, profileAssignIndex);
                F_ENUM(&(val->qos.ingress), -1, profilePrecedence, CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT);
                F_ENUM(&(val->qos.ingress), -1, up1Cmd, CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ENT);
                F_NUMBER(&(val->qos.ingress), -1, up1, GT_U32);
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    /* ]]] done manually */
    F_STRUCT(val, -1, redirect, CPSS_DXCH_PCL_ACTION_REDIRECT_STC);
    F_STRUCT(val, -1, policer, CPSS_DXCH_PCL_ACTION_POLICER_STC);
    /* done manually [[[ */
    lua_getfield(L, -1, "vlan");
    if (lua_istable(L, -1))
    {
        if (val->egressPolicy == GT_TRUE)
        {
            lua_getfield(L, -1, "egress");
            if (lua_istable(L, -1))
            {
                F_ENUM(&(val->vlan.egress), -1, vlanCmd, CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT);
                F_NUMBER(&(val->vlan.egress), -1, vlanId, GT_U32);
                F_BOOL(&(val->vlan.egress), -1, vlanId1ModifyEnable);
                F_NUMBER(&(val->vlan.egress), -1, vlanId1, GT_U32);
            }
            lua_pop(L, 1);
        }
        else
        {
            lua_getfield(L, -1, "ingress");
            if (lua_istable(L, -1))
            {
                F_ENUM(&(val->vlan.ingress), -1, modifyVlan, CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT);
                F_BOOL(&(val->vlan.ingress), -1, nestedVlan);
                F_NUMBER(&(val->vlan.ingress), -1, vlanId, GT_U32);
                F_ENUM(&(val->vlan.ingress), -1, precedence, CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT);
                F_ENUM(&(val->vlan.ingress), -1, vlanId1Cmd, CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ENT);
                F_NUMBER(&(val->vlan.ingress), -1, vlanId1, GT_U32);
                F_ENUM(&(val->vlan.ingress), -1, cfi1, CPSS_DXCH_PCL_ACTION_CFI_UPDATE_ENT);
                F_ENUM(&(val->vlan.ingress), -1, cfi0, CPSS_DXCH_PCL_ACTION_CFI_UPDATE_ENT);
                F_BOOL(&(val->vlan.ingress), -1, updateTag1FromUdb);
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
    /* ]]] done manually */
    F_STRUCT(val, -1, ipUcRoute, CPSS_DXCH_PCL_ACTION_IP_UC_ROUTE_STC);
    F_STRUCT(val, -1, sourceId, CPSS_DXCH_PCL_ACTION_SOURCE_ID_STC);
    F_STRUCT(val, -1, oam, CPSS_DXCH_PCL_ACTION_OAM_STC);
    F_NUMBER(val, -1, flowId, GT_U32);
    F_STRUCT(val, -1, sourcePort, CPSS_DXCH_PCL_ACTION_SOURCE_PORT_STC);
    F_BOOL(  val, -1, setMacToMe);
    F_BOOL(  val, -1, channelTypeToOpcodeMapEnable);
    F_NUMBER(val, -1, tmQueueId, GT_U32);
    F_BOOL(  val, -1, unknownSaCommandEnable);
    F_ENUM(  val, -1, unknownSaCommand , CPSS_PACKET_CMD_ENT);
    F_BOOL(  val, -1, terminateCutThroughMode);
    F_STRUCT(val, -1, latencyMonitor, CPSS_DXCH_PCL_ACTION_LATENCY_MONITORING_STC);
    F_BOOL(  val, -1, skipFdbSaLookup);
    F_BOOL(  val, -1, triggerInterrupt);
    F_BOOL(  val, -1, exactMatchOverPclEn);
    if (val->egressPolicy == GT_TRUE)
    {
        F_STRUCT(val, -1, epclPhaInfo, CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_STC);
    }
    F_STRUCT(val, -1, copyReserved, CPSS_DXCH_PCL_ACTION_COPY_RESERVED_STC);
    F_BOOL(  val, -1, ipfixEnable);
    F_BOOL(  val, -1, triggerHashCncClient);
}

void prv_c_to_lua_CPSS_DXCH_PCL_ACTION_STC(
    lua_State *L,
    CPSS_DXCH_PCL_ACTION_STC *val
)
{
    int t,t1,t2;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, pktCmd, CPSS_PACKET_CMD_ENT);
    FO_BOOL(val, t, actionStop);
    FO_BOOL(val, t, bypassBridge);
    FO_BOOL(val, t, bypassIngressPipe);
    FO_BOOL(val, t, egressPolicy);
    FO_STRUCT(val, t, lookupConfig, CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC);
    FO_STRUCT(val, t, mirror, CPSS_DXCH_PCL_ACTION_MIRROR_STC);
    FO_STRUCT(val, t, matchCounter, CPSS_DXCH_PCL_ACTION_MATCH_COUNTER_STC);
    /* done manually [[[ */
    lua_newtable(L);
    t1 = lua_gettop(L);
        if (val->egressPolicy == GT_TRUE)
        {
            lua_newtable(L);
            t2 = lua_gettop(L);
                FO_ENUM(&(val->qos.egress), t2, modifyDscp, CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_ENT);
                FO_NUMBER(&(val->qos.egress), t2, dscp, GT_U32);
                FO_ENUM(&(val->qos.egress), t2, modifyUp, CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT);
                FO_NUMBER(&(val->qos.egress), t2, up, GT_U32);
                FO_BOOL(&(val->qos.egress), t2, up1ModifyEnable);
                FO_NUMBER(&(val->qos.egress), t2, up1, GT_U32);
            lua_setfield(L, t1, "egress");
        }
        else
        {
            lua_newtable(L);
            t2 = lua_gettop(L);
                FO_ENUM(&(val->qos.ingress), t2, modifyDscp, CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT);
                FO_ENUM(&(val->qos.ingress), t2, modifyUp, CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT);
                FO_NUMBER(&(val->qos.ingress), t2, profileIndex, GT_U32);
                FO_BOOL(&(val->qos.ingress), t2, profileAssignIndex);
                FO_ENUM(&(val->qos.ingress), t2, profilePrecedence, CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT);
                FO_ENUM(&(val->qos.ingress), t2, up1Cmd, CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ENT);
                FO_NUMBER(&(val->qos.ingress), t2, up1, GT_U32);
            lua_setfield(L, t1, "ingress");
        }
    lua_setfield(L, t, "qos");
    /* ]]] done manually */
    FO_STRUCT(val, t, redirect, CPSS_DXCH_PCL_ACTION_REDIRECT_STC);
    FO_STRUCT(val, t, policer, CPSS_DXCH_PCL_ACTION_POLICER_STC);
    /* done manually [[[ */
    lua_newtable(L);
    t1 = lua_gettop(L);
        if (val->egressPolicy == GT_TRUE)
        {
            lua_newtable(L);
            t2 = lua_gettop(L);
                FO_ENUM(&(val->vlan.egress), t2, vlanCmd, CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT);
                FO_NUMBER(&(val->vlan.egress), t2, vlanId, GT_U32);
                FO_BOOL(&(val->vlan.egress), t2, vlanId1ModifyEnable);
                FO_NUMBER(&(val->vlan.egress), t2, vlanId1, GT_U32);
            lua_setfield(L, t1, "egress");
        }
        else
        {
            lua_newtable(L);
            t2 = lua_gettop(L);
                FO_ENUM(&(val->vlan.ingress), t2, modifyVlan, CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT);
                FO_BOOL(&(val->vlan.ingress), t2, nestedVlan);
                FO_NUMBER(&(val->vlan.ingress), t2, vlanId, GT_U32);
                FO_ENUM(&(val->vlan.ingress), t2, precedence, CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT);
                FO_ENUM(&(val->vlan.ingress), t2, vlanId1Cmd, CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ENT);
                FO_NUMBER(&(val->vlan.ingress), t2, vlanId1, GT_U32);
                FO_ENUM(&(val->vlan.ingress), t2, cfi1, CPSS_DXCH_PCL_ACTION_CFI_UPDATE_ENT);
                FO_ENUM(&(val->vlan.ingress), t2, cfi0, CPSS_DXCH_PCL_ACTION_CFI_UPDATE_ENT);
                FO_BOOL(&(val->vlan.ingress), t2, updateTag1FromUdb);
            lua_setfield(L, t1, "ingress");
        }
    lua_setfield(L, t, "vlan");
    /* ]]] done manually */
    FO_STRUCT(val, t, ipUcRoute, CPSS_DXCH_PCL_ACTION_IP_UC_ROUTE_STC);
    FO_STRUCT(val, t, sourceId, CPSS_DXCH_PCL_ACTION_SOURCE_ID_STC);
    FO_STRUCT(val, t, oam, CPSS_DXCH_PCL_ACTION_OAM_STC);
    FO_NUMBER(val, t, flowId, GT_U32);
    FO_STRUCT(val, t, sourcePort, CPSS_DXCH_PCL_ACTION_SOURCE_PORT_STC);
    FO_BOOL(  val, t, setMacToMe);
    FO_BOOL(  val, t, channelTypeToOpcodeMapEnable);
    FO_NUMBER(val, t, tmQueueId, GT_U32);
    FO_BOOL(  val, t, unknownSaCommandEnable);
    FO_ENUM(  val, t, unknownSaCommand , CPSS_PACKET_CMD_ENT);
    FO_BOOL(  val, t, terminateCutThroughMode);
    FO_STRUCT(val, t, latencyMonitor, CPSS_DXCH_PCL_ACTION_LATENCY_MONITORING_STC);
    FO_BOOL(  val, t, skipFdbSaLookup);
    FO_BOOL(  val, t, triggerInterrupt);
    FO_BOOL(  val, t, exactMatchOverPclEn);
    if (val->egressPolicy == GT_TRUE)
    {
        FO_STRUCT(val, t, epclPhaInfo, CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_STC);
    }
    FO_STRUCT(val, t, copyReserved, CPSS_DXCH_PCL_ACTION_COPY_RESERVED_STC);
    FO_BOOL(  val, t, ipfixEnable);
    FO_BOOL(  val, t, triggerHashCncClient);
    lua_settop(L, t);
}
add_mgm_type(CPSS_DXCH_PCL_ACTION_STC);


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_ACTION_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_ACTION_STC fields
*
* Parameters:
*       name                      - structure variable name or path to
*                                   structure field
*       nameExtension             - structure field name (could be NULL)
*       val                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_ACTION_STC
(
    const GT_CHAR_PTR               name,
    const GT_CHAR_PTR               nameExtension,
    const CPSS_DXCH_PCL_ACTION_STC  *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_ENUM(val,     fullName, pktCmd, CPSS_PACKET_CMD_ENT);
    P_BOOL(val,     fullName, actionStop);
    P_BOOL(val,     fullName, bypassBridge);
    P_BOOL(val,     fullName, bypassIngressPipe);
    P_BOOL(val,     fullName, egressPolicy);
    P_STRUCT(val,   fullName, lookupConfig,
                                        CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC);
    P_STRUCT(val,   fullName, mirror, CPSS_DXCH_PCL_ACTION_MIRROR_STC);
    P_STRUCT(val,   fullName, matchCounter,
                                        CPSS_DXCH_PCL_ACTION_MATCH_COUNTER_STC);
    if (GT_TRUE == val->egressPolicy)
    {
        P_NAME_CONVERSIONS_BEGIN(qosEgressName, fullName, "qos.egress");
        {
            P_ENUM(&(val->qos.egress),      qosEgressName, modifyDscp,
                                    CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_ENT);
            P_NUMBER(&(val->qos.egress),    qosEgressName, dscp, GT_U32);
            P_ENUM(&(val->qos.egress),      qosEgressName, modifyUp,
                                    CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT);
            P_NUMBER(&(val->qos.egress),    qosEgressName, up, GT_U32);
            P_BOOL(&(val->qos.egress),      qosEgressName, up1ModifyEnable);
            P_NUMBER(&(val->qos.egress),    qosEgressName, up1, GT_U32);
        }
        P_NAME_CONVERSIONS_END(qosEgressName, fullName, "qos.egress");
    }
    else
    {
        P_NAME_CONVERSIONS_BEGIN(qosIngressName, fullName, "qos.ingress");
        {
            P_ENUM(&(val->qos.ingress),     qosIngressName, modifyDscp,
                                        CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT);
            P_ENUM(&(val->qos.ingress),     qosIngressName, modifyUp,
                                        CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT);
            P_NUMBER(&(val->qos.ingress),   qosIngressName, profileIndex,
                                                                        GT_U32);
            P_BOOL(&(val->qos.ingress),     qosIngressName, profileAssignIndex);
            P_ENUM(&(val->qos.ingress),     qosIngressName, profilePrecedence,
                                CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT);
            P_ENUM(&(val->qos.ingress),     qosIngressName, up1Cmd,
                                    CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ENT);
            P_NUMBER(&(val->qos.ingress),   qosIngressName, up1, GT_U32);

        }
        P_NAME_CONVERSIONS_END(qosIngressName, fullName, "qos.ingress");
    }
    P_STRUCT(val,   fullName, redirect, CPSS_DXCH_PCL_ACTION_REDIRECT_STC);
    P_STRUCT(val,   fullName, policer, CPSS_DXCH_PCL_ACTION_POLICER_STC);
    if (val->egressPolicy == GT_TRUE)
    {
        P_NAME_CONVERSIONS_BEGIN(vlanEgressName, fullName, "vlan.egress");
        {
            P_ENUM(&(val->vlan.egress),     vlanEgressName, vlanCmd,
                                    CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_ENT);
            P_NUMBER(&(val->vlan.egress),   vlanEgressName, vlanId, GT_U32);
            P_BOOL(&(val->vlan.egress),     vlanEgressName,
                                                        vlanId1ModifyEnable);
            P_NUMBER(&(val->vlan.egress),   vlanEgressName, vlanId1, GT_U32);
        }
        P_NAME_CONVERSIONS_END(vlanEgressName, fullName, "qos.egress");
    }
    else
    {
        P_NAME_CONVERSIONS_BEGIN(vlanIngressName, fullName, "vlan.ingress");
        {
            P_ENUM(&(val->vlan.ingress),    vlanIngressName, modifyVlan,
                                        CPSS_PACKET_ATTRIBUTE_ASSIGN_CMD_ENT);
            P_BOOL(&(val->vlan.ingress),    vlanIngressName, nestedVlan);
            P_NUMBER(&(val->vlan.ingress),  vlanIngressName, vlanId, GT_U32);
            P_ENUM(&(val->vlan.ingress),    vlanIngressName, precedence,
                                CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT);
            P_ENUM(&(val->vlan.ingress),    vlanIngressName, vlanId1Cmd,
                                CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ENT);
            P_NUMBER(&(val->vlan.ingress),  vlanIngressName, vlanId1, GT_U32);
            P_ENUM(&(val->vlan.ingress),    vlanIngressName, cfi1,
                                CPSS_DXCH_PCL_ACTION_CFI_UPDATE_ENT);
            P_ENUM(&(val->vlan.ingress),    vlanIngressName, cfi0,
                                CPSS_DXCH_PCL_ACTION_CFI_UPDATE_ENT);
            P_BOOL(&(val->vlan.ingress),    vlanIngressName, updateTag1FromUdb);
        }
        P_NAME_CONVERSIONS_END(vlanIngressName, fullName, "vlan.ingress");
    }
    P_STRUCT(val,   fullName, ipUcRoute, CPSS_DXCH_PCL_ACTION_IP_UC_ROUTE_STC);
    P_STRUCT(val,   fullName, sourceId, CPSS_DXCH_PCL_ACTION_SOURCE_ID_STC);
    P_NUMBER(val,   fullName, flowId, GT_U32);
    P_STRUCT(val,   fullName, sourcePort, CPSS_DXCH_PCL_ACTION_SOURCE_PORT_STC);
    P_BOOL(  val,   fullName, setMacToMe);
    P_BOOL(  val,   fullName, channelTypeToOpcodeMapEnable);
    P_NUMBER(val,   fullName, tmQueueId, GT_U32);
    P_BOOL(  val,   fullName, exactMatchOverPclEn);
    P_STRUCT(val,   fullName, epclPhaInfo, CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_STC);
    P_BOOL(  val,   fullName, ipfixEnable);
}
/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_STC fields
*
* Parameters:
*       name                          - structure variable name or path
*                                       to structure field
*       nameExtension                 - structure field name (could be
*                                       NULL)
*       val                           - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_STC
(
    const GT_CHAR_PTR                   name,
    const GT_CHAR_PTR                   nameExtension,
    const CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_STC   *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);

    if(val->phaThreadType == CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_NOT_USED_E)
    {
        P_NUMBER(&(val->phaThreadUnion),  fullName, notNeeded, GT_U32);
    }
    else
    {
        P_NAME_CONVERSIONS_BEGIN(phaThreadUnion, fullName, "phaThreadUnion.epclIoamIngressSwitch");
        {
            P_NUMBER(&(val->phaThreadUnion.epclIoamIngressSwitch),  phaThreadUnion, ioamIncludesBothE2EOptionAndTraceOption, GT_U32);
        }
        P_NAME_CONVERSIONS_END(phaThreadUnion, fullName, "phaThreadUnion.epclIoamIngressSwitch");
    }

}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC fields
*
* Parameters:
*       name                                      - structure variable
*                                                   name or path to
*                                                   structure field
*       nameExtension                             - structure field name
*                                                   (could be NULL)
*       val                                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC
(
    const GT_CHAR_PTR                               name,
    const GT_CHAR_PTR                               nameExtension,
    const CPSS_DXCH_PCL_ACTION_LOOKUP_CONFIG_STC    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_NUMBER(val,   fullName, ipclConfigIndex, GT_U32);
    P_ENUM(val,     fullName, pcl0_1OverrideConfigIndex,
                                        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT);
    P_ENUM(val,     fullName, pcl1OverrideConfigIndex,
                                        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_ENT);
}

/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_ACTION_MIRROR_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_ACTION_MIRROR_STC fields
*
* Parameters:
*       name                              - structure variable name or
*                                           path to structure field
*       nameExtension                     - structure field name (could be
*                                           NULL)
*       val                               - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_ACTION_MIRROR_STC
(
    const GT_CHAR_PTR                       name,
    const GT_CHAR_PTR                       nameExtension,
    const CPSS_DXCH_PCL_ACTION_MIRROR_STC   *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_ENUM(val,     fullName, cpuCode, CPSS_NET_RX_CPU_CODE_ENT);
    P_BOOL(val,     fullName, mirrorToRxAnalyzerPort);
    P_NUMBER(val,   fullName, ingressMirrorToAnalyzerIndex, GT_U32);
    P_BOOL(val,     fullName, mirrorTcpRstAndFinPacketsToCpu);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_ACTION_MATCH_COUNTER_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_ACTION_MATCH_COUNTER_STC fields
*
* Parameters:
*       name                                      - structure variable
*                                                   name or path to
*                                                   structure field
*       nameExtension                             - structure field name
*                                                   (could be NULL)
*       val                                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_ACTION_MATCH_COUNTER_STC
(
    const GT_CHAR_PTR                               name,
    const GT_CHAR_PTR                               nameExtension,
    const CPSS_DXCH_PCL_ACTION_MATCH_COUNTER_STC    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_BOOL(val,     fullName, enableMatchCount);
    P_NUMBER(val,   fullName, matchCounterIndex, GT_U32);
}


void prv_lua_to_c_CPSS_DXCH_PCL_ACTION_REDIRECT_STC
(
    lua_State *L,
    CPSS_DXCH_PCL_ACTION_REDIRECT_STC *val
)
{
    F_ENUM(val, -1, redirectCmd, CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ENT);
    /* done manually [[[ */
    lua_getfield(L, -1, "data");
    if (lua_istable(L, -1))
    {
        switch (val->redirectCmd)
        {
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E:
                lua_getfield(L, -1, "noRedirect");
                lua_pop(L, 1);
                break;
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E:
                lua_getfield(L, -1, "outIf");
                if (lua_istable(L, -1))
                {
                    F_STRUCT(&(val->data.outIf), -1, outInterface, CPSS_INTERFACE_INFO_STC);
                    F_BOOL(&(val->data.outIf), -1, vntL2Echo);
                    F_BOOL(&(val->data.outIf), -1, tunnelStart);
                    F_NUMBER(&(val->data.outIf), -1, tunnelPtr, GT_U32);
                    F_ENUM(&(val->data.outIf), -1, tunnelType, CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ENT);
                    F_NUMBER(&(val->data.outIf), -1, arpPtr, GT_U32);

                    F_BOOL(&(val->data.outIf), -1, modifyMacDa);
                    F_BOOL(&(val->data.outIf), -1, modifyMacSa);
                }
                lua_pop(L, 1);
                break;
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E: /* Fall through */
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ECMP_E:
                F_NUMBER(&val->data,-1,routerLttIndex,GT_U32);
                break;
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E:
                F_NUMBER(&val->data,-1,vrfId,GT_U32);
                break;
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E:
                lua_getfield(L, -1, "modifyMacSa");
                if (lua_istable(L, -1))
                {
                    F_NUMBER(&(val->data.modifyMacSa), -1, arpPtr, GT_U32);
                    F_STRUCT_CUSTOM(&(val->data.modifyMacSa), -1, macSa, GT_ETHERADDR);
                }
                lua_pop(L, 1);
                break;

            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_LOGICAL_PORT_ASSIGN_E:
                lua_getfield(L, -1, "logicalSourceInterface");
                if (lua_istable(L, -1))
                {
                    F_STRUCT(&(val->data.logicalSourceInterface), -1, logicalInterface, CPSS_INTERFACE_INFO_STC);
                    F_BOOL  (&(val->data.logicalSourceInterface), -1, sourceMeshIdSetEnable);
                    F_NUMBER(&(val->data.logicalSourceInterface), -1, sourceMeshId, GT_U32);
                    F_BOOL  (&(val->data.logicalSourceInterface), -1, userTagAcEnable);
                }
                lua_pop(L, 1);
                break;
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_AND_ASSIGN_VRF_ID_E: /* fall through */
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ECMP_AND_ASSIGN_VRF_ID_E:
                lua_getfield(L, -1, "routeAndVrfId");
                if (lua_istable(L, -1))
                {
                    F_NUMBER(&(val->data.routeAndVrfId), -1, routerLttIndex, GT_U32);
                    F_NUMBER(&(val->data.routeAndVrfId), -1, vrfId, GT_U32);
                }
                lua_pop(L, 1);
                break;
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ASSIGN_GENERIC_ACTION_E:
                lua_getfield(L, -1, "assignGenericAction");
                lua_pop(L, 1);
                break;
        }
    }
    lua_pop(L, 1);
    /* ]]] done manually */
}


void prv_c_to_lua_CPSS_DXCH_PCL_ACTION_REDIRECT_STC(
    lua_State *L,
    CPSS_DXCH_PCL_ACTION_REDIRECT_STC *val
)
{
    int t, t1, t2;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, redirectCmd, CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ENT);
    /* done manually [[[ */
    lua_newtable(L);
    t1 = lua_gettop(L);
        switch (val->redirectCmd)
        {
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E:
                lua_newtable(L);
                lua_setfield(L, t1, "noRedirect");
                break;
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E:
                lua_newtable(L);
                t2 = lua_gettop(L);
                    FO_STRUCT(&(val->data.outIf), t2, outInterface, CPSS_INTERFACE_INFO_STC);
                    FO_BOOL(&(val->data.outIf), t2, vntL2Echo);
                    FO_BOOL(&(val->data.outIf), t2, tunnelStart);
                    FO_NUMBER(&(val->data.outIf), t2, tunnelPtr, GT_U32);
                    FO_ENUM(&(val->data.outIf), t2, tunnelType, CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ENT);
                    FO_NUMBER(&(val->data.outIf), t2, arpPtr, GT_U32);

                    FO_BOOL(&(val->data.outIf), t2, modifyMacDa);
                    FO_BOOL(&(val->data.outIf), t2, modifyMacSa);
                lua_setfield(L, t1, "outIf");
                break;
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E: /* Fall through */
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ECMP_E:
                FO_UNION_MEMBER_NUMBER(val->data, t1, routerLttIndex, GT_U32);
                break;
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E:
                FO_UNION_MEMBER_NUMBER(val->data, t1, vrfId, GT_U32);
                break;
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E:
                lua_newtable(L);
                t2 = lua_gettop(L);
                    FO_NUMBER(&(val->data.modifyMacSa), t2, arpPtr, GT_U32);
                    FO_STRUCT(&(val->data.modifyMacSa), t2, macSa, GT_ETHERADDR);
                lua_setfield(L, t1, "modifyMacSa");
                break;
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_LOGICAL_PORT_ASSIGN_E:
                lua_newtable(L);
                t2 = lua_gettop(L);
                    FO_STRUCT(&(val->data.logicalSourceInterface), t2, logicalInterface, CPSS_INTERFACE_INFO_STC);
                    FO_BOOL  (&(val->data.logicalSourceInterface), t2, sourceMeshIdSetEnable);
                    FO_NUMBER(&(val->data.logicalSourceInterface), t2, sourceMeshId, GT_U32);
                    FO_BOOL  (&(val->data.logicalSourceInterface), t2, userTagAcEnable);
                lua_setfield(L, t1, "logicalSourceInterface");
                break;
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_AND_ASSIGN_VRF_ID_E: /* Fall through */
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ECMP_AND_ASSIGN_VRF_ID_E:
                lua_newtable(L);
                t2 = lua_gettop(L);
                    FO_NUMBER(&(val->data.routeAndVrfId), t2, routerLttIndex, GT_U32);
                    FO_NUMBER(&(val->data.routeAndVrfId), t2, vrfId, GT_U32);
                lua_setfield(L, t1, "routeAndVrfId");
                break;
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ASSIGN_GENERIC_ACTION_E:
                lua_newtable(L);
                lua_setfield(L, t1, "assignGenericAction");
                break;
        }
    lua_setfield(L, t, "data");
    /* ]]] done manually */
    lua_settop(L, t);
}
add_mgm_type(CPSS_DXCH_PCL_ACTION_REDIRECT_STC);


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_ACTION_REDIRECT_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_ACTION_REDIRECT_STC fields
*
* Parameters:
*       name                              - structure variable name or
*                                           path to structure field
*       nameExtension                     - structure field name (could be
*                                           NULL)
*       val                               - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_ACTION_REDIRECT_STC
(
    const GT_CHAR_PTR                       name,
    const GT_CHAR_PTR                       nameExtension,
    const CPSS_DXCH_PCL_ACTION_REDIRECT_STC *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_ENUM(val, fullName, redirectCmd, CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ENT);

    switch (val->redirectCmd)
    {
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E:
            break;

        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E:
            P_NAME_CONVERSIONS_BEGIN(dataOutIfName, fullName, "data.outIf");
            {
                P_STRUCT(&(val->data.outIf),    dataOutIfName, outInterface,
                                                    CPSS_INTERFACE_INFO_STC);
                P_BOOL(&(val->data.outIf),      dataOutIfName, vntL2Echo);
                P_BOOL(&(val->data.outIf),      dataOutIfName, tunnelStart);
                P_NUMBER(&(val->data.outIf),    dataOutIfName, tunnelPtr,
                                                                        GT_U32);
                P_ENUM(&(val->data.outIf),      dataOutIfName, tunnelType,
                                CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ENT);
                P_NUMBER(&(val->data.outIf),    dataOutIfName, arpPtr, GT_U32);
                P_BOOL(&(val->data.outIf),      dataOutIfName, modifyMacDa);
                P_BOOL(&(val->data.outIf),      dataOutIfName, modifyMacSa);
            }
            P_NAME_CONVERSIONS_END(dataOutIfName, fullName, "data.outIf");
            break;

        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E:
            P_NAME_CONVERSIONS_BEGIN(dataName, fullName, "data");
            {
                P_NUMBER(&(val->data),  dataName, routerLttIndex, GT_U32);
            }
            P_NAME_CONVERSIONS_END(dataName, fullName, "data");
            break;

        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E:
            P_NAME_CONVERSIONS_BEGIN(dataName, fullName, "data");
            {
                P_NUMBER(&(val->data),  dataName, vrfId, GT_U32);
            }
            P_NAME_CONVERSIONS_END(dataName, fullName, "data");
            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_AND_ASSIGN_VRF_ID_E:
            P_NAME_CONVERSIONS_BEGIN(dataName, fullName, "data");
            {
                P_NUMBER(&(val->data.routeAndVrfId),  dataName, routerLttIndex, GT_U32);
            }
            P_NAME_CONVERSIONS_END(dataName, fullName, "data");

            P_NAME_CONVERSIONS_BEGIN(dataName, fullName, "data");
            {
                P_NUMBER(&(val->data.routeAndVrfId),  dataName, vrfId, GT_U32);
            }
            P_NAME_CONVERSIONS_END(dataName, fullName, "data");
            break;

        default:
            P_MESSAGE("Wrong redirect command");
            break;
    }
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_ACTION_POLICER_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_ACTION_POLICER_STC fields
*
* Parameters:
*       name                              - structure variable name or
*                                           path to structure field
*       nameExtension                     - structure field name (could be
*                                           NULL)
*       val                               - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_ACTION_POLICER_STC
(
    const GT_CHAR_PTR                       name,
    const GT_CHAR_PTR                       nameExtension,
    const CPSS_DXCH_PCL_ACTION_POLICER_STC  *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_ENUM(val,     fullName, policerEnable, CPSS_DXCH_PCL_POLICER_ENABLE_ENT);
    P_NUMBER(val,   fullName, policerId, GT_U32);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_ACTION_IP_UC_ROUTE_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_ACTION_IP_UC_ROUTE_STC fields
*
* Parameters:
*       name                                  - structure variable name or
*                                               path to structure field
*       nameExtension                         - structure field name
*                                               (could be NULL)
*       val                                   - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_ACTION_IP_UC_ROUTE_STC
(
    const GT_CHAR_PTR                           name,
    const GT_CHAR_PTR                           nameExtension,
    const CPSS_DXCH_PCL_ACTION_IP_UC_ROUTE_STC  *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_BOOL(val,     fullName, doIpUcRoute);
    P_NUMBER(val,   fullName, arpDaIndex, GT_U32);
    P_BOOL(val,     fullName, decrementTTL);
    P_BOOL(val,     fullName, bypassTTLCheck);
    P_BOOL(val,     fullName, icmpRedirectCheck);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_ACTION_SOURCE_ID_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_ACTION_SOURCE_ID_STC fields
*
* Parameters:
*       name                                  - structure variable name or
*                                               path to structure field
*       nameExtension                         - structure field name
*                                               (could be NULL)
*       val                                   - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_ACTION_SOURCE_ID_STC
(
    const GT_CHAR_PTR                           name,
    const GT_CHAR_PTR                           nameExtension,
    const CPSS_DXCH_PCL_ACTION_SOURCE_ID_STC    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_BOOL(val,     fullName, assignSourceId);
    P_NUMBER(val,   fullName, sourceIdValue, GT_U32);
}

/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_ACTION_SOURCE_PORT_STC
*
* Description:
*       Prints CPSS_DXCH_PCL_ACTION_SOURCE_PORT_STC fields
*
* Parameters:
*       name                                  - structure variable name or
*                                               path to structure field
*       nameExtension                         - structure field name
*                                               (could be NULL)
*       val                                   - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_ACTION_SOURCE_PORT_STC
(
    const GT_CHAR_PTR                           name,
    const GT_CHAR_PTR                           nameExtension,
    const CPSS_DXCH_PCL_ACTION_SOURCE_PORT_STC  *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_BOOL(val,     fullName, assignSourcePortEnable);
    P_NUMBER(val,   fullName, sourcePortValue, GT_U32);
}


void prv_lua_to_c_CPSS_DXCH3_POLICER_METERING_ENTRY_STC(
    lua_State *L,
    CPSS_DXCH3_POLICER_METERING_ENTRY_STC *val
)
{
    F_NUMBER(val, -1, countingEntryIndex, GT_U32);
    F_ENUM(val, -1, mngCounterSet, CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT);
    F_ENUM(val, -1, meterColorMode, CPSS_POLICER_COLOR_MODE_ENT);
    F_ENUM(val, -1, meterMode, CPSS_DXCH3_POLICER_METER_MODE_ENT);
    /* done manually [[[ */
    lua_getfield(L, -1, "tokenBucketParams");
    if (lua_istable(L, -1))
    {
        switch (val->meterMode)
        {
            case CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E:
                lua_getfield(L, -1, "srTcmParams");
                if (lua_istable(L, -1))
                {
                    F_NUMBER(&(val->tokenBucketParams.srTcmParams), -1, cir, GT_U32);
                    F_NUMBER(&(val->tokenBucketParams.srTcmParams), -1, cbs, GT_U32);
                    F_NUMBER(&(val->tokenBucketParams.srTcmParams), -1, ebs, GT_U32);
                }
                lua_pop(L, 1);
                break;
            case CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E:
            case CPSS_DXCH3_POLICER_METER_MODE_MEF0_E:
            case CPSS_DXCH3_POLICER_METER_MODE_MEF1_E:
                lua_getfield(L, -1, "trTcmParams");
                if (lua_istable(L, -1))
                {
                    F_NUMBER(&(val->tokenBucketParams.trTcmParams), -1, cir, GT_U32);
                    F_NUMBER(&(val->tokenBucketParams.trTcmParams), -1, cbs, GT_U32);
                    F_NUMBER(&(val->tokenBucketParams.trTcmParams), -1, pir, GT_U32);
                    F_NUMBER(&(val->tokenBucketParams.trTcmParams), -1, pbs, GT_U32);
                }
                lua_pop(L, 1);
                break;
            case CPSS_DXCH3_POLICER_METER_MODE_START_OF_ENVELOPE_E:
            case CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E:
                lua_getfield(L, -1, "envelope");
                if (lua_istable(L, -1))
                {
                    F_NUMBER(&(val->tokenBucketParams.envelope), -1, cir, GT_U32);
                    F_NUMBER(&(val->tokenBucketParams.envelope), -1, cbs, GT_U32);
                    F_NUMBER(&(val->tokenBucketParams.envelope), -1, eir, GT_U32);
                    F_NUMBER(&(val->tokenBucketParams.envelope), -1, ebs, GT_U32);
                    F_NUMBER(&(val->tokenBucketParams.envelope), -1, maxCir, GT_U32);
                    F_NUMBER(&(val->tokenBucketParams.envelope), -1, maxEir, GT_U32);
                }
                lua_pop(L, 1);
                break;
        }
    }
    lua_pop(L, 1);
    /* ]]] done manually */
    F_ENUM(val, -1, modifyUp, CPSS_DXCH_POLICER_MODIFY_UP_ENT);
    F_ENUM(val, -1, modifyDscp, CPSS_DXCH_POLICER_MODIFY_DSCP_ENT);
    F_ENUM(val, -1, modifyDp,  CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT);
    F_ENUM(val, -1, modifyTc,  CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT);
    F_ENUM(val, -1, modifyExp, CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT);
    F_ENUM(val, -1, greenPcktCmd, CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT);
    F_ENUM(val, -1, yellowPcktCmd, CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT);
    F_ENUM(val, -1, redPcktCmd, CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT);
    F_NUMBER(val, -1, qosProfile, GT_U32);
    F_ENUM(val, -1, remarkMode, CPSS_DXCH_POLICER_REMARK_MODE_ENT);
    F_ENUM(val, -1, byteOrPacketCountingMode, CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT);
    F_ENUM(val, -1, packetSizeMode, CPSS_POLICER_PACKET_SIZE_MODE_ENT);
    F_ENUM(val, -1, tunnelTerminationPacketSizeMode, CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT);
    F_ENUM(val, -1, dsaTagCountingMode, CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_ENT);
    F_ENUM(val, -1, timeStampCountingMode, CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_ENT);
    F_BOOL(val, -1, yellowEcnMarkingEnable);
    F_BOOL(val, -1, couplingFlag);
    F_NUMBER(val, -1, maxRateIndex, GT_U32);
    F_BOOL(val, -1, greenMirrorToAnalyzerEnable);
    F_BOOL(val, -1, yellowMirrorToAnalyzerEnable);
    F_BOOL(val, -1, redMirrorToAnalyzerEnable);
}

void prv_c_to_lua_CPSS_DXCH3_POLICER_METERING_ENTRY_STC(
    lua_State *L,
    CPSS_DXCH3_POLICER_METERING_ENTRY_STC *val
)
{
    int t, t1, t2;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, countingEntryIndex, GT_U32);
    FO_ENUM(val, t, mngCounterSet, CPSS_DXCH3_POLICER_MNG_CNTR_SET_ENT);
    FO_ENUM(val, t, meterColorMode, CPSS_POLICER_COLOR_MODE_ENT);
    FO_ENUM(val, t, meterMode, CPSS_DXCH3_POLICER_METER_MODE_ENT);
    /* done manually [[[ */
    lua_newtable(L);
    t1 = lua_gettop(L);
        switch (val->meterMode)
        {
            case CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E:
                lua_newtable(L);
                t2 = lua_gettop(L);
                    FO_NUMBER(&(val->tokenBucketParams.srTcmParams), t2, cir, GT_U32);
                    FO_NUMBER(&(val->tokenBucketParams.srTcmParams), t2, cbs, GT_U32);
                    FO_NUMBER(&(val->tokenBucketParams.srTcmParams), t2, ebs, GT_U32);
                lua_setfield(L, t1, "srTcmParams");
                break;
            case CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E:
            case CPSS_DXCH3_POLICER_METER_MODE_MEF0_E:
            case CPSS_DXCH3_POLICER_METER_MODE_MEF1_E:
               lua_newtable(L);
                t2 = lua_gettop(L);
                    FO_NUMBER(&(val->tokenBucketParams.trTcmParams), t2, cir, GT_U32);
                    FO_NUMBER(&(val->tokenBucketParams.trTcmParams), t2, cbs, GT_U32);
                    FO_NUMBER(&(val->tokenBucketParams.trTcmParams), t2, pir, GT_U32);
                    FO_NUMBER(&(val->tokenBucketParams.trTcmParams), t2, pbs, GT_U32);
                lua_setfield(L, t1, "trTcmParams");
                break;
            case CPSS_DXCH3_POLICER_METER_MODE_START_OF_ENVELOPE_E:
            case CPSS_DXCH3_POLICER_METER_MODE_NOT_START_OF_ENVELOPE_E:
                lua_newtable(L);
                 t2 = lua_gettop(L);
                     FO_NUMBER(&(val->tokenBucketParams.envelope), t2, cir, GT_U32);
                     FO_NUMBER(&(val->tokenBucketParams.envelope), t2, cbs, GT_U32);
                     FO_NUMBER(&(val->tokenBucketParams.envelope), t2, eir, GT_U32);
                     FO_NUMBER(&(val->tokenBucketParams.envelope), t2, ebs, GT_U32);
                     FO_NUMBER(&(val->tokenBucketParams.envelope), t2, maxCir, GT_U32);
                     FO_NUMBER(&(val->tokenBucketParams.envelope), t2, maxEir, GT_U32);
                 lua_setfield(L, t1, "trTcmParams");
                 break;
        }
    lua_setfield(L, t, "tokenBucketParams");
    /* ]]] done manually */

    FO_ENUM(val, t, modifyUp, CPSS_DXCH_POLICER_MODIFY_UP_ENT);
    FO_ENUM(val, t, modifyDscp, CPSS_DXCH_POLICER_MODIFY_DSCP_ENT);
    FO_ENUM(val, t, modifyDp,  CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT);
    FO_ENUM(val, t, modifyTc,  CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT);
    FO_ENUM(val, t, modifyExp, CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT);
    FO_ENUM(val, t, greenPcktCmd, CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT);
    FO_ENUM(val, t, yellowPcktCmd, CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT);
    FO_ENUM(val, t, redPcktCmd, CPSS_DXCH3_POLICER_NON_CONFORM_CMD_ENT);
    FO_NUMBER(val, t, qosProfile, GT_U32);
    FO_ENUM(val, t, remarkMode, CPSS_DXCH_POLICER_REMARK_MODE_ENT);
    FO_ENUM(val, t, byteOrPacketCountingMode, CPSS_DXCH3_POLICER_METER_RESOLUTION_ENT);
    FO_ENUM(val, t, packetSizeMode, CPSS_POLICER_PACKET_SIZE_MODE_ENT);
    FO_ENUM(val, t, tunnelTerminationPacketSizeMode, CPSS_DXCH3_POLICER_TT_PACKET_SIZE_MODE_ENT);
    FO_ENUM(val, t, dsaTagCountingMode, CPSS_DXCH3_POLICER_DSA_TAG_COUNTING_MODE_ENT);
    FO_ENUM(val, t, timeStampCountingMode, CPSS_DXCH3_POLICER_TS_TAG_COUNTING_MODE_ENT);
    FO_BOOL(val, t, yellowEcnMarkingEnable);
    FO_BOOL(val, t, couplingFlag);
    FO_NUMBER(val, t, maxRateIndex, GT_U32);
    FO_BOOL(val, t, greenMirrorToAnalyzerEnable);
    FO_BOOL(val, t, yellowMirrorToAnalyzerEnable);
    FO_BOOL(val, t, redMirrorToAnalyzerEnable);
    lua_settop(L, t);
}
add_mgm_type(CPSS_DXCH3_POLICER_METERING_ENTRY_STC);


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STC
*
* Description:
*       Prints CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STC fields
*
* Parameters:
*       name                                      - structure variable
*                                                   name or  path to
*                                                   structure field
*       nameExtension                             - structure field name
*                                                   (could be NULL)
*       val                                       - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STC
(
    const GT_CHAR_PTR                               name,
    const GT_CHAR_PTR                               nameExtension,
    const CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STC    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_ENUM(val,     fullName, cmd, CPSS_PACKET_CMD_ENT);
    P_ENUM(val,     fullName, cpuCodeIdx, CPSS_DXCH_IP_CPU_CODE_INDEX_ENT);
    P_BOOL(val,     fullName, appSpecificCpuCodeEnable);
    P_BOOL(val,     fullName, unicastPacketSipFilterEnable);
    P_BOOL(val,     fullName, ttlHopLimitDecEnable);
    P_BOOL(val,     fullName, ttlHopLimDecOptionsExtChkByPass);
    P_BOOL(val,     fullName, ingressMirror);
    P_NUMBER(val,   fullName, ingressMirrorToAnalyzerIndex, GT_U32);
    P_BOOL(val,     fullName, qosProfileMarkingEnable);
    P_NUMBER(val,   fullName, qosProfileIndex, GT_U32);
    P_ENUM(val,     fullName, qosPrecedence,
                                CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT);
    P_ENUM(val,     fullName, modifyUp, CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT);
    P_ENUM(val,     fullName, modifyDscp, CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT);
    P_ENUM(val,     fullName, countSet, CPSS_IP_CNT_SET_ENT);
    P_BOOL(val,     fullName, trapMirrorArpBcEnable);
    P_NUMBER(val,   fullName, sipAccessLevel, GT_U32);
    P_NUMBER(val,   fullName, dipAccessLevel, GT_U32);
    P_BOOL(val,     fullName, ICMPRedirectEnable);
    P_BOOL(val,     fullName, scopeCheckingEnable);
    P_ENUM(val,     fullName, siteId, CPSS_IP_SITE_ID_ENT);
    P_NUMBER(val,   fullName, mtuProfileIndex, GT_U32);
    P_BOOL(val,     fullName, isTunnelStart);
    P_BOOL(val,     fullName, isNat);
    P_NUMBER(val,   fullName, nextHopVlanId, GT_U16);
    P_STRUCT(val,   fullName, nextHopInterface, CPSS_INTERFACE_INFO_STC);
    P_NUMBER(val,   fullName, nextHopARPPointer, GT_U32);
    P_NUMBER(val,   fullName, nextHopTunnelPointer, GT_U32);
    P_NUMBER(val,   fullName, nextHopNatPointer, GT_U32);
    P_NUMBER(val,   fullName, nextHopVlanId1, GT_U16);
}

/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_IP_UC_ECMP_RPF_FORMAT_STC
*
* Description:
*       Prints CPSS_DXCH_IP_UC_ECMP_RPF_FORMAT_STC fields
*
* Parameters:
*       name                                  - structure variable name or
*                                               path to structure field
*       nameExtension                         - structure field name
*                                               (could be NULL)
*       val                                   - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_IP_UC_ECMP_RPF_FORMAT_STC
(
    const GT_CHAR_PTR                           name,
    const GT_CHAR_PTR                           nameExtension,
    const CPSS_DXCH_IP_UC_ECMP_RPF_FORMAT_STC   *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_ARRAY_START(val, fullName, vlanArray);
    {
        int idx;
        for (idx = 0; idx < 8; idx++) {
            P_ARRAY_NUMBER(val, fullName, vlanArray, idx, GT_U16);
        }
    }
    P_ARRAY_END(val, fullName, vlanArray);
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC
*
* Description:
*       Prints CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC fields
*
* Parameters:
*
* Returns:
*       name                              - structure variable name or
*                                           path to structure field
*       nameExtension                     - structure field name (could be
*                                           NULL)
*       val                               - pointer to structure
*
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC
(
    const GT_CHAR_PTR                       name,
    const GT_CHAR_PTR                       nameExtension,
    const CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC   *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_ENUM(val,     fullName, type, CPSS_DXCH_IP_UC_ROUTE_ENTRY_TYPE_ENT);

    switch (val->type)
    {
        case CPSS_DXCH_IP_UC_ROUTE_ENTRY_E:
            prv_lua_c_print_CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STC(
                fullName, "entry.regularEntry", &(val->entry.regularEntry));
            break;

        case CPSS_DXCH_IP_UC_ECMP_RPF_E:
            prv_lua_c_print_CPSS_DXCH_IP_UC_ECMP_RPF_FORMAT_STC(
                fullName, "entry.ecmpRpfCheck", &(val->entry.ecmpRpfCheck));
            break;
    }
}


/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_IP_LTT_ENTRY_STC
*
* Description:
*       Prints CPSS_DXCH_IP_LTT_ENTRY_STC fields
*
* Parameters:
*       name                          - structure variable name or path
*                                       to structure field
*       nameExtension                 - structure field name (could be
*                                       NULL)
*       val                           - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_IP_LTT_ENTRY_STC
(
    const GT_CHAR_PTR                   name,
    const GT_CHAR_PTR                   nameExtension,
    const CPSS_DXCH_IP_LTT_ENTRY_STC    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_ENUM(val,     fullName, routeType, CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ENT);
    P_NUMBER(val,   fullName, numOfPaths, GT_U32);
    P_NUMBER(val,   fullName, routeEntryBaseIndex, GT_U32);
    P_BOOL(val,     fullName, ucRPFCheckEnable);
    P_BOOL(val,     fullName, sipSaCheckMismatchEnable);
    P_ENUM(val,     fullName, ipv6MCGroupScopeLevel, CPSS_IPV6_PREFIX_SCOPE_ENT);
    P_ENUM(val,     fullName, priority, CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT);

}

/* CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT */
UNION_MEMBER_STRUCT(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT, pclIpUcAction, CPSS_DXCH_PCL_ACTION_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT, ipLttEntry, CPSS_DXCH_IP_LTT_ENTRY_STC)

/* CPSS_DXCH_PHA_THREAD_INFO_UNT */
UNION_MEMBER_NUMBER(CPSS_DXCH_PHA_THREAD_INFO_UNT, notNeeded, GT_U32)
UNION_MEMBER_STRUCT(CPSS_DXCH_PHA_THREAD_INFO_UNT, ioamIngressSwitchIpv4, CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV4_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PHA_THREAD_INFO_UNT, ioamIngressSwitchIpv6, CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV6_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PHA_THREAD_INFO_UNT, ioamTransitSwitchIpv4, CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV4_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PHA_THREAD_INFO_UNT, ioamTransitSwitchIpv6, CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_IPV6_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PHA_THREAD_INFO_UNT, ioamEgressSwitchIpv6, CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_EGRESS_SWITCH_IPV6_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PHA_THREAD_INFO_UNT, unifiedSRIpv6, CPSS_DXCH_PHA_THREAD_INFO_TYPE_UNIFIED_SR_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PHA_THREAD_INFO_UNT, classifierNshOverVxlanGpe, CPSS_DXCH_PHA_THREAD_INFO_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PHA_THREAD_INFO_UNT, srv6SrcNode, CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_SRC_NODE_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PHA_THREAD_INFO_UNT, sgtNetwork, CPSS_DXCH_PHA_THREAD_INFO_TYPE_SGT_NETWORK_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PHA_THREAD_INFO_UNT, vxlanGbpSourceGroupPolicyId, CPSS_DXCH_PHA_THREAD_INFO_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PHA_THREAD_INFO_UNT, srv6Coc32GsidCommonPrefix, CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_COC32_GSID_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PHA_THREAD_INFO_UNT, sflowV5Mirror, CPSS_DXCH_PHA_THREAD_INFO_TYPE_SFLOW_V5_STC)

/* CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT */
UNION_MEMBER_NUMBER(CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT, rawFormat, GT_U32)
UNION_MEMBER_STRUCT(CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT, erspanSameDevMirror, CPSS_DXCH_PHA_PORT_ENTRY_TYPE_ERSPAN_SAME_DEV_MIRROR_STC)

/* CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT */
UNION_MEMBER_NUMBER(CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT, rawFormat, GT_U32)
UNION_MEMBER_STRUCT(CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT, erspanSameDevMirror, CPSS_DXCH_PHA_PORT_ENTRY_TYPE_ERSPAN_SAME_DEV_MIRROR_STC)


#if undef
/* sample */

/* GT_DEV_ID */
PRV_UNION_LUA_TO_C_START(GT_DEV_ID, devNum)
    F_NUMBER(val, -1, devNum, GT_U8);
PRV_UNION_LUA_TO_C_END()

PRV_UNION_C_TO_LUA_START(GT_DEV_ID,devNum)
    FO_NUMBER(val, t, devNum, GT_U8);
PRV_UNION_C_TO_LUA_END()

add_mgm_type_union(GT_DEV_ID,devNum);

PRV_UNION_LUA_TO_C_START(GT_DEV_ID, i2cBaseAddr)
    F_NUMBER(val, -1, i2cBaseAddr, GT_U16);
PRV_UNION_LUA_TO_C_END()

PRV_UNION_C_TO_LUA_START(GT_DEV_ID,i2cBaseAddr)
    FO_NUMBER(val, t, i2cBaseAddr, GT_U16);
PRV_UNION_C_TO_LUA_END()

add_mgm_type_union(GT_DEV_ID,i2cBaseAddr);
#endif

/* CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT */
UNION_MEMBER_STRUCT(CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT, srTcmParams, CPSS_DXCH3_POLICER_METER_TB_SR_TCM_PARAMS_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT, trTcmParams, CPSS_DXCH3_POLICER_METER_TB_TR_TCM_PARAMS_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT, envelope, CPSS_DXCH3_POLICER_METER_TB_ENVELOPE_PARAMS_STC)

/* CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT */
UNION_MEMBER_STRUCT(CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT, compressed1BitVector, CPSS_DXCH_LPM_COMPRESSED_1_BIT_VECTOR_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT, compressed2BitVector, CPSS_DXCH_LPM_COMPRESSED_2_BIT_VECTOR_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_LPM_NODE_RANGE_SELECTION_UNT, regularBitVector, CPSS_DXCH_LPM_REGULAR_BIT_VECTOR_STC)

/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT
*
* Description:
*       Prints CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT fields
*
* Parameters:
*       name                                      - union variable name or
*                                                   path to union field
*       nameExtension                             - union field name
*                                                   (could be NULL)
*       val                                       - union variable
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT
(
    const GT_CHAR_PTR                               name,
    const GT_CHAR_PTR                               nameExtension,
    const CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT    *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    prv_lua_c_print_CPSS_DXCH_PCL_ACTION_STC(fullName, "pclIpUcAction",
                                             &(val->pclIpUcAction));
    prv_lua_c_print_CPSS_DXCH_IP_LTT_ENTRY_STC(fullName, "ipLttEntry",
                                               &(val->ipLttEntry));
}


/* CPSS_DXCH_PCL_ACTION_QOS_UNT */
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_ACTION_QOS_UNT, ingress, CPSS_DXCH_PCL_ACTION_QOS_INGRESS_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_ACTION_QOS_UNT, egress, CPSS_DXCH_PCL_ACTION_QOS_EGRESS_STC)

/* CPSS_DXCH_PCL_ACTION_VLAN_UNT */
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_ACTION_VLAN_UNT, ingress, CPSS_DXCH_PCL_ACTION_VLAN_INGRESS_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_ACTION_VLAN_UNT, egress, CPSS_DXCH_PCL_ACTION_VLAN_EGRESS_STC)

/* CPSS_DXCH_PCL_RULE_FORMAT_UNT */
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleStdNotIp, CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleStdIpL2Qos, CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleStdIpv4L4, CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleStdIpv6Dip, CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV6_DIP_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleIngrStdUdb, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleExtNotIpv6, CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleExtIpv6L2, CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleExtIpv6L4, CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleIngrExtUdb, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleEgrStdNotIp, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleEgrStdIpL2Qos, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleEgrStdIpv4L4, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleEgrExtNotIpv6, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleEgrExtIpv6L2, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleEgrExtIpv6L4, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleStdIpv4RoutedAclQos, CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleExtIpv4PortVlanQos, CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV4_PORT_VLAN_QOS_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleUltraIpv6PortVlanQos, CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleUltraIpv6RoutedAclQos, CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_ROUTED_ACL_QOS_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleEgrExtIpv4RaclVacl, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleEgrUltraIpv6RaclVacl, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleIngrUdbOnly, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_ONLY_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_PCL_RULE_FORMAT_UNT, ruleEgrUdbOnly, CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_ONLY_STC)

use_prv_struct(CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_IOAM_INGRESS_SWITCH_STC);

/*************************************************************************
* prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_UNT
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_UNT fields
*
* Parameters:
*       name                          - structure variable name or path to
*                                       structure field
*       nameExtension                 - structure field name (could be
*                                       NULL)
*       val                           - pointer to structure
*
*************************************************************************/
void prv_lua_c_print_CPSS_DXCH_PCL_RULE_FORMAT_UNT
(
    const GT_CHAR_PTR                   name,
    const GT_CHAR_PTR                   nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_UNT *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    P_STRUCT(val,   fullName, ruleStdNotIp,
                                    CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC);
    P_STRUCT(val,   fullName, ruleStdIpL2Qos,
                                CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC);
    P_STRUCT(val,   fullName, ruleStdIpv4L4,
                                    CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC);
    P_STRUCT(val,   fullName, ruleStdIpv6Dip,
                                    CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV6_DIP_STC);
    P_STRUCT(val,   fullName, ruleIngrStdUdb,
                            CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC);
    P_STRUCT(val,   fullName, ruleExtNotIpv6,
                                    CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC);
    P_STRUCT(val,   fullName, ruleExtIpv6L2,
                                    CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC);
    P_STRUCT(val,   fullName, ruleExtIpv6L4,
                                    CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC);
    P_STRUCT(val,   fullName, ruleIngrExtUdb,
                            CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC);
    P_STRUCT(val,   fullName, ruleEgrStdNotIp,
                            CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_STC);
    P_STRUCT(val,   fullName, ruleEgrStdIpL2Qos,
                            CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC);
    P_STRUCT(val,   fullName, ruleEgrStdIpv4L4,
                            CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC);
    P_STRUCT(val,   fullName, ruleEgrExtNotIpv6,
                            CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC);
    P_STRUCT(val,   fullName, ruleEgrExtIpv6L2,
                            CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC);
    P_STRUCT(val,   fullName, ruleEgrExtIpv6L4,
                            CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC);
    P_STRUCT(val,   fullName, ruleStdIpv4RoutedAclQos,
                        CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC);
    P_STRUCT(val,   fullName, ruleExtIpv4PortVlanQos,
                        CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV4_PORT_VLAN_QOS_STC);
    P_STRUCT(val,   fullName, ruleUltraIpv6PortVlanQos,
                        CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC);
    P_STRUCT(val,   fullName, ruleUltraIpv6RoutedAclQos,
                    CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_ROUTED_ACL_QOS_STC);
    P_STRUCT(val,   fullName, ruleEgrExtIpv4RaclVacl,
                    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_STC);
    P_STRUCT(val,   fullName, ruleEgrUltraIpv6RaclVacl,
                    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_STC);
}


/*************************************************************************
* prv_lua_c_union_print_CPSS_DXCH_PCL_RULE_FORMAT_UNT
*
* Description:
*       Prints CPSS_DXCH_PCL_RULE_FORMAT_UNT fields occording to entry key
*
* Parameters:
*       name                                  - structure variable name or
*                                               path to structure field
*       nameExtension                         - structure field name
*                                               (could be NULL)
*       key                                   - union key
*       val                                   - pointer to structure
*
*************************************************************************/
void prv_lua_c_union_print_CPSS_DXCH_PCL_RULE_FORMAT_UNT
(
    const GT_CHAR_PTR                           name,
    const GT_CHAR_PTR                           nameExtension,
    const CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT    key,
    const CPSS_DXCH_PCL_RULE_FORMAT_UNT         *val
)
{
    P_NAME_CONVERSIONS(fullName, name, nameExtension);
    switch (key)
    {
    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E:
        P_STRUCT(val,   fullName, ruleStdNotIp,
                                    CPSS_DXCH_PCL_RULE_FORMAT_STD_NOT_IP_STC);
        break;

    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E:
        P_STRUCT(val,   fullName, ruleStdIpL2Qos,
                                CPSS_DXCH_PCL_RULE_FORMAT_STD_IP_L2_QOS_STC);
        break;

    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E:
        P_STRUCT(val,   fullName, ruleStdIpv4L4,
                                    CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_L4_STC);
        break;

    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E:
        P_STRUCT(val,   fullName, ruleStdIpv6Dip,
                                    CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV6_DIP_STC);
        break;

    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E:
        P_STRUCT(val,   fullName, ruleExtNotIpv6,
                                    CPSS_DXCH_PCL_RULE_FORMAT_EXT_NOT_IPV6_STC);
        break;

    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E:
        P_STRUCT(val,   fullName, ruleExtIpv6L2,
                                    CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L2_STC);
        break;

    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E:
        P_STRUCT(val,   fullName, ruleExtIpv6L4,
                                    CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV6_L4_STC);
        break;

    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E:
        P_STRUCT(val,   fullName, ruleEgrStdNotIp,
                            CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_STC);
        break;

    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E:
        P_STRUCT(val,   fullName, ruleEgrStdIpL2Qos,
                            CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_STC);
        break;

    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E:
        P_STRUCT(val,   fullName, ruleEgrStdIpv4L4,
                            CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_STC);
        break;

    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E:
        P_STRUCT(val,   fullName, ruleEgrExtNotIpv6,
                            CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_STC);
        break;

    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E:
        P_STRUCT(val,   fullName, ruleEgrExtIpv6L2,
                            CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_STC);
        break;

    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E:
        P_STRUCT(val,   fullName, ruleEgrExtIpv6L4,
                            CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_STC);
        break;

    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E:
        P_STRUCT(val,   fullName, ruleIngrStdUdb,
                            CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STANDARD_UDB_STC);
        break;

    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E:
        P_STRUCT(val,   fullName, ruleIngrExtUdb,
                            CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXTENDED_UDB_STC);
        break;

    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E:
        P_STRUCT(val,   fullName, ruleStdIpv4RoutedAclQos,
                        CPSS_DXCH_PCL_RULE_FORMAT_STD_IPV4_ROUTED_ACL_QOS_STC);
        break;

    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E:
        P_STRUCT(val,   fullName, ruleExtIpv4PortVlanQos,
                        CPSS_DXCH_PCL_RULE_FORMAT_EXT_IPV4_PORT_VLAN_QOS_STC);
        break;

    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E:
        P_STRUCT(val,   fullName, ruleUltraIpv6PortVlanQos,
                        CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_PORT_VLAN_QOS_STC);
        break;

    case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E:
        P_STRUCT(val,   fullName, ruleUltraIpv6RoutedAclQos,
                    CPSS_DXCH_PCL_RULE_FORMAT_ULTRA_IPV6_ROUTED_ACL_QOS_STC);
        break;

    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E:
        P_STRUCT(val,   fullName, ruleEgrExtIpv4RaclVacl,
                    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_STC);
        break;

    case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E:
        P_STRUCT(val,   fullName, ruleEgrUltraIpv6RaclVacl,
                    CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_STC);
        break;

    default:
        P_MESSAGE("Wrong rule format key.");
        break;
    }
}



void prv_lua_to_c_CPSS_DXCH_PORT_EOM_MATRIX_STC(
    lua_State *L,
    CPSS_DXCH_PORT_EOM_MATRIX_STC *val
)
{
    F_ARRAY_START(val, -1, upperMatrix);
    {
        int idx;
        for (idx = 0; idx < CPSS_DXCH_PORT_EOM_VOLT_RANGE_CNS; idx++) {
            lua_rawgeti(L, -1, idx);
            if (lua_type(L, -1) == LUA_TTABLE)
            {
                int idx2;
                for (idx2 = 0; idx2 < CPSS_DXCH_PORT_EOM_PHASE_RANGE_CNS; idx2++)
                {
                    F_ARRAY_NUMBER(val, upperMatrix[idx], idx2, GT_U32);
                }
            }
            lua_pop(L, 1);
        }
    }
    F_ARRAY_END(val, -1, upperMatrix);
    F_ARRAY_START(val, -1, lowerMatrix);
    {
        int idx;
        for (idx = 0; idx < CPSS_DXCH_PORT_EOM_VOLT_RANGE_CNS; idx++) {
            lua_rawgeti(L, -1, idx);
            if (lua_type(L, -1) == LUA_TTABLE)
            {
                int idx2;
                for (idx2 = 0; idx2 < CPSS_DXCH_PORT_EOM_PHASE_RANGE_CNS; idx2++)
                {
                    F_ARRAY_NUMBER(val, lowerMatrix[idx], idx2, GT_U32);
                }
            }
            lua_pop(L, 1);
        }
    }
    F_ARRAY_END(val, -1, lowerMatrix);
}

void prv_c_to_lua_CPSS_DXCH_PORT_EOM_MATRIX_STC(
    lua_State *L,
    CPSS_DXCH_PORT_EOM_MATRIX_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, upperMatrix);
    {
        int idx, idx2;
        for (idx = 0; idx < CPSS_DXCH_PORT_EOM_VOLT_RANGE_CNS; idx++) {
            lua_newtable(L);
            for (idx2 = 0; idx2 < CPSS_DXCH_PORT_EOM_PHASE_RANGE_CNS; idx2++)
            {
                FO_ARRAY_NUMBER(val, upperMatrix[idx], idx2, GT_U32);
            }
            lua_rawseti(L, -2, idx);
        }
    }
    FO_ARRAY_END(val, t, upperMatrix);
    FO_ARRAY_START(val, t, lowerMatrix);
    {
        int idx, idx2;
        for (idx = 0; idx < CPSS_DXCH_PORT_EOM_VOLT_RANGE_CNS; idx++) {
            lua_newtable(L);
            for (idx2 = 0; idx2 < CPSS_DXCH_PORT_EOM_PHASE_RANGE_CNS; idx2++)
            {
                FO_ARRAY_NUMBER(val, lowerMatrix[idx], idx2, GT_U32);
            }
            lua_rawseti(L, -2, idx);
        }
    }
    FO_ARRAY_END(val, t, lowerMatrix);
    lua_settop(L, t);
}
add_mgm_type(CPSS_DXCH_PORT_EOM_MATRIX_STC);

/* CPSS_DXCH_TTI_RULE_UNT */
UNION_MEMBER_STRUCT(CPSS_DXCH_TTI_RULE_UNT, ipv4, CPSS_DXCH_TTI_IPV4_RULE_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_TTI_RULE_UNT, mpls, CPSS_DXCH_TTI_MPLS_RULE_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_TTI_RULE_UNT, eth, CPSS_DXCH_TTI_ETH_RULE_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_TTI_RULE_UNT, mim, CPSS_DXCH_TTI_MIM_RULE_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_TTI_RULE_UNT, udbArray, CPSS_DXCH_TTI_UDB_ARR_STC)


/* CPSS_DXCH_TUNNEL_START_CONFIG_UNT */
UNION_MEMBER_STRUCT(CPSS_DXCH_TUNNEL_START_CONFIG_UNT, ipv4Cfg, CPSS_DXCH_TUNNEL_START_IPV4_CONFIG_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_TUNNEL_START_CONFIG_UNT, ipv6Cfg, CPSS_DXCH_TUNNEL_START_IPV6_CONFIG_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_TUNNEL_START_CONFIG_UNT, mplsCfg, CPSS_DXCH_TUNNEL_START_MPLS_CONFIG_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_TUNNEL_START_CONFIG_UNT, mimCfg, CPSS_DXCH_TUNNEL_START_MIM_CONFIG_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_TUNNEL_START_CONFIG_UNT, genCfg, CPSS_DXCH_TUNNEL_START_GENERIC_CONFIG_STC)

/* CPSS_DXCH_IP_NAT_ENTRY_UNT */
UNION_MEMBER_STRUCT(CPSS_DXCH_IP_NAT_ENTRY_UNT, nat44Entry, CPSS_DXCH_IP_NAT44_ENTRY_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_IP_NAT_ENTRY_UNT, nat66Entry, CPSS_DXCH_IP_NAT66_ENTRY_STC)

/* CPSS_DXCH_EXACT_MATCH_ACTION_UNT */
UNION_MEMBER_STRUCT(CPSS_DXCH_EXACT_MATCH_ACTION_UNT, ttiAction, CPSS_DXCH_TTI_ACTION_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_EXACT_MATCH_ACTION_UNT, pclAction, CPSS_DXCH_PCL_ACTION_STC)

/* CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT */
UNION_MEMBER_STRUCT(CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT, ttiExpandedActionOrigin, CPSS_DXCH_EXACT_MATCH_EXPANDED_TTI_ACTION_ORIGIN_STC)
UNION_MEMBER_STRUCT(CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT, pclExpandedActionOrigin, CPSS_DXCH_EXACT_MATCH_EXPANDED_PCL_ACTION_ORIGIN_STC)

/* CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_UNT */
UNION_MEMBER_NUMBER(CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_UNT, globalEport, GT_PORT_NUM)
UNION_MEMBER_NUMBER(CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_UNT, trunkId, GT_TRUNK_ID)
UNION_MEMBER_NUMBER(CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_UNT, fid, GT_U32)
UNION_MEMBER_STRUCT(CPSS_DXCH_BRG_FDB_MANAGER_DYNAMIC_MAC_UC_LIMIT_UNT, devPort, CPSS_INTERFACE_DEV_PORT_STC)

void prv_lua_to_c_CPSS_DXCH_OAM_SRV_TRANSMIT_PARAMS_STC(
    lua_State *L,
    CPSS_DXCH_OAM_SRV_TRANSMIT_PARAMS_STC *val
)
{
    F_BOOL(val, -1, l2HeaderValid);
    /* done manually [[[ */
    lua_getfield(L, -1, "l2Header");
    if (lua_istable(L, -1))
    {
        /* assume the only correct member set in table */
        F_ARRAY_START(&(val->l2Header), -1, header);
        {
            int idx;
            for (idx = 0; idx < 64; idx++) {
                F_ARRAY_NUMBER(&(val->l2Header), header, idx, GT_U8);
            }
        }
        F_ARRAY_END(&(val->l2Header), -1, header);
        /* if set */
        lua_getfield(L, -1, "macAddr");
        if (lua_type(L, -1) != LUA_TNIL)
        {
            prv_lua_to_c_GT_ETHERADDR(L, &(val->l2Header.macAddr));
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    /* ]]] done manually */
    F_BOOL(val, -1, dsaParamValid);
    F_STRUCT(val, -1, dsaParam, CPSS_DXCH_NET_DSA_PARAMS_STC);
}

void prv_c_to_lua_CPSS_DXCH_OAM_SRV_TRANSMIT_PARAMS_STC(
    lua_State *L,
    CPSS_DXCH_OAM_SRV_TRANSMIT_PARAMS_STC *val
)
{
    int t, t1;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, l2HeaderValid);
    /* done manually [[[ */
    lua_newtable(L);
    t1 = lua_gettop(L);
        FO_ARRAY_START(&(val->l2Header), t1, header);
        {
            int idx;
            for (idx = 0; idx < 64; idx++) {
                FO_ARRAY_NUMBER(&(val->l2Header), header, idx, GT_U8);
            }
        }
        FO_ARRAY_END(&(val->l2Header), t1, header);

        FO_STRUCT(&(val->l2Header), t1, macAddr, GT_ETHERADDR);

    lua_setfield(L, t, "l2Header");
    /* ]]] done manually */
    FO_BOOL(val, t, dsaParamValid);
    FO_STRUCT(val, t, dsaParam, CPSS_DXCH_NET_DSA_PARAMS_STC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_DXCH_OAM_SRV_TRANSMIT_PARAMS_STC);


void prv_lua_to_c_CPSS_DXCH_PORT_PIZZA_CFG_SLICE_LIST_STC(
    lua_State *L,
    CPSS_DXCH_PIZZA_SLICE_ID *val
)
{
    if (lua_type(L, -1) == LUA_TTABLE)
    {
        int idx;
        for (idx = 0; idx < PIZZA_MAX_SLICES_PER_PORT_CNS+1; idx++)
        {
            lua_rawgeti(L, -1, idx);
            if (lua_type(L, -1) == LUA_TNUMBER)
            {
                val[idx] = (CPSS_DXCH_PIZZA_SLICE_ID)lua_tonumber(L, -1);
            }
            else
            {
                val[idx] = (CPSS_DXCH_PIZZA_SLICE_ID)0;
            }
            lua_pop(L, 1);
        }
    }
}

void prv_c_to_lua_CPSS_DXCH_PORT_PIZZA_CFG_SLICE_LIST_STC(
    lua_State *L,
    CPSS_DXCH_PIZZA_SLICE_ID *val
)
{
    int t, idx;
    lua_newtable(L);
    t = lua_gettop(L);
    for (idx = 0; idx < PIZZA_MAX_SLICES_PER_PORT_CNS+1; idx++)
    {
        lua_pushnumber(L, val[idx]);
        lua_rawseti(L, -2, idx);
    }
    lua_settop(L, t);
}
int mgmType_to_c_CPSS_DXCH_PORT_PIZZA_CFG_SLICE_LIST_STC(
    lua_State *L
)
{
    CPSS_DXCH_PIZZA_SLICE_ID *val = NULL;
    lua_newuserdata(L, sizeof(CPSS_DXCH_PORT_PIZZA_CFG_SLICE_LIST_STC));

    val = (CPSS_DXCH_PIZZA_SLICE_ID*)lua_touserdata(L, -1);
    cmdOsMemSet(val, 0, sizeof(CPSS_DXCH_PORT_PIZZA_CFG_SLICE_LIST_STC));
    if (lua_isnil(L, 1))
    {
        return 1;
    }
    lua_pushvalue(L, 1);
    prv_lua_to_c_CPSS_DXCH_PORT_PIZZA_CFG_SLICE_LIST_STC(L, val);
    lua_pop(L, 1);
    return 1;
}
int mgmType_to_lua_CPSS_DXCH_PORT_PIZZA_CFG_SLICE_LIST_STC(
    lua_State *L
)
{
    CPSS_DXCH_PIZZA_SLICE_ID *val;
    if (lua_isnil(L, 1))
        return 1;
    if (!lua_isuserdata(L, 1))
    {
        lua_pushnil(L);
        return 1;
    }
    val = (CPSS_DXCH_PIZZA_SLICE_ID*)lua_touserdata(L, 1);
    prv_c_to_lua_CPSS_DXCH_PORT_PIZZA_CFG_SLICE_LIST_STC(L, val);

    return 1;
}

void prv_lua_to_c_CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC(
    lua_State *L,
    CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC *val
)
{
    F_NUMBER(val, -1, maxPortNum, GT_U32);
    F_ARRAY_START(val, -1, portConfArr);
    {
        int idx;
        for (idx = 0; idx < PIZZA_MAX_PORTS_PER_PORTGROUP_CNS; idx++) {
            lua_rawgeti(L, -1, idx);
            if (lua_type(L, -1) == LUA_TTABLE)
            {
                prv_lua_to_c_CPSS_DXCH_PORT_PIZZA_CFG_SLICE_LIST_STC(L, val->portConfArr[idx]);
            }
            lua_pop(L, 1);
        }
    }
    F_ARRAY_END(val, -1, portConfArr);
}

void prv_c_to_lua_CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC(
    lua_State *L,
    CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, maxPortNum, GT_U32);
    FO_ARRAY_START(val, t, portConfArr);
    {
        int idx;
        for (idx = 0; idx < PIZZA_MAX_PORTS_PER_PORTGROUP_CNS; idx++) {
            prv_c_to_lua_CPSS_DXCH_PORT_PIZZA_CFG_SLICE_LIST_STC(L, val->portConfArr[idx]);
            lua_rawseti(L, -2, idx);
        }
    }
    FO_ARRAY_END(val, t, portConfArr);
    lua_settop(L, t);
}
add_mgm_type(CPSS_DXCH_PORT_PIZZA_CFG_PORT_SLICE_LIST_STC);


use_prv_struct(CPSS_DXCH_LION2_DEV_PIZZA_ARBITER_STATE_STC);
use_prv_struct(CPSS_DXCH_BOBCAT2_DEV_PIZZA_ARBITER_STATE_STC);
use_prv_struct(CPSS_DXCH_BOBK_DEV_PIZZA_ARBITER_STATE_STC);

void prv_lua_to_c_CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC(
    lua_State *L,
    CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC *val
)
{
    F_ENUM(val, -1, devType, CPSS_PP_FAMILY_TYPE_ENT);
    lua_getfield(L, -1, "devState");
    if (lua_istable(L, -1))
    {
        switch (val->devType)
        {
            case CPSS_PP_FAMILY_DXCH_LION2_E:
                lua_getfield(L, -1, "lion2");
                prv_lua_to_c_CPSS_DXCH_LION2_DEV_PIZZA_ARBITER_STATE_STC(L, &((val)->devState.lion2));
                lua_pop(L, 1);
                break;
            case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
                lua_getfield(L, -1, "bobK");
                prv_lua_to_c_CPSS_DXCH_BOBK_DEV_PIZZA_ARBITER_STATE_STC(L, &((val)->devState.bobK));
                lua_pop(L, 1);
                break;
            case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            case CPSS_PP_FAMILY_DXCH_AC3X_E:
                lua_getfield(L, -1, "bobK");
                prv_lua_to_c_CPSS_DXCH_BOBK_DEV_PIZZA_ARBITER_STATE_STC(L, &((val)->devState.bobK));
                lua_pop(L, 1);
                break;
            case CPSS_PP_FAMILY_DXCH_FALCON_E:
            case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
                lua_getfield(L, -1, "bc2");
                prv_lua_to_c_CPSS_DXCH_BOBCAT2_DEV_PIZZA_ARBITER_STATE_STC(L, &((val)->devState.bc2));
                lua_pop(L, 1);
                break;
            default:
                break;
        }
    }
    lua_pop(L, 1);
}

void prv_c_to_lua_CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC(
    lua_State *L,
    CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, devType, CPSS_PP_FAMILY_TYPE_ENT);
    lua_newtable(L);

    switch (val->devType)
    {
        case CPSS_PP_FAMILY_DXCH_LION2_E:
            prv_c_to_lua_CPSS_DXCH_LION2_DEV_PIZZA_ARBITER_STATE_STC(L, &((val)->devState.lion2));
            lua_setfield(L, t+1, "lion2");
            break;
        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            prv_c_to_lua_CPSS_DXCH_BOBK_DEV_PIZZA_ARBITER_STATE_STC(L, &((val)->devState.bobK));
            lua_setfield(L, t+1, "bobK");
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            prv_c_to_lua_CPSS_DXCH_BOBK_DEV_PIZZA_ARBITER_STATE_STC(L, &((val)->devState.bobK));
            lua_setfield(L, t+1, "bobK");
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
            prv_c_to_lua_CPSS_DXCH_BOBCAT2_DEV_PIZZA_ARBITER_STATE_STC(L, &((val)->devState.bc2));
            lua_setfield(L, t+1, "bc2");
            break;
        default:
            break;
    }
    lua_setfield(L, t, "devState");
}
add_mgm_type(CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC);

extern void prv_lua_to_c_CPSS_TRUNK_MEMBER_STC(
    lua_State *L,
    CPSS_TRUNK_MEMBER_STC *val
);
extern void prv_c_to_lua_CPSS_TRUNK_MEMBER_STC(
    lua_State *L,
    CPSS_TRUNK_MEMBER_STC *val
);

/* replace CPSS_TRUNK_MEMBER_STC for :
    cpssDxChTrunkTableEntrySet,
    cpssDxChTrunkTableEntryGet
*/
typedef struct{
    GT_U32                numMembers;
    CPSS_TRUNK_MEMBER_STC membersArray[CPSS_TRUNK_4K_MAX_NUM_OF_MEMBERS_CNS];
}CPSS_TRUNK_MEMBERS_AND_NUM_STC;

void prv_lua_to_c_CPSS_TRUNK_MEMBERS_AND_NUM_STC(
    lua_State *L,
    CPSS_TRUNK_MEMBERS_AND_NUM_STC *val
)
{
    F_NUMBER(val, -1, numMembers, GT_U32);
    F_ARRAY_START(val, -1, members);
    {
        GT_U32 idx;
        for (idx = 0; idx < val->numMembers ; idx++) {
            lua_rawgeti(L, -1, idx);
            if (lua_type(L, -1) == LUA_TTABLE)
            {
                prv_lua_to_c_CPSS_TRUNK_MEMBER_STC(L, &(val->membersArray[idx]));
            }
            lua_pop(L, 1);
        }
    }
    F_ARRAY_END(val, -1, members);
}

void prv_c_to_lua_CPSS_TRUNK_MEMBERS_AND_NUM_STC(
    lua_State *L,
    CPSS_TRUNK_MEMBERS_AND_NUM_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, numMembers, GT_U32);
    FO_ARRAY_START(val, t, members);
    {
        GT_U32 idx;
        for (idx = 0; idx < val->numMembers ; idx++) {
            prv_c_to_lua_CPSS_TRUNK_MEMBER_STC(L, &(val->membersArray[idx]));
            lua_rawseti(L, -2, idx);
        }
    }
    FO_ARRAY_END(val, t, members);
    lua_settop(L, t);
}
add_mgm_type(CPSS_TRUNK_MEMBERS_AND_NUM_STC);


/*************************************************************************
* prv_lua_to_c_APP_DEMO_MICRO_INIT_STC
*
* Description:
*       Convert to "c" APP_DEMO_MICRO_INIT_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_APP_DEMO_MICRO_INIT_STC
(
    lua_State                               *L,
    APP_DEMO_MICRO_INIT_STC  *val
)
{
    F_NUMBER(val,   -1, deviceId, GT_U32);
    F_NUMBER(val,   -1, mngInterfaceType, GT_U32);
    F_NUMBER(val,   -1, coreClock, GT_U32);
    F_NUMBER(val,   -1, deviceNum, GT_U32);
    F_NUMBER(val,   -1, featuresBitmap, GT_U32);
}


/*************************************************************************
* prv_c_to_lua_APP_DEMO_MICRO_INIT_STC
*
* Description:
*       Convert APP_DEMO_MICRO_INIT_STC to lua
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_c_to_lua_APP_DEMO_MICRO_INIT_STC(
    lua_State                               *L,
    APP_DEMO_MICRO_INIT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val,  t, deviceId, GT_U32);
    FO_NUMBER(val,  t, mngInterfaceType, GT_U32);
    FO_NUMBER(val,  t, coreClock, GT_U32);
    FO_NUMBER(val,  t, deviceNum, GT_U32);
    FO_NUMBER(val,  t, featuresBitmap, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(APP_DEMO_MICRO_INIT_STC);



void prv_lua_to_c_CPSS_DXCH_LOGICAL_INDEX_INFO_STC(
    lua_State *L,
    CPSS_DXCH_LOGICAL_INDEX_INFO_STC *val
)
{
    F_NUMBER(val, -1, numOfLogicalTables, GT_U32);
    F_ARRAY_START(val, -1, logicaTableInfo);
    {
        int idx;
        for (idx = 0; idx < CPSS_DXCH_LOGICAL_IN_LOCATION_MAX_CNS; idx++) {
            F_ARRAY_STRUCT(val, logicaTableInfo, idx, CPSS_DXCH_LOGICAL_TABLE_INFO_STC);
        }
    }
    F_ARRAY_END(val, -1, portConfArr);
}

void prv_c_to_lua_CPSS_DXCH_LOGICAL_INDEX_INFO_STC(
    lua_State *L,
    CPSS_DXCH_LOGICAL_INDEX_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val, t, numOfLogicalTables, GT_U32);
    FO_ARRAY_START(val, t, logicaTableInfo);
    {
        int idx;
        for (idx = 0; idx < CPSS_DXCH_LOGICAL_IN_LOCATION_MAX_CNS; idx++) {
            F_ARRAY_STRUCT(val, logicaTableInfo, idx, CPSS_DXCH_LOGICAL_TABLE_INFO_STC);
        }
    }
    FO_ARRAY_END(val, t, portConfArr);
    lua_settop(L, t);
}
add_mgm_type(CPSS_DXCH_LOGICAL_INDEX_INFO_STC);

/* support typedef of one STC with other STC , like :
typedef CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC
        CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_IPV4_STC;
*/
#define SUPPORT_STC_TYPEDEF(stcSrc,stcTrg)                      \
extern void prv_lua_to_c_ ## stcSrc(lua_State *L,stcSrc *val);  \
extern void prv_c_to_lua_ ## stcSrc(lua_State *L,stcSrc *val);  \
void prv_lua_to_c_ ## stcTrg(lua_State *L, stcTrg *val)         \
{                                                               \
    prv_lua_to_c_ ## stcSrc(L, val);                            \
}                                                               \
void prv_c_to_lua_ ## stcTrg(lua_State *L, stcTrg *val)         \
{                                                               \
    prv_c_to_lua_ ## stcSrc(L, val);                            \
}

add_mgm_type(CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_STC);
void prv_lua_to_c_CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_STC(
    lua_State *L,
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_STC *val
)
{
    F_ENUM(val, -1, phaThreadIdAssignmentMode, CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENT);
    F_NUMBER(val, -1, phaThreadId, GT_U32);
    F_ENUM(val, -1, phaThreadType, CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_ENT);
    F_UNION(val, -1, phaThreadUnion, CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_UNT);
}

void prv_c_to_lua_CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_STC(
    lua_State *L,
    CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INFO_STC *val
)
{
    int t,t1;
    lua_newtable(L);
    t = lua_gettop(L);

    FO_ENUM(val, t, phaThreadIdAssignmentMode, CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENT);
    FO_NUMBER(val, t, phaThreadId, GT_U32);
    FO_ENUM(val, t, phaThreadType, CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_ENT);

    /* done manually [[[ */
    lua_newtable(L);
    t1 = lua_gettop(L);
    if(val->phaThreadType == CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_NOT_USED_E)
    {
        FO_UNION_MEMBER_NUMBER(val->phaThreadUnion, t1, notNeeded, GT_U32);
    }
    else
    {
        /*
        lua_newtable(L);
        t2 = lua_gettop(L);
            FO_BOOL(&(val->phaThreadUnion.epclIoamIngressSwitch), t2, ioamIncludesBothE2EOptionAndTraceOption);
        lua_setfield(L, t1, "epclIoamIngressSwitch");
        */
        FO_UNION_MEMBER_STRUCT(val->phaThreadUnion, t1, epclIoamIngressSwitch, CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_IOAM_INGRESS_SWITCH_STC);
    }
    lua_setfield(L, t, "phaThreadUnion");
    /* ]]] done manually */

    lua_settop(L, t);
}

void prv_lua_to_c_CPSS_DXCH_FLOW_MANAGER_STC(
    lua_State *L,
    CPSS_DXCH_FLOW_MANAGER_STC *val
)
{
    F_STRUCT(val, -1, cpssFwCfg, CPSS_DXCH_FLOW_MANAGER_FW_CFG_STC);
    F_NUMBER(val, -1, flowDbSize, GT_U32);
    val->appCbFlowNotificaion = NULL;
}

void prv_c_to_lua_CPSS_DXCH_FLOW_MANAGER_STC(
    lua_State *L,
    CPSS_DXCH_FLOW_MANAGER_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_STRUCT(val, t, cpssFwCfg, CPSS_DXCH_FLOW_MANAGER_FW_CFG_STC);
    FO_NUMBER(val, t, flowDbSize, GT_U32);
    FO_PTR(val, t, appCbFlowNotificaion, CPSS_DXCH_FLOW_MANAGER_NEW_FLOW_CB_FUNC);
    lua_settop(L, t);
}
add_mgm_type(CPSS_DXCH_FLOW_MANAGER_STC);


void prv_lua_to_c_CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC(
    lua_State *L,
    CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC *val
)
{
    lua_getfield(L, -1, "udsNameArr");
    if (lua_type(L, -1) == LUA_TTABLE)
    {
        const char *array[128];
        int idx,len=0,stop=0;
        for (idx = 0; idx < 128; idx++) {
            lua_rawgeti(L, -1, idx);
            if ((lua_type(L, -1) == LUA_TSTRING) || (lua_type(L, -1) == LUA_TNUMBER))
            {
                if(cpssOsStrCmp(lua_tostring(L, -1),"#")==0)
                    stop=1;

                if (stop == 0) {
                    array[idx] = lua_tostring(L, -1);
                    len++;
                }
            }
            lua_pop(L, 1);
        }

        lua_pop(L, 1);
        for (idx=0; idx < len; idx++) {
            cpssOsStrCpy(&val->udsNameArr[idx],array[idx]);
        }
    }
    else
    {
        const char *array = lua_tostring(L, -1);
        unsigned int idx;
        for (idx=0; idx<lua_objlen(L,-1); idx++)
        {
            cpssOsStrNCpy(&val->udsNameArr[idx],&array[idx],sizeof(char));
        }
        lua_pop(L, 1);
    }
}

void prv_c_to_lua_CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC(
    lua_State *L,
    CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, udsNameArr);
    {
        int idx;
        for (idx = 0; idx < 128; idx++) {
            const char *array = &val->udsNameArr[idx];
            lua_pushstring(L, array);
            lua_rawseti(L, -2, idx);
        }
    }
    FO_ARRAY_END(val, t, udsNameArr);
    lua_settop(L, t);
}
add_mgm_type(CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC);

void prv_lua_to_c_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC(
    lua_State *L,
    CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC *val
)
{
    lua_getfield(L, -1, "instanceId");
    if (lua_type(L, -1) == LUA_TTABLE)
    {
        const char *array[128];
        int idx,len=0,stop=0;
        for (idx = 0; idx < 128; idx++) {
            lua_rawgeti(L, -1, idx);
            if ((lua_type(L, -1) == LUA_TSTRING) || (lua_type(L, -1) == LUA_TNUMBER))
            {
                if(cpssOsStrCmp(lua_tostring(L, -1),"#")==0)
                    stop=1;

                if (stop == 0) {
                    array[idx] = lua_tostring(L, -1);
                    len++;
                }
            }
            lua_pop(L, 1);
        }

        lua_pop(L, 1);
        for (idx=0; idx < len; idx++) {
            cpssOsStrCpy(&val->instanceId[idx],array[idx]);
        }
    }
    else
    {
        const char *array = lua_tostring(L, -1);
        unsigned int idx;
        for (idx=0; idx<lua_objlen(L,-1); idx++)
        {
            cpssOsStrNCpy(&val->instanceId[idx],&array[idx],sizeof(char));
        }
        lua_pop(L, 1);
    }


    lua_getfield(L, -1, "interfaceId");
    if (lua_type(L, -1) == LUA_TTABLE)
    {
        const char *array[128];
        int idx,len=0,stop=0;
        for (idx = 0; idx < 128; idx++) {
            lua_rawgeti(L, -1, idx);
            if ((lua_type(L, -1) == LUA_TSTRING) || (lua_type(L, -1) == LUA_TNUMBER))
            {
                if(cpssOsStrCmp(lua_tostring(L, -1),"#")==0)
                    stop=1;

                if (stop == 0) {
                    array[idx] = lua_tostring(L, -1);
                    len++;
                }
            }
            lua_pop(L, 1);
        }

        lua_pop(L, 1);
        for (idx=0; idx < len; idx++) {
            cpssOsStrCpy(&val->interfaceId[idx],array[idx]);
        }
    }
    else
    {
        const char *array = lua_tostring(L, -1);
        unsigned int idx;
        for (idx=0; idx<lua_objlen(L,-1); idx++)
        {
            cpssOsStrNCpy(&val->interfaceId[idx],&array[idx],sizeof(char));
        }
        lua_pop(L, 1);
    }

    F_NUMBER(val, -1, interfaceIndex, GT_U32);
}

void prv_c_to_lua_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC(
    lua_State *L,
    CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, instanceId);
    {
        int idx;
        for (idx = 0; idx < 128; idx++) {
            const char *array = &val->instanceId[idx];
            lua_pushstring(L, array);
            lua_rawseti(L, -2, idx);
        }
    }
    FO_ARRAY_END(val, t, instanceId);
    FO_ARRAY_START(val, t, interfaceId);
    {
        int idx;
        for (idx = 0; idx < 128; idx++) {
            const char *array = &val->interfaceId[idx];
            lua_pushstring(L, array);
            lua_rawseti(L, -2, idx);
        }
    }
    FO_ARRAY_END(val, t, interfaceId);
    FO_NUMBER(val, t, interfaceIndex, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC);

void prv_lua_to_c_CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC(
    lua_State *L,
    CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC *val
)
{
    lua_getfield(L, -1, "udfNameArr");
    if (lua_type(L, -1) == LUA_TTABLE)
    {
        const char *array[128];
        int idx,len=0,stop=0;
        for (idx = 0; idx < 128; idx++) {
            lua_rawgeti(L, -1, idx);
            if ((lua_type(L, -1) == LUA_TSTRING) || (lua_type(L, -1) == LUA_TNUMBER))
            {
                if(cpssOsStrCmp(lua_tostring(L, -1),"#")==0)
                    stop=1;

                if (stop == 0) {
                    array[idx] = lua_tostring(L, -1);
                    len++;
                }
            }
            lua_pop(L, 1);
        }

        lua_pop(L, 1);
        for (idx=0; idx < len; idx++) {
            cpssOsStrCpy(&val->udfNameArr[idx],array[idx]);
        }
    }
    else
    {
        const char *array = lua_tostring(L, -1);
        unsigned int idx;
        for (idx=0; idx<lua_objlen(L,-1); idx++)
        {
            cpssOsStrNCpy(&val->udfNameArr[idx],&array[idx],sizeof(char));
        }
        lua_pop(L, 1);
    }
}

void prv_c_to_lua_CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC(
    lua_State *L,
    CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, udfNameArr);
    {
        int idx;
        for (idx = 0; idx < 128; idx++) {
            const char *array = &val->udfNameArr[idx];
            lua_pushstring(L, array);
            lua_rawseti(L, -2, idx);
        }
    }
    FO_ARRAY_END(val, t, udfNameArr);
    lua_settop(L, t);
}
add_mgm_type(CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC);

void prv_lua_to_c_CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC(
    lua_State *L,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC *val
)
{
    lua_getfield(L, -1, "fieldNameArr");
    if (lua_type(L, -1) == LUA_TTABLE)
    {
        const char *array[128];
        int idx,len=0,stop=0;
        for (idx = 0; idx < 128; idx++) {
            lua_rawgeti(L, -1, idx);
            if ((lua_type(L, -1) == LUA_TSTRING) || (lua_type(L, -1) == LUA_TNUMBER))
            {
                if(cpssOsStrCmp(lua_tostring(L, -1),"#")==0)
                    stop=1;

                if (stop == 0) {
                    array[idx] = lua_tostring(L, -1);
                    len++;
                }
            }
            lua_pop(L, 1);
        }

        lua_pop(L, 1);
        for (idx=0; idx < len; idx++) {
            cpssOsStrCpy(&val->fieldNameArr[idx],array[idx]);
        }
    }
    else
    {
        const char *array = lua_tostring(L, -1);
        unsigned int idx;
        for (idx=0; idx<lua_objlen(L,-1); idx++)
        {
            cpssOsStrNCpy(&val->fieldNameArr[idx],&array[idx],sizeof(char));
        }
        lua_pop(L, 1);
    }
}

void prv_c_to_lua_CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC(
    lua_State *L,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, fieldNameArr);
    {
        int idx;
        for (idx = 0; idx < 128; idx++) {
            const char *array = &val->fieldNameArr[idx];
            lua_pushstring(L, array);
            lua_rawseti(L, -2, idx);
        }
    }
    FO_ARRAY_END(val, t, fieldNameArr);
    lua_settop(L, t);
}
add_mgm_type(CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC);

void prv_lua_to_c_CPSS_DXCH_PORT_SERDES_EYE_RESULT_STC(
    lua_State *L,
    CPSS_DXCH_PORT_SERDES_EYE_RESULT_STC *val
)
{
    const char *array = lua_tostring(L, -1);
    unsigned int idx;

    lua_getfield(L, -1, "matrixPtr");
    for (idx=0; idx<lua_objlen(L,-1); idx++)
    {
        cpssOsStrNCpy(&val->matrixPtr[idx],&array[idx],sizeof(char));
    }
    lua_pop(L, 1);

    F_NUMBER(val, -1, x_points, GT_U32);
    F_NUMBER(val, -1, y_points, GT_U32);

    lua_getfield(L, -1, "vbtcPtr");
    for (idx=0; idx<lua_objlen(L,-1); idx++)
    {
        cpssOsStrNCpy(&val->vbtcPtr[idx],&array[idx],sizeof(char));
    }
    lua_pop(L, 1);

    lua_getfield(L, -1, "hbtcPtr");
    for (idx=0; idx<lua_objlen(L,-1); idx++)
    {
        cpssOsStrNCpy(&val->hbtcPtr[idx],&array[idx],sizeof(char));
    }
    lua_pop(L, 1);

    F_NUMBER(val, -1, height_mv, GT_U32);
    F_NUMBER(val, -1, width_mui, GT_U32);
    F_NUMBER(val, -1, globalSerdesNum, GT_U32);
}

void prv_c_to_lua_CPSS_DXCH_PORT_SERDES_EYE_RESULT_STC(
    lua_State *L,
    CPSS_DXCH_PORT_SERDES_EYE_RESULT_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);

    if(val->matrixPtr)
    {
        FO_ARRAY_START(val, t, matrixPtr);
        {
            const char *array = &val->matrixPtr[0];
            lua_pushstring(L, array);
            lua_rawseti(L, -2, 0);
        }
        FO_ARRAY_END(val, t, matrixPtr);
    }
    FO_NUMBER(val, t, x_points, GT_U32);
    FO_NUMBER(val, t, y_points, GT_U32);

    if(val->vbtcPtr)
    {
        FO_ARRAY_START(val, t, vbtcPtr);
        {
            const char *array = &val->vbtcPtr[0];
            lua_pushstring(L, array);
            lua_rawseti(L, -2, 0);
        }
        FO_ARRAY_END(val, t, vbtcPtr);
    }

    if(val->hbtcPtr)
    {
        FO_ARRAY_START(val, t, hbtcPtr);
        {
            const char *array = &val->hbtcPtr[0];
            lua_pushstring(L, array);
            lua_rawseti(L, -2, 0);
        }
        FO_ARRAY_END(val, t, hbtcPtr);
    }
    FO_NUMBER(val, t, height_mv, GT_U32);
    FO_NUMBER(val, t, width_mui, GT_U32);
    FO_NUMBER(val, t, globalSerdesNum, GT_U32);
    lua_settop(L, t);
}
add_mgm_type(CPSS_DXCH_PORT_SERDES_EYE_RESULT_STC);

/*************************************************************************
* prv_lua_to_c_APP_DEMO_INIT_DEV_INFO_STC
*
* Description:
*       Convert to "c" APP_DEMO_INIT_DEV_INFO_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_APP_DEMO_INIT_DEV_INFO_STC
(
    lua_State                               *L,
    APP_DEMO_INIT_DEV_INFO_STC  *val
)
{
    F_NUMBER(val,   -1, devType, GT_U32);
    F_NUMBER(val,   -1, swDevId, GT_U32);
    F_NUMBER(val,   -1, hwDevId, GT_U32);
    F_NUMBER(val,   -1, pciBus, GT_U32);
    F_NUMBER(val,   -1, pciDev, GT_U32);
    F_NUMBER(val,   -1, pciFunc, GT_U32);
    F_ARRAY_START(val, -1, pciHeaderInfo);
    {
        int idx;
        for (idx = 0; idx < 16; idx++)
        {
            F_ARRAY_NUMBER(val, pciHeaderInfo, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, pciHeaderInfo);

}


/*************************************************************************
* prv_c_to_lua_APP_DEMO_INIT_DEV_INFO_STC
*
* Description:
*       Convert APP_DEMO_INIT_DEV_INFO_STC to lua
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_c_to_lua_APP_DEMO_INIT_DEV_INFO_STC(
    lua_State                               *L,
    APP_DEMO_INIT_DEV_INFO_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val,  t, devType, GT_U32);
    FO_NUMBER(val,  t, swDevId, GT_U32);
    FO_NUMBER(val,  t, hwDevId, GT_U32);
    FO_NUMBER(val,  t, pciBus, GT_U32);
    FO_NUMBER(val,  t, pciDev, GT_U32);
    FO_NUMBER(val,  t, pciFunc, GT_U32);
    FO_ARRAY_START(val, t, pciHeaderInfo);
    {
        int idx;
        for (idx = 0; idx < 16; idx++) {
            FO_ARRAY_NUMBER(val, pciHeaderInfo, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, pciHeaderInfo);


    lua_settop(L, t);
}

add_mgm_type(APP_DEMO_INIT_DEV_INFO_STC);





/*************************************************************************
* prv_lua_to_c_CPSS_PP_HA_2_PHASES_INIT_PHASE1_STC
*
* Description:
*       Convert to "c" CPSS_PP_HA_2_PHASES_INIT_PHASE1_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_CPSS_PP_HA_2_PHASES_INIT_PHASE1_STC
(
    lua_State                               *L,
    CPSS_PP_HA_2_PHASES_INIT_PHASE1_STC     *val
)
{
    F_NUMBER(val,   -1, pciBus, GT_U32);
    F_NUMBER(val,   -1, pciDev, GT_U32);
    F_NUMBER(val,   -1, pciFunc, GT_U32);
    F_NUMBER(val,   -1, devType, GT_U32);
    F_ARRAY_START(val, -1, pciHeaderInfo);
    {
        int idx;
        for (idx = 0; idx < 16; idx++)
        {
            F_ARRAY_NUMBER(val, pciHeaderInfo, idx, GT_U32);
        }
    }
    F_ARRAY_END(val, -1, pciHeaderInfo);
    F_NUMBER(val,   -1, deviceRevision, GT_U8);
    F_NUMBER(val,   -1, coreClock, GT_U32);
    F_BOOL(val, -1, maxLengthSrcIdInFdbEn);
    F_BOOL(val, -1, tag1VidFdbEn);
    F_NUMBER(val, -1, portVlanfltTabAccessMode, GT_U32);
    F_NUMBER(val,   -1, portIsolationLookupPortBits, GT_U32);
    F_NUMBER(val,   -1, portIsolationLookupDeviceBits, GT_U32);
    F_NUMBER(val,   -1, portIsolationLookupTrunkBits, GT_U32);
    F_NUMBER(val,   -1, portIsolationLookupTrunkIndexBase, GT_U32);

}


/*************************************************************************
* prv_c_to_lua_CPSS_PP_HA_2_PHASES_INIT_PHASE1_STC
*
* Description:
*       Convert CPSS_PP_HA_2_PHASES_INIT_PHASE1_STC to lua
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_c_to_lua_CPSS_PP_HA_2_PHASES_INIT_PHASE1_STC(
    lua_State                               *L,
    CPSS_PP_HA_2_PHASES_INIT_PHASE1_STC     *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_NUMBER(val,  t, pciBus, GT_U32);
    FO_NUMBER(val,  t, pciDev, GT_U32);
    FO_NUMBER(val,  t, pciFunc, GT_U32);
    FO_NUMBER(val,  t, devType, GT_U32);
    FO_ARRAY_START(val, t, pciHeaderInfo);
    {
        int idx;
        for (idx = 0; idx < 16; idx++) {
            FO_ARRAY_NUMBER(val, pciHeaderInfo, idx, GT_U32);
        }
    }
    FO_ARRAY_END(val, t, pciHeaderInfo);
    FO_NUMBER(val,  t, deviceRevision, GT_U32);
    FO_NUMBER(val,  t, coreClock, GT_U32);
    FO_BOOL(val, t, maxLengthSrcIdInFdbEn);
    FO_BOOL(val, t, tag1VidFdbEn);
    FO_NUMBER(val, t, portVlanfltTabAccessMode, GT_U32);
    FO_NUMBER(val,  t, portIsolationLookupPortBits, GT_U32);
    FO_NUMBER(val,  t, portIsolationLookupDeviceBits, GT_U32);
    FO_NUMBER(val,  t, portIsolationLookupTrunkBits, GT_U32);
    FO_NUMBER(val,  t, portIsolationLookupTrunkIndexBase, GT_U32);
    lua_settop(L, t);
}

add_mgm_type(CPSS_PP_HA_2_PHASES_INIT_PHASE1_STC);

/* Traffic Gen */
void prv_lua_to_c_TGF_TRAFFIC_GEN_L4_CUSTOM_TAG_UNT(
    lua_State *L,
    TGF_TRAFFIC_GEN_L4_CUSTOM_TAG_UNT *val
)
{
    F_ARRAY_START(val, -1, udp);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            F_ARRAY_NUMBER(val, udp, idx, GT_U8);
        }
    }
    F_ARRAY_END(val, -1, udp);
    F_ARRAY_START(val, -1, tcp);
    {
        int idx;
        for (idx = 0; idx < 16; idx++) {
            F_ARRAY_NUMBER(val, tcp, idx, GT_U8);
        }
    }
    F_ARRAY_END(val, -1, tcp);
}

void prv_c_to_lua_TGF_TRAFFIC_GEN_L4_CUSTOM_TAG_UNT(
    lua_State *L,
    TGF_TRAFFIC_GEN_L4_CUSTOM_TAG_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, udp);
    {
        int idx;
        for (idx = 0; idx < 4; idx++) {
            FO_ARRAY_NUMBER(val, udp, idx, GT_U8);
        }
    }
    FO_ARRAY_END(val, t, udp);
    FO_ARRAY_START(val, t, tcp);
    {
        int idx;
        for (idx = 0; idx < 16; idx++) {
            FO_ARRAY_NUMBER(val, tcp, idx, GT_U8);
        }
    }
    FO_ARRAY_END(val, t, tcp);
    lua_settop(L, t);
}
add_mgm_type(TGF_TRAFFIC_GEN_L4_CUSTOM_TAG_UNT);
add_mgm_union(TGF_TRAFFIC_GEN_L4_CUSTOM_TAG_UNT);

void prv_lua_to_c_TGF_TRAFFIC_GEN_PARAMS_STC(
    lua_State *L,
    TGF_TRAFFIC_GEN_PARAMS_STC *val
)
{
    F_ENUM(val, -1, pktType, TGF_TRAFFIC_GEN_PKT_TYPE_ENT);
    F_ENUM(val, -1, streamType, TGF_TRAFFIC_GEN_STREAM_TYPE_ENT);
    F_NUMBER(val, -1, pktCnt, GT_U32);
    F_NUMBER(val, -1, port, GT_PORT_NUM);
    F_NUMBER(val, -1, pktSize, GT_U32);
    F_NUMBER(val, -1, vlanId, GT_U16);
    F_STRUCT_CUSTOM(val, -1, saMac, GT_ETHERADDR);
    F_STRUCT_CUSTOM(val, -1, daMac, GT_ETHERADDR);
    F_ARRAY_START(val, -1, customTagL2);
    {
        int idx;
        for (idx = 0; idx < 16; idx++) {
            F_ARRAY_NUMBER(val, customTagL2, idx, GT_U8);
        }
    }
    F_ARRAY_END(val, -1, customTagL2);
    F_NUMBER(val, -1, l2tagSize, GT_U8);
    F_BOOL(val, -1, isIpv6);
    F_UNION(val, -1, ipSrcAddr, GT_IP_ADDR_TYPE_UNT);
    F_UNION(val, -1, ipDstAddr, GT_IP_ADDR_TYPE_UNT);
    F_UNION(val, -1, customTagL3, TGF_TRAFFIC_GEN_L3_IP_CUSTOM_TAG_UNT);
    F_BOOL(val, -1, isTcp);
    F_BOOL(val, -1, isCustomTagL4);
    F_BOOL(val, -1, isCustomTagL3);
    F_NUMBER(val, -1, tcpSrcPort, GT_U16);
    F_NUMBER(val, -1, tcpDstPort, GT_U16);
    F_NUMBER(val, -1, udpSrcPort, GT_U16);
    F_NUMBER(val, -1, udpDstPort, GT_U16);
    F_UNION(val, -1, customTagL4, TGF_TRAFFIC_GEN_L4_CUSTOM_TAG_UNT);
}

void prv_c_to_lua_TGF_TRAFFIC_GEN_PARAMS_STC(
    lua_State *L,
    TGF_TRAFFIC_GEN_PARAMS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ENUM(val, t, pktType, TGF_TRAFFIC_GEN_PKT_TYPE_ENT);
    FO_ENUM(val, t, streamType, TGF_TRAFFIC_GEN_STREAM_TYPE_ENT);
    FO_NUMBER(val, t, pktCnt, GT_U32);
    FO_NUMBER(val, t, port, GT_PORT_NUM);
    FO_NUMBER(val, t, pktSize, GT_U32);
    FO_NUMBER(val, t, vlanId, GT_U16);
    FO_STRUCT(val, t, saMac, GT_ETHERADDR);
    FO_STRUCT(val, t, daMac, GT_ETHERADDR);
    FO_ARRAY_START(val, t, customTagL2);
    {
        int idx;
        for (idx = 0; idx < 16; idx++) {
            FO_ARRAY_NUMBER(val, customTagL2, idx, GT_U8);
        }
    }
    FO_ARRAY_END(val, t, customTagL2);
    FO_NUMBER(val, t, l2tagSize, GT_U8);
    FO_BOOL(val, t, isIpv6);
    FO_UNION(val, t, ipSrcAddr, GT_IP_ADDR_TYPE_UNT);
    FO_UNION(val, t, ipDstAddr, GT_IP_ADDR_TYPE_UNT);
    FO_UNION(val, t, customTagL3, TGF_TRAFFIC_GEN_L3_IP_CUSTOM_TAG_UNT);
    FO_BOOL(val, t, isTcp);
    FO_BOOL(val, t, isCustomTagL4);
    FO_BOOL(val, t, isCustomTagL3);
    FO_NUMBER(val, t, tcpSrcPort, GT_U16);
    FO_NUMBER(val, t, tcpDstPort, GT_U16);
    FO_NUMBER(val, t, udpSrcPort, GT_U16);
    FO_NUMBER(val, t, udpDstPort, GT_U16);
    FO_UNION(val, t, customTagL4, TGF_TRAFFIC_GEN_L4_CUSTOM_TAG_UNT);
    lua_settop(L, t);
}
add_mgm_type(TGF_TRAFFIC_GEN_PARAMS_STC);

add_mgm_enum(TGF_TRAFFIC_GEN_STREAM_TYPE_ENT);
add_mgm_enum(TGF_TRAFFIC_GEN_PKT_TYPE_ENT);

void prv_lua_to_c_CPSS_DXCH_TTI_DIP_VLAN_STC(
    lua_State *L,
    CPSS_DXCH_TTI_DIP_VLAN_STC *val
)
{
    F_BOOL(val, -1, isValid);
    F_ENUM(val, -1, ipType, CPSS_IP_PROTOCOL_STACK_ENT);
    F_UNION(val, -1, destIp, GT_IP_ADDR_TYPE_UNT);
    F_NUMBER(val, -1, prefixLength, GT_U32);
    F_BOOL(val, -1, includeVlanId);
    F_NUMBER(val, -1, vlanId, GT_U16);
}

void prv_c_to_lua_CPSS_DXCH_TTI_DIP_VLAN_STC(
    lua_State *L,
    CPSS_DXCH_TTI_DIP_VLAN_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);

    FO_BOOL(val, t, isValid);
    FO_ENUM(val, t, ipType, CPSS_IP_PROTOCOL_STACK_ENT);
    FO_UNION(val, t, destIp, GT_IP_ADDR_TYPE_UNT);
    FO_NUMBER(val, t, prefixLength, GT_U32);
    FO_BOOL(val, t, includeVlanId);
    FO_NUMBER(val, t, vlanId, GT_U16);
    lua_settop(L, t);
}
add_mgm_type(CPSS_DXCH_TTI_DIP_VLAN_STC);

void prv_lua_to_c_TGF_TRAFFIC_GEN_L3_IP_CUSTOM_TAG_UNT(
    lua_State *L,
    TGF_TRAFFIC_GEN_L3_IP_CUSTOM_TAG_UNT *val
)
{
    F_ARRAY_START(val, -1, ipv4);
    {
        int idx;
        for (idx = 0; idx < 12; idx++) {
            F_ARRAY_NUMBER(val, ipv4, idx, GT_U8);
        }
    }
    F_ARRAY_END(val, -1, ipv4);
    F_ARRAY_START(val, -1, ipv6);
    {
        int idx;
        for (idx = 0; idx < 8; idx++) {
            F_ARRAY_NUMBER(val, ipv6, idx, GT_U8);
        }
    }
    F_ARRAY_END(val, -1, ipv6);
}

void prv_c_to_lua_TGF_TRAFFIC_GEN_L3_IP_CUSTOM_TAG_UNT(
    lua_State *L,
    TGF_TRAFFIC_GEN_L3_IP_CUSTOM_TAG_UNT *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_ARRAY_START(val, t, ipv4);
    {
        int idx;
        for (idx = 0; idx < 12; idx++) {
            FO_ARRAY_NUMBER(val, ipv4, idx, GT_U8);
        }
    }
    FO_ARRAY_END(val, t, ipv4);
    FO_ARRAY_START(val, t, ipv6);
    {
        int idx;
        for (idx = 0; idx < 8; idx++) {
            FO_ARRAY_NUMBER(val, ipv6, idx, GT_U8);
        }
    }
    FO_ARRAY_END(val, t, ipv6);
    lua_settop(L, t);
}
add_mgm_type(TGF_TRAFFIC_GEN_L3_IP_CUSTOM_TAG_UNT);
add_mgm_union(TGF_TRAFFIC_GEN_L3_IP_CUSTOM_TAG_UNT);

/*************************************************************************
* prv_lua_to_c_PRV_CPSS_BOOT_CH_PORT_STATUS_STC
*
* Description:
*       Convert to "c" PRV_CPSS_BOOT_CH_PORT_STATUS_STC
*
* Parameters:
*       value at the top of stack
*
* Returns:
*
*
*************************************************************************/
void prv_lua_to_c_PRV_CPSS_BOOT_CH_PORT_STATUS_STC
(
    lua_State *L,
    PRV_CPSS_BOOT_CH_PORT_STATUS_STC *val
)
{
    F_BOOL(val, -1, isLinkUp);
    F_ENUM(val, -1, ifMode, CPSS_PORT_INTERFACE_MODE_ENT);
    F_ENUM(val, -1, speed, CPSS_PORT_SPEED_ENT);
    F_ENUM(val, -1, fecMode, CPSS_PORT_FEC_MODE_ENT);
    F_BOOL(val, -1, apMode);
}

/*************************************************************************
* prv_c_to_lua_PRV_CPSS_BOOT_CH_PORT_STATUS_STC
*
* Description:
*       Convert PRV_CPSS_BOOT_CH_PORT_STATUS_STC to lua
*
* Parameters:
*       value at the top of stack
*
* Returns:
*       data at the top of stack
*
*************************************************************************/
void prv_c_to_lua_PRV_CPSS_BOOT_CH_PORT_STATUS_STC(
    lua_State *L,
    PRV_CPSS_BOOT_CH_PORT_STATUS_STC *val
)
{
    int t;
    lua_newtable(L);
    t = lua_gettop(L);
    FO_BOOL(val, t, isLinkUp);
    FO_ENUM(val, t, ifMode, CPSS_PORT_INTERFACE_MODE_ENT);
    FO_ENUM(val, t, speed, CPSS_PORT_SPEED_ENT);
    FO_ENUM(val, t, fecMode, CPSS_PORT_FEC_MODE_ENT);
    FO_BOOL(val, t, apMode);
    lua_settop(L, t);
}
add_mgm_type(PRV_CPSS_BOOT_CH_PORT_STATUS_STC);
