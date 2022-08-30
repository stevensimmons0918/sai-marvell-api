
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
* @file prvTgfPclEgressUDB60PclId.h
*
* @brief EPCL 60B UDB Pcl ID test
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPclEgressUDB60PclIdh
#define __prvTgfPclEgressUDB60PclIdh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPclGen.h>

/**
* @enum PRV_TGF_EPCL_60B_PCL_ID_STAGE_ENT
 *
 * @brief
*/
typedef enum{

    /** stage 0 - rule's PCL ID does not match the PCL ID  */
    PRV_TGF_EPCL_60B_PCL_ID_STAGE_0_ENT,

    /** stage 1 - rule's PCL ID matches the PCL ID  */
    PRV_TGF_EPCL_60B_PCL_ID_STAGE_1_ENT,

} PRV_TGF_EPCL_60B_PCL_ID_STAGE_ENT;

/**
* @internal prvTgfPclEgreesUDB60PclIdDropCfgSet function
* @endinternal
*
* @brief  Configure Egress PCL Rule with:
*         action pkt cmd : CPSS_PACKET_CMD_TRAP_TO_CPU_E
*         and mask : pcl-id (according to the test stage)
*
*/
GT_VOID prvTgfPclEgreesUDB60PclIdDropCfgSet
(
    PRV_TGF_EPCL_60B_PCL_ID_STAGE_ENT stage
);

/**
* @internal prvTgfPclEgreesUDB60PclIdDropTrafficGenerateAndCheck function
* @endinternal
*
* @brief  Configure to send packet to port 1 and check counters
*/
GT_VOID prvTgfPclEgreesUDB60PclIdDropTrafficGenerateAndCheck
(
    PRV_TGF_EPCL_60B_PCL_ID_STAGE_ENT stage
);

/**
* @internal prvTgfPclEgreesUDB60PclIdDropCfgRestore function
* @endinternal
*
* @brief  restore test configurations
*/
GT_VOID prvTgfPclEgreesUDB60PclIdDropCfgRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
