
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
* @file prvTgfIngressMirrorSflow.h
*
* @brief Ingress SFLOW use cases testing.
*        Supporting SFLOW V5 use case
*
*        Ports allocation:
*           ^ Port#1: Analyzer port
*           ^ Port#2: Target port
*           ^ Port#3: Ingress port
*
*        Packet flow:
*
* @version   1
********************************************************************************
*/
/**
* @internal prvTgfFalconMirrorIngressSflowIpv4ConfigurationSet function
* @endinternal
*
* @brief  Test ingress mirroring functionality with sflow v5 encapsulation
*    configure VLAN;
*    configure an analyzer index, sampling rate and eport;
*    enable Rx mirroring for ingress port;
*    E2phy and IPv4 TS for eport
*    Configure OAM, EPLR & ERMRK for other fields of SFLOW header
*    configure EPCL rule to trigger IPv4 PHA thread, and pha metadata
*    send BC traffic and verify expected Sflow mirrored traffic on analyzer port;
*/
void prvTgfFalconMirrorIngressSflowIpv4ConfigurationSet();
/**
* @internal prvTgfFalconMirrorIngressSflowIpv4TrafficSend function
* @endinternal
*
* @brief  Traffic test for ingress mirroring functionality 
*           with sflow v5 IPv4 encapsulation
*           RX port #1
*           TX analyzer port #2
*           Check in the SFLOW header for
*           1. agent IPv4 address
*           2. enterprise format
*           3. sampling ratio
*/
void prvTgfFalconMirrorIngressSflowIpv4TrafficSend();
/**
* @internal prvTgfFalconMirrorIngressSflowIpv4ConfigurationReset function
* @endinternal
*
* @brief  Restore ingress mirroring functionality with sflow v5 encapsulation
*/
void prvTgfFalconMirrorIngressSflowIpv4ConfigurationReset();
/**
* @internal prvTgfFalconMirrorIngressSflowIpv6ConfigurationSet function
* @endinternal
*
* @brief Test ingress mirroring functionality with sflow v5 encapsulation
*    configure VLAN;
*    configure an analyzer index, sampling rate and eport;
*    enable Rx mirroring for ingress port;
*    E2phy and IPv6 TS for eport
*    Configure OAM, EPLR & ERMRK for other fields of SFLOW header
*    configure EPCL rule to trigger IPv6 PHA thread, and pha metadata
*    send BC traffic and verify expected Sflow mirrored traffic on analyzer port;
*/
void prvTgfFalconMirrorIngressSflowIpv6ConfigurationSet();
/**
* @internal prvTgfFalconMirrorIngressSflowIpv6TrafficSend function
* @endinternal
*
* @brief  Traffic test for ingress mirroring functionality 
*           with sflow v5 IPv6 encapsulation
*           RX port #1
*           TX analyzer port #2
*           Check in the SFLOW header for
*           1. agent IPv4 address
*           2. enterprise format
*           3. sampling ratio
*/
void prvTgfFalconMirrorIngressSflowIpv6TrafficSend();
/**
* @internal prvTgfFalconMirrorIngressSflowIpv6ConfigurationReset function
* @endinternal
*
* @brief  Restore ingress mirroring functionality with sflow v5 encapsulation
*/
void prvTgfFalconMirrorIngressSflowIpv6ConfigurationReset();
/**
* @internal prvTgfFalconMirrorIngressSflowMultiTargetConfig function
* @endinternal
*
* @brief   Multi target Sflow Use case.
*          It needs 2 pass mechanism for sending a ingress SFLOW Mirrored to multiple collectors
*           ===PASS 1===
*               prepare ports, p#0 RX, p#1 TX1, p#2 TX2, p#3 LB
*               setup ingress mirroring index, target to eport, sampling ratio 
*               enable Rx mirroring on port=1, index=0 
*               prepare the analyzer eport, e2phy to LB, push reserved eth type 0xffff
*               configure the P#3 as LB port
*           ===PASS 2===
*               enable "VLAN tag based Remote Physical Port mapping"  for TPID 0xFFFF
*               TTI config, match reserved TPID, action: bypass ingress, assign eVIDX
*               configure evidx to l2mll mapping
*               for each eport of MLL
*                 1. e2phy to egress ports
*                 2. TS index for V4/V6
*                 3. PHA thread id
*               configure TS L2+IP+UDP, and TS profile
*               e2phy  and enable TS for eports
*               setup PHA threads
*/
void prvTgfFalconMirrorIngressSflowMultiTargetConfig();
/**
* @internal prvTgfFalconMirrorIngressSflowMultiTargetTraffic function
* @endinternal
*
* @brief   Multi target Sflow Use case.
*           Test the captures for the configs for Multi sflow collectors.
*           RX:
*               L2 pkt for Vlan5
*           TX:
*               2 pkts on 2 Egr analyzer ports.
*               1. IPv4 encapsulated 
*               2. IPv6 encapsulated 
*/
void prvTgfFalconMirrorIngressSflowMultiTargetTraffic();
/**
* @internal prvTgfFalconMirrorIngressSflowMultiTargetConfigReset function
* @endinternal
*
* @brief   Multi target Sflow Use case.
*           Reset all the configs for Multi sflow collectors
*/
void prvTgfFalconMirrorIngressSflowMultiTargetConfigReset();
