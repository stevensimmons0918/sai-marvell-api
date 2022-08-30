/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxPortDynamicPAPortSpeedDB.c
*
* DESCRIPTION:
*       pipe (algorithmic) pizza arbiter Port Speed DB
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*******************************************************************************/
#ifndef __PRV_CPSS_PX_PORT_DYNAMIC_PA_PORT_SPEED_DB_H
#define __PRV_CPSS_PX_PORT_DYNAMIC_PA_PORT_SPEED_DB_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/px/port/cpssPxPortCtrl.h>

/**
* @internal prvCpssPxPortDynamicPAPortSpeedDBInit function
* @endinternal
*
* @brief   init port speed DB for given device
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
*/
GT_STATUS prvCpssPxPortDynamicPAPortSpeedDBInit
(
    IN GT_SW_DEV_NUM            devNum
);

/**
* @internal prvCpssPxPortDynamicPAPortSpeedDBSet function
* @endinternal
*
* @brief   set port BW in PA
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] speedInMBit              - speed in Mbps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
*/
GT_STATUS prvCpssPxPortDynamicPAPortSpeedDBSet
(
    IN    GT_SW_DEV_NUM            devNum,
    IN    GT_PHYSICAL_PORT_NUM     portNum,
    IN    GT_U32                   speedInMBit
);

/**
* @internal prvCpssPxPortDynamicPAPortSpeedDBGet function
* @endinternal
*
* @brief   Get port PA BW (if it has it)
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] isInitilizedPtr          - (pointer to)whether port is initialized
* @param[out] speedInMBitPtr           - (pointer to)speed in Mbps in case when  initialized
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, port num
* @retval GT_BAD_PTR               - null pointer
*/
GT_STATUS prvCpssPxPortDynamicPAPortSpeedDBGet
(
    IN    GT_SW_DEV_NUM              devNum,
    IN    GT_PHYSICAL_PORT_NUM       portNum,
    OUT   GT_BOOL                   *isInitilizedPtr,
    OUT   GT_U32                    *speedInMBitPtr
);



#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif


