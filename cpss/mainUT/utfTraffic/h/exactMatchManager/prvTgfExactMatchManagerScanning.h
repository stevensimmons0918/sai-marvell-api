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
* @file prvTgfExactMatchManagerScanning.h
*
* @brief Test Exact Match Manager scanning functionality with full capacity
*        exact match entries
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfExactMatchManagerScanningh
#define __prvTgfExactMatchManagerScanningh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_db.h>
#endif /*CHX_FAMILY*/

/**
* @internal prvTgfExactMatchManagerScanningReducedUdb19ConfigSet function
* @endinternal
*
* @brief   Set PCL/TTI test configuration related to Exact Match Expanded Action
*          and Reduced Action with keySize=19B
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerScanningReducedUdb19ConfigSet
(
    IN GT_U32                               exactMatchManagerId,
    CPSS_PACKET_CMD_ENT                     pktCmd
);
/**
* @internal prvTgfExactMatchManagerScanningEntriesAdd function
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
GT_VOID prvTgfExactMatchManagerScanningEntriesAdd
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd,
    IN GT_BOOL                              firstCall
);
/**
* @internal prvTgfExactMatchManagerScanningEntriesDelete
*           function
* @endinternal
*
* @brief   Delete entries added to the exact match table
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerScanningEntriesDelete
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd
);
/**
* @internal prvTgfExactMatchManagerScanningManagerDelete
*           function
* @endinternal
*
* @brief   Delete Exact Match Manager
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerScanningManagerDelete
(
    IN GT_U32                               exactMatchManagerId
);
/*
* @internal prvTgfExactMatchManagerScanningBasicConfigAdd
*           function
* @endinternal
*
* @brief
*       configure TTI , PCL and Excat Match Manager in such way that
*       we can send a packet that will get a match in the Exact Match
*       Entry. This is a basic configuration for a Sanning test
*       using a single Exact Match Entry.
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] firstCall           - GT_TRUE: calling this API for the first time
*                                  GT_FALSE: calling this API for the second time
* @param[in] scanDelete          - GT_TRUE: set configuration for scan delete test
*                                  GT_FALSE: set configuration for scan ageout test
*
*/
GT_VOID prvTgfExactMatchManagerScanningBasicConfigAdd
(
    IN GT_U32   exactMatchManagerId,
    IN GT_BOOL  firstCall,
    IN GT_BOOL  scanDelete
);
/**
* @internal
*           prvTgfExactMatchManagerScanningValidityCheckAndTrafficSend
*           function
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
GT_VOID prvTgfExactMatchManagerScanningValidityCheckAndTrafficSend
(
    IN GT_U32   exactMatchManagerId
);

/**
* @internal prvTgfExactMatchManagerScanningDeleteFilterConfig
*           function
* @endinternal
*
* @brief  Delete a single entry by using scan delete mechanism
*
* @param[in] exactMatchManagerId - manager Id
*/
GT_VOID prvTgfExactMatchManagerScanningDeleteFilterConfig
(
    IN GT_U32   exactMatchManagerId
);

/**
* @internal prvTgfExactMatchManagerScanningDelBasicConfig
*           function
* @endinternal
*
* @brief  Delete all Exact Match configuration done
*
* @param[in] exactMatchManagerId - manager Id
*/
GT_VOID prvTgfExactMatchManagerScanningDelBasicConfig
(
    IN GT_U32   exactMatchManagerId
);

/**
* @internal prvTgfExactMatchManagerScanningBasicConfigRestore
*           function
* @endinternal
*
* @brief  restore old configuration and delete exact match entry and manager
*
* @param[in] exactMatchManagerId - manager Id
*/
GT_VOID prvTgfExactMatchManagerScanningBasicConfigRestore
(
    IN GT_U32   exactMatchManagerId
);
/**
* @internal prvTgfExactMatchManagerScanningAgingEntriesAdd function
* @endinternal
*
* @brief   Add entries to the exact match table with rehashEnable=GT_TRUE
*          and aging configuration
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
* @param[in] firstCall           - GT_TRUE: calling this API for the first time
*                                  GT_FALSE: calling this API for the second time
*
*/
GT_VOID prvTgfExactMatchManagerScanningAgingEntriesAdd
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd,
    IN GT_BOOL                              firstCall
);

/**
* @internal prvTgfExactMatchManagerScanningAgeoutFilterConfig
*           function
* @endinternal
*
* @brief  Aging filter mechanism check
*
* @param[in] exactMatchManagerId - manager Id
*/
GT_VOID prvTgfExactMatchManagerScanningAgeoutFilterConfig
(
    IN GT_U32   exactMatchManagerId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
