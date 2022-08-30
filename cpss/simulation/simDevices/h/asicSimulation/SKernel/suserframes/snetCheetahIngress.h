/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file snetCheetahIngress.h
*
* @brief This is a external API definition for SMem module of SKernel.
*
* @version   22
********************************************************************************
*/
#ifndef __snetCheetahIngressh
#define __snetCheetahIngressh

#include <asicSimulation/SKernel/smain/smain.h>
#include <common/Utils/FrameInfo/sframeInfoAddr.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahL2.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3TTermination.h>
#include <asicSimulation/SKernel/suserframes/snet.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* NULL port */
#define SNET_CHT_NULL_PORT_CNS                          62

#define MTI_CPU_PORT_MIF_PORT(dev,_cpu_port_index)\
    SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_MTI_cpu_port[_cpu_port_index].MIF_CPU
#define MTI_CPU_PORT_MIF_GLOBAL(dev,_cpu_port_index)\
    SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_MTI_cpu_port[_cpu_port_index].MIF_CPU_global
#define MTI_PORT_MIF(dev,_macPort)\
    SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_MTI[_macPort].MIF
#define MTI_PORT_MIF_GLOBAL(dev,_representativePort)\
    SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_MTI_EXTERNAL[_representativePort].MIF_global

/* values taken from RFC 2460 */

/* end of packet or unknown not L4 data */
#define SNET_CHT_IPV6_EXT_HDR_NO_EXT_AND_NO_L4_CNS     59
/* IPV6 extended headers */
#define SNET_CHT_IPV6_EXT_HDR_HOP_BY_HOP_CNS            0
#define SNET_CHT_IPV6_EXT_HDR_ROUTING_CNS              43
#define SNET_CHT_IPV6_EXT_HDR_FRAGMENT_CNS             44
#define SNET_CHT_IPV6_EXT_HDR_ENCAP_SECUR_PAYLOAD_CNS  50
#define SNET_CHT_IPV6_EXT_HDR_AUTENTICATION_CNS        51
#define SNET_CHT_IPV6_EXT_HDR_DESTINATION_OPTIONS_CNS  60
#define SNET_CHT_IPV6_EXT_HDR_MOBILITY_HEADER_CNS     135
/* new in sip 6 */
#define SNET_CHT_IPV6_EXT_HDR_HOST_IDENTIFY_PRTOCOL   139
#define SNET_CHT_IPV6_EXT_HDR_SHIM6_PRTOCOL           140
#define SNET_CHT_IPV6_EXT_HDR_EXPERIMENTAL_1          253
#define SNET_CHT_IPV6_EXT_HDR_EXPERIMENTAL_2          254

/* the types of the DMA clients */
typedef enum{
    SNET_CHT_DMA_CLIENT_PACKET_FROM_CPU_E,
    SNET_CHT_DMA_CLIENT_PACKET_TO_CPU_E,
    SNET_CHT_DMA_CLIENT_AUQ_E,
    SNET_CHT_DMA_CLIENT_FUQ_E,

    SNET_CHT_MEM_OVER_PCIe_CLIENT_CM3_E,/* the CM3 want to access the memory over the PEX */

    SNET_CHT_DMA_CLIENT___LAST___E

}SNET_CHT_DMA_CLIENT_ENT;

/* the types MIB counters of the MAC port */
typedef enum{
    SNET_CHT_PORT_MIB_COUNTERS_GOOD_OCTETS_RECEIVED_E,
    SNET_CHT_PORT_MIB_COUNTERS_BAD_OCTETS_RECEIVED_E,
    SNET_CHT_PORT_MIB_COUNTERS_CRC_ERRORS_SENT_E,
    SNET_CHT_PORT_MIB_COUNTERS_GOOD_UNICAST_FRAMES_RECEIVED_E,
    SNET_CHT_PORT_MIB_COUNTERS_FRAMES_1024_TO_1518_OCTETS_E,
    SNET_CHT_PORT_MIB_COUNTERS_BROADCAST_FRAMES_RECEIVED_E,
    SNET_CHT_PORT_MIB_COUNTERS_MULTICAST_FRAMES_RECEIVED_E,
    SNET_CHT_PORT_MIB_COUNTERS_FRAMES_64_OCTETS_E,
    SNET_CHT_PORT_MIB_COUNTERS_FRAMES_65_TO_127_OCTETS_E,
    SNET_CHT_PORT_MIB_COUNTERS_FRAMES_128_TO_255_OCTETS_E,
    SNET_CHT_PORT_MIB_COUNTERS_FRAMES_256_TO_511_OCTETS_E,
    SNET_CHT_PORT_MIB_COUNTERS_FRAMES_512_TO_1023_OCTETS_E,
    SNET_CHT_PORT_MIB_COUNTERS_FRAMES_1024_TO_MAX_OCTETS_E,
    SNET_CHT_PORT_MIB_COUNTERS_GOOD_OCTETS_SENT_E,
    SNET_CHT_PORT_MIB_COUNTERS_UNICAST_FRAME_SENT_E,
    SNET_CHT_PORT_MIB_COUNTERS_RESERVED_2_E,
    SNET_CHT_PORT_MIB_COUNTERS_MULTICAST_FRAMES_SENT_E,
    SNET_CHT_PORT_MIB_COUNTERS_BROADCAST_FRAMES_SENT_E,
    SNET_CHT_PORT_MIB_COUNTERS_RESERVED_3_E,
    SNET_CHT_PORT_MIB_COUNTERS_FC_SENT_E,
    SNET_CHT_PORT_MIB_COUNTERS_FC_RECEIVED_E,
    SNET_CHT_PORT_MIB_COUNTERS_RECEIVED_FIFO_OVERRUN_E,
    SNET_CHT_PORT_MIB_COUNTERS_UNDERSIZE_E,
    SNET_CHT_PORT_MIB_COUNTERS_FRAGMENTS_E,
    SNET_CHT_PORT_MIB_COUNTERS_OVERSIZE_E,
    SNET_CHT_PORT_MIB_COUNTERS_JABBER_E,
    SNET_CHT_PORT_MIB_COUNTERS_RX_ERROR_FRAME_RECEIVED_E,
    SNET_CHT_PORT_MIB_COUNTERS_BAD_CRC_E,
    SNET_CHT_PORT_MIB_COUNTERS_COLLISION_E,
    SNET_CHT_PORT_MIB_COUNTERS_LATE_COLLISION_E,

    SNET_CHT_PORT_MIB_COUNTERS____LAST____E
}SNET_CHT_PORT_MIB_COUNTERS_ENT;


/* for 100G port : the types MIB counters of the MAC port */
typedef enum{
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_aFramesTransmittedOK_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_aFramesReceivedOK_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_aFrameCheckSequenceErrors_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_aAlignmentErrors_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_aPAUSEMACCtrlFramesTransmitted_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_aPAUSEMACCtrlFramesReceived_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_aFrameTooLongErrors_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_aInRangeLengthErrors_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_VLANTransmittedOK_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_VLANReceivedOK_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ifOutOctets_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ifInOctets_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ifInUcastPkts_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ifInMulticastPkts_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ifInBroadcastPkts_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ifOutErrors_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ifOutUcastPkts_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ifOutMulticastPkts_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ifOutBroadcastPkts_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsDropEvents_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsOctets_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsPkts_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsUndersizePkts_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsPkts64Octets_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsPkts65to127Octets_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsPkts128to255Octets_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsPkts256to511Octets_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsPkts512to1023Octets_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsPkts1024to1518Octets_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsPkts1519toMaxOctets_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsOversizePkts_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsJabbers_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsFragments_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ifInErrors_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_aCBFCPAUSEFramesTransmitted_0_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_aCBFCPAUSEFramesTransmitted_7_E =
        SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_aCBFCPAUSEFramesTransmitted_0_E + 7,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_aCBFCPAUSEFramesReceived_0_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_aCBFCPAUSEFramesReceived_7_E =
        SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_aCBFCPAUSEFramesReceived_0_E + 7,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_aMACControlFramesTransmitted_E,
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_aMACControlFramesReceived_E,

    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT____LAST____E
}SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ENT;

/* for 50/10G and 200/400G port : the types for port MTI staistic counters */
typedef enum{
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsOctets_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_OctetsReceivedOK_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_aAlignmentErrors_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_aPauseMacCtrlFramesReceived_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_aFrameTooLong_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_aInRangeLengthError_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_aFramesReceivedOK_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_aFrameCheckSequenceErrors_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_VLANReceivedOK_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_ifInErrors_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_ifInUcastPkts_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_ifInMulticastPkts_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_ifInBroadcastPkts_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsDropEvents_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsPkts_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsUndersizePkts_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsPkts64Octets_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsPkts65to127Octets_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsPkts128to255Octets_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsPkts256to511Octets_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsPkts512to1023Octets_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsPkts1024to1518Octets_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsPkts1519toMaxOctets_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsOversizePkts_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsJabbers_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsFragments_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_aCBFCPAUSEFramesReceived0_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_aCBFCPAUSEFramesReceived15_E =
        SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_aCBFCPAUSEFramesReceived0_E + 15,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_aMACControlFramesReceived_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_etherStatsOctets_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_OctetsTransmittedOK_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_aPauseMacCtrlFramesTransmitted_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_aFramesTransmittedOK_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_VLANTransmittedOK_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_ifOutErrors_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_ifOutUcastPkts_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_ifOutMulticastPkts_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_ifOutBroadcastPkts_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_etherStatsPkts64Octets_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_etherStatsPkts65to127Octets_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_etherStatsPkts128to255Octets_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_etherStatsPkts256to511Octets_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_etherStatsPkts512to1023Octets_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_etherStatsPkts1024to1518Octets_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_etherStatsPkts1519toMaxOctets_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_aCBFCPAUSEFramesTransmitted0_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_aCBFCPAUSEFramesTransmitted15_E =
        SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_aCBFCPAUSEFramesTransmitted0_E + 15,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_aMACControlFramesTransmitted_E,
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_TX_etherStatsPkts_E,

    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS____LAST____E
}SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_ENT;

/* enum for fields in the MAC */
typedef enum{
     SNET_CHT_PORT_MAC_FIELDS_PortEn_E
    ,SNET_CHT_PORT_MAC_FIELDS_PortMACReset_E
    ,SNET_CHT_PORT_MAC_FIELDS_LinkState_E
    ,SNET_CHT_PORT_MAC_FIELDS_mru_E
    ,SNET_CHT_PORT_MAC_FIELDS_MIBCntEn_E
    ,SNET_CHT_PORT_MAC_FIELDS_forwardPfcFramesEnable_E
    ,SNET_CHT_PORT_MAC_FIELDS_forwardUnknownMacControlFramesEnable_E
    ,SNET_CHT_PORT_MAC_FIELDS_forceLinkDown_E
    ,SNET_CHT_PORT_MAC_FIELDS_forceLinkPass_E
    ,SNET_CHT_PORT_MAC_FIELDS_LPI_request_enable_E
    ,SNET_CHT_PORT_MAC_FIELDS_portInLoopBack_E
    ,SNET_CHT_PORT_MAC_FIELDS_int_bit_for_PCS_Tx_Path_Received_LPI_E  /*get the bit of the interrupt*/
    ,SNET_CHT_PORT_MAC_FIELDS_int_bit_for_PCS_Rx_Path_Received_LPI_E  /*get the bit of the interrupt*/
    ,SNET_CHT_PORT_MAC_FIELDS_int_bit_for_MAC_Rx_Path_Received_LPI_E  /*get the bit of the interrupt*/
    ,SNET_CHT_PORT_MAC_FIELDS_Unidirectional_enable_support_802_3ah_OAM_E
    ,SNET_CHT_PORT_MAC_FIELDS_PaddingDis_E
    ,SNET_CHT_PORT_MAC_FIELDS_histogram_rx_en_E
    ,SNET_CHT_PORT_MAC_FIELDS_histogram_tx_en_E
    ,SNET_CHT_PORT_MAC_FIELDS_mib_4_count_hist_E
    ,SNET_CHT_PORT_MAC_FIELDS_mib_4_limit_1518_1522_E
    ,SNET_CHT_PORT_MAC_FIELDS_Port_Clear_After_Read_E
    ,SNET_CHT_PORT_MAC_FIELDS_rx_path_en_E                          /* MAC receive path enable. Applicable for Falcon devices and above */
    ,SNET_CHT_PORT_MAC_FIELDS_tx_path_en_E                          /* MAC transmit path enable. Applicable for Falcon devices and above */
    ,SNET_CHT_PORT_MAC_FIELDS_open_drain_E                          /* SAU pass/drop the packets from getting to the MAC of MTI (Falcon) */
    ,SNET_CHT_PORT_MAC_FIELDS_open_drain_counter_increment_E        /* increment counter in SAU of drops (Falcon) */
    ,SNET_CHT_PORT_MAC_FIELDS_LMU_enable_E                          /* enable LMU for the port (Falcon) */
    ,SNET_CHT_PORT_MAC_FIELDS_is_mti_segmented_E                    /* is the port segmented (200G/400G) port (Falcon) */
    ,SNET_CHT_PORT_MAC_FIELDS_mti_mac_addr_0_E                      /* sip6:the ADDRESS[0] of the mac */
    ,SNET_CHT_PORT_MAC_FIELDS_mti_mac_addr_1_E                      /* sip6:the ADDRESS[1] of the mac */
    ,SNET_CHT_PORT_MAC_FIELDS_mti_mib_counter_is_emac_pmac_shared_E /* sip6.10: is the PMAC do mib counting shared with EMAC , or in separate counters */
    ,SNET_CHT_PORT_MAC_FIELDS_forwardFcPacketsEnable_E
    ,SNET_CHT_PORT_MAC_FIELDS_clock_Enable_E                        /* allow MI to set clock on the port */
    ,SNET_CHT_PORT_MAC_FIELDS_segmented_E                           /* allow MI to set port as segmented */
    ,SNET_CHT_PORT_MAC_FIELDS_fec_RS_FEC_Enable_Lane_E              /* allow MI to set serdes bmp */
    ,SNET_CHT_PORT_MAC_FIELDS_fec_KP_Mode_Enable_E                  /* allow MI to set serdes bmp */
    ,SNET_CHT_PORT_MAC_FIELDS_sgmii_speed_E                         /* allow MI to set SGMII speed */

    ,SNET_CHT_PORT_MAC_FIELDS____LAST____E

}SNET_CHT_PORT_MAC_FIELDS_ENT;

/* enum for ingress physical port assignment mode */
typedef enum{
    SNET_CHT_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DISABLED_E
    ,SNET_CHT_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_INTERLAKEN_E
    ,SNET_CHT_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DSA_E
    ,SNET_CHT_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_E_TAG_E
    ,SNET_CHT_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_VLAN_TAG_E

}SNET_CHT_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT;


typedef struct{
    GT_U32  offset;
    GT_U32  numWords;
    GT_CHAR* name;
}MIB_COUNTER_INFO_STC;

/**
* @internal snetChtProcessInit function
* @endinternal
*
* @brief   Init module.
*/
GT_VOID snetChtProcessInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal snetChtIngressAfterTti function
* @endinternal
*
* @brief   ingress pipe processing after the TTI unit
*/
GT_VOID snetChtIngressAfterTti
(
        IN SKERNEL_DEVICE_OBJECT * devObjPtr,
        IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/*******************************************************************************
*
* snetChtIngressAfterL3IpReplication:
*       ingress pipe processing after the IP MLL (L3 IP replication) unit
*
* INPUTS:
*       devObjPtr    - pointer to device object.
*       descrPtr     - frame data buffer Id
*
* RETURN:
*
*******************************************************************************/
GT_VOID snetChtIngressAfterL3IpReplication
(
        IN SKERNEL_DEVICE_OBJECT * devObjPtr,
        IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);


/**
* @internal snetCht3IngressMllSingleMllOutlifSet function
* @endinternal
*
* @brief   set the target outLif into descriptor - for single MLL
*         and Update mll counters
*/
void snetCht3IngressMllSingleMllOutlifSet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN   SNET_CHT3_SINGLE_MLL_STC * singleMllPtr
);


/**
* @internal snetChtPktCmdResolution function
* @endinternal
*
* @brief   resolve from old and current commands the new command
*
* @param[in] prevCmd                  --- previous command
* @param[in] currCmd                  — current command
*
* @note [2] Table 5: cpu code changes conflict resolution — page 17
*
*/
extern GT_U32 snetChtPktCmdResolution
(
    IN SKERNEL_EXT_PACKET_CMD_ENT prevCmd,
    IN SKERNEL_EXT_PACKET_CMD_ENT currCmd
);

/**
* @internal snetChtCpuCodeResolution function
* @endinternal
*
* @brief   resolve from old and new commands the new packet cpu code / DROP code
*
* @param[in] devObjPtr                - (pointer to) device object.
* @param[in,out] descrPtr                 - (pointer to) frame descriptor
* @param[in] prevCmd                  - previous command
* @param[in] currCmd                  ~W current command
* @param[in] afterResolutionCmd       - the command after the resolution
* @param[in] prevCpuCode              - previous cpu code
* @param[in] currCpuCode              ~W current cpu code
*/
extern void snetChtCpuCodeResolution
(
    IN SKERNEL_DEVICE_OBJECT                    *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC       *descrPtr,
    IN SKERNEL_EXT_PACKET_CMD_ENT prevCmd,
    IN SKERNEL_EXT_PACKET_CMD_ENT currCmd,
    IN SKERNEL_EXT_PACKET_CMD_ENT afterResolutionCmd,
    IN GT_U32                prevCpuCode,
    IN GT_U32                currCpuCode
);

/**
* @internal snetChtIngressCommandAndCpuCodeResolution function
* @endinternal
*
* @brief   1. resolve from old and current commands the new packet command
*         2. resolve from old and new commands the new packet cpu code (relevant
*         to copy that goes to CPU)
* @param[in] devObjPtr                - (pointer to) device object.
* @param[in,out] descrPtr                 - (pointer to) frame descriptor
* @param[in] prevCmd                  - previous command
* @param[in] currCmd                  — current command
* @param[in] prevCpuCode              - previous cpu code
* @param[in] currCpuCode              — current cpu code
* @param[in] engineUnit               - the engine unit that need resolution with previous engine
* @param[in] isFirst                  = GT_TRUE) or need resolution with previous
*                                      hit inside the same engine (isFirst = GT_FALSE)
* @param[in] isFirst                  - indication that the resolution is within the same engine or
*                                      with previous engine
* @param[in,out] descrPtr                 - (pointer to) frame descriptor
*                                      RETURN:
*                                      none
*                                      COMMENTS:
*/
extern void snetChtIngressCommandAndCpuCodeResolution
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr,
    IN SKERNEL_EXT_PACKET_CMD_ENT prevCmd,
    IN SKERNEL_EXT_PACKET_CMD_ENT currCmd,
    IN GT_U32                     prevCpuCode,
    IN GT_U32                     currCpuCode,
    IN SNET_CHEETAH_ENGINE_UNIT_ENT engineUnit,
    IN GT_BOOL                    isFirst
);

/**
* @internal snetChtL2Parsing function
* @endinternal
*
* @brief   L2 header Parsing (vlan tag , ethertype , nested vlan , encapsulation)
*         coming from port interface or comming from tunnel termination interface
*         for Ethernet over MPLS.
*/
extern GT_VOID snetChtL2Parsing
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT_FRAME_PARSE_MODE_ENT   parseMode,
    IN INTERNAL_TTI_DESC_INFO_STC        * internalTtiInfoPtr
);

/**
* @internal snetChtL3L4ProtParsing function
* @endinternal
*
* @brief   Parsing of L3 and L4 protocols header
*
* @param[out] descrPtr                 - pointer to the frame's descriptor.
*/
extern GT_VOID snetChtL3L4ProtParsing
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN GT_U32                           etherType,
    IN INTERNAL_TTI_DESC_INFO_STC      *internalTtiInfoPtr
);

/**
* @internal snetChtL3L4ProtParsingResetDesc function
* @endinternal
*
* @brief   reset the fields that involved in Parsing of L3 and L4 protocols header
*         this is needed since the TTI need to reparse the L3,L4 when it terminates
*         the tunnel.
*/
extern GT_VOID snetChtL3L4ProtParsingResetDesc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetChtL3L4ProtCopyDesc function
* @endinternal
*
* @brief   copy ONLY L3/L4 relevant fields from source to target descriptor
*/
extern GT_VOID snetChtL3L4ProtCopyDesc
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    OUT SKERNEL_FRAME_CHEETAH_DESCR_STC * trgDescrPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC * srcDescrPtr
);

/**
* @internal snetChtParsingTrillInnerFrame function
* @endinternal
*
* @brief   L2,3,QOS Parsing for 'Inner frame' of TRILL. need to be done regardless
*         to tunnel termination.
*         the function will save the 'parsing descriptor' in descrPtr->trillInfo.innerFrameDescrPtr
*/
extern GT_VOID snetChtParsingTrillInnerFrame
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN INTERNAL_TTI_DESC_INFO_STC        * internalTtiInfoPtr
);

/**
* @internal snetChtMplsTransitTunnelsProtParsing function
* @endinternal
*
* @brief   Parsing of transit tunnels
*/
GT_VOID snetChtMplsTransitTunnelsProtParsing
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN LION3_MPLS_TRANSIT_TUNNEL_PARSING_MODE_ENT    pmode,
    IN INTERNAL_TTI_DESC_INFO_STC      *internalTtiInfoPtr
);

/**
* @internal snetIngressTablesFormatInit function
* @endinternal
*
* @brief   init the format of ingress tables.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void snetIngressTablesFormatInit(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr
);

/**
* @internal snetLion3PassengerOuterTagIsTag0_1 function
* @endinternal
*
* @brief   set innerTag0Exists,innerPacketTag0Vid,innerPacketTag0CfiDei,innerPacketTag0Up fields in descriptor
*
* @param[in,out] descrPtr                 - pointer to the frame's descriptor.
*/
void  snetLion3PassengerOuterTagIsTag0_1(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr
);

/**
* @internal snetLion3IngressMllAccessCheck function
* @endinternal
*
* @brief   Lion3 : check that IP/L2 MLL is not access out of range.
*         generate interrupt in case of access violation.
* @param[in] devObjPtr                - pointer to device object
* @param[in,out] descrPtr                 - pointer to frame descriptor
* @param[in] usedForIp                - GT_TRUE  - used for IP-MLL
*                                      GT_FALSE - used for L2-MLL
* @param[in] index                    -  into the MLL table
*                                      OUTPUT:
*                                      RETURN:
*                                      indication that did error.
*                                      GT_TRUE  - error (access out of range) , and interrupt was generated.
*                                      the MLL memory should NOT be accessed
*                                      GT_FALSE - no error , can continue MLL processing.
*/
GT_BOOL snetLion3IngressMllAccessCheck
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr,
    IN GT_BOOL      usedForIp,
    IN GT_U32       index
);


/**
* @internal snetXcatGetGreEtherTypes function
* @endinternal
*
* @brief   check if GRE over IPv4/6, and get the 'GRE ethertypes'
*
* @param[in] devObjPtr                - pointer to device object
* @param[in,out] descrPtr                 - pointer to frame's descriptor
*
* @param[out] greEtherTypePtr          - gre etherType, relevant if function returns GT_TRUE(can be NULL)
*                                      gre0EtherTypePtr - gre 0 etherType, GRE protocols that are recognized as Ethernet-over-GRE(can be NULL)
* @param[out] gre1EtherTypePtr         - gre 1 etherType, GRE protocols that are recognized as Ethernet-over-GRE(can be NULL)
*                                      RETURN:
*                                      GT_BOOL      - is gre or not
*                                      COMMENTS:
*/
GT_BOOL snetXcatGetGreEtherTypes
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    OUT   GT_U32                            *greEtherTypePtr,
    OUT   GT_U32                            *gre1EtherTypePtr,
    OUT   GT_U32                            *gre2EtherTypePtr
);

/**
* @internal snetLion3IngressReassignSrcEPort function
* @endinternal
*
* @brief   TRILL/TTI/PCL reassign new src EPort
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame descriptor
* @param[in] clientNamePtr            - the name of the client (for the LOGGER)
* @param[in] newSrcEPort              - the new srcEPort
* @param[in,out] descrPtr                 - pointer to frame descriptor
*                                      RETURN:
*                                      COMMENTS:
*/
GT_VOID snetLion3IngressReassignSrcEPort
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    GT_CHAR*                          clientNamePtr,
    IN    GT_U32                            newSrcEPort
);

/**
* @internal snetChtPerformScibDmaRead function
* @endinternal
*
* @brief   wrap the scibDmaRead to allow the LOG parser to emulate the DMA:
*         write to HOST CPU DMA memory function.
*         Asic is calling this function to write DMA.
* @param[in] clientName               - the DMA client name
* @param[in] deviceId                 - device id. (of the device in the simulation)
* @param[in] address                  - physical  that PP refer to.
*                                      HOST CPU must convert it to HOST memory address
* @param[in] memSize                  - the number of words/bytes (according to dataIsWords)
* @param[in] dataIsWords              - the data to read is words or bytes
*                                      1 - words --> swap network order to cpu order
*                                      0 - bytes --> NO swap network order to cpu order
*
* @param[out] memPtr                   - (pointer to) PP's memory in which HOST CPU memory will be
*                                      copied.
*                                      RETURN:
*                                      COMMENTS:
*/
void snetChtPerformScibDmaRead
(
    IN SNET_CHT_DMA_CLIENT_ENT clientName,
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    OUT GT_U32 * memPtr,
    IN GT_U32  dataIsWords
);

/**
* @internal snetChtPerformScibDmaWrite function
* @endinternal
*
* @brief   wrap the scibDmaWrite to allow the LOG parser to emulate the DMA:
*         write to HOST CPU DMA memory function.
*         Asic is calling this function to write DMA.
* @param[in] clientName               - the DMA client name
* @param[in] deviceId                 - device id. (of the device in the simulation)
* @param[in] address                  - physical  that PP refer to.
*                                      HOST CPU must convert it to HOST memory address
* @param[in] memSize                  - number of words of ASIC memory to write .
* @param[in] memPtr                   - (pointer to) data to write to HOST CPU memory.
* @param[in] dataIsWords              - the data to read is words or bytes
*                                      1 - words --> swap network order to cpu order
*                                      0 - bytes --> NO swap network order to cpu order
*/
void snetChtPerformScibDmaWrite
(
    IN SNET_CHT_DMA_CLIENT_ENT clientName,
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_U32 * memPtr,
    IN GT_U32  dataIsWords
);

/**
* @internal snetChtPerformScibSetInterrupt function
* @endinternal
*
* @brief   wrap the scibSetInterrupt to allow the LOG parser to emulate the interrupt:
*         Generate interrupt for SKernel device.
* @param[in] deviceId                 - ID of device.
*/
void snetChtPerformScibSetInterrupt
(
    IN  GT_U32        deviceId
);
/**
* @internal snetChtPerformScibUnSetInterrupt function
* @endinternal
*
* @brief   wrap the scibSetInterrupt to allow the LOG parser to emulate the de-assert of interrupt.
* @param[in] deviceId                 - ID of device.
*/
void snetChtPerformScibUnSetInterrupt
(
    IN  GT_U32        deviceId
);

/**
* @internal snetChtFromCpuDmaTxQueue function
* @endinternal
*
* @brief   Process transmitted frames per single SDMA queue
*/
GT_VOID snetChtFromCpuDmaTxQueue
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 txQue,
    OUT GT_BOOL *isLastPacketPtr
);

/**
* @internal snetBobcat2EeeProcess function
* @endinternal
*
* @brief   process EEE
*/
void snetBobcat2EeeProcess(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   macPort,
    IN SMAIN_DIRECTION_ENT      direction
);
/**
* @internal snetBobcat2EeeCheckInterrupts function
* @endinternal
*
* @brief   check for EEE interrupts
*/
void snetBobcat2EeeCheckInterrupts(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   macPort
);

/**
* @internal snetChtPortMibOffsetUpdate function
* @endinternal
*
* @brief   update the offset from the start of the MIB memory of the port , for
*         specific MIB counter type
*/
void  snetChtPortMibOffsetUpdate
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    IN GT_U32                           macPort,
    IN GT_U32                           *mibMemoryPtr,
    IN SNET_CHT_PORT_MIB_COUNTERS_ENT   mibType,
    IN GT_U32                           incValue
);
/**
* @internal snetChtPortMibOffsetUpdate_100GMac function
* @endinternal
*
* @brief   for 100G mac (CG port)
*         update the offset from the start of the MIB memory of the port , for
*         specific MIB counter type
*/
void  snetChtPortMibOffsetUpdate_100GMac
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    IN GT_U32                           macPort,
    IN GT_U32                           *mibMemoryPtr,
    IN SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ENT   mibType,
    IN GT_U32                           incValue
);

/**
* @internal snetChtPortMacFieldGet function
* @endinternal
*
* @brief   get the field value from the MAC according to its current 'MAC' used
*/
GT_U32    snetChtPortMacFieldGet
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    IN GT_U32                           macPort,
    IN SNET_CHT_PORT_MAC_FIELDS_ENT     fieldType
);


/**
* @internal snetChtPortMacFieldSet function
* @endinternal
*
* @brief   set field value to the MAC according to its current 'MAC' used
*/
void snetChtPortMacFieldSet
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    IN GT_U32                           macPort,
    IN SNET_CHT_PORT_MAC_FIELDS_ENT     fieldType,
    IN GT_U32                           value
);

/**
* @internal snetHawkMifTxDmaToMacConvert function
* @endinternal
*
* @brief   convert global txdma port num to global mac port num and indication if
*          the port is 'preemptive' and if the MIF enabled (clock and force link).
*          using the MIF tx mapping feature
*
*/
void snetHawkMifTxDmaToMacConvert
(
    IN SKERNEL_DEVICE_OBJECT   *devObjPtr,
    IN GT_U32                  txDmaGlobalPort,
    OUT GT_U32                 *globalMacPtr ,
    OUT GT_U32                 *isPreemptiveChannelPtr,
    OUT GT_U32                 *mif_is_clock_enable_Ptr,
    OUT GT_U32                 *mif_is_channel_force_link_down_Ptr
);

/**
* @internal snetHawkMifIngressMacToRxDmaConvert function
* @endinternal
*
* @brief   convert global mac port num and indication of port is 'preemptive' to
*          local txdma and indication if the MIF enabled (clock and enabled).
*
*/
void snetHawkMifIngressMacToRxDmaConvert
(
    IN SKERNEL_DEVICE_OBJECT   *devObjPtr,
    IN GT_U32                  globalMacNum ,
    IN GT_U32                  isPreemptiveChannel,
    OUT GT_U32                 *mif_is_clock_enable_Ptr,
    OUT GT_U32                 *mif_is_Rx_channel_enable_Ptr,
    OUT GT_U32                 *local_dma_port_number_Ptr
);


/**
* @internal snetChtRxPort function
* @endinternal
*
* @brief   Rx Port layer processing of frames
*/
GT_BOOL snetChtRxPort
(
        IN SKERNEL_DEVICE_OBJECT * devObjPtr,
        IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetChtExtendedPortMacGet function
* @endinternal
*
* @brief   Get MAC number for extended ports.
*/
GT_U32 snetChtExtendedPortMacGet
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32                 port,
    IN GT_BOOL                isRx
);

/**
* @internal snetChtMacCounter1024OrMoreGet function
* @endinternal
*
* @brief   get the counter that need to count the 1024+ bytes of the packet
*         !!! NOT relevant to CG MAC (100G MAC) !!! (because hold different enum for counters!)
*/
SNET_CHT_PORT_MIB_COUNTERS_ENT snetChtMacCounter1024OrMoreGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32  frameSize,
    IN GT_U32  macPort
);

/**
* @internal snetChtFrameProcess_bypassRxMacLayer function
* @endinternal
*
* @brief   Process frames in the Cheetah with MAC layer bypass
*/
GT_VOID snetChtFrameProcess_bypassRxMacLayer
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SBUF_BUF_ID bufferId,
    IN GT_U32 srcPort
);

/**
* @internal snetSip6ChtRxPortDebugRxToCpCounterUpdate function
* @endinternal
*
* @brief   Rx to CP counter update
*/
GT_VOID snetSip6ChtRxPortDebugRxToCpCounterUpdate
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetSip6ChtRxPortDebugCpToRxCounterUpdate function
* @endinternal
*
* @brief   CP to Rx counter update
*/
GT_VOID snetSip6ChtRxPortDebugCpToRxCounterUpdate
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetSip6_30SmuProcess function
* @endinternal
*
* @brief   do the sip6.30 SMU processing.
*/
GT_VOID snetSip6_30SmuProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

/**
* @internal snetSip6_10_8B_Tag_parse function
* @endinternal
*
* @brief   8B Tag parsing support for SIP_6_10 devices
*          (or 6B for SIP6_30)
*
* return GT_OK              - Successfully parsed
*        GT_FAIL            - Error
*        GT_NOT_SUPPORTED   - Not supported
*        GT_NOT_FOUND       - Tag1 was not found (Assign default)
*/
GT_STATUS snetSip6_10_8B_Tag_parse
(
    IN SKERNEL_DEVICE_OBJECT            *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC  *descrPtr,
    IN GT_U32                            ethTypeOffset,
    IN GT_U32                            tpidIndex
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetCheetahIngressh */
