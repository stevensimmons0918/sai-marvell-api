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
* @file prvCpssPxHwInitLedCtrl.h
*
* @brief PX : Private function for LED API definition.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssPxHwInitLedCtrl_h
#define __prvCpssPxHwInitLedCtrl_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>

/* this constant defines invalid port LED position */
#define PRV_CPSS_PX_PORT_LED_POSITION_VOID_CNS 0x3f

/**
* @internal prvCpssPxLedErrataPortLedMacTypeSet function
* @endinternal
*
* @brief   Init Port Led Mac type set and if postion is configured
*         writes position to approptiate mac
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - portNum
* @param[in] macType                  - macType, (PRV_CPSS_PORT_NOT_EXISTS_E, if MAC is deacivated)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssPxLedErrataPortLedMacTypeSet
(
    IN  GT_SW_DEV_NUM              devNum,
    IN  GT_PHYSICAL_PORT_NUM       portNum,
    IN  PRV_CPSS_PORT_TYPE_ENT     macType
);

/**
* @internal prvCpssPxLedErrataPortLedInit function
* @endinternal
*
* @brief   Init Port Led related Data Base
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssPxLedErrataPortLedInit
(
    IN  GT_SW_DEV_NUM                   devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPxHwInitLedCtrl_h */


