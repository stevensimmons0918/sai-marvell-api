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
* @file prvTgfIpv4UcPbrLpmRoutingPriority.h
*
* @brief Policy and LPM based IPV4 UC Routing.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpv4UcPbrLpmRoutingPriorityh
#define __prvTgfIpv4UcPbrLpmRoutingPriorityh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <common/tgfLpmGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfBridgeGen.h>

/**
* @internal prvTgfIpv4UcPbrLpmRoutingPriorityBaseConfigSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] prvUtfVrfId    - virtual router ID
*
*/
GT_VOID prvTgfIpv4UcPbrLpmRoutingPriorityBaseConfigSet
(
    GT_U32  prvUtfVrfId
);

/**
* @internal prvTgfIpv4UcPbrLpmRoutingPriorityPclConfigSet function 
* @endinternal
*
* @brief   Set PCL configuration
*/
GT_VOID prvTgfIpv4UcPbrLpmRoutingPriorityPclConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfIpv4UcPbrLpmRoutingPriorityFdbRoutingGenericConfigSet function
* @endinternal
*
* @brief   Set FDB Generic Route Configuration
*/
GT_VOID prvTgfIpv4UcPbrLpmRoutingPriorityFdbRoutingGenericConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfIpv4UcPbrLpmRoutingPriorityFdbRoutingConfigSet function 
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*
*/
GT_VOID prvTgfIpv4UcPbrLpmRoutingPriorityFdbRoutingConfigSet
(
    GT_U32  prvUtfVrfId
);

/**
* @internal prvTgfIpv4UcPbrLpmRoutingPriorityLeafEntrySet function 
* @endinternal
*
* @brief   Set LPM leaf entry
*
* @param[in] priority              - LPM leaf entry priority
*
*/
GT_STATUS prvTgfIpv4UcPbrLpmRoutingPriorityLeafEntrySet
(
    CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT priority
);

/**
* @internal prvTgfIpv4UcPbrLpmRoutingPriorityRouteConfigSet function 
* @endinternal
*
* @brief   Set FDB Route Configuration
*
* @param[in] routingByIndex           - GT_TRUE:  set routing entry by index
*                                      GT_FALSE: set routing entry by message
* @param[in] macEntryPtr              - (pointer to) mac entry
*/
GT_VOID prvTgfIpv4UcPbrLpmRoutingPriorityRouteConfigSet
(
    GT_BOOL                                 routingByIndex,
    PRV_TGF_BRG_MAC_ENTRY_STC               *macEntryPtr
);

/**
* @internal prvTgfIpv4UcPbrLpmRoutingPriorityGenericLttRouteConfigSet function
* @endinternal
*
* @brief   Set LTT Route Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*/
GT_VOID prvTgfIpv4UcPbrLpmRoutingPriorityGenericLttRouteConfigSet
(
    GT_U32      prvUtfVrfId
);

/**
* @internal prvTgfIpv4UcPbrLpmRoutingPriorityLpmLeafEntrySet function 
* @endinternal
*
* @brief   Set routing priority between LPM and FDB loopup
*
* @param[in] prvUtfVrfId        - virtual router index
* @param[in] priority           - priority 
* @param[in] applyPbr           - apply policy based routing
* @param[in] deleteExisted      - GT_TRUE - delete existed prefix
*                                 GT_FALSE - don't delete
* @param[in] isEcmpTest         - GT_TRUE - update dst address for ecmp test.
*                               - GT_FALSE - use default dst address
* @param[in] isFdbMismatchTest  - GT_TRUE - update dst address for fdb mismatch test
*                                 GT_FALSE - use default dst address
*
*/
GT_VOID prvTgfIpv4UcPbrLpmRoutingPriorityLpmLeafEntrySet
(
    GT_U32                                        prvUtfVrfId,
    CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT    priority,
    GT_BOOL                                       applyPbr,
    GT_BOOL                                       deleteExisted,
    GT_BOOL                                       isEcmpTest,
    GT_BOOL                                       isFdbMismatchTest
);

/**
* @internal prvTgfIpv4UcPbrLpmRoutingPriorityTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*  
* @param[in] expectLpmRouting - GT_TRUE - expect LPM routing
*                               GT_FALSE - expect non LPM routing
* @param[in] expectPbrRouting - GT_TRUE - expect PBR routing
*                               GT_FALSE - expect non PBR routing
* @param[in] expectFdbRouting - GT_TRUE - expect FDB routing
*                               GT_FALSE - expect non FDB routing
* @param[in] enableFdbMismatch - GT_TRUE - Modify dst address to get FDB mismatch
*                              - GT_FALSE - do not modify dst address
* @param[in] isEcmpTest        - GT_TRUE - Modify dst address with new address
*                                GT_FALSE - do not modify dst address
*/
GT_VOID prvTgfIpv4UcPbrLpmRoutingPriorityTrafficGenerate
(
    GT_BOOL expectLpmRouting,
    GT_BOOL expectPbrRouting,
    GT_BOOL expectFdbRouting,
    GT_BOOL enableFdbMismatch,
    GT_BOOL isEcmpTest
);

/**
* @internal prvTgfIpv4UcPbrLpmRoutingPriorityConfigRestore function 
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*
*/
GT_VOID prvTgfIpv4UcPbrLpmRoutingPriorityConfigRestore
(
    GT_U32   prvUtfVrfId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpv4UcPbrLpmRoutingPriorityh */


