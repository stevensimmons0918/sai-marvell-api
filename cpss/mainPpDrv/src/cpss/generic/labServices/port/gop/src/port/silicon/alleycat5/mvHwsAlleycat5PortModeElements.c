/*******************************************************************************
*            Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsAlleycat3PortModeElements.c
*
* @brief
*
* @version   11
********************************************************************************
*/

#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/generic/labservices/port/gop/silicon/alleycat3/mvHwsAlleycat3PortIf.h>

#define MV_AC5_NW_PORT_START   24

#ifdef WIN32
extern void printPortModeTable(MV_HWS_PORT_INIT_PARAMS **hwsPortsParams, GT_U32 numOfPorts);
#endif

#if !defined (AC5_DEV_SUPPORT)
/*
For future 2.5G-SGMII
  {SGMII2_5,                 GEMAC_NET_SG,0,         GPCS_NET,      0,      FEC_OFF,  _3_125G,   0,        {0,0,0,0},    1,     XAUI_MEDIA,  _10BIT_ON,  FEC_OFF         },

*/

/* Last line of each array MUST be NON_SUP_MODE */
static const MV_HWS_PORT_INIT_PARAMS hwsAlleycat5Port0SupModes[] = {
 /* port mode,                MAC Type   Mac Num     PCS Type    PCS Num    FEC       Speed    FirstLane   LanesList  LanesNum   Media       10Bit       FecSupported */
  {SGMII,                    GEMAC_NET_SG,0,         GPCS_NET,      0,      FEC_OFF,  _1_25G,  NA_NUM,     {0,0,0,0},    1,     XAUI_MEDIA,  _10BIT_ON,  FEC_OFF         },
  {_1000Base_X,              GEMAC_NET_X, 0,         GPCS_NET,      0,      FEC_OFF,  _1_25G,  NA_NUM,     {0,0,0,0},    1,     XAUI_MEDIA,  _10BIT_ON,  FEC_OFF         },
  {SGMII2_5,                 GEMAC_NET_SG,0,         GPCS_NET,      0,      FEC_OFF,  _3_125G, NA_NUM,     {0,0,0,0},    1,     XAUI_MEDIA,  _10BIT_ON,  FEC_OFF         },
  {QSGMII,                   QSGMII_MAC,  0,         QSGMII_PCS,    0,      FEC_OFF,  _5G ,    NA_NUM,     {0,0,0,0},    1,     XAUI_MEDIA,  _10BIT_OFF, FEC_OFF         },
  {_2500Base_X,              GEMAC_NET_X, 0,         GPCS_NET,      0,      FEC_OFF,  _3_125G, NA_NUM,     {0,0,0,0},    1,     XAUI_MEDIA,  _10BIT_ON,  FEC_OFF         },

  {NON_SUP_MODE,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},    0,     XAUI_MEDIA,  _10BIT_NA,  FEC_NA}
};

static const MV_HWS_PORT_INIT_PARAMS hwsAlleycat5Port24SupModes[] = {
  /* port mode,               MAC Type    Mac Num    PCS Type    PCS Num    FEC       Speed     FirstLane  LanesList LanesNum   Media         10Bit     FecSupported*/
  {SGMII,                     GEMAC_SG,   24,        GPCS_NET,      24,     FEC_OFF,  _1_25G,    NA_NUM,   {6,0,0,0},   1,      XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_1000Base_X,               GEMAC_X,    24,        GPCS_NET,      24,     FEC_OFF,  _1_25G,    NA_NUM,   {6,0,0,0},   1,      XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {SGMII2_5,                  GEMAC_SG,   24,        GPCS_NET,      24,     FEC_OFF,  _3_125G,   NA_NUM,   {6,0,0,0},   1,      XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_2500Base_X,               GEMAC_X    ,24,        GPCS_NET,      24,     FEC_OFF,  _3_125G,   NA_NUM,   {6,0,0,0},   1,      XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_10GBase_KX4,              XGMAC,      24,        XPCS,          24,     FEC_OFF,  _3_125G,   NA_NUM,   {6,0,0,0},   4,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_10GBase_KR,               XLGMAC,     24,        MMPCS,         24,     FEC_OFF,  _10_3125G, NA_NUM,   {6,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_20GBase_KR2,              XLGMAC,     24,        MMPCS,         24,     FEC_OFF,  _10_3125G, NA_NUM,   {6,0,0,0},   2,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_20GBase_KX4,              XGMAC,      24,        XPCS,          24,     FEC_OFF,  _6_25G,    NA_NUM,   {6,0,0,0},   4,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_10GBase_SR_LR,            XLGMAC,     24,        MMPCS,         24,     FEC_OFF,  _10_3125G, NA_NUM,   {6,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_20GBase_SR_LR2,           XLGMAC,     24,        MMPCS,         24,     FEC_OFF,  _10_3125G, NA_NUM,   {6,0,0,0},   2,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_12_1GBase_KR,             XLGMAC,     24,        MMPCS,         24,     FEC_OFF,  _12_5G,    NA_NUM,   {6,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_2_5GBase_QX,              XGMAC,      24,        XPCS,          24,     FEC_OFF,  _3_125G,   NA_NUM,   {6,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_5GBase_DQX,               XGMAC,      24,        XPCS,          24,     FEC_OFF,  _6_25G,    NA_NUM,   {6,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_5GBase_HX,                XGMAC,      24,        XPCS,          24,     FEC_OFF,  _3_125G,   NA_NUM,   {6,0,0,0},   2,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_12GBaseR,                 XLGMAC,     24,        MMPCS,         24,     FEC_OFF,  _12_1875G, NA_NUM,   {6,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_5_625GBaseR,              XLGMAC,     24,        MMPCS,         24,     FEC_OFF,  _5_625G,   NA_NUM,   {6,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_5GBaseR,                  XLGMAC,     24,        MMPCS,         24,     FEC_OFF,  _5_15625G, NA_NUM,   {6,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_22GBase_SR,               XLGMAC,     24,        MMPCS,         24,     FEC_OFF,  _11_5625G, NA_NUM,   {6,0,0,0},   2,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },

  {NON_SUP_MODE,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA}
};

static const MV_HWS_PORT_INIT_PARAMS hwsAlleycat5Port25SupModes[] = {
  /* port mode,               MAC Type    Mac Num    PCS Type    PCS Num    FEC       Speed     FirstLane  LanesList LanesNum   Media         10Bit     FecSupported*/
  {SGMII,                     GEMAC_SG,   25,        GPCS_NET,      25,     FEC_OFF,  _1_25G,    NA_NUM,   {7,0,0,0},   1,      XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_1000Base_X,               GEMAC_X,    25,        GPCS_NET,      25,     FEC_OFF,  _1_25G,    NA_NUM,   {7,0,0,0},   1,      XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {SGMII2_5,                  GEMAC_SG,   25,        GPCS_NET,      25,     FEC_OFF,  _3_125G,   NA_NUM,   {7,0,0,0},   1,      XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_2500Base_X,               GEMAC_X    ,25,        GPCS_NET,      25,     FEC_OFF,  _3_125G,   NA_NUM,   {7,0,0,0},   1,     XAUI_MEDIA,   _10BIT_ON, FEC_OFF         },
  {_10GBase_KR,               XLGMAC,     25,        MMPCS,         25,     FEC_OFF,  _10_3125G, NA_NUM,   {7,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_10GBase_SR_LR,            XLGMAC,     25,        MMPCS,         25,     FEC_OFF,  _10_3125G, NA_NUM,   {7,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_12_1GBase_KR,             XLGMAC,     25,        MMPCS,         25,     FEC_OFF,  _12_5G,    NA_NUM,   {7,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_NA          },
  {_12GBaseR,                 XLGMAC,     25,        MMPCS,         25,     FEC_OFF,  _12_1875G, NA_NUM,   {7,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_5_625GBaseR,              XLGMAC,     25,        MMPCS,         25,     FEC_OFF,  _5_625G,   NA_NUM,   {7,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_5GBaseR,                  XLGMAC,     25,        MMPCS,         25,     FEC_OFF,  _5_15625G, NA_NUM,   {7,0,0,0},   1,      XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },

  {NON_SUP_MODE,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},   0,      XAUI_MEDIA,  _10BIT_NA, FEC_NA}
};

static const MV_HWS_PORT_INIT_PARAMS hwsAlleycat5Port26SupModes[] = {
  /* port mode,               MAC Type    Mac Num    PCS Type    PCS Num    FEC       Speed     FirstLane  LanesList LanesNum   Media         10Bit     FecSupported*/
  {SGMII,                     GEMAC_SG,   26,        GPCS_NET,      26,     FEC_OFF,  _1_25G,    NA_NUM,   {8,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_1000Base_X,               GEMAC_X,    26,        GPCS_NET,      26,     FEC_OFF,  _1_25G,    NA_NUM,   {8,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {SGMII2_5,                  GEMAC_SG,   26,        GPCS_NET,      26,     FEC_OFF,  _3_125G,   NA_NUM,   {8,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_2500Base_X,               GEMAC_X    ,26,        GPCS_NET,      26,     FEC_OFF,  _3_125G,   NA_NUM,   {8,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_10GBase_KR,               XLGMAC,     26,        MMPCS,         26,     FEC_OFF,  _10_3125G, NA_NUM,   {8,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_20GBase_KR2,              XLGMAC,     26,        MMPCS,         26,     FEC_OFF,  _10_3125G, NA_NUM,   {8,0,0,0},  2,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_10GBase_SR_LR,            XLGMAC,     26,        MMPCS,         26,     FEC_OFF,  _10_3125G, NA_NUM,   {8,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_20GBase_SR_LR2,           XLGMAC,     26,        MMPCS,         26,     FEC_OFF,  _10_3125G, NA_NUM,   {8,0,0,0},  2,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_12_1GBase_KR,             XLGMAC,     26,        MMPCS,         26,     FEC_OFF,  _12_5G,    NA_NUM,   {8,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_2_5GBase_QX,              XGMAC,      26,        XPCS,          26,     FEC_OFF,  _3_125G,   NA_NUM,   {8,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_5GBase_DQX,               XGMAC,      26,        XPCS,          26,     FEC_OFF,  _6_25G,    NA_NUM,   {8,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_5GBase_HX,                XGMAC,      26,        XPCS,          26,     FEC_OFF,  _3_125G,   NA_NUM,   {8,0,0,0},  2,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_12GBaseR,                 XLGMAC,     26,        MMPCS,         26,     FEC_OFF,  _12_1875G, NA_NUM,   {8,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_5_625GBaseR,              XLGMAC,     26,        MMPCS,         26,     FEC_OFF,  _5_625G,   NA_NUM,   {8,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_5GBaseR,                  XLGMAC,     26,        MMPCS,         26,     FEC_OFF,  _5_15625G, NA_NUM,   {8,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_22GBase_SR,               XLGMAC,     26,        MMPCS,         26,     FEC_OFF,  _11_5625G, NA_NUM,   {8,0,0,0},  2,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },

  {NON_SUP_MODE,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA}
};

static const MV_HWS_PORT_INIT_PARAMS hwsAlleycat5Port27SupModes[] = {
  /* port mode,               MAC Type    Mac Num    PCS Type    PCS Num    FEC       Speed     FirstLane  LanesList LanesNum   Media         10Bit     FecSupported*/
  {SGMII,                     GEMAC_SG,   27,        GPCS_NET,      27,     FEC_OFF,  _1_25G,    NA_NUM,   {9,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_1000Base_X,               GEMAC_X,    27,        GPCS_NET,      27,     FEC_OFF,  _1_25G,    NA_NUM,   {9,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {SGMII2_5,                  GEMAC_SG,   27,        GPCS_NET,      27,     FEC_OFF,  _3_125G,   NA_NUM,   {9,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_2500Base_X,               GEMAC_X    ,27,        GPCS_NET,      27,     FEC_OFF,  _3_125G,   NA_NUM,   {9,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_10GBase_KR,               XLGMAC,     27,        MMPCS,         27,     FEC_OFF,  _10_3125G, NA_NUM,   {9,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_10GBase_SR_LR,            XLGMAC,     27,        MMPCS,         27,     FEC_OFF,  _10_3125G, NA_NUM,   {9,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_12_1GBase_KR,             XLGMAC,     27,        MMPCS,         27,     FEC_OFF,  _12_5G,    NA_NUM,   {9,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_12GBaseR,                 XLGMAC,     27,        MMPCS,         27,     FEC_OFF,  _12_1875G, NA_NUM,   {9,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_5_625GBaseR,              XLGMAC,     27,        MMPCS,         27,     FEC_OFF,  _5_625G,   NA_NUM,   {9,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_5GBaseR,                  XLGMAC,     27,        MMPCS,         27,     FEC_OFF,  _5_15625G, NA_NUM,   {9,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },

  {NON_SUP_MODE,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA}
};

static const MV_HWS_PORT_INIT_PARAMS hwsAlleycat5Port28SupModes[] = {
  /* port mode,               MAC Type    Mac Num    PCS Type    PCS Num    FEC       Speed     FirstLane  LanesList LanesNum   Media         10Bit     FecSupported*/
  {SGMII,                     GEMAC_SG,   28,        GPCS_NET,   28,        FEC_OFF,  _1_25G,    NA_NUM,   {10,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_1000Base_X,               GEMAC_X,    28,        GPCS_NET,   28,        FEC_OFF,  _1_25G,    NA_NUM,   {10,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {SGMII2_5,                  GEMAC_SG,   28,        GPCS_NET,   28,        FEC_OFF,  _3_125G,   NA_NUM,   {10,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_2500Base_X,               GEMAC_X,    28,        GPCS_NET,   28,        FEC_OFF,  _3_125G,   NA_NUM,   {10,0,0,0},  1,       XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_10GBase_KR,               XLGMAC,     28,        MMPCS,      28,        FEC_OFF,  _10_3125G, NA_NUM,   {10,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_10GBase_SR_LR,            XLGMAC,     28,        MMPCS,      28,        FEC_OFF,  _10_3125G, NA_NUM,   {10,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_12_1GBase_KR,             XLGMAC,     28,        MMPCS,      28,        FEC_OFF,  _12_5G,    NA_NUM,   {10,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_5GBase_DQX,               XGMAC,      28,        XPCS,       28,        FEC_OFF,  _6_25G,    NA_NUM,   {10,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_12GBaseR,                 XLGMAC,     28,        MMPCS,      28,        FEC_OFF,  _12_1875G, NA_NUM,   {10,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_5_625GBaseR,              XLGMAC,     28,        MMPCS,      28,        FEC_OFF,  _5_625G,   NA_NUM,   {10,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_5GBaseR,                  XLGMAC,     28,        MMPCS,      28,        FEC_OFF,  _5_15625G, NA_NUM,   {10,0,0,0},  1,       XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },

  {NON_SUP_MODE,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,       XAUI_MEDIA,  _10BIT_NA, FEC_NA}
};

static const MV_HWS_PORT_INIT_PARAMS hwsAlleycat5Port29SupModes[] = {
  /* port mode,               MAC Type    Mac Num    PCS Type    PCS Num    FEC       Speed     FirstLane  LanesList LanesNum   Media          10Bit     FecSupported*/
  {SGMII,                     GEMAC_SG,   29,        GPCS_NET,   29,        FEC_OFF,  _1_25G,    NA_NUM,   {11,0,0,0},  1,        XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_1000Base_X,               GEMAC_X,    29,        GPCS_NET,   29,        FEC_OFF,  _1_25G,    NA_NUM,   {11,0,0,0},  1,        XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {SGMII2_5,                  GEMAC_SG,   29,        GPCS_NET,   29,        FEC_OFF,  _3_125G,   NA_NUM,   {11,0,0,0},  1,        XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_2500Base_X,               GEMAC_X,    29,        GPCS_NET,   29,        FEC_OFF,  _3_125G,   NA_NUM,   {11,0,0,0},  1,        XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
  {_10GBase_KR,               XLGMAC,     29,        MMPCS,      29,        FEC_OFF,  _10_3125G, NA_NUM,   {11,0,0,0},  1,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_20GBase_KR2,              XLGMAC,     29,        MMPCS,      29,        FEC_OFF,  _10_3125G, NA_NUM,   {10,0,0,0},  2,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_10GBase_SR_LR,            XLGMAC,     29,        MMPCS,      29,        FEC_OFF,  _10_3125G, NA_NUM,   {11,0,0,0},  1,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_20GBase_SR_LR2,           XLGMAC,     29,        MMPCS,      29,        FEC_OFF,  _10_3125G, NA_NUM,   {10,0,0,0},  2,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_12_1GBase_KR,             XLGMAC,     29,        MMPCS,      29,        FEC_OFF,  _12_5G,    NA_NUM,   {11,0,0,0},  1,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_2_5GBase_QX,              XGMAC,      29,        XPCS,       29,        FEC_OFF,  _3_125G,   NA_NUM,   {11,0,0,0},  1,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_5GBase_DQX,               XGMAC,      29,        XPCS,       29,        FEC_OFF,  _6_25G,    NA_NUM,   {11,0,0,0},  1,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_5GBase_HX,                XGMAC,      29,        XPCS,       29,        FEC_OFF,  _3_125G,   NA_NUM,   {10,0,0,0},  2,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_12GBaseR,                 XLGMAC,     29,        MMPCS,      29,        FEC_OFF,  _12_1875G, NA_NUM,   {11,0,0,0},  1,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },
  {_5_625GBaseR,              XLGMAC,     29,        MMPCS,      29,        FEC_OFF,  _5_625G,   NA_NUM,   {11,0,0,0},  1,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_5GBaseR,                  XLGMAC,     29,        MMPCS,      29,        FEC_OFF,  _5_15625G, NA_NUM,   {11,0,0,0},  1,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF         },
  {_22GBase_SR,               XLGMAC,     29,        MMPCS,      29,        FEC_OFF,  _11_5625G, NA_NUM,   {10,0,0,0},  2,        XAUI_MEDIA,  _10BIT_OFF,FEC_OFF|FC_FEC  },

  {NON_SUP_MODE,              MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA}
};

static const MV_HWS_PORT_INIT_PARAMS hwsAlleycat5Port31SupModes[] = {
  /* port mode,               MAC Type    Mac Num    PCS Type    PCS Num    FEC       Speed     FirstLane  LanesList LanesNum   Media          10Bit     FecSupported*/
    {SGMII,                   GEMAC_SG,   31,        GPCS_NET,      31,     FEC_OFF,  _1_25G,    NA_NUM,   {0,10,0,0},  1,        XAUI_MEDIA,  _10BIT_ON, FEC_OFF         },
    {NON_SUP_MODE,            MAC_NA,     NA_NUM,    PCS_NA,     NA_NUM,    FEC_NA,   SPEED_NA,  0,        {0,0,0,0},  0,        XAUI_MEDIA,  _10BIT_NA, FEC_NA}
};

#if !defined(BC2_DEV_SUPPORT) && !defined(AC3_DEV_SUPPORT)
/* array of all ports (32) init parameters */
const MV_HWS_PORT_INIT_PARAMS *hwsPortsAlleycat5ParamsSupModesMap[] =
{
    /* 0*/hwsAlleycat5Port0SupModes, /* 1*/hwsAlleycat5Port0SupModes, /* 2*/hwsAlleycat5Port0SupModes, /* 3*/hwsAlleycat5Port0SupModes,
    /* 4*/hwsAlleycat5Port0SupModes, /* 5*/hwsAlleycat5Port0SupModes, /* 6*/hwsAlleycat5Port0SupModes, /* 7*/hwsAlleycat5Port0SupModes,
    /* 8*/hwsAlleycat5Port0SupModes, /* 9*/hwsAlleycat5Port0SupModes, /*10*/hwsAlleycat5Port0SupModes, /*11*/hwsAlleycat5Port0SupModes,
    /*12*/hwsAlleycat5Port0SupModes, /*13*/hwsAlleycat5Port0SupModes, /*14*/hwsAlleycat5Port0SupModes, /*15*/hwsAlleycat5Port0SupModes,
    /*16*/hwsAlleycat5Port0SupModes, /*17*/hwsAlleycat5Port0SupModes, /*18*/hwsAlleycat5Port0SupModes, /*19*/hwsAlleycat5Port0SupModes,
    /*20*/hwsAlleycat5Port0SupModes, /*21*/hwsAlleycat5Port0SupModes, /*22*/hwsAlleycat5Port0SupModes, /*23*/hwsAlleycat5Port0SupModes,
    /*24*/hwsAlleycat5Port24SupModes,
    /*25*/hwsAlleycat5Port25SupModes,
    /*26*/hwsAlleycat5Port26SupModes,
    /*27*/hwsAlleycat5Port27SupModes,
    /*28*/hwsAlleycat5Port28SupModes,/*Extended 25*/
    /*29*/hwsAlleycat5Port29SupModes,/*Extended 27*/
    /*30*/NULL,
    /*31*/hwsAlleycat5Port31SupModes
};

#if 0
/* init ports 1 - 23 */
GT_STATUS hwsAlleycat5Ports1To23Cfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup
)
{
    MV_HWS_DEVICE_PORTS_ELEMENTS    curDevicePorts = hwsDevicesPortsElementsArray[devNum];
    MV_HWS_PORT_INIT_PARAMS         *curPortParams;
    GT_U8 portNum;
    GT_U8 portMode;

    /* avoid warning */
    portGroup = portGroup;
    for (portNum = 1; portNum <= 23; portNum++)
    {
        curPortParams = (MV_HWS_PORT_INIT_PARAMS *)hwsOsMallocFuncPtr(sizeof(hwsAlleycat5Port0SupModes));
        if (curPortParams == NULL)
        {
          return GT_NO_RESOURCE;
        }

        curDevicePorts[portNum].supModesCatalog = curPortParams;
        hwsOsMemCopyFuncPtr(curPortParams, hwsAlleycat5Port0SupModes, sizeof(hwsAlleycat5Port0SupModes));

        for (portMode = 0; portMode < hwsDeviceSpecInfo[devNum].lastSupPortMode; portMode++)
        {

            if (curPortParams[portMode].portStandard == NON_SUP_MODE)
            {
                break;
            }

            if (curPortParams[portMode].portMacType == MAC_NA)
            {
                continue;
            }

            if (((portNum % 4) != 0) && curPortParams[portMode].portStandard == QSGMII)
            {
                curPortParams[portMode].portMacNumber = portNum; /* in order to be able to check the link */
                curPortParams[portMode].portPcsNumber = portNum; /* in order to be able to set loopback on port */
                curPortParams[portMode].numOfActLanes = 0;
                curPortParams[portMode].firstLaneNum = 0;

                continue;
            }
            if ((portNum % 4) != 0)
            {
                if(   curPortParams[portMode].portStandard == SGMII
                   || curPortParams[portMode].portStandard == SGMII2_5
                   || curPortParams[portMode].portStandard == _1000Base_X
                   || curPortParams[portMode].portStandard == _2500Base_X)
                {
                    curPortParams[portMode].portMacNumber = NA_NUM; /* in order to be able to check the link */
                    curPortParams[portMode].portPcsNumber = NA_NUM; /* in order to be able to set loopback on port */
                    curPortParams[portMode].numOfActLanes = 0;
                    curPortParams[portMode].firstLaneNum = NA_NUM;
                    continue;
                }
            }

            curPortParams[portMode].portMacNumber = portNum;
            curPortParams[portMode].portPcsNumber = portNum;
            curPortParams[portMode].firstLaneNum = portNum / 4;

        }
    }

    return GT_OK;
}
#endif
#endif
#endif
GT_STATUS hwsAlleycat5PortParamsGetLanes
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

    /* avoid warnings */
    devNum = devNum;
    portGroup = portGroup;

    if (portParams->portStandard == NON_SUP_MODE)
    {
        return GT_NOT_SUPPORTED;
    }

    if (phyPortNum < MV_AC5_NW_PORT_START)
    {
        portParams->firstLaneNum  =  (GT_U16)phyPortNum/4;

        if (((phyPortNum % 4) != 0) && portParams->portStandard == QSGMII)
        {
            portParams->numOfActLanes = 0;
            /*portParams->firstLaneNum = 0;*/
        }
        else if ((phyPortNum % 4) != 0)
        {
            if(   portParams->portStandard == SGMII
               || portParams->portStandard == SGMII2_5
               || portParams->portStandard == _1000Base_X
               || portParams->portStandard == _2500Base_X)
            {
                portParams->numOfActLanes = 0;
                portParams->firstLaneNum = NA_NUM;
            }
        }
    }
    else
    {
        portParams->firstLaneNum  =  portParams->activeLanesList[0];
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
#if !defined (AC5_DEV_SUPPORT)
GT_STATUS hwsAlleycat5PortParamsSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_PORT_INIT_PARAMS         *curPortParams;
    const MV_HWS_PORT_INIT_PARAMS   *newPortParams;
    MV_HWS_DEVICE_PORTS_ELEMENTS    hwsPortsAlleycat5Params = hwsDevicesPortsElementsArray[devNum];
    GT_U32                          portModeIndex;

    /* avoid warnings */
    portGroup = portGroup;

    if (phyPortNum > hwsDeviceSpecInfo[devNum].portsNum)
    {
        return GT_OUT_OF_RANGE;
    }
    if (NULL == hwsPortsAlleycat5Params)
    {
        return GT_NOT_INITIALIZED;
    }

    if ( hwsExtendedPortNumGet(devNum, portGroup, NULL, &phyPortNum ) != GT_OK)
    {
        return GT_FAIL;
    }

    curPortParams = &hwsPortsAlleycat5Params[phyPortNum].curPortParams;
    if (NON_SUP_MODE == portMode) /* Reset port */
    {
        /* initialize entry with default port params */
        hwsOsMemCopyFuncPtr(curPortParams, &hwsSingleEntryInitParams, sizeof(MV_HWS_PORT_INIT_PARAMS));
        curPortParams->portMacNumber =  (GT_U16)phyPortNum;
        curPortParams->portPcsNumber =  (GT_U16)phyPortNum;
        curPortParams->firstLaneNum  =  (GT_U16)phyPortNum/4;
        return GT_OK;
    }

    newPortParams = hwsPortsElementsFindMode(hwsDevicePortsElementsSupModesCatalogGet(devNum,phyPortNum), portMode, &portModeIndex);
    if (NULL == newPortParams || newPortParams->numOfActLanes == 0)
    {
        return GT_NOT_SUPPORTED;
    }
    /* Copy the right port mode line from the port's specific supported modes list*/
    hwsOsMemCopyFuncPtr(curPortParams, newPortParams, sizeof(MV_HWS_PORT_INIT_PARAMS));

    curPortParams->portMacNumber =  (GT_U16)phyPortNum;
    curPortParams->portPcsNumber =  (GT_U16)phyPortNum;
    if (phyPortNum < MV_AC5_NW_PORT_START)
    {
        if ((phyPortNum % 4) != 0)
        {
            if(   portMode == SGMII
               || portMode == SGMII2_5
               || portMode == _1000Base_X
               || portMode == _2500Base_X)
            {
                curPortParams->portMacNumber = NA_NUM; /* in order to be able to check the link */
                curPortParams->portPcsNumber = NA_NUM; /* in order to be able to set loopback on port */
            }
        }
    }

    curPortParams->portFecMode = hwsPortsAlleycat5Params[phyPortNum].perModeFecList[portModeIndex];

    /* set the first lane and the active lanes list */
    return hwsAlleycat5PortParamsGetLanes(devNum,portGroup,phyPortNum,curPortParams);
}
#endif