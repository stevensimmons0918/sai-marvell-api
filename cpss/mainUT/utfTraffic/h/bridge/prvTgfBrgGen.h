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
* @file prvTgfBrgGen.h
*
* @brief Bridge Generic APIs UT.
*
* @version   2.
********************************************************************************
*/
#ifndef __prvTgfBrgGen
#define __prvTgfBrgGen

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvTgfBrgGenIeeeReservedMcastProfilesConfigurationSet function
* @endinternal
*
* @brief   Set test configuration:
*         Create VLAN 2 on all ports (0,8,18,23)
*         Set capture to CPU
*         BridgingModeSet = CPSS_BRG_MODE_802_1Q_E
*         GenIeeeReservedMcastTrapEnable = GT_TRUE
*         For all ports (0,8,18,23):
*         For all profileIndex (0...3):
*         IeeeReservedMcastProfileIndexSet (devNum = 0,
*         portNum,
*         profileIndex)
*         For all protocols (0...255)
*         IeeeReservedMcastProtCmdSet1 (dev = 0,
*         profileIndex,
*         protocol_iterator,
*         CPSS_PACKET_CMD_TRAP_TO_CPU_E)
*         Send Packet from to portNum.
*         IeeeReservedMcastProtCmdSet1 (dev = 0,
*         profileIndex,
*         protocol_iterator,
*         CPSS_PACKET_CMD_DROP_SOFT_E)
*         Success Criteria:
*         Fisrt packet is not captured on ports 0,8,18,23 but is captured in CPU.
*         Second packet is dropped.
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastProfilesConfigurationSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgGenIeeeReservedMcastProfilesTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port 0 IP4 packet:
*         000000   01 80 c2 00 00 03 00 00 00 00 00 11 81 00 00 02
*         000010 08 00 45 00 00 2a 00 00 00 00 40 ff 79 d4 00 00
*         000020  00 00 00 00 00 02 00 01 02 03 04 05 06 07 08 09
*         000030  0a 0b 0c 0d 0e 0f 10 11 12 13 14 15
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastProfilesTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfBrgGenIeeeReservedMcastProfilesConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgGenIeeeReservedMcastProfilesConfigurationRestore
(
    GT_VOID
);

/**
* @internal prvTgfBrgGenBypassModeTest function
* @endinternal
*
* @brief   Bridge Bypass mode envelope
*/
GT_VOID prvTgfBrgGenBypassModeTest
(
    GT_VOID
);

/**
* @internal prvTgfBrgPhysicalPortsSetUpTest function
* @endinternal
*
* @brief   Bridge physical port setup test
*/
GT_VOID prvTgfBrgPhysicalPortsSetUpTest
(
    GT_VOID
);

/**
* @internal prvTgfBrgGenPortBypassModeTest function
* @endinternal
*
* @brief   Per Source Physical Port Bridge Bypass mode envelope
*/
GT_VOID prvTgfBrgGenPortBypassModeTest
(
    GT_VOID
);


/**
* @internal prvTgfBrgGenIpLinkLocalProtCmdSetConfig function
* @endinternal
*
* @brief   Function to config the IP MC linklocal packets on global and vlan level.
*
*/
GT_VOID prvTgfBrgGenIpLinkLocalProtCmdSetConfig();
/**
* @internal prvTgfBrgGenIpLinkLocalProtCmdSetTrafficTest function
* @endinternal
*
* @brief   Function to send traffic and test the IP MC
*           linklocal packets on global and vlan level.
*
*/
GT_VOID prvTgfBrgGenIpLinkLocalProtCmdSetTrafficTest();
/**
* @internal prvTgfBrgGenIpLinkLocalProtCmdSetConfigReset function
* @endinternal
*
* @brief   Function to reset config the IP MC linklocal packets on global and vlan level.
*
*/
GT_VOID prvTgfBrgGenIpLinkLocalProtCmdSetConfigReset();

/**
* @internal prvTgfBrgGenEportAssignEgressAttributesLocallySet function
* @endinternal
*
* @brief   Function to config the assigning the eport attr locally.
*           Create VLAN 5 with tagged ports [0,1,2,3]
*           add FDB entry with MAC 00:00:00:00:00:02,  port 2
*           Add FDB entry dest port as remoteDev, ePort
*           set cascade mapping for remote device\port
*           assign local attributes to the eport enable
*           config push tag on local eport index.
*           config TPID index entry
*           config eport with the TPID entry
*
*/
GT_VOID prvTgfPacketBrgLocalAssignConfig();
/**
* @internal prvTgfPacketBrgLocalAssignTrafficTest function
* @endinternal
*
* @brief   Function to send traffic and test the egress eport filtering mechanism.
*           1. Enable locally assign to TRUE and send packet.
*               Expected: Packet egressed has outer tag as configured.
*           2. Disable locally assign to TRUE and send packet.
*               Expected: Packet egressed SHOULD NOT have outer tag
*
*/
GT_VOID prvTgfPacketBrgLocalAssignTrafficTest();
/**
* @internal prvTgfPacketBrgLocalAssignConfigReset function
* @endinternal
*
* @brief   Function to reset the assigning the eport attr locally.
*
*/
GT_VOID prvTgfPacketBrgLocalAssignConfigReset();

/**
* @internal prvTgfBrgFdbUploadConfigSet function
* @endinternal
*
* @brief   Bridge FDB Upload Configuration
*/
GT_VOID prvTgfBrgFdbUploadConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfBrgFdbUploadConfigRestore function
* @endinternal
*
* @brief   Bridge FDB Upload Configuration Restore
*/
GT_VOID prvTgfBrgFdbUploadConfigRestore
(
    GT_VOID
);
/**
* @internal prvTgfBrgFdbEpgConfigSet function
* @endinternal
*
* @brief The api to config the FDB SA/DA based EPG id.
*    Create VLAN with tagged ports [0,1,2,3]
*    add FDB entry with MAC 00:00:00:00:00:02,  port 2
*    Set FDB entry for DA  match
*    config all SRC-ID bits to be used for EPG bits
*    global fdb based src id assignment
*    config the SRC-ID for source EPG ID based on SA lookup
*    EPCL matching the SRC and DST EPG ID and modify the DSCP of the original packet
*/
GT_VOID prvTgfBrgFdbEpgConfigSet(GT_BOOL setSrcEpg);
/**
* @internal prvTgfBrgSrcIdScalableSgtTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic and test the egress packet DSCP as set by 
*          the EPCL group ID indices.
*/
GT_VOID prvTgfBrgFdbEpgTrafficGenerate();
/**
* @internal prvTgfBrgFdbEpgConfigRestore function
* @endinternal
*
* @brief Restore configuration
*/
GT_VOID prvTgfBrgFdbEpgConfigRestore();
/**
* @internal prvTgfBrgFdbEpgEpclConfig function
* @endinternal
*
* @brief The api to config the EPCL matching the SRC and DST EPG ID,
*           and modify the DSCP of the original packet
*/
GT_VOID prvTgfBrgFdbEpgEpclConfig(GT_U32 srcGrpId, GT_U32 dstGrpId);
/**
* @internal prvTgfBrgFdbEpgEpclConfigRestore function
* @endinternal
*
* @brief Restore EPCL configuration for EPG
*/
GT_VOID prvTgfBrgFdbEpgEpclConfigRestore();
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgGen */


