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
* @file cpssPxPortDynamicPAUnitBW.h
*
* @brief pipe and higher dynamic (algorithmic) pizza arbiter
*
* @version   1
********************************************************************************
*/
#ifndef __CPSS_PX_PORT_DYNAMIC_PA_UNIT_BW_H
#define __CPSS_PX_PORT_DYNAMIC_PA_UNIT_BW_H

#include <cpss/common/cpssTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @enum CPSS_PX_MIN_SPEED_ENT
 *
 * @brief granularity speed used in some pizza algorithms
*/
typedef enum{

    /** invalide speed */
    CPSS_PX_MIN_SPEED_INVALID_E = 0

    /** granularity 0.5G */
    ,CPSS_PX_MIN_SPEED_500_Mbps_E

    /** granularity 1G */
    ,CPSS_PX_MIN_SPEED_1000_Mbps_E

    /** granularity 2G */
    ,CPSS_PX_MIN_SPEED_2000_Mbps_E

    /** granularity 5G */
    ,CPSS_PX_MIN_SPEED_5000_Mbps_E

    /** granularity 10G */
    ,CPSS_PX_MIN_SPEED_10000_Mbps_E

    ,CPSS_PX_MIN_SPEED_MAX

} CPSS_PX_MIN_SPEED_ENT;

/**
* @internal cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionSet function
* @endinternal
*
* @brief   set pipe BW for TxDMA, RXDMA, TxQ, Tx-FIFO, and slice resolution for TxQ
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] pipeBandwithInGbps       - pipe bandwidth in GBits
* @param[in] minimalPortSpeedMBps     - min port speed resolution in MBps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, combination of BW and slice resolution
* @retval GT_FAIL                  - on error
*
* @note a. API does not change system BW, it only informs pizza arbiter about requested BW.
*       b. API returns GT_BAD_PARAM, if number of available slices resolution is smaller than requested BW,
*       ex: PIPE has 340 slices, default resolution is 1G , max BW that can be supported is 340[slices] 1[Gbps/slice] = 340 [Gpbs]
*       ex: if one like to support BW 400Gpbs, than ceil(400[Gpbs]/340[slices]) = 2[Gpbs/slice] = 2000 [Mpbs/slice]
*       c. API checks whether there are ports with speed smaller than resolution. If any is present, API returns with error (GT_FAIL).
*       d. API cpssPxPortModeSpeedSet() will return error, if requested speed is smaller than speed resolution.
*
*/
GT_STATUS cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionSet
(
    IN GT_SW_DEV_NUM             devNum,
    IN GT_U32                    pipeBandwithInGbps,
    IN CPSS_PX_MIN_SPEED_ENT     minimalPortSpeedMBps
);

/**
* @internal cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionGet function
* @endinternal
*
* @brief   get pipe BW for TxDMA, RXDMA, TxQ, Tx-FIFO, and slice resolution for TxQ
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] pipeBandwithInGbpsPtr    - pipe bandwidth in GBits
* @param[out] minimalSliceResolutionInMBpsPtr - min port speed resolution in MBps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - wrong devNum, combination of BW and slice resolution
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionGet
(
    IN  GT_SW_DEV_NUM           devNum,
    OUT GT_U32                 *pipeBandwithInGbpsPtr,
    OUT CPSS_PX_MIN_SPEED_ENT  *minimalSliceResolutionInMBpsPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif




