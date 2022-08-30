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
* @file prvTgfBrgIPv4RIPv1controlCPU.h
*
* @brief IPv4 RIPv1 control CPU configuration test
*
* @version   0
********************************************************************************
*/
#ifndef __prvTgfBrgIPv4RIPv1controlCPU
#define __prvTgfBrgIPv4RIPv1controlCPU

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfIpV4BrgConfigurationTrafficGenerator function
* @endinternal
*
* @brief   Generate traffic and compare counters
*
* @param[in] packetToCpu              - send packet to cpu?
* @param[in] floodOrDrop              - send packet to ports?
*/
GT_VOID prvTgfIpV4BrgConfigurationTrafficGenerator(
    IN CPSS_PACKET_CMD_ENT cmd
);

/**
* @internal prvTgfIpV4BrgConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfIpV4BrgConfigurationSet(
    GT_VOID
);

/**
* @internal prvTgfIpV4BrgConfigurationTest function
* @endinternal
*
* @brief   Manage the test settings and traffic
*/
GT_VOID prvTgfIpV4BrgConfigurationTest(
    GT_VOID
);

 
/**
* @internal prvTgfIpV4BrgConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfIpV4BrgConfigurationRestore(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBrgIPv4RIPv1controlCPU */



