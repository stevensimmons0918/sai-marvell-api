/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/********************************************************************************
* .h
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
#ifndef __INCLUDE_MRVL_HAL_ASIC_PRV_H
#define __INCLUDE_MRVL_HAL_ASIC_PRV_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssTypes.h>
#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpssHalExt.h>
#include <cpssHalProfile.h>

/* per supported PP parameters */
typedef struct
{
    CPSS_PP_DEVICE_TYPE                  devType;
    GT_BOOL
    needDevMapping; /* to remap ports to 0..n */
    GT_BOOL                              needServiceCpu; /* support service cpu */
    CPSS_DXCH_PP_SERDES_REF_CLOCK_ENT    serdesRefClock;
    CPSS_DXCH_IMPLEMENT_WA_ENT           waList[CPSS_DXCH_IMPLEMENT_WA_LAST_E
                                                +1]; /* wa list for this PP */
} mrvSupportedDevinfo;

extern mrvSupportedDevinfo mrvSupportedDevData[];

extern const int mrvSupportedDevDataSize;

extern int gEnableCpssLog;
extern PROFILE_STC DB_CPSS_aldrinProfile[];/* CPSS DB single Aldrin */
extern PROFILE_STC aldrinProfile[];
extern PROFILE_STC aldrin2Profile[];
extern PROFILE_STC defaultProfile[];
extern PROFILE_STC ac3x_b2b_profile0[];
extern PROFILE_STC ac3x_b2b_profile1[];
extern PROFILE_STC ac3x_b2b_tg48mp_profile0[];
extern PROFILE_STC ac3x_b2b_tg48mp_profile1[];
extern PROFILE_STC ac5x_rd_board_profile[];
extern PROFILE_STC ac5x_48x1G6x10G_port_profile[];
extern PROFILE_STC falcon_3_2_profile[];
extern PROFILE_STC falcon_6_4_profile[];
extern PROFILE_STC falcon_default_profile[];
extern PROFILE_STC falcon_belly2belly_profile[];
extern PROFILE_STC falcon_32_port_profile[];
extern PROFILE_STC falcon_32x100_8x400_port_profile[];
extern PROFILE_STC falcon_32x25_port_profile[];
extern PROFILE_STC falcon_32x400_port_profile[];
extern PROFILE_STC falcon_32x25_6_4_port_profile[];
extern PROFILE_STC falcon_64x25_port_profile[];
extern PROFILE_STC falcon_64x100_port_profile[];
extern PROFILE_STC falcon_64x100_R4_port_profile[];
extern PROFILE_STC falcon_64x100_ixia_port_profile[];
extern PROFILE_STC falcon_32x100_R4_6_4_port_profile_ixia[];
extern PROFILE_STC falcon_256_port_profile[];
extern PROFILE_STC falcon_16x400_port_profile[];
extern PROFILE_STC falcon_16x25_port_profile[];
extern PROFILE_STC falcon_32x25_8_ixia[];
extern PROFILE_STC falcon_24x25_8x100_12_8_port_profile[];
extern PROFILE_STC falcon_24x25_8x200_port_profile[];
extern PROFILE_STC falcon_24x100_4x400_port_profile[];
extern PROFILE_STC falcon_48x100_8x400_port_profile[];
extern PROFILE_STC falcon_32x100_16x400_port_profile[];
extern PROFILE_STC falcon_16x100_8x400_port_profile[];
extern PROFILE_STC falcon_24x25_8x100_6_4_port_profile[];
extern PROFILE_STC falcon_24x25_4x200_6_4_port_profile[];
extern PROFILE_STC falcon_96x100_8x400_port_profile[];
extern PROFILE_STC falcon_128x50_port_profile[];
extern PROFILE_STC falcon_128x10_port_profile[];
extern PROFILE_STC falcon_128x25_port_profile[];
extern PROFILE_STC falcon_128x100_port_profile[];
extern PROFILE_STC falcon_16x25_4_ixia[];
extern PROFILE_STC falcon_48x10_8x100_port_profile[];
extern PROFILE_STC falcon_48x25_8x100_port_profile[];
extern PROFILE_STC falcon_64x25_64x10_port_profile[];
extern PROFILE_STC falcon_32x25_8x100_port_profile[];
extern PROFILE_STC falcon_2T_80x25_port_profile[];
extern PROFILE_STC falcon_2T_48x25_8x100_port_profile[];
extern PROFILE_STC falcon_2T_48x10_8x100_port_profile[];
extern PROFILE_STC cygnus[];
extern PROFILE_STC aldrin2_xl_port_profile[];
extern PROFILE_STC aldrin2_xl_fujitsu_large_profile[];
extern PROFILE_STC ac3x_fujitsu_small_profile[];
extern PROFILE_STC ac3x_fujitsu_small_25_profile[];
extern PROFILE_STC ac3x_fujitsu_small_ila_profile[];

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__INCLUDE_MRVL_HAL_ASIC_PRV_H*/
