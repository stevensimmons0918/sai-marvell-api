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
* @file cpssPxTypes.h
*
* @brief CPSS PX Generic types.
*
* @version   8
********************************************************************************
*/

#ifndef __cpssPxTypesh
#define __cpssPxTypesh


#include <cpss/common/cpssTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * typedef: CPSS_PX_PACKET_TYPE
 *
 * Description: scalar representing Packet Type
 *          (APPLICABLE RANGE: 0..31)
 */
typedef GT_U32  CPSS_PX_PACKET_TYPE;

/* check packet type */
#define CPSS_PX_PACKET_TYPE_MAX_CNS 32
#define CPSS_PX_PACKET_TYPE_CHECK_MAC(packetType)   \
    CPSS_DATA_CHECK_MAX_MAC(packetType,CPSS_PX_PACKET_TYPE_MAX_CNS)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxTypesh */


