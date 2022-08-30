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
* @file smacHash.h
*
* @brief Hash calculate for MAC address table definition.
*
* @version   6
********************************************************************************
*/

#ifndef __smacHashh
#define __smacHashh

#include <os/simTypes.h>

/**
* @enum SGT_MAC_TBL_SIZE
 *
 * @brief Enumeration of MAC Table size in entries
*/
typedef enum{

    SGT_MAC_TBL_8K = 0,

    SGT_MAC_TBL_16K,

    SGT_MAC_TBL_4K

} SGT_MAC_TBL_SIZE;

/**
* @enum GT_MAC_VL
 *
 * @brief Enumeration of VLAN Learning modes
*/
typedef enum{

    /** Independent VLAN Learning */
    GT_IVL=1,

    /** Shared VLAN Learning */
    GT_SVL

} GT_MAC_VL;


/**
* @enum GT_ADDR_LOOKUP_MODE
 *
 * @brief Enumeration of Address lookup modes
*/
typedef enum{

    /** @brief Optimized for sequential MAC addresses and
     *  sequential VLAN id's.
     */
    GT_MAC_SQN_VLAN_SQN = 0,

    /** @brief Optimized for random MAC addresses and
     *  sequential VLAN id's. Reserved.
     */
    GT_MAC_RND_VLAN_SQN,

    /** @brief Optimized for sequential MAC addresses and
     *  random VLAN id's. Reserved.
     */
    GT_MAC_SQN_VLAN_RND,

    /** @brief Optimized for random MAC addresses and
     *  random VLAN id's.
     */
    GT_MAC_RND_VLAN_RND

} GT_ADDR_LOOKUP_MODE;

/**
* @enum GT_MAC_HASH_KIND
 *
 * @brief Enumeration of Mac hash function kind,
*/
typedef enum{

    /** old hash function. */
    GT_OLD_MAC_HASH_FUNCTION = 0,

    /** improved hash function.. */
    GT_NEW_MAC_HASH_FUNCTION = 1

} GT_MAC_HASH_KIND;


/**
* @struct SGT_MAC_HASH
 *
 * @brief struct contains the hardware parameters for hash mac Address
 * calculates.
*/
typedef struct{

    /** @brief the MAC lookup mask
     *  are used in the MAC table lookup.
     *  the array length need to be 6. (48 bits)
     */
    GT_ETHERADDR macMask;

    /** the VLAN lookup mode. */
    GT_MAC_VL vlanMode;

    /** @brief the Vid lookup cyclic shift left.
     *  the vidShift range is 0-2.
     */
    GT_U8 vidShift;

    /** @brief the Vid lookup mask
     *  are used in the MAC table lookup.
     *  the vidMask is 12 bit.
     */
    GT_U16 vidMask;

    /** @brief the Vid lookup cyclic shift left.
     *  the macShift range is 0-5.
     */
    GT_U8 macShift;

    /** the address lookup mode. */
    GT_ADDR_LOOKUP_MODE addressMode;

    /** the entries number in the hash table. */
    SGT_MAC_TBL_SIZE size;

    /** mac hash chain length. */
    GT_U32 macChainLen;

    GT_MAC_HASH_KIND macHashKind;

} SGT_MAC_HASH;


/**
* @internal sgtMacHashCalc function
* @endinternal
*
* @brief   This function calculates the hash index for the mac address table.
*         for specific mac address and VLAN id.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS sgtMacHashCalc
(
    IN  GT_ETHERADDR   	*	addr,
    IN  GT_U16          	vid,
    IN  SGT_MAC_HASH 	*   macHashStructPtr,
    OUT GT_U32          *	hash
);

#endif /* __macHashh */

