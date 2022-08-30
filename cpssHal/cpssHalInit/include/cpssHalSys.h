/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/********************************************************************************
* cpssHalInitializeSystemConfig.h
*
* DESCRIPTION:
*       define system configuration paramaters, that cannot be read from the device
*       example for configuration:
*           Pci scan device Id array
*           mngInterfaceType
*           size of cpss memory
*           serdesRefClock
*           ...
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __INCLUDE_MRVL_HAL_SYS_PRV_H
#define __INCLUDE_MRVL_HAL_SYS_PRV_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>
#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cpss/generic/port/cpssPortCtrl.h>

/* replace CPSS_MAX_PORTS_NUM_CNS to save memory */
#define MAX_PORT 1024


CPSS_PP_INTERFACE_CHANNEL_ENT cpssHalSys_param_mngInterfaceType(
    XP_DEV_TYPE_T xpDevType);
GT_U32  cpssHalSys_param_default_mem_init_size(void);
GT_U32 cpssHalSys_param_rxDescNum(XP_DEV_TYPE_T xpDevType);
GT_U32 cpssHalSys_param_txDescNum(XP_DEV_TYPE_T xpDevType);
GT_U32 cpssHalSys_param_rxBuffSize(XP_DEV_TYPE_T xpDevType);
GT_U32 cpssHalSys_param_rxBuffNum(XP_DEV_TYPE_T xpDevType);
GT_U32 cpssHalSys_param_rxBufferPercentage_0(XP_DEV_TYPE_T xpDevType);
GT_U32 cpssHalSys_param_rxBufferPercentage_1(XP_DEV_TYPE_T xpDevType);
GT_U32 cpssHalSys_param_rxBufferPercentage_2(XP_DEV_TYPE_T xpDevType);
GT_U32 cpssHalSys_param_rxBufferPercentage_3(XP_DEV_TYPE_T xpDevType);
GT_U32 cpssHalSys_param_rxBufferPercentage_4(XP_DEV_TYPE_T xpDevType);
GT_U32 cpssHalSys_param_rxBufferPercentage_5(XP_DEV_TYPE_T xpDevType);
GT_U32 cpssHalSys_param_rxBufferPercentage_6(XP_DEV_TYPE_T xpDevType);
GT_U32 cpssHalSys_param_rxBufferPercentage_7(XP_DEV_TYPE_T xpDevType);
GT_U32 cpssHalSys_param_auqDescNum(XP_DEV_TYPE_T xpDevType);
GT_BOOL cpssHalSys_param_fuqUseSeparate(XP_DEV_TYPE_T xpDevType);
GT_U32 cpssHalSys_param_fuqDescBlockSize(XP_DEV_TYPE_T xpDevType);
CPSS_DXCH_CFG_ROUTING_MODE_ENT cpssHalSys_param_ppRoutingMode(
    XP_DEV_TYPE_T xpDevType);
GT_U32 cpssHalSys_param_maxNumOfPbrEntries(XP_DEV_TYPE_T xpDevType);
CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT cpssHalSys_param_trunkMembersMode(
    XP_DEV_TYPE_T xpDevType);
GT_U32 cpssHalSys_param_mru(XP_DEV_TYPE_T xpDevType);
GT_U32 cpssHalSys_param_maxNumOfQueues(XP_DEV_TYPE_T xpDevType);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__INCLUDE_MRVL_HAL_SYS_PRV_H*/
