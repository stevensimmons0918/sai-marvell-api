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
* @file prvTgfIpv6UcRoutingCapacitySip6MergeBanks.h
*
* @brief IPV4 & IPV6 UC Routing when filling the Lpm and using many Add and
* Delete operations.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpv6UcRoutingCapacitySip6MergeBanksh
#define __prvTgfIpv6UcRoutingCapacitySip6MergeBanksh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @internal prvTgfIpv6UcRoutingCapacitySip6MergeBanks function
 * @endinternal
 *
 * @brief Run IPv6 LPM capacity test for merge banks.
 *        This is the test for defragmentation phase 4
 *        1. Use 20 small banks.
 *        2. Add Ipv6 prefixes of length 64 bits by incrementing octet = 7. (Step = 4) until LPM banks are full.
 *        3. Delete half of the prefixes by incrementing octet 6 (Step = 2) and incrementing octet 7 (Step = 4)
 *        4. Add Ipv6 prefixes of length 80 bits until LPM banks are full. Merge operation occurs here.
 *        5. Delete half of remaining prefixes of length 64 bits by incrementing octet 6 (Step = 4)
 *           and octet 7 (Step = 4).
 *        6. Add Ipv6 prefixes of length 96 bits until LPM banks are full. Merge operation occurs here.
 *        7. Run validity at each of the above steps.
 *        8. Flush all prefixes.
 */
GT_VOID prvTgfIpv6UcRoutingCapacitySip6MergeBanks
(
    GT_VOID
);

/**
 * @internal prvTgfIpv6UcRoutingCapacitySip6MergeBanksWithShrink function 
 * @endinternal 
 *  
 * @brief Run IPv6 LPM capacity test for merge banks. 
 *        This is the test for defragmentation phase 4
 *        1. Use 20 small banks.
 *        2. Add Ipv6 prefixes of length 64 bits by incrementing octet = 7. (Step = 4) until LPM banks are full.
 *        3. Delete half of the prefixes by incrementing octet 6 (Step = 2) and incrementing octet 7 (Step = 4)
 *        4. Add Ipv6 prefixes of length 80 bits until LPM banks are full. Merge operation occurs here.
 *        5. Delete half of remaining prefixes of length 64 bits by incrementing octet 6 (Step = 4)
 *           and octet 7 (Step = 4).
 *        6. Add Ipv6 prefixes of length 96 bits until LPM banks are full. Merge operation occurs here.
 *        7. Run validity at each of the above steps.
 *        8. Flush all prefixes.
 */
GT_VOID prvTgfIpv6UcRoutingCapacitySip6MergeBanksWithShrink
(
    GT_VOID
);

GT_VOID prvTgfIpv6McRoutingCapacitySip6DeleteAndRefill
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpv6UcRoutingCapacitySip6MergeBanksh */
