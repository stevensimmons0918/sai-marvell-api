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
* @file prvCpssBrgVlanTypes.h
* @version   1
********************************************************************************
*/

#ifndef __prvCpssBrgVlanTypesh
#define __prvCpssBrgVlanTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/************ Includes ********************************************************/
#include <cpss/common/cpssTypes.h>

/* max num of vlans -- range 0..4096 */
#define PRV_CPSS_MAX_NUM_VLANS_CNS (4096)

/* check vlan value -- this macro check the the vid < 4096
    this macro used to check that the user not give over size value of vid
    into 12 bits of HW. (12 bits = 4095 max value)
*/
#define PRV_CPSS_VLAN_VALUE_CHECK_MAC(vid)    \
    if(vid >= PRV_CPSS_MAX_NUM_VLANS_CNS)   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif   /* __prvCpssBrgVlanTypesh */


