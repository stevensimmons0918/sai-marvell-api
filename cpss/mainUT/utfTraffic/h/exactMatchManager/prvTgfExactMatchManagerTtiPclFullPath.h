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
* @file prvTgfExactMatchManagerTtiPclFullPath.h
*
* @brief Test Exact Match Manager Expanded Action functionality with TTI and PCL
*        Configuration
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfExactMatchManagerTtiPclFullPathh
#define __prvTgfExactMatchManagerTtiPclFullPathh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfExactMatchManagerTtiPclFullPathManagerCreate function
* @endinternal
*
* @brief   Create Exact Match Manager with device
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathManagerCreate
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd
);

/**
* @internal prvTgfExactMatchManagerTtiPclFullPathReducedManagerCreate function
* @endinternal
*
* @brief   Create Exact Match Manager with device and set reduce entry to be used
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathReducedManagerCreate
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd
);

/**
* @internal prvTgfExactMatchManagerTtiPclFullPathReducedNonZeroKeyStartManagerCreate function
* @endinternal
*
* @brief   Create Exact Match Manager with device and set reduce entry to be used, with non zero key start
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathReducedNonZeroKeyStartManagerCreate
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd
);

/**
* @internal prvTgfExactMatchManagerTtiPclFullPathReducedNonFullKeyMaskManagerCreate function
* @endinternal
*
* @brief   Create Exact Match Manager with device and set reduce
*          entry to be used, with non full key mask
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathReducedNonFullKeyMaskManagerCreate
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd
);


/**
* @internal prvTgfExactMatchManagerTtiPclFullPathReducedUdb47ManagerCreate function
* @endinternal
*
* @brief   Create Exact Match Manager with device and set reduce
*          entry to be used, with UDB 47B key size
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathReducedUdb47ManagerCreate
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd
);

/**
* @internal prvTgfExactMatchManagerTtiPclFullPathConfigSet function
* @endinternal
*
* @brief   Set PCL/TTI test configuration related to Exact Match Expanded Action
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathConfigSet
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd
);

/**
* @internal prvTgfExactMatchManagerTtiPclFullPathReducedConfigSet function
* @endinternal
*
* @brief   Set PCL/TTI test configuration related to Exact Match Expanded Action
*          and Reduced Action
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathReducedConfigSet
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd
);
/**
* @internal prvTgfExactMatchManagerTtiPclFullPathReducedUdb47ConfigSet function
* @endinternal
*
* @brief   Set PCL/TTI test configuration related to Exact Match Expanded Action
*          and Reduced Action with keySize=47B
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathReducedUdb47ConfigSet
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd
);
/**
* @internal prvTgfExactMatchManagerTtiPclFullPathManagerDelete function
* @endinternal
*
* @brief   Delete Exact Match Manager
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathManagerDelete
(
    IN GT_U32                               exactMatchManagerId
);

/**
* @internal prvTgfExactMatchManagerTtiPclFullPathInvalidateEmEntry function
* @endinternal
*
* @brief   Delete Exact Match Entry from manager
* @param[in] exactMatchManagerId - manager Id
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathInvalidateEmEntry
(
    IN GT_U32                               exactMatchManagerId
);
/**
* @internal prvTgfExactMatchManagerTtiPclFullPathHaReplayInvalidEntry function
* @endinternal
*
* @brief   Add invalid entries and expect replay to fail.
*          Add a valid entry that will not be replayed and
*          expect this entry to be deleted in the complition stage.
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
* @param[in] firstCall           - GT_TRUE: calling this API for the first time
*                                  GT_FALSE: calling this API for the second time
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathHaReplayInvalidEntry
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd,
    IN GT_BOOL                              firstCall
);
/**
* @internal prvTgfExactMatchManagerTtiPclFullPathHaReplayInvalidManager function
* @endinternal
*
* @brief   Add invalid manager and expect replay to fail.
*
* @param[in] exactMatchManagerId - manager Id
* @param[in] pktCmd              - packet command to assign in Exact Match Action
*
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathHaReplayInvalidManager
(
    IN GT_U32                               exactMatchManagerId,
    IN CPSS_PACKET_CMD_ENT                  pktCmd
);
/**
* @internal prvTgfExactMatchManagerTtiPclFullPathHaAddBasicConfigSingleEntry function
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
GT_VOID prvTgfExactMatchManagerTtiPclFullPathHaAddBasicConfigSingleEntry
(
    IN GT_U32   exactMatchManagerId,
    IN GT_BOOL  firstCall
);
/**
* @internal prvTgfExactMatchManagerTtiPclFullPathHaValidityCheckAndTrafficSendSingleEntry function
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
GT_VOID prvTgfExactMatchManagerTtiPclFullPathHaValidityCheckAndTrafficSendSingleEntry
(
    IN GT_U32   exactMatchManagerId
);
/**
* @internal prvTgfExactMatchManagerTtiPclFullPathHaSystemRecoveryStateSet function
* @endinternal
*
* @brief  Keep Exact Match Manager DB values for reconstruct and
*         set flag for HA process.
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathHaSystemRecoveryStateSet();


/**
* @internal prvTgfExactMatchManagerTtiPclFullPathHaDelBasicConfigSingleEntry function
* @endinternal
*
* @brief  Delete all Exact Match configuration done
*
* @param[in] exactMatchManagerId - manager Id
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathHaDelBasicConfigSingleEntry
(
    IN GT_U32   exactMatchManagerId
);

/**
* @internal prvTgfExactMatchManagerTtiPclFullPathHaSyncSwHwSingleEntry function
* @endinternal
*
* @brief  do any suncronization left between HW and Shadow
*         set complition state
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathHaSyncSwHwSingleEntry();

/**
* @internal prvTgfExactMatchManagerTtiPclFullPathHaRestoreBasicConfigSingleEntry function
* @endinternal
*
* @brief  restore old configuration and delete exact match entry and manager
*
* @param[in] exactMatchManagerId - manager Id
*/
GT_VOID prvTgfExactMatchManagerTtiPclFullPathHaRestoreBasicConfigSingleEntry
(
    IN GT_U32   exactMatchManagerId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
