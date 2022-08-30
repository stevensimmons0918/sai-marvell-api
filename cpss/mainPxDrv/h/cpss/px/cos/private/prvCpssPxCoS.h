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
* @file prvCpssPxCoS.h
*
* @brief PX : COS structures definition .
*
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssPxCoS_h
#define __prvCpssPxCoS_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/cos/cpssCosTypes.h>


/* macro to validate the value of trafic class parameter */
#define PRV_CPSS_PX_COS_CHECK_TC_MAC(trfClass) \
    CPSS_PARAM_CHECK_MAX_MAC(trfClass,CPSS_TC_RANGE_CNS)

/* convert DP level to HW values */
#define PRV_CPSS_PX_COS_DP_TO_HW_CONVERT_MAC(_dpLevel, _hwValue)  \
    switch(_dpLevel)                                              \
    {                                                             \
        case CPSS_DP_GREEN_E:  _hwValue = 0; break;               \
        case CPSS_DP_YELLOW_E: _hwValue = 1; break;               \
        case CPSS_DP_RED_E:    _hwValue = 2; break;               \
        default:                                                  \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);  \
    }

/* convert HW values to DP level */
#define PRV_CPSS_PX_COS_HW_TO_DP_CONVERT_MAC(_hwValue, _dpLevel)  \
    switch(_hwValue)                                              \
    {                                                             \
        case 0:  _dpLevel = CPSS_DP_GREEN_E;  break;              \
        case 1:  _dpLevel = CPSS_DP_YELLOW_E; break;              \
        case 2:  _dpLevel = CPSS_DP_RED_E;    break;              \
        default:                                                  \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);  \
    }


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPxCoS_h */

