/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*  mvHwsAc5pPortModeElements.c
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
#include <cpss/generic/labservices/port/gop/silicon/ac5p/mvHwsAc5pPortIf.h>

/************************* definition *****************************************************/
/************************* Globals ********************************************************/



/* Last line of each SupModes array MUST be NON_SUP_MODE */
const MV_HWS_PORT_INIT_PARAMS hwsHawkPort0SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type         PCS Num  FEC             Speed            FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { SGMII,         MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,   0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,   0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { SGMII2_5,      MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,   0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,   0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _5GBaseR,      MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR,   MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_C, MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_C, MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_S, MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_CR_S, MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_SR,   MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _40GBase_KR4,  MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _40GBase_CR4,  MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _40GBase_SR_LR4,MTI_MAC_100_BR,0,    MTI_PCS_100,         0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _40GBase_KR2,  MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR2,  MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2,  MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR2_C,MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2_C,MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_SR2,  MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_CR,   MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_SR_LR,MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_KR4, MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _100GBase_CR4, MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _100GBase_SR4, MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _107GBase_KR4, MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC,         _27_5G,          NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, RS_FEC},
    { _100GBase_KR2, MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_CR2, MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_SR_LR2,MTI_MAC_100_BR,0,   MTI_PCS_100,         0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _106GBase_KR2, MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC_544_514, _28_125G_PAM4,   NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_KR4, MTI_MAC_400,   0,     MTI_PCS_400,         0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_CR4, MTI_MAC_400,   0,     MTI_PCS_400,         0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_SR_LR4,MTI_MAC_400, 0,     MTI_PCS_400,         0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _212GBase_KR4, MTI_MAC_400,   0,     MTI_PCS_400,         0,   RS_FEC_544_514, _28_125G_PAM4,   NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _400GBase_KR8, MTI_MAC_400,   0,     MTI_PCS_400,         0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _400GBase_CR8, MTI_MAC_400,   0,     MTI_PCS_400,         0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _400GBase_SR_LR8,MTI_MAC_400, 0,     MTI_PCS_400,         0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _424GBase_KR8, MTI_MAC_400,   0,     MTI_PCS_400,         0,   RS_FEC_544_514, _28_125G_PAM4,   NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_KR8, MTI_MAC_400,   0,     MTI_PCS_400,         0,   RS_FEC_544_514, _26_5625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _40BIT_ON, RS_FEC_544_514},
    { _200GBase_CR8, MTI_MAC_400,   0,     MTI_PCS_400,         0,   RS_FEC_544_514, _26_5625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _40BIT_ON, RS_FEC_544_514},
    { _200GBase_SR_LR8, MTI_MAC_400,0,     MTI_PCS_400,         0,   RS_FEC_544_514, _26_5625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _40BIT_ON, RS_FEC_544_514},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,         NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

const MV_HWS_PORT_INIT_PARAMS hwsHawkPort1SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type           PCS Num  FEC             Speed          FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { SGMII,         MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { SGMII2_5,      MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _5GBaseR,      MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR,   MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_C, MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_C, MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_S, MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_CR_S, MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_SR,   MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_CR,   MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_SR_LR,MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,           NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};
const MV_HWS_PORT_INIT_PARAMS cluster0_hwsHawkPort2SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type           PCS Num  FEC             Speed          FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { SGMII,         MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { SGMII2_5,      MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _5GBaseR,      MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_C, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_C, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_S, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_CR_S, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_SR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _40GBase_KR2,  MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR2,  MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2,  MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR2_C,MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2_C,MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_SR2,  MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_CR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_SR_LR,MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_KR2, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_CR2, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_SR_LR2,MTI_MAC_100_BR,0,   MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _106GBase_KR2, MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC_544_514, _28_125G_PAM4,   NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},

    { QSGMII,        MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _5G,             NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _2_5G_SXGMII,  MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _2_578125,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _5G_SXGMII,    MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _10G_SXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _5G_DXGMII,    MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _10G_DXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _20G_DXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _5G_QUSGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _5G,             NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _10G_QXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _20G_QXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _10G_OUSGMII,  MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _10G,            NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _20G_OXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},

    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,           NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

const MV_HWS_PORT_INIT_PARAMS cluster2_hwsHawkPort2SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type           PCS Num  FEC             Speed          FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { SGMII,         MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { SGMII2_5,      MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _5GBaseR,      MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_C, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_C, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_S, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_CR_S, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_SR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _40GBase_KR2,  MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR2,  MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2,  MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR2_C,MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2_C,MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_SR2,  MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_CR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_SR_LR,MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_KR2, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_CR2, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_SR_LR2,MTI_MAC_100_BR,0,   MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _106GBase_KR2, MTI_MAC_100_BR,0,     MTI_PCS_100,         0,   RS_FEC_544_514, _28_125G_PAM4,   NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,           NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

const MV_HWS_PORT_INIT_PARAMS hwsHawkPort3SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type           PCS Num  FEC             Speed           FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { QSGMII,        MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _5G,             NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _5G_QUSGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _5G,             NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _10G_QXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _20G_QXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _10G_OUSGMII,  MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _10G,            NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _20G_OXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},

    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,           NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};


const MV_HWS_PORT_INIT_PARAMS hwsHawkPort4SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type           PCS Num  FEC             Speed           FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { QSGMII,        MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _5G,             NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _5G_DXGMII,    MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _10G_DXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _20G_DXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _5G_QUSGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _5G,             NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _10G_QXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _20G_QXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _10G_OUSGMII,  MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _10G,            NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _20G_OXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},

    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,           NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};


const MV_HWS_PORT_INIT_PARAMS cluster0_hwsHawkPort6SupModes[] = {
     /* port mode,    MAC Type    Mac Num   PCS Type          PCS Num  FEC             Speed           FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { SGMII,         MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { SGMII2_5,      MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _5GBaseR,      MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR,   MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_C, MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_C, MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_S, MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_CR_S, MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_SR,   MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_CR,   MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_SR_LR,MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},

    { QSGMII,        MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _5G,             NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _2_5G_SXGMII,  MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _2_578125,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _5G_SXGMII,    MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _10G_SXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _5G_DXGMII,    MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _10G_DXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _20G_DXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _5G_QUSGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _5G,             NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _10G_QXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _20G_QXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _10G_OUSGMII,  MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _10G,            NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _20G_OXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},

    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,           NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

const MV_HWS_PORT_INIT_PARAMS cluster2_hwsHawkPort6SupModes[] = {
     /* port mode,    MAC Type    Mac Num   PCS Type          PCS Num  FEC             Speed           FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { SGMII,         MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { SGMII2_5,      MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _5GBaseR,      MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR,   MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_C, MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_C, MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_S, MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_CR_S, MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_SR,   MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_CR,   MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_SR_LR,MTI_MAC_100_BR,0,     MTI_PCS_50,            0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,           NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

const MV_HWS_PORT_INIT_PARAMS cluster0_hwsHawkPort10SupModes[] = {
     /* port mode,    MAC Type    Mac Num   PCS Type        PCS Num  FEC             Speed           FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { SGMII,         MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { SGMII2_5,      MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _5GBaseR,      MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_C, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_C, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_S, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_CR_S, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_SR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _40GBase_KR4,  MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _40GBase_CR4,  MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _40GBase_SR_LR4,MTI_MAC_100_BR,0,    MTI_PCS_100,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _40GBase_KR2,  MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR2,  MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2,  MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR2_C,MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2_C,MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_SR2,  MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_CR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_SR_LR,MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_KR4, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _100GBase_CR4, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _100GBase_SR4, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _107GBase_KR4, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _27_5G,          NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, RS_FEC},
    { _100GBase_KR2, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_CR2, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_SR_LR2,MTI_MAC_100_BR,0,   MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _106GBase_KR2, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _28_125G_PAM4,   NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_KR4, MTI_MAC_400,   0,     MTI_PCS_400,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_CR4, MTI_MAC_400,   0,     MTI_PCS_400,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_SR_LR4,MTI_MAC_400, 0,     MTI_PCS_400,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _212GBase_KR4, MTI_MAC_400,   0,     MTI_PCS_400,           0,   RS_FEC_544_514, _28_125G_PAM4,   NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},

    { QSGMII,        MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _5G,             NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _2_5G_SXGMII,  MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _2_578125,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _5G_SXGMII,    MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _10G_SXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _5G_DXGMII,    MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _10G_DXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _20G_DXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _5G_QUSGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _5G,             NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _10G_QXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _20G_QXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _10G_OUSGMII,  MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _10G,            NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _20G_OXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},

    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,           NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

const MV_HWS_PORT_INIT_PARAMS cluster2_hwsHawkPort10SupModes[] = {
     /* port mode,    MAC Type    Mac Num   PCS Type        PCS Num  FEC             Speed           FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { SGMII,         MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { SGMII2_5,      MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_MAC_100_BR,0,     MTI_PCS_LOW_SPEED,     0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _5GBaseR,      MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_C, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_C, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_S, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_CR_S, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_SR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _40GBase_KR4,  MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _40GBase_CR4,  MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _40GBase_SR_LR4,MTI_MAC_100_BR,0,    MTI_PCS_100,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _40GBase_KR2,  MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR2,  MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2,  MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR2_C,MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2_C,MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_SR2,  MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_CR,   MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _50GBase_SR_LR,MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_KR4, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _100GBase_CR4, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _100GBase_SR4, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _107GBase_KR4, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC,         _27_5G,          NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, RS_FEC},
    { _100GBase_KR2, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_CR2, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _100GBase_SR_LR2,MTI_MAC_100_BR,0,   MTI_PCS_100,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _106GBase_KR2, MTI_MAC_100_BR,0,     MTI_PCS_100,           0,   RS_FEC_544_514, _28_125G_PAM4,   NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_KR4, MTI_MAC_400,   0,     MTI_PCS_400,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_CR4, MTI_MAC_400,   0,     MTI_PCS_400,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _200GBase_SR_LR4,MTI_MAC_400, 0,     MTI_PCS_400,           0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { _212GBase_KR4, MTI_MAC_400,   0,     MTI_PCS_400,           0,   RS_FEC_544_514, _28_125G_PAM4,   NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _80BIT_ON, RS_FEC_544_514},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,           NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

const MV_HWS_PORT_INIT_PARAMS hwsHawkCpuPortSupModes[] = {
    /* port mode,    MAC Type      Mac Num   PCS Type       PCS Num    FEC             Speed            FirstLane  LanesList       LanesNum  Media          10Bit     FecSupported */
    { SGMII,         MTI_CPU_MAC,   105,     MTI_CPU_SGPCS,  105,      FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_CPU_MAC,   105,     MTI_CPU_SGPCS,  105,      FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { SGMII2_5,      MTI_CPU_MAC,   105,     MTI_CPU_SGPCS,  105,      FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_CPU_MAC,   105,     MTI_CPU_SGPCS,  105,      FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _10GBase_KR,   MTI_CPU_MAC,   105,     MTI_CPU_PCS,    105,      FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_CPU_MAC,   105,     MTI_CPU_PCS,    105,      FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_CPU_MAC,   105,     MTI_CPU_PCS,    105,      RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR,   MTI_CPU_MAC,   105,     MTI_CPU_PCS,    105,      RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_C, MTI_CPU_MAC,   105,     MTI_CPU_PCS,    105,      RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_C, MTI_CPU_MAC,   105,     MTI_CPU_PCS,    105,      RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_S, MTI_CPU_MAC,   105,     MTI_CPU_PCS,    105,      FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_CR_S, MTI_CPU_MAC,   105,     MTI_CPU_PCS,    105,      FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_SR,   MTI_CPU_MAC,   105,     MTI_CPU_PCS,    105,      RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { NON_SUP_MODE,  MAC_NA,        NA_NUM,  PCS_NA,         NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

/* map the right supported modes table for each port */
const MV_HWS_PORT_INIT_PARAMS **hwsPortsHawkParamsSupModesMap;

/* the 26 MACs in the GOP Claster 0 and 1 of the Hawk */
#define HAWK_P0_P1_SUP_MODES             hwsHawkPort0SupModes/* 0*/, hwsHawkPort1SupModes/*1*/
#define CLUSTER0_HAWK_P2_P5_SUP_MODES    cluster0_hwsHawkPort2SupModes/* 2*/, hwsHawkPort3SupModes/* 3*/,hwsHawkPort4SupModes/* 4*/,hwsHawkPort3SupModes/* 5*/
#define CLUSTER0_HAWK_P6_P9_SUP_MODES    cluster0_hwsHawkPort6SupModes/* 6*/, hwsHawkPort3SupModes/* 7*/,hwsHawkPort4SupModes/* 8*/,hwsHawkPort3SupModes/* 9*/
#define CLUSTER0_HAWK_P10_P13_SUP_MODES  cluster0_hwsHawkPort10SupModes/*10*/,hwsHawkPort3SupModes/*11*/,hwsHawkPort4SupModes/*12*/,hwsHawkPort3SupModes/*13*/
#define CLUSTER0_HAWK_P14_P17_SUP_MODES  cluster0_hwsHawkPort6SupModes /*14*/,hwsHawkPort3SupModes/*15*/,hwsHawkPort4SupModes/*16*/,hwsHawkPort3SupModes/*17*/
#define CLUSTER0_HAWK_P18_P21_SUP_MODES  cluster0_hwsHawkPort2SupModes /*18*/,hwsHawkPort3SupModes/*19*/,hwsHawkPort4SupModes/*20*/,hwsHawkPort3SupModes/*21*/
#define CLUSTER0_HAWK_P22_P25_SUP_MODES  cluster0_hwsHawkPort6SupModes /*22*/,hwsHawkPort3SupModes/*23*/,hwsHawkPort4SupModes/*24*/,hwsHawkPort3SupModes/*25*/

/* the 26 MACs in the GOP Claster 2 and 3 of the Hawk */
#define CLUSTER2_HAWK_P2_P5_SUP_MODES    cluster2_hwsHawkPort2SupModes/* 2*/, NULL/* 3*/,NULL/* 4*/,NULL/* 5*/
#define CLUSTER2_HAWK_P6_P9_SUP_MODES    cluster2_hwsHawkPort6SupModes/* 6*/, NULL/* 7*/,NULL/* 8*/,NULL/* 9*/
#define CLUSTER2_HAWK_P10_P13_SUP_MODES  cluster2_hwsHawkPort10SupModes/*10*/,NULL/*11*/,NULL/*12*/,NULL/*13*/
#define CLUSTER2_HAWK_P14_P17_SUP_MODES  cluster2_hwsHawkPort6SupModes /*14*/,NULL/*15*/,NULL/*16*/,NULL/*17*/
#define CLUSTER2_HAWK_P18_P21_SUP_MODES  cluster2_hwsHawkPort2SupModes /*18*/,NULL/*19*/,NULL/*20*/,NULL/*21*/
#define CLUSTER2_HAWK_P22_P25_SUP_MODES  cluster2_hwsHawkPort6SupModes /*22*/,NULL/*23*/,NULL/*24*/,NULL/*25*/

#define CLUSTER0_HAWK_SINGLE_GOP          \
    HAWK_P0_P1_SUP_MODES,                 \
    CLUSTER0_HAWK_P2_P5_SUP_MODES,        \
    CLUSTER0_HAWK_P6_P9_SUP_MODES,        \
    CLUSTER0_HAWK_P10_P13_SUP_MODES,      \
    CLUSTER0_HAWK_P14_P17_SUP_MODES,      \
    CLUSTER0_HAWK_P18_P21_SUP_MODES,      \
    CLUSTER0_HAWK_P22_P25_SUP_MODES

#define CLUSTER2_HAWK_SINGLE_GOP          \
    HAWK_P0_P1_SUP_MODES,                 \
    CLUSTER2_HAWK_P2_P5_SUP_MODES,        \
    CLUSTER2_HAWK_P6_P9_SUP_MODES,        \
    CLUSTER2_HAWK_P10_P13_SUP_MODES,      \
    CLUSTER2_HAWK_P14_P17_SUP_MODES,      \
    CLUSTER2_HAWK_P18_P21_SUP_MODES,      \
    CLUSTER2_HAWK_P22_P25_SUP_MODES

const MV_HWS_PORT_INIT_PARAMS *hwsPortsHawk_ParamsSupModesMap[] =
{
    /* Hawk hold 4 GOPs of 26 MACs */
    CLUSTER0_HAWK_SINGLE_GOP,/*GOP0*/
    CLUSTER0_HAWK_SINGLE_GOP,/*GOP1*/
    CLUSTER2_HAWK_SINGLE_GOP,/*GOP2*/
    CLUSTER2_HAWK_SINGLE_GOP,/*GOP3*/
    NULL,
    hwsHawkCpuPortSupModes
};


/************************* functions ******************************************************/

GT_VOID hwsHawkPortElementsDbInit
(
    GT_VOID
)
{

    hwsPortsHawkParamsSupModesMap = hwsPortsHawk_ParamsSupModesMap;

}

GT_STATUS hwsHawkPortParamsGetLanes
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_INIT_PARAMS*    portParams
)
{
    GT_U16    i = 0, j = 0;
    MV_HWS_HAWK_CONVERT_STC         convertIdx;
    GT_U32    portOffset, firstSdInGop;
    MV_HWS_PORT_SERDES_TO_MAC_MUX   hwsMacToSerdesMuxStc;

    /* avoid warnings */
    portGroup = portGroup;

    if (portParams->portStandard == NON_SUP_MODE)
    {
        return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(mvHwsGlobalMacToLocalIndexConvert(devNum, phyPortNum, portParams->portStandard, &convertIdx));

    if(mvHwsMtipIsReducedPort(devNum,phyPortNum))
    {
        portParams->activeLanesList[0] = 32;
        portParams->firstLaneNum = portParams->activeLanesList[0];
        return GT_OK;
    }

    if(mvHwsUsxModeCheck(devNum, phyPortNum, portParams->portStandard) == GT_FALSE)
    {
        if(GT_OK == mvHwsPortLaneMacToSerdesMuxGet(devNum, (GT_U8)portGroup, phyPortNum, &hwsMacToSerdesMuxStc))
        {
            portOffset = convertIdx.ciderIndexInUnit;
            firstSdInGop = convertIdx.ciderUnit * 8;
        }
        else
        {
            portParams->firstLaneNum = (GT_U16)(convertIdx.ciderUnit * 8 + convertIdx.ciderIndexInUnit);
            portOffset = 0;
            firstSdInGop = portParams->firstLaneNum;
        }

        for (i = 0 ; i < portParams->numOfActLanes ; i++)
        {
            j = portOffset + i; /* start from the right offset in the ports group */
            if(hwsMacToSerdesMuxStc.enableSerdesMuxing)
            {
                portParams->activeLanesList[i] = (GT_U16)firstSdInGop + (GT_U16)(hwsMacToSerdesMuxStc.serdesLanes[j]);
            }
            else
            {
                portParams->activeLanesList[i] = (GT_U16)firstSdInGop + j;
            }
        }
        portParams->firstLaneNum = portParams->activeLanesList[0];

    }
    else
    {
        portParams->firstLaneNum  = (GT_U16)((convertIdx.ciderIndexInUnit / 8)*2 + 2);

        if(HWS_USX_O_MODE_CHECK(portParams->portStandard))
        {
            if(convertIdx.ciderIndexInUnit % 8 != 0)
            {
                /*curPortParams->numOfActLanes = 0;*/
            }
        }
        else
        {
            if ((convertIdx.ciderIndexInUnit % 8) >= 4)
            {
                portParams->firstLaneNum++;
            }
            if(convertIdx.ciderIndexInUnit % 4 != 0)
            {
                /*curPortParams->numOfActLanes = 0;*/
            }
        }
        if(GT_OK == mvHwsPortLaneMacToSerdesMuxGet(devNum, (GT_U8)portGroup, phyPortNum, &hwsMacToSerdesMuxStc))
        {
            portParams->firstLaneNum = hwsMacToSerdesMuxStc.serdesLanes[portParams->firstLaneNum];
        }
        portParams->firstLaneNum  += (GT_U16)(convertIdx.ciderUnit * 8);
        portParams->activeLanesList[0] = portParams->firstLaneNum;
    }

    return GT_OK;
}

GT_STATUS hwsHawkPortParamsSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
)
{
#ifndef MV_HWS_REDUCED_BUILD
    MV_HWS_DEVICE_PORTS_ELEMENTS    curDevicePorts = hwsDevicesPortsElementsArray[devNum];
    MV_HWS_PORT_INIT_PARAMS         *curPortParams;
    const MV_HWS_PORT_INIT_PARAMS   *newPortParams;
    GT_U32                          portModeIndex;

    if (phyPortNum > hwsDeviceSpecInfo[devNum].portsNum)
    {
        return GT_OUT_OF_RANGE;
    }

    curPortParams = &curDevicePorts[phyPortNum].curPortParams;
    if (NON_SUP_MODE == portMode) /* Reset port */
    {
        /* initialize entry with default port params */
        hwsOsMemCopyFuncPtr(curPortParams, &hwsSingleEntryInitParams, sizeof(MV_HWS_PORT_INIT_PARAMS));
        curPortParams->portMacNumber = (GT_U16)phyPortNum;
        curPortParams->portPcsNumber = (GT_U16)phyPortNum;
        curPortParams->firstLaneNum  = (GT_U16)phyPortNum;
        return GT_OK;
    }

    newPortParams = hwsPortsElementsFindMode(hwsDevicePortsElementsSupModesCatalogGet(devNum,phyPortNum),portMode,&portModeIndex);
    if ( (NULL == newPortParams) || (newPortParams->numOfActLanes == 0))
    {
        return GT_NOT_SUPPORTED;
    }
    /* Copy the right port mode line from the port's specific supported modes list*/
    hwsOsMemCopyFuncPtr(curPortParams, newPortParams, sizeof(MV_HWS_PORT_INIT_PARAMS));
    curPortParams->portMacNumber = (GT_U16)phyPortNum;
    curPortParams->portPcsNumber = (GT_U16)phyPortNum;
    curPortParams->portFecMode   = curDevicePorts[phyPortNum].perModeFecList[portModeIndex];

    /* build active lane list */
    CHECK_STATUS(hwsPortParamsGetLanes(devNum,portGroup,phyPortNum,curPortParams));
    return GT_OK;

#else

    return GT_OK;

#endif
}


