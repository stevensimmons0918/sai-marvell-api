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
* @file prvTgfExactMatchManagerHaFullCapacity.h
*
* @brief Teest Exact Match Manager HA functionality with full capacity
*        exact match entries
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfExactMatchManagerHaFullCapacityh
#define __prvTgfExactMatchManagerHaFullCapacityh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfExactMatchManagerHaFullCapacityReducedUdb19ConfigSet function
* @endinternal
*
* @brief   Set PCL/TTI test configuration related to Exact Match Expanded Action
*          and Reduced Action with keySize=19B
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerHaFullCapacityReducedUdb19ConfigSet
(
    IN GT_U32                               exactMatchManagerId,
    CPSS_PACKET_CMD_ENT                     pktCmd
);
/**
* @internal prvTgfExactMatchManagerHaFullCapacityEntriesAdd function
* @endinternal
*
* @brief   Add entries to the exact match table with rehashEnable=GT_TRUE
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
* @param[in] firstCall           - GT_TRUE: calling this API for the first time
*                                  GT_FALSE: calling this API for the second time
*
*/
GT_VOID prvTgfExactMatchManagerHaFullCapacityEntriesAdd
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd,
    IN GT_BOOL                              firstCall
);
/**
* @internal prvTgfExactMatchManagerHaFullCapacityEntriesDelete function
* @endinternal
*
* @brief   Delete entries added to the exact match table
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerHaFullCapacityEntriesDelete
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd
);
/**
* @internal prvTgfExactMatchManagerHaFullCapacityManagerDelete function
* @endinternal
*
* @brief   Delete Exact Match Manager
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerHaFullCapacityManagerDelete
(
    IN GT_U32                               exactMatchManagerId
);
/**
* @internal prvTgfExactMatchManagerHaFullCapacityBasicConfigAdd function
* @endinternal
*
* @brief
*       configure TTI , PCL and Excat Match Manager in such way that
*       we can send a packet that will get a match in the Exact Match
*       Entry. This is a basic configuration for a HA test using a
*       single Exact Match Entry.
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] firstCall           - GT_TRUE: calling this API for the first time
*                                  GT_FALSE: calling this API for the second time
*
*/
GT_VOID prvTgfExactMatchManagerHaFullCapacityBasicConfigAdd
(
    IN GT_U32   exactMatchManagerId,
    IN GT_BOOL  firstCall
);
/**
* @internal prvTgfExactMatchManagerHaFullCapacityValidityCheckAndTrafficSend function
* @endinternal
*
* @brief
*      call validity check and sent traffic
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] firstCall           - GT_TRUE: calling this API for the first time
*                                  GT_FALSE: calling this API for the second time
*
*/
GT_VOID prvTgfExactMatchManagerHaFullCapacityValidityCheckAndTrafficSend
(
    IN GT_U32   exactMatchManagerId
);
/**
* @internal prvTgfExactMatchManagerHaFullCapacitySystemRecoveryStateSet function
* @endinternal
*
* @brief  Keep Exact Match Manager DB values for reconstruct and
*         set flag for HA process.
*/
GT_VOID prvTgfExactMatchManagerHaFullCapacitySystemRecoveryStateSet();

/**
* @internal prvTgfExactMatchManagerHaFullCapacityDelBasicConfig function
* @endinternal
*
* @brief  Delete all Exact Match configuration done
*
* @param[in] exactMatchManagerId - manager Id
*/
GT_VOID prvTgfExactMatchManagerHaFullCapacityDelBasicConfig
(
    IN GT_U32   exactMatchManagerId
);
/**
* @internal prvTgfExactMatchManagerHaFullCapacitySyncSwHw function
* @endinternal
*
* @brief  do any suncronization left between HW and Shadow
*         set complition state
*/
GT_VOID prvTgfExactMatchManagerHaFullCapacitySyncSwHw();

/**
* @internal prvTgfExactMatchManagerHaFullCapacityBasicConfigRestore function
* @endinternal
*
* @brief  restore old configuration and delete exact match entry and manager
*
* @param[in] exactMatchManagerId - manager Id
*/
GT_VOID prvTgfExactMatchManagerHaFullCapacityBasicConfigRestore
(
    IN GT_U32   exactMatchManagerId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
