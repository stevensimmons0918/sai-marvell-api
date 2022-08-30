
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
* @file mvHwsPortCtrlCommonEng.h
*
* @brief Port Control AP and AN common Definitions
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsPortCtrlCommonEng_H
#define __mvHwsPortCtrlCommonEng_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal mvApRefClockUpdate function
* @endinternal
*
* @brief   update reference clock
*
* @param[in] refClockSrcFromDb     - source ref clock
* @param[in] validFromDb           - valid or not
* @param[in] cpllOutFreqFromDb     - cpll out frequency
* @param[out] refClockPtr          - pointer to updated ref clock
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE            - on unknown cpll value
* */
GT_STATUS mvApRefClockUpdate
(
    GT_U8                       refClockSrcFromDb,
    GT_U8                       validFromDb,
    GT_U8                       cpllOutFreqFromDb,
    MV_HWS_REF_CLOCK_SUP_VAL    *refClockPtr
);


/**
* @internal mvHwsApConvertPortMode function
* @endinternal
*
* @brief   update reference clock
*
* @param[in] apPortMode         - HCD port mode
*
* @retval MV_HWS_PORT_STANDARD  - port mode in HWS format
* */
MV_HWS_PORT_STANDARD mvHwsApConvertPortMode(MV_HWA_AP_PORT_MODE apPortMode);


/**
* @internal mvApSerdesPowerUp function
* @endinternal
*
* @brief   power up serdes to start AN
*
* @param[in] serdesNum         - serdes number
* @param[in] powerUp           - port number
* @param[in] portNum           - powerUp or powerDown
* @param[in] phase             - powerUp phase
*
* @retval 0                        - on success
* @retval 1                        - on error
* */
GT_STATUS mvApSerdesPowerUp(GT_U8 serdesNum, GT_BOOL powerUp, GT_U8 portNum, GT_U8 phase);


/**
* @internal mvApPolarityCfg function
* @endinternal
*
* @brief   Configure polarity of serdes lanes of port
*
* @param[in] portNum           - powerUp or powerDown
* @param[in] portMode          - port mode in HWS format
* @param[in] polarityVector    - polarity Tx, Rx
*
*
* @retval GT_OK                    - on success
* @retval otherwise                - error
*/
GT_STATUS mvApPolarityCfg
(
    GT_U8                portNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_U16               polarityVector
);

/**
* @internal prvPortCtrlApEngLinkUpCheck function
* @endinternal
*
* @brief   Check link/sync status, which means in AP terms - connection established
*         succesfully or not
* @param[in] portNum                  - port number
* @param[in] portMode                 - interface found by negotiation
* @param[in] fecMode                  - FEC mode found by negotiation(affects which PCS used)
*
* @param[out] linkUpPtr                - connection status
*                                       None.
*
* @retval GT_OK                    - on success
* @retval otherwise                - error
*/
GT_STATUS prvPortCtrlApEngLinkUpCheck
(
    GT_U8                   portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  fecMode,
    GT_BOOL                 *linkUpPtr
);

#ifndef BOBK_DEV_SUPPORT
/**
* @internal mvPortCtrlApPortGetMaxLanes function
* @endinternal
*
* @brief   get port max lanes accourding to capability
* @param[in] capability             - port capability
*
* */
GT_U32 mvPortCtrlApPortGetMaxLanes
(
    GT_U32 capability
);
#endif

#ifdef __cplusplus
#endif /* __cplusplus */

#endif /* __mvHwsPortCtrlCommonEng_H */

