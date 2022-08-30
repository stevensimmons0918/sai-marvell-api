
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
* @file prvTgfBasicIpv4McRouting.h
*
* @brief Basic IPV4 UC Routing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpCtrl
#define __prvTgfIpCtrl

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfIpGen.h>

/**
* @internal prvTgfIpNhMuxModeConfig function
* @endinternal
*
* @brief  Configure the VLANs and add port
*           enable IP UC on vlan and ingress port
*           Add mac2me entry
*           Add NH entry with EPG bits
*           Add EPCL rule to match target EPG and action to modify DSCP
*/
GT_VOID prvTgfIpNhMuxModeConfig();
/**
* @internal prvTgfIpNhMuxModeConfigRestore function
* @endinternal
*
* @brief  Restore all config done for target EPG from Router
*/
GT_VOID prvTgfIpNhMuxModeConfigRestore();
/**
* @internal prvTgfIpNhMuxModeTraffic function
* @endinternal
*
* @brief   
*          Generate traffic and test the egress packet DSCP as set by 
*          the EPCL group ID indices.
*/
GT_VOID prvTgfIpNhMuxModeTraffic();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__prvTgfIpCtrl*/


