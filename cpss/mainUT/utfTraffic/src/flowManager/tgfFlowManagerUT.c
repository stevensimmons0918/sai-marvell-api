/*******************************************************************************
*              (c), Copyright 2019, Marvell International Ltd.                 *
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
* @file tgfFlowManagerUT.c
*
* @brief Enhanced UTs for Flow Manager Init.
*
* @version 1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <flowManager/prvTgfFlowManager.h>
#include <cpss/dxCh/dxChxGen/flowManager/cpssDxChFlowManager.h>
#include <cpss/dxCh/dxChxGen/flowManager/prvCpssDxChFlowManagerDb.h>
#include <flowManager/prvTgfFlowManager.h>

#ifndef ASIC_SIMULATION
static GT_U32 flowMngId = 1;
#endif

/*----------------------------------------------------------------------------*/
/* AUTODOC: TC1: Flow classification for IPV4 TCP packet based on all allowed fields

Configuration

1. Init TCAM
2. Init PCL
3. Init Policer Engine
4. Init IPFIX wrap-around configuration
5. Install default rules on all ports in vTcam region for short rules (30B) and long rules (60B)
6. Install IPFIX bound rules on all ports in vTcam region dedicated for FW
7. Init the Flow Manager and load and init service cpu
8. Configure pcl udb keys for flow classification based on DIP only
8. Bind the message parsing function with the Rx event
9. Configuring flow field config for IPv4-TCP pkt type
10. Bind the keyTableIndex to the port to start flow classification

Generate Traffic
1. Clear MAC counters
2. Clear Rx Packet table and start capture
3. Send 4 packet each with different SIP, VLAN, Src Port, Dst Port

Expected
1. Verify that the flow was learned
2. Verify that all the packet correspoding to the flow was accounted for
3. Verify that the source port learning was done and packet based classification happened
*/

UTF_TEST_CASE_MAC(tgfPortTxFlowManagerSetTcpFlowField)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    prvTgfFlowManagerInit(flowMngId, GT_TRUE, CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E);
    tgfPortTxFlowManagerResourcesMultiTrafficGenerate(CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E);
    prvTgfFlowManagerDelete(flowMngId);
#endif
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: TC2: Flow classification for IPV4 TCP packet based on all allowed fields

Configuration

Same as above except the pcl udb keys configuration allows all applicable fields for IPV4 TCP

Generate Traffic
1. Clear MAC counters
2. Clear Rx Packet table and start capture
3. Send 1 packet each with different SIP, VLAN, Src Port, Dst Port

Expected
1. Verify that the flow was learned
2. Verify that all the packet correspoding to the flow was accounted for
3. Verify that the source port learning was done and packet based classification happened
*/
UTF_TEST_CASE_MAC(tgfPortTxFlowManagerSetUdpFlowField)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    prvTgfFlowManagerInit(flowMngId, GT_TRUE, CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E);
    tgfPortTxFlowManagerResourcesMultiTrafficGenerate(CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E);
    prvTgfFlowManagerDelete(flowMngId);
#endif
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: TC3: Flow classification for other packet based on all allowed fields

Configuration

Same as above except the pcl udb keys configuration allows all applicable fields for IPV4 OTHER PACKET

Generate Traffic
1. Clear MAC counters
2. Clear Rx Packet table and start capture
3. Send 1 packet each with different SIP, VLAN

Expected
1. Verify that the flow was learned
2. Verify that all the packet correspoding to the flow was accounted for
3. Verify that the source port learning was done and packet based classification happened
*/
UTF_TEST_CASE_MAC(tgfPortTxFlowManagerSetOtherFlowField)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    prvTgfFlowManagerInit(flowMngId, GT_TRUE, CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E);
    tgfPortTxFlowManagerResourcesMultiTrafficGenerate(CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E);
    prvTgfFlowManagerDelete(flowMngId);
#endif
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: TC4: Flow classification for mixed packet based on all allowed fields

Configuration

Same as above except the pcl udb keys configuration allows all applicable fields for MIXED PACKET
i.e IPV4-TCP, IPV4-UDP, OTHER based on per packet type configuration.

Generate Traffic
1. Clear MAC counters
2. Clear Rx Packet table and start capture
3. Send 2 packets each of different flows i.e ipv4-tcp flows on port0, ipv4-udp flows on port 12
and other flows on port 36 with different SIP, VLAN

Expected
1. Verify that the flow was learned
2. Verify that all the packet correspoding to the flow was accounted for
3. Verify that the source port learning was done and packet based classification happened
*/
UTF_TEST_CASE_MAC(tgfPortTxFlowManagerSetMixedFlowField)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    prvTgfFlowManagerMixedFlowInit(flowMngId, GT_TRUE);
    tgfPortTxFlowManagerResourcesMixedTrafficGenerate();
    prvTgfFlowManagerDelete(flowMngId);
#endif
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: TC5: Flow classification for mixed packet based on all allowed fields

Configuration

Same as above except the pcl udb keys configuration allows all applicable fields for MIXED PACKET
i.e. IPV4-TCP, IPV4-UDP, OTHER based on per port type with all flows bound to port 0

Generate Traffic
1. Clear MAC counters
2. Clear Rx Packet table and start capture
3. Send 2 packets each of different flows i.e ipv4-tcp, ipv4-udp and other flows on port 0
with different SIP, VLAN

Expected
1. Verify that the flow was learned
2. Verify that all the packet correspoding to the flow was accounted for
3. Verify that the source port learning was done and packet based classification happened
*/
UTF_TEST_CASE_MAC(tgfPortTxFlowManagerSetMixedFlowPortField)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    prvTgfFlowManagerMixedFlowInit(flowMngId, GT_TRUE);
    tgfPortTxFlowManagerResourcesMixedPortTrafficGenerate();
    prvTgfFlowManagerDelete(flowMngId);
#endif
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: TC6: Flow classification for other packet based on all allowed fields

Configuration

Same as TC3 except the pcl udb keys configuration allows QinQ/Nested VLANs

Generate Traffic
1. Clear MAC counters
2. Clear Rx Packet table and start capture
3. Send 1 packet each with different SIP, Nested VLANs

Expected
1. Verify that the flow was learned
2. Verify that all the packet correspoding to the flow was accounted for
3. Verify that the source port learning was done and packet based classification happened
*/
UTF_TEST_CASE_MAC(tgfPortTxFlowManagerSetOtherQinQFlowField)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    prvTgfFlowManagerQinQInit(flowMngId, GT_TRUE, CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E);
    tgfPortQinQTxFlowManagerResourcesMultiTrafficGenerate(CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E);
    prvTgfFlowManagerDelete(flowMngId);
#endif
}

/*
 * Configuration of tgfPort suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfFlowManager)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortTxFlowManagerSetTcpFlowField)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortTxFlowManagerSetUdpFlowField)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortTxFlowManagerSetOtherFlowField)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortTxFlowManagerSetOtherQinQFlowField)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortTxFlowManagerSetMixedFlowField)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPortTxFlowManagerSetMixedFlowPortField)
UTF_SUIT_END_TESTS_MAC(tgfFlowManager)

