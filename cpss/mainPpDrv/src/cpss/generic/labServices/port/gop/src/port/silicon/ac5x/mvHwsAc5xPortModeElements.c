/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*  mvHwsAc5xPortModeElements.c
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
#include <cpss/generic/labservices/port/gop/silicon/ac5x/mvHwsAc5xPortIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>

/************************* definition *****************************************************/
/************************* Globals ********************************************************/



/* Last line of each SupModes array MUST be NON_SUP_MODE */
const MV_HWS_PORT_INIT_PARAMS hwsPhoenixPort0SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type           PCS Num  FEC             Speed          FirstLane  LanesList       LanesNum    Media          10Bit     FecSupported */
    { _1000Base_X,   MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { SGMII,         MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { QSGMII,        MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _5G,             NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { SGMII2_5,      MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
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
    { _10GBase_KR,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_SR,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _5GBaseR,      MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_CR_C, MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_C, MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_S, MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR_S, MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,           NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

/* Last line of each SupModes array MUST be NON_SUP_MODE */
const MV_HWS_PORT_INIT_PARAMS hwsPhoenixPort1SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type           PCS Num  FEC             Speed          FirstLane  LanesList       LanesNum    Media          10Bit     FecSupported */
    { QSGMII,        MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _5G,             NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
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

/* Last line of each SupModes array MUST be NON_SUP_MODE */
const MV_HWS_PORT_INIT_PARAMS hwsPhoenixPort2SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type           PCS Num  FEC             Speed          FirstLane  LanesList       LanesNum    Media          10Bit     FecSupported */
    { QSGMII,        MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _5G,             NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _5G_QUSGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _5G,             NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _10G_QXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _20G_QXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _10G_OUSGMII,  MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _10G,            NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _20G_OXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,           NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

/* Last line of each SupModes array MUST be NON_SUP_MODE */
const MV_HWS_PORT_INIT_PARAMS hwsPhoenixPort4SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type           PCS Num  FEC             Speed          FirstLane  LanesList       LanesNum    Media          10Bit     FecSupported */
    { _10G_OUSGMII,  MTI_USX_MAC,   0,     MTI_USX_PCS_LOW_SPEED, 0,   FEC_OFF,        _10G,            NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _20G_OXGMII,   MTI_USX_MAC,   0,     MTI_USX_PCS,           0,   RS_FEC,         _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,           NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

/* Last line of each SupModes array MUST be NON_SUP_MODE */
const MV_HWS_PORT_INIT_PARAMS hwsPhoenixPort48SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type         PCS Num  FEC             Speed            FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { SGMII,         MTI_CPU_MAC,   0,     MTI_CPU_SGPCS,       0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_CPU_MAC,   0,     MTI_CPU_SGPCS,       0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { SGMII2_5,      MTI_CPU_MAC,   0,     MTI_CPU_SGPCS,       0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_CPU_MAC,   0,     MTI_CPU_SGPCS,       0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _10GBase_KR,   MTI_CPU_MAC,   0,     MTI_CPU_PCS,         0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_CPU_MAC,   0,     MTI_CPU_PCS,         0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_CPU_MAC,   0,     MTI_CPU_PCS,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR,   MTI_CPU_MAC,   0,     MTI_CPU_PCS,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_SR,   MTI_CPU_MAC,   0,     MTI_CPU_PCS,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _5GBaseR,      MTI_CPU_MAC,   0,     MTI_CPU_PCS,         0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_CR_C, MTI_CPU_MAC,   0,     MTI_CPU_PCS,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_C, MTI_CPU_MAC,   0,     MTI_CPU_PCS,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_S, MTI_CPU_MAC,   0,     MTI_CPU_PCS,         0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR_S, MTI_CPU_MAC,   0,     MTI_CPU_PCS,         0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,         NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};


/* Last line of each SupModes array MUST be NON_SUP_MODE */
const MV_HWS_PORT_INIT_PARAMS hwsPhoenixPort50SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type         PCS Num  FEC             Speed            FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { SGMII,         MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,   0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,   0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { SGMII2_5,      MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,   0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,   0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _10GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_50,          0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_MAC_100,   0,     MTI_PCS_50,          0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR,   MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_SR,   MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_C, MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_C, MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_S, MTI_MAC_100,   0,     MTI_PCS_50,          0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR_S, MTI_MAC_100,   0,     MTI_PCS_50,          0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _40GBase_KR2,  MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR2,  MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2,  MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR2_C,MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2_C,MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_SR2,  MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _40GBase_KR4,  MTI_MAC_100,   0,     MTI_PCS_50,          0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _40GBase_CR4,  MTI_MAC_100,   0,     MTI_PCS_50,          0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _40GBase_SR_LR4,MTI_MAC_100,  0,     MTI_PCS_50,          0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _100GBase_KR4, MTI_MAC_100,   0,     MTI_PCS_100,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _100GBase_CR4, MTI_MAC_100,   0,     MTI_PCS_100,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _100GBase_SR4, MTI_MAC_100,   0,     MTI_PCS_100,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   4,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|RS_FEC},
    { _5GBaseR,      MTI_MAC_100,   0,     MTI_PCS_50,          0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,         NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

const MV_HWS_PORT_INIT_PARAMS hwsPhoenixPort51SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type         PCS Num  FEC             Speed            FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { SGMII,         MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,   0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,   0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { SGMII2_5,      MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,   0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,   0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _10GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_25,          0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_MAC_100,   0,     MTI_PCS_25,          0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_25,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR,   MTI_MAC_100,   0,     MTI_PCS_25,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_SR,   MTI_MAC_100,   0,     MTI_PCS_25,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _5GBaseR,      MTI_MAC_100,   0,     MTI_PCS_25,          0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_CR_C, MTI_MAC_100,   0,     MTI_PCS_25,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_C, MTI_MAC_100,   0,     MTI_PCS_25,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_S, MTI_MAC_100,   0,     MTI_PCS_25,          0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR_S, MTI_MAC_100,   0,     MTI_PCS_25,          0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,         NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

const MV_HWS_PORT_INIT_PARAMS hwsPhoenixPort52SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type         PCS Num  FEC             Speed            FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { SGMII,         MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,   0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,   0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { SGMII2_5,      MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,   0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_MAC_100,   0,     MTI_PCS_LOW_SPEED,   0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _10GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_50,          0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_MAC_100,   0,     MTI_PCS_50,          0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR,   MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_SR,   MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_C, MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_KR_C, MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _25GBase_CR_S, MTI_MAC_100,   0,     MTI_PCS_50,          0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _25GBase_KR_S, MTI_MAC_100,   0,     MTI_PCS_50,          0,   FC_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _40GBase_KR2,  MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _20_625G,        NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR2,  MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2,  MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_KR2_C,MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_CR2_C,MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _50GBase_SR2,  MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   2,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC|RS_FEC},
    { _5GBaseR,      MTI_MAC_100,   0,     MTI_PCS_50,          0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,         NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

/* the 54 MACs in the GOP of the Phoenix */
#define PHOENIX_P0_P3_SUP_MODES    hwsPhoenixPort0SupModes /* 0*/,hwsPhoenixPort1SupModes /* 1*/, hwsPhoenixPort2SupModes /* 2*/, hwsPhoenixPort2SupModes /* 3*/
#define PHOENIX_P4_P7_SUP_MODES    hwsPhoenixPort4SupModes /* 4*/,hwsPhoenixPort4SupModes /* 5*/, hwsPhoenixPort4SupModes /* 6*/, hwsPhoenixPort4SupModes /* 7*/
#define PHOENIX_P8_P15_SUP_MODES   PHOENIX_P0_P3_SUP_MODES, PHOENIX_P4_P7_SUP_MODES
#define PHOENIX_P16_P23_SUP_MODES  PHOENIX_P8_P15_SUP_MODES
#define PHOENIX_P24_P31_SUP_MODES  PHOENIX_P8_P15_SUP_MODES
#define PHOENIX_P32_P39_SUP_MODES  PHOENIX_P8_P15_SUP_MODES
#define PHOENIX_P40_P47_SUP_MODES  PHOENIX_P8_P15_SUP_MODES
#define PHOENIX_P48_SUP_MODES      hwsPhoenixPort48SupModes /*48*/
#define PHOENIX_P49_SUP_MODES      hwsPhoenixPort48SupModes /*49*/
#define PHOENIX_P50_53_SUP_MODES   hwsPhoenixPort50SupModes, hwsPhoenixPort51SupModes, hwsPhoenixPort52SupModes, hwsPhoenixPort51SupModes


/* map the right supported modes table for each port */
const MV_HWS_PORT_INIT_PARAMS **hwsPortsPhoenixParamsSupModesMap;

const MV_HWS_PORT_INIT_PARAMS *hwsPortsPhoenix_ParamsSupModesMap[] =
{
    /* Phoenix 'dummy' hold  */
     PHOENIX_P0_P3_SUP_MODES
    ,PHOENIX_P4_P7_SUP_MODES
    ,PHOENIX_P8_P15_SUP_MODES
    ,PHOENIX_P16_P23_SUP_MODES
    ,PHOENIX_P24_P31_SUP_MODES
    ,PHOENIX_P32_P39_SUP_MODES
    ,PHOENIX_P40_P47_SUP_MODES
    ,PHOENIX_P48_SUP_MODES
    ,PHOENIX_P49_SUP_MODES
    ,PHOENIX_P50_53_SUP_MODES
    /* CPU port to be done */
};

#define IRONMAN_P48_P55_SUP_MODES PHOENIX_P40_P47_SUP_MODES

const MV_HWS_PORT_INIT_PARAMS *hwsPortsIronman_ParamsSupModesMap[] =
{
    /* Phoenix 'dummy' hold  */
     PHOENIX_P0_P3_SUP_MODES
    ,PHOENIX_P4_P7_SUP_MODES
    ,PHOENIX_P8_P15_SUP_MODES
    ,PHOENIX_P16_P23_SUP_MODES
    ,PHOENIX_P24_P31_SUP_MODES
    ,PHOENIX_P32_P39_SUP_MODES
    ,PHOENIX_P40_P47_SUP_MODES
    ,IRONMAN_P48_P55_SUP_MODES/*48..55 - dummy for Ironman*/
};


GT_STATUS hwsPhoenixPortElementsDbInit  /*hwsPhoenixAsHawkCheck()*/
(
    GT_U8 devNum
)
{
    devNum = devNum;

    if(hwsIsIronmanAsPhoenix())
    {
        hwsPortsPhoenixParamsSupModesMap = hwsPortsIronman_ParamsSupModesMap;
    }
    else
    {
        hwsPortsPhoenixParamsSupModesMap = hwsPortsPhoenix_ParamsSupModesMap;
    }

    return GT_OK;
}


GT_STATUS hwsPhoenixPortParamsGetLanes
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_INIT_PARAMS*    portParams
)
{
    GT_U16    laneCtnt = 0;
    GT_U16    *activeLanes = NULL;
    GT_U16    i = 0;
    MV_HWS_HAWK_CONVERT_STC         convertIdx;

#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3))
      /* SerDes-Muxing related variables */
    MV_HWS_PORT_SERDES_TO_MAC_MUX   hwsMacToSerdesMuxStc;
    GT_U32                          phyPortNumTmp;
    GT_U16                          idx;
#endif
    /* avoid warnings */
    portGroup = portGroup;

    if (portParams->portStandard == NON_SUP_MODE)
    {
        return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(mvHwsGlobalMacToLocalIndexConvert(devNum, phyPortNum, portParams->portStandard, &convertIdx));

    if(phyPortNum < MV_HWS_AC5X_GOP_PORT_CPU_0_CNS || hwsIsIronmanAsPhoenix())
    {
         portParams->firstLaneNum = (GT_U16)(convertIdx.ciderUnit);
    }
    else if(phyPortNum <= MV_HWS_AC5X_GOP_PORT_CPU_1_CNS)/*48,49*/
    {
        portParams->firstLaneNum = (GT_U16)(convertIdx.ciderUnit + 6);
    }
    else/*50,51,52,53*/
    {
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3))
        phyPortNumTmp = 50;/* portNum=50 only supported */
        if(GT_OK == mvHwsSip6_10PortLaneMacToSerdesMuxGet(devNum,(GT_U8)portGroup,phyPortNumTmp,&hwsMacToSerdesMuxStc))
        {
            idx = phyPortNum-phyPortNumTmp;
            if(idx > 4)
            {
                return GT_FAIL;
            }

            activeLanes = portParams->activeLanesList;
            for(i = 0; i < portParams->numOfActLanes; i++)
            {
                activeLanes[laneCtnt++] = (GT_U16)(8+ hwsMacToSerdesMuxStc.serdesLanes[idx +i]);
            }
            portParams->firstLaneNum = activeLanes[0];
            return GT_OK;
        }
        else
#endif
        {
            portParams->firstLaneNum = (GT_U16)(8 + convertIdx.ciderIndexInUnit);
        }
    }

    if((mvHwsUsxModeCheck(devNum, phyPortNum, portParams->portStandard) == GT_FALSE) && ((portParams->firstLaneNum % portParams->numOfActLanes) != 0))
    {
        return GT_NOT_SUPPORTED;
    }

    /* build active lane list */
    activeLanes = portParams->activeLanesList;
    for (i = portParams->firstLaneNum; /* build the active lanes list for this portMode */
         i < (portParams->firstLaneNum + portParams->numOfActLanes); i++)
    {
        activeLanes[laneCtnt++] = i;
    }
    return GT_OK;
}



GT_STATUS hwsPhoenixPortParamsSet
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

    if (phyPortNum >= hwsDeviceSpecInfo[devNum].portsNum)
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


