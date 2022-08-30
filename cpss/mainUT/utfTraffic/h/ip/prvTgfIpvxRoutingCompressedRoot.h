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
* @file prvTgfIpvxRoutingCompressedRoot.h
*
* @brief IPV4/IPV6 UC/MC Routing with compressed root node
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpvxRoutingCompressedRooth
#define __prvTgfIpvxRoutingCompressedRooth

/**
 * @internal prvTgfIpv4UcRoutingCompressedRoot function
 * @endinternal
 *
 * @brief Ipv4 Unicast Prefixes add/delete with compressed root node.
 *        1. Add basic configurations.
 *        2. Check root node converstions, embedded, compressed, regular.
 *        3. Create multiple VRFs and prefixes in each vrf
 *        4. Delete prefixes in each vrf and delete VRFs
*/
GT_VOID prvTgfIpv4UcRoutingCompressedRoot
(
    GT_VOID
);

/**
 * @internal prvTgfIpv6UcRoutingCompressedRoot function
 * @endinternal
 *
 * @brief Ipv6 Unicast Prefixes add/delete with compressed root node.
 *        1. Add basic configurations.
 *        2. Check root node converstions, embedded, compressed, regular.
 *        3. Create multiple VRFs and prefixes in each vrf
 *        4. Delete prefixes in each vrf and delete VRFs
*/
GT_VOID prvTgfIpv6UcRoutingCompressedRoot
(
    GT_VOID
);

/**
 * @internal prvTgfIpv6McRoutingCompressedRoot function
 * @endinternal
 *
 * @brief Ipv6 Multicast Prefixes add/delete with compressed root node.
 *        1. Add basic configurations.
 *        2. Check root node converstions, embedded, compressed, regular.
*/
GT_VOID prvTgfIpv6McRoutingCompressedRoot
(
    GT_VOID
);

/**
 * @internal prvTgfIpv4UcMcRoutingCompressedRoot function
 * @endinternal
 *
 * @brief Ipv4 unicast and multicast Prefixes add/delete with compressed root node.
 *        1. Add basic configurations.
 *        2. Check root node conversions for source bucket, embedded,
 *           compressed, regular.
*/
GT_VOID prvTgfIpv4UcMcRoutingCompressedRoot
(
    GT_VOID
);

/**
 *  @internal prvTgfIpvx4kVrfsAddDeleteCreateVrfs function
 *  @endinternal
 *
 *  @brief Create 4k Vrfs
*/
GT_VOID prvTgfIpvx4kVrfsAddDeleteCreateVrfs
(
    GT_VOID
);

/**
 * @internal  prvTgfIpvx4kVrfsAddDeleteAddPrefixes function
 * @endinternal
 *
 * @brief Add Ipv4/Ipv6 UC/MC prefixes
 *
*/
GT_VOID prvTgfIpvx4kVrfsAddDeleteAddPrefixes
(
    GT_VOID
);

/**
 * @internal  prvTgfIpvx4kVrfsAddDeleteDeleteVrfs
 * @endinternal
 *
 * @brief Delete Vrfs
 */
GT_VOID prvTgfIpvx4kVrfsAddDeleteDeleteVrfs
(
    GT_VOID
);

/**
 * @internal prvTgfIpv4UcRoutingVrfsAddDeleteForMerge function
 * @endinternal
 *
 * @brief Add and delete Vrfs and prefixes in each vrf such that
 *        defrag occurs in root bucket.
*/
GT_VOID prvTgfIpv4UcRoutingVrfsAddDeleteForMerge
(
    GT_VOID
);

/**
 * @internal prvTgfIpv4McValidityWithUcPresence function
 * @endinternal
 *
 * @brief Add UC prefixes then add MC prefixes to test 
 *        Mc validity logic.
*/
GT_VOID prvTgfIpv4McValidityWithUcPresence
(
    GT_VOID
);

#endif /*__prvTgfIpvxRoutingCompressedRooth*/
