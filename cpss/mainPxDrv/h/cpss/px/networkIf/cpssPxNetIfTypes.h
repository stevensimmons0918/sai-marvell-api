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
* @file cpssPxNetIfTypes.h
* @version   1
********************************************************************************
*/

#ifndef __cpssPxNetIfTypes_h
#define __cpssPxNetIfTypes_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************ Includes ********************************************************/
#include <cpss/common/cpssTypes.h>
#include <cpss/common/networkIf/cpssGenNetIfTypes.h>

/**
* @enum CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT
 *
 * @brief Defines the behavior in case of Rx SDMA resource error
*/
typedef enum{

    /** the packet remains scheduled for transmission. */
    CPSS_PX_NET_RESOURCE_ERROR_MODE_RETRY_E,

    /** the packet is dropped. */
    CPSS_PX_NET_RESOURCE_ERROR_MODE_ABORT_E

} CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT;

/**
* @struct CPSS_PX_NET_SDMA_RX_COUNTERS_STC
 *
 * @brief information about Rx counters -- per Queue (traffic class)
*/
typedef struct{

    /** Packets received on this queue since last read */
    GT_U32 rxInPkts;

    /** Octets received on this queue since last read */
    GT_U32 rxInOctets;

} CPSS_PX_NET_SDMA_RX_COUNTERS_STC;

/**
* @struct CPSS_PX_NET_TX_PARAMS_STC
 *
 * @brief structure of Tx parameters , that contain information on
 * how to send packet from CPU to PP.
*/
typedef struct{

    /** @brief the queue that packet should be served by the TX SDMA.
     *  (APPLICABLE RANGES: 0..7)
     */
    GT_U32 txQueue;

    /** @brief GT_TRUE
     *  GT_FALSE - leave packet unchanged.
     *  NOTE : The PX device always add 4 bytes of CRC when need to recalcCrc = GT_TRUE
     */
    GT_BOOL recalcCrc;

} CPSS_PX_NET_TX_PARAMS_STC;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif   /* __cpssPxNetIfTypes_h */


