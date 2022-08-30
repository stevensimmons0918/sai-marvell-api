/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file prvCpssDxChPacketAnalyzer.c
*
* @brief Private Packet Analyzer APIs for CPSS.
*
* @version   1
********************************************************************************
*/
#include <cpss/dxCh/dxChxGen/packetAnalyzer/private/prvCpssDxChPacketAnalyzer.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/cpssDxChPacketAnalyzerTypes.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/private/prvCpssDxChIdebug.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/diag/private/prvCpssCommonDiag.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**************************** Constants definitions *****************************/

const GT_CHAR_PTR packetAnalyzerFieldToIdebugFieldArr[PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1] = {

    /**@brief Port & Device */
    "local_dev_src_is_trunk",
    "local_dev_src_port",
    "local_dev_src_trunk_id",
    "local_dev_src_eport",
    "local_dev_trg_phy_port",
    "orig_is_trunk",
    "orig_src_dev",
    "orig_src_phy_is_trunk",
    "orig_src_phy_port_or_trunk_id",
    "orig_src_trunk_id",
    "trg_eport",
    "trg_phy_port",

    /**@brief L2 */
    "mac_sa",
    "mac_da",
    "orig_vid",
    "evidx",
    "evlan",
    "ether_type",

    /**@brief L3 */
    "ipv4_sip",
    "ipv4_dip",
    "ipv6_sip",
    "ipv6_dip",
    "ipx_protocol",
    "dscp",

    /**@brief MetaData */
    "byte_count",
    "is_ip",
    "is_ipv4",
    "is_ipv6",
    "ip_legal",
    "ipm",
    "l4_valid",
    "mac_to_me",
    "packet_cmd",
    "queue_port",
    "queue_priority",
    "use_vidx",
    "egress_filter_drop",
    "egress_filter_en",
    "egress_filter_registered",
    "cpu_code",
    "marvell_tagged",
    "marvell_tagged_extended",
    "bypass_bridge",
    "bypass_ingress_pipe",
    "tunnel_start",
    "tunnel_terminated",
    "egress_packet_cmd",
    "packet_trace",
    "outgoing_mtag_cmd",
    "queue_offset"
};

const char* paStageStrArr[PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E+1] = {

    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_1_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_2_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_3_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_4_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_5_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_6_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_7_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_8_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_9_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_10_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_11_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_12_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_13_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_14_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_15_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_16_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_17_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_18_E",
    "CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_19_E"
};

const char* paFieldStrArr[PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1] = {

    "CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_IS_TRUNK_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_PORT_OR_TRUNK_ID_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_TRUNK_ID_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_EPORT_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_TRG_PHY_PORT_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_IS_TRUNK_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_DEV_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_PHY_IS_TRUNK_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_PHY_PORT_OR_TRUNK_ID_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_TRUNK_ID_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_TRG_EPORT_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_TRG_PHY_PORT_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_VID_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_EVIDX_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_EVLAN_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_ETHER_TYPE_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_DIP_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_DIP_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_IPX_PROTOCOL_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_DSCP_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IPV4_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IPV6_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_IP_LEGAL_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_IPM_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_L4_VALID_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_TO_ME_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_QUEUE_PORT_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_QUEUE_PRIORITY_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_USE_VIDX_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_EGRESS_FILTER_DROP_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_EGRESS_FILTER_EN_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_EGRESS_FILTER_REGISTERED_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_CPU_CODE_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_MARVELL_TAGGED_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_MARVELL_TAGGED_EXTENDED_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_BYPASS_BRIDGE_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_BYPASS_INGRESS_PIPE_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_TUNNEL_START_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_TUNNEL_TERMINATED_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_EGRESS_PACKET_CMD_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_TRACE_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_OUTGOING_MTAG_CMD_E",
    "CPSS_DXCH_PACKET_ANALYZER_FIELD_QUEUE_OFFSET_E"
};

const CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT AldrinPipelineOrderStage [PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_ALDRIN_CNS] = {

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E
};

const CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT Aldrin2PipelineOrderStage [PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_ALDRIN2_CNS] = {

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E
};

const CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT FalconPipelineOrderStage  [PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_FALCON_CNS] = {

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E

};

const CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT HawkPipelineOrderStage  [PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_HAWK_CNS] = {

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E
};

const CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT PhoenixPipelineOrderStage  [PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_PHOENIX_CNS] = {

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E
};

const CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT HarrierPipelineOrderStage  [PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_HARRIER_CNS] = {

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E,
    /*CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E,*/
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E/*,*/
    /*CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E*/
};

#define PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(_manager)                                   \
        PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.idebugDir.packetAnalyzerSrc.paMngDB[_manager])

/* check that the managerId was initialized */
#define PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId)                                         \
    if (PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId) == NULL)                         \
    {                                                                                                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);                                      \
    }                                                                                                       \
    if (PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->numOfDevices == 0)              \
    {                                                                                                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);                                      \
    }

#define PRV_CPSS_PA_NUM_OF_TILE_GET_MAC(_devNum,_numberOfTiles)\
    do\
    {\
     if(PRV_CPSS_SIP_6_CHECK_MAC(_devNum))\
     {\
        switch(PRV_CPSS_PP_MAC(_devNum)->multiPipe.numOfTiles)\
        {\
            case 0:\
            case 1:\
                _numberOfTiles = 1;\
                break;\
           case 2:\
           case 4:\
                _numberOfTiles =PRV_CPSS_PP_MAC(_devNum)->multiPipe.numOfTiles;\
                break;\
            default:\
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ",PRV_CPSS_PP_MAC(_devNum)->multiPipe.numOfTiles);\
                break;\
        }\
      }\
      else\
      {\
        _numberOfTiles =1;\
      }\
     }while(0);

/**
* @internal prvCpssDxChPacketAnalyzerDbDeviceGet function
* @endinternal
*
* @brief   The function returns pointer to devNum in packet
*          analyzer manger DB.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId      - manager identification
*                             (APPLICABLE RANGES: 1..10)
* @param[in]  devNum        - device number
* @param[out] paDevDbPtr    - (pointer to) PA device structure
*
* @retval GT_OK             - on success
* @retval GT_NOT_FOUND      - on not found parameters
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
* @retval GT_BAD_PTR        - on NULL pointer value
*/
GT_STATUS prvCpssDxChPacketAnalyzerDbDeviceGet
(
    IN  GT_U32                                      managerId,
    IN  GT_U8                                       devNum,
    OUT PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR       *paDevDbPtr
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR   paMngDbPtr;
    GT_U32                                  i;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    CPSS_NULL_PTR_CHECK_MAC(paDevDbPtr);

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* check that the device was added to bitmap */
    if (PRV_CPSS_PACKET_ANALYZER_MNG_IS_DEV_BMP_SET_MAC(paMngDbPtr->devsBitmap, devNum))
    {
        for (i=0; i<paMngDbPtr->numOfDevices; i++)
        {
            if (paMngDbPtr->paDevPtrArr[i]->paDevId == devNum){
                *paDevDbPtr = paMngDbPtr->paDevPtrArr[i];
                if (paDevDbPtr == NULL)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: wrong managerId [%d] for devNum [%d]",managerId,devNum);
                return GT_OK;
            }
        }

        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "Error: didn't find managerId [%d] for devNum [%d]",managerId,devNum);
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "Error: devNum [%d] wasn't added to managerId [%d]",devNum,managerId);
}

/**
* @internal prvCpssDxChPacketAnalyzerDbManagerGet function
* @endinternal
*
* @brief   The function returns pointer to manager in packet
*          analyzer manger DB.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId      - manager identification
*                             (APPLICABLE RANGES: 1..10)
* @param[out] paMngDbPtr    - (pointer to) PA manager structure
*
* @retval GT_OK             - on success
* @retval GT_NOT_FOUND      - on not found parameters
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
* @retval GT_BAD_PTR        - on NULL pointer value
*/
GT_STATUS prvCpssDxChPacketAnalyzerDbManagerGet
(
    IN  GT_U32                                      managerId,
    OUT PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       *paMngDbPtr
)
{
    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    CPSS_NULL_PTR_CHECK_MAC(paMngDbPtr);

    *paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    return GT_OK;
}

/**
* @internal prvCpssDxChPacketAnalyzerDbActionGet function
* @endinternal
*
* @brief   The function returns pointer to action in packet
*          analyzer manger DB.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId      - manager identification
*                             (APPLICABLE RANGES: 1..10)
* @param[in] actionId       - rule action identification
*                             (APPLICABLE RANGES:1..128)
* @param[out] paActDbPtr    - (pointer to) PA action structure
*
* @retval GT_OK             - on success
* @retval GT_NOT_FOUND      - on not found parameters
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
* @retval GT_BAD_PTR        - on NULL pointer value
*/
GT_STATUS prvCpssDxChPacketAnalyzerDbActionGet
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      actionId,
    OUT PRV_CPSS_DXCH_PACKET_ANALYZER_ACT_PTR       *paActDbPtr
)
{
    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_ACTION_ID_CHECK_MAC(actionId);
    CPSS_NULL_PTR_CHECK_MAC(paActDbPtr);

    if (PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paActPtrArr[actionId] == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    *paActDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paActPtrArr[actionId];

    return GT_OK;
}

/**
* @internal prvCpssDxChPacketAnalyzerDbKeyGet function
* @endinternal
*
* @brief   The function returns pointer to key in packet analyzer manger DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  managerId     - manager identification
*                             (APPLICABLE RANGES: 1..10)
* @param[in]  devNum        - device number
* @param[in]  keyId         - logical key identification
*                             (APPLICABLE RANGES: 1..1024)
* @param[out] paKeyDbPtr    - (pointer to) PA key in DB
*
* @retval GT_OK             - on success
* @retval GT_NOT_FOUND      - on not found parameters
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
* @retval GT_BAD_PTR        - on NULL pointer value
*/
GT_STATUS prvCpssDxChPacketAnalyzerDbKeyGet
(
    IN  GT_U32                                          managerId,
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          keyId,
    OUT PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR           *paKeyDbPtr
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR               paMngDbPtr = NULL;
    GT_U32                                              i;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_KEY_ID_CHECK_MAC(keyId);
    CPSS_NULL_PTR_CHECK_MAC(paKeyDbPtr);

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    if (PRV_CPSS_PACKET_ANALYZER_MNG_IS_KEY_BMP_SET_MAC(paMngDbPtr->keysBitmap, keyId))
    {
        for (i=1; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_LOGICAL_KEY_NUM_CNS; i++)
        {
            /* if this key is not defined continue to the next key */
            if (paMngDbPtr->paKeyPtrArr[i] == NULL)
                continue;

            if (paMngDbPtr->paKeyPtrArr[i]->paKeyId == keyId)
            {
                *paKeyDbPtr = paMngDbPtr->paKeyPtrArr[i];
                if (paKeyDbPtr == NULL)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: failed to get keyId [%d] of devNum [%d]",keyId,devNum);
                return GT_OK;
            }
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChPacketAnalyzerDbRuleGet function
* @endinternal
*
* @brief   The function returns pointer to rule in packet analyzer group DB
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  managerId      - manager identification
*                             (APPLICABLE RANGES: 1..10)
* @param[in]  ruleId         - rule identification
*                             (APPLICABLE RANGES: 1..0xFFFFFFFF)
* @param[in]  groupId        - group identification
*                             (APPLICABLE RANGES: 1..1024)
* @param[out] paRuleDbPtr    - (pointer to) PA rule in DB
*
* @retval GT_OK             - on success
* @retval GT_NOT_FOUND      - on not found parameters
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
* @retval GT_BAD_PTR        - on NULL pointer value
*/
GT_STATUS prvCpssDxChPacketAnalyzerDbRuleGet
(
    IN  GT_U32                                          managerId,
    IN  GT_U32                                          ruleId,
    IN  GT_U32                                          groupId,
    OUT PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_PTR          *paRuleDbPtr
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_STC               *paGrpDbPtr = NULL;
    GT_U32                                              i;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_RULE_ID_CHECK_MAC(ruleId);
    PRV_CPSS_PACKET_ANALYZER_GROUP_ID_CHECK_MAC(groupId);
    CPSS_NULL_PTR_CHECK_MAC(paRuleDbPtr);

    paGrpDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->paGrpPtrArr[groupId];
    if (paGrpDbPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "Error: groupId [%d] doesn't exist",groupId);

    for (i=1; i<=paGrpDbPtr->numOfRules; i++)
    {
        if (paGrpDbPtr->paRulePtrArr[i]->paRuleId == ruleId)
        {
            *paRuleDbPtr = paGrpDbPtr->paRulePtrArr[i];
            if (*paRuleDbPtr == NULL)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: ruleId [%d] in groupId [%d] doesn't exist",ruleId,groupId);
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "Error: ruleId [%d] in groupId [%d] doesn't exist",ruleId,groupId);
}

/**
* @internal prvCpssDxChPacketAnalyzerDbStageGet function
* @endinternal
*
* @brief   The function returns pointer to stage in packet
*          analyzer manger DB.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  managerId     - manager identification
*                             (APPLICABLE RANGES: 1..10)
* @param[in]  devNum        - device number
* @param[in]  stageId       - stage identification
* @param[out] stagePtr      - (pointer to) PA stage
*
* @retval GT_OK             - on success
* @retval GT_FAIL           - on error
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
*/
GT_STATUS prvCpssDxChPacketAnalyzerDbStageGet
(
    IN    GT_U32                                            managerId,
    IN    GT_U8                                             devNum,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT       stageId,
    OUT   PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC          **stagePtr
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR                   paDevDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC                *stageDbPtr = NULL;
    GT_STATUS                                               rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);


    /* get pointer to device in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbDeviceGet(managerId,devNum,&paDevDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    stageDbPtr = &(paDevDbPtr->stagesArr[stageId]);
    if (stageDbPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: failed to get stageId [%s] of devNum [%d]",paStageStrArr[stageId],devNum);

    *stagePtr = stageDbPtr;

    return GT_OK;
}

/**
* @internal prvCpssDxChPacketAnalyzerDbDevGet function
* @endinternal
*
* @brief   The function returns pointer to device number
*          attached to key.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in]  managerId      - manager identification
*                             (APPLICABLE RANGES: 1..10)
* @param[in]  keyId          - logical key identification
*                             (APPLICABLE RANGES: 1..1024)
* @param[out] devNumPtr      - (pointer to) device number
*
* @retval GT_OK             - on success
* @retval GT_FAIL           - on error
* @retval GT_NOT_FOUND      - on not found parameters
*/
GT_STATUS prvCpssDxChPacketAnalyzerDbDevGet
(
    IN  GT_U32                               managerId,
    IN  GT_U32                               keyId,
    OUT  GT_U8                               *devNumPtr
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    GT_U8                                       devNum;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_KEY_ID_CHECK_MAC(keyId);
    CPSS_NULL_PTR_CHECK_MAC(devNumPtr);

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);
    if (PRV_CPSS_PACKET_ANALYZER_MNG_IS_DEV_BMP_SET_MAC(paMngDbPtr->keysBitmap, keyId))
    {
        *devNumPtr = devNum;
        return GT_OK;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "Error: failed to get devNum of keyId [%d]",keyId);
}

/**
* @internal prvCpssDxChPacketAnalyzerDbManagerValidCheck function
* @endinternal
*
* @brief   The function checks if manager id was initialized.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId      - manager identification
*                             (APPLICABLE RANGES: 1..10)
*
* @retval status            - status of manager id
*/
GT_STATUS prvCpssDxChPacketAnalyzerDbManagerValidCheck
(
    IN  GT_U32                               managerId
)
{
    if (managerId == 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: managerId range [1..%d]",PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_MANAGERS_NUM_CNS);

    if (PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId) == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "Error: managerId [%d] doesn't exist",managerId);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPacketAnalyzerDfxPowerSaveEnableSet
*           function
* @endinternal
*
* @brief   Enable/disable DFX power save mode for all connected
*          XSB units.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - enable/disable DFX power
*                                       save mode
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChPacketAnalyzerDfxPowerSaveEnableSet
(
    IN  GT_U8                                       devNum,
    IN  GT_BOOL                                     enable
)
{
    GT_U32 regAddr,value,i;
    GT_U32 numberOfTiles,tileOffset;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    value = BOOL2BIT_MAC(enable);

    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        case 0:
        case 1:
            numberOfTiles = 1;
            break;
       case 2:
       case 4:
            numberOfTiles =PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d",PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
            break;

    }

    for(i=0; i<numberOfTiles; i++)
    {
        tileOffset = prvCpssSip6TileOffsetGet(devNum,i/*tileId*/);

        /* XSB clock enable/disable for all connected XSB units */
        regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXClientUnits.clientControl;
        regAddr+=tileOffset;

        /* Set DFX multiinstance data  */
        PRV_CPSS_DFX_MULTI_INSTANCE_DATA_SET_MAC(devNum, CPSS_DIAG_DATA_INTEGRITY_DFX_INSTANCE_TILE_E, i, 0);

        rc = prvCpssDfxClientSetRegField(devNum, 0, PRV_CPSS_DFX_CLIENT_BC_CNS, regAddr, 30, 1, value);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = prvCpssDxChIdebugDbPowerSaveSet(devNum,enable);
    return rc;

}

/**
* @internal prvCpssDxChPacketAnalyzerStageInterfacesSet function
* @endinternal
*
* @brief   Set stage idebug interfaces attributes list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in] stageId                  - stage identification
* @param[in] boundInterface           - bounded interface
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPacketAnalyzerStageInterfacesSet
(
    IN GT_U32                                           managerId,
    IN GT_U8                                            devNum,
    IN CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT      stageId,
    IN PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC boundInterface
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR       paDevDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC    *stageDbPtr = NULL;
    GT_STATUS                                   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_ID_CHECK_MAC(stageId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_VALID_CHECK_MAC(managerId,devNum,stageId);

    /* get pointer to device in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbDeviceGet(managerId,devNum,&paDevDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    stageDbPtr = &(paDevDbPtr->stagesArr[stageId]);
    if (stageDbPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: failed to get stageId [%s] of devNum [%d]",paStageStrArr[stageId],devNum);

    cpssOsMemCpy(&(stageDbPtr->boundInterface), &boundInterface.info,sizeof(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC));
    stageDbPtr->isValid = boundInterface.isValid;

    return GT_OK;

}

/**
* @internal prvCpssDxChPacketAnalyzerStageInterfacesGet function
* @endinternal
*
* @brief   Get stage idebug interfaces attributes list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in] stageId                  - stage identification
* @param[out] boundInterfacePtr       - (pointer to) bounded
*                                       interface
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChPacketAnalyzerStageInterfacesGet
(
    IN    GT_U32                                            managerId,
    IN    GT_U8                                             devNum,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT       stageId,
    OUT   PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC *boundInterfacePtr
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC    *paStageDbPtr = NULL;
    GT_STATUS                                   rc;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_STAGE_ID_CHECK_MAC(stageId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_VALID_CHECK_MAC(managerId,devNum,stageId);
    CPSS_NULL_PTR_CHECK_MAC(boundInterfacePtr);

    cpssOsMemSet(boundInterfacePtr,0,sizeof(PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC));

    /* get pointer to stage in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbStageGet(managerId,devNum,stageId,&paStageDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsMemCpy(&boundInterfacePtr->info, &(paStageDbPtr->boundInterface),sizeof(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC));
    boundInterfacePtr->isValid = paStageDbPtr->isValid;

    return GT_OK;
}

/**** DUMP APIs for packet analyzer *******/

/**
* @internal prvCpssDxChPacketAnalyzerStageFieldsGet function
* @endinternal
*
* @brief   Get stage fields list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in] stageId                  - stage identification
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields for stage
*                                       out: (pointer to) actual
*                                       number of fields for
*                                       stage
* @param[out] fieldsArr               - (pointer to) stage
*                                       fields list array
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPacketAnalyzerStageFieldsGet
(
    IN    GT_U32                                            managerId,
    IN    GT_U8                                             devNum,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT       stageId,
    INOUT GT_U32                                            *numOfFieldsPtr,
    OUT   PRV_CPSS_DXCH_IDEBUG_FIELD_STC                    fieldsArr[]
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC    *stageDbPtr = NULL;
    GT_U32                                      j,k;
    GT_STATUS                                   rc=GT_OK;
    GT_CHAR                                     interface[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS] = {0};
    GT_U32                                      numOfFields,totalNumOfFields = 0;
    PRV_CPSS_DXCH_IDEBUG_FIELD_STC              *fieldsArrPtr = NULL;
    GT_BOOL                                     fieldExists = GT_FALSE;


    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_ID_CHECK_MAC(stageId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_VALID_CHECK_MAC(managerId,devNum,stageId);
    CPSS_NULL_PTR_CHECK_MAC(numOfFieldsPtr);
    CPSS_NULL_PTR_CHECK_MAC(fieldsArr);

    /* get pointer to stage in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbStageGet(managerId,devNum,stageId,&stageDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get number of fields in interface */
    cpssOsStrCpy(interface,stageDbPtr->boundInterface.interfaceId);
    rc = prvCpssDxChIdebugInterfaceNumFieldsGet(devNum,interface,&numOfFields);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (numOfFields > *numOfFieldsPtr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, "Error: num of fields is [%d] for stageId [%s]",numOfFields,paStageStrArr[stageId]);

    /* allocate fields array for interface */
    fieldsArrPtr = (PRV_CPSS_DXCH_IDEBUG_FIELD_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_IDEBUG_FIELD_STC)*numOfFields);
    if (fieldsArrPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);

    cpssOsMemSet(fieldsArrPtr,0,sizeof(PRV_CPSS_DXCH_IDEBUG_FIELD_STC)*numOfFields);

    /* fill fields array for interface */
    rc = prvCpssDxChIdebugInterfaceFieldsGet(devNum,interface,&numOfFields,fieldsArrPtr);
    if (rc != GT_OK)
    {
        cpssOsFree(fieldsArrPtr);
        return rc;
    }

    /* copy without duplication to array */
    for (j=0; j<numOfFields; j++)
    {
        fieldExists = GT_FALSE;
        for (k=0; k<totalNumOfFields; k++)
        {
            if (cpssOsStrCmp(fieldsArr[k].fieldName,fieldsArrPtr[j].fieldName) == 0)
            {
                /* field already exists in array */
                fieldExists = GT_TRUE;
                break;
            }
        }
        /* add the field to the array */
        if (fieldExists == GT_FALSE)
        {
            cpssOsMemCpy(&(fieldsArr[totalNumOfFields]),&(fieldsArrPtr[j]),sizeof(PRV_CPSS_DXCH_IDEBUG_FIELD_STC));
            totalNumOfFields++;
        }
    }

    cpssOsFree(fieldsArrPtr);


    *numOfFieldsPtr = totalNumOfFields;

    return GT_OK;

}

/**
* @internal prvCpssDxChPacketAnalyzerInstanceInterfacesGet function
* @endinternal
*
* @brief   Get instance interfaces list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in] instanceId               - instance identification
* @param[in,out] numOfInterfacesPtr   - in: (pointer to)
*                                       allocated number of
*                                       interfaces for
*                                       instance
*                                       out: (pointer to) actual
*                                       number of interfaces for
*                                       instance
* @param[out] interfacesArr           - (pointer to) instance
*                                       interface list array
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPacketAnalyzerInstanceInterfacesGet
(
    IN    GT_U32                                        managerId,
    IN    GT_U8                                         devNum,
    IN    GT_CHAR_PTR                                   instanceId,
    INOUT GT_U32                                        *numOfInterfacesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC interfacesArr[]
)
{
    mxml_node_t *node;
    const GT_CHAR *attr;
    GT_U32 i=0;
    GT_STATUS    rc;
    GT_CHAR_PTR  muxTokeName;
    GT_BOOL noInterface = GT_FALSE;

     /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_INSTANCE_ID_CHECK_MAC(devNum,instanceId);
    CPSS_NULL_PTR_CHECK_MAC(numOfInterfacesPtr);
    CPSS_NULL_PTR_CHECK_MAC(interfacesArr);

    /* first interface */
    node = mxmlFindInstanceFirstInterfaceNode(devNum,instanceId,&noInterface);
    if (!node){
        /*from sip6_10 instance can be without interfaces and return null */
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            if (noInterface == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: failed to instanceId [%s] in XML", instanceId);
            }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: failed to instanceId [%s] in XML", instanceId);
        }
    }

    while(node != NULL)
    {
        PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)

        if(i > *numOfInterfacesPtr)
        {
            *numOfInterfacesPtr = i;
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, "Error: number of interfaces is [%d] for instanceId [%s]",i,instanceId);
        }
        attr = prvCpssMxmlElementGetAttr(node,"internalName");
        cpssOsStrCpy(interfacesArr[i].interfaceId,attr);
        cpssOsStrCpy(interfacesArr[i].instanceId,instanceId);
        rc = prvCpssDxChIdebugMuxTokenNameGet(devNum,&muxTokeName);
        if (rc != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        attr = prvCpssMxmlElementGetAttr(node,muxTokeName);
        interfacesArr[i].interfaceIndex = cpssOsStrTo32(attr);
        i++;
        node = node->next;
    }

    *numOfInterfacesPtr = i;
    return GT_OK;
}

/**
* @internal prvCpssDxChPacketAnalyzerFieldInterfacesGet function
* @endinternal
*
* @brief   Get field interfaces list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in] fieldName                - field identification
* @param[in,out] numOfInterfacesPtr   - in: (pointer to)
*                                       allocated number of
*                                       interfaces for
*                                       field
*                                       out: (pointer to) actual
*                                       number of interfaces for
*                                       field
* @param[out] interfaceArr            - (pointer to) field
*                                       interface list array
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChPacketAnalyzerFieldInterfacesGet
(
    IN    GT_U32                                  managerId,
    IN    GT_U8                                   devNum,
    IN    GT_CHAR_PTR                             fieldName,
    INOUT GT_U32                                  *numOfInterfacesPtr,
    OUT   GT_CHAR                                 *interfacesArr[]
)
{
    mxml_node_t *node;
    GT_U32 i=0, j=0,numOfInterfaces;
    const GT_CHAR *attr;
    GT_CHAR_PTR *interfacesArrDbPtr;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_IDEBUG_FIELD_ID_CHECK_MAC(devNum,fieldName);
    CPSS_NULL_PTR_CHECK_MAC(numOfInterfacesPtr);
    CPSS_NULL_PTR_CHECK_MAC(interfacesArr);

    /* get pointer to interfaces array in iDebug DB */
    rc = prvCpssDxChIdebugDbInterfaceArrayGet(devNum,&numOfInterfaces,&interfacesArrDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    for(i=0; i<numOfInterfaces; i++)
    {

        node = mxmlFindInterfaceFirstFieldNode(devNum,interfacesArrDbPtr[i]);
        if(node)
        {
            while (node != NULL)
            {
                PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)

                attr = prvCpssMxmlElementGetAttr(node,"internalName");
                if(cpssOsStrStr(attr,fieldName) && (cpssOsStrlen(attr) == cpssOsStrlen(fieldName)))
                {
                    if(j >= *numOfInterfacesPtr)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, "Error: num of interfaces is [%d] for field [%s]",j,fieldName);
                    }

                    cpssOsStrCpy(interfacesArr[j],interfacesArrDbPtr[i]);
                    j++;
                    break;
                }
                node = node->next;
            }
         }
         else
         {
            if(GT_TRUE == prvCpssDxChIdebugFailOnUnfoundToken(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: failed to find interface [%s] in XML",interfacesArrDbPtr[i]);
            }
         }
    }

    *numOfInterfacesPtr = j;

    return GT_OK;
}

/**
* @internal prvCpssDxChPacketAnalyzerInterfacesGet function
* @endinternal
*
* @brief   Get interfaces list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in,out] numOfInterfacesPtr   - in: (pointer to)
*                                       allocated number of
*                                       interfaces
*                                       out: (pointer to) actual
*                                       number of interfaces
* @param[out] interfaceArr            - (pointer to) interface
*                                       list array
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPacketAnalyzerInterfacesGet
(
    IN    GT_U32                                    managerId,
    IN    GT_U8                                     devNum,
    INOUT GT_U32                                    *numOfInterfacesPtr,
    OUT   GT_CHAR                                   **interfacesArr[]
)
{
    GT_U32 numOfInterfaces;
    GT_CHAR_PTR *interfacesArrDbPtr;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    CPSS_NULL_PTR_CHECK_MAC(numOfInterfacesPtr);
    CPSS_NULL_PTR_CHECK_MAC(interfacesArr);

    /* get pointer to interfaces array in iDebug DB */
    rc = prvCpssDxChIdebugDbInterfaceArrayGet(devNum,&numOfInterfaces,&interfacesArrDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (*numOfInterfacesPtr < numOfInterfaces)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, "Error: num of interfaces is [%d]",numOfInterfaces);

    *numOfInterfacesPtr = numOfInterfaces;
    *interfacesArr = interfacesArrDbPtr;
    return GT_OK;
}

/**
* @internal prvCpssDxChPacketAnalyzerInstancesGet function
* @endinternal
*
* @brief   Get instances list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in,out] numOfInstancesPtr    - in: (pointer to)
*                                       allocated number of
*                                       instances
*                                       out: (pointer to) actual
*                                       number of instances
* @param[out] instancesArr            - (pointer to) instance
*                                       list array
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPacketAnalyzerInstancesGet
(
    IN    GT_U32                                    managerId,
    IN    GT_U8                                     devNum,
    INOUT GT_U32                                    *numOfInstancesPtr,
    OUT   GT_CHAR                                   **instancesArr[]
)
{
    GT_U32 numOfInstances;
    GT_CHAR_PTR *instancesArrDbPtr;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    CPSS_NULL_PTR_CHECK_MAC(numOfInstancesPtr);
    CPSS_NULL_PTR_CHECK_MAC(instancesArr);

    /* get pointer to instances array in iDebug DB */
    rc = prvCpssDxChIdebugDbInstanceArrayGet(devNum,&numOfInstances,&instancesArrDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (*numOfInstancesPtr < numOfInstances)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, "Error: num of instances is [%d]",numOfInstances);

    *numOfInstancesPtr = numOfInstances;
    *instancesArr = instancesArrDbPtr;
    return GT_OK;
}

/**
* @internal prvCpssDxChPacketAnalyzerFieldsGet function
* @endinternal
*
* @brief   Get fields list array.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in,out] numOfFieldsPtr       - in: (pointer to)
*                                       allocated number of
*                                       fields
*                                       out: (pointer to) actual
*                                       number of fields
* @param[out] fieldsArr               - (pointer to) fields
*                                       list array
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPacketAnalyzerFieldsGet
(
    IN    GT_U32                                    managerId,
    IN    GT_U8                                     devNum,
    INOUT GT_U32                                    *numOfFieldsPtr,
    OUT   GT_CHAR                                   **fieldsArr[]
)
{
    GT_U32 numOfFields;
    GT_CHAR_PTR *fieldsArrDbPtr;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    CPSS_NULL_PTR_CHECK_MAC(numOfFieldsPtr);
    CPSS_NULL_PTR_CHECK_MAC(fieldsArr);

    /* get pointer to instances array in iDebug DB */
    rc = prvCpssDxChIdebugDbFieldsArrayGet(devNum,&numOfFields,&fieldsArrDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (*numOfFieldsPtr < numOfFields)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, "Error: num of fields is [%d]",numOfFields);

    *numOfFieldsPtr = numOfFields;
    *fieldsArr = fieldsArrDbPtr;
    return GT_OK;
}

/**
* @internal prvCpssDxChPacketAnalyzerInterfaceStatusGet function
* @endinternal
*
* @brief   Check if Interface is bounded to a stage.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface identification
* @param[out] isBoundedPtr            - (pointer to) whether
*                                       interface is bounded to
*                                       a stage
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChPacketAnalyzerInterfaceStatusGet
(
    IN  GT_U32                                      managerId,
    IN  GT_U8                                       devNum,
    IN  GT_CHAR_PTR                                 interfaceName,
    OUT GT_BOOL                                     *isBoundedPtr
)
{
    GT_U32                                    i;
    GT_STATUS                                 rc = GT_TRUE;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC  *stageDbPtr = NULL;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT stageId;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(isBoundedPtr);

    for (i=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_FIRST_E; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E; i++)
    {
        stageId = i;

        /* get pointer to stage in packet analyzer manager DB */
        rc = prvCpssDxChPacketAnalyzerDbStageGet(managerId,devNum,stageId,&stageDbPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (cpssOsStrCmp(interfaceName,stageDbPtr->boundInterface.interfaceId) == 0)
        {
            if (stageId >= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E)
            {
                rc = prvCpssDxChPacketAnalyzerUserDefinedValidCheck(managerId,stageId);
                if (rc == GT_OK)
                {
                    *isBoundedPtr = GT_TRUE;
                    return GT_OK;
                }
            }
            else
            {
                *isBoundedPtr = GT_TRUE;
                return GT_OK;
            }
        }
    }

    *isBoundedPtr = GT_FALSE;
    return GT_OK;
}

/**
* @internal prvCpssDxChPacketAnalyzerInterfaceInfoGet function
* @endinternal
*
* @brief   Get interface stage and instance.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in] interfaceName            - interface identification
* @param[out] stageIdPtr              - (pointer to) stage
*                                       identification
* @param[out] instanceIdPtr           - (pointer to) instance
*                                       identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on invalid parameter
*/
GT_STATUS prvCpssDxChPacketAnalyzerInterfaceInfoGet
(
    IN    GT_U32                                        managerId,
    IN    GT_U8                                         devNum,
    IN    GT_CHAR_PTR                                   interfaceName,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   *stageIdPtr,
    OUT   GT_CHAR                                       *instanceIdPtr
)
{
    GT_U32 instanceDfxPipeId, instanceDfxBaseAddr, interfaceDfxIndex,id,i;
    GT_CHAR instanceId[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    mxml_node_t *node;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR         paMngDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR         paDevDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_PTR      stageDbPtr = NULL;
    GT_STATUS rc;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stage;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);
    CPSS_NULL_PTR_CHECK_MAC(stageIdPtr);
    CPSS_NULL_PTR_CHECK_MAC(instanceIdPtr);

    node = mxmlInstanceInfoGet(devNum,interfaceName,0/*do not care if this is multiple,return first*/,instanceId,&instanceDfxPipeId,&instanceDfxBaseAddr,&interfaceDfxIndex,NULL);
    if (!node)
    {
        *stageIdPtr = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OK, "Warning: failed to find instance info for interface [%s] in XML",interfaceName);
    }


    cpssOsStrCpy(instanceIdPtr,instanceId);

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* get pointer to device in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbDeviceGet(managerId,devNum,&paDevDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

        /* try to find interface in bounded array */
        for (i=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_FIRST_E; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E; i++)
        {
            if(prvCpssDxChPacketAnalyzerIsStageApplicableInDev(devNum,i) == GT_FALSE)
                continue;

            stage = i;

            /* get pointer to stage in packet analyzer manager DB */
            rc = prvCpssDxChPacketAnalyzerDbStageGet(managerId,devNum,stage,&stageDbPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            if ((cpssOsStrCmp(interfaceName,stageDbPtr->boundInterface.interfaceId) == 0) )
            {
                *stageIdPtr = stage;
                return GT_OK;
            }
        }

    /* try to find interface in uds array */
    for (i=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E; i<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E; i++)
    {
        stage = i;

        /* get pointer to stage in packet analyzer manager DB */
        rc = prvCpssDxChPacketAnalyzerDbStageGet(managerId,devNum,stage,&stageDbPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        id = stage - (PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1);

        if ((cpssOsStrCmp(interfaceName,stageDbPtr->boundInterface.interfaceId) == 0) && (paMngDbPtr->udsArr[id].valid == GT_TRUE))
        {
            *stageIdPtr = stage;
            return GT_OK;
        }
    }

    node = mxmlInterfaceStageGet(devNum,interfaceName,stageIdPtr);
    if (!node)
    {
        *stageIdPtr = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E;
        return GT_OK;
    }

    stageDbPtr = &(paDevDbPtr->stagesArr[*stageIdPtr]);
    if (stageDbPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: failed to find stage [%s] in DB",paStageStrArr[*stageIdPtr]);

    if (cpssOsStrCmp(stageDbPtr->boundInterface.interfaceId,interfaceName) == 0)
    {
        return GT_OK;
    }

    *stageIdPtr = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E;
    return GT_OK;
}

/**
* @internal prvCpssDxChPacketAnalyzerUserDefinedValidCheck
*           function
* @endinternal
*
* @brief   Get packet analyzer user defined stage validity.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] udsId                    - uds identification
*
* @retval GT_OK                    - on valid
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_FAIL                  - on non valid
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_NOT_FOUND             - on not found parameters
* @retval GT_ALREADY_EXIST         - on already initialized
*/
GT_STATUS prvCpssDxChPacketAnalyzerUserDefinedValidCheck
(
    IN GT_U32                                           managerId,
    IN CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT      udsId
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;
    GT_U8                                       devNum;
    GT_U32                                      id;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    if (udsId < PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E || udsId > PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: uds index [%d] is not in range ",udsId);

    id = udsId - (PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1);
    if (paMngDbPtr->udsArr[id].valid == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    return GT_OK;
}

/**
* @internal prvCpssDxChPacketAnalyzerFieldSizeGet function
* @endinternal
*
* @brief   Get packet analyzer field's size.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in] fieldName                - field name
* @param[out] lengthPtr               - (pointer to) fields
*                                       length in bits
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPacketAnalyzerFieldSizeGet
(
    IN  GT_U32                                  managerId,
    IN  GT_U8                                   devNum,
    IN  GT_CHAR_PTR                             fieldName,
    OUT GT_U32                                  *lengthPtr
)
{
    mxml_node_t *node;
    GT_U32 i=0, startBit, endBit;
    const GT_CHAR *attr;
    GT_CHAR_PTR *interfacesArrDbPtr;
    GT_U32 numOfInterfaces;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_IDEBUG_FIELD_ID_CHECK_MAC(devNum,fieldName);
    CPSS_NULL_PTR_CHECK_MAC(lengthPtr);

    /* get pointer to interfaces array in iDebug DB */
    rc = prvCpssDxChIdebugDbInterfaceArrayGet(devNum,&numOfInterfaces,&interfacesArrDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    for(i=0; i<numOfInterfaces; i++)
    {
        node = mxmlFindInterfaceFirstFieldNode(devNum,interfacesArrDbPtr[i]);
        if(!node)
        {   if(GT_TRUE==prvCpssDxChIdebugFailOnUnfoundToken(devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: failed to find interface [%s] in XML",interfacesArrDbPtr[i]);
            }
        }

        while (node != NULL)
        {
            PRV_CPSS_IDEBUG_SKIP_NON_MXML_ELEMENT(node)

            attr = prvCpssMxmlElementGetAttr(node,"internalName");
            if(cpssOsStrStr(attr,fieldName) && (cpssOsStrlen(attr) == cpssOsStrlen(fieldName)))
            {
                attr = prvCpssMxmlElementGetAttr(node,"bitStop");
                endBit = cpssOsStrTo32(attr);
                attr = prvCpssMxmlElementGetAttr(node,"bitStart");
                startBit= cpssOsStrTo32(attr);
                *lengthPtr = endBit - startBit + 1;

                return GT_OK;
            }
            node = node->next;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
}

GT_STATUS prvCpssDxChIdebugInterfaceListDump
(
    IN  GT_U32                                  managerId,
    IN  GT_U8                                   devNum
)
{
    GT_U32                                      i;
    GT_STATUS                                   rc;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC    *stagePtr = NULL;

    for(i=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_FIRST_E;i<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E;i++)
    {

        rc = prvCpssDxChPacketAnalyzerDbStageGet(managerId,devNum,i,&stagePtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        cpssOsPrintf("Interface 0 is %s \n",stagePtr->boundInterface.interfaceId);
    }
    return GT_OK;
}

/* Prevent warning - return GT_TRUE in case of simulation run */
GT_BOOL paIsSimulation
(
    GT_VOID
)
{
    GT_BOOL ans = GT_FALSE;

#ifdef GM_USED
    ans=GT_TRUE;
#endif
    return ans;
}

GT_STATUS prvCpssDxChPacketAnalyzerMarkMuxedStagesSet
(
    IN  GT_U32                                      managerId,
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      numOfStages,
    IN  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT *stagesArr,
    OUT GT_U32                                      *muxValidBmpPtr
)
{
    GT_U32 i,k;
    GT_32 j;
    GT_U32                                       *activeMuxNum;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT  currentStage;
    GT_CHAR                                      currentBus[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS] = {0}; /* instanceId */
    GT_U32                                       mux;                                                         /* interfaceIndex */
    GT_STATUS                                    rc;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC     *stageDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR        paDevDbPtr = NULL;

    GT_U32 numOfInstances;
    GT_CHAR_PTR *instancesArrDbPtr;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC boundInterfaceGet;

    CPSS_NULL_PTR_CHECK_MAC(stagesArr);
    CPSS_NULL_PTR_CHECK_MAC(muxValidBmpPtr);

    /* get pointer to device in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbDeviceGet(managerId,devNum,&paDevDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get pointer to instances array in iDebug DB */
    rc = prvCpssDxChIdebugDbInstanceArrayGet(devNum,&numOfInstances,&instancesArrDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    activeMuxNum = (GT_U32*)cpssOsMalloc(sizeof(GT_U32)*numOfInstances);
    if (activeMuxNum == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    *muxValidBmpPtr = 0xFFFFFFFF;/*all valid  ,not valid stages will be elliminated later*/

    for(i=0;i<numOfInstances;i++)
    {
       activeMuxNum[i] = 0;
    }

    /*chek mux*/
    for(i=0;i<numOfStages;i++)
    {
        currentStage = stagesArr[i];

        if ((currentStage >= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E) && (currentStage <= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E ))
            if (prvCpssDxChPacketAnalyzerUserDefinedValidCheck(managerId,currentStage) != GT_OK)
                continue;

        if(prvCpssDxChPacketAnalyzerIsStageApplicableInDev(devNum,currentStage) == GT_FALSE)
        {
            (*muxValidBmpPtr)&=(~(1<<i));
            continue;
        }

        rc = prvCpssDxChPacketAnalyzerStageInterfacesGet(managerId,devNum,currentStage,&boundInterfaceGet);
        if (rc != GT_OK) {
            return rc;
        }

        stageDbPtr = &(paDevDbPtr->stagesArr[currentStage]);
        if (stageDbPtr == NULL)
        {
            if (currentStage <= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_19_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: failed to get stageId [%s] of devNum [%d]", paStageStrArr[currentStage], devNum);
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: failed to get stageId [%d] of devNum [%d]", currentStage, devNum);
            }
        }

        if(GT_TRUE == stageDbPtr->isValid)
        {
            cpssOsStrCpy(currentBus,boundInterfaceGet.info.instanceId);
            for (j=0; j<(GT_32)numOfInstances; j++)
            {
                if (cpssOsStrCmp(currentBus,instancesArrDbPtr[j]) == 0)
                    break;

            }
            activeMuxNum[j]++;
        }
        else
        {
            (*muxValidBmpPtr)&=(~(1<<i));
        }
    }

    for(i=0;i<numOfInstances;i++)
    {
        if(activeMuxNum[i]>1)
        {
            for(j=7;j>=0;j--)
            {
                rc = prvCpssDxChIdebugSelectInerfaceByPriority(devNum,instancesArrDbPtr[i],j,&mux);
                if(rc!=GT_OK)
                {
                    return rc;
                }

                /*check if present*/
                for(k=0;k<numOfStages;k++)
                {
                    currentStage = stagesArr[k];
                    if(prvCpssDxChPacketAnalyzerIsStageApplicableInDev(devNum,currentStage) == GT_FALSE)
                        continue;

                    rc = prvCpssDxChPacketAnalyzerStageInterfacesGet(managerId,devNum,currentStage,&boundInterfaceGet);

                    cpssOsStrCpy(currentBus,boundInterfaceGet.info.instanceId);
                    if (cpssOsStrCmp(currentBus,instancesArrDbPtr[i]) == 0)
                    {
                        /*winner found,winner = k*/
                        if(mux == boundInterfaceGet.info.interfaceIndex)
                        {
                            break;
                        }
                    }
                 }

                 if(k==numOfStages)
                 {
                    /*there is no one in this priority*/
                    continue;
                 }

                 /*disable loosers*/
                for(k=0;k<numOfStages;k++)
                {
                    currentStage = stagesArr[k];
                    if(prvCpssDxChPacketAnalyzerIsStageApplicableInDev(devNum,currentStage) == GT_FALSE)
                        continue;

                    rc = prvCpssDxChPacketAnalyzerStageInterfacesGet(managerId,devNum,currentStage,&boundInterfaceGet);
                    cpssOsStrCpy(currentBus,boundInterfaceGet.info.instanceId);
                    if (cpssOsStrCmp(currentBus,instancesArrDbPtr[i]) == 0)
                    {
                        if(mux!=boundInterfaceGet.info.interfaceIndex)
                        {
                            (*muxValidBmpPtr)&=(~(1<<k));
                        }
                    }
                 }
                 break;
            }

        }
    }

    cpssOsFree(activeMuxNum);
    return GT_OK;
}

/**
 * @internal
 *           prvCpssDxChPacketAnalyzerStageValiditySet function
 * @endinternal
 *
 * @brief   The function set stage validity in case of mux
 *          between interfaces.
 *
 * @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
 *
 * @param[in] managerId                - manager identification
 *                                       (APPLICABLE RANGES: 1..10)
 * @param[in] devNum                   - device number
 * @param[in] stageId                  - stage identification
 * @param[in] isValid                  - interface validity
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong value in any of the parameters
 * @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
 * @retval GT_BAD_PTR               - on NULL pointer value.
 * @retval GT_FAIL                  - on error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_BAD_STATE             - on invalid parameter
 */
GT_STATUS prvCpssDxChPacketAnalyzerStageValiditySet
(
    IN  GT_U32                                          managerId,
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     stageId,
    IN  GT_BOOL                                         isValid
)
{
    GT_STATUS                                       rc = GT_OK;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC        *stageDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR           paDevDbPtr = NULL;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_ID_CHECK_MAC(stageId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_VALID_CHECK_MAC(managerId,devNum,stageId);

    /* get pointer to device in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbDeviceGet(managerId,devNum,&paDevDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    stageDbPtr = &(paDevDbPtr->stagesArr[stageId]);
    if (stageDbPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: failed to get stageId [%s] of devNum [%d]",paStageStrArr[stageId],devNum);


    stageDbPtr->isValid = isValid;
    return rc;
}

static const PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC     aldrinFixedBoundInterfaceArr[PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1] =
{
    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E */
    {{"rxdma0_ctrl_pipe_and_nextct","RXDMA_2_CTRL_PIPE",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E */
    {{"tti_desc_lower","TTI_2_PCL_LOWER_DESC",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E */
    {{"pcl_l2i_desc","pcl2l2i_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E */
    {{"pcl_l2i_desc","l2i2ipvx_desc",1},GT_FALSE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E */
    {{"ipvx2ioam_desc","ipvx_2_ioam_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E */
    {{"upper_ingress_desc_push","iplr02iplr1_desc",2},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E */
    {{"upper_ingress_desc_push","iplr12mll_desc",3},GT_FALSE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E */
    {{"upper_ingress_desc_push","mll2eq_desc",0},GT_FALSE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E */
    {{"upper_ingress_desc_push","eq2egf_desc",4},GT_FALSE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E */
    {{"egf_desc_push","egf2txq_q_desc",4},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E */
    {{"txdma0_desc","txdma2ha_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E */
    {{"egress_desc","epcl2tmqmap_desc",1},GT_FALSE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E */
    {{"egress_desc","ha2ekgen_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E */
    {{"egress_desc","eoam2eplr_desc",3},GT_FALSE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E */
    {{"egress_desc","eplr2tmdrop_desc",4},GT_FALSE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E */
    {{"egress_desc","ermrk2txfifo_desc",6},GT_FALSE}};


static const PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC     aldrin2FixedBoundInterfaceArr[PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1] =
{
    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E */
    {{"rxdma_ctrl_pipe_and_mppm_0_0_21","rxdma2ctrl_pipe_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E */
    {{"tti_desc_0_5","tti2pcl_desc_low",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E */
    {{"pcl_desc_0_19","pcl2l2i_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E */
    {{"l2i_ipvx_desc_or_debug_buses_0_8","l2i2ipvx_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E */
    {{"l2i_ipvx_desc_or_debug_buses_0_8","ipvx_2_ioam_desc",3},GT_FALSE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E */
    {{"iplr_ioam_desc_0_9","iplr02iplr1_desc",1},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E */
    {{"iplr_ioam_desc_0_9","iplr12mll_desc",2},GT_FALSE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E */
    {{"mll_eq_desc_0_10","mll2eq_desc",0},GT_FALSE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E */
    {{"mll_eq_desc_0_10","eq2eft_desc",1},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E */
    {{"qag2txq_bus_0_18","qag2txq_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E */
    {{"ha_desc_and_header_0_20","txdma2ha_desc",1},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E */
    {{"egress_desc_or_hdr_1_3","tmqmap2eoam_desc",5},GT_FALSE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E */
    {{"ha2ermrk_or_debug_bus_0_20","ha2ekgen_desc",4},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E */
    {{"egress_desc_or_hdr_1_3","eoam2eplr_desc",4},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E */
    {{"egress_desc_or_hdr_1_3","eplr2tmdrop_desc",3},GT_FALSE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E */
    {{"egress_desc_or_hdr_1_3","ermrk2td_desc",1},GT_FALSE}};

static const PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC     falconFixedBoundInterfaceArr[PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1] =
{
    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E */
    {{"tile0_pipe0_eagle_ia_0_macro_hbu2tti","hbuing2tti_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E */
    {{"tile0_pipe0_eagle_tti_0_macro_tti_desc","tti2pcl_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E */
    {{"tile0_pipe0_eagle_pcl_0_macro_pcl_desc","pcl2l2i_desc" ,7},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E */
    {{"tile0_pipe0_eagle_l2i_ipvx_0_macro_ipvx_desc","l2i2ipvx_desc",3},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E */
    {{"tile0_pipe0_eagle_l2i_ipvx_0_macro_ioam_desc","ipvx2ioam_desc",2},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E */
    {{"tile0_pipe0_eagle_upper_ing_0_macro_iplr_desc","iplr02iplr1_desc",1},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E */
    {{"tile0_pipe0_eagle_upper_ing_0_macro_mll_eq_desc","iplr12mll_desc",0},GT_FALSE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E */
    {{"tile0_pipe0_eagle_upper_ing_0_macro_mll_eq_desc","mll2eq_desc",1},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E */
    {{"tile0_pipe0_eagle_upper_ing_0_macro_eft_desc","eq2eft_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E */
    {{"tile0_pipe0_eagle_ia_0_macro_hbu2tti","hbuegr2iaegr_desc",3},GT_FALSE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E */
    {{"tile0_pipe0_eagle_egf_0_macro_sht_qag","qag2ha_desc",2},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E */
    {{"tile0_pipe0_eagle_epcl_0_macro_epcl_desc","preq2eoam_desc",1},GT_FALSE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E */
    {{"tile0_pipe0_eagle_ha_0_macro_ha_desc","ha2ekgen_desc",1},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E */
    {{"tile0_pipe0_eagle_epcl_0_macro_epcl_desc","eoam2eplr_desc",2},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E */
    {{"tile0_pipe0_eagle_epcl_0_macro_epcl_desc","eplr2psu_desc",3},GT_FALSE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E */
    {{"tile0_pipe0_txqs_macro_i0_pipe0_2","pdx2pdsi_desc",1},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E */
    {{"tile0_pipe0_eagle_epcl_0_macro_epcl_desc","ermrk2phal_desc",4},GT_FALSE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E */
    {{"tile0_pipe1_eagle_pha_1_macro_pha_desc","pha2erep_desc",0},GT_TRUE}};


static const PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC     hawkFixedBoundInterfaceArr[PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1] =
{
    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E */
    {{"tile0_epcl_ha_macro_hbu2tti","hbuing2tti_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E */
    {{"tile0_tti_macro_tti_desc","tti2pcl_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E */
    {{"tile0_pcl_macro_pcl_desc","pcl2l2i_desc" ,5},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E */
    {{"tile0_l2i_ipvx_macro_ipvx_desc","l2i2ipvx_desc",5},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E */
    {{"tile0_l2i_ipvx_macro_ioam_desc","ipvx2ioam_desc",5},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E */
    {{"tile0_upper_ing_macro_iplr0_desc","iplr02iplr1_desc",2},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E */
    {{"tile0_upper_ing_macro_iplr1_desc","iplr12mll_desc",1},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E */
    {{"tile0_upper_ing_macro_mll_eq_desc","mll2eq_desc",3},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E */
    {{"tile0_upper_ing_macro_eft_desc","eq2eft_desc",2},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E */
    {{"tile0_epcl_ha_macro_erep2hbu","erep2hbuegr_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E */
    {{"tile0_egf_macro_sht_qag","qag2ha_desc",3},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E */
    {{"tile0_epcl_ha_macro_ermrk_desc","preq2eoam_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E */
    {{"tile0_epcl_ha_macro_ha_desc","ha2ekgen_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E */
    {{"tile0_epcl_ha_macro_epcl_desc","eoam2eplr_desc",2},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E */
    {{"tile0_epcl_ha_macro_eplr_desc","eplr2psu_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E */
    {{"tile0_hawk_txqp_macro_pdx0","pdx2pdsi_desc",5},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E */
    {{"tile0_epcl_ha_macro_ermrk_desc","ermrk2phal_desc",1},GT_FALSE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E */
    {{"tile0_pha_macro_pha_desc","pha2erep_desc",0},GT_TRUE}};

static const PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC     phoenixFixedBoundInterfaceArr[PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1] =
{
    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E */
    {{"pnx_pha_macro_hbu2tti","hbuing2tti_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E */
    {{"pnx_tti_macro_tti_desc","tti2pcl_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E */
    {{"pnx_pcl_macro_pcl_desc","pcl2l2i_desc" ,5},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E */
    {{"pnx_l2i_ipvx_macro_ipvx_desc","l2i2ipvx_desc",5},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E */
    {{"pnx_l2i_ipvx_macro_ioam_desc","ipvx2ioam_desc",5},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E */
    {{"pnx_upper_ing_macro_iplr0_desc","iplr02iplr1_desc",2},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E */
    {{"pnx_upper_ing_macro_iplr1_desc","iplr12mll_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E */
    {{"pnx_upper_ing_macro_mll_eq_desc","mll2eq_desc",3},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E */
    {{"pnx_upper_ing_macro_eft_desc","eq2eft_desc",2},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E */
    {{"pnx_pha_macro_erep2hbu","erep2hbuegr_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E */
    {{"pnx_egf_macro_qag_ha","qag2ha_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E */
    {{"pnx_epcl_macro_ermrk_desc","preq2eoam_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E */
    {{"pnx_epcl_macro_ha_desc","ha2ekgen_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E */
    {{"pnx_epcl_macro_epcl_desc","eoam2eplr_desc",2},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E */
    {{"pnx_epcl_macro_eplr_desc","eplr2psu_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E */
    {{"pnx_txq_macro_pdx0","pdx2pdsi_desc",2},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E */
    {{"pnx_epcl_macro_ermrk_desc","ermrk2phal_desc",1},GT_FALSE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E */
    {{"pnx_pha_macro_pha_desc","pha2erep_desc",0},GT_TRUE}};

static const PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC     harrierFixedBoundInterfaceArr[PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1] =
{
    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E */
    {{"har_pha_macro_hbu2tti","hbuing2tti_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E */
    {{"har_tti_macro_tti_desc","tti2pcl_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E */
    {{"har_pcl_macro_pcl_desc","pcl2l2i_desc" ,5},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E */
    {{"har_l2i_ipvx_macro_ipvx_desc","l2i2ipvx_desc",5},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E */
    {{"har_l2i_ipvx_macro_ioam_desc","ipvx2ioam_desc",5},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E */
    {{"har_upper_ing_macro_iplr0_desc","iplr02iplr1_desc",2},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E */
    {{"har_upper_ing_macro_iplr1_desc","iplr12mll_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E */
    {{"har_upper_ing_macro_mll_eq_desc","mll2eq_desc",3},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E */
    {{"har_upper_ing_macro_eft_desc","eq2eft_desc",2},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E */
    {{"har_pha_macro_erep2hbu","erep2hbuegr_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E */
    {{"har_upper_ing_macro_qag_ha","qag2ha_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E */
    {{"har_epcl_macro_ha_desc","preq2eoam_desc",3},GT_FALSE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E */
    {{"har_epcl_macro_ha_desc", "ha2ekgen_desc", 0}, GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E */
    {{"har_epcl_macro_epcl_desc","eoam2eplr_desc",2},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E */
    {{"har_epcl_macro_eplr_desc","eplr2psu_desc",0},GT_TRUE},

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E */
    {{ "empty", "empty - pdx2pdsi_desc", 0 }, GT_FALSE },

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E */
    {{ "empty", "empty - ermrk2phal_desc", 0 }, GT_FALSE },

    /* CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E */
    {{"har_pha_macro_pha_desc","pha2erep_desc",0},GT_TRUE}};

/**
 * @internal prvCpssDxChPacketAnalyzerBoundedInterfaceArrGet
 *           function
 * @endinternal
 *
 * @brief   Get bounded interfaces of specific device
 *
 * @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
 *
 * @param[in] devNum                    - device number
 * @param[out] boundedInterfacesArrPtr  - (pointer to) bounded
 *                                        interfaces of specific
 *                                        device
 * @param[out] sizePtr                  - (pointer to) size of
 *                                        array
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong value in any of the parameters
 * @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
 * @retval GT_BAD_PTR               - on NULL pointer value.
 * @retval GT_FAIL                  - on error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_BAD_STATE             - on invalid parameter
 */
GT_STATUS prvCpssDxChPacketAnalyzerBoundedInterfaceArrGet
(
    IN   GT_U8                                              devNum,
    OUT  PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC  *boundedInterfacesArrPtr,
    OUT  GT_U32                                             *sizePtr
)
{
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(boundedInterfacesArrPtr);
    CPSS_NULL_PTR_CHECK_MAC(sizePtr);

    *sizePtr =  PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1;

    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
             cpssOsMemCpy(boundedInterfacesArrPtr, aldrinFixedBoundInterfaceArr, sizeof(aldrinFixedBoundInterfaceArr));
             break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
             cpssOsMemCpy(boundedInterfacesArrPtr, aldrin2FixedBoundInterfaceArr, sizeof(aldrin2FixedBoundInterfaceArr));
             break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
             cpssOsMemCpy(boundedInterfacesArrPtr, falconFixedBoundInterfaceArr, sizeof(falconFixedBoundInterfaceArr));
             break;
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
             cpssOsMemCpy(boundedInterfacesArrPtr, hawkFixedBoundInterfaceArr, sizeof(hawkFixedBoundInterfaceArr));
             break;
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
             cpssOsMemCpy(boundedInterfacesArrPtr, phoenixFixedBoundInterfaceArr, sizeof(phoenixFixedBoundInterfaceArr));
             break;
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
             cpssOsMemCpy(boundedInterfacesArrPtr, harrierFixedBoundInterfaceArr, sizeof(harrierFixedBoundInterfaceArr));
             break;
         default:
              CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"Error: Device family is not supported  for iDebug");
             break;
    }

    return GT_OK;
}

/**
 * @internal
 *           prvCpssDxChPacketAnalyzerIsStageApplicableInDev function
 * @endinternal
 *
 * @brief  check if stage is applicable in device
 *
 * @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
 *
 * @param[in] devNum               - device number
 * @param[in] stageId              - stageId
 *
 * @retval GT_TRUE                 - in case stage is applicable in device
 * @retval GT_FALSE                - in case stage is not applicable in device
 */
GT_BOOL prvCpssDxChPacketAnalyzerIsStageApplicableInDev
(
    IN GT_U8                                           devNum,
    IN CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     stageId
)
{
    GT_U32  ii ;

    /*uds are applicable in all devices*/
    if (stageId >= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E && stageId <= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E)
        return GT_TRUE;

    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            for (ii = 0 ;ii<PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_ALDRIN_CNS; ii++)
            {
                if (stageId == AldrinPipelineOrderStage[ii])
                    return GT_TRUE;
            }
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            for (ii = 0 ;ii<PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_ALDRIN2_CNS; ii++)
            {
                if (stageId == Aldrin2PipelineOrderStage[ii])
                    return GT_TRUE;
            }
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            for (ii = 0 ;ii<PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_FALCON_CNS; ii++)
            {
                if (stageId == FalconPipelineOrderStage[ii])
                    return GT_TRUE;
            }
            break;
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            for (ii = 0 ;ii<PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_HAWK_CNS; ii++)
            {
                if (stageId == HawkPipelineOrderStage[ii])
                    return GT_TRUE;
            }
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            for (ii = 0 ;ii<PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_PHOENIX_CNS; ii++)
            {
                if (stageId == PhoenixPipelineOrderStage[ii])
                    return GT_TRUE;
            }
            break;
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            for (ii = 0 ;ii<PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_HARRIER_CNS; ii++)
            {
                if (stageId == HarrierPipelineOrderStage[ii])
                    return GT_TRUE;
            }
            break;
        default:
             break;
    }
    return GT_FALSE;
}

/**
 * @internal
 *           prvCpssDxChPacketAnalyzerStageOrderGet function
 * @endinternal
 *
 * @brief  Get stage by order index for specific device
 *
 * @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
 *
 * @param[in] managerId             - manager identification
 *                                   (APPLICABLE RANGES: 1..10)
 * @param[in]  devNum               - device number
 * @param[in]  index                - index of wanted stage
 * @param[in]  offset               - offset to reduce
 * @param[out] stageIdPtr           - (pointer to)stage match to index
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong value in any of the parameters
 * @retval GT_BAD_PTR               - on NULL pointer value.
 * @retval GT_FAIL                  - on error
 * @retval GT_NOT_SUPPORTED         - on not supported parameter
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssDxChPacketAnalyzerStageOrderGet
(
    IN  GT_U32                                          managerId,
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          index,
    IN  GT_U32                                          offset,
    OUT CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     *stageIdPtr
)
{
    GT_U32 calcIndex = 0;
    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_STAGE_ID_CHECK_MAC(index);
    CPSS_NULL_PTR_CHECK_MAC(stageIdPtr);
    /*uds have the same order in all devices*/
    if (index >= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E && index <= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E)
    {
        *stageIdPtr = index;
        return GT_OK;
    }
    calcIndex = index - offset ;

    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
             if (calcIndex >= PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_ALDRIN_CNS)
             {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "(index - offset) is out of bound ");
             }
             *stageIdPtr = AldrinPipelineOrderStage[calcIndex];
             break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
             if (calcIndex >= PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_ALDRIN2_CNS)
             {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "(index - offset) is out of bound ");
             }
             *stageIdPtr = Aldrin2PipelineOrderStage[calcIndex];
             break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
             if (calcIndex >= PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_FALCON_CNS)
             {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "(index - offset) is out of bound ");
             }
             *stageIdPtr = FalconPipelineOrderStage[calcIndex];
             break;
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
             if (calcIndex >= PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_HAWK_CNS)
             {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "(index - offset) is out of bound ");
             }
             *stageIdPtr = HawkPipelineOrderStage[calcIndex];
             break;
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
             if (calcIndex >= PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_PHOENIX_CNS)
             {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "(index - offset) is out of bound ");
             }
             *stageIdPtr = PhoenixPipelineOrderStage[calcIndex];
             break;
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
             if (calcIndex >= PRV_CPSS_DXCH_PACKET_ANALYZER_NUM_OF_STAGES_HARRIER_CNS)
             {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "(index - offset) is out of bound ");
             }
             *stageIdPtr = HarrierPipelineOrderStage[calcIndex];
             break;
         default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"Error: Device family is not supported for iDebug");
             break;
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPacketAnalyzerFirstDevGet function
* @endinternal
*
* @brief   The function returns pointer to first device number
*          in packet analyzer manger DB.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId      - manager identification
*                             (APPLICABLE RANGES: 1..10)
* @param[out] devNumPtr     - (pointer to) device number
*
* @retval GT_OK             - on success
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
* @retval GT_BAD_PTR        - on NULL pointer value
* @retval GT_FAIL           - on error
*/
GT_STATUS prvCpssDxChPacketAnalyzerFirstDevGet
(
    IN  GT_U32                               managerId,
    OUT  GT_U8                               *devNumPtr
)
{
    GT_U32 i;

    /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    CPSS_NULL_PTR_CHECK_MAC(devNumPtr);

    for(i=0; i<PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
    {
        if(PRV_CPSS_PACKET_ANALYZER_MNG_IS_DEV_BMP_SET_MAC(PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->devsBitmap, i))
        {
            *devNumPtr = (GT_U8) i;
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
}

/**
* @internal
*           prvCpssDxChPacketAnalyzerDefaultInterfaceInitSet
*           function
* @endinternal
*
* @brief   The function set default choosed interfaces.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
* @param[in] boundedInterfacesArr     - (pointer to) bounded
*                                       interfaces array
* @param[in]size                      - size of bounded
*                                       interfaces array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPacketAnalyzerDefaultInterfaceInitSet
(
    IN  GT_U32                                              managerId,
    IN  GT_U8                                               devNum,
    IN  PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC   *boundedInterfacesArr,
    IN  GT_U32                                              size
)
{
     GT_STATUS                                  rc;
     GT_U32                                     instanceDfxPipeId,instanceDfxBaseAddr,interfaceDfxIndex;
     mxml_node_t                                *node;
     GT_CHAR                                    interfaceName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
     GT_CHAR                                    instanceId[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
     GT_CHAR                                    polarity[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
     GT_U32                                     i,value,muxValidBmp;
     CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT stagesArr[PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1];
     GT_U32                                     numberOfTiles,tileIterator;

     PRV_CPSS_PA_NUM_OF_TILE_GET_MAC(devNum,numberOfTiles);

     for(i=0;i<PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1;i++)
     {
         stagesArr[i]=(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)i;
     }

    /*mark valid stages*/
    rc = prvCpssDxChPacketAnalyzerMarkMuxedStagesSet(managerId,devNum,size,stagesArr,&muxValidBmp);
    if (rc != GT_OK)
    {
        return rc;
    }

    for(i=0;i<size;i++)
    {
         /*write is duplicated at the low level,writing to tile 0 result in writing to all tiles*/
         tileIterator=0;

         if((muxValidBmp&(1<<i)))
         {
             cpssOsStrCpy(interfaceName,boundedInterfacesArr[i].info.interfaceId);

             node = mxmlInstanceInfoGet(devNum,interfaceName,0/*TBD -currently set only multiplication 0*/,instanceId,&instanceDfxPipeId,&instanceDfxBaseAddr,&interfaceDfxIndex,NULL);
             if (!node)
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: failed to find instance info for interface [%s] in XML",interfaceName);


              node = mxmlInterfacePolarityGet(devNum,interfaceName,polarity);
              if(!node)
                  CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: faild to get interface [%s] polarity in XML",interfaceName);

             /* select DFX pipe for Read and Write Transaction */
             rc = prvCpssDxChIdebugInstanceDfxPipeSet(devNum,instanceDfxPipeId);
             if(rc != GT_OK)
             {
                 return rc;
             }
            /* select default interface */
            rc = prvCpssDxChIdebugInstanceDfxInterfaceSelect(devNum,instanceDfxBaseAddr,interfaceDfxIndex,polarity);
            if(rc != GT_OK)
            {
                return rc;
            }

            /*read from all tiles*/
            for(tileIterator=0;tileIterator<numberOfTiles;tileIterator++)
            {
                /*in order to clear*/
                rc = prvCpssDxChIdebugPortGroupInterfaceReadMatchCounter(devNum,instanceDfxBaseAddr+PRV_DFX_XSB_TILE_OFFSET_MAC(devNum,tileIterator),&value);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }

         }

     }

     return GT_OK;
}

GT_STATUS prvCpssDxChPacketAnalyzerDumpSwCounters
(
    IN  GT_U32                                          managerId,
    IN  GT_U8                                           devNum,
    IN  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     stage
)
{

    PRV_CPSS_DXCH_IDEBUG_INTERFACE_STC       *idebugInterfaceDb;
    GT_U32                                   tile,dp,pipe,i,base;
    GT_STATUS                                rc;
    GT_CHAR_PTR                              interfaceName;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC *stageDbPtr = NULL;

    if(GT_FALSE==PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        return GT_OK;
    }

    /* get pointer to stage in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbStageGet(managerId,devNum,stage,&stageDbPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    interfaceName = stageDbPtr->boundInterface.interfaceId;

    if (cpssOsStrCmp(interfaceName,"") == 0)
    {
        cpssOsPrintf("\nstage %s is not bounded to interface\n",interfaceName);
        return GT_OK;
    }

    cpssOsPrintf("\ninterfaceName  %s\n\n",interfaceName);

    /* get pointer to head of MXML node tree from iDebug DB */
    rc = prvCpssDxChIdebugDbInterfaceGet(devNum,interfaceName,&idebugInterfaceDb);
    if (rc != GT_OK)
    {
        return rc;
    }

    cpssOsPrintf("\n+-------------+--------+--------+--------+--------+------------+");
    cpssOsPrintf("\n+   index     + value  +  tile  +  pipe  +   dp   +    base    +");
    cpssOsPrintf("\n+-------------+--------+--------+--------+--------+------------+");

    for(i=0;i<MAX_MULTIPLICATIONS_NUM;i++)
    {

        if(idebugInterfaceDb->matchCounterMetaData[i]&1<<PRV_CPSS_DXCH_IDEBUG_META_DATA_VALID_OFFSET_CNS)
        {
            tile =idebugInterfaceDb->matchCounterMetaData[i]&(0xf<<PRV_CPSS_DXCH_IDEBUG_META_DATA_TILE_OFFSET_CNS);
            tile>>=PRV_CPSS_DXCH_IDEBUG_META_DATA_TILE_OFFSET_CNS;
            pipe = idebugInterfaceDb->matchCounterMetaData[i]&(0xf<<PRV_CPSS_DXCH_IDEBUG_META_DATA_PIPE_OFFSET_CNS);
            pipe>>=PRV_CPSS_DXCH_IDEBUG_META_DATA_PIPE_OFFSET_CNS;
            dp = idebugInterfaceDb->matchCounterMetaData[i]&(0xf<<PRV_CPSS_DXCH_IDEBUG_META_DATA_DP_OFFSET_CNS);
            dp>>=PRV_CPSS_DXCH_IDEBUG_META_DATA_DP_OFFSET_CNS;
            base = idebugInterfaceDb->matchCounterMetaData[i]&(0xfff<<PRV_CPSS_DXCH_IDEBUG_META_DATA_BASE_OFFSET_CNS);
            base>>=PRV_CPSS_DXCH_IDEBUG_META_DATA_BASE_OFFSET_CNS;


            if(idebugInterfaceDb->matchCounterMetaData[i]&1<<PRV_CPSS_DXCH_IDEBUG_META_DATA_PER_DP_OFFSET_CNS)
            {
               cpssOsPrintf("\n+%13d+%8d+%8d+%8d+%8d+ 0x%dbe%3X00 +",i,idebugInterfaceDb->matchCounterValue[i],tile,pipe,dp,1+2*tile,base);
            }
            else
            {
                cpssOsPrintf("\n+%13d+%8d+%8d+%8d+   NA   + 0x%dbe%3X00 +",i,idebugInterfaceDb->matchCounterValue[i],tile,pipe,1+2*tile,base);
            }
            cpssOsPrintf("\n+-------------+--------+--------+--------+--------+------------+");
        }
    }

    cpssOsPrintf("\n");

    return GT_OK;
}

static GT_STATUS prvCpssDxChPacketAnalyzerIsInterfaceUsedGet
(
    IN  GT_U32                                          managerId,
    IN  GT_U8                                           devNum,
    IN  GT_CHAR_PTR                                     interfaceName,
    IN  GT_BOOL                                         *isUsedPtr
)
{
    GT_U32    i,id=0;
    GT_STATUS rc;
    PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR       paDevDbPtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR       paMngDbPtr = NULL;

    PRV_CPSS_PACKET_ANALYZER_INTERFACE_ID_CHECK_MAC(devNum,interfaceName);

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    /* get pointer to device in packet analyzer manager DB */
    rc = prvCpssDxChPacketAnalyzerDbDeviceGet(managerId,devNum,&paDevDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    *isUsedPtr= GT_FALSE;
    for(i=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_FIRST_E;i<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E;i++)
    {
        if(prvCpssDxChPacketAnalyzerIsStageApplicableInDev(devNum,i) == GT_FALSE)
            continue;

        if (cpssOsStrCmp(paDevDbPtr->stagesArr[i].boundInterface.interfaceId,interfaceName) == 0)
        {
            *isUsedPtr= GT_TRUE;
            break;
        }
    }

    for(i=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E;i<=PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E;i++)
    {
        id = i - (PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E+1);

        if (paMngDbPtr->udsArr[id].valid == GT_TRUE)
        {
            if (cpssOsStrCmp(paDevDbPtr->stagesArr[i].boundInterface.interfaceId,interfaceName) == 0)
            {
                *isUsedPtr= GT_TRUE;
                break;
            }
        }
    }

    return GT_OK;

}

/**
* @internal
*           prvCpssDxChPacketAnalyzerAllUsedInterfacesSamplingDisable
*           function
* @endinternal
*
* @brief   Disable sampling on all used interfaces in device.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_CPU_MEM        - on cpu memory allocation failure
*/
GT_STATUS prvCpssDxChPacketAnalyzerAllUsedInterfacesSamplingDisable
(
    IN  GT_U32                                  managerId,
    IN  GT_U8                                   devNum
)
{
    GT_U32                                      i,numOfInterfaces;
    GT_CHAR_PTR                                 *interfacesArrDbPtr = NULL;
    GT_STATUS                                   rc = GT_OK;
    GT_BOOL                                     isUsed;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SIM_NOT_SUPPORTED_MAC();

    /* get pointer to interfaces array in iDebug DB */
    rc = prvCpssDxChIdebugDbInterfaceArrayGet(devNum,&numOfInterfaces,&interfacesArrDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* disable sampling for all used interfaces in devNum */
    for (i=0; i<numOfInterfaces; i++)
    {
        isUsed = GT_FALSE;
        rc = prvCpssDxChPacketAnalyzerIsInterfaceUsedGet(managerId,devNum,interfacesArrDbPtr[i],&isUsed);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(GT_TRUE ==isUsed)
        {
            rc = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,interfacesArrDbPtr[i],GT_FALSE);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

static GT_STATUS prvCpssDxChPacketAnalyzerFieldLocationGet
(
    IN  GT_U32                                          managerId,
    IN  GT_CHAR_PTR                                     interfaceName,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT             fieldName,
    OUT GT_U32                                          *startBitPtr,
    OUT GT_U32                                          *endBitPtr
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                    paMngDbPtr = NULL;
    GT_U32                                                   id;
    mxml_node_t                                              *node;
    GT_CHAR                                                  udfName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_U8                                                    devNum;

    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);

    paMngDbPtr = PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId);

    if ((fieldName >= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E) && (fieldName <= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E))
    {
        id = fieldName - (PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E+1);
        if (paMngDbPtr->udfArr[id].valid == GT_FALSE)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: user defined field [%d] is not valid",fieldName);

        cpssOsStrCpy(udfName,paMngDbPtr->udfArr[id].iDebugField);

        node = mxmlInterfaceFieldSizeGet(devNum,interfaceName,udfName,startBitPtr,endBitPtr);
        if(!node)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"Error: failed to get field [%s] size in XML",udfName);
    }
    else
    {
        node = mxmlInterfaceFieldSizeGet(devNum,interfaceName,packetAnalyzerFieldToIdebugFieldArr[fieldName],startBitPtr,endBitPtr);
        if(!node)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"Error: failed to get field [%s] size in XML",paFieldStrArr[fieldName]);
    }

    return GT_OK;
}

/**
* @internal
*           prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields
*           function
* @endinternal
*
* @brief   Check no overlapping fields in rule.
*
* @note   APPLICABLE DEVICES:      Aldrin; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; AC3X; Bobcat3.
*
* @param[in] managerId                - manager identification
*                                       (APPLICABLE RANGES: 1..10)
* @param[in] keyId                    - logical key identification
*                                       (APPLICABLE RANGES: 1..1024)
* @param[in] numOfFields              - number of fields for
*                                       rule and group identification
* @param[in] fieldsValueArr           - rule content array
*                                       (Data and Mask)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to parameters list
* @retval GT_BAD_PTR               - on NULL pointer value.
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - not found group or action
* @retval GT_FULL                  - if array is full
* @retval GT_ALREADY_EXIST         - already initialized
* @retval GT_OUT_OF_CPU_MEM        - Cpu memory allocation failed
*/
GT_STATUS prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields
(
    IN  GT_U32                                          managerId,
    IN  GT_U32                                          keyId,
    IN  GT_U32                                          numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC       fieldsValueArr[]
)
{
    PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR                    paKeyDbPtr = NULL;
    GT_U8                                                    devNum;
    GT_U32                                                   i,j,k,n;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT              stageId;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                      fieldName;
    GT_STATUS                                                rc;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC        interfaceId;
    GT_U32                                                   endBit,startBit,endBitTemp,startBitTemp;
    GT_BOOL                                                  found = GT_FALSE;

     /* check parameters */
    PRV_CPSS_PACKET_ANALYZER_MANAGER_ID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_MANAGER_VALID_CHECK_MAC(managerId);
    PRV_CPSS_PACKET_ANALYZER_FIRST_DEVICE_GET_MAC(managerId,&devNum);
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_AC3X_E );
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_DFX_SERVER_SUPPORTED_MAC(devNum);
    PRV_CPSS_PACKET_ANALYZER_KEY_ID_CHECK_MAC(keyId);
    CPSS_NULL_PTR_CHECK_MAC(fieldsValueArr);

    if (numOfFields > PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: numOfFields %d is out of bounds",numOfFields) ;

    rc = prvCpssDxChPacketAnalyzerDbDevGet(managerId,keyId,&devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* get pointer to device in packet analyzer key DB */
    rc = prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,keyId,&paKeyDbPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* check that no overlapping fields in fieldsValueArr[] */
    if (paKeyDbPtr->fieldMode == CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_ALL_E)
    {

        for (n=0; n<paKeyDbPtr->numOfStages; n++)
        {
            /* stage should contain all the fields */
            stageId = paKeyDbPtr->stagesArr[n];

            /* get bounded interface */
            rc = prvCpssDxChPacketAnalyzerStageInterfacesGet(managerId,devNum,stageId,&interfaceId);
            if (rc != GT_OK)
            {
                return rc;
            }

            for (i=0; i<numOfFields; i++)
            {

                fieldName = fieldsValueArr[i].fieldName;
                if (fieldName >= CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: fieldName %d is out of bounds",fieldName) ;
                }

                /* get information for field */
                rc = prvCpssDxChPacketAnalyzerFieldLocationGet(managerId,interfaceId.info.interfaceId,fieldName,&startBit,&endBit);
                if (rc != GT_OK)
                {
                    return rc;
                }

                /* check field compared to other fields */
                for (j=0; j<numOfFields; j++)
                {
                    if (fieldsValueArr[j].fieldName == fieldName)
                        continue;

                    /* get information for field */
                    rc = prvCpssDxChPacketAnalyzerFieldLocationGet(managerId,interfaceId.info.interfaceId,fieldsValueArr[j].fieldName,&startBitTemp,&endBitTemp);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }

                    /* check overlapping bits */
                    if ((startBit >= startBitTemp && startBit <= endBitTemp) ||
                        (endBit >= startBitTemp && endBit <= endBitTemp) ||
                        (startBitTemp >= startBit && startBitTemp <= endBit) ||
                        (endBitTemp >= startBit && endBitTemp <= endBit))
                    {
                        if ((fieldName > PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E) ||
                            (fieldsValueArr[j].fieldName > PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E) )
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: overlapping fields: field [%d] bits [%d..%d] and field [%d] bits [%d..%d]",
                                                          fieldName, startBit, endBit,
                                                          fieldsValueArr[j].fieldName, startBitTemp, endBitTemp);
                        }
                        else
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: overlapping fields: field [%s] bits [%d..%d] and field [%s] bits [%d..%d]",
                                                          paFieldStrArr[fieldName], startBit, endBit,
                                                          paFieldStrArr[fieldsValueArr[j].fieldName], startBitTemp, endBitTemp);
                        }
                    }
                }
            }
        }

    }
    else
    {
        for (i=0; i<numOfFields; i++)
        {
            fieldName = fieldsValueArr[i].fieldName;
            found = GT_FALSE;

            if (fieldName >= CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: fieldName %d is out of bounds",fieldName) ;
            }

            /* find field's all stages in key */
            for (j=0; j<paKeyDbPtr->numOfStages; j++)
            {
                found = GT_FALSE;
                stageId = paKeyDbPtr->stagesArr[j];
                for (k=0; k<paKeyDbPtr->perStageNumOfFields[stageId]; k++)
                {
                    if (fieldName == paKeyDbPtr->perStagesFieldsArr[stageId][k])
                    {
                        found = GT_TRUE;
                        break;
                    }
                }
                if (found == GT_FALSE)
                    continue;

                /* get bounded interface */
                rc = prvCpssDxChPacketAnalyzerStageInterfacesGet(managerId,devNum,stageId,&interfaceId);
                if (rc != GT_OK)
                {
                    return rc;
                }

                /* get information for field */
                rc = prvCpssDxChPacketAnalyzerFieldLocationGet(managerId,interfaceId.info.interfaceId,fieldName,&startBit,&endBit);
                if (rc != GT_OK)
                {
                    return rc;
                }

                /* check field compared to other fields in stageId */
                for (n=0; n<numOfFields; n++)
                {
                    if (fieldsValueArr[n].fieldName == fieldName)
                        continue;

                    /* check if field exists in stageId */
                    found = GT_FALSE;
                    for (k=0; k<paKeyDbPtr->perStageNumOfFields[stageId]; k++)
                    {
                        if (fieldsValueArr[n].fieldName == paKeyDbPtr->perStagesFieldsArr[stageId][k])
                        {
                            found = GT_TRUE;
                            break;
                        }
                    }
                    if (found == GT_FALSE)
                        continue;

                    /* get information for field */
                    rc = prvCpssDxChPacketAnalyzerFieldLocationGet(managerId,interfaceId.info.interfaceId,fieldsValueArr[n].fieldName,&startBitTemp,&endBitTemp);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }

                    /* check overlapping bits */
                    if ((startBit >= startBitTemp && startBit <= endBitTemp) ||
                        (endBit >= startBitTemp && endBit <= endBitTemp) ||
                        (startBitTemp >= startBit && startBitTemp <= endBit) ||
                        (endBitTemp >= startBit && endBitTemp <= endBit))
                    {
                        if ((fieldName > PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E) ||
                            (fieldsValueArr[j].fieldName > PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E) )
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: overlapping fields: field [%d] bits [%d..%d] and field [%d] bits [%d..%d]",
                                                          fieldName, startBit, endBit,
                                                          fieldsValueArr[j].fieldName, startBitTemp, endBitTemp);
                        }
                        else
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: overlapping fields: field [%s] bits [%d..%d] and field [%s] bits [%d..%d]",
                                                          paFieldStrArr[fieldName], startBit, endBit,
                                                          paFieldStrArr[fieldsValueArr[j].fieldName], startBitTemp, endBitTemp);
                        }
                    }
                }
            }
        }
    }

    return GT_OK;
}


/**
 * @internal prvCpssDxChPacketAnalyzerIsXmlPresent function
 * @endinternal
 *
 * @brief  check if xml is present for device
 *
 * @param[in] devNum               - device number
 *
 * @retval GT_TRUE                 - in case xml is present
 * @retval GT_FALSE                - in case xml is not present
 */
GT_BOOL prvCpssDxChPacketAnalyzerIsXmlPresent
(
    IN GT_U8       devNum
)
{
    CPSS_OS_FILE_TYPE_STC     file;
    GT_STATUS                 rc;
    GT_CHAR_PTR               xmlFileName;

    /* open CIDER XML */
    rc= prvCpssDxChIdebugFileNameGet(devNum,&xmlFileName);
    if(rc!=GT_OK)
    {
         return GT_FALSE;
    }

    file.fd =cpssOsFopen(xmlFileName, "r",&file);
    if(file.fd == CPSS_OS_FILE_INVALID)
    {
        return GT_FALSE;
    }

    cpssOsFclose(&file);
    return GT_TRUE;
}
