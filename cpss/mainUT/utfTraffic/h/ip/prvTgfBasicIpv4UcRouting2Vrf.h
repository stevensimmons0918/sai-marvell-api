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
* @file prvTgfBasicIpv4UcRouting2Vrf.h
*
* @brief Basic IPV4 UC Routing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfBasicIpv4UcRouting2Vrfh
#define __prvTgfBasicIpv4UcRouting2Vrfh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <common/tgfLpmGen.h>
#include <common/tgfIpGen.h>

/**
* @internal prvTgfBasicIpv4UcRoutingBaseConfiguration2VrfSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] fdbPortNum - the FDB port num to set in the macEntry destination Interface None
*/
GT_VOID prvTgfBasicIpv4UcRoutingBaseConfiguration2VrfSet
(
    GT_U32   fdbPortNum
);
/**
* @internal prvTgfBasicIpv4UcRoutingTraffic2VrfGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] sendPortNum              - port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
* @param[in] useSecondPkt             - whether to use the second packet (prvTgfPacket2PartArray)
* @param[in] useSecondArp             - whether to use the second ARP MAC address (prvTgfArpMac1)
* @param[in] isUrpfTest               - whether this is unicast RPF test
* @param[in] expectNoTraffic          - whether to expect traffic or not
* @param[in] expectDoubleTag          - whether to expect packet with double tag
*
*/
GT_VOID prvTgfBasicIpv4UcRoutingTraffic2VrfGenerate
(
    GT_U32    sendPortNum,
    GT_U32    nextHopPortNum,
    GT_BOOL   useSecondPkt,
    GT_BOOL   useSecondArp,
    GT_BOOL   isUrpfTest,
    GT_BOOL   expectNoTraffic,
    GT_BOOL   expectDoubleTag
);
/**
* @internal prvTgfBasicIpv4UcRoutingRouteConfiguration2VrfSet function
* @endinternal
*
* @brief   Set Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] sendPortIndex            - index of port sending traffic
* @param[in] nextHopPortNum           - port receiving traffic
*
*/
GT_VOID prvTgfBasicIpv4UcRoutingRouteConfiguration2VrfSet
(
    GT_U32   prvUtfVrfId0,
    GT_U32   prvUtfVrfId1,
    GT_U8    sendPortIndex,
    GT_U32   nextHopPortNum0,
    GT_U32   nextHopPortNum1
);

/**
* @internal prvTgfBasicIpv4UcRoutingConfiguration2VrfRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] prvUtfVrfId              - virtual router index
* @param[in] prvUtfVrfId              - second virtual router index
* @param[in] sendPortIndex            - index of port sending traffic
*
* @note 2. Restore Route Configuration
*       1. Restore Base Configuration
*
*/
GT_VOID prvTgfBasicIpv4UcRoutingConfiguration2VrfRestore
(
    GT_U32   prvUtfVrfId0,
    GT_U32   prvUtfVrfId1,
    GT_U8    sendPortIndex
);
/**
* @internal prvTgfBasicIpv4UcRoutingConfiguration2VrfSave
*           function
* @endinternal
*
* @brief   Save configuration
*
* @param[in] devNum                   - device number
*
* @note none
*
*/
GT_VOID prvTgfBasicIpv4UcRoutingConfiguration2VrfSave
(
    GT_U32   devNum
);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfBasicIpv4UcRouting2Vrfh */


