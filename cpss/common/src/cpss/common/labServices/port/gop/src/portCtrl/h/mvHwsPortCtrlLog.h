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
* @file mvHwsPortCtrlLog.h
*
* @brief Port Control Log
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsPortCtrlLog_H
#define __mvHwsPortCtrlLog_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal mvPortCtrlLogInit function
* @endinternal
*
*/
GT_STATUS mvPortCtrlLogInit(void);

/**
* @internal mvPortCtrlLogInfoGet function
* @endinternal
*
*/
void mvPortCtrlLogInfoGet(GT_U32 *fwBaseAddr, GT_U32 *logBaseAddr, GT_U32 *logPointer,
                          GT_U32 *logCount, GT_U32 *logReset);

void mvHwsLogInfoGet(GT_U32 *fwBaseAddr, GT_U32 *hwsLogBaseAddr, GT_U32 *hwsLogPointerAddr,
                     GT_U32 *hwsLogCountAddr, GT_U32 *hwsLogResetAddr);

/**
* @internal mvPortCtrlLogAdd function
* @endinternal
*
*/
void mvPortCtrlLogAdd(GT_U32 entry);

#ifdef RAVEN_DEV_SUPPORT
/**
* @internal mvPortCtrlLogAddStatus function
* @endinternal
*
* @brief   Add Log status entry to the Log and override last
*         status
*/
void mvPortCtrlLogAddStatus
(
    GT_U32 idx,
    GT_U32 entry
);
#endif

/**
* @internal mvPortCtrlLogDump function
* @endinternal
*
*/
void mvPortCtrlLogDump(GT_U32 port);

/**
* @internal mvPortCtrlLogLock function
* @endinternal
*
*/
GT_STATUS mvPortCtrlLogLock(void);
GT_STATUS mvPortCtrlLogUnlock(void);

/**
* @internal arbSmStateDesc function
* @endinternal
*
* @brief   Print AP ARB State machine status description
*/
void arbSmStateDesc(GT_U32 mask, GT_U32 statusId);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvHwsPortCtrlLog_H */

