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
* @file  prvTgfPacketExceptionCheck.h
*
* @brief Packet Exception Check Test
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPacketExceptionCheckh
#define __prvTgfPacketExceptionCheckh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* port to send traffic to */
#define PRV_TGF_SEND_PORT_NUM   0

/* traffic class queue */
#define PRV_TGF_TC_QUEUE_NUM   1

/**
* @internal prvTgfPacketExceptionCheckConfigurationSet function
* @endinternal
*
* @brief  Packet exception check configuration
*
* @param[in] sendPortNum      - source port number
* @param[in] queueIdx         - traffic class queue
*
*/
extern GT_STATUS prvTgfPacketExceptionCheckConfigurationSet
(
    IN GT_U32 sendPortNum,
    IN GT_U8  queueIdx
);

/**
* @internal prvTgfPacketExceptionCheckTrafficTest function
* @endinternal
*
* @brief  Packet exception check trafic test
*
* @param[in] sendPortNum      - port number from which traffic is ingress to device
*
*/
GT_STATUS prvTgfPacketExceptionCheckTrafficTest
(
    IN GT_U32 sendPortNum
);

/**
* @internal prvTgfPclIngressReflectActionRestore function
* @endinternal
*
* @brief  Packet exception check configuration restore
*/
GT_STATUS prvTgfPacketExceptionCheckConfigurationRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
