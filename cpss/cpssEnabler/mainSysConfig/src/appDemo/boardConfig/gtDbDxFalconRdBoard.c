/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtDbDxFalconRdBoard.c
*
* @brief Initialization functions for the Falcon - SIP6 - RD board.
*
* @version   1
********************************************************************************
*/
#include <appDemo/boardConfig/gtDbDxFalcon.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoI2cConfig.h>
#include <cpss/common/port/cpssPortManager.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

typedef struct {
    char *name;
    CPSS_PORT_SPEED_ENT speed;

} FALCON_RD_PORT_SPEED_STC, *PFALCON_RD_PORT_SPEED_STC;

typedef struct {
    char *name;
    CPSS_PORT_INTERFACE_MODE_ENT mode;

} FALCON_RD_PORT_MODE_STC, *PFALCON_RD_PORT_MODE_STC;

FALCON_RD_PORT_SPEED_STC falcon_RD_PortSpeedNames[] =
{
    { "100G", CPSS_PORT_SPEED_100G_E },
    { "50G",  CPSS_PORT_SPEED_50000_E },
    { "25G",  CPSS_PORT_SPEED_25000_E },
    { NULL,   CPSS_PORT_SPEED_NA_E }
};

FALCON_RD_PORT_MODE_STC falcon_RD_PortModeNames[] =
{
    { "KR4",    CPSS_PORT_INTERFACE_MODE_KR4_E },
    { "CR4",    CPSS_PORT_INTERFACE_MODE_CR4_E },
    { "SR_LR4", CPSS_PORT_INTERFACE_MODE_SR_LR4_E },
    { "KR2",    CPSS_PORT_INTERFACE_MODE_KR2_E },
    { "CR2",    CPSS_PORT_INTERFACE_MODE_CR2_E },
    { "KR",     CPSS_PORT_INTERFACE_MODE_KR_E },
    { "CR",     CPSS_PORT_INTERFACE_MODE_CR_E},
    { "SR_LR",  CPSS_PORT_INTERFACE_MODE_SR_LR_E},
    { NULL,     CPSS_PORT_INTERFACE_MODE_NA_E }
};

CPSS_PORT_MAC_TO_SERDES_STC  falcon_RD_MacToSerdesMap[FALCON_RD_MAC_TO_SERDES_MAP_ARR_SIZE] =
{
   {{0,3,5,7,6,1,4,2}}, /*  line card 1 P7-P10*/
   {{5,1,2,4,0,3,7,6}}, /*  line card 1 P5,6,11,12*/
   {{4,5,1,3,7,2,6,0}}, /*  line card 1 P3,4,13,14*/
   {{5,7,2,1,4,0,3,6}}, /*  line card 1 P1,2,15,16*/
   {{0,2,4,6,7,1,5,3}}, /*  line card 2 P7-P10*/
   {{7,3,4,2,0,1,5,6}}, /*  line card 2 P5,6,11,12*/
   {{1,5,3,6,7,2,4,0}}, /*  line card 2 P3,4,13,14*/
   {{6,5,4,1,2,0,3,7}}, /*  line card 2 P1,2,15,16*/

   {{0,3,5,7,6,1,4,2}}, /*  line card 3 P7-P10*/
   {{5,1,2,4,0,3,7,6}}, /*  line card 3 P5,6,11,12*/
   {{4,5,1,3,7,2,6,0}}, /*  line card 3 P3,4,13,14*/
   {{5,7,2,1,4,0,3,6}}, /*  line card 3 P1,2,15,16*/
   {{0,2,4,6,7,1,5,3}}, /*  line card 4 P7-P10*/
   {{7,3,4,2,0,1,5,6}}, /*  line card 4 P5,6,11,12*/
   {{1,5,3,6,7,2,4,0}}, /*  line card 4 P3,4,13,14*/
   {{6,5,4,1,2,0,3,7}}, /*  line card 4 P1,2,15,16*/

   {{0,3,5,7,6,1,4,2}}, /*  line card 5 P7-P10*/
   {{5,1,2,4,0,3,7,6}}, /*  line card 5 P5,6,11,12*/
   {{4,5,1,3,7,2,6,0}}, /*  line card 5 P3,4,13,14*/
   {{5,7,2,1,4,0,3,6}}, /*  line card 5 P1,2,15,16*/
   {{0,2,4,6,7,1,5,3}}, /*  line card 6 P7-P10*/
   {{7,3,4,2,0,1,5,6}}, /*  line card 6 P5,6,11,12*/
   {{1,5,3,6,7,2,4,0}}, /*  line card 6 P3,4,13,14*/
   {{6,5,4,1,2,0,3,7}}, /*  line card 6 P1,2,15,16*/

   {{0,3,5,7,6,1,4,2}}, /*  line card 7 P7-P10*/
   {{5,1,2,4,0,3,7,6}}, /*  line card 7 P5,6,11,12*/
   {{4,5,1,3,7,2,6,0}}, /*  line card 7 P3,4,13,14*/
   {{5,7,2,1,4,0,3,6}}, /*  line card 7 P1,2,15,16*/
   {{0,2,4,6,7,1,5,3}}, /*  line card 8 P7-P10*/
   {{7,3,4,2,0,1,5,6}}, /*  line card 8 P5,6,11,12*/
   {{1,5,3,6,7,2,4,0}}, /*  line card 8 P3,4,13,14*/
   {{6,5,4,1,2,0,3,7}}  /*  line card 8 P1,2,15,16*/
};

APPDEMO_SERDES_LANE_POLARITY_STC  falcon_RD_PolarityArray[FALCON_RD_POLARITY_ARR_SIZE] =
{
/* laneNum  invertTx    invertRx */
    { 0,    GT_FALSE,   GT_FALSE  },
    { 1,    GT_FALSE,   GT_FALSE  },
    { 2,    GT_FALSE,   GT_FALSE  },
    { 3,    GT_FALSE,   GT_FALSE  },
    { 4,    GT_FALSE,   GT_FALSE  },
    { 5,    GT_FALSE,   GT_FALSE  },
    { 6,    GT_FALSE,   GT_FALSE  },
    { 7,    GT_FALSE,   GT_FALSE  },
    { 8,    GT_FALSE,   GT_FALSE  },
    { 9,    GT_FALSE,   GT_FALSE  },
    { 10,   GT_FALSE,   GT_FALSE  },
    { 11,   GT_TRUE ,   GT_FALSE  },
    { 12,   GT_TRUE ,   GT_FALSE  },
    { 13,   GT_FALSE,   GT_FALSE  },
    { 14,   GT_FALSE,   GT_FALSE  },
    { 15,   GT_FALSE,   GT_FALSE  },
    { 16,   GT_TRUE ,   GT_FALSE  },
    { 17,   GT_FALSE,   GT_FALSE  },
    { 18,   GT_FALSE,   GT_FALSE  },
    { 19,   GT_FALSE,   GT_FALSE  },
    { 20,   GT_FALSE,   GT_FALSE  },
    { 21,   GT_FALSE,   GT_FALSE  },
    { 22,   GT_FALSE,   GT_FALSE  },
    { 23,   GT_FALSE,   GT_FALSE  },
    { 24,   GT_FALSE,   GT_FALSE  },
    { 25,   GT_FALSE,   GT_FALSE  },
    { 26,   GT_FALSE,   GT_FALSE  },
    { 27,   GT_FALSE,   GT_FALSE  },
    { 28,   GT_FALSE,   GT_FALSE  },
    { 29,   GT_FALSE,   GT_FALSE  },
    { 30,   GT_FALSE,   GT_FALSE  },
    { 31,   GT_FALSE,   GT_FALSE  },
    { 32,   GT_FALSE,   GT_TRUE   },
    { 33,   GT_FALSE,   GT_FALSE  },
    { 34,   GT_FALSE,   GT_TRUE   },
    { 35,   GT_FALSE,   GT_FALSE  },
    { 36,   GT_FALSE,   GT_TRUE   },
    { 37,   GT_FALSE,   GT_FALSE  },
    { 38,   GT_FALSE,   GT_TRUE   },
    { 39,   GT_FALSE,   GT_FALSE  },
    { 40,   GT_FALSE,   GT_FALSE  },
    { 41,   GT_FALSE,   GT_FALSE  },
    { 42,   GT_FALSE,   GT_FALSE  },
    { 43,   GT_TRUE ,   GT_FALSE  },
    { 44,   GT_TRUE ,   GT_FALSE  },
    { 45,   GT_FALSE,   GT_FALSE  },
    { 46,   GT_FALSE,   GT_FALSE  },
    { 47,   GT_FALSE,   GT_FALSE  },
    { 48,   GT_FALSE,   GT_FALSE  },
    { 49,   GT_FALSE,   GT_FALSE  },
    { 50,   GT_FALSE,   GT_FALSE  },
    { 51,   GT_FALSE,   GT_FALSE  },
    { 52,   GT_FALSE,   GT_FALSE  },
    { 53,   GT_FALSE,   GT_FALSE  },
    { 54,   GT_FALSE,   GT_FALSE  },
    { 55,   GT_FALSE,   GT_FALSE  },
    { 56,   GT_FALSE,   GT_FALSE  },
    { 57,   GT_FALSE,   GT_FALSE  },
    { 58,   GT_FALSE,   GT_FALSE  },
    { 59,   GT_FALSE,   GT_FALSE  },
    { 60,   GT_FALSE,   GT_FALSE  },
    { 61,   GT_FALSE,   GT_FALSE  },
    { 62,   GT_FALSE,   GT_FALSE  },
    { 63,   GT_FALSE,   GT_FALSE  },
    { 64,   GT_FALSE,   GT_FALSE  },
    { 65,   GT_FALSE,   GT_FALSE  },
    { 66,   GT_FALSE,   GT_FALSE  },
    { 67,   GT_FALSE,   GT_FALSE  },
    { 68,   GT_FALSE,   GT_FALSE  },
    { 69,   GT_FALSE,   GT_FALSE  },
    { 70,   GT_FALSE,   GT_FALSE  },
    { 71,   GT_FALSE,   GT_FALSE  },
    { 72,   GT_FALSE,   GT_FALSE  },
    { 73,   GT_FALSE,   GT_FALSE  },
    { 74,   GT_FALSE,   GT_FALSE  },
    { 75,   GT_TRUE ,   GT_FALSE  },
    { 76,   GT_TRUE ,   GT_FALSE  },
    { 77,   GT_FALSE,   GT_FALSE  },
    { 78,   GT_FALSE,   GT_FALSE  },
    { 79,   GT_FALSE,   GT_FALSE  },
    { 80,   GT_TRUE ,   GT_FALSE  },
    { 81,   GT_FALSE,   GT_FALSE  },
    { 82,   GT_FALSE,   GT_FALSE  },
    { 83,   GT_FALSE,   GT_FALSE  },
    { 84,   GT_FALSE,   GT_FALSE  },
    { 85,   GT_FALSE,   GT_FALSE  },
    { 86,   GT_FALSE,   GT_FALSE  },
    { 87,   GT_FALSE,   GT_FALSE  },
    { 88,   GT_FALSE,   GT_FALSE  },
    { 89,   GT_FALSE,   GT_FALSE  },
    { 90,   GT_FALSE,   GT_FALSE  },
    { 91,   GT_FALSE,   GT_FALSE  },
    { 92,   GT_FALSE,   GT_FALSE  },
    { 93,   GT_FALSE,   GT_FALSE  },
    { 94,   GT_FALSE,   GT_FALSE  },
    { 95,   GT_FALSE,   GT_FALSE  },
    { 96,   GT_FALSE,   GT_TRUE   },
    { 97,   GT_FALSE,   GT_FALSE  },
    { 98,   GT_FALSE,   GT_TRUE   },
    { 99,   GT_FALSE,   GT_FALSE  },
    { 100,  GT_FALSE,   GT_TRUE   },
    { 101,  GT_FALSE,   GT_FALSE  },
    { 102,  GT_FALSE,   GT_TRUE   },
    { 103,  GT_FALSE,   GT_FALSE  },
    { 104,  GT_FALSE,   GT_FALSE  },
    { 105,  GT_FALSE,   GT_FALSE  },
    { 106,  GT_FALSE,   GT_FALSE  },
    { 107,  GT_TRUE ,   GT_FALSE  },
    { 108,  GT_TRUE ,   GT_FALSE  },
    { 109,  GT_FALSE,   GT_FALSE  },
    { 110,  GT_FALSE,   GT_FALSE  },
    { 111,  GT_FALSE,   GT_FALSE  },
    { 112,  GT_FALSE,   GT_FALSE  },
    { 113,  GT_FALSE,   GT_FALSE  },
    { 114,  GT_FALSE,   GT_FALSE  },
    { 115,  GT_FALSE,   GT_FALSE  },
    { 116,  GT_FALSE,   GT_FALSE  },
    { 117,  GT_FALSE,   GT_FALSE  },
    { 118,  GT_FALSE,   GT_FALSE  },
    { 119,  GT_FALSE,   GT_FALSE  },
    { 120,  GT_FALSE,   GT_FALSE  },
    { 121,  GT_FALSE,   GT_FALSE  },
    { 122,  GT_FALSE,   GT_FALSE  },
    { 123,  GT_FALSE,   GT_FALSE  },
    { 124,  GT_FALSE,   GT_FALSE  },
    { 125,  GT_FALSE,   GT_FALSE  },
    { 126,  GT_FALSE,   GT_FALSE  },
    { 127,  GT_FALSE,   GT_FALSE  },
    { 128,  GT_FALSE,   GT_FALSE  },
    { 129,  GT_FALSE,   GT_FALSE  },
    { 130,  GT_FALSE,   GT_FALSE  },
    { 131,  GT_FALSE,   GT_FALSE  },
    { 132,  GT_FALSE,   GT_FALSE  },
    { 133,  GT_FALSE,   GT_FALSE  },
    { 134,  GT_FALSE,   GT_FALSE  },
    { 135,  GT_FALSE,   GT_FALSE  },
    { 136,  GT_FALSE,   GT_FALSE  },
    { 137,  GT_FALSE,   GT_FALSE  },
    { 138,  GT_FALSE,   GT_FALSE  },
    { 139,  GT_TRUE ,   GT_FALSE  },
    { 140,  GT_TRUE ,   GT_FALSE  },
    { 141,  GT_FALSE,   GT_FALSE  },
    { 142,  GT_FALSE,   GT_FALSE  },
    { 143,  GT_FALSE,   GT_FALSE  },
    { 144,  GT_TRUE ,   GT_FALSE  },
    { 145,  GT_FALSE,   GT_FALSE  },
    { 146,  GT_FALSE,   GT_FALSE  },
    { 147,  GT_FALSE,   GT_FALSE  },
    { 148,  GT_FALSE,   GT_FALSE  },
    { 149,  GT_FALSE,   GT_FALSE  },
    { 150,  GT_FALSE,   GT_FALSE  },
    { 151,  GT_FALSE,   GT_FALSE  },
    { 152,  GT_FALSE,   GT_FALSE  },
    { 153,  GT_FALSE,   GT_FALSE  },
    { 154,  GT_FALSE,   GT_FALSE  },
    { 155,  GT_FALSE,   GT_FALSE  },
    { 156,  GT_FALSE,   GT_FALSE  },
    { 157,  GT_FALSE,   GT_FALSE  },
    { 158,  GT_FALSE,   GT_FALSE  },
    { 159,  GT_FALSE,   GT_FALSE  },
    { 160,  GT_FALSE,   GT_TRUE   },
    { 161,  GT_FALSE,   GT_FALSE  },
    { 162,  GT_FALSE,   GT_TRUE   },
    { 163,  GT_FALSE,   GT_FALSE  },
    { 164,  GT_FALSE,   GT_TRUE   },
    { 165,  GT_FALSE,   GT_FALSE  },
    { 166,  GT_FALSE,   GT_TRUE   },
    { 167,  GT_FALSE,   GT_FALSE  },
    { 168,  GT_FALSE,   GT_FALSE  },
    { 169,  GT_FALSE,   GT_FALSE  },
    { 170,  GT_FALSE,   GT_FALSE  },
    { 171,  GT_TRUE ,   GT_FALSE  },
    { 172,  GT_TRUE ,   GT_FALSE  },
    { 173,  GT_FALSE,   GT_FALSE  },
    { 174,  GT_FALSE,   GT_FALSE  },
    { 175,  GT_FALSE,   GT_FALSE  },
    { 176,  GT_FALSE,   GT_FALSE  },
    { 177,  GT_FALSE,   GT_FALSE  },
    { 178,  GT_FALSE,   GT_FALSE  },
    { 179,  GT_FALSE,   GT_FALSE  },
    { 180,  GT_FALSE,   GT_FALSE  },
    { 181,  GT_FALSE,   GT_FALSE  },
    { 182,  GT_FALSE,   GT_FALSE  },
    { 183,  GT_FALSE,   GT_FALSE  },
    { 184,  GT_FALSE,   GT_FALSE  },
    { 185,  GT_FALSE,   GT_FALSE  },
    { 186,  GT_FALSE,   GT_FALSE  },
    { 187,  GT_FALSE,   GT_FALSE  },
    { 188,  GT_FALSE,   GT_FALSE  },
    { 189,  GT_FALSE,   GT_FALSE  },
    { 190,  GT_FALSE,   GT_FALSE  },
    { 191,  GT_FALSE,   GT_FALSE  },
    { 192,  GT_FALSE,   GT_FALSE  },
    { 193,  GT_FALSE,   GT_FALSE  },
    { 194,  GT_FALSE,   GT_FALSE  },
    { 195,  GT_FALSE,   GT_FALSE  },
    { 196,  GT_FALSE,   GT_FALSE  },
    { 197,  GT_FALSE,   GT_FALSE  },
    { 198,  GT_FALSE,   GT_FALSE  },
    { 199,  GT_FALSE,   GT_FALSE  },
    { 200,  GT_FALSE,   GT_FALSE  },
    { 201,  GT_FALSE,   GT_FALSE  },
    { 202,  GT_FALSE,   GT_FALSE  },
    { 203,  GT_TRUE ,   GT_FALSE  },
    { 204,  GT_TRUE ,   GT_FALSE  },
    { 205,  GT_FALSE,   GT_FALSE  },
    { 206,  GT_FALSE,   GT_FALSE  },
    { 207,  GT_FALSE,   GT_FALSE  },
    { 208,  GT_TRUE ,   GT_FALSE  },
    { 209,  GT_FALSE,   GT_FALSE  },
    { 210,  GT_FALSE,   GT_FALSE  },
    { 211,  GT_FALSE,   GT_FALSE  },
    { 212,  GT_FALSE,   GT_FALSE  },
    { 213,  GT_FALSE,   GT_FALSE  },
    { 214,  GT_FALSE,   GT_FALSE  },
    { 215,  GT_FALSE,   GT_FALSE  },
    { 216,  GT_FALSE,   GT_FALSE  },
    { 217,  GT_FALSE,   GT_FALSE  },
    { 218,  GT_FALSE,   GT_FALSE  },
    { 219,  GT_FALSE,   GT_FALSE  },
    { 220,  GT_FALSE,   GT_FALSE  },
    { 221,  GT_FALSE,   GT_FALSE  },
    { 222,  GT_FALSE,   GT_FALSE  },
    { 223,  GT_FALSE,   GT_FALSE  },
    { 224,  GT_FALSE,   GT_TRUE   },
    { 225,  GT_FALSE,   GT_FALSE  },
    { 226,  GT_FALSE,   GT_TRUE   },
    { 227,  GT_FALSE,   GT_FALSE  },
    { 228,  GT_FALSE,   GT_TRUE   },
    { 229,  GT_FALSE,   GT_FALSE  },
    { 230,  GT_FALSE,   GT_TRUE   },
    { 231,  GT_FALSE,   GT_FALSE  },
    { 232,  GT_FALSE,   GT_FALSE  },
    { 233,  GT_FALSE,   GT_FALSE  },
    { 234,  GT_FALSE,   GT_FALSE  },
    { 235,  GT_TRUE,    GT_FALSE  },
    { 236,  GT_TRUE,    GT_FALSE  },
    { 237,  GT_FALSE,   GT_FALSE  },
    { 238,  GT_FALSE,   GT_FALSE  },
    { 239,  GT_FALSE,   GT_FALSE  },
    { 240,  GT_FALSE,   GT_FALSE  },
    { 241,  GT_FALSE,   GT_FALSE  },
    { 242,  GT_FALSE,   GT_FALSE  },
    { 243,  GT_FALSE,   GT_FALSE  },
    { 244,  GT_FALSE,   GT_FALSE  },
    { 245,  GT_FALSE,   GT_FALSE  },
    { 246,  GT_FALSE,   GT_FALSE  },
    { 247,  GT_FALSE,   GT_FALSE  },
    { 248,  GT_FALSE,   GT_FALSE  },
    { 249,  GT_FALSE,   GT_FALSE  },
    { 250,  GT_FALSE,   GT_FALSE  },
    { 251,  GT_FALSE,   GT_FALSE  },
    { 252,  GT_FALSE,   GT_FALSE  },
    { 253,  GT_FALSE,   GT_FALSE  },
    { 254,  GT_FALSE,   GT_FALSE  },
    { 255,  GT_FALSE,   GT_FALSE  },
    { 257,  GT_FALSE,   GT_TRUE  },
    { 258,  GT_FALSE,   GT_FALSE  }
};

#ifndef ASIC_SIMULATION

GT_APPDEMO_XPHY_INFO_STC falcon_RD_PhyInfo[FALCON_RD_PHY_INFO_ARR_SIZE] =
{
    /*Card 0*/
    {0,  0,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x0, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {1,  0,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x4, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {2,  0,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x8, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {3,  0,  CPSS_PHY_XSMI_INTERFACE_1_E,  0xc, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},

    /*Card 1*/
    {4,  1,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x0, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {5,  1,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x4, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {6,  1,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x8, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {7,  1,  CPSS_PHY_XSMI_INTERFACE_1_E,  0xc, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    /*Card 2*/
    {8,  2,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x0, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {9,  2,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x4, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {10, 2,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x8, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {11, 2,  CPSS_PHY_XSMI_INTERFACE_1_E,  0xc, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    /*Card 3*/
    {12, 3,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x0, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {13, 3,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x4, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {14, 3,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x8, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {15, 3,  CPSS_PHY_XSMI_INTERFACE_1_E,  0xc, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    /*Card 4*/
    {16, 4,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x0, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {17, 4,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x4, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {18, 4,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x8, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {19, 4,  CPSS_PHY_XSMI_INTERFACE_1_E,  0xc, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    /*Card 5*/
    {20, 5,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x0, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {21, 5,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x4, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {22, 5,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x8, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {23, 5,  CPSS_PHY_XSMI_INTERFACE_1_E,  0xc, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    /*Card 6*/
    {24, 6,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x0, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {25, 6,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x4, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {26, 6,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x8, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {27, 6,  CPSS_PHY_XSMI_INTERFACE_1_E,  0xc, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    /*Card 7*/
    {28, 7,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x0, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {29, 7,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x4, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {30, 7,  CPSS_PHY_XSMI_INTERFACE_1_E,  0x8, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {31, 7,  CPSS_PHY_XSMI_INTERFACE_1_E,  0xc, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
};

#if 1 /* TX parameters according to Thomas Wang for Falcon RD A3 */
CPSS_PORT_SERDES_TUNE_STC falcon_RD_PhyTuneParams[FALCON_RD_PHY_TUNE_PARAMS_ARR_SIZE] =
{
    /* RD_SHORT_TRACE_PARAMS */
    /* HOST-SIDE  ===========> {Tx=atten,post,pre,pre2,pre3} {Rx = sqlch,DC,LF,HF,BW} */
    {CPSS_PORT_SERDES_AVAGO_E, {.avago={3, 4, 2, 2, 1}},     {.avago={0, 0, 5, 0, 0, 0, {0}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {0}, 0, 0, 0}}},
    /* LINE-SIDE  ===========> {Tx=atten,post,pre,pre2,pre3} {Rx = sqlch,DC,LF,HF,BW} */
    {CPSS_PORT_SERDES_AVAGO_E, {.avago={1, 6, 2, 3, 0}},     {.avago={0, 0, 5, 5, 5, 0, {0}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {0}, 0, 0, 0}}},

    /* RD_LONG_TRACE_PARAMS */
    /* HOST-SIDE  ===========> {Tx=atten,post,pre,pre2,pre3} {Rx = sqlch,DC,LF,HF,BW} */
    {CPSS_PORT_SERDES_AVAGO_E, {.avago={1, 6, 2, 2, 1}},     {.avago={0, 0, 5, 0, 0, 0, {0}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {0}, 0, 0, 0}}},
    /* LINE-SIDE  ===========> {Tx=atten,post,pre,pre2,pre3} {Rx = sqlch,DC,LF,HF,BW} */
    {CPSS_PORT_SERDES_AVAGO_E, {.avago={1, 6, 2, 3, 0}},     {.avago={0, 0, 5, 5, 5, 0, {0}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {0}, 0, 0, 0}}},
};
#else
CPSS_PORT_SERDES_TUNE_STC falcon_RD_PhyTuneParams[FALCON_RD_PHY_TUNE_PARAMS_ARR_SIZE] =
{
    /* RD_SHORT_TRACE_PARAMS */
    /* HOST-SIDE  ===========> {Tx=atten,post,pre,pre2,pre3} {Rx = sqlch,DC,LF,HF,BW} */
    {CPSS_PORT_SERDES_AVAGO_E, {.avago={5, 6, 2, 2, 1}},     {.avago={0, 0, 5, 0, 0, 0, {0}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}},
    /* LINE-SIDE  ===========> {Tx=atten,post,pre,pre2,pre3} {Rx = sqlch,DC,LF,HF,BW} */
    {CPSS_PORT_SERDES_AVAGO_E, {.avago={0, 4, 0, 0, 0}},     {.avago={0, 0, 5, 5, 5, 0, {0}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}},

    /* RD_LONG_TRACE_PARAMS */
    /* HOST-SIDE  ===========> {Tx=atten,post,pre,pre2,pre3} {Rx = sqlch,DC,LF,HF,BW} */
    {CPSS_PORT_SERDES_AVAGO_E, {.avago={1, 6, 2, 2, 1}},     {.avago={0, 0, 5, 0, 0, 0, {0}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}},
    /* LINE-SIDE  ===========> {Tx=atten,post,pre,pre2,pre3} {Rx = sqlch,DC,LF,HF,BW} */
    {CPSS_PORT_SERDES_AVAGO_E, {.avago={0, 4, 0, 0, 0}},     {.avago={0, 0, 5, 5, 5, 0, {0}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}},
};
#endif

CPSS_PORT_SERDES_TX_CONFIG_STC  falcon_RD_serdesTxParams[2] /* RD Short and long trace parameters */ =
{
    {CPSS_PORT_SERDES_AVAGO_E, {.avago={3, 4, 2, 2, 1}}},
    {CPSS_PORT_SERDES_AVAGO_E, {.avago={0, 4, 0, 0, 0}}},
};



#endif /* ASIC_SIMULATION */

static GT_STATUS falcon_RD_cpssParamsToPhyParams
(
    IN     GT_U8                           devNum,
    IN     GT_PHYSICAL_PORT_NUM            falconPortNum,
    INOUT  CPSS_PORT_INTERFACE_MODE_ENT    *ifMode,
    INOUT  CPSS_PORT_SPEED_ENT             *speed,
    INOUT  CPSS_PORT_FEC_MODE_ENT          *fecMode,
    OUT    GT_APPDEMO_XPHY_OP_MODE         *phyOpMode,
    OUT    GT_APPDEMO_XPHY_FEC_MODE        *phyFecMode
)
{
    GT_U32 fecEnumValues[] =
    {
        /* CPSS_PORT_FEC_MODE_ENABLED_E            */ GT_APPDEMO_XPHY_FC_FEC,
        /* CPSS_PORT_FEC_MODE_DISABLED_E           */ GT_APPDEMO_XPHY_NO_FEC,
        /* CPSS_PORT_RS_FEC_MODE_ENABLED_E         */ GT_APPDEMO_XPHY_RS_FEC,
        /* CPSS_PORT_BOTH_FEC_MODE_ENABLED_E       */ GT_APPDEMO_XPHY_NO_FEC,
        /* CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E */ GT_APPDEMO_XPHY_RS_FEC_544_514
    };

    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(falconPortNum);

    *phyFecMode = fecEnumValues[*fecMode];

    if(*speed == CPSS_PORT_SPEED_100G_E)
    {
        switch(*ifMode)
        {
            case CPSS_PORT_INTERFACE_MODE_CR4_E:
            case CPSS_PORT_INTERFACE_MODE_KR4_E:
                /**
                 * Fix CPSS-10168,10265;
                 * TBD why seen the issue when phyOpMode is set to
                 * PCS_HOST_100G_KR2_LINE_100G_CR4
                 */
                *phyOpMode = PCS_HOST_100G_KR2_LINE_100G_SR4;
                *ifMode    = CPSS_PORT_INTERFACE_MODE_KR2_E;
                *fecMode   = CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                break;
            case CPSS_PORT_INTERFACE_MODE_SR_LR4_E:
                *phyOpMode = PCS_HOST_100G_KR2_LINE_100G_SR4;
                *ifMode    = CPSS_PORT_INTERFACE_MODE_KR2_E;
                *fecMode   = CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                break;
            case CPSS_PORT_INTERFACE_MODE_KR2_E:
                *phyOpMode = PCS_HOST_100G_KR2_LINE_100G_KR2;
                *ifMode    = CPSS_PORT_INTERFACE_MODE_KR2_E;
                *fecMode   = CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                break;
            case CPSS_PORT_INTERFACE_MODE_CR2_E:
                *phyOpMode = PCS_HOST_100G_KR2_LINE_100G_CR2;
                *ifMode    = CPSS_PORT_INTERFACE_MODE_KR2_E;
                *fecMode   = CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                break;
            default:
                return GT_NOT_SUPPORTED;
        }
    }

    else if(*speed == CPSS_PORT_SPEED_50000_E)
    {
        switch(*ifMode)
        {
            case CPSS_PORT_INTERFACE_MODE_KR_E:
            case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
            case CPSS_PORT_INTERFACE_MODE_CR_E:
                *phyOpMode = PCS_HOST_50G_KR_LINE_50G_KR;
                *ifMode    = CPSS_PORT_INTERFACE_MODE_KR_E;
                *fecMode   = CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E;
                break;
            default:
                return GT_NOT_SUPPORTED;
        }
    }
    else
    {
        return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}

#if !defined(ASIC_SIMULATION) && defined(INCLUDE_MPD)
static GT_STATUS falcon_RD_cpssSpeedAndModeToString
(
    IN  GT_U8                        devNum,
    IN  CPSS_PORT_SPEED_ENT          speed,
    IN  CPSS_PORT_INTERFACE_MODE_ENT mode,
    OUT char                         **speedName,
    OUT char                         **modeName
)
{
    int i;
    GT_UNUSED_PARAM(devNum);
    if( CPSS_PORT_INTERFACE_MODE_NA_E <= mode && CPSS_PORT_SPEED_NA_E <= speed) {
        return GT_OK;
    }

    for (i=0; NULL != falcon_RD_PortSpeedNames[i].name; i++) {
        if (speed == falcon_RD_PortSpeedNames[i].speed) {
            *speedName = falcon_RD_PortSpeedNames[i].name;
        }
    }

    for (i=0; NULL != falcon_RD_PortModeNames[i].name; i++) {
        if (mode == falcon_RD_PortModeNames[i].mode) {
            *modeName = falcon_RD_PortModeNames[i].name;
        }
    }

    return GT_OK;
}

static GT_STATUS falcon_RD_phyParamsToCpssParams
(
    IN     GT_U8                           devNum,
    IN     GT_PHYSICAL_PORT_NUM            falconPortNum,
    IN     GT_APPDEMO_XPHY_OP_MODE         *phyOpMode,
    IN     GT_APPDEMO_XPHY_FEC_MODE        *phyFecMode,
    OUT    CPSS_PORT_INTERFACE_MODE_ENT    *ifMode,
    OUT    CPSS_PORT_SPEED_ENT             *speed
)
{
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(falconPortNum);
    GT_UNUSED_PARAM(phyFecMode);

    switch(*phyOpMode)
    {
        case PCS_HOST_25G_KR_LINE_25G_KR:
            *ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
            *speed  = CPSS_PORT_SPEED_50000_E;
            break;
        case PCS_HOST_50G_KR_LINE_50G_KR:
            *ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
            *speed  = CPSS_PORT_SPEED_50000_E;
            break;
        case PCS_HOST_100G_KR2_LINE_100G_CR4:
            *ifMode = CPSS_PORT_INTERFACE_MODE_CR4_E;
            *speed  = CPSS_PORT_SPEED_100G_E;
            break;
        case PCS_HOST_100G_KR2_LINE_100G_SR4:
        case PCS_HOST_100G_KR2_LINE_100G_LR4:
        case PCS_HOST_100G_KR2_AP_LINE_100G_LR4_AP:
        case PCS_HOST_100G_KR2_AP_LINE_100G_SR4:
            *ifMode = CPSS_PORT_INTERFACE_MODE_SR_LR4_E;
            *speed  = CPSS_PORT_SPEED_100G_E;
            break;
        case PCS_HOST_100G_KR2_LINE_100G_KR2:
            *ifMode = CPSS_PORT_INTERFACE_MODE_KR2_E;
            *speed  = CPSS_PORT_SPEED_100G_E;
            break;
        case PCS_HOST_100G_KR2_LINE_100G_CR2:
            *ifMode = CPSS_PORT_INTERFACE_MODE_CR2_E;
            *speed  = CPSS_PORT_SPEED_100G_E;
            break;
        default:
            *ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
            *speed  = CPSS_PORT_SPEED_NA_E;
    }

    return GT_OK;
}

/**
* @internal falcon_RD_getPortParams function
* @endinternal
*
* @brief   Get Falcon port params in RD board including for the
*          outside user - get the line side opMode and fecMode
*          in cpss enums
*
* @param [in]  devNum               - The CPSS devNum.
* @param [in]  falconPortNum        - The CPSS portNum.
* @param [out] ifMode               - Intrface mode (as seen
*       from outside)
* @param [out] speed                - speed (as seen from
*       outside)
* @param [out] linkStatus           - PCS link status (as seen
*       from outside)
*
* @retval GT_OK                     - on success,
* @retval GT_FAIL                   - otherwise.
*
*/
static GT_STATUS falcon_RD_getPortParams
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            falconPortNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT    *ifMode,
    OUT CPSS_PORT_SPEED_ENT             *speed
)
{
    GT_APPDEMO_XPHY_OP_MODE     phyOpMode;
    GT_APPDEMO_XPHY_FEC_MODE    phyFecMode;
    GT_U32                      phyPortNum, phyIndex;
    GT_STATUS                   rc;

    rc = falcon_phy_getPortNum(devNum,falconPortNum,&phyIndex,&phyPortNum);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = gtAppDemoPhyGetPortMode(falconPortNum, &phyOpMode,&phyFecMode,NULL,NULL, NULL);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = falcon_RD_phyParamsToCpssParams(devNum,falconPortNum,&phyOpMode,&phyFecMode,ifMode,speed);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
 *
 * falcon_RD_ShowPortStatus
 *
 * @param devNum
 * @param falconPortNum
 *
 * @return GT_STATUS
 */
GT_STATUS falcon_RD_ShowPortStatus
(   IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            falconPortNum,
    IN  GT_BOOL                         all
)
{
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode     = CPSS_PORT_INTERFACE_MODE_NA_E;
    CPSS_PORT_SPEED_ENT             speed      = CPSS_PORT_SPEED_NA_E;
    GT_U16                          linkStatus = 0;
    char                            *speedName = "n/a";
    char                            *modeName  = "n/a";
    char                            devPort[9];
    char                            devMode[64];
    GT_STATUS                       rc = GT_OK;

    GT_U32 i, firstPortNum = 128, lastPortNum = 255;

    if (GT_FALSE == all)
    {
        firstPortNum = falconPortNum;
        lastPortNum = falconPortNum;
    }

    osPrintf("\nDev/Port       Mode        Link   Speed  Duplex  Loopback Mode  Port Manager\n");
    osPrintf("---------  ----------------  -----  -----  ------  -------------  ------------\n");

    for ( i = firstPortNum; i <= lastPortNum; i++)
    {
        speedName = "n/a";
        modeName = "n/a";
        linkStatus = 0;
        rc = falcon_RD_getPortParams(devNum, i,&ifMode,&speed);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(CPSS_PORT_INTERFACE_MODE_NA_E != ifMode){
            rc = falcon_phy_getPortLinkStatus(devNum,i,&linkStatus);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = falcon_RD_cpssSpeedAndModeToString(devNum,speed,ifMode,&speedName,&modeName);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        sprintf( devPort, "%d/%d\t\t",devNum,i);
        sprintf( devMode, "%s\t   %s\t  %s\t N/A\t\tN/A\t  N/A",modeName,(1==linkStatus)?"Up":"Down",speedName);
        osPrintf("%s%s\n",devPort,devMode);
    }

    return GT_OK;
}
#endif

/**
 * @internal falcon_RD_deletePort
 * @endinternal
 *
 * @param devNum
 * @param falconPortNum
 *
 * @return GT_STATUS
 */
GT_STATUS falcon_RD_deletePort
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            falconPortNum
)
{
    CPSS_PORT_MANAGER_STC       portEventStc;
    GT_STATUS                   rc;

    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_DELETE_E;
    rc = cpssDxChPortManagerEventSet(devNum, falconPortNum, &portEventStc);
    if (rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_RD_deletePort failed: cpssDxChPortManagerEventSet (Delete)", rc);
        return rc;
    }
#if !defined(ASIC_SIMULATION) && defined(INCLUDE_MPD)
    rc = falcon_phy_deletePort(devNum, falconPortNum);
    if (rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_RD_deletePort failed: falcon_phy_deletePort", rc);
        return rc;
    }
#endif /*ASIC_SIMULATION*/

    return GT_OK;
}

/**
 *
 * falcon_RD_deletePortRange
 *
 * @param devNum
 * @param falconPortNum
 * @param numOfPorts
 *
 * @return GT_STATUS
 */
GT_STATUS falcon_RD_deletePortRange
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            falconPortNum,
    IN  GT_U8                           numOfPorts
)
{
    GT_U32 i;
    GT_STATUS rc = GT_OK;

    for (i=0; i<numOfPorts; i++)
    {
        rc = falcon_RD_deletePort(devNum,falconPortNum+i);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal falcon_RD_createPort function
* @endinternal
*
* @brief   Create Falcon port in RD board including CPSS port
*          create and PHY port create
*
* @param[in] devNum                - The CPSS devNum.
* @param[in] falconPortNum         - The CPSS portNum.
* @param[in] ifMode                - intrface mode
* @param[in] speed                 - speed
* @param[in] fecMode               - fec mode
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS falcon_RD_createPort
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            falconPortNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  CPSS_PORT_FEC_MODE_ENT          fecMode
)
{
    CPSS_PORT_MANAGER_STC       portEventStc;
    GT_APPDEMO_XPHY_OP_MODE     phyOpMode;
    GT_APPDEMO_XPHY_FEC_MODE    phyFecMode;
    GT_STATUS                   rc;

    rc = falcon_RD_cpssParamsToPhyParams(devNum, falconPortNum, &ifMode, &speed, &fecMode, &phyOpMode, &phyFecMode);
    if (rc != GT_OK)
    {
        return rc;
    }
    cpssOsPrintf("Falcon port %d (slot %d port %d): set phyOpMode=%d, phyFecMode=%d-\n",
             falconPortNum, falconPortNum/16, falconPortNum%16, phyOpMode, phyFecMode);

    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_DELETE_E;
    rc = cpssDxChPortManagerEventSet(devNum, falconPortNum, &portEventStc);
    if (rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_RD_createPort failed: cpssDxChPortManagerEventSet (Delete)", rc);
        return rc;
    }

    rc = cpssDxChSamplePortManagerMandatoryParamsSet(devNum, falconPortNum, ifMode, speed, fecMode);
    if (rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_RD_createPort failed: cpssDxChSamplePortManagerMandatoryParamsSet", rc);
        return rc;
    }
#if !defined(ASIC_SIMULATION) && defined(INCLUDE_MPD)
    rc = falcon_phy_createPort(devNum, falconPortNum, phyOpMode, phyFecMode, GT_TRUE);
    if (rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_RD_createPort failed: falcon_phy_createPort", rc);
        return rc;
    }
#endif /*ASIC_SIMULATION*/
    portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_CREATE_E;
    rc = cpssDxChPortManagerEventSet(devNum, falconPortNum, &portEventStc);
    if (rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_RD_createPort failed: cpssDxChPortManagerEventSet (Create)", rc);
        return rc;
    }
#if !defined(ASIC_SIMULATION) && defined(INCLUDE_MPD)
    rc = falcon_phy_tunePort(devNum, falconPortNum, GT_APPDEMO_XPHY_BOTH_SIDES);
    if (rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_RD_createPort failed: falcon_phy_tunePort", rc);
        return rc;
    }
#endif /*ASIC_SIMULATION*/

    return GT_OK;
}

/**
 * falcon_RD_createPortRange
 *
 *
 * @param devNum
 * @param falconPortNum
 * @param numOfPorts
 * @param ifMode
 * @param speed
 * @param fecMode
 *
 * @return GT_STATUS
 */
GT_STATUS falcon_RD_createPortRange
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            falconPortNum,
    IN  GT_U8                           numOfPorts,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  CPSS_PORT_FEC_MODE_ENT          fecMode
)
{
    GT_U32 i;
    GT_STATUS rc = GT_OK;

    for (i=0; i<numOfPorts; i++)
    {
        rc = falcon_RD_createPort(devNum,falconPortNum+i,ifMode,speed,fecMode);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

static CPSS_FALCON_LED_STREAM_INDICATIONS_STC rd_falcon_12_8_led_indications[CPSS_CHIPLETS_MAX_NUM_CNS] =
{
    {64, 71, GT_FALSE},
    {64, 72, GT_FALSE},
    {64, 71, GT_FALSE},
    {64, 71, GT_FALSE},
    {64, 71, GT_FALSE},
    {64, 71, GT_FALSE},
    {64, 71, GT_FALSE},
    {64, 71, GT_FALSE},
    {64, 71, GT_FALSE},
    {64, 71, GT_FALSE},
    {64, 71, GT_FALSE},
    {64, 71, GT_FALSE},
    {64, 71, GT_FALSE},
    {64, 71, GT_FALSE},
    {64, 71, GT_FALSE},
    {64, 71, GT_FALSE}
};

GT_STATUS falcon_RD_ledInit
(
    GT_U8    devNum
)
{
    GT_STATUS                       rc;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_LED_CONF_STC               ledConfig;
    CPSS_LED_CLASS_MANIPULATION_STC ledClassManip;
    GT_U32                          position;
    GT_U32                          tileIndex;
    GT_U32                          ravenIndex;
    GT_U32                          ledUnit;
    CPSS_FALCON_LED_STREAM_INDICATIONS_STC * ledStreamIndication;

    /* Macro returns LED position per MTI400 ports
      (two ports per Raven, one LED per port) */
    #define LED_PORT_POSITION_GET_MAC(portMacNum) \
            (portMacNum % 8 == 0) ? (portMacNum / 8) % 2 : APPDEMO_BAD_VALUE

    cpssOsMemSet(&ledConfig, 0, sizeof(CPSS_LED_CONF_STC));

    ledConfig.ledOrganize                       = CPSS_LED_ORDER_MODE_BY_CLASS_E;                       /* LedControl.OrganizeMode     [ 1: 1] =   1  by class       */
    ledConfig.sip6LedConfig.ledClockFrequency   = 1627;                                                 /* LedChainClockControl.ledClkOutDiv  [11: 22] = 512 (app_clock - 833333/led_clock_out - 1627)  */
    ledConfig.pulseStretch                      = CPSS_LED_PULSE_STRETCH_1_E;                           /* LedControl.Pulse-stretch-div[ 0: 29] =  100000 (ledClockFrequencyDivider - 10 * 10000  */

    ledConfig.blink0Duration          = CPSS_LED_BLINK_DURATION_1_E;    /* 64 ms */                     /* BlinkDivision0.blink0Duration  [ 0: 29]  = 0x32DCD40 (64/app_clock in mSec)*/
    ledConfig.blink0DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E;  /* 50%   */                     /* blinkGlobalControl.Blink0-Duty [ 3: 4]  = 1 25%  */
    ledConfig.blink1Duration          = CPSS_LED_BLINK_DURATION_1_E;    /* 64 ms */                     /* BlinkDivision1.blink0Duration  [ 0: 29]  = 0x32DCD40 (64/app_clock in mSec)*/
    ledConfig.blink1DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E;  /* 50%   */                     /* blinkGlobalControl.Blink1-Duty [ 8: 9]  = 1 25%  */

    ledConfig.disableOnLinkDown       = GT_FALSE;  /* don't care , see led-class manipulation */
    ledConfig.clkInvert               = GT_FALSE;  /* don't care */
    ledConfig.class5select            = CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E;   /* don't care */
    ledConfig.class13select           = CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E; /* don't care */

    ledStreamIndication = rd_falcon_12_8_led_indications;
    for (tileIndex = 0; tileIndex < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles; tileIndex++)
    {
        for(ravenIndex = 0; ravenIndex < FALCON_RAVENS_PER_TILE; ravenIndex++)
        {
            ledUnit = PRV_DXCH_LED_NUM_MAC(tileIndex, ravenIndex);

            /* Start of LED stream location - Class 2 */
            ledConfig.sip6LedConfig.ledStart[ledUnit] = ledStreamIndication[ledUnit].ledStart;    /* LedControl.LedStart[5:12] */
            /* End of LED stream location: LED ports 0, 1 or 0, 1 and 2*/
            ledConfig.sip6LedConfig.ledEnd[ledUnit] = ledStreamIndication[ledUnit].ledEnd;        /* LedControl.LedEnd[5:12] */
        }
    }

    rc = cpssDxChLedStreamConfigSet(devNum, 0, &ledConfig);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamConfigSet", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    ledClassManip.invertEnable            = GT_FALSE; /* not relevant for Falcon */
    ledClassManip.blinkSelect             = CPSS_LED_BLINK_SELECT_0_E;                            /* blinkGlobalControl.Blink Sel            [10-17] = 0 blink-0         */
    ledClassManip.forceEnable             = GT_FALSE;                                             /* classesAndGroupConfig.Force En          [24-29] = pos = classNum = 0*/
    ledClassManip.forceData               = 0;                                                    /* classForcedData[class]                  [ 0:31] = 0                 */
    ledClassManip.pulseStretchEnable      = GT_FALSE;                                             /* classGlobalControl.stretch class        [ 6-11] = pos = classNum = 0/1*/

    ledClassManip.disableOnLinkDown       = GT_TRUE;                                  /* classGlobalControl.disable on link down [ 0- 5] = pos = classNum = 0*/
    ledClassManip.blinkEnable             = GT_TRUE;
    rc = cpssDxChLedStreamClassManipulationSet(devNum, 0,
                                               CPSS_DXCH_LED_PORT_TYPE_XG_E /* don't care , not applicable for Falcon */
                                               ,2 ,&ledClassManip);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamClassManipulationSet", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    for(portNum = 128 ; portNum <  256; portNum++)
    {
        switch(portNum%16)
        {
            case 0 :
            case 4 :
                position = 0 ;
                break;
             case 1 :
             case 5 :
                position = 1 ;
                break;
             case 2 :
             case 6 :
                position = 2 ;
                break;

             case 3 :
             case 7 :
                position = 3 ;
                break;

             case 8 :
             case 12 :
                position = 4 ;
                break;

             case  9:
             case 13 :
                position = 5 ;
                break;

             case  10:
             case 14 :
                position = 6;
                break;

             case  11:
             case 15 :
                position = 7;
                break;

        }

        rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamPortPositionSet", rc);
        if(rc != GT_OK)
        {
            return rc;
        }

        /*handle cpu port */
        rc = cpssDxChLedStreamPortPositionSet(devNum, 0, 8);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamPortPositionSet", rc);
        if(rc != GT_OK)
        {
            return rc;
        }

        /*rd_falcon the invert is done by CPLD */
        rc = cpssDxChLedStreamPortClassPolarityInvertEnableSet(devNum, portNum, /*classNum*/2, /*invertEnable*/1);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamPortClassPolarityInvertEnableSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
   }

    return GT_OK;
}




#ifndef ASIC_SIMULATION

/*rd_falcon thermal task main function
main purpose is keep the rd-falcon IC temprature to be in the range of A~B , B> A
if the speed is <= B, then fan speed down 5% of full speed, to make the temperature go higher.
if the speed >= A , then fan speed up 5% of full speed to make the temperature lower.
if the temperature go up to crtical point , then fan at full speed , to make temperature go down quickly
*/

#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

static unsigned __TASKCONV falcon_RD_ThermalTask();
static GT_TASK cpss_rd_falcon_thermal_task_id ;
static CPSS_OS_MSGQ_ID cpss_rd_falcon_thermal_queue_id;

typedef enum _THERMAL_STATE{
    THERMAL_LOW ,
    THERMAL_NORMAL,
    THERMAL_HIGH,
    THERMAL_CRITICAL ,

} THERMAL_STATE;

GT_U32 thermal_critical = 100;
GT_U32 thermal_range_high = 90;
GT_U32 thermal_range_low =80;
GT_U32 cpss_rd_falcon_thermal_en = 1;
GT_BOOL cpss_thermal_debug = 0;

/**
 * falcon_RD_IcStateGet function
 *
 *
 * @param thermal
 *
 * @return THERMAL_STATE
 */
THERMAL_STATE falcon_RD_IcStateGet(GT_32 thermal)
{
    THERMAL_STATE state ;
    if(thermal >= (GT_32)thermal_critical)
        state = THERMAL_CRITICAL ;
    else if(thermal >= (GT_32)thermal_range_high)
        state = THERMAL_HIGH;
    else if(thermal >= (GT_32)thermal_range_low)
        state = THERMAL_NORMAL;
    else
        state = THERMAL_LOW ;

    return state ;
}

/**
 * falcon_RD_SetThermalRange function
 *
 *
 * @param critical
 * @param high
 * @param low
 *
 * @return GT_STATUS
 */
GT_STATUS  falcon_RD_SetThermalRange(GT_U32 critical, GT_U32 high, GT_U32 low)
{
    thermal_range_high = high ;
    thermal_range_low = low ;
    thermal_critical = critical ;

    osPrintf("thermal_range_high = %d \n", thermal_range_high);
    osPrintf("thermal_range_low = %d \n", thermal_range_low);
    osPrintf("thermal_critical = %d \n", thermal_critical);

    return GT_OK  ;
}

/**
 * falcon_RD_TurnOnThermalDebug function
 *
 *
 * @param en
 *
 * @return GT_STATUS
 */
GT_STATUS  falcon_RD_TurnOnThermalDebug(GT_BOOL en)
{
    cpss_thermal_debug = en ;
    return GT_OK  ;
}

/**
 * falcon_RD_GetSlotBitmap function
 *
 *
 * @return GT_U32
 */
static GT_U32 falcon_RD_GetSlotBitmap()
{
    GT_U32 value;
    GT_U8 slaveAddr = 0x76;
    GT_U8 regAddr = 0x3;
    GT_STATUS rc = GT_OK;

    rc = appDemoHostI2cRead(0,slaveAddr,1/*offset type: 8 bit*/,regAddr,1/*size: 1 Byte*/,(GT_U8*)&value);
    if (rc != GT_OK)
    {
        return rc;
    }


    return value ;
}

/**
 * falcon_RD_EnableThermalTask function
 *
 *
 * @param en
 *
 * @return GT_STATUS
 */
GT_STATUS falcon_RD_EnableThermalTask(GT_U32 en)
{
    cpss_rd_falcon_thermal_en = en ;
    return GT_OK ;

}

/**
 * falcon_RD_SetFanSpeed function
 *
 *
 * @param value
 */
static GT_STATUS falcon_RD_SetFanSpeed( GT_U32 value)
{
    GT_STATUS rc = GT_OK;
    /* Select CPLD */
    rc = appDemoHostI2cWrite(0/*bus id*/,0x76/*slave address*/,1/*offset_type: 8bit*/,0xd/*offset*/,1/*number of args*/,0x80/*data*/,0,0,0);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Select slave device */
    rc = appDemoHostI2cWrite(0/*bus id*/,0x76/*slave address*/,1/*offset_type: 8bit*/,0xe/*offset*/,1/*number of args*/,0x83/*data*/,0,0,0);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Set fan speed */
    rc = appDemoHostI2cWrite(0/*bus id*/,0x47/*slave address*/,1/*offset_type: 8bit*/,0x21/*offset*/,1/*number of args*/,value/*data*/,0,0,0);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(cpss_thermal_debug)
    {
        osPrintf("value =%d\n", value) ;
    }

    /* Select MAC */
    rc = appDemoHostI2cWrite(0/*bus id*/,0x76/*slave address*/,1/*offset_type: 8bit*/,0xd/*offset*/,1/*number of args*/,0/*data*/,0,0,0);
    if (rc != GT_OK)
    {
        return rc;
    }


    return GT_OK;
}

/**
 * falcon_RD_ThermalTask function
 *
 *
 * @param param
 *
 * @return unsigned __TASKCONV
 */
static unsigned __TASKCONV falcon_RD_ThermalTask
(
    GT_VOID * param
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 i, j;

    GT_UNUSED_PARAM(param);

    GT_32  thermal, phy_thermal , ic_thermal;
    GT_U32 fan_duty = 50 ;
    THERMAL_STATE  ic_state  ;
    GT_U32 slot_bitmap ;

    /*do once when system is up */
     slot_bitmap = falcon_RD_GetSlotBitmap();
     osPrintf("slot_bitmap =%x \n", slot_bitmap) ;

    while (1)
    {
        phy_thermal = 0 ;

        if(cpss_rd_falcon_thermal_en == 0)
        {
           sleep(30);
           continue ;
        }

        for(j = 0 ; j < 8 ; j++)
        {
            if((slot_bitmap & (0x1<<j)) == 0)
            {
                continue;
            }

            for(i=0; i < 16 ; i++)
            {
                thermal  = 0;
#if !defined(ASIC_SIMULATION) && defined(INCLUDE_MPD)
                rc = falcon_phy_GetThermal(0, j*16 + i + 128  , &thermal);
#endif
                if(cpss_thermal_debug)
                {
                    osPrintf("port =%d , thermal =%d \n", j*16 + i + 128 , thermal);
                }
                if(thermal > phy_thermal)
                {
                    phy_thermal = thermal;
                }
            }
        }

        cpssDxChDiagDeviceTemperatureGet(0, &ic_thermal);

        if(cpss_thermal_debug)
        {
            osPrintf(" ic_thermal =%d \n", ic_thermal);
        }

        ic_state = falcon_RD_IcStateGet(ic_thermal);


        if(cpss_thermal_debug)
        {
            osPrintf(" ic_state =%d \n", ic_state);
        }

        switch(ic_state)
        {
        case THERMAL_CRITICAL :
            falcon_RD_SetFanSpeed(100);
            fan_duty = 100;
            break ;

        case THERMAL_HIGH :
            if(fan_duty <= 95)
            {
                fan_duty += 5 ;
            }
            falcon_RD_SetFanSpeed(fan_duty);
            break ;

        case  THERMAL_NORMAL:
            break ;

        case  THERMAL_LOW:
            if(fan_duty >= 25)
            {
                fan_duty -= 5 ;
            }
            falcon_RD_SetFanSpeed(fan_duty);
            break ;
        }
        sleep(30);
    }

    return rc ;
}

/**
 * falcon_RD_CreateThermalTask function
 *
 *
 * @return GT_STATUS
 */
GT_STATUS  falcon_RD_CreateThermalTask()
{
    GT_STATUS rc ;

    rc = osTaskCreate("cpss_rd_falcon_thermal_task",
                     200,
                     _8KB,
                     falcon_RD_ThermalTask,
                     &cpss_rd_falcon_thermal_queue_id,
                     &cpss_rd_falcon_thermal_task_id );

    if(rc != GT_OK)
    {
        osPrintf("spawn cpss_rd_falcon_thermal_task fail \n");

    }

    return 0 ;
}

/**
 * @internal falcon_RD_XSmiSelectCard
 * @endinternal
 *
 * @param hostDevNum
 * @param portGroupId
 *
 * @return GT_STATUS
 */
GT_STATUS falcon_RD_XSmiSelectCard
(
     GT_U8    hostDevNum,
     GT_U32   portGroupId
)
{
    GT_STATUS rc;
    GT_U32 output_en;
    GT_U32 ouput_data;

    rc = cpssDrvPpHwRegisterRead(hostDevNum, portGroupId, 0x1e018104, &output_en);
    if(rc != GT_OK)
    {
        return rc ;
    }

    output_en &= 0xFC03FFFF;
    cpssDrvPpHwRegisterWrite(hostDevNum, portGroupId, 0x1e018104, output_en);

    rc = cpssDrvPpHwRegisterRead(hostDevNum, portGroupId, 0x1e018100, &ouput_data);
    if(rc != GT_OK)
    {
        return rc ;
    }
    ouput_data &= 0xFC3FFFFF;
    ouput_data |= (portGroupId << 22);

    rc = cpssDrvPpHwRegisterWrite(hostDevNum, portGroupId, 0x1e018100, ouput_data);
    return GT_OK ;
}

#endif /*ASIC_SIMULATION*/



