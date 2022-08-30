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
* @file prvCpssDxChPortDpIronman.h
*
* @brief  CPSS Ironman Data Path Pizza and resource configuration.
*
* @version   0
********************************************************************************
*/
#ifndef __prvCpssDxChPortDpIronman_h
#define __prvCpssDxChPortDpIronman_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/port/cpssPortCtrl.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/port/private/prvCpssPortTypes.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceHawk.h>

/**
* @internal prvCpssDxChPortDpIronmanDeviceInit function
* @endinternal
*
* @brief    Initialyze Ironman DP units.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in]  devNum                - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_PTR               - on null pointer
*/
GT_STATUS prvCpssDxChPortDpIronmanDeviceInit
(
    IN    GT_U8                                devNum
);

/**
* @internal prvCpssDxChPortDpIronmanPortDown function
* @endinternal
*
* @brief    Disable port on all DP, PB, PCA and MIF units.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChPortDpIronmanPortDown
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum
);

/**
* @internal prvCpssDxChPortDpIronmanPortConfigure function
* @endinternal
*
* @brief    Configure port on all DP, PB, PCA and MIF units.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] portNum                - physical port number
* @param[in] ifMode                 - Interface mode,
*                                     not relevant SDMA, PCA units not configured
* @param[in] portSpeed              - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChPortDpIronmanPortConfigure
(
    IN    GT_U8                                devNum,
    IN    GT_PHYSICAL_PORT_NUM                 portNum,
    IN    CPSS_PORT_INTERFACE_MODE_ENT         ifMode,
    IN    CPSS_PORT_SPEED_ENT                  portSpeed
);

/**
* @internal prvCpssDxChPortDpIronmanPortMifPfcEnableSet function
* @endinternal
*
* @brief  Set MIF PFC enable/disable
*
* @param[in] devNum                - system device number
* @param[in] globalMac             - global MAC number
* @param[in] enableTx              - Tx PFC: GT_TRUE - enable, GT_FALSE - disable
* @param[in] enableRx              - Rx PFC: GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval other                    - on error
*/
GT_STATUS prvCpssDxChPortDpIronmanPortMifPfcEnableSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               globalMac,
    IN  GT_BOOL              enableTx,
    IN  GT_BOOL              enableRx
);

/**
* @internal prvCpssDxChPortDpIronmanPortMifPfcEnableGet function
* @endinternal
*
* @brief  Get MIF PFC enable/disable
*
* @param[in] devNum                - system device number
* @param[in] globalMac             - global MAC number
* @param[out] enableTx             - (Pointer to)Tx PFC: GT_TRUE - enable, GT_FALSE - disable
* @param[out] enableRx             - (Pointer to)Rx PFC: GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval other                    - on error
*/
GT_STATUS prvCpssDxChPortDpIronmanPortMifPfcEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               globalMac,
    OUT GT_BOOL              *enableTxPtr,
    OUT GT_BOOL              *enableRxPtr
);

/**
* @internal prvCpssDxChPortDpIronmanMifUnitChannelDump function
* @endinternal
*
* @brief    Dump MIF unit channel data.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon;  AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - physical device number
* @param[in] unitNum                - MIF unit number
* @param[in] channelNum             - MIF channel number inside the unit 
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_BAD_STATE             - other channel slots overlapped
*/
GT_STATUS prvCpssDxChPortDpIronmanMifUnitChannelDump
(
    IN    GT_U8                  devNum,
    IN    GT_U32                 unitNum,
    IN    GT_U32                 channelNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__prvCpssDxChPortDpIronman_h*/

