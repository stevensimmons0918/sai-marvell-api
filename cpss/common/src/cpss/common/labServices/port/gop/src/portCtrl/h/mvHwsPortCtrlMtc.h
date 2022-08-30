

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
* @file mvHwsPortCtrlMtc.h
*
* @brief Port Control Mtc Definitions
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsPortCtrlMtc_H
#define __mvHwsPortCtrlMtc_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#if defined( MV_PORT_MTC_TASK) || defined (MV_PORT_MTC_WO_TASK)
#ifdef MV_PORT_MTC_TASK
/**
* @internal mvHwsPortMaintananceRoutine function
* @endinternal
*  
* @brief execute the Routine (ACTIVE -> IDLE->)  
*
*/
void mvHwsPortMtcRoutine
(
    IN  void*      pvParameters
);
#endif
/**
* @internal mvPortMaintananceTransmitTemperature function
* @endinternal
*
* @brief   check if adaptive is running and transmit temperature
*
* @param[in] portIndex                 - number of physical port
*
* @retval GT_OK                    - on success/delete
* */
GT_STATUS mvPortMaintananceTransmitTemperature
(
    IN GT_U8 portIndex,
    IN GT_32 temperature
);

/**
* @internal mvPortMaintananceBroadcastTemperature function
* @endinternal
*
* @brief   broadcast temperature
*
* @param[in] temperature                 - temperature to
*       broadcast
*
* @retval GT_OK                    - on success/delete
* */
GT_STATUS mvPortMaintananceBroadcastTemperature
(
    IN GT_32 temperature
);

/**
* @internal mvPortCtrlPortMaintananceActive function
* @endinternal
*
* @brief   AP Detect Active state execution
*         - Scan all ports and perform maintanance actions
*/
void mvPortCtrlPortMaintananceActive(void);
#endif
#ifdef __cplusplus
#endif /* __cplusplus */

#endif /* __mvHwsPortCtrlMtc_H */



