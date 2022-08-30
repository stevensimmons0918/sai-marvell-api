/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*  mvHwsFalconPortModeElements.c
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/generic/labservices/port/gop/silicon/falcon/mvHwsFalconPortIf.h>


/************************* definition *****************************************************/
#define FALCON_X2_SUP_MODES     hwsFalconPort0SupModes,hwsFalconPort1SupModes
#define FALCON_X4_SUP_MODES     FALCON_X2_SUP_MODES,FALCON_X2_SUP_MODES
#define FALCON_X8_SUP_MODES     FALCON_X4_SUP_MODES,hwsFalconPort4SupModes,hwsFalconPort1SupModes,FALCON_X2_SUP_MODES
#define FALCON_X16_SUP_MODES    FALCON_X8_SUP_MODES,FALCON_X8_SUP_MODES
#define FALCON_X32_SUP_MODES    FALCON_X16_SUP_MODES,FALCON_X16_SUP_MODES
#define FALCON_X64_SUP_MODES    FALCON_X32_SUP_MODES,FALCON_X32_SUP_MODES
#define FALCON_X128_SUP_MODES   FALCON_X64_SUP_MODES,FALCON_X64_SUP_MODES
#define FALCON_X256_SUP_MODES   FALCON_X128_SUP_MODES,FALCON_X128_SUP_MODES

/************************* Globals ********************************************************/
/* Last line of each SupModes array MUST be NON_SUP_MODE */
const MV_HWS_PORT_INIT_PARAMS hwsFalconPort0SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type         PCS Num  FEC             Speed           FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { SGMII,         MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,  0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,  0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,  0,   FEC_OFF,        _3_125G  ,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _5GBaseR,      MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _10GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_C, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR,   MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_C, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_SR,   MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_S, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_S, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _40GBase_KR4,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _40GBase_CR4,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    {_40GBase_SR_LR4,MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _40GBase_KR2,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},/*FEC_OFF? FC_FEC?*/
    { _42GBase_KR4,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _10_9375G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _48GBaseR4,    MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _12_1875G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    /*{ _48GBase_SR4,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _12_1875G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},*/
    { _50GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_100,        0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_CR,   MTI_MAC_100,   0,     MTI_PCS_100,        0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_SR_LR,MTI_MAC_100,   0,     MTI_PCS_100,        0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_KR2,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR2_C,MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2_C,MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_SR2,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _53GBase_KR2,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   RS_FEC,         _27_34375G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _100GBase_KR2, MTI_MAC_100,   0,     MTI_PCS_100,        0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_CR2, MTI_MAC_100,   0,     MTI_PCS_100,        0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_SR_LR2,MTI_MAC_100, 0,     MTI_PCS_100,        0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _102GBase_KR2, MTI_MAC_100,   0,     MTI_PCS_100,        0,   RS_FEC_544_514, _27_1875_PAM4,   NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_KR4, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _100GBase_CR4, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _100GBase_SR4, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _106GBase_KR4, MTI_MAC_100,   0,     MTI_PCS_100,        0,   RS_FEC,         _27_34375G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _200GBase_KR4, MTI_MAC_400,   0,     MTI_PCS_400,        0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_CR4, MTI_MAC_400,   0,     MTI_PCS_400,        0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_SR_LR4, MTI_MAC_400,0,     MTI_PCS_400,        0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_KR8, MTI_MAC_400,   0,     MTI_PCS_400,        0,   RS_FEC_544_514, _26_5625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _40BIT_ON, RS_FEC_544_514},
    { _200GBase_CR8, MTI_MAC_400,   0,     MTI_PCS_400,        0,   RS_FEC_544_514, _26_5625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _40BIT_ON, RS_FEC_544_514},
    { _200GBase_SR_LR8,MTI_MAC_400, 0,     MTI_PCS_400,        0,   RS_FEC_544_514, _26_5625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _40BIT_ON, RS_FEC_544_514},
    { _400GBase_KR8, MTI_MAC_400,   0,     MTI_PCS_400,        0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _400GBase_CR8, MTI_MAC_400,   0,     MTI_PCS_400,        0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _400GBase_SR_LR8,MTI_MAC_400, 0,     MTI_PCS_400,        0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
#if 0
    { _424GBase_KR8, MTI_MAC_400,   0,     MTI_PCS_400,        0,   RS_FEC_544_514, _28_125G_PAM4, NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
#endif
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,        NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};
const MV_HWS_PORT_INIT_PARAMS hwsFalconPort1SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type         PCS Num  FEC             Speed           FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { SGMII,         MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,  0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,  0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,  0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _5GBaseR,      MTI_MAC_100,   0,     MTI_PCS_50,         0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _10GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_50,         0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_MAC_100,   0,     MTI_PCS_50,         0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_50,         0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_C, MTI_MAC_100,   0,     MTI_PCS_50,         0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR,   MTI_MAC_100,   0,     MTI_PCS_50,         0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_C, MTI_MAC_100,   0,     MTI_PCS_50,         0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_SR,   MTI_MAC_100,   0,     MTI_PCS_50,         0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_S, MTI_MAC_100,   0,     MTI_PCS_50,         0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_S, MTI_MAC_100,   0,     MTI_PCS_50,         0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_50,         0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_CR,   MTI_MAC_100,   0,     MTI_PCS_50,         0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_SR_LR,MTI_MAC_100,   0,     MTI_PCS_50,         0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,        NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};
const MV_HWS_PORT_INIT_PARAMS hwsFalconPort4SupModes[] = {
     /* port mode,    MAC Type    Mac Num   PCS Type        PCS Num  FEC             Speed           FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { SGMII,         MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,  0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,  0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,  0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _5GBaseR,      MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _10GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_C, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR,   MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_C, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_SR,   MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_S, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_S, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _40GBase_KR4,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _40GBase_CR4,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _40GBase_SR_LR4,MTI_MAC_100,  0,     MTI_PCS_100,        0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _40GBase_KR2,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},/*FEC_OFF? FC_FEC?*/
    { _42GBase_KR4,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _10_9375G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _48GBaseR4,    MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _12_1875G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    /*{ _48GBase_SR4,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _12_1875G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},*/
    { _50GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_100,        0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_CR,   MTI_MAC_100,   0,     MTI_PCS_100,        0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_SR_LR,MTI_MAC_100,   0,     MTI_PCS_100,        0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_KR2,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR2_C,MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _53GBase_KR2,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   RS_FEC,         _27_34375G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _50GBase_CR2_C,MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_SR2,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _100GBase_KR2, MTI_MAC_100,   0,     MTI_PCS_100,        0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_CR2, MTI_MAC_100,   0,     MTI_PCS_100,        0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _102GBase_KR2, MTI_MAC_100,   0,     MTI_PCS_100,        0,   RS_FEC_544_514, _27_1875_PAM4,   NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_KR4, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _106GBase_KR4, MTI_MAC_100,   0,     MTI_PCS_100,        0,   RS_FEC,         _27_34375G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _100GBase_CR4, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _100GBase_SR4, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _200GBase_KR4, MTI_MAC_400,   0,     MTI_PCS_200,        0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_CR4, MTI_MAC_400,   0,     MTI_PCS_200,        0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_SR_LR4,MTI_MAC_400, 0,     MTI_PCS_200,        0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_SR_LR2,MTI_MAC_100, 0,     MTI_PCS_100,        0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,        NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};


const MV_HWS_PORT_INIT_PARAMS hwsFalconCpu0PortSupModes[] = {
    /* port mode,    MAC Type      Mac Num   PCS Type       PCS Num  FEC             Speed        FirstLane  LanesList LanesNum  Media         10Bit     FecSupported */
    { _10GBase_KR,   MTI_CPU_MAC,   512,     MTI_CPU_PCS,    512,    FEC_OFF,        _10_3125G,   NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_CPU_MAC,   512,     MTI_CPU_PCS,    512,    FEC_OFF,        _10_3125G,   NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { NON_SUP_MODE,  MAC_NA,        NA_NUM,  PCS_NA,         NA_NUM, FEC_OFF,        SPEED_NA,    NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

/* Armstrong2 support - configurations with each serdes up to 25G */

const MV_HWS_PORT_INIT_PARAMS hwsFalconSpeedsUpTo25GigaLanesUpTo4SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type         PCS Num  FEC             Speed           FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { SGMII,         MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,  0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,  0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,  0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _5GBaseR,      MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _10GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_C, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR,   MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_C, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_SR,   MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_S, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_S, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _40GBase_KR4,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _40GBase_CR4,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    {_40GBase_SR_LR4,MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _40GBase_KR2,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},/*FEC_OFF? FC_FEC?*/
    { _42GBase_KR4,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _10_9375G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _48GBaseR4,    MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _12_1875G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    /*{ _48GBase_SR4,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _12_1875G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},*/
    { _50GBase_KR2,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_SR2,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR2_C,MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2_C,MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _53GBase_KR2,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   RS_FEC,         _27_34375G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _100GBase_KR4, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _106GBase_KR4, MTI_MAC_100,   0,     MTI_PCS_100,        0,   RS_FEC,         _27_34375G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _100GBase_CR4, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _100GBase_SR4, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,        NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

const MV_HWS_PORT_INIT_PARAMS hwsFalconSpeedsUpTo25GigaLanesUpTo2SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type         PCS Num  FEC             Speed           FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { SGMII,         MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,  0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,  0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,  0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _5GBaseR,      MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _10GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_C, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR,   MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_C, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_SR,   MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_S, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_S, MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _40GBase_KR2,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},/*FEC_OFF? FC_FEC?*/
    { _50GBase_KR2,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_SR2,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR2_C,MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2_C,MTI_MAC_100,   0,     MTI_PCS_100,        0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _53GBase_KR2,  MTI_MAC_100,   0,     MTI_PCS_100,        0,   RS_FEC,         _27_34375G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,        NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

const MV_HWS_PORT_INIT_PARAMS hwsFalconSpeedsUpTo25GigaLanesUpTo1SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type         PCS Num  FEC             Speed           FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { SGMII,         MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,  0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,  0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,  0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _5GBaseR,      MTI_MAC_100,   0,     MTI_PCS_50,         0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _10GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_50,         0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_MAC_100,   0,     MTI_PCS_50,         0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_50,         0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_C, MTI_MAC_100,   0,     MTI_PCS_50,         0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR,   MTI_MAC_100,   0,     MTI_PCS_50,         0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_C, MTI_MAC_100,   0,     MTI_PCS_50,         0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_SR,   MTI_MAC_100,   0,     MTI_PCS_50,         0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_S, MTI_MAC_100,   0,     MTI_PCS_50,         0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_S, MTI_MAC_100,   0,     MTI_PCS_50,         0,   FEC_OFF,        _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,        NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

const MV_HWS_PORT_INIT_PARAMS hwsFalconSpeedsUpTo10GigaLanesUpTo1SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type         PCS Num  FEC             Speed           FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { _10GBase_KR,   MTI_CPU_MAC,     0,     MTI_CPU_PCS,      0,    FEC_OFF,        _10_3125G,   NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,        NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

#define FALCON_25G_X4_SUP_MODES \
    hwsFalconSpeedsUpTo25GigaLanesUpTo4SupModes, \
    hwsFalconSpeedsUpTo25GigaLanesUpTo1SupModes, \
    hwsFalconSpeedsUpTo25GigaLanesUpTo2SupModes, \
    hwsFalconSpeedsUpTo25GigaLanesUpTo1SupModes
#define FALCON_25G_X8_SUP_MODES FALCON_25G_X4_SUP_MODES, FALCON_25G_X4_SUP_MODES
#define FALCON_25G_X16_SUP_MODES FALCON_25G_X8_SUP_MODES, FALCON_25G_X8_SUP_MODES
#define FALCON_25G_X32_SUP_MODES FALCON_25G_X16_SUP_MODES, FALCON_25G_X16_SUP_MODES
#define FALCON_25G_X64_SUP_MODES FALCON_25G_X32_SUP_MODES, FALCON_25G_X32_SUP_MODES
#define FALCON_25G_X128_SUP_MODES FALCON_25G_X64_SUP_MODES, FALCON_25G_X64_SUP_MODES
#define FALCON_25G_X256_SUP_MODES FALCON_25G_X128_SUP_MODES, FALCON_25G_X128_SUP_MODES

#define FALCON_NONE_X4_SUP_MODES NULL,NULL,NULL,NULL
#define FALCON_NONE_X8_SUP_MODES FALCON_NONE_X4_SUP_MODES, FALCON_NONE_X4_SUP_MODES
#define FALCON_NONE_X16_SUP_MODES FALCON_NONE_X8_SUP_MODES, FALCON_NONE_X8_SUP_MODES

#define FALCON_10G_X4_SUP_MODES \
    hwsFalconSpeedsUpTo10GigaLanesUpTo1SupModes, \
    hwsFalconSpeedsUpTo10GigaLanesUpTo1SupModes, \
    hwsFalconSpeedsUpTo10GigaLanesUpTo1SupModes, \
    hwsFalconSpeedsUpTo10GigaLanesUpTo1SupModes
#define FALCON_10G_X8_SUP_MODES FALCON_10G_X4_SUP_MODES, FALCON_10G_X4_SUP_MODES
#define FALCON_10G_X16_SUP_MODES FALCON_10G_X8_SUP_MODES, FALCON_10G_X8_SUP_MODES

/* map the right supported modes table for each port */
const MV_HWS_PORT_INIT_PARAMS **hwsPortsFalconParamsSupModesMap;
/* each tile hole 4 Ravens each with CPU port */
#define    FALCON_X4_CPU_MODES \
    hwsFalconCpu0PortSupModes, \
    hwsFalconCpu0PortSupModes, \
    hwsFalconCpu0PortSupModes, \
    hwsFalconCpu0PortSupModes

const MV_HWS_PORT_INIT_PARAMS *hwsPortsFalcon_3_2T_ParamsSupModesMap[] =
{
    FALCON_X64_SUP_MODES,
    FALCON_X4_CPU_MODES
};

const MV_HWS_PORT_INIT_PARAMS *hwsPortsFalcon_6_4T_ParamsSupModesMap[] =
{
    FALCON_X128_SUP_MODES,
    FALCON_X4_CPU_MODES,
    FALCON_X4_CPU_MODES
};

const MV_HWS_PORT_INIT_PARAMS *hwsPortsFalcon_128x25g_ParamsSupModesMap[] =
{
    FALCON_25G_X128_SUP_MODES,
    FALCON_10G_X8_SUP_MODES
};

const MV_HWS_PORT_INIT_PARAMS *hwsPortsFalcon_12_8T_ParamsSupModesMap[] =
{
    FALCON_X256_SUP_MODES,
    FALCON_10G_X16_SUP_MODES
};

const MV_HWS_PORT_INIT_PARAMS *hwsPortsFalcon_256x25g_ParamsSupModesMap[] =
{
    FALCON_25G_X256_SUP_MODES,
    FALCON_10G_X16_SUP_MODES
};

const MV_HWS_PORT_INIT_PARAMS *hwsPortsArmstrong2_80x25G_ParamsSupModesMap[] =
{
    /* 25G MAC 0..15 per Raven */
    /*Raven0*/ FALCON_25G_X16_SUP_MODES,
    /*Raven1*/ FALCON_NONE_X16_SUP_MODES,
    /*Raven2*/ FALCON_25G_X16_SUP_MODES,
    /*Raven3*/ FALCON_NONE_X16_SUP_MODES,
    /*Raven4*/ FALCON_NONE_X16_SUP_MODES,
    /*Raven5*/ FALCON_25G_X16_SUP_MODES,
    /*Raven6*/ FALCON_25G_X16_SUP_MODES,
    /*Raven7*/ FALCON_25G_X16_SUP_MODES,
    /* 10G MAC 16 per raven */
    FALCON_10G_X8_SUP_MODES
};

const MV_HWS_PORT_INIT_PARAMS *hwsPortsFalcon_80x25G_ParamsSupModesMap[] =
{
    /* 25G MAC 0..15 per Raven */
    /*Raven0*/ FALCON_25G_X16_SUP_MODES,
    /*Raven1*/ FALCON_NONE_X16_SUP_MODES,
    /*Raven2*/ FALCON_25G_X16_SUP_MODES,
    /*Raven3*/ FALCON_NONE_X16_SUP_MODES,
    /*Raven4*/ FALCON_25G_X16_SUP_MODES,
    /*Raven5*/ FALCON_25G_X16_SUP_MODES,
    /*Raven6*/ FALCON_NONE_X16_SUP_MODES,
    /*Raven7*/ FALCON_25G_X16_SUP_MODES,
    /* 10G MAC 16 per raven */
    FALCON_10G_X8_SUP_MODES
};

/************************* functions ******************************************************/

GT_STATUS hwsFalconPortElementsDbInit
(
    GT_U8 devNum
)
{

    if ((hwsFalconTypeGet(devNum) == HWS_FALCON_DEV_98CX8514_E) ||
        (hwsFalconTypeGet(devNum) == HWS_FALCON_DEV_98EX5614_E))
    {
            hwsPortsFalconParamsSupModesMap = hwsPortsFalcon_80x25G_ParamsSupModesMap;
            return GT_OK;
    }
    else if ((hwsFalconTypeGet(devNum) == HWS_ARMSTRONG2_DEV_98EX5610_E) ||
             (hwsFalconTypeGet(devNum) == HWS_ARMSTRONG2_DEV_98CX8512_E))
    {
            hwsPortsFalconParamsSupModesMap = hwsPortsArmstrong2_80x25G_ParamsSupModesMap;
            return GT_OK;
    }
    else if (hwsFalconTypeGet(devNum) == HWS_FALCON_DEV_98CX8522_E)
    {
            hwsPortsFalconParamsSupModesMap = hwsPortsFalcon_128x25g_ParamsSupModesMap;
            return GT_OK;
    }
    else if (hwsFalconTypeGet(devNum) == HWS_FALCON_DEV_98CX8542_E)
    {
            hwsPortsFalconParamsSupModesMap = hwsPortsFalcon_256x25g_ParamsSupModesMap;
            return GT_OK;
    }

    switch(hwsFalconNumOfRavens)
    {
        case HWS_FALCON_3_2_RAVEN_NUM_CNS:
            hwsPortsFalconParamsSupModesMap = hwsPortsFalcon_3_2T_ParamsSupModesMap;
            break;
        case HWS_FALCON_6_4_RAVEN_NUM_CNS:
            hwsPortsFalconParamsSupModesMap = hwsPortsFalcon_6_4T_ParamsSupModesMap;
            break;
        case HWS_FALCON_12_8_RAVEN_NUM_CNS:
            hwsPortsFalconParamsSupModesMap = hwsPortsFalcon_12_8T_ParamsSupModesMap;
            break;
        default:
            return GT_BAD_STATE;
    }

    return GT_OK;
}
