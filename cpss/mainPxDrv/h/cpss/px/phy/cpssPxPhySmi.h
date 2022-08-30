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
* @file cpssPxPhySmi.h
*
* @brief API implementation for port Serial Management Interface facility.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssPxPhySmih
#define __cpssPxPhySmih

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/common/phy/cpssGenPhySmi.h>

/**
* @internal cpssPxPhySmiMdcDivisionFactorSet function
* @endinternal
*
* @brief   Sets fast MDC Division Selector
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] divisionFactor           - division factor of the core clock
*                                      to get the MDC (Serial Management Interface Clock)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPhySmiMdcDivisionFactorSet
(
    IN GT_SW_DEV_NUM devNum,
    IN CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactor
);

/**
* @internal cpssPxPhySmiMdcDivisionFactorGet function
* @endinternal
*
* @brief   Gets fast MDC Division Selector
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
*
* @param[out] divisionFactorPtr        - (pointer to) division factor of the core clock
*                                      to get the MDC (Serial Management Interface Clock)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPhySmiMdcDivisionFactorGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT *divisionFactorPtr
);

/**
* @internal cpssPxPhyXsmiMdcDivisionFactorSet function
* @endinternal
*
* @brief   Sets MDC frequency for Master XSMI Interface
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] divisionFactor           - division factor of the core clock
*                                      to get the MDC (Serial Management Interface Clock).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPhyXsmiMdcDivisionFactorSet
(
    IN GT_SW_DEV_NUM devNum,
    IN CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT divisionFactor
);

/**
* @internal cpssPxPhyXsmiMdcDivisionFactorGet function
* @endinternal
*
* @brief   Gets MDC frequency for Master XSMI Interface.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
*
* @param[out] divisionFactorPtr        - (pointer to) division factor of the core clock
*                                      to get the MDC (Serial Management Interface Clock)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPhyXsmiMdcDivisionFactorGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT *divisionFactorPtr
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxPhySmih */

