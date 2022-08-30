/*******************************************************************************
*              (c), Copyright 2019, Marvell International Ltd.                 *
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
* @file prvTgfFlowManager.h
*
* @brief FlowManager test header file
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfFlowManager
#define __prvTgfFlowManager

#include <trafficEngine/tgfTrafficGenerator.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <cpss/dxCh/dxChxGen/flowManager/cpssDxChFlowManager.h>
#include <cpss/dxCh/dxChxGen/flowManager/prvCpssDxChFlowManagerDb.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @internal prvTgfFlowManagerInit function
* @endinternal
*
* @brief Prepare test configuration
*
* @param[in] flowMngId                - unique flow manager id
* @param[in] flowFieldEnable          - to enable / disabl flow field config
*                                       GT_TRUE -- flow field enable
*                                       GT_FALSE -- flow field disable
* @param[in] packetType               - flow based packet type
*/
GT_STATUS prvTgfFlowManagerInit 
(
    IN GT_U32                                 flowMngId,
    IN GT_BOOL                                flowFieldEnable,
    IN CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_ENT packetType
);

/**
* @internal prvTgfFlowManagerQinQInit function
* @endinternal
*
* @brief Prepare test configuration for qinq based flow classification
*
* @param[in] flowMngId                - unique flow manager id
* @param[in] flowFieldEnable          - to enable / disabl flow field config
*                                       GT_TRUE -- flow field enable
*                                       GT_FALSE -- flow field disable
* @param[in] packetType               - flow based packet type
*/
GT_STATUS prvTgfFlowManagerQinQInit 
(
    IN GT_U32                                 flowMngId,
    IN GT_BOOL                                flowFieldEnable,
    IN CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_ENT packetType
);

/**
* @internal prvTgfFlowManagerMixedFlowInit function
* @endinternal
*
* @brief Prepare test configuration
*
* @param[in] flowMngId                - unique flow manager id
* @param[in] flowFieldEnable          - to enable / disabl flow field config
*                                       GT_TRUE -- flow field enable
*                                       GT_FALSE -- flow field disable
*/
GT_STATUS prvTgfFlowManagerMixedFlowInit 
(
    IN GT_U32                                 flowMngId,
    IN GT_BOOL                                flowFieldEnable
);

/**
* @internal prvTgfFlowManagerDelete function
* @endinternal
*
* @brief Restore/delete test configuration
*
* @param[in] flowMngId                - unique flow manager id
*/
GT_STATUS prvTgfFlowManagerDelete
(
    IN GT_U32                                 flowMngId
);

/**
* @internal tgfPortTxFlowManagerResourcesTcpTrafficGenerate function
* @endinternal
*
* @brief Send tcp traffic
*
* @param[in] packetType               - flow based packet type
*/
GT_STATUS tgfPortTxFlowManagerResourcesMultiTrafficGenerate
(
    IN CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_ENT packetType
);

/**
* @internal tgfPortQinQTxFlowManagerResourcesMultiTrafficGenerate function
* @endinternal
*
* @brief Send qinq tcp/udp/other traffic
*
* @param[in] packetType               - flow based packet type
*/
GT_STATUS tgfPortQinQTxFlowManagerResourcesMultiTrafficGenerate
(
    IN CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_ENT packetType
);

/**
* @internal tgfPortTxFlowManagerResourcesMixedTrafficGenerate function
* @endinternal
*
* @brief Send ipv4 mixed traffic 
*
*/
GT_STATUS tgfPortTxFlowManagerResourcesMixedTrafficGenerate
(
);

/**
* @internal tgfPortTxFlowManagerResourcesMixedPortTrafficGenerate function
* @endinternal
*
* @brief Send ipv4 mixed traffic 
*
*/
GT_STATUS tgfPortTxFlowManagerResourcesMixedPortTrafficGenerate
(
);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ____prvTgfFlowManager */


