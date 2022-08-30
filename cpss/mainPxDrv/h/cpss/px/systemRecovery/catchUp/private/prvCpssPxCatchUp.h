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
* @file prvCpssPxCatchUp.h
*
* @brief private CPSS Px catch up data.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssPxCatchUph
#define __prvCpssPxCatchUph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* prvCpssDrvPPinitPollingEnableSet
*
* DESCRIPTION:
*       enable/disable Interrupt handler task (polling mode).
* INPUTS:
*       devNum - devNum
*       enable - enable/disable enter to rotine
*/
extern GT_STATUS prvCpssDrvPPinitPollingEnableSet
(
    GT_U8  devNum,
    GT_BOOL enable
);

/*******************************************************************************
* prvCpssCatchUpFuncPtr
*
* DESCRIPTION:
*       Define type of functions pointer performing synchronization of sw DB
*       with hw contents.
*
* APPLICABLE DEVICES:
*       Pipe.
*
* NOT APPLICABLE DEVICES:
*       None.
*
* INPUTS:
*       devNum - device number.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK                    - on success
*       GT_HW_ERROR              - on hardware error
*       GT_FAIL                  - on failure
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* COMMENTS:
*       None.
*
*******************************************************************************/
typedef GT_STATUS (*prvCpssCatchUpFuncPtr)(GT_U8 devNum);


/**
* @internal prvCpssPxSystemRecoveryCatchUpHandle function
* @endinternal
*
* @brief   Perform synchronization of hardware data and software DB after special init sequence.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxSystemRecoveryCatchUpHandle
(
   GT_VOID
);

/**
* @internal prvCpssPxHwDevNumCatchUp function
* @endinternal
*
* @brief   Synchronize hw device number in software DB by its hw value
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_OUT_OF_RANGE          - on hwDevNum > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxHwDevNumCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssPxCpuPortModeCatchUp function
* @endinternal
*
* @brief   Synchronize cpu port mode in software DB by its hw value
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_OUT_OF_RANGE          - on hwDevNum > 31
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxCpuPortModeCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssPxPortModeParamsCatchUp function
* @endinternal
*
* @brief   Synchronize Port Mode parameters in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortModeParamsCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssPxPortResourcesCatchUp function
* @endinternal
*
* @brief   Synchronize Port resources parameters in software DB by its hw values
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortResourcesCatchUp
(
    IN GT_U8    devNum
);

/**
* @internal prvCpssPxSystemRecoveryCompletionHandle function
* @endinternal
*
* @brief   Perform synchronization of hardware data and software DB after special init sequence.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  none.
*
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxSystemRecoveryCompletionHandle
(
   GT_VOID
);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPxHsuh */


