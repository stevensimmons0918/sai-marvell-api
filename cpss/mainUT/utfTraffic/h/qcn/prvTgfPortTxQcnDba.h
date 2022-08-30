/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
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
* @file prvTgfPortTxQcnDba.h
*
* @brief "QCN DBA" enhanced UT for QCN Trigger APIs
*
* @version   1
********************************************************************************
*/

#ifndef __prvTgfPortTxQcnDbah
#define __prvTgfPortTxQcnDbah

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <common/tgfQcnGen.h>

/**
* @internal tgfPortTxQcnQueueResourcesConfigure function
* @endinternal
*
* @brief Prepare or restore  test configuration
*
* @param[in] configSet    - store/restore configuration
*                           GT_TRUE  -- configure
*                           GT_FALSE -- restore
* @param[in] ingressPktHdr - inlude/exclude IngressPktHdr in QCN
*                            GT_TRUE  -- include
*                            GT_FALSE -- exclude
*/
GT_VOID tgfPortTxQcnQueueResourcesConfigure
(
    GT_BOOL configSet,
    GT_BOOL ingressPktHdr
);

/**
* @internal tgfPortTxQcnDbaQueueResourcesConfigure function
* @endinternal
*
* @brief Prepare or restore tgfPortTxQcnDbaQueueResourcesConfigure test configuration
*
* @param[in] configSet    - store/restore configuration
*                           GT_TRUE  -- configure
*                           GT_FALSE -- restore
*/
GT_VOID tgfPortTxQcnDbaQueueResourcesConfigure
(
    GT_BOOL configSet
);

/**
* @internal tgfPortTxQcnPortResourcesTrafficGenerate function
* @endinternal
*
* @brief Generate traffic fot tgfPortTxQcnPortResourcesTrafficGenerate test.
*
* @param[in] ingressPktHdr - inlude/exclude IngressPktHdr in QCN
*                            GT_TRUE  -- include
*                            GT_FALSE -- exclude
*/
GT_VOID tgfPortTxQcnPortResourcesTrafficGenerate
(
    GT_BOOL ingressPktHdr
);

/**
* @internal tgfPortTxQcnDbaPortResourcesTrafficGenerate function
* @endinternal
*
* @brief Generate traffic fot tgfPortTxQcnDbaPortResourcesTrafficGenerate test.
*
* @param[in] ingressPktHdr - inlude/exclude IngressPktHdr in QCN
*                            GT_TRUE  -- include
*                            GT_FALSE -- exclude
*/
GT_VOID tgfPortTxQcnDbaPortResourcesTrafficGenerate
(
    GT_BOOL ingressPktHdr
);

/**
* @internal tgfPortTxSip6QcnQueueResourcesConfigure function
* @endinternal
*
* @brief Prepare or restore tgfPortTxSip6QcnQueueResources test configuration
*
* @param[in] configSet       - store/restore configuration
*                              GT_TRUE  -- configure
*                              GT_FALSE -- restore
* @param[in] ingressPktHdr   - include/exclude the incoming packet header
*                              GT_TRUE -- include incoming packet header
*                              GT_FALSE -- esclude incoming pakcet header
* @param[in] resource        - specifies the global/pool available buffer limit
*                              for QCN DBA
*                              CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_GLOBAL_E/
*                              CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_POOL_E
* @param[in] poolNum         - specifies the pool number 0/1
* @param[in] queuePortLimits - specifies the QCN is triggered based on
*                              queue/port profile limits
*                              PRV_TGF_PORT_CN_QUEUE_LIMIT_E/
*                              PRV_TGF_PORT_CN_PORT_LIMIT_E
*/
GT_VOID tgfPortTxSip6QcnQueueResourcesConfigure
(
    GT_BOOL configSet,
    GT_BOOL ingressPktHdr,
    CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT    resourceMode,
    GT_U32                                      poolNum,
    PRV_TGF_PORT_CN_LIMIT_ENT                   queuePortLimits
);

/**
* @internal tgfPortTxSip6QcnDbaPortResourcesTrafficGenerate function
* @endinternal
*
* @brief Generate traffic for tgfPortTxSip6QcnDbaQueueResources test.
*
* @param[in] ingressPktHdr   - include/exclude the incoming packet header
*                              GT_TRUE -- include incoming packet header
*                              GT_FALSE -- esclude incoming pakcet header
* @param[in] queuePortLimits - specifies the QCN is triggered based on
*                              queue/port profile limits
*                              PRV_TGF_PORT_CN_QUEUE_LIMIT_E/
*                              PRV_TGF_PORT_CN_PORT_LIMIT_E
* @param[in] resourceMode    - specifies the global/pool available buffer limit
*                              for QCN DBA
*                              CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_GLOBAL_E/
*                              CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_POOL_E
*/
GT_VOID tgfPortTxSip6QcnDbaPortResourcesTrafficGenerate
(
    GT_BOOL                                     ingressPktHdr,
    PRV_TGF_PORT_CN_LIMIT_ENT                   queuePortLimits,
    CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT    resourceMode
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPortTxQcnDbah */

