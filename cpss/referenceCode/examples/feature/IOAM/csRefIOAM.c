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

/*******************************************************************************
* csRefIOAM.c
*
* DESCRIPTION:
*  This files provide APIs to configure IOAM nodes - ingress, transit and egress and also the configuration for monitoring port on egress node.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*
*       $Revision: 1 $
*
*******************************************************************************/

#include <string.h>
#include <stdlib.h>

#include "csRefIOAM.h"
#include "../../infrastructure/csRefServices/tcamUtils.h"

#include <cpss/generic/private/prvCpssCommonDefs.h>
/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgPrvEdgeVlan.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnel.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIp.h>
#include <cpss/dxCh/dxChxGen/ptp/cpssDxChPtp.h>

/* count 1's in a binary representation of an integer */
GT_U32 ones(GT_U32 number)
{
  GT_U32 count = 0;

    while(number)
    {
      number &= (number -1);
      count ++;
    }
    return count;
}

/*****************************************************/
/**************** Static Varaibles       *********************/
/*****************************************************/

/* transit ARP MAC address is MAC DA of an IOAM packet that ingresses to Egress node and also the ARP MAC DA for IOAM packet that egresses Transit node,
    where we change MAC DA in additon to adding a record to IOAM header */
static  GT_ETHERADDR       transitArpMacAddr = {{0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0x01}};
static  GT_ETHERADDR       egressArpMacAddr = {{0x00, 0xDA, 0xBE, 0xDE, 0xF4, 0xA7}};
static  GT_ETHERADDR       ingressMacDA = {{0x00, 0x00, 0x00, 0x00, 0x34, 0x02}};
/* transit MAC DA is MAC DA of IOAM packet that ingresses to Transit node and also the TS MAC DA for IOAM packet that egresses Ingress node */
static  GT_ETHERADDR       transitMacDA = {{0x00, 0xEE, 0x11, 0x22, 0x33, 0x55}};
/* MAC DA assigned to packet mirrored in GRE tunnel to a monotor from the Egress node */
static  GT_ETHERADDR       monitorMacDA = {{0x00, 0x99, 0x88, 0x77, 0x66, 0x55}};

/**
* @internal csRefIoamSetIngressMacDa function
* @endinternal
*
* @brief   sets MAC DA exspected of non-IOAM packet ingressing to Ingress Node
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] byte0           - 1-st (most significant) byte of MAC DA address
* @param[in] byte1           - 2-nd byte of MAC DA address
* @param[in] byte2           - 3-d byte of MAC DA address
* @param[in] byte3           - 4-th byte of MAC DA address
* @param[in] byte4           - 5-th byte of MAC DA address
* @param[in] byte5           - 6-th (least significant) byte of MAC DA address
*
* @retval GT_OK                                - on success
*
* @note  must be called before csRefIoamNodeConfigCreate
*
*/
GT_STATUS csRefIoamSetIngressMacDa
(
    IN GT_U8 byte0,
    IN GT_U8 byte1,
    IN GT_U8 byte2,
    IN GT_U8 byte3,
    IN GT_U8 byte4,
    IN GT_U8 byte5
)
{
    ingressMacDA.arEther[0] = byte0;
    ingressMacDA.arEther[1] = byte1;
    ingressMacDA.arEther[2] = byte2;
    ingressMacDA.arEther[3] = byte3;
    ingressMacDA.arEther[4] = byte4;
    ingressMacDA.arEther[5] = byte5;

    return GT_OK;
}

/**
* @internal csRefIoamSetTransitMacDa function
* @endinternal
*
* @brief    sets MAC DA exspected of IOAM packet that ingresses to Transit node and also the TS MAC DA for IOAM packet that egresses Ingress node
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] byte0           - 1-st (most significant) byte of MAC DA address
* @param[in] byte1           - 2-nd byte of MAC DA address
* @param[in] byte2           - 3-d byte of MAC DA address
* @param[in] byte3           - 4-th byte of MAC DA address
* @param[in] byte4           - 5-th byte of MAC DA address
* @param[in] byte5           - 6-th (least significant) byte of MAC DA address
*
* @retval GT_OK                                - on success
*
* @note  must be called before csRefIoamNodeConfigCreate
*
*/
GT_STATUS csRefIoamSetTransitMacDa
(
    IN GT_U8 byte0,
    IN GT_U8 byte1,
    IN GT_U8 byte2,
    IN GT_U8 byte3,
    IN GT_U8 byte4,
    IN GT_U8 byte5
)
{
    transitMacDA.arEther[0] = byte0;
    transitMacDA.arEther[1] = byte1;
    transitMacDA.arEther[2] = byte2;
    transitMacDA.arEther[3] = byte3;
    transitMacDA.arEther[4] = byte4;
    transitMacDA.arEther[5] = byte5;

    return GT_OK;
}

/**
* @internal csRefIoamSetEgressMacDa function
* @endinternal
*
* @brief   sets  MAC DA exspected of an IOAM packet that ingresses to Egress node and also the ARP MAC DA for IOAM packet that egresses Transit node,
    where we change MAC DA in additon to adding a record to IOAM header
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] byte0           - 1-st (most significant) byte of MAC DA address
* @param[in] byte1           - 2-nd byte of MAC DA address
* @param[in] byte2           - 3-d byte of MAC DA address
* @param[in] byte3           - 4-th byte of MAC DA address
* @param[in] byte4           - 5-th byte of MAC DA address
* @param[in] byte5           - 6-th (least significant) byte of MAC DA address
*
* @retval GT_OK                                - on success
*
* @note  must be called before csRefIoamNodeConfigCreate
*
*/
GT_STATUS csRefIoamSetEgressMacDa
(
    IN GT_U8 byte0,
    IN GT_U8 byte1,
    IN GT_U8 byte2,
    IN GT_U8 byte3,
    IN GT_U8 byte4,
    IN GT_U8 byte5
)
{
    transitArpMacAddr.arEther[0] = byte0;
    transitArpMacAddr.arEther[1] = byte1;
    transitArpMacAddr.arEther[2] = byte2;
    transitArpMacAddr.arEther[3] = byte3;
    transitArpMacAddr.arEther[4] = byte4;
    transitArpMacAddr.arEther[5] = byte5;

    return GT_OK;
}

/**
* @internal csRefIoamSetMonitorMacDa function
* @endinternal
*
* @brief   sets MAC DA assigned to packet mirrored in GRE tunnel to a monitor from the Egress node
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] byte0           - 1-st (most significant) byte of MAC DA address
* @param[in] byte1           - 2-nd byte of MAC DA address
* @param[in] byte2           - 3-d byte of MAC DA address
* @param[in] byte3           - 4-th byte of MAC DA address
* @param[in] byte4           - 5-th byte of MAC DA address
* @param[in] byte5           - 6-th (least significant) byte of MAC DA address
*
* @retval GT_OK                                - on success
*
* @note  must be called before csRefIoamNodeConfigCreate
*
*/
GT_STATUS csRefIoamSetMonitorMacDa
(
    IN GT_U8 byte0,
    IN GT_U8 byte1,
    IN GT_U8 byte2,
    IN GT_U8 byte3,
    IN GT_U8 byte4,
    IN GT_U8 byte5
)
{
    monitorMacDA.arEther[0] = byte0;
    monitorMacDA.arEther[1] = byte1;
    monitorMacDA.arEther[2] = byte2;
    monitorMacDA.arEther[3] = byte3;
    monitorMacDA.arEther[4] = byte4;
    monitorMacDA.arEther[5] = byte5;

    return GT_OK;
}


/* configure interface for monitor on Egress node */
static GT_STATUS prvIoamSetTxMirroring
(
    IN GT_U8                devNum,
    IN GT_PORT_NUM mirrorTxPort,
    IN GT_PHYSICAL_PORT_NUM analyzerTxPort,
    IN GT_BOOL enablePHA,
    IN GT_BOOL mirrorAsIngress
)
{
    GT_STATUS                               rc;
    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC interfaceMirror;
    CPSS_INTERFACE_INFO_STC                 eToPhy;
    GT_U32                                  analyzerTxIndex = analyzerTxPort%7;
    GT_PORT_NUM                             eportTxAnalyzer = 300+analyzerTxIndex;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC       commonInfo;
    CPSS_DXCH_PHA_THREAD_INFO_UNT              extInfo;
    GT_U32                                     phaThreadBEgressIndex = 7;
    CPSS_DXCH_PHA_THREAD_TYPE_ENT              threadType;
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT          tunnelStartConf;
/*    GT_ETHERADDR                               tsMacDa = {{0x00, 0x99, 0x88, 0x77, 0x66, 0x55}}; */
    GT_U32                                     tunnelStartInd = 20;
    GT_U32                                     i;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC         egressInfo;

    cpssOsPrintf("analyzerTxIndex %d, eportTxAnalyzer %d\n", analyzerTxIndex, eportTxAnalyzer);

    rc = cpssDxChMirrorAnalyzerMirrorOnDropEnableSet(devNum,analyzerTxIndex,GT_FALSE);
    if( GT_OK != rc)
       return rc;

/*  we consider mirrorTxPort  ePort */
    rc = cpssDxChMirrorTxPortSet(devNum, mirrorTxPort, GT_FALSE, GT_TRUE, analyzerTxIndex);
    if( GT_OK != rc)
       return rc;

    interfaceMirror.interface.type = CPSS_INTERFACE_PORT_E;
    interfaceMirror.interface.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(0);
    interfaceMirror.interface.devPort.portNum = eportTxAnalyzer; /* can be an ePort */

    rc = cpssDxChMirrorAnalyzerInterfaceSet(devNum, analyzerTxIndex, &interfaceMirror);
    if( GT_OK != rc)
       return rc;

    cpssOsMemSet(&eToPhy,0,sizeof(CPSS_INTERFACE_INFO_STC));
    eToPhy.type = CPSS_INTERFACE_PORT_E;
    eToPhy.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(0);
    eToPhy.devPort.portNum = analyzerTxPort;
    rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum, eportTxAnalyzer, &eToPhy);
    if( GT_OK != rc)
      return rc;

    /***********  define IPV6 GRE tunnel header *******************/
    cpssOsMemSet(&tunnelStartConf,0,sizeof(tunnelStartConf));

    for(i=0;i<6;i++)
        tunnelStartConf.ipv6Cfg.macDa.arEther[i] = monitorMacDA.arEther[i];
    tunnelStartConf.ipv6Cfg.destIp.u32Ip[0]  = 0;
    tunnelStartConf.ipv6Cfg.destIp.u32Ip[1]  = 0;
    tunnelStartConf.ipv6Cfg.destIp.u32Ip[2]  = 0x58585858;
    tunnelStartConf.ipv6Cfg.destIp.u32Ip[3]  = 0x31313131;
    tunnelStartConf.ipv6Cfg.srcIp.u32Ip[0]   = 0;
    tunnelStartConf.ipv6Cfg.srcIp.u32Ip[1]   = 0;
    tunnelStartConf.ipv6Cfg.srcIp.u32Ip[2]   = 0x79797979;
    tunnelStartConf.ipv6Cfg.srcIp.u32Ip[3]   = 0xDEDEDEDE;
    tunnelStartConf.ipv6Cfg.ttl              = 0x80;
    tunnelStartConf.ipv6Cfg.ipHeaderProtocol = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E;
    tunnelStartConf.ipv6Cfg.profileIndex     = 0; /* for extension - no need for 4 byte GRE header */
    tunnelStartConf.ipv6Cfg.greProtocolForEthernet   = 0x86DD;
    tunnelStartConf.ipv6Cfg.greFlagsAndVersion       = 0;
    tunnelStartConf.ipv6Cfg.tagEnable        = GT_FALSE;
    tunnelStartConf.ipv6Cfg.vlanId           = 0;

    rc = cpssDxChTunnelStartEntrySet(devNum, tunnelStartInd, CPSS_TUNNEL_GENERIC_IPV6_E, &tunnelStartConf);
    if( GT_OK != rc)
      return rc;

    /* set ePort info */
    cpssOsMemSet(&egressInfo,0,sizeof(CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC));
    egressInfo.tsPassengerPacketType    = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
    egressInfo.tunnelStart              = GT_TRUE;
    egressInfo.tunnelStartPtr           = tunnelStartInd;

    rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, eportTxAnalyzer, &egressInfo);
    if( GT_OK != rc)
      return rc;

    /*************** egress thread B**********************************/
    if(enablePHA == GT_TRUE)
    {
        threadType = CPSS_DXCH_PHA_THREAD_TYPE_IOAM_EGRESS_SWITCH_IPV6_E;
        cpssOsPrintf("set thread ID %d for port %d\n", threadType, eportTxAnalyzer);
        rc = cpssDxChPhaPortThreadIdSet(devNum, eportTxAnalyzer, GT_TRUE, phaThreadBEgressIndex);
        if( GT_OK != rc)
            return rc;

        cpssOsMemSet(&commonInfo,0,sizeof(CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC));
        cpssOsMemSet(&extInfo,0,sizeof(CPSS_DXCH_PHA_THREAD_INFO_UNT));
        commonInfo.stallDropCode =  CPSS_NET_FIRST_USER_DEFINED_E;
        commonInfo.statisticalProcessingFactor = 0;
        commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;

        extInfo.ioamTransitSwitchIpv6.node_id         = 19;

        rc = cpssDxChPhaThreadIdEntrySet(devNum, phaThreadBEgressIndex, &commonInfo,
            threadType, &extInfo);
        if( GT_OK != rc)
            return rc;

    }
    else
    {
        rc = cpssDxChPhaPortThreadIdSet(devNum, eportTxAnalyzer, GT_FALSE, 0);
        if( GT_OK != rc)
            return rc;
    }

    rc = cpssDxChMirrorTreatEgressMirroredAsIngressMirroredEnableSet(devNum, analyzerTxIndex, mirrorAsIngress);
    if( GT_OK != rc)
        return rc;

    return rc;
}



/**
* @internal csRefIoamNodeConfigCreate function
* @endinternal
*
* @brief   The function creates configuration for IOAM node. the switch can serve as either ingress, transit or egress node, nodes are recognized by
*             MAC DA of incoming packets
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum          - device number
* @param[in] ingressPort        port number where IOAM packet enters
* @param[in] egressPort         port number where IOAM packet egresses
* @param[in] analyzerTxPort   port number where IOAM packet is copied to a monitoring interface when switch workes as an egress node
* @param[in] traceTypeBmp - trace bitmap in IOAM header - defines wht type of information will be added by the node to the IOAM header
*
* @retval GT_OK                                    - on success
* @retval GT_BAD_PARAM                      - on wrong parameters
* @retval GT_OUT_OF_RANGE                 - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
* @retval GT_FAIL                                  - otherwise
*
* @note  demostrates configuration for IOAM using PHA; Same ingress port serves for any IOAM node role
*          use like: csRefIoamNodeConfigCreate 0,0,36,32,0xfa00
*/
GT_STATUS csRefIoamNodeConfigCreate
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM ingressPort,
    IN GT_PHYSICAL_PORT_NUM egressPort,
    IN GT_PHYSICAL_PORT_NUM analyzerTxPort,
    IN GT_U32               traceTypeBmp
)
{
    CPSS_MAC_ENTRY_EXT_STC                     macEntry;
    CPSS_INTERFACE_INFO_STC                    eToPhy;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC         egressInfo;
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT          tunnelStartConf;
    CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC  profileData;
    GT_U32                                     phaThreadIngressId = 2, phaThreadTransitId = 3, phaThreadIngressIdIPv4 = 4, phaThreadTransitIdIPv4 = 5, phaThreadEgressId = 6;
    GT_U32                                     profileIndex = 1, headerLength;
/* we make sure that ARP and TS indexes not overlap */
    GT_U32                                     tunnelStartInd = 4, tunnelStartIndIPv4 = 6, arpIndex = 11, arpEgressIndex = 13, i;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC       commonInfo;
    CPSS_DXCH_PHA_THREAD_INFO_UNT              extInfo;
    GT_STATUS                                  rc;
    GT_ETHERADDR                               srcMac;
    GT_PORT_NUM                                eportIngress=100, eportIngressIPv4 = 101, eportTransit=200, eportTransitIPv4=201, eportEgress=400;
/*    GT_ETHERADDR                                tsMacDa  = {{0x00, 0xEE, 0x11, 0x22, 0x33, 0x55}};*/
    GT_ETHERADDR                               tsMacDaIPv4 = {{0x00, 0xFF, 0x66, 0x77, 0x88, 0x99}};
    CPSS_DXCH_PTP_TSU_TIMESTAMP_MASKING_PROFILE_STC  maskingProfile;
    /* enablePHA and debug_VXLAN are for debug purposes - see how used in code */
    GT_BOOL                                    enablePHA = GT_TRUE;
    GT_BOOL                                    changeGOP = GT_TRUE;

    cpssOsPrintf("Enter ioamPortConfig 9\n");

/***********  define packet forwarding interface - FDB or PVE *******************/
/* transit/ ingress and IPv4/IPV6 tunnel are diffirentiated by MAC DA */
    cpssOsMemSet(&macEntry, 0, sizeof(CPSS_MAC_ENTRY_EXT_STC));
    macEntry.daRoute = GT_FALSE;
    macEntry.isStatic = GT_TRUE;
    for(i=0; i<6; i++)
        macEntry.key.key.macVlan.macAddr.arEther[i] = ingressMacDA.arEther[i];
    macEntry.key.key.macVlan.vlanId = 1;
    macEntry.key.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(0);
    macEntry.dstInterface.devPort.portNum = eportIngress;

    rc = cpssDxChBrgFdbMacEntrySet(devNum, &macEntry);
    if(rc != GT_OK)
        return rc;
    cpssOsTimerWkAfter(100);
    /* IPv4 TS case */
    macEntry.key.key.macVlan.macAddr.arEther[5] = 0x03;
    macEntry.dstInterface.devPort.portNum = eportIngressIPv4;
    rc = cpssDxChBrgFdbMacEntrySet(devNum, &macEntry);
    if(rc != GT_OK)
        return rc;

    /* IPv6 transit case */
    /* we need to wait for the previous address to be written */
    cpssOsTimerWkAfter(100);
    for(i=0;i<6;i++)
        macEntry.key.key.macVlan.macAddr.arEther[i] = transitMacDA.arEther[i];
    macEntry.dstInterface.devPort.portNum = eportTransit;

    rc = cpssDxChBrgFdbMacEntrySet(devNum, &macEntry);
    if(rc != GT_OK)
        return rc;

    /* IPv4 transit case */

    cpssOsTimerWkAfter(100);
    for(i=0;i<6;i++)
        macEntry.key.key.macVlan.macAddr.arEther[i] = tsMacDaIPv4.arEther[i];
    macEntry.dstInterface.devPort.portNum = eportTransitIPv4;

    rc = cpssDxChBrgFdbMacEntrySet(devNum, &macEntry);
    if(rc != GT_OK)
        return rc;

    cpssOsTimerWkAfter(100);
    for(i=0;i<6;i++)
        macEntry.key.key.macVlan.macAddr.arEther[i] = transitArpMacAddr.arEther[i];
    macEntry.dstInterface.devPort.portNum = eportEgress;
    rc = cpssDxChBrgFdbMacEntrySet(devNum, &macEntry);
    if(rc != GT_OK)
        return rc;


GT_UNUSED_PARAM(ingressPort);
#if 0
/* PVE option - not used for now */
    rc = cpssDxChBrgPrvEdgeVlanEnable(devNum, GT_TRUE);
    if(rc != GT_OK)
        return rc;

    rc = cpssDxChBrgPrvEdgeVlanPortEnable(devNum,
        ingressPort,
        GT_TRUE,
        egressPort, PRV_CPSS_HW_DEV_NUM_MAC(0), GT_FALSE);
    if(rc != GT_OK)
        return rc;
#endif
/*****************  mapping of ePort to physical port  *******************/

    cpssOsMemSet(&eToPhy,0,sizeof(CPSS_INTERFACE_INFO_STC));
    eToPhy.type = CPSS_INTERFACE_PORT_E;
    eToPhy.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(0);
    eToPhy.devPort.portNum = egressPort;
    rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum, eportIngress, &eToPhy);
    if( GT_OK != rc)
      return rc;

    rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum, eportTransit, &eToPhy);
    if( GT_OK != rc)
      return rc;

    rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum, eportIngressIPv4, &eToPhy);
    if( GT_OK != rc)
      return rc;

    rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum, eportTransitIPv4, &eToPhy);
    if( GT_OK != rc)
      return rc;

    rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum, eportEgress, &eToPhy);
    if( GT_OK != rc)
      return rc;

    /***********  define tunnel header for ingress and ARP for transit case*******************/
    cpssOsMemSet(&tunnelStartConf,0,sizeof(tunnelStartConf));

    for(i=0;i<6;i++)
        tunnelStartConf.ipv6Cfg.macDa.arEther[i] = transitMacDA.arEther[i];
    tunnelStartConf.ipv6Cfg.destIp.u32Ip[0]  = 0;
    tunnelStartConf.ipv6Cfg.destIp.u32Ip[1]  = 0;
    tunnelStartConf.ipv6Cfg.destIp.u32Ip[2]  = 0x22222222;
    tunnelStartConf.ipv6Cfg.destIp.u32Ip[3]  = 0x33333333;
    tunnelStartConf.ipv6Cfg.srcIp.u32Ip[0]   = 0;
    tunnelStartConf.ipv6Cfg.srcIp.u32Ip[1]   = 0;
    tunnelStartConf.ipv6Cfg.srcIp.u32Ip[2]   = 0x55555555;
    tunnelStartConf.ipv6Cfg.srcIp.u32Ip[3]   = 0x77777777;
    tunnelStartConf.ipv6Cfg.ttl              = 0x64;
    tunnelStartConf.ipv6Cfg.ipHeaderProtocol = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
    tunnelStartConf.ipv6Cfg.profileIndex     = profileIndex; /* for extension */
    tunnelStartConf.ipv6Cfg.udpDstPort       = 4790;
    tunnelStartConf.ipv6Cfg.udpSrcPort       = 122; /* will be overwritten by extention */
    tunnelStartConf.ipv6Cfg.tagEnable        = GT_TRUE;
    tunnelStartConf.ipv6Cfg.vlanId           = 1;

    rc = cpssDxChTunnelStartEntrySet(devNum, tunnelStartInd, CPSS_TUNNEL_GENERIC_IPV6_E, &tunnelStartConf);
    if( GT_OK != rc)
      return rc;

    cpssOsMemSet(&tunnelStartConf,0,sizeof(tunnelStartConf));

    for(i=0;i<6;i++)
        tunnelStartConf.ipv4Cfg.macDa.arEther[i] = tsMacDaIPv4.arEther[i];
    tunnelStartConf.ipv4Cfg.destIp.arIP[0]  = 0;
    tunnelStartConf.ipv4Cfg.destIp.arIP[1]  = 1;
    tunnelStartConf.ipv4Cfg.destIp.arIP[2]  = 2;
    tunnelStartConf.ipv4Cfg.destIp.arIP[3]  = 3;
    tunnelStartConf.ipv4Cfg.srcIp.arIP[0]   = 0x77;
    tunnelStartConf.ipv4Cfg.srcIp.arIP[1]   = 0x77;
    tunnelStartConf.ipv4Cfg.srcIp.arIP[2]   = 0x77;
    tunnelStartConf.ipv4Cfg.srcIp.arIP[3]   = 0x77;
    tunnelStartConf.ipv4Cfg.ttl              = 0x64;
    tunnelStartConf.ipv4Cfg.ipHeaderProtocol = CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
    tunnelStartConf.ipv4Cfg.profileIndex     = profileIndex; /* for extension */
    tunnelStartConf.ipv4Cfg.udpDstPort       = 4790;
    tunnelStartConf.ipv4Cfg.udpSrcPort       = 122; /* will be overwritten by extention */
    tunnelStartConf.ipv4Cfg.tagEnable        = GT_TRUE;
    tunnelStartConf.ipv4Cfg.vlanId           = 1;
    rc = cpssDxChTunnelStartEntrySet(devNum, tunnelStartIndIPv4, CPSS_TUNNEL_GENERIC_IPV4_E, &tunnelStartConf);
    if( GT_OK != rc)
      return rc;

    /************************************ set ePort info **************************************/
    cpssOsMemSet(&egressInfo,0,sizeof(CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC));
    egressInfo.tsPassengerPacketType    = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
    egressInfo.tunnelStart              = GT_TRUE;
    egressInfo.tunnelStartPtr           = tunnelStartInd;

    rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, eportIngress, &egressInfo);
    if( GT_OK != rc)
      return rc;

    egressInfo.tunnelStartPtr           = tunnelStartIndIPv4;

    rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, eportIngressIPv4, &egressInfo);
    if( GT_OK != rc)
      return rc;

    /************************************ set ARP ******************************************* */
    rc = cpssDxChIpRouterArpAddrWrite(devNum,arpIndex,&transitArpMacAddr);
    if(rc != GT_OK)
        return rc;

    rc = cpssDxChIpRouterArpAddrWrite(devNum,arpEgressIndex,&egressArpMacAddr);
    if(rc != GT_OK)
        return rc;

/* set ePort info */
    cpssOsMemSet(&egressInfo,0,sizeof(CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC));
    egressInfo.modifyMacSa = GT_TRUE;
    egressInfo.modifyMacDa = GT_TRUE;
    egressInfo.arpPtr = arpIndex;

    rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, eportTransit, &egressInfo);
    if( GT_OK != rc)
      return rc;

    cpssOsMemSet(&egressInfo,0,sizeof(CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC));
    egressInfo.modifyMacSa = GT_TRUE;
    egressInfo.modifyMacDa = GT_TRUE;
    egressInfo.arpPtr = arpIndex;
    rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, eportTransitIPv4, &egressInfo);
    if( GT_OK != rc)
      return rc;

    cpssOsMemSet(&egressInfo,0,sizeof(CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC));
    egressInfo.modifyMacSa = GT_TRUE;
    egressInfo.modifyMacDa = GT_TRUE;
    egressInfo.arpPtr = arpEgressIndex;
    rc = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, eportEgress, &egressInfo);
    if( GT_OK != rc)
      return rc;

    /***********  define tunnel header extension VX LAN*******************/
    cpssOsMemSet(&profileData,0,sizeof(CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC));

    profileData.udpSrcPortMode = CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E;
    profileData.templateDataSize = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_16B_E;
    /* note, that bytes are count from start of the packet, but bits inside a byte are counted from least significant bit, so 01:80 should be
           coded as templateDataBitsCfg[0] = 1, templateDataBitsCfg[15] = 1 */
#if 0
    profileData.templateDataBitsCfg[0] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;
    profileData.templateDataBitsCfg[127] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;
#endif
    /* Code first byte - 0x0F */
    /* I-bit */
    profileData.templateDataBitsCfg[0] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;
    /* P-bit */
    profileData.templateDataBitsCfg[1] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;
    /* B-bit  - not sure if should set it*/
    profileData.templateDataBitsCfg[2] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;
    /* 0-bit */
    profileData.templateDataBitsCfg[3] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;
    /* Code 4-th byte - 0x81    */
    /* Next protocol: IOAM is 0x81 per RFC 5226 and "Generic Protocol Extension for VXLAN draft-ietf-nvo3-vxlan-gpe-07"*/
    profileData.templateDataBitsCfg[24] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;
    profileData.templateDataBitsCfg[31] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;
    /* VNI - use arbitrary, 0x18 in our case*/
    profileData.templateDataBitsCfg[51] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;
    profileData.templateDataBitsCfg[52] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;
    profileData.templateDataBitsCfg[63] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;
    profileData.templateDataBitsCfg[55] = CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E;

    rc = cpssDxChTunnelStartGenProfileTableEntrySet(devNum, profileIndex, &profileData);
    if( GT_OK != rc)
      return rc;

    if(enablePHA == GT_FALSE)
    {
        rc = cpssDxChPhaPortThreadIdSet(devNum, eportIngress, GT_FALSE, 0);
        rc = cpssDxChPhaPortThreadIdSet(devNum, eportTransit, GT_FALSE, 0);
        rc = cpssDxChPhaPortThreadIdSet(devNum, eportIngressIPv4, GT_FALSE, 0);
        rc = cpssDxChPhaPortThreadIdSet(devNum, eportTransitIPv4, GT_FALSE, 0);
        return rc;
    }

    /*******************  configure change of MAC SA for tunnel ***********************************/
    srcMac.arEther[0] = 0;
    srcMac.arEther[1] = 0x11;
    srcMac.arEther[2] = 0x12;
    srcMac.arEther[3] = 0x13;
    srcMac.arEther[4] = 0x14;
    srcMac.arEther[5] = 0x15;

    rc = cpssDxChIpRouterMacSaModifyEnable(devNum,egressPort, GT_TRUE);
    if(rc != GT_OK)
        return rc;

    rc = cpssDxChIpRouterMacSaBaseSet(devNum,&srcMac);
    if(rc != GT_OK)
        return rc;
    rc = cpssDxChIpPortRouterMacSaLsbModeSet(devNum, egressPort, CPSS_SA_LSB_FULL_48_BIT_GLOBAL);
    if(rc != GT_OK)
        return rc;

    rc = cpssDxChIpRouterGlobalMacSaSet(devNum, 0, &srcMac);
    if(rc != GT_OK)
        return rc;

    rc = cpssDxChIpRouterPortGlobalMacSaIndexSet(devNum, egressPort, 0); /* should change param to Physical port ? */
    if(rc != GT_OK)
        return rc;

/***************  trigger PHA  ************************************/
    headerLength = ones(traceTypeBmp) + 2;
    cpssOsPrintf("header length should be %d\n",headerLength);

   /*************** ingress thread IPv6**********************************/
    rc = cpssDxChPhaPortThreadIdSet(devNum, eportIngress, GT_TRUE, phaThreadIngressId);
    if( GT_OK != rc)
      return rc;

    cpssOsMemSet(&commonInfo,0,sizeof(CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC));
    cpssOsMemSet(&extInfo,0,sizeof(CPSS_DXCH_PHA_THREAD_INFO_UNT));
    commonInfo.stallDropCode =  CPSS_NET_FIRST_USER_DEFINED_E;
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;

    extInfo.ioamIngressSwitchIpv6.Type1           = 0x1; /* incremental trace option */
    extInfo.ioamIngressSwitchIpv6.IOAM_HDR_len1   = headerLength;
    extInfo.ioamIngressSwitchIpv6.Reserved1       = 0;
    extInfo.ioamIngressSwitchIpv6.Next_Protocol1  = 1 /*0x0800*/;
    extInfo.ioamIngressSwitchIpv6.IOAM_Trace_Type = traceTypeBmp; /* can be 0xfa00, 0xf200 (no delay), 0xfE00 (extra entry for clearing */
    extInfo.ioamIngressSwitchIpv6.Maximum_Length  = 128;
    extInfo.ioamIngressSwitchIpv6.Flags           = 0;
    extInfo.ioamIngressSwitchIpv6.node_id         = 15;
    extInfo.ioamIngressSwitchIpv6.Hop_Lim         = 10;

    rc = cpssDxChPhaThreadIdEntrySet(devNum, phaThreadIngressId, &commonInfo,
        CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E, &extInfo);
    if( GT_OK != rc)
      return rc;

   /*************** transit thread **********************************/

    rc = cpssDxChPhaPortThreadIdSet(devNum, eportTransit, GT_TRUE, phaThreadTransitId);
    if( GT_OK != rc)
      return rc;

    cpssOsMemSet(&commonInfo,0,sizeof(CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC));
    cpssOsMemSet(&extInfo,0,sizeof(CPSS_DXCH_PHA_THREAD_INFO_UNT));
    commonInfo.stallDropCode =  CPSS_NET_FIRST_USER_DEFINED_E;
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;

    extInfo.ioamTransitSwitchIpv6.node_id         = 17;

    rc = cpssDxChPhaThreadIdEntrySet(devNum, phaThreadTransitId, &commonInfo,
        CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV6_E, &extInfo);
    if( GT_OK != rc)
      return rc;

    /*************** ingress thread IPv4 **********************************/

    rc = cpssDxChPhaPortThreadIdSet(devNum, eportIngressIPv4, GT_TRUE, phaThreadIngressIdIPv4);
    if( GT_OK != rc)
      return rc;

    cpssOsMemSet(&commonInfo,0,sizeof(CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC));
    cpssOsMemSet(&extInfo,0,sizeof(CPSS_DXCH_PHA_THREAD_INFO_UNT));
    commonInfo.stallDropCode =  CPSS_NET_FIRST_USER_DEFINED_E;
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;

    extInfo.ioamIngressSwitchIpv4.Type1           = 0x1; /* incremental trace option */
    extInfo.ioamIngressSwitchIpv4.IOAM_HDR_len1   = headerLength;
    extInfo.ioamIngressSwitchIpv4.Reserved1       = 0;
    extInfo.ioamIngressSwitchIpv4.Next_Protocol1  = 1 /*0x0800*/;
    extInfo.ioamIngressSwitchIpv4.IOAM_Trace_Type = traceTypeBmp; /* can be 0xfa00, 0xf200 (no delay), 0xfE00 (extra entry for clearing */
    /* check behavior without delay record */
    extInfo.ioamIngressSwitchIpv4.Maximum_Length  = 128;
    extInfo.ioamIngressSwitchIpv4.Flags           = 0;
    extInfo.ioamIngressSwitchIpv4.node_id         = 19;
    extInfo.ioamIngressSwitchIpv4.Hop_Lim         = 10;

    rc = cpssDxChPhaThreadIdEntrySet(devNum, phaThreadIngressIdIPv4, &commonInfo,
        CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E, &extInfo);
    if( GT_OK != rc)
      return rc;

   /*************** transit thread IPv4 **********************************/

    rc = cpssDxChPhaPortThreadIdSet(devNum, eportTransitIPv4, GT_TRUE, phaThreadTransitIdIPv4);
    if( GT_OK != rc)
      return rc;

    cpssOsMemSet(&commonInfo,0,sizeof(CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC));
    cpssOsMemSet(&extInfo,0,sizeof(CPSS_DXCH_PHA_THREAD_INFO_UNT));
    commonInfo.stallDropCode =  CPSS_NET_FIRST_USER_DEFINED_E;
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;

    extInfo.ioamTransitSwitchIpv4.node_id         = 21;

    rc = cpssDxChPhaThreadIdEntrySet(devNum, phaThreadTransitIdIPv4, &commonInfo,
        CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E, &extInfo);
    if( GT_OK != rc)
      return rc;

/*************** egress thread A**********************************/
    rc = cpssDxChPhaPortThreadIdSet(devNum, eportEgress, GT_TRUE, phaThreadEgressId);
    if( GT_OK != rc)
        return rc;

    cpssOsMemSet(&commonInfo,0,sizeof(CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC));
    cpssOsMemSet(&extInfo,0,sizeof(CPSS_DXCH_PHA_THREAD_INFO_UNT));
    commonInfo.stallDropCode =  CPSS_NET_FIRST_USER_DEFINED_E;
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;

    rc = cpssDxChPhaThreadIdEntrySet(devNum, phaThreadEgressId, &commonInfo,
        CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_EGRESS_SWITCH_E, &extInfo);
    if( GT_OK != rc)
    return rc;
/******************* change GOP to appy transit timestamp in correct position *****************/
    if(changeGOP)
    {
    /* masking profile 2 is set by PHA - as only PHA FW has ability to set masking porofile on a descriptor */
        rc = cpssDxChPtpTsuTimeStampMaskingProfileGet(devNum, egressPort, 2, &maskingProfile);
        if( GT_OK != rc)
          return rc;
        cpssOsPrintf("Changing masking Profile from numOfBits %d, numOfLsbits %d, numOfShifts %d\n",
            maskingProfile.numOfBits, maskingProfile.numOfLsbits, maskingProfile.numOfShifts);
        maskingProfile.numOfBits = 32;
        maskingProfile.numOfLsbits = 31;
        maskingProfile.numOfShifts = 16;

        rc = cpssDxChPtpTsuTimeStampMaskingProfileSet(devNum, egressPort, 2, &maskingProfile);
        if( GT_OK != rc)
          return rc;

        rc = cpssDxChPtpTsuTimeStampMaskingProfileGet(devNum, analyzerTxPort, 2, &maskingProfile);
        if( GT_OK != rc)
          return rc;
        cpssOsPrintf("Changing masking Profile from numOfBits %d, numOfLsbits %d, numOfShifts %d\n",
            maskingProfile.numOfBits, maskingProfile.numOfLsbits, maskingProfile.numOfShifts);
        maskingProfile.numOfBits = 32;
        maskingProfile.numOfLsbits = 31;
        maskingProfile.numOfShifts = 16;

        rc = cpssDxChPtpTsuTimeStampMaskingProfileSet(devNum, analyzerTxPort, 2, &maskingProfile);
        if( GT_OK != rc)
          return rc;
    }

    /******************* set Mirror for egress *****************/
    rc = prvIoamSetTxMirroring(devNum, eportEgress, analyzerTxPort, GT_TRUE, GT_TRUE);
    if( GT_OK != rc)
      return rc;

    return GT_OK;

}


/**
* @internal csRefIoamioamEgressConfig function
* @endinternal
*
* @brief   The function creates configuration for egress IOAM node.
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number
* @param[in] ingressPort         port number where IOAM packet enters
* @param[in] egressPort          port number where IOAM packet egresses
* @param[in] txAnalyzerPort   port number where IOAM packet is copied to a monitoring interface when switch works as an egress node
* @param[in] ttHeaderLength        - total length of IOAM header - how many bytes to cut in order to do correct tunnel termination
*
* @retval GT_OK                                - on success
* @retval GT_BAD_PARAM                    - on wrong parameters
* @retval GT_OUT_OF_RANGE               - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
* @retval GT_FAIL                              - otherwise
*
* @note  don't use the function for now - egress functionality with regard to analyzer port is done by csRefIoamNodeConfigCreate.
*            Because of an errata TT and egress mirroring cannot work simultaneously o the same port
*          use like: csRefIoamioamEgressConfig 0,48,56,51,72
*/

GT_STATUS csRefIoamioamEgressConfig
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM ingressPort,
    IN GT_PHYSICAL_PORT_NUM egressPort,
    IN GT_PHYSICAL_PORT_NUM txAnalyzerPort,
    IN GT_U32               ttHeaderLength
)
{
    GT_STATUS                           rc=GT_OK;
    GT_U32                              tcamIndex;
    GT_U32                              firstIndexTTI, firstIndexIPCL, firstIndexEPCL;
    GT_U32                              i;
    CPSS_DXCH_TTI_RULE_UNT              pattern;
    CPSS_DXCH_TTI_RULE_UNT              mask;
    CPSS_DXCH_TTI_ACTION_STC            action;
    CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType, keyTypeIpV4;
    CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType;
    GT_PORT_NUM                         eportEgress=400;
    CPSS_INTERFACE_INFO_STC             eToPhy;
    GT_BOOL                             enTT = GT_TRUE;
    GT_U32      regAddr;     /* register address */
    GT_U8 ipclBlocks = 8;
    GT_U8 epclBlocks = 2;
    GT_U8 ttiBlocks = 6;


    regAddr = PRV_DXCH_REG1_UNIT_HA_MAC(0).haGlobalConfig2 + 8;

    rc = csRefInfraFindTCAMFirstRule(devNum, &firstIndexIPCL, &firstIndexEPCL, &firstIndexTTI);
    if(rc != GT_OK)
        return rc;

    tcamIndex = firstIndexTTI;
    rc = csRefInfraTCAMCountersSet(0,ipclBlocks,epclBlocks,ttiBlocks);
    if(rc != GT_OK)
        return rc;


    keyType = CPSS_DXCH_TTI_KEY_UDB_IPV6_UDP_E;
    keyTypeIpV4 = CPSS_DXCH_TTI_KEY_UDB_IPV4_UDP_E;
    ruleType = CPSS_DXCH_TTI_RULE_UDB_10_E;

    rc = cpssDxChTtiPacketTypeKeySizeSet(devNum, keyType, CPSS_DXCH_TTI_KEY_SIZE_10_B_E);
    if(rc != GT_OK)
        return rc;

    rc = cpssDxChTtiPacketTypeKeySizeSet(devNum, keyTypeIpV4, CPSS_DXCH_TTI_KEY_SIZE_10_B_E);
    if(rc != GT_OK)
        return rc;

    for(i=0;i<6;i++)
    {
        /* UDB for MAC DA, all bytes */
        rc = cpssDxChTtiUserDefinedByteSet(devNum, keyType, i,
                                           CPSS_DXCH_TTI_OFFSET_L2_E, i);
        if(rc != GT_OK)
            return rc;
    }

    /* UDB for source ePort*/
    rc = cpssDxChTtiUserDefinedByteSet(devNum, keyType, 6,
                                       CPSS_DXCH_TTI_OFFSET_METADATA_E, 26);
    if(rc != GT_OK)
        return rc;

    rc = cpssDxChTtiPortLookupEnableSet(devNum, ingressPort, keyType, GT_TRUE);
    if(rc != GT_OK)
        return rc;

    /*  For ingress port: Create TTI rule */

    cpssOsMemSet(&(mask), 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    cpssOsMemSet(&(pattern), 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    for(i=0;i<6;i++)
    {
        pattern.udbArray.udb[i] = transitArpMacAddr.arEther[i];
        mask.udbArray.udb[i] = 0xFF;
    }

    pattern.udbArray.udb[6] = ingressPort;
    mask.udbArray.udb[6]    = 0xFF;

/*  action: 1) assigned source eport (example 101) 2) assign evlan id 3) forwarding */
    cpssOsMemSet(&action, 0, sizeof(CPSS_DXCH_TTI_ACTION_STC));
    action.tag1VlanCmd        = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
    action.userDefinedCpuCode = CPSS_NET_FIRST_USER_DEFINED_E + 1;
    action.tag0VlanCmd        = CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E;

    action.command                 = CPSS_PACKET_CMD_FORWARD_E ;
    action.tunnelTerminate         = enTT ;
    /* remove restriction on header length */
    if(ttHeaderLength != 0)
    {
        action.ttHeaderLength            = ttHeaderLength ; /*62? Granularity is in 2 Bytes */
        action.tunnelHeaderLengthAnchorType = CPSS_DXCH_TUNNEL_HEADER_LENGTH_L4_ANCHOR_TYPE_E;
/*      action.tunnelHeaderStartL4Enable = GT_TRUE ; */

        rc = prvCpssHwPpSetRegField(devNum, regAddr, 11, 1, 0);
        if(rc != GT_OK)
            return rc;

    }
    else
    {
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 11, 1, 1);
        if(rc != GT_OK)
            return rc;

        action.ttHeaderLength            = 0 ;
/*        action.tunnelHeaderStartL4Enable = GT_FALSE ; */
    }
    action.ttPassengerPacketType            = CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    action.egressInterface.type             = CPSS_INTERFACE_PORT_E;
    action.egressInterface.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(0); /* the port is on local device*/;
    action.egressInterface.devPort.portNum  = eportEgress;
    action.redirectCommand                  = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
    action.ingressPipeBypass                 = GT_TRUE;
    action.bridgeBypass                      = GT_TRUE;

    action.bindToCentralCounter         = GT_TRUE;
    action.centralCounterIndex          = (ipclBlocks+epclBlocks)*1024 + 1;

    rc = cpssDxChTtiRuleSet(devNum, tcamIndex, ruleType, &pattern, &mask, &action);
    if(rc != GT_OK)
        return rc;

    cpssOsMemSet(&eToPhy,0,sizeof(CPSS_INTERFACE_INFO_STC));
    eToPhy.type = CPSS_INTERFACE_PORT_E;
    eToPhy.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(0);
    eToPhy.devPort.portNum = egressPort;
    rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum, eportEgress, &eToPhy);
    if( GT_OK != rc)
        return rc;

    rc = prvIoamSetTxMirroring(devNum, eportEgress, txAnalyzerPort, GT_FALSE, GT_TRUE);
    if( GT_OK != rc)
        return rc;

    return GT_OK;
}

/**
* @internal csRefIoamNodeConfigDelete function
* @endinternal
*
* @brief   The function cleans all configurations created by csRefIoamNodeConfigCreate .
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefIoamNodeConfigDelete
(
  IN GT_U8         devNum
)
{
    GT_BOOL         finished;
    GT_U32          i, waitLoops=0;
    GT_STATUS                                  rc=GT_OK;
    CPSS_DXCH_TUNNEL_START_CONFIG_UNT          tunnelStartConf;

/* clear FDB */
    rc = cpssDxChBrgFdbStaticDelEnable(devNum, CPSS_DXCH_BRG_FDB_DEL_MODE_ALL_E);
    if(rc != GT_OK)
        return rc;

    rc = cpssDxChBrgFdbActionActiveDevSet(devNum, 0, 0);
    if(rc != GT_OK)
        return rc;

    rc = cpssDxChBrgFdbActionActiveInterfaceSet(devNum,0,0,0,0);
    if(rc != GT_OK)
        return rc;

    rc = cpssDxChBrgFdbActionActiveVlanSet(devNum,0,0);
    if(rc != GT_OK)
        return rc;

    rc = cpssDxChBrgFdbTrigActionStart(devNum,CPSS_FDB_ACTION_DELETING_E);
    if((rc != GT_OK) && (rc != GT_BAD_STATE))
    {
        return rc;
    }
    do
    {
        cpssOsTimerWkAfter(1);
        rc = cpssDxChBrgFdbTrigActionStatusGet(devNum, &finished);
        if(rc != GT_OK)
        {
            cpssOsPrintf("cpssDxChBrgFdbTrigActionStatusGet returned %d\n", rc);
            return rc;
        }
        if(waitLoops>0)
        {
          cpssOsPrintf("Not finished, i=%d\n",waitLoops);
        }
        waitLoops++;
        if(waitLoops>1000)
            break;
    }while(finished != GT_TRUE);
    if(finished != GT_TRUE)
        cpssOsPrintf("csRefIoamNodeConfigDelete: failed to fluch all entries\n");
    cpssOsPrintf("Flush finished after %d wait loops\n",waitLoops);

    /* clear ARP and TS entries, all indexes used in config were below 20 */
    cpssOsMemSet(&tunnelStartConf,0,sizeof(tunnelStartConf));

    for(i=0; i<20; i=i+2)
        rc = cpssDxChTunnelStartEntrySet(devNum, i, CPSS_TUNNEL_GENERIC_IPV6_E, &tunnelStartConf);
    if( GT_OK != rc)
        return rc;
    return rc;
}

/*
00EE1122335500111213141586DD6000000000BC11640000000000000000555555557777777700000000000000002222222233333333C00212B600BC00000F00008100001800010D0001F20080006400001100300038000007012D54D8F8000000000A00000F003000380000047E15F0D85B000000000000000034020000000F070008004500002A000000007FFF36D201000001030000030000000000E0C560E700010080023BC43660616774E2CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC0000008F5C78E20000008005485C7778DB62DF8A55555555
010D0001F200800064000011003000*/
static char ethIPv6[] = "86DD";
static char ethIPv4[] = "0800";
static char destUDP[] = "12B6"; /* it's 4790 in HEX */

/**
* @internal csRefIoamParser function
* @endinternal
*
* @brief   parses IOAM packet and prints its most interesting fields.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pktString           - device number
*
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefIoamParser
(
  IN char pktString[1024]
)
{
    GT_STATUS  rc=GT_OK;
    char       twoByteChar[5];
    char       fourByteChar[9];
    char       byteChar[3];
    GT_U32     i, nextHeadProt, ttl, lngthTemp, traceTypeBmp, hopLimit, nodeID, ingressInt, egressInt, numOfRecords;
    GT_BOOL    isIPv4;
    char       *pktStringTmp;
    char       *ioamPtr;

    i = 0;
    while (pktString[i] != '\0')
    {
      if (pktString[i] >= 'a' && pktString[i] <= 'z')
      {
         pktString[i] = pktString[i] - 32;
      }
      i++;
    }
    cpssOsPrintf("Parsing %s for IOAM data\n",pktString);
    pktStringTmp = pktString;

parsing_return_label:
    ioamPtr = strstr(pktStringTmp, ethIPv6);
    if(ioamPtr != NULL)
    {
        lngthTemp = strlen(pktStringTmp) - strlen(ioamPtr);
        if((lngthTemp > 23) && (lngthTemp <= 32))
           cpssOsPrintf("IPv6 packet\n");
        else
        {
            cpssOsPrintf("Cannot parse - bad Ethertype placement\n");
            return rc;
        }
        isIPv4 = GT_FALSE;
    }
    else
    {
        ioamPtr = strstr(pktStringTmp, ethIPv4);
        if(ioamPtr == NULL)
        {
            cpssOsPrintf("Cannot parse\n");
            return rc;
        }
        lngthTemp = strlen(pktStringTmp) - strlen(ioamPtr);
        if((lngthTemp>23) && (lngthTemp<=32))
           cpssOsPrintf("IPv4 packet\n");
        else
        {
            cpssOsPrintf("Cannot parse - bad Ethertype placement\n");
            return rc;
        }
        isIPv4 = GT_TRUE;
    }
    if(strlen(ioamPtr) < 102)
    {
        cpssOsPrintf("Cannot parse - packet too small length %d\n", strlen(ioamPtr));
        return rc;
    }

    if(isIPv4 == GT_FALSE)
    {
        cpssOsPrintf("IPv6 Payload length: ");
        ioamPtr = ioamPtr+12;
        strncpy(twoByteChar, ioamPtr, 4);
        twoByteChar[4] = 0;
        lngthTemp = strtol(twoByteChar, NULL, 16);
        cpssOsPrintf(" %d (0x%s)\n", lngthTemp, twoByteChar);

        cpssOsPrintf("Next Header protocol: ");
        ioamPtr = ioamPtr+4;
        strncpy(byteChar, ioamPtr, 2);
        byteChar[2] = 0;
        nextHeadProt = strtol(byteChar, NULL, 16);
        cpssOsPrintf(" %d (0x%s)\n", nextHeadProt, byteChar);

        cpssOsPrintf("Hop Limit: ");
        ioamPtr = ioamPtr+2;
        strncpy(byteChar, ioamPtr, 2);
        byteChar[2] = 0;
        ttl = strtol(byteChar, NULL, 16);
        cpssOsPrintf(" %d (0x%s)\n", ttl, byteChar);
    }

    if(nextHeadProt == 47)
    {
        i=70;
        cpssOsPrintf("GRE encapsulation: \nProtocol Type: ");
        ioamPtr = ioamPtr+i;
        strncpy(twoByteChar, ioamPtr, 4);
        twoByteChar[4] = 0;
        cpssOsPrintf(" 0x%s\n", twoByteChar);
        pktStringTmp = ioamPtr + 4;
        goto parsing_return_label;
    }
    else if(nextHeadProt != 17)
    {
        cpssOsPrintf("wrong NH protocol\n ");
        return GT_OK;
    }
    cpssOsPrintf("UDP: ");
    if(isIPv4 == GT_FALSE)
    {
        ioamPtr = ioamPtr+70; /* 88 */
        strncpy(twoByteChar, ioamPtr, 4);
        twoByteChar[4] = 0;
        cpssOsPrintf("dest port: %d (0x%s)\n", strtol(twoByteChar, NULL, 16), twoByteChar);
        if (strncmp(destUDP,twoByteChar,4) != 0)
        {
            cpssOsPrintf("Bad UDP dest port\n");
            return rc;
        }
        ioamPtr = ioamPtr+4;
        strncpy(twoByteChar, ioamPtr, 4);
        twoByteChar[4] = 0;
        cpssOsPrintf("UDP length:  %d (0x%s)\n", strtol(twoByteChar, NULL, 16), twoByteChar);

        ioamPtr = ioamPtr+4;
        strncpy(twoByteChar, ioamPtr, 4);
        twoByteChar[4] = 0;
        cpssOsPrintf("checksum: %s\n", twoByteChar);
    }
    cpssOsPrintf("IOAM:\n");
    ioamPtr = ioamPtr+20;
    byteChar[0] = ioamPtr[2];
    byteChar[1] = ioamPtr[3];
    byteChar[2] = 0;
    lngthTemp = strtol(byteChar, NULL, 16);
    byteChar[0] = ioamPtr[8];
    byteChar[1] = ioamPtr[9];
    traceTypeBmp = strtol(byteChar, NULL, 16);

    cpssOsPrintf("Type: %c%c\n Header Length %d (units are in 4B word)\n Next Prot %c%c\nTrace Type 0x%X",
        ioamPtr[0],
        ioamPtr[1],
        lngthTemp,
        ioamPtr[6],
        ioamPtr[7],
        traceTypeBmp
        );
    cpssOsPrintf(" (");
    if(traceTypeBmp&0x80)
        cpssOsPrintf("Hop_lim and Node ID: YES; ");
    else
        cpssOsPrintf("Hop_lim and Node ID: NO; ");

    if(traceTypeBmp&0x40)
        cpssOsPrintf("Ingress egress inf: YES; ");
    else
        cpssOsPrintf("Ingress egress inf: NO; ");

    if(traceTypeBmp&0x20)
        cpssOsPrintf("Timestamp sec: YES; ");
    else
        cpssOsPrintf("Timestamp sec: NO; ");

    if(traceTypeBmp&0x10)
        cpssOsPrintf("Timestamp ns: YES; ");
    else
        cpssOsPrintf("Timestamp ns: NO; ");

    if(traceTypeBmp&0x08)
        cpssOsPrintf("Delay: YES; ");
    else
        cpssOsPrintf("Delay: NO; ");

    if(traceTypeBmp&0x04)
         cpssOsPrintf("appData: YES; ");
    else
         cpssOsPrintf("appData: NO; ");

    if(traceTypeBmp&0x02)
         cpssOsPrintf("queue depth: YES; ");
    else
         cpssOsPrintf("queue depth: NO; ");

    cpssOsPrintf(")\n");
    numOfRecords = (lngthTemp - 2) / ones(traceTypeBmp);

    cpssOsPrintf(" Maximum Length 0x%c%c (units are in 4B word)\n Flags 0x%c%c\nnum Of Records %d\n",
        ioamPtr[12],
        ioamPtr[13],
         ioamPtr[14],
         ioamPtr[15],
         numOfRecords);

    ioamPtr = ioamPtr+16;

    /* we parse in each record only the fileds "enabled" in the trace type*/
    for(i=0;i<numOfRecords;i++)
    {
        cpssOsPrintf("\nRECORD %d\n\n",i);
    /* hop limit entry */
        if(traceTypeBmp&0x80)
        {
            byteChar[0] = ioamPtr[0];
            byteChar[1] = ioamPtr[1];
            byteChar[2] = 0;
            hopLimit = strtol(byteChar, NULL, 16);
            fourByteChar[0] = ioamPtr[2];
            fourByteChar[1] = ioamPtr[3];
            fourByteChar[2] = ioamPtr[4];
            fourByteChar[3] = ioamPtr[5];
            fourByteChar[4] = ioamPtr[6];
            fourByteChar[5] = ioamPtr[7];
            fourByteChar[6] = 0;
            nodeID = strtol(fourByteChar, NULL, 16);
            cpssOsPrintf("hopLimit %d (IP header ttl is %d) nodeID %d\n", hopLimit, ttl, nodeID);

            ioamPtr = ioamPtr+8;
        }

    /* ingress and egress interfaces entry */
        if(traceTypeBmp&0x40)
        {
            twoByteChar[0] = ioamPtr[0];
            twoByteChar[1] = ioamPtr[1];
            twoByteChar[2] = ioamPtr[2];
            twoByteChar[3] = ioamPtr[3];
            twoByteChar[4] = 0;
            ingressInt = strtol(twoByteChar, NULL, 16);
            twoByteChar[0] = ioamPtr[4];
            twoByteChar[1] = ioamPtr[5];
            twoByteChar[2] = ioamPtr[6];
            twoByteChar[3] = ioamPtr[7];
            twoByteChar[4] = 0;
            egressInt = strtol(twoByteChar, NULL, 16);
            cpssOsPrintf("interface: ingress  %d egress %d\n", ingressInt, egressInt);
            ioamPtr = ioamPtr+8;
        }


        if(traceTypeBmp&0x20)
        {
            fourByteChar[0] = ioamPtr[0];
            fourByteChar[1] = ioamPtr[1];
            fourByteChar[2] = ioamPtr[2];
            fourByteChar[3] = ioamPtr[3];
            fourByteChar[4] = ioamPtr[4];
            fourByteChar[5] = ioamPtr[5];
            fourByteChar[6] = ioamPtr[6];
            fourByteChar[7] = ioamPtr[7];
            fourByteChar[8] = 0;
            lngthTemp = strtol(fourByteChar, NULL, 16);
            cpssOsPrintf("timestamp sec  %d (0x%X) \n", lngthTemp, lngthTemp);
            ioamPtr = ioamPtr+8;
        }

        if(traceTypeBmp&0x10)
        {
            fourByteChar[0] = ioamPtr[0];
            fourByteChar[1] = ioamPtr[1];
            fourByteChar[2] = ioamPtr[2];
            fourByteChar[3] = ioamPtr[3];
            fourByteChar[4] = ioamPtr[4];
            fourByteChar[5] = ioamPtr[5];
            fourByteChar[6] = ioamPtr[6];
            fourByteChar[7] = ioamPtr[7];
            fourByteChar[8] = 0;
            lngthTemp = strtol(fourByteChar, NULL, 16);
            cpssOsPrintf("timestamp ns  %d (0x%X) \n", lngthTemp, lngthTemp);
            ioamPtr = ioamPtr+8;
        }

        if(traceTypeBmp&0x08)
        {
            fourByteChar[0] = ioamPtr[0];
            fourByteChar[1] = ioamPtr[1];
            fourByteChar[2] = ioamPtr[2];
            fourByteChar[3] = ioamPtr[3];
            fourByteChar[4] = ioamPtr[4];
            fourByteChar[5] = ioamPtr[5];
            fourByteChar[6] = ioamPtr[6];
            fourByteChar[7] = ioamPtr[7];
            fourByteChar[8] = 0;
            lngthTemp = strtol(fourByteChar, NULL, 16);
            cpssOsPrintf("delay ns  %d (0x%X) \n", lngthTemp, lngthTemp);
            ioamPtr = ioamPtr+8;
        }

        if(traceTypeBmp&0x04)
        {
            fourByteChar[0] = ioamPtr[0];
            fourByteChar[1] = ioamPtr[1];
            fourByteChar[2] = ioamPtr[2];
            fourByteChar[3] = ioamPtr[3];
            fourByteChar[4] = ioamPtr[4];
            fourByteChar[5] = ioamPtr[5];
            fourByteChar[6] = ioamPtr[6];
            fourByteChar[7] = ioamPtr[7];
            fourByteChar[8] = 0;
            lngthTemp = strtol(fourByteChar, NULL, 16);
            cpssOsPrintf("appData ns  %d \n", lngthTemp);
            ioamPtr = ioamPtr+8;
        }

        if(traceTypeBmp&0x02)
        {
            fourByteChar[0] = ioamPtr[0];
            fourByteChar[1] = ioamPtr[1];
            fourByteChar[2] = ioamPtr[2];
            fourByteChar[3] = ioamPtr[3];
            fourByteChar[4] = ioamPtr[4];
            fourByteChar[5] = ioamPtr[5];
            fourByteChar[6] = ioamPtr[6];
            fourByteChar[7] = ioamPtr[7];
            fourByteChar[8] = 0;
            lngthTemp = strtol(fourByteChar, NULL, 16);
            cpssOsPrintf("queue depth  %d \n", lngthTemp);
            ioamPtr = ioamPtr+8;
        }
    }

    return GT_OK;
}





