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
* @file gtDbDxFalcon.c
*
* @brief Initialization functions for the Falcon - SIP6 - board.
*
* @version   2
********************************************************************************
*/
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>
#include <appDemo/userExit/userEventHandler.h>
#include <gtExtDrv/drivers/gtUartDrv.h>
#include <appDemo/userExit/dxCh/appDemoDxChEventHandle.h>

#ifdef INCLUDE_UTF
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#endif /* INCLUDE_UTF */
#include <gtOs/gtOsExc.h>

#include <appDemo/boardConfig/gtbDbDxFalconSpeedControl.h>
#include <appDemo/boardConfig/gtDbDxFalcon.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNstPortIsolation.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpssDriver/pp/hardware/private/prvCpssDriverCreate.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgL2Dlb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define APPDEMO_FALCON_2T4T_MAC_2_SD_MAP_0 {{0,1,2,3,4,5,6,7}}
#define APPDEMO_FALCON_2T4T_MAC_NOT_EXISTS {{0,1,2,3,4,5,6,7}}
#define APPDEMO_FALCON_2T4T_MAC_2_SD_MAP_1 {{7,6,5,4,3,2,1,0}}

GT_BOOL                            appDemoLpmRamConfigInfoSet=GT_FALSE;
CPSS_DXCH_LPM_RAM_CONFIG_INFO_STC  appDemoLpmRamConfigInfo[CPSS_DXCH_CFG_NUM_OF_DEV_TYPES_MANAGED_CNS];
GT_U32                             appDemoLpmRamConfigInfoNumOfElements;

#ifndef ASIC_SIMULATION

#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>

#ifdef INCLUDE_MPD
static GT_U32 mpdIfIndexArr[512];
CPSS_OS_MUTEX  xsmi_extn_mtx;

#endif

#define INIT_ALL_PHYS                   0xff
#define NUM_OF_LANES_PER_PHY            16
#define GET_PHY_INDEX_FROM_GROUP_ID_MAC(portGroup,phyIndex) \
        if(falcon_PhyInfo !=NULL) {                                                \
            for (phyIndex = 0; phyIndex < falcon_PhyInfo_arrSize  ; phyIndex++)    \
                { if (portGroup == falcon_PhyInfo[phyIndex].portGroupId) break; } \
            if (phyIndex == falcon_PhyInfo_arrSize) phyIndex = SKIP_PHY; }

/*typedef GT_U32 ARR_OF_PHY_LANES[NUM_OF_LANES_PER_PHY];*/
typedef struct {
    GT_U32  lanes[NUM_OF_LANES_PER_PHY];
}ARR_OF_PHY_LANES;
GT_U32              falcon_RavenLaneToPhyLaneMap_arrSize;
ARR_OF_PHY_LANES*   falcon_RavenLaneToPhyLaneMap = NULL;
static ARR_OF_PHY_LANES    falcon_DB_RavenLaneToPhyLaneMap[] =
{/* sorted by Raven lane (serdes) number*/
    {{1,3,0,7,5,4,2,6,15,13,12,10,14,11,8,9}},/* Raven0 */
    {{0,1,5,2,3,7,4,6,15,14,13,12,11,10,9,8}},/* Raven1 */
    {{1,0,3,2,5,4,7,6,13,14,15,10,12,9,8,11}},/* Raven14 */
    {{0,2,1,5,4,3,6,7,15,13,14,12,11,9,10,8}} /* Raven15 */
};

/*typedef APPDEMO_SERDES_LANE_POLARITY_STC ARR_OF_PHY_LANE_POLARITY[NUM_OF_LANES_PER_PHY];*/
typedef struct {
    APPDEMO_SERDES_LANE_POLARITY_STC lanesPolarity[NUM_OF_LANES_PER_PHY];
}ARR_OF_PHY_LANE_POLARITY;
GT_U32                          falcon_PhyGearboxHostSidePolarityArray_arrSize;
ARR_OF_PHY_LANE_POLARITY*       falcon_PhyGearboxHostSidePolarityArray = NULL;
static ARR_OF_PHY_LANE_POLARITY falcon_DB_PhyGearboxHostSidePolarityArray[] =
{
    {{/* Phy0 - Host Side
        laneNum invertTx   invertRx */
        {0,    GT_TRUE,    GT_FALSE},
        {1,    GT_TRUE,    GT_FALSE},
        {2,    GT_FALSE,   GT_FALSE},
        {3,    GT_FALSE,   GT_FALSE},
        {4,    GT_FALSE,   GT_FALSE},
        {5,    GT_FALSE,   GT_FALSE},
        {6,    GT_FALSE,   GT_FALSE},
        {7,    GT_FALSE,   GT_FALSE},
        {8,    GT_FALSE,   GT_FALSE},
        {9,    GT_FALSE,   GT_FALSE},
        {10,   GT_FALSE,   GT_FALSE},
        {11,   GT_FALSE,   GT_FALSE},
        {12,   GT_FALSE,   GT_FALSE},
        {13,   GT_FALSE,   GT_TRUE },
        {14,   GT_FALSE,   GT_FALSE},
        {15,   GT_FALSE,   GT_FALSE}
    }},
    {{/* Phy1 - Host Side
        laneNum invertTx  invertRx */
        {0,    GT_TRUE,    GT_FALSE},
        {1,    GT_TRUE,    GT_FALSE},
        {2,    GT_FALSE,   GT_FALSE},
        {3,    GT_FALSE,   GT_FALSE},
        {4,    GT_FALSE,   GT_FALSE},
        {5,    GT_FALSE,   GT_FALSE},
        {6,    GT_FALSE,   GT_FALSE},
        {7,    GT_FALSE,   GT_FALSE},
        {8,    GT_FALSE,   GT_FALSE},
        {9,    GT_FALSE,   GT_FALSE},
        {10,   GT_FALSE,   GT_FALSE},
        {11,   GT_FALSE,   GT_FALSE},
        {12,   GT_FALSE,   GT_FALSE},
        {13,   GT_FALSE,   GT_FALSE},
        {14,   GT_FALSE,   GT_FALSE},
        {15,   GT_FALSE,   GT_FALSE}

    }},
    {{/* Phy14 - Host Side
        laneNum invertTx   invertRx */
        {0,    GT_TRUE,    GT_FALSE},
        {1,    GT_TRUE,    GT_FALSE},
        {2,    GT_FALSE,   GT_FALSE},
        {3,    GT_TRUE,    GT_FALSE},
        {4,    GT_FALSE,   GT_FALSE},
        {5,    GT_TRUE,    GT_FALSE},
        {6,    GT_FALSE,   GT_FALSE},
        {7,    GT_TRUE,    GT_FALSE},
        {8,    GT_FALSE,   GT_FALSE},
        {9,    GT_FALSE,   GT_FALSE},
        {10,   GT_FALSE,   GT_FALSE},
        {11,   GT_FALSE,   GT_FALSE},
        {12,   GT_FALSE,   GT_FALSE},
        {13,   GT_FALSE,   GT_TRUE },
        {14,   GT_FALSE,   GT_FALSE},
        {15,   GT_FALSE,   GT_FALSE}

    }},
    {{/* Phy15 - Host Side
        laneNum invertTx   invertRx */
        {0,    GT_TRUE,    GT_FALSE},
        {1,    GT_TRUE,    GT_FALSE},
        {2,    GT_TRUE,    GT_FALSE},
        {3,    GT_TRUE,    GT_FALSE},
        {4,    GT_TRUE,    GT_FALSE},
        {5,    GT_TRUE,    GT_FALSE},
        {6,    GT_TRUE,    GT_FALSE},
        {7,    GT_TRUE,    GT_FALSE},
        {8,    GT_TRUE,    GT_FALSE},
        {9,    GT_TRUE,    GT_FALSE},
        {10,   GT_TRUE,    GT_FALSE},
        {11,   GT_TRUE,    GT_FALSE},
        {12,   GT_TRUE,    GT_FALSE},
        {13,   GT_TRUE,    GT_TRUE },
        {14,   GT_TRUE,    GT_FALSE},
        {15,   GT_FALSE,   GT_FALSE}
    }}
};

GT_U32                          falcon_PhyGearboxLineSidePolarityArray_arrSize;
ARR_OF_PHY_LANE_POLARITY*       falcon_PhyGearboxLineSidePolarityArray = NULL;
static ARR_OF_PHY_LANE_POLARITY falcon_DB_PhyGearboxLineSidePolarityArray[] =
{
    {{/* Phy0 - Line Side
        laneNum invertTx   invertRx */
        {0,    GT_FALSE,   GT_TRUE },
        {1,    GT_FALSE,   GT_TRUE },
        {2,    GT_FALSE,   GT_FALSE},
        {3,    GT_FALSE,   GT_FALSE},
        {4,    GT_FALSE,   GT_FALSE},
        {5,    GT_FALSE,   GT_FALSE},
        {6,    GT_FALSE,   GT_FALSE},
        {7,    GT_FALSE,   GT_FALSE},
        {8,    GT_FALSE,   GT_FALSE},
        {9,    GT_FALSE,   GT_FALSE},
        {10,   GT_FALSE,   GT_FALSE},
        {11,   GT_FALSE,   GT_FALSE},
        {12,   GT_FALSE,   GT_FALSE},
        {13,   GT_TRUE,    GT_FALSE},
        {14,   GT_FALSE,   GT_FALSE},
        {15,   GT_FALSE,   GT_FALSE}
    }},
    {{/* Phy1 - Line Side
        laneNum invertTx  invertRx */
        {0,    GT_FALSE,   GT_TRUE },
        {1,    GT_FALSE,   GT_TRUE },
        {2,    GT_FALSE,   GT_FALSE},
        {3,    GT_FALSE,   GT_FALSE},
        {4,    GT_FALSE,   GT_FALSE},
        {5,    GT_FALSE,   GT_FALSE},
        {6,    GT_FALSE,   GT_FALSE},
        {7,    GT_FALSE,   GT_FALSE},
        {8,    GT_FALSE,   GT_FALSE},
        {9,    GT_FALSE,   GT_FALSE},
        {10,   GT_FALSE,   GT_FALSE},
        {11,   GT_FALSE,   GT_FALSE},
        {12,   GT_FALSE,   GT_FALSE},
        {13,   GT_FALSE,   GT_FALSE},
        {14,   GT_FALSE,   GT_FALSE},
        {15,   GT_FALSE,   GT_FALSE}
    }},
    {{/* Phy14 - Line Side
        laneNum invertTx   invertRx */
        {0,    GT_FALSE,   GT_TRUE },
        {1,    GT_FALSE,   GT_TRUE },
        {2,    GT_FALSE,   GT_FALSE},
        {3,    GT_FALSE,   GT_TRUE },
        {4,    GT_FALSE,   GT_FALSE},
        {5,    GT_FALSE,   GT_TRUE },
        {6,    GT_FALSE,   GT_FALSE},
        {7,    GT_FALSE,   GT_TRUE },
        {8,    GT_FALSE,   GT_FALSE},
        {9,    GT_FALSE,   GT_FALSE},
        {10,   GT_FALSE,   GT_FALSE},
        {11,   GT_FALSE,   GT_FALSE},
        {12,   GT_FALSE,   GT_FALSE},
        {13,   GT_TRUE,    GT_FALSE},
        {14,   GT_FALSE,   GT_FALSE},
        {15,   GT_FALSE,   GT_FALSE}
    }},
    {{/* Phy15 - Line Side
        laneNum invertTx   invertRx */
        {0,    GT_FALSE,   GT_TRUE },
        {1,    GT_FALSE,   GT_TRUE },
        {2,    GT_FALSE,   GT_TRUE },
        {3,    GT_FALSE,   GT_TRUE },
        {4,    GT_FALSE,   GT_TRUE },
        {5,    GT_FALSE,   GT_TRUE },
        {6,    GT_FALSE,   GT_TRUE },
        {7,    GT_FALSE,   GT_TRUE },
        {8,    GT_FALSE,   GT_TRUE },
        {9,    GT_FALSE,   GT_TRUE },
        {10,   GT_FALSE,   GT_TRUE },
        {11,   GT_FALSE,   GT_TRUE },
        {12,   GT_FALSE,   GT_TRUE },
        {13,   GT_TRUE,    GT_TRUE },
        {14,   GT_FALSE,   GT_TRUE },
        {15,   GT_FALSE,   GT_FALSE}
    }}
};

GT_U32                              falcon_PhyTuneParams_arrSize;
CPSS_PORT_SERDES_TUNE_STC*          falcon_PhyTuneParams = NULL;
CPSS_PORT_SERDES_TUNE_STC           falcon_DB_PhyTuneParams[GT_APPDEMO_XPHY_BOTH_SIDES] =
{
    {CPSS_PORT_SERDES_AVAGO_E, {.avago={0, 4, 0, 0, 0}}, {.avago={0, 0, 5, 0, 0, 0, {0}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {0}, 0, 0, 0}}},
    {CPSS_PORT_SERDES_AVAGO_E, {.avago={0, 4, 0, 0, 0}}, {.avago={0, 0, 5, 5, 5, 0, {0}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {0}, 0, 0, 0}}}
};

GT_U32                              falcon_PhyInfo_arrSize;
GT_APPDEMO_XPHY_INFO_STC*           falcon_PhyInfo = NULL;
GT_APPDEMO_XPHY_INFO_STC            falcon_DB_PhyInfo[] =
{
    {0, 0,  CPSS_PHY_XSMI_INTERFACE_1_E,  0, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {1, 1,  CPSS_PHY_XSMI_INTERFACE_1_E,  8, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {2, 14, CPSS_PHY_XSMI_INTERFACE_13_E, 0, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL},
    {3, 15, CPSS_PHY_XSMI_INTERFACE_13_E, 8, GT_APPDEMO_XPHY_88X7120, SKIP_PHY, NULL}
};

#endif /*ASIC_SIMULATION*/

GT_U32                              falcon_MacToSerdesMap_arrSize;
CPSS_PORT_MAC_TO_SERDES_STC*        falcon_MacToSerdesMap = NULL;
static CPSS_PORT_MAC_TO_SERDES_STC  falcon_DB_MacToSerdesMap[] =
{
   {{6,7,2,5,1,3,0,4}},  /* 0-7*/    /*Raven 0 */
   {{0,5,1,7,4,3,2,6}},  /* 8-15*/   /*Raven 0 */
   {{3,7,0,6,4,5,1,2}},  /*16-23*/   /*Raven 1 */
   {{1,5,3,7,0,4,2,6}},  /*24-31*/   /*Raven 1 */
   {{3,7,1,5,2,6,0,4}},  /*32-39*/   /*Raven 2 */
   {{2,4,7,5,1,3,0,6}},  /*40-47*/   /*Raven 2 */
   {{5,7,2,4,1,6,0,3}},  /*48-55*/   /*Raven 3 */
   {{0,6,3,7,2,4,1,5}},  /*56-63*/   /*Raven 3 */
   {{3,5,2,4,1,6,0,7}},  /*64-71*/   /*Raven 4 */
   {{3,5,1,7,4,0,2,6}},  /*72-79*/   /*Raven 4 */
   {{1,7,0,4,6,5,3,2}},  /*80-87*/   /*Raven 5 */
   {{1,5,3,7,0,4,2,6}},  /*88-95*/   /*Raven 5 */
   {{3,7,1,5,2,6,0,4}},  /*96-103*/  /*Raven 6 */
   {{2,4,7,5,1,3,0,6}},  /*104-111*/ /*Raven 6 */
   {{5,7,2,4,1,6,0,3}},  /*112-119*/ /*Raven 7 */
   {{0,6,3,7,2,4,1,5}},  /*120-127*/ /*Raven 7 */
   {{3,7,2,5,1,6,0,4}},  /*128-135*/ /*Raven 8 */
   {{0,5,1,7,4,3,2,6}},  /*136-143*/ /*Raven 8 */
   {{1,7,0,4,6,5,3,2}},  /*144-151*/ /*Raven 9 */
   {{1,5,3,7,0,4,2,6}},  /*152-159*/ /*Raven 9 */
   {{3,7,1,5,2,6,0,4}},  /*160-167*/ /*Raven 10*/
   {{2,4,7,5,1,3,0,6}},  /*168-175*/ /*Raven 10*/
   {{5,7,2,4,1,6,0,3}},  /*176-183*/ /*Raven 11*/
   {{0,6,3,7,2,4,1,5}},  /*184-191*/ /*Raven 11*/
   {{6,7,2,5,1,3,0,4}},  /*192-199*/ /*Raven 12*/
   {{0,5,4,7,1,3,2,6}},  /*200-207*/ /*Raven 12*/
   {{1,7,0,4,6,5,3,2}},  /*208-215*/ /*Raven 13*/
   {{1,5,3,7,0,4,2,6}},  /*216-223*/ /*Raven 13*/
   {{3,7,1,5,2,6,0,4}},  /*224-231*/ /*Raven 14*/
   {{2,7,0,5,1,3,4,6}},  /*232-239*/ /*Raven 14*/
   {{1,6,0,4,5,7,2,3}},  /*240-247*/ /*Raven 15*/
   {{0,4,1,5,2,6,3,7}}   /*248-255*/ /*Raven 15*/
};

static CPSS_PORT_MAC_TO_SERDES_STC  falcon_Belly2Belly_MacToSerdesMap[] =
{
   {{0,7,2,4,1,5,3,6}},  /* 0-7*/    /*Raven 0 */
   {{0,5,1,7,3,2,4,6}},  /* 8-15*/   /*Raven 0 */
   {{1,5,0,7,4,2,3,6}},  /*16-23*/   /*Raven 1 */
   {{0,6,1,7,2,3,4,5}},  /*24-31*/   /*Raven 1 */
   {{0,6,2,7,1,5,4,3}},  /*32-39*/   /*Raven 2 */
   {{0,5,1,6,2,7,4,3}},  /*40-47*/   /*Raven 2 */
   {{2,6,1,3,0,4,5,7}},  /*48-55*/   /*Raven 3 */
   {{1,6,2,7,0,4,3,5}},  /*56-63*/   /*Raven 3 */
   {{0,7,2,4,1,5,3,6}},  /*64-71*/   /*Raven 4 */
   {{0,5,1,7,3,2,4,6}},  /*72-79*/   /*Raven 4 */
   {{1,5,0,7,4,2,3,6}},  /*80-87*/   /*Raven 5 */
   {{0,6,1,7,2,3,4,5}},  /*88-95*/   /*Raven 5 */
   {{0,6,2,7,1,5,4,3}},  /*96-103*/  /*Raven 6 */
   {{0,5,1,6,2,7,4,3}},  /*104-111*/ /*Raven 6 */
   {{2,6,1,3,0,4,5,7}},  /*112-119*/ /*Raven 7 */
   {{1,6,2,7,0,4,3,5}},  /*120-127*/ /*Raven 7 */
   {{0,7,2,4,1,5,3,6}},  /*128-135*/ /*Raven 8 */
   {{0,5,1,7,3,2,4,6}},  /*136-143*/ /*Raven 8 */
   {{1,5,0,7,4,2,3,6}},  /*144-151*/ /*Raven 9 */
   {{0,6,1,7,2,3,4,5}},  /*152-159*/ /*Raven 9 */
   {{0,6,2,7,1,5,4,3}},  /*160-167*/ /*Raven 10*/
   {{0,5,1,6,2,7,4,3}},  /*168-175*/ /*Raven 10*/
   {{2,6,1,3,0,4,5,7}},  /*176-183*/ /*Raven 11*/
   {{1,6,2,7,0,4,3,5}},  /*184-191*/ /*Raven 11*/
   {{0,7,2,4,1,5,3,6}},  /*192-199*/ /*Raven 12*/
   {{0,5,1,7,3,2,4,6}},  /*200-207*/ /*Raven 12*/
   {{1,5,0,7,4,2,3,6}},  /*208-215*/ /*Raven 13*/
   {{0,6,1,7,2,3,4,5}},  /*216-223*/ /*Raven 13*/
   {{0,6,2,7,1,5,4,3}},  /*224-231*/ /*Raven 14*/
   {{0,5,1,6,2,7,4,3}},  /*232-239*/ /*Raven 14*/
   {{2,6,1,3,0,4,5,7}},  /*240-247*/ /*Raven 15*/
   {{1,6,2,7,0,4,3,5}},  /*248-255*/ /*Raven 15*/
};

static CPSS_PORT_MAC_TO_SERDES_STC  falcon_2T4T_MacToSerdesMap[] =
{
    APPDEMO_FALCON_2T4T_MAC_2_SD_MAP_0, /* Raven 0 MACs: 0-7      SDs: 0-7   */
    APPDEMO_FALCON_2T4T_MAC_2_SD_MAP_1, /* Raven 0 MACs: 8-15     SDs: 8-15  */
    APPDEMO_FALCON_2T4T_MAC_NOT_EXISTS,
    APPDEMO_FALCON_2T4T_MAC_NOT_EXISTS,
    APPDEMO_FALCON_2T4T_MAC_2_SD_MAP_0, /* Raven 2 MACs: 32-39    SDs: 16-23 */
    APPDEMO_FALCON_2T4T_MAC_2_SD_MAP_1, /* Raven 2 MACs: 40-47    SDs: 24-31 */
    APPDEMO_FALCON_2T4T_MAC_NOT_EXISTS,
    APPDEMO_FALCON_2T4T_MAC_NOT_EXISTS,
    {{0,3,1,2,5,7,4,6}},                /* Raven 4 MACs: 64-71    SDs: 32-39 */
    {{7,4,6,5,2,0,3,1}},                /* Raven 4 MACs: 72-79    SDs: 40-47 */
    {{0,3,1,2,5,7,4,6}},                /* Raven 5 MACs: 80-87    SDs: 48-55 */
    {{7,4,6,5,2,0,3,1}},                /* Raven 5 MACs: 88-95    SDs: 56-63 */
    APPDEMO_FALCON_2T4T_MAC_NOT_EXISTS,
    APPDEMO_FALCON_2T4T_MAC_NOT_EXISTS,
    APPDEMO_FALCON_2T4T_MAC_2_SD_MAP_0, /* Raven 7 MACs: 112-119  SDs: 64-71 */
    APPDEMO_FALCON_2T4T_MAC_2_SD_MAP_1, /* Raven 7 MACs: 120-127  SDs: 72-79 */
};

GT_U32                              falcon_PolarityArray_arrSize;
APPDEMO_SERDES_LANE_POLARITY_STC*   falcon_PolarityArray = NULL;
static APPDEMO_SERDES_LANE_POLARITY_STC  falcon_DB_PolarityArray[] =
{
/* laneNum  invertTx    invertRx */
    { 0,    GT_FALSE,    GT_TRUE },
    { 1,    GT_FALSE,   GT_FALSE },
    { 2,    GT_FALSE,   GT_FALSE },
    { 3,    GT_FALSE,   GT_FALSE },
    { 4,    GT_FALSE,   GT_FALSE },
    { 5,    GT_FALSE,   GT_FALSE },
    { 6,    GT_TRUE,    GT_FALSE },
    { 7,    GT_FALSE,   GT_FALSE },
    { 8,    GT_FALSE,   GT_FALSE },
    { 9,    GT_TRUE,    GT_FALSE },
    { 10,   GT_FALSE,   GT_FALSE },
    { 11,   GT_FALSE,   GT_FALSE },
    { 12,   GT_FALSE,   GT_TRUE  },
    { 13,   GT_FALSE,   GT_FALSE },
    { 14,   GT_FALSE,   GT_FALSE },
    { 15,   GT_FALSE,   GT_FALSE },
    { 16,   GT_FALSE,   GT_TRUE  },
    { 17,   GT_FALSE,   GT_TRUE  },
    { 18,   GT_FALSE,   GT_FALSE },
    { 19,   GT_FALSE,   GT_FALSE },
    { 20,   GT_FALSE,   GT_FALSE },
    { 21,   GT_FALSE,   GT_FALSE },
    { 22,   GT_FALSE,   GT_FALSE },
    { 23,   GT_FALSE,   GT_FALSE },
    { 24,   GT_FALSE,   GT_TRUE  },
    { 25,   GT_FALSE,   GT_TRUE  },
    { 26,   GT_FALSE,   GT_TRUE  },
    { 27,   GT_FALSE,   GT_FALSE },
    { 28,   GT_FALSE,   GT_FALSE },
    { 29,   GT_FALSE,   GT_FALSE },
    { 30,   GT_FALSE,   GT_FALSE },
    { 31,   GT_FALSE,   GT_FALSE },
    { 32,   GT_FALSE,   GT_TRUE  },
    { 33,   GT_FALSE,   GT_FALSE },
    { 34,   GT_FALSE,   GT_FALSE },
    { 35,   GT_FALSE,   GT_FALSE },
    { 36,   GT_FALSE,   GT_FALSE },
    { 37,   GT_FALSE,   GT_FALSE },
    { 38,   GT_FALSE,   GT_FALSE },
    { 39,   GT_FALSE,   GT_FALSE },
    { 40,   GT_FALSE,   GT_FALSE },
    { 41,   GT_FALSE,   GT_FALSE },
    { 42,   GT_FALSE,   GT_FALSE },
    { 43,   GT_FALSE,   GT_FALSE },
    { 44,   GT_FALSE,   GT_FALSE },
    { 45,   GT_FALSE,   GT_FALSE },
    { 46,   GT_FALSE,   GT_FALSE },
    { 47,   GT_FALSE,   GT_TRUE  },
    { 48,   GT_FALSE,   GT_FALSE },
    { 49,   GT_FALSE,   GT_FALSE },
    { 50,   GT_FALSE,   GT_FALSE },
    { 51,   GT_FALSE,   GT_FALSE },
    { 52,   GT_FALSE,   GT_FALSE },
    { 53,   GT_FALSE,   GT_FALSE },
    { 54,   GT_FALSE,   GT_FALSE },
    { 55,   GT_FALSE,   GT_FALSE },
    { 56,   GT_FALSE,   GT_FALSE },
    { 57,   GT_FALSE,   GT_FALSE },
    { 58,   GT_FALSE,   GT_FALSE },
    { 59,   GT_FALSE,   GT_FALSE },
    { 60,   GT_FALSE,   GT_FALSE },
    { 61,   GT_FALSE,   GT_FALSE },
    { 62,   GT_FALSE,   GT_FALSE },
    { 63,   GT_FALSE,   GT_FALSE },
    { 64,   GT_FALSE,   GT_FALSE },
    { 65,   GT_FALSE,   GT_TRUE  },
    { 66,   GT_FALSE,   GT_TRUE  },
    { 67,   GT_FALSE,   GT_TRUE  },
    { 68,   GT_FALSE,   GT_TRUE  },
    { 69,   GT_FALSE,   GT_FALSE },
    { 70,   GT_FALSE,   GT_FALSE },
    { 71,   GT_FALSE,   GT_TRUE  },
    { 72,   GT_FALSE,   GT_TRUE  },
    { 73,   GT_FALSE,   GT_FALSE },
    { 74,   GT_FALSE,   GT_TRUE  },
    { 75,   GT_FALSE,   GT_FALSE },
    { 76,   GT_FALSE,   GT_TRUE  },
    { 77,   GT_FALSE,   GT_TRUE  },
    { 78,   GT_FALSE,   GT_TRUE  },
    { 79,   GT_FALSE,   GT_TRUE  },
    { 80,   GT_FALSE,   GT_TRUE  },
    { 81,   GT_FALSE,   GT_TRUE  },
    { 82,   GT_FALSE,   GT_FALSE },
    { 83,   GT_FALSE,   GT_FALSE },
    { 84,   GT_FALSE,   GT_TRUE  },
    { 85,   GT_FALSE,   GT_FALSE },
    { 86,   GT_FALSE,   GT_FALSE },
    { 87,   GT_FALSE,   GT_TRUE  },
    { 88,   GT_FALSE,   GT_TRUE  },
    { 89,   GT_FALSE,   GT_FALSE },
    { 90,   GT_FALSE,   GT_TRUE  },
    { 91,   GT_FALSE,   GT_FALSE },
    { 92,   GT_FALSE,   GT_TRUE  },
    { 93,   GT_FALSE,   GT_FALSE },
    { 94,   GT_FALSE,   GT_TRUE  },
    { 95,   GT_FALSE,   GT_FALSE },
    { 96,   GT_FALSE,   GT_TRUE  },
    { 97,   GT_FALSE,   GT_TRUE  },
    { 98,   GT_FALSE,   GT_TRUE  },
    { 99,   GT_FALSE,   GT_TRUE  },
    { 100,  GT_FALSE,   GT_TRUE  },
    { 101,  GT_FALSE,   GT_TRUE  },
    { 102,  GT_FALSE,   GT_FALSE },
    { 103,  GT_FALSE,   GT_TRUE  },
    { 104,  GT_FALSE,   GT_TRUE  },
    { 105,  GT_FALSE,   GT_FALSE },
    { 106,  GT_FALSE,   GT_TRUE  },
    { 107,  GT_FALSE,   GT_TRUE  },
    { 108,  GT_FALSE,   GT_TRUE  },
    { 109,  GT_FALSE,   GT_TRUE  },
    { 110,  GT_FALSE,   GT_TRUE  },
    { 111,  GT_FALSE,   GT_FALSE },
    { 112,  GT_FALSE,   GT_TRUE  },
    { 113,  GT_FALSE,   GT_TRUE  },
    { 114,  GT_FALSE,   GT_TRUE  },
    { 115,  GT_FALSE,   GT_TRUE  },
    { 116,  GT_FALSE,   GT_TRUE  },
    { 117,  GT_FALSE,   GT_TRUE  },
    { 118,  GT_FALSE,   GT_TRUE  },
    { 119,  GT_FALSE,   GT_TRUE  },
    { 120,  GT_FALSE,   GT_TRUE  },
    { 121,  GT_FALSE,   GT_TRUE  },
    { 122,  GT_FALSE,   GT_TRUE  },
    { 123,  GT_FALSE,   GT_TRUE  },
    { 124,  GT_FALSE,   GT_TRUE  },
    { 125,  GT_FALSE,   GT_FALSE },
    { 126,  GT_FALSE,   GT_TRUE  },
    { 127,  GT_FALSE,   GT_FALSE },
    { 128,  GT_FALSE,   GT_FALSE },
    { 129,  GT_FALSE,   GT_TRUE  },
    { 130,  GT_FALSE,   GT_TRUE  },
    { 131,  GT_FALSE,   GT_TRUE  },
    { 132,  GT_FALSE,   GT_TRUE  },
    { 133,  GT_FALSE,   GT_TRUE  },
    { 134,  GT_FALSE,   GT_TRUE  },
    { 135,  GT_FALSE,   GT_FALSE },
    { 136,  GT_FALSE,   GT_TRUE  },
    { 137,  GT_FALSE,   GT_TRUE  },
    { 138,  GT_FALSE,   GT_FALSE },
    { 139,  GT_FALSE,   GT_TRUE  },
    { 140,  GT_FALSE,   GT_TRUE  },
    { 141,  GT_FALSE,   GT_TRUE  },
    { 142,  GT_FALSE,   GT_TRUE  },
    { 143,  GT_FALSE,   GT_TRUE  },
    { 144,  GT_FALSE,   GT_TRUE  },
    { 145,  GT_FALSE,   GT_TRUE  },
    { 146,  GT_FALSE,   GT_FALSE },
    { 147,  GT_FALSE,   GT_TRUE  },
    { 148,  GT_FALSE,   GT_TRUE  },
    { 149,  GT_FALSE,   GT_FALSE },
    { 150,  GT_FALSE,   GT_TRUE  },
    { 151,  GT_FALSE,   GT_TRUE  },
    { 152,  GT_FALSE,   GT_TRUE  },
    { 153,  GT_FALSE,   GT_FALSE },
    { 154,  GT_FALSE,   GT_TRUE  },
    { 155,  GT_FALSE,   GT_FALSE },
    { 156,  GT_FALSE,   GT_TRUE  },
    { 157,  GT_FALSE,   GT_FALSE },
    { 158,  GT_FALSE,   GT_TRUE  },
    { 159,  GT_FALSE,   GT_FALSE },
    { 160,  GT_FALSE,   GT_TRUE  },
    { 161,  GT_FALSE,   GT_TRUE  },
    { 162,  GT_FALSE,   GT_TRUE  },
    { 163,  GT_FALSE,   GT_TRUE  },
    { 164,  GT_FALSE,   GT_TRUE  },
    { 165,  GT_FALSE,   GT_TRUE  },
    { 166,  GT_FALSE,   GT_FALSE },
    { 167,  GT_FALSE,   GT_TRUE  },
    { 168,  GT_FALSE,   GT_TRUE  },
    { 169,  GT_FALSE,   GT_FALSE },
    { 170,  GT_FALSE,   GT_TRUE  },
    { 171,  GT_FALSE,   GT_TRUE  },
    { 172,  GT_FALSE,   GT_TRUE  },
    { 173,  GT_FALSE,   GT_TRUE  },
    { 174,  GT_FALSE,   GT_TRUE  },
    { 175,  GT_FALSE,   GT_FALSE },
    { 176,  GT_FALSE,   GT_TRUE  },
    { 177,  GT_FALSE,   GT_TRUE  },
    { 178,  GT_FALSE,   GT_TRUE  },
    { 179,  GT_FALSE,   GT_TRUE  },
    { 180,  GT_FALSE,   GT_TRUE  },
    { 181,  GT_FALSE,   GT_TRUE  },
    { 182,  GT_FALSE,   GT_TRUE  },
    { 183,  GT_FALSE,   GT_TRUE  },
    { 184,  GT_FALSE,   GT_TRUE  },
    { 185,  GT_FALSE,   GT_TRUE  },
    { 186,  GT_FALSE,   GT_TRUE  },
    { 187,  GT_FALSE,   GT_TRUE  },
    { 188,  GT_FALSE,   GT_TRUE  },
    { 189,  GT_FALSE,   GT_TRUE  },
    { 190,  GT_FALSE,   GT_TRUE  },
    { 191,  GT_FALSE,   GT_FALSE },
    { 192,  GT_FALSE,   GT_FALSE },
    { 193,  GT_FALSE,   GT_FALSE },
    { 194,  GT_FALSE,   GT_TRUE  },
    { 195,  GT_FALSE,   GT_FALSE },
    { 196,  GT_FALSE,   GT_FALSE },
    { 197,  GT_FALSE,   GT_FALSE },
    { 198,  GT_FALSE,   GT_FALSE },
    { 199,  GT_FALSE,   GT_FALSE },
    { 200,  GT_FALSE,   GT_TRUE  },
    { 201,  GT_FALSE,   GT_TRUE  },
    { 202,  GT_FALSE,   GT_FALSE },
    { 203,  GT_FALSE,   GT_FALSE },
    { 204,  GT_FALSE,   GT_TRUE  },
    { 205,  GT_FALSE,   GT_FALSE },
    { 206,  GT_FALSE,   GT_FALSE },
    { 207,  GT_FALSE,   GT_FALSE },
    { 208,  GT_FALSE,   GT_FALSE },
    { 209,  GT_FALSE,   GT_TRUE  },
    { 210,  GT_FALSE,   GT_FALSE },
    { 211,  GT_FALSE,   GT_FALSE },
    { 212,  GT_FALSE,   GT_TRUE  },
    { 213,  GT_FALSE,   GT_FALSE },
    { 214,  GT_FALSE,   GT_FALSE },
    { 215,  GT_FALSE,   GT_FALSE },
    { 216,  GT_FALSE,   GT_TRUE  },
    { 217,  GT_FALSE,   GT_TRUE  },
    { 218,  GT_FALSE,   GT_TRUE  },
    { 219,  GT_FALSE,   GT_TRUE  },
    { 220,  GT_FALSE,   GT_FALSE },
    { 221,  GT_FALSE,   GT_FALSE },
    { 222,  GT_FALSE,   GT_FALSE },
    { 223,  GT_FALSE,   GT_FALSE },
    { 224,  GT_FALSE,   GT_FALSE },
    { 225,  GT_FALSE,   GT_TRUE  },
    { 226,  GT_FALSE,   GT_FALSE },
    { 227,  GT_FALSE,   GT_TRUE  },
    { 228,  GT_FALSE,   GT_FALSE },
    { 229,  GT_FALSE,   GT_TRUE  },
    { 230,  GT_FALSE,   GT_TRUE  },
    { 231,  GT_FALSE,   GT_FALSE },
    { 232,  GT_TRUE,    GT_TRUE  },
    { 233,  GT_FALSE,   GT_TRUE  },
    { 234,  GT_FALSE,   GT_FALSE },
    { 235,  GT_FALSE,   GT_FALSE },
    { 236,  GT_FALSE,   GT_FALSE },
    { 237,  GT_FALSE,   GT_FALSE },
    { 238,  GT_FALSE,   GT_FALSE },
    { 239,  GT_FALSE,   GT_TRUE  },
    { 240,  GT_FALSE,   GT_TRUE  },
    { 241,  GT_FALSE,   GT_TRUE  },
    { 242,  GT_FALSE,   GT_TRUE  },
    { 243,  GT_FALSE,   GT_TRUE  },
    { 244,  GT_FALSE,   GT_FALSE },
    { 245,  GT_FALSE,   GT_FALSE },
    { 246,  GT_FALSE,   GT_TRUE  },
    { 247,  GT_FALSE,   GT_TRUE  },
    { 248,  GT_FALSE,   GT_FALSE },
    { 249,  GT_TRUE,    GT_FALSE },
    { 250,  GT_FALSE,   GT_TRUE  },
    { 251,  GT_FALSE,   GT_TRUE  },
    { 252,  GT_FALSE,   GT_TRUE  },
    { 253,  GT_FALSE,   GT_FALSE },
    { 254,  GT_FALSE,   GT_TRUE  },
    { 255,  GT_FALSE,   GT_TRUE  },
    { 257,  GT_FALSE,   GT_TRUE  },
    { 258,  GT_TRUE,    GT_TRUE  }
};
static APPDEMO_SERDES_LANE_POLARITY_STC  falcon_DB_interposers_PolarityArray[] =
{
    { 0,    GT_TRUE,    GT_FALSE },
    { 1,    GT_FALSE,   GT_FALSE },
    { 2,    GT_TRUE,    GT_TRUE  },
    { 3,    GT_FALSE,   GT_FALSE },
    { 4,    GT_FALSE,   GT_FALSE },
    { 5,    GT_FALSE,   GT_FALSE },
    { 6,    GT_TRUE,    GT_FALSE },
    { 7,    GT_FALSE,   GT_FALSE },
    { 8,    GT_TRUE,    GT_TRUE  },
    { 9,    GT_TRUE,    GT_FALSE },
    { 10,   GT_FALSE,   GT_FALSE },
    { 11,   GT_FALSE,   GT_FALSE },
    { 12,   GT_TRUE,    GT_FALSE },
    { 13,   GT_FALSE,   GT_FALSE },
    { 14,   GT_FALSE,   GT_FALSE },
    { 15,   GT_FALSE,   GT_FALSE },
    { 16,   GT_TRUE,    GT_FALSE },
    { 17,   GT_TRUE,    GT_FALSE },
    { 18,   GT_FALSE,   GT_FALSE },
    { 19,   GT_FALSE,   GT_FALSE },
    { 20,   GT_FALSE,   GT_FALSE },
    { 21,   GT_FALSE,   GT_FALSE },
    { 22,   GT_FALSE,   GT_FALSE },
    { 23,   GT_FALSE,   GT_FALSE },
    { 24,   GT_TRUE,    GT_FALSE },
    { 25,   GT_TRUE,    GT_FALSE },
    { 26,   GT_FALSE,   GT_TRUE  },
    { 27,   GT_FALSE,   GT_FALSE },
    { 28,   GT_FALSE,   GT_FALSE },
    { 29,   GT_FALSE,   GT_FALSE },
    { 30,   GT_FALSE,   GT_FALSE },
    { 31,   GT_FALSE,   GT_FALSE },
    { 32,   GT_FALSE,   GT_TRUE  },
    { 33,   GT_FALSE,   GT_FALSE },
    { 34,   GT_FALSE,   GT_FALSE },
    { 35,   GT_FALSE,   GT_FALSE },
    { 36,   GT_FALSE,   GT_FALSE },
    { 37,   GT_FALSE,   GT_FALSE },
    { 38,   GT_FALSE,   GT_FALSE },
    { 39,   GT_FALSE,   GT_FALSE },
    { 40,   GT_FALSE,   GT_FALSE },
    { 41,   GT_FALSE,   GT_FALSE },
    { 42,   GT_FALSE,   GT_FALSE },
    { 43,   GT_FALSE,   GT_FALSE },
    { 44,   GT_FALSE,   GT_FALSE },
    { 45,   GT_FALSE,   GT_FALSE },
    { 46,   GT_FALSE,   GT_FALSE },
    { 47,   GT_FALSE,   GT_TRUE  },
    { 48,   GT_FALSE,   GT_FALSE },
    { 49,   GT_FALSE,   GT_FALSE },
    { 50,   GT_FALSE,   GT_FALSE },
    { 51,   GT_FALSE,   GT_FALSE },
    { 52,   GT_FALSE,   GT_FALSE },
    { 53,   GT_FALSE,   GT_FALSE },
    { 54,   GT_FALSE,   GT_FALSE },
    { 55,   GT_FALSE,   GT_FALSE },
    { 56,   GT_FALSE,   GT_FALSE },
    { 57,   GT_FALSE,   GT_FALSE },
    { 58,   GT_FALSE,   GT_FALSE },
    { 59,   GT_FALSE,   GT_FALSE },
    { 60,   GT_FALSE,   GT_FALSE },
    { 61,   GT_FALSE,   GT_FALSE },
    { 62,   GT_FALSE,   GT_FALSE },
    { 63,   GT_FALSE,   GT_FALSE },
    { 64,   GT_FALSE,   GT_FALSE },
    { 65,   GT_FALSE,   GT_TRUE  },
    { 66,   GT_FALSE,   GT_TRUE  },
    { 67,   GT_FALSE,   GT_TRUE  },
    { 68,   GT_FALSE,   GT_TRUE  },
    { 69,   GT_FALSE,   GT_FALSE },
    { 70,   GT_FALSE,   GT_FALSE },
    { 71,   GT_FALSE,   GT_TRUE  },
    { 72,   GT_FALSE,   GT_TRUE  },
    { 73,   GT_FALSE,   GT_FALSE },
    { 74,   GT_FALSE,   GT_TRUE  },
    { 75,   GT_FALSE,   GT_FALSE },
    { 76,   GT_FALSE,   GT_TRUE  },
    { 77,   GT_FALSE,   GT_TRUE  },
    { 78,   GT_FALSE,   GT_TRUE  },
    { 79,   GT_FALSE,   GT_TRUE  },
    { 80,   GT_FALSE,   GT_TRUE  },
    { 81,   GT_FALSE,   GT_TRUE  },
    { 82,   GT_FALSE,   GT_FALSE },
    { 83,   GT_FALSE,   GT_FALSE },
    { 84,   GT_FALSE,   GT_TRUE  },
    { 85,   GT_FALSE,   GT_FALSE },
    { 86,   GT_FALSE,   GT_FALSE },
    { 87,   GT_FALSE,   GT_TRUE  },
    { 88,   GT_FALSE,   GT_TRUE  },
    { 89,   GT_FALSE,   GT_FALSE },
    { 90,   GT_FALSE,   GT_TRUE  },
    { 91,   GT_FALSE,   GT_FALSE },
    { 92,   GT_FALSE,   GT_TRUE  },
    { 93,   GT_FALSE,   GT_FALSE },
    { 94,   GT_FALSE,   GT_TRUE  },
    { 95,   GT_FALSE,   GT_FALSE },
    { 96,   GT_FALSE,   GT_TRUE  },
    { 97,   GT_FALSE,   GT_TRUE  },
    { 98,   GT_FALSE,   GT_TRUE  },
    { 99,   GT_FALSE,   GT_TRUE  },
    { 100,  GT_FALSE,   GT_TRUE  },
    { 101,  GT_FALSE,   GT_TRUE  },
    { 102,  GT_FALSE,   GT_FALSE },
    { 103,  GT_FALSE,   GT_TRUE  },
    { 104,  GT_FALSE,   GT_TRUE  },
    { 105,  GT_FALSE,   GT_FALSE },
    { 106,  GT_FALSE,   GT_TRUE  },
    { 107,  GT_FALSE,   GT_TRUE  },
    { 108,  GT_FALSE,   GT_TRUE  },
    { 109,  GT_FALSE,   GT_TRUE  },
    { 110,  GT_FALSE,   GT_TRUE  },
    { 111,  GT_FALSE,   GT_FALSE },
    { 112,  GT_FALSE,   GT_TRUE  },
    { 113,  GT_FALSE,   GT_TRUE  },
    { 114,  GT_FALSE,   GT_TRUE  },
    { 115,  GT_FALSE,   GT_TRUE  },
    { 116,  GT_FALSE,   GT_TRUE  },
    { 117,  GT_FALSE,   GT_TRUE  },
    { 118,  GT_FALSE,   GT_TRUE  },
    { 119,  GT_FALSE,   GT_TRUE  },
    { 120,  GT_FALSE,   GT_TRUE  },
    { 121,  GT_FALSE,   GT_TRUE  },
    { 122,  GT_FALSE,   GT_TRUE  },
    { 123,  GT_FALSE,   GT_TRUE  },
    { 124,  GT_FALSE,   GT_TRUE  },
    { 125,  GT_FALSE,   GT_FALSE },
    { 126,  GT_FALSE,   GT_TRUE  },
    { 127,  GT_FALSE,   GT_FALSE },
    { 128,  GT_FALSE,   GT_FALSE },
    { 129,  GT_FALSE,   GT_TRUE  },
    { 130,  GT_FALSE,   GT_TRUE  },
    { 131,  GT_FALSE,   GT_TRUE  },
    { 132,  GT_FALSE,   GT_TRUE  },
    { 133,  GT_FALSE,   GT_TRUE  },
    { 134,  GT_FALSE,   GT_TRUE  },
    { 135,  GT_FALSE,   GT_FALSE },
    { 136,  GT_FALSE,   GT_TRUE  },
    { 137,  GT_FALSE,   GT_TRUE  },
    { 138,  GT_FALSE,   GT_FALSE },
    { 139,  GT_FALSE,   GT_TRUE  },
    { 140,  GT_FALSE,   GT_TRUE  },
    { 141,  GT_FALSE,   GT_TRUE  },
    { 142,  GT_FALSE,   GT_TRUE  },
    { 143,  GT_FALSE,   GT_TRUE  },
    { 144,  GT_FALSE,   GT_TRUE  },
    { 145,  GT_FALSE,   GT_TRUE  },
    { 146,  GT_FALSE,   GT_FALSE },
    { 147,  GT_FALSE,   GT_TRUE  },
    { 148,  GT_FALSE,   GT_TRUE  },
    { 149,  GT_FALSE,   GT_FALSE },
    { 150,  GT_FALSE,   GT_TRUE  },
    { 151,  GT_FALSE,   GT_TRUE  },
    { 152,  GT_FALSE,   GT_TRUE  },
    { 153,  GT_FALSE,   GT_FALSE },
    { 154,  GT_FALSE,   GT_TRUE  },
    { 155,  GT_FALSE,   GT_FALSE },
    { 156,  GT_FALSE,   GT_TRUE  },
    { 157,  GT_FALSE,   GT_FALSE },
    { 158,  GT_FALSE,   GT_TRUE  },
    { 159,  GT_FALSE,   GT_FALSE },
    { 160,  GT_FALSE,   GT_TRUE  },
    { 161,  GT_FALSE,   GT_TRUE  },
    { 162,  GT_FALSE,   GT_TRUE  },
    { 163,  GT_FALSE,   GT_TRUE  },
    { 164,  GT_FALSE,   GT_TRUE  },
    { 165,  GT_FALSE,   GT_TRUE  },
    { 166,  GT_FALSE,   GT_FALSE },
    { 167,  GT_FALSE,   GT_TRUE  },
    { 168,  GT_FALSE,   GT_TRUE  },
    { 169,  GT_FALSE,   GT_FALSE },
    { 170,  GT_FALSE,   GT_TRUE  },
    { 171,  GT_FALSE,   GT_TRUE  },
    { 172,  GT_FALSE,   GT_TRUE  },
    { 173,  GT_FALSE,   GT_TRUE  },
    { 174,  GT_FALSE,   GT_TRUE  },
    { 175,  GT_FALSE,   GT_FALSE },
    { 176,  GT_FALSE,   GT_TRUE  },
    { 177,  GT_FALSE,   GT_TRUE  },
    { 178,  GT_FALSE,   GT_TRUE  },
    { 179,  GT_FALSE,   GT_TRUE  },
    { 180,  GT_FALSE,   GT_TRUE  },
    { 181,  GT_FALSE,   GT_TRUE  },
    { 182,  GT_FALSE,   GT_TRUE  },
    { 183,  GT_FALSE,   GT_TRUE  },
    { 184,  GT_FALSE,   GT_TRUE  },
    { 185,  GT_FALSE,   GT_TRUE  },
    { 186,  GT_FALSE,   GT_TRUE  },
    { 187,  GT_FALSE,   GT_TRUE  },
    { 188,  GT_FALSE,   GT_TRUE  },
    { 189,  GT_FALSE,   GT_TRUE  },
    { 190,  GT_FALSE,   GT_TRUE  },
    { 191,  GT_FALSE,   GT_FALSE },
    { 192,  GT_FALSE,   GT_FALSE },
    { 193,  GT_FALSE,   GT_FALSE },
    { 194,  GT_FALSE,   GT_TRUE  },
    { 195,  GT_FALSE,   GT_FALSE },
    { 196,  GT_FALSE,   GT_FALSE },
    { 197,  GT_FALSE,   GT_FALSE },
    { 198,  GT_FALSE,   GT_FALSE },
    { 199,  GT_FALSE,   GT_FALSE },
    { 200,  GT_FALSE,   GT_TRUE  },
    { 201,  GT_FALSE,   GT_TRUE  },
    { 202,  GT_FALSE,   GT_FALSE },
    { 203,  GT_FALSE,   GT_FALSE },
    { 204,  GT_FALSE,   GT_TRUE  },
    { 205,  GT_FALSE,   GT_FALSE },
    { 206,  GT_FALSE,   GT_FALSE },
    { 207,  GT_FALSE,   GT_FALSE },
    { 208,  GT_FALSE,   GT_FALSE },
    { 209,  GT_FALSE,   GT_TRUE  },
    { 210,  GT_FALSE,   GT_FALSE },
    { 211,  GT_FALSE,   GT_FALSE },
    { 212,  GT_FALSE,   GT_TRUE  },
    { 213,  GT_FALSE,   GT_FALSE },
    { 214,  GT_FALSE,   GT_FALSE },
    { 215,  GT_FALSE,   GT_FALSE },
    { 216,  GT_FALSE,   GT_TRUE  },
    { 217,  GT_FALSE,   GT_TRUE  },
    { 218,  GT_FALSE,   GT_TRUE  },
    { 219,  GT_FALSE,   GT_TRUE  },
    { 220,  GT_FALSE,   GT_FALSE },
    { 221,  GT_FALSE,   GT_FALSE },
    { 222,  GT_FALSE,   GT_FALSE },
    { 223,  GT_FALSE,   GT_FALSE },
    { 224,  GT_TRUE,    GT_TRUE  },
    { 225,  GT_TRUE,    GT_FALSE },
    { 226,  GT_FALSE,   GT_FALSE },
    { 227,  GT_FALSE,   GT_TRUE  },
    { 228,  GT_FALSE,   GT_FALSE },
    { 229,  GT_FALSE,   GT_TRUE  },
    { 230,  GT_FALSE,   GT_TRUE  },
    { 231,  GT_FALSE,   GT_FALSE },
    { 232,  GT_TRUE,    GT_TRUE  },
    { 233,  GT_TRUE,    GT_FALSE },
    { 234,  GT_TRUE,    GT_TRUE  },
    { 235,  GT_FALSE,   GT_FALSE },
    { 236,  GT_FALSE,   GT_FALSE },
    { 237,  GT_FALSE,   GT_FALSE },
    { 238,  GT_FALSE,   GT_FALSE },
    { 239,  GT_FALSE,   GT_TRUE  },
    { 240,  GT_TRUE,    GT_FALSE },
    { 241,  GT_FALSE,   GT_TRUE  },
    { 242,  GT_TRUE,    GT_FALSE },
    { 243,  GT_FALSE,   GT_TRUE  },
    { 244,  GT_FALSE,   GT_FALSE },
    { 245,  GT_FALSE,   GT_FALSE },
    { 246,  GT_FALSE,   GT_TRUE  },
    { 247,  GT_FALSE,   GT_TRUE  },
    { 248,  GT_TRUE,    GT_TRUE  },
    { 249,  GT_TRUE,    GT_FALSE },
    { 250,  GT_TRUE,    GT_FALSE },
    { 251,  GT_FALSE,   GT_TRUE  },
    { 252,  GT_FALSE,   GT_TRUE  },
    { 253,  GT_FALSE,   GT_FALSE },
    { 254,  GT_FALSE,   GT_TRUE  },
    { 255,  GT_FALSE,   GT_TRUE  },
    { 257,  GT_FALSE,   GT_TRUE  },
    { 258,  GT_TRUE,    GT_TRUE  }
};

static APPDEMO_SERDES_LANE_POLARITY_STC  falcon_Belly2Belly_PolarityArray[] =
{
/* laneNum  invertTx    invertRx */
    { 0,    GT_TRUE ,   GT_FALSE },
    { 1,    GT_FALSE,   GT_FALSE },
    { 2,    GT_FALSE,   GT_TRUE  },
    { 3,    GT_FALSE,   GT_FALSE },
    { 4,    GT_FALSE,   GT_FALSE },
    { 5,    GT_FALSE,   GT_FALSE },
    { 6,    GT_FALSE,   GT_FALSE },
    { 7,    GT_FALSE,   GT_FALSE },
    { 8,    GT_FALSE,   GT_FALSE },
    { 9,    GT_FALSE,   GT_FALSE },
    { 10,   GT_FALSE,   GT_FALSE },
    { 11,   GT_FALSE,   GT_TRUE  },
    { 12,   GT_FALSE,   GT_FALSE },
    { 13,   GT_FALSE,   GT_FALSE },
    { 14,   GT_FALSE,   GT_FALSE },
    { 15,   GT_FALSE,   GT_FALSE },
    { 16,   GT_FALSE,   GT_FALSE },
    { 17,   GT_FALSE,   GT_FALSE },
    { 18,   GT_FALSE,   GT_FALSE },
    { 19,   GT_FALSE,   GT_FALSE },
    { 20,   GT_FALSE,   GT_FALSE },
    { 21,   GT_FALSE,   GT_FALSE },
    { 22,   GT_FALSE,   GT_FALSE },
    { 23,   GT_FALSE,   GT_FALSE },
    { 24,   GT_FALSE,   GT_TRUE  },
    { 25,   GT_FALSE,   GT_FALSE },
    { 26,   GT_FALSE,   GT_TRUE  },
    { 27,   GT_FALSE,   GT_FALSE },
    { 28,   GT_FALSE,   GT_TRUE  },
    { 29,   GT_FALSE,   GT_FALSE },
    { 30,   GT_FALSE,   GT_FALSE },
    { 31,   GT_FALSE,   GT_FALSE },
    { 32,   GT_FALSE,   GT_FALSE },
    { 33,   GT_FALSE,   GT_FALSE },
    { 34,   GT_FALSE,   GT_FALSE },
    { 35,   GT_FALSE,   GT_FALSE },
    { 36,   GT_FALSE,   GT_TRUE  },
    { 37,   GT_FALSE,   GT_FALSE },
    { 38,   GT_FALSE,   GT_FALSE },
    { 39,   GT_FALSE,   GT_FALSE },
    { 40,   GT_FALSE,   GT_FALSE },
    { 41,   GT_FALSE,   GT_TRUE  },
    { 42,   GT_FALSE,   GT_FALSE },
    { 43,   GT_FALSE,   GT_FALSE },
    { 44,   GT_FALSE,   GT_FALSE },
    { 45,   GT_FALSE,   GT_FALSE },
    { 46,   GT_FALSE,   GT_FALSE },
    { 47,   GT_FALSE,   GT_FALSE },
    { 48,   GT_FALSE,   GT_FALSE },
    { 49,   GT_FALSE,   GT_FALSE },
    { 50,   GT_FALSE,   GT_FALSE },
    { 51,   GT_FALSE,   GT_FALSE },
    { 52,   GT_FALSE,   GT_FALSE },
    { 53,   GT_FALSE,   GT_FALSE },
    { 54,   GT_FALSE,   GT_FALSE },
    { 55,   GT_FALSE,   GT_FALSE },
    { 56,   GT_FALSE,   GT_FALSE },
    { 57,   GT_FALSE,   GT_FALSE },
    { 58,   GT_FALSE,   GT_FALSE },
    { 59,   GT_FALSE,   GT_FALSE },
    { 60,   GT_FALSE,   GT_FALSE },
    { 61,   GT_FALSE,   GT_FALSE },
    { 62,   GT_FALSE,   GT_FALSE },
    { 63,   GT_FALSE,   GT_FALSE },
    { 64,   GT_FALSE,   GT_TRUE  },
    { 65,   GT_FALSE,   GT_TRUE  },
    { 66,   GT_FALSE,   GT_TRUE  },
    { 67,   GT_FALSE,   GT_FALSE },
    { 68,   GT_FALSE,   GT_TRUE  },
    { 69,   GT_FALSE,   GT_FALSE },
    { 70,   GT_FALSE,   GT_TRUE  },
    { 71,   GT_FALSE,   GT_FALSE },
    { 72,   GT_FALSE,   GT_FALSE },
    { 73,   GT_FALSE,   GT_FALSE },
    { 74,   GT_FALSE,   GT_TRUE  },
    { 75,   GT_FALSE,   GT_TRUE  },
    { 76,   GT_FALSE,   GT_TRUE  },
    { 77,   GT_FALSE,   GT_TRUE  },
    { 78,   GT_FALSE,   GT_TRUE  },
    { 79,   GT_FALSE,   GT_TRUE  },
    { 80,   GT_FALSE,   GT_TRUE  },
    { 81,   GT_FALSE,   GT_TRUE  },
    { 82,   GT_FALSE,   GT_FALSE },
    { 83,   GT_FALSE,   GT_FALSE },
    { 84,   GT_FALSE,   GT_FALSE },
    { 85,   GT_FALSE,   GT_TRUE  },
    { 86,   GT_FALSE,   GT_FALSE },
    { 87,   GT_FALSE,   GT_TRUE  },
    { 88,   GT_FALSE,   GT_FALSE },
    { 89,   GT_FALSE,   GT_FALSE },
    { 90,   GT_FALSE,   GT_TRUE  },
    { 91,   GT_FALSE,   GT_TRUE  },
    { 92,   GT_FALSE,   GT_TRUE  },
    { 93,   GT_FALSE,   GT_TRUE  },
    { 94,   GT_FALSE,   GT_FALSE },
    { 95,   GT_FALSE,   GT_FALSE },
    { 96,   GT_FALSE,   GT_TRUE  },
    { 97,   GT_FALSE,   GT_TRUE  },
    { 98,   GT_FALSE,   GT_TRUE  },
    { 99,   GT_FALSE,   GT_TRUE  },
    { 100,  GT_FALSE,   GT_TRUE  },
    { 101,  GT_FALSE,   GT_FALSE },
    { 102,  GT_FALSE,   GT_TRUE  },
    { 103,  GT_FALSE,   GT_TRUE  },
    { 104,  GT_FALSE,   GT_TRUE  },
    { 105,  GT_FALSE,   GT_FALSE },
    { 106,  GT_FALSE,   GT_FALSE },
    { 107,  GT_FALSE,   GT_TRUE  },
    { 108,  GT_FALSE,   GT_TRUE  },
    { 109,  GT_FALSE,   GT_TRUE  },
    { 110,  GT_FALSE,   GT_TRUE  },
    { 111,  GT_FALSE,   GT_TRUE  },
    { 112,  GT_FALSE,   GT_TRUE  },
    { 113,  GT_FALSE,   GT_TRUE  },
    { 114,  GT_FALSE,   GT_TRUE  },
    { 115,  GT_FALSE,   GT_TRUE  },
    { 116,  GT_FALSE,   GT_TRUE  },
    { 117,  GT_FALSE,   GT_TRUE  },
    { 118,  GT_FALSE,   GT_TRUE  },
    { 119,  GT_FALSE,   GT_TRUE  },
    { 120,  GT_FALSE,   GT_TRUE  },
    { 121,  GT_FALSE,   GT_TRUE  },
    { 122,  GT_FALSE,   GT_TRUE  },
    { 123,  GT_FALSE,   GT_TRUE  },
    { 124,  GT_FALSE,   GT_TRUE  },
    { 125,  GT_FALSE,   GT_FALSE },
    { 126,  GT_FALSE,   GT_TRUE  },
    { 127,  GT_FALSE,   GT_FALSE },
    { 128,  GT_FALSE,   GT_TRUE  },
    { 129,  GT_FALSE,   GT_TRUE  },
    { 130,  GT_FALSE,   GT_TRUE  },
    { 131,  GT_FALSE,   GT_FALSE },
    { 132,  GT_FALSE,   GT_TRUE  },
    { 133,  GT_FALSE,   GT_TRUE  },
    { 134,  GT_FALSE,   GT_TRUE  },
    { 135,  GT_FALSE,   GT_FALSE },
    { 136,  GT_FALSE,   GT_TRUE  },
    { 137,  GT_FALSE,   GT_TRUE  },
    { 138,  GT_FALSE,   GT_TRUE  },
    { 139,  GT_FALSE,   GT_TRUE  },
    { 140,  GT_FALSE,   GT_FALSE },
    { 141,  GT_FALSE,   GT_TRUE  },
    { 142,  GT_FALSE,   GT_TRUE  },
    { 143,  GT_FALSE,   GT_TRUE  },
    { 144,  GT_FALSE,   GT_TRUE  },
    { 145,  GT_FALSE,   GT_TRUE  },
    { 146,  GT_FALSE,   GT_FALSE },
    { 147,  GT_FALSE,   GT_TRUE  },
    { 148,  GT_FALSE,   GT_TRUE  },
    { 149,  GT_FALSE,   GT_TRUE  },
    { 150,  GT_FALSE,   GT_FALSE },
    { 151,  GT_FALSE,   GT_TRUE  },
    { 152,  GT_FALSE,   GT_FALSE },
    { 153,  GT_FALSE,   GT_FALSE },
    { 154,  GT_FALSE,   GT_TRUE  },
    { 155,  GT_FALSE,   GT_TRUE  },
    { 156,  GT_FALSE,   GT_TRUE  },
    { 157,  GT_FALSE,   GT_TRUE  },
    { 158,  GT_FALSE,   GT_FALSE },
    { 159,  GT_FALSE,   GT_FALSE },
    { 160,  GT_FALSE,   GT_TRUE  },
    { 161,  GT_FALSE,   GT_TRUE  },
    { 162,  GT_FALSE,   GT_TRUE  },
    { 163,  GT_FALSE,   GT_TRUE  },
    { 164,  GT_FALSE,   GT_TRUE  },
    { 165,  GT_FALSE,   GT_FALSE },
    { 166,  GT_FALSE,   GT_TRUE  },
    { 167,  GT_FALSE,   GT_TRUE  },
    { 168,  GT_FALSE,   GT_TRUE  },
    { 169,  GT_FALSE,   GT_FALSE },
    { 170,  GT_FALSE,   GT_FALSE },
    { 171,  GT_FALSE,   GT_TRUE  },
    { 172,  GT_FALSE,   GT_TRUE  },
    { 173,  GT_FALSE,   GT_TRUE  },
    { 174,  GT_FALSE,   GT_TRUE  },
    { 175,  GT_FALSE,   GT_TRUE  },
    { 176,  GT_FALSE,   GT_TRUE  },
    { 177,  GT_FALSE,   GT_TRUE  },
    { 178,  GT_FALSE,   GT_TRUE  },
    { 179,  GT_FALSE,   GT_TRUE  },
    { 180,  GT_FALSE,   GT_TRUE  },
    { 181,  GT_FALSE,   GT_TRUE  },
    { 182,  GT_FALSE,   GT_TRUE  },
    { 183,  GT_FALSE,   GT_TRUE  },
    { 184,  GT_FALSE,   GT_TRUE  },
    { 185,  GT_FALSE,   GT_TRUE  },
    { 186,  GT_FALSE,   GT_TRUE  },
    { 187,  GT_FALSE,   GT_TRUE  },
    { 188,  GT_FALSE,   GT_TRUE  },
    { 189,  GT_FALSE,   GT_TRUE  },
    { 190,  GT_FALSE,   GT_TRUE  },
    { 191,  GT_FALSE,   GT_FALSE },
    { 192,  GT_FALSE,   GT_FALSE },
    { 193,  GT_FALSE,   GT_FALSE },
    { 194,  GT_FALSE,   GT_TRUE  },
    { 195,  GT_FALSE,   GT_FALSE },
    { 196,  GT_FALSE,   GT_FALSE },
    { 197,  GT_FALSE,   GT_FALSE },
    { 198,  GT_FALSE,   GT_FALSE },
    { 199,  GT_FALSE,   GT_FALSE },
    { 200,  GT_TRUE,    GT_TRUE  },
    { 201,  GT_FALSE,   GT_TRUE  },
    { 202,  GT_FALSE,   GT_FALSE },
    { 203,  GT_FALSE,   GT_TRUE  },
    { 204,  GT_FALSE,   GT_FALSE },
    { 205,  GT_FALSE,   GT_FALSE },
    { 206,  GT_FALSE,   GT_FALSE },
    { 207,  GT_FALSE,   GT_FALSE },
    { 208,  GT_FALSE,   GT_FALSE },
    { 209,  GT_FALSE,   GT_TRUE  },
    { 210,  GT_FALSE,   GT_FALSE },
    { 211,  GT_FALSE,   GT_FALSE },
    { 212,  GT_FALSE,   GT_FALSE },
    { 213,  GT_FALSE,   GT_FALSE },
    { 214,  GT_FALSE,   GT_FALSE },
    { 215,  GT_FALSE,   GT_TRUE  },
    { 216,  GT_FALSE,   GT_TRUE  },
    { 217,  GT_FALSE,   GT_TRUE  },
    { 218,  GT_FALSE,   GT_TRUE  },
    { 219,  GT_FALSE,   GT_FALSE },
    { 220,  GT_FALSE,   GT_TRUE  },
    { 221,  GT_FALSE,   GT_FALSE },
    { 222,  GT_FALSE,   GT_FALSE },
    { 223,  GT_FALSE,   GT_FALSE },
    { 224,  GT_FALSE,   GT_TRUE  },
    { 225,  GT_FALSE,   GT_TRUE  },
    { 226,  GT_FALSE,   GT_FALSE },
    { 227,  GT_FALSE,   GT_TRUE  },
    { 228,  GT_FALSE,   GT_TRUE  },
    { 229,  GT_FALSE,   GT_FALSE },
    { 230,  GT_FALSE,   GT_FALSE },
    { 231,  GT_FALSE,   GT_TRUE  },
    { 232,  GT_FALSE,   GT_FALSE },
    { 233,  GT_FALSE,   GT_TRUE  },
    { 234,  GT_FALSE,   GT_TRUE  },
    { 235,  GT_FALSE,   GT_FALSE },
    { 236,  GT_FALSE,   GT_TRUE  },
    { 237,  GT_FALSE,   GT_FALSE },
    { 238,  GT_FALSE,   GT_FALSE },
    { 239,  GT_FALSE,   GT_FALSE },
    { 240,  GT_FALSE,   GT_FALSE },
    { 241,  GT_FALSE,   GT_FALSE },
    { 242,  GT_FALSE,   GT_TRUE  },
    { 243,  GT_FALSE,   GT_TRUE  },
    { 244,  GT_FALSE,   GT_FALSE },
    { 245,  GT_FALSE,   GT_FALSE },
    { 246,  GT_FALSE,   GT_FALSE },
    { 247,  GT_FALSE,   GT_FALSE },
    { 248,  GT_FALSE,   GT_FALSE },
    { 249,  GT_FALSE,   GT_FALSE },
    { 250,  GT_FALSE,   GT_FALSE },
    { 251,  GT_FALSE,   GT_TRUE  },
    { 252,  GT_FALSE,   GT_FALSE },
    { 253,  GT_FALSE,   GT_TRUE  },
    { 254,  GT_FALSE,   GT_FALSE },
    { 255,  GT_FALSE,   GT_FALSE },
    { 257,  GT_FALSE,   GT_FALSE },
    { 258,  GT_FALSE,   GT_FALSE }
};

static APPDEMO_SERDES_LANE_POLARITY_STC  falcon_2T4T_PolarityArray[] =
{
/* laneNum  invertTx    invertRx */
    {  0,  GT_FALSE,   GT_TRUE  },
    {  1,  GT_FALSE,   GT_TRUE  },
    {  2,  GT_TRUE,    GT_TRUE  },
    {  3,  GT_FALSE,   GT_TRUE  },
    {  4,  GT_TRUE,    GT_TRUE  },
    {  5,  GT_TRUE,    GT_FALSE },
    {  6,  GT_FALSE,   GT_TRUE  },
    {  7,  GT_TRUE,    GT_FALSE },
    {  8,  GT_TRUE,    GT_FALSE },
    {  9,  GT_TRUE,    GT_TRUE  },
    { 10,  GT_FALSE,   GT_FALSE },
    { 11,  GT_TRUE,    GT_TRUE  },
    { 12,  GT_TRUE,    GT_FALSE },
    { 13,  GT_TRUE,    GT_TRUE  },
    { 14,  GT_TRUE,    GT_FALSE },
    { 15,  GT_TRUE,    GT_TRUE  },
    { 16,  GT_TRUE,    GT_TRUE  },
    { 17,  GT_FALSE,   GT_FALSE },
    { 18,  GT_TRUE,    GT_TRUE  },
    { 19,  GT_TRUE,    GT_FALSE },
    { 20,  GT_FALSE,   GT_TRUE  },
    { 21,  GT_FALSE,   GT_FALSE },
    { 22,  GT_FALSE,   GT_TRUE  },
    { 23,  GT_FALSE,   GT_FALSE },
    { 24,  GT_FALSE,   GT_FALSE },
    { 25,  GT_FALSE,   GT_TRUE  },
    { 26,  GT_TRUE,    GT_FALSE },
    { 27,  GT_FALSE,   GT_FALSE },
    { 28,  GT_TRUE,    GT_FALSE },
    { 29,  GT_TRUE,    GT_TRUE  },
    { 30,  GT_FALSE,   GT_FALSE },
    { 31,  GT_FALSE,   GT_TRUE  },
    { 32,  GT_FALSE,   GT_FALSE },
    { 33,  GT_TRUE,    GT_FALSE },
    { 34,  GT_TRUE,    GT_FALSE },
    { 35,  GT_TRUE,    GT_TRUE  },
    { 36,  GT_TRUE,    GT_TRUE  },
    { 37,  GT_FALSE,   GT_TRUE  },
    { 38,  GT_FALSE,   GT_FALSE },
    { 39,  GT_FALSE,   GT_FALSE },
    { 40,  GT_TRUE,    GT_FALSE },
    { 41,  GT_FALSE,   GT_FALSE },
    { 42,  GT_TRUE,    GT_TRUE  },
    { 43,  GT_TRUE,    GT_TRUE  },
    { 44,  GT_TRUE,    GT_TRUE  },
    { 45,  GT_TRUE,    GT_FALSE },
    { 46,  GT_FALSE,   GT_FALSE },
    { 47,  GT_FALSE,   GT_FALSE },
    { 48,  GT_FALSE,   GT_FALSE },
    { 49,  GT_FALSE,   GT_FALSE },
    { 50,  GT_TRUE,    GT_FALSE },
    { 51,  GT_FALSE,   GT_TRUE  },
    { 52,  GT_FALSE,   GT_TRUE  },
    { 53,  GT_TRUE,    GT_TRUE  },
    { 54,  GT_FALSE,   GT_FALSE },
    { 55,  GT_TRUE,    GT_FALSE },
    { 56,  GT_FALSE,   GT_FALSE },
    { 57,  GT_FALSE,   GT_FALSE },
    { 58,  GT_FALSE,   GT_TRUE  },
    { 59,  GT_FALSE,   GT_TRUE  },
    { 60,  GT_FALSE,   GT_TRUE  },
    { 61,  GT_FALSE,   GT_FALSE },
    { 62,  GT_FALSE,   GT_FALSE },
    { 63,  GT_FALSE,   GT_FALSE },
    { 64,  GT_TRUE,    GT_TRUE  },
    { 65,  GT_TRUE,    GT_FALSE },
    { 66,  GT_FALSE,   GT_TRUE  },
    { 67,  GT_FALSE,   GT_TRUE  },
    { 68,  GT_FALSE,   GT_FALSE },
    { 69,  GT_FALSE,   GT_TRUE  },
    { 70,  GT_FALSE,   GT_TRUE  },
    { 71,  GT_FALSE,   GT_TRUE  },
    { 72,  GT_TRUE,    GT_TRUE  },
    { 73,  GT_TRUE,    GT_TRUE  },
    { 74,  GT_FALSE,   GT_TRUE  },
    { 75,  GT_TRUE,    GT_TRUE  },
    { 76,  GT_TRUE,    GT_TRUE  },
    { 77,  GT_TRUE,    GT_TRUE  },
    { 78,  GT_TRUE,    GT_TRUE  },
    { 79,  GT_TRUE,    GT_TRUE  },
           /* cpu ports */
    { 80,  GT_FALSE,   GT_TRUE  },
    { 81,  GT_TRUE,    GT_FALSE }
};

#ifndef GM_USED
static CPSS_FALCON_LED_STREAM_INDICATIONS_STC falcon_12_8_led_indications[CPSS_CHIPLETS_MAX_NUM_CNS] =
{
    {64, 65, GT_FALSE},   /* Raven  0 - 2 LED ports */
    {64, 66, GT_TRUE },   /* Raven  1 - 2 LED ports + CPU Port */
    {64, 66, GT_TRUE },   /* Raven  2 - 2 LED ports + CPU Port */
    {64, 65, GT_FALSE},   /* Raven  3 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  4 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  5 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  6 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  7 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  8 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  9 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven 10 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven 11 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven 12 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven 13 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven 14 - 2 LED ports */
    {64, 65, GT_FALSE}    /* Raven 15 - 2 LED ports */
};

static CPSS_FALCON_LED_STREAM_INDICATIONS_STC falcon_6_4_led_indications[CPSS_CHIPLETS_MAX_NUM_CNS] =
{
    {64, 66, GT_TRUE },   /* Raven  0 - 2 LED ports + CPU Port */
    {64, 65, GT_FALSE},   /* Raven  1 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  2 - 2 LED ports */
    {64, 66, GT_TRUE },   /* Raven  3 - 2 LED ports + CPU Port */
    {64, 66, GT_TRUE },   /* Raven  4 - 2 LED ports + CPU Port */
    {64, 65, GT_FALSE},   /* Raven  5 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  6 - 2 LED ports */
    {64, 66, GT_TRUE },   /* Raven  7 - 2 LED ports + CPU Port */
};

static CPSS_FALCON_LED_STREAM_INDICATIONS_STC falcon_6_4_reduced_led_indications[CPSS_CHIPLETS_MAX_NUM_CNS] =
{
    {64, 66, GT_TRUE },   /* Raven  0 - 2 LED ports + CPU Port */
    { 0,  0, GT_FALSE},   /* Raven  1 - not used */
    {64, 65, GT_FALSE},   /* Raven  2 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  3 - 2 LED ports */
    { 0,  0, GT_FALSE},   /* Raven  4 - not used */
    {64, 66, GT_TRUE },   /* Raven  5 - 2 LED ports + CPU Port */
    { 0,  0, GT_FALSE},   /* Raven  6 - not used */
    { 0,  0, GT_FALSE},   /* Raven  7 - not used */
};

static CPSS_FALCON_LED_STREAM_INDICATIONS_STC falcon_2T_4T_led_indications[CPSS_CHIPLETS_MAX_NUM_CNS] =
{
    {64, 79, GT_FALSE},   /* Raven  0 - 16 LED ports */
    { 0,  0, GT_FALSE},   /* Raven  1 - not used */
    {64, 79, GT_FALSE},   /* Raven  2 - 16 LED ports */
    { 0,  0, GT_FALSE},   /* Raven  3 - not used */
    {64, 79, GT_FALSE},   /* Raven  4 - 16 LED ports */
    {64, 80, GT_TRUE },   /* Raven  5 - 16 LED ports + CPU Port */
    { 0,  0, GT_FALSE},   /* Raven  6 - not used */
    {64, 80, GT_TRUE },   /* Raven  7 - 16 LED ports + CPU Port */
};

#endif

static GT_BOOL  isUnderAddDeviceCatchup = GT_FALSE;
#define START_LOOP_ALL_DEVICES(_devNum) \
    for (devIndex = SYSTEM_DEV_NUM_MAC(0); \
          (devIndex < SYSTEM_DEV_NUM_MAC(appDemoPpConfigDevAmount)); devIndex++) \
    {                                                                            \
        _devNum =   appDemoPpConfigList[devIndex].devNum;                        \
        SYSTEM_SKIP_NON_ACTIVE_DEV(devIndex)

#define END_LOOP_ALL_DEVICES \
    }
static GT_STATUS gtDbFalconBoardReg_deviceSimpleInit
(
    IN  GT_U8  devIndex,
    IN  GT_U8  boardRevId
);

/* offset used during HW device ID calculation formula */
extern GT_U8 appDemoHwDevNumOffset;
/* cpssInitSystem has been run */
extern GT_BOOL systemInitialized;
/* port manager boolean variable */
extern GT_BOOL portMgr;

static GT_BOOL phyGearboxMode;

extern GT_STATUS userForceBoardType(IN GT_U32 boardType);

/* read TSEN value of the Eagle */
static GT_BOOL tsenEagleRead;

/* indication that during initialization the call to appDemoBc2PortListInit(...)
   was already done so no need to call it for second time. */
extern GT_U32  appDemo_PortsInitList_already_done;

extern CPSS_DXCH_PORT_MAP_STC  *appDemoPortMapPtr;
/* number of elements in appDemoPortMapPtr */
extern GT_U32                  appDemoPortMapSize;

extern GT_STATUS appDemoDxLibrariesInit(
    IN  GT_U8                       dev,
    IN  APP_DEMO_LIB_INIT_PARAMS    *libInitParamsPtr,
    IN  CPSS_PP_CONFIG_INIT_STC     *sysConfigParamsPtr
);
extern GT_STATUS appDemoDxPpGeneralInit(GT_U8 dev);
extern GT_STATUS appDemoDxTrafficEnable(GT_U8 dev);
extern GT_STATUS boardAutoPollingConfigure(IN  GT_U8   dev,IN  GT_U32  boardType);
extern GT_STATUS appDemoBc2IpLpmRamDefaultConfigCalc(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
);
extern GT_STATUS appDemoBc2PortListInit
(
    IN GT_U8 dev,
    IN PortInitList_STC * portInitList,
    IN GT_BOOL            skipCheckEnable
);
extern GT_STATUS appDemoDxHwDevNumChange
(
    IN  GT_U8                       dev,
    IN  GT_HW_DEV_NUM               hwDevNum
);
GT_STATUS appDemoBc2PIPEngineInit
(
    IN  GT_U8       boardRevId
);
GT_STATUS appDemoB2PtpConfig
(
    IN  GT_U8  devNum
);
extern GT_STATUS appDemoDxHwPpPhase1DbEntryInit
(
    IN  GT_U8                        dev,
    INOUT  CPSS_DXCH_PP_PHASE1_INIT_INFO_STC   *cpssPpPhase1ParamsPtr,
    IN  CPSS_PP_DEVICE_TYPE         deviceId,
    OUT GT_U32                      *waIndexPtr,
    OUT CPSS_DXCH_IMPLEMENT_WA_ENT  *waFromCpss,
    OUT GT_BOOL                     *trunkToCpuWaEnabledPtr
);

GT_STATUS   appDemoFalconMainUtForbidenTests(void);

static GT_STATUS falcon_appDemoDbUpdate
(
    IN GT_U32             devIndex,
    IN GT_SW_DEV_NUM      devNum,
    IN CPSS_PP_DEVICE_TYPE     devType
);

/* allow to notify the CPSS that not need the WA */
extern GT_STATUS prvFalconPortDeleteWa_enableSet(IN GT_U32 waNeeded);

/* by default the WA is not disabled */
static GT_U32   falcon_initPortDelete_WA_disabled = 0;
/* allow to init with/without the WA for the 'port delete' */
GT_STATUS   falcon_initPortDelete_WA_set(IN GT_U32  waNeeded)
{
    falcon_initPortDelete_WA_disabled = !waNeeded;

    /* notify the CPSS that not need the WA */
    (void) prvFalconPortDeleteWa_enableSet(waNeeded);

    return GT_OK;
}

static GT_U32   reservedCpuSdmaGlobalQueue[2] = {0};

#define _2_TILES_PORTS_OFFSET   56
#define TESTS_PORTS_OFFSET(devNum)  (_2_TILES_PORTS_OFFSET)

#define PRINT_SKIP_DUE_TO_DB_FLAG(reasonPtr , flagNamePtr) \
        cpssOsPrintf("NOTE: '%s' skipped ! (due to flag '%s') \n",  \
            reasonPtr , flagNamePtr)

#define ARR_PTR_AND_SIZE_MAC(myArr , arrPtr , arrLen) \
    arrPtr = &myArr[0];                               \
    arrLen = sizeof(myArr)/sizeof(myArr[0])

#define UNUSED_PARAM_MAC(x) x = x

#define FALCON_TXQ_PER_DQ   (8 * (FALCON_MAC_PER_DP + 1))
#define FALCON_MAC_PER_DP   8 /* -- without CPU port -- first ports are those with SERDES 28/56 */
#define FALCON_NUM_DP       8 /* DP port groups */

/* build TXQ_port from macPort, dpIndex */
#define FALCON_TXQ_PORT(macPort, dpIndex)   \
    (FALCON_TXQ_PER_DQ*(dpIndex) + (macPort))

/* build MAC_port from macPort, dpIndex */
#define FALCON_MAC_PORT(macPort, dpIndex)   \
    (FALCON_MAC_PER_DP*(dpIndex) + (macPort))

#define CPU_PORT    CPSS_CPU_PORT_NUM_CNS
/* value to not collide with TXQ of other port */
#define TXQ_CPU_PORT    300

typedef struct
{
    GT_PHYSICAL_PORT_NUM                startPhysicalPortNumber;
    GT_U32                              numOfPorts;
    GT_U32                              startGlobalDmaNumber;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT     mappingType;
    GT_U32                              jumpDmaPorts;/* allow to skip DMA ports (from startGlobalDmaNumber).
                                                        in case that mappingType is CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E
                                                        this field Contains the cascade port number*/
}FALCON_PORT_MAP_STC;

#define IGNORE_DMA_STEP 0 /*value 0 or 1 means 'ignore' */
/* jump DMA in steps of 2 because we currently limited to '128 physical ports' mode */
/* so the 12.8T device with the 256+ MACs ... need to use only limited MACs */
#define DMA_STEP_OF_2   2
#define DMA_STEP_OF_3   3
#define DMA_STEP_OF_4   4


/*stay with same DMA - needed for remote ports */
#define DMA_NO_STEP   0xFFFF
#define NO_REMOTE_PORT 0xFFFF

#if 0
just like falcon_onEmulator_prepare_tiles(IN GT_U32    numOfTiles) but for 'HW'
#endif

/* function needed to hint us for the phase1Params->maxNumOfPhyPortsToUse */
GT_STATUS falcon_prepare_tiles(IN GT_U32    numOfTiles)
{
    appDemoDbEntryAdd("numOfTiles", numOfTiles);
    return GT_OK;
}

extern GT_STATUS appDemoTrace_skipTrace_onAddress(
    IN GT_U32   address,
    IN GT_U32   index /* up to 16 indexes supported 0..15 */
);

typedef struct{
    GT_U32  sdmaPort;
    GT_U32  macPort;
}MUX_DMA_STC;
/* MUXING info */
static MUX_DMA_STC falcon_3_2_muxed_SDMAs[]  = {{ 68, 66},{ 70, 67},{GT_NA,GT_NA}};
static MUX_DMA_STC falcon_6_4_muxed_SDMAs[]  = {{136,130},{138,131},{140,132},{142,133},{GT_NA,GT_NA}};
static MUX_DMA_STC falcon_12_8_muxed_SDMAs[] = {{272,258},{274,259},{276,260},{278,261},{280,266},{282,267},{284,268},{286,269},{GT_NA,GT_NA}};


/*  in sip5_20 was 2 ports modes:
     256, 512.
    sip6 have 5 ports modes:
    64, 128, 256, 512, 1024.
    Note that falcon have 3.2 device, 6.4 device and 12.8 device, each of these devices can have the ports modes above so in total it's (3 * 5) = 15 different modes
    */

/* 64 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports).*/
static FALCON_PORT_MAP_STC falcon_3_2_defaultMap_64_port_mode[] =
{
    /* physical ports 0..49 , mapped to MAC 0..32 */
     {0               , 49          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 68 */
    ,{CPU_PORT/*63*/  , 1           ,  68/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
     /* map 'CPU SDMA ports' 56..58 (no MAC/SERDES) DMA 69..71 */
    ,{56              , 3           ,  69/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_3_2_defaultMap_64_port_mode = sizeof(falcon_3_2_defaultMap_64_port_mode)/sizeof(falcon_3_2_defaultMap_64_port_mode[0]);


/* 128 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports). */
static FALCON_PORT_MAP_STC falcon_3_2_defaultMap[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 68 */
    ,{CPU_PORT/*63*/  , 1           ,  68/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* physical ports 64..68 , mapped to MAC 59..63 */
    ,{64              , (63-59)+1   ,  59       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP }
    /* map 'Ethernet CPU ports' (non-SDMA) */

    /* physical port 69 , mapped to MAC 65 */
/* on DP[2] : CPU network port on DMA 65 (NOT muxed with any SDMAs) */
    ,{69              , 1           ,  65       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
/* on DP[4] : CPU network port on DMA 66 is muxed with DMA 68 used by SDMA0 */

    /* map 'CPU SDMA ports' 80..82 (no MAC/SERDES) DMA 69..71 */
    ,{80              , 3           ,  69/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_3_2_defaultMap = sizeof(falcon_3_2_defaultMap)/sizeof(falcon_3_2_defaultMap[0]);

/* 128 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports) + One Network CPU port */
static FALCON_PORT_MAP_STC falcon_3_2_defaultMap_TwoNetCpuPorts[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 69 (DMA 68 muxed with DMA 66 ... so cant be used)
       meaning using MG[1] (and MG[0] is unused) */
    ,{CPU_PORT/*63*/  , 1           ,  69/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP }
    /* physical ports 64..68 , mapped to MAC 59..63 */
    ,{64              , (63-59)+1   ,  59       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP }
    /* map 'Ethernet CPU ports' (non-SDMA) */
    /* physical port 69 , mapped to MAC 65 */
    ,{69              , 1           ,  65       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

    /* muxed with MG[0] on DP[4] */
    ,{70              , 1           ,  66       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

    ,{80              , 1           ,  70/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    ,{81              , 1           ,  71/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}


};
static GT_U32   actualNum_falcon_3_2_defaultMap_TwoNetCpuPorts = sizeof(falcon_3_2_defaultMap_TwoNetCpuPorts)/sizeof(falcon_3_2_defaultMap_TwoNetCpuPorts[0]);


/* currently 256 and 512 modes are using 128 mode port map to avoid failed tests that not support other ports selection*/

/* 256 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_3_2_defaultMap_256_port_mode[] =
{
    /* physical ports 128..186 , mapped to MAC 0..58 */
     {0              , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 65 */
    ,{CPU_PORT/*63*/  , 1           ,  68/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* physical ports 192..196 , mapped to MAC 59..63 */
    ,{128             , (63-59)+1   ,  59       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' 80..82 (no MAC/SERDES) DMA 66..68 */
    ,{80              , 3           ,  69/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_3_2_defaultMap_256_port_mode = sizeof(falcon_3_2_defaultMap_256_port_mode)/sizeof(falcon_3_2_defaultMap_256_port_mode[0]);


/* 1024 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports). include remote ports configurations */
static FALCON_PORT_MAP_STC falcon_3_2_defaultMap_1024_port_mode_remote_ports[] =
{
    /* native ports: not remote ports .physical ports 128..186 , mapped to MAC 0..58 */
     {(0 /*+ 128*/)              , 59/*56*/          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP }
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 65 */
    ,{CPU_PORT/*63*/  , 1           ,  68/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* remote physical ports 590..599 , mapped to MAC 59 , with phisical port 64. this is comment out to enable the enh-UT ports  58 12 36 0 */
   /* ,{590             , 10   ,  59       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,DMA_NO_STEP , 64}*/
    /* remote physical ports 600..609 , mapped to MAC 60 , with phisical port 65 */
    ,{600             , 10   ,  60       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,65}
    /* remote physical ports 610..619 , mapped to MAC 61 , with phisical port 66 */
    ,{610             , 10   ,  61       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,66}
    /* remote physical ports 620..629 , mapped to MAC 62 , with phisical port 67 */
    ,{620             , 9   ,  62       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , 67}
    /* remote physical ports 630..639 , mapped to MAC 63 , with phisical port 68 */
    ,{630             , 9   ,  63       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , 68}
#if 0 /* muxed with MG[2] on DP[6] */
    /* map 'Ethernet CPU ports' (non-SDMA) */
    /* physical port 69 , mapped to MAC 67 */
    ,{69              , 1           ,  67       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
#endif /*0*/

    /* map 'CPU SDMA ports' 80..82 (no MAC/SERDES) DMA 66..68 */
    ,{80              , 3           ,  69/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}


};
static GT_U32   actualNum_falcon_3_2_defaultMap_1024_port_mode_remote_ports = sizeof(falcon_3_2_defaultMap_1024_port_mode_remote_ports)/sizeof(falcon_3_2_defaultMap_1024_port_mode_remote_ports[0]);


/* we have PHYSICAL ports 0..59,63,64..119*/
/* this macro hold the number of ports from 64..119 */
#define NUM_PORTS_64_TO_119  ((119-64)+1)
/* 64 port mode: mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_6_4_defaultMap_64_port_mode[] =
{
    /* physical ports 0..52 , mapped to MAC 0..52 */
    /* leave physical port 53 to be used by the 'port delete WA' */
     {0               , 53          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* physical ports 54 , mapped to MAC 54 */
    ,{54              ,  1          ,   54       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 136 */
    ,{CPU_PORT/*63*/  , 1           ,  136/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E   ,IGNORE_DMA_STEP}

    /* map 'CPU SDMA ports' 55 - 61 to DMA 137 - 143 */
    ,{55              , 7           , 137/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_6_4_defaultMap_64_port_mode = sizeof(falcon_6_4_defaultMap_64_port_mode)/sizeof(falcon_6_4_defaultMap_64_port_mode[0]);

/* mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_6_4_defaultMap_256_port_mode[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0              , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 136 */
    ,{CPU_PORT/*63*/  , 1           ,  136/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* physical ports 64..132 , mapped to MAC 59..127 */
    ,{64              ,(127 - 59 + 1),  59 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

#if 0  /* the 2 ports muxed with SDMAs */
    /* CPU#0 port */
    ,{133             ,1,               131 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* CPU#1 port */
    ,{134             ,1,               132 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
#endif
    /* map 'CPU SDMA ports' 135 - 141 to DMA 137 - 143 */
    ,{135              , 7           , 137/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_6_4_defaultMap_256_port_mode = sizeof(falcon_6_4_defaultMap_256_port_mode)/sizeof(falcon_6_4_defaultMap_256_port_mode[0]);

/* mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_6_4_defaultMap[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 136 */
    ,{CPU_PORT/*63*/  , 1           ,  136/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* physical ports 64..119 , mapped to MAC 59..116 */
    ,{64              ,NUM_PORTS_64_TO_119,  59 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' 121 - 127 to DMA 137 - 143 */
    ,{121              , 7           , 137/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_6_4_defaultMap = sizeof(falcon_6_4_defaultMap)/sizeof(falcon_6_4_defaultMap[0]);

static FALCON_PORT_MAP_STC falcon_6_4_100G_defaultMap[] =
{
    /* physical ports 0..58 , mapped to MAC 0..116 with step 2 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 136 */
    ,{CPU_PORT/*63*/  , 1           ,  136/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}

    ,{64              , 5,             118,         CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2}

    /* map 'CPU SDMA ports' 121 - 127 to DMA 137 - 143 */
    ,{121              , 7           , 137/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_6_4_100G_defaultMap = sizeof(falcon_6_4_100G_defaultMap)/sizeof(falcon_6_4_100G_defaultMap[0]);


/***************************************************************/
/* flag to use the MAX number of macs that the device supports */
/* AND max SDMAs                                               */
/***************************************************************/
static GT_U32   modeMaxMac = 0;
static GT_U32   mode100G   = 0;
static GT_U32   cpuPortMux1UseSdma = 1; /* default mode - SDMA (for CPU should be used '0' by calling appDemoDbEntrySet) */
static GT_U32   cpuPortMux2UseSdma = 1; /* default mode - SDMA (for CPU should be used '0' by calling appDemoDbEntrySet) */
/* mode for 128 Front panel ports  + one or 2 reduced ports :
    the ports can be as:
    1. 400G                                     max of 32 *400 = 12.8 T
    2. 4 * 10           (only even MACs)        max of 128*10  =  1.28T
    3. 4 * 25           (only even MACs)        max of 128*25  =  3.2 T
    4. 2 * 100G R4                              max of 64*100  =  6.4 T
    5. 2 * 40G R4                               max of 64*40   =  2.56T
*/
static GT_U32   modeSonic128MacPorts = 0;

static FALCON_PORT_MAP_STC falcon_6_4_maxMAC[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 136 */
    ,{CPU_PORT/*63*/  , 1           ,  136/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* physical ports 64..123 , mapped to MAC 59..118 */
    ,{64              ,60,             59 ,        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

    /* NOTE: the 'mux' are removed in run time !!! depend on :
            use_falcon_6_4_cpu_ports_group1
            use_falcon_6_4_cpu_ports_group2
    */
    /* CPU#0 port Raven 0 - dp[0]*/
    ,{124             ,1,               128 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* CPU#1 port Raven 3 - dp[6]*/
    ,{125             ,1,               131 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* CPU#2 port Raven 4 - dp[8]*/
    ,{126             ,1,               132 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* CPU#3 port Raven 7 - dp[14]*/
    ,{127             ,1,               135 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_6_4_maxMAC = sizeof(falcon_6_4_maxMAC)/sizeof(falcon_6_4_maxMAC[0]);

static FALCON_PORT_MAP_STC falcon_6_4_256_port_maxMAC[] =
{
    /* first 59 MACs : physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0        , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 136 */
    ,{CPU_PORT/*63*/  , 1           ,  136/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* next 128-59 MACs : physical ports 64..132 , mapped to MAC 59..127 */
    ,{64              ,(128-59)     ,   59       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

    /* NOTE: the 'mux' are removed in run time !!! depend on :
            use_falcon_6_4_cpu_ports_group1
            use_falcon_6_4_cpu_ports_group2
    */
    /* CPU#0 port Raven 0 - dp[0]*/
    ,{133             ,1,               128 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* CPU#1 port Raven 3 - dp[6]*/
    ,{134             ,1,               131 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* CPU#2 port Raven 4 - dp[8]*/
    ,{135             ,1,               132 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* CPU#3 port Raven 7 - dp[14]*/
    ,{136             ,1,               135 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

    /* map 'CPU SDMA ports' 137 - 143 to DMA 137 - 143 ,
       NOTE: the 'mux' of DMA 138 (muxed with MAC 131) and DMA 140 (muxed with MAC 132)
            is removed in runtime if
    */
    ,{137              , 7           , 137/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_6_4_256_port_maxMAC = sizeof(falcon_6_4_256_port_maxMAC)/sizeof(falcon_6_4_256_port_maxMAC[0]);

/* 1024 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports). include remote ports configurations */
static FALCON_PORT_MAP_STC falcon_6_4_defaultMap_1024_port_mode_remote_ports[] =
{
    /* native ports: first 29 MACs : physical ports 0..28 , mapped to MAC 0..28 */
     {0              , 28          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP}
     /* skip physicals 29..64 */
     /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 136 */
    ,{CPU_PORT/*63*/  , 1           ,  136/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
       /* remote physical ports 600..609 , mapped to MAC 65 (DP 8) , with phisical port 75 */
    ,{600             , 10   ,  65     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , 75}
    /* remote physical ports 610..619 , mapped to MAC 33 (DP 4) , with phisical port 76 */
    ,{610             , 10   ,  33       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,76}
    /* remote physical ports 620..629 , mapped to MAC 66 (DP 8), with phisical port 77 */
    ,{620             , 9   ,  66       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , 77}
    /* remote physical ports 630..639 , mapped to MAC 33 , with phisical port 78 */
    ,{630             , 9   ,  35       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , 78}
    /* next 67-77 MACs : physical ports 64..74 */
     ,{64              ,10     ,     67     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* next 77- 97 MACs : physical ports 100..119 */
     ,{100              ,20     ,     77     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' */
    ,{133             , 7           , 137/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};

static GT_U32   actualNum_falcon_6_4_defaultMap_1024_port_mode_remote_ports = sizeof(falcon_6_4_defaultMap_1024_port_mode_remote_ports)/sizeof(falcon_6_4_defaultMap_1024_port_mode_remote_ports[0]);


static FALCON_PORT_MAP_STC falcon_6_4_port_mode_9[] =
{
     /* DP #0 -3  */
      {0               , 1          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{8               , 1          ,   8       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{16               , 1          ,   16       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{24               , 1          ,   24       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     /* DP #4 physical ports  4 (MAC 32) connected to 48 remote ports 200-347 */
    ,{255             , 48   ,         32     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,32}
     /* DP #5 physical ports  5 (MAC 40) connected to 48 remote ports 248-395 */
    ,{303             , 48   ,         40     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,40}
     /* DP #6 physical ports  6 (MAC 48) connected to 48 remote ports 296-343 */
    ,{351             , 48   ,         48     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,48}
     /* DP #7 physical ports  7 (MAC 56 ) connected to 48 remote ports 344-391 */
    ,{399             , 48   ,         56     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,56}
     /* DP #8 physical ports  8 (MAC 64 ) connected to 48 remote ports 392-439 */
    ,{447             , 48   ,         64     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,69}
     /* DP #9 physical ports 9 (MAC 72) connected to 48 remote ports 440-487 */
    ,{495             , 48   ,         72     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,77}
     /* DP #10 physical ports  10 (MAC 80) connected to 48 remote ports 488-535 */
    ,{543             , 48   ,         80     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,85}
     /* DP #11 physical ports  11 (MAC 88) connected to 48 remote ports 536-583 */
    ,{591             , 48   ,         88     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,93}
     /* DP #12 -15  */
     ,{101               , 1          , 96       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{109               , 1          , 104       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{117               , 1          , 112       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{125               , 1          , 120       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     /* map 'CPU SDMA ports' */
    ,{CPU_PORT/*63*/  , 1           ,  136/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    ,{133             , 7           , 137/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};

static GT_U32   actualNum_falcon_6_4_port_mode_9 =
    sizeof(falcon_6_4_port_mode_9)
    /sizeof(falcon_6_4_port_mode_9[0]);


static FALCON_PORT_MAP_STC falcon_3_2_port_mode_9[] =
{
     /* DP #0 -3  */
      {0               , 1          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{8               , 1          ,   8       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{16               , 1          ,   16       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{24               , 1          ,   24       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     /* DP #4 physical ports  4 (MAC 32) connected to 48 remote ports 200-347 */
    ,{255             , 48   ,         32     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,32}
     /* DP #5 physical ports  5 (MAC 40) connected to 48 remote ports 248-395 */
    ,{303             , 48   ,         40     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,40}
     /* DP #6 physical ports  6 (MAC 48) connected to 48 remote ports 296-343 */
    ,{351             , 48   ,         48     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,48}
     /* DP #7 physical ports  7 (MAC 56 ) connected to 48 remote ports 344-391 */
    ,{399             , 48   ,         56     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,56}
    /* map 'CPU SDMA ports' 80..82 (no MAC/SERDES) DMA 66..68 */
    ,{80              , 3           ,  69/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};

static GT_U32   actualNum_falcon_3_2_port_mode_9 =
    sizeof(falcon_3_2_port_mode_9)
    /sizeof(falcon_3_2_port_mode_9[0]);



/*
NOTE: in 12.8 we select only 8 CPU SDMA ports although 16 available , because:
1. we work in 128 ports mode ... so using all 16 will reduce dramatically the number of regular ports.
2. we want to test 'non-consecutive' MG units so we use (MGs:1,3..5,10..13)
3. we test CPU_PORT (63) to work with MG1 instead of MG0 , because DP[4] (connected to MG0)
    may serve 'CPU network port'
*/


/* mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_12_8_defaultMap_64_port_mode[] =
{
    /* physical ports 0..32 , mapped to MAC 0..58 */
     {0               , 49          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 273 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           ,  273/*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' , DMA 50..52 of MG [3..5] */
    ,{50              , 3           , 275/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' , DMA 54..57 of MG [10..13] */
    ,{54              , 4          , 282/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_12_8_defaultMap_64_port_mode = sizeof(falcon_12_8_defaultMap_64_port_mode)/sizeof(falcon_12_8_defaultMap_64_port_mode[0]);

/* mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_12_8_defaultMap_256_port_mode[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 273 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           ,  273/*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* due to CPSS bug : CPSS-7899 : Falcon : CPSS fail when 'first DMA' in DP was not mapped in 'port mapping' but others from that DP mapped */
    /* we need for the additional 7 CPU ports , the existence of next ports : in DP[8..11].
    meaning DMA ports : 64,72,80,88 */
    /* physical ports 64..109 , mapped to MAC 64..154 (in steps of 2) */
    ,{64              ,NUM_PORTS_64_TO_119-10,  128  , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, DMA_STEP_OF_2}
    /* physical ports 110..119 , mapped to MAC 176..216 (in steps of 4) */
   /* ,{110             ,                    10,  176 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, DMA_STEP_OF_4,NO_REMOTE_PORT}*/

    /* map 'CPU SDMA ports' , DMA 50..52 of MG [3..5] */
    ,{121              , 3           , 275/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' , DMA 54..57 of MG [10..13] */
    ,{124              , 4          , 282/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_12_8_defaultMap_256_port_mode = sizeof(falcon_12_8_defaultMap_256_port_mode)/sizeof(falcon_12_8_defaultMap_256_port_mode[0]);

/*For Falcon RD */
/* guide lines:
1.  Mapped 128 physical ports (128..255) to 128 MACs in step of 2 (0,2,4..254)
2.  Mapped 63 to DMA 273  (not 272 because muxed with 258) (CPU_SDMA)
3.  Mapped 14 physical ports (0,13) to DMAs (273..287) (CPU_SDMA)
    so only 15 CPU SDMAs instead of 16
4.  Mapped additional 2 physical ports (64,65) to 2 reduced MAC ports (257,258)
*/

/* mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_12_8_rd_defaultMap_256_port_mode[] =
{
    {0               ,    1          ,  257
    , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP},
    {1               ,   1          ,   258
    , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP},
    {CPU_PORT  /*63*/  , 1           ,  273
    ,CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP} ,

      /* map 'CPU SDMA ports'  : 274....287 only 14 , because totally up to 16 ,but 272 of MG0 muxed with MAC 258 */
    {64              , 14          ,  274
    ,CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    , IGNORE_DMA_STEP} ,

    /*line card 1 , refer to line card 7 */
    {1+127               ,1         , 24+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
    {2+127               ,1         , 26+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
    {3+127               ,1         , 16+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
    {4+127               ,1         , 18+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
    {5+127               ,1         , 8+64*1      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
    {6+127               ,1         , 10+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
    {7+127               ,1         , 0+64*1      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
    {8+127               ,1         , 2+64*1      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
    {9+127               ,1         , 4+64*1      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
    {10+127              ,1         , 6+64*1      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
    {11+127              ,1         , 12+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
    {12+127              ,1         , 14+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
    {13+127              ,1         , 20+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
    {14+127              ,1         , 22+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
    {15+127              ,1         , 28+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
    {16+127              ,1         , 30+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},

    /*line card 2 , refer to card 8  */
     {1+127+16*1         ,1         , 56+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {2+127+16*1         ,1         , 58+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {3+127+16*1         ,1         , 48+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {4+127+16*1         ,1         , 50+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {5+127+16*1         ,1         , 40+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {6+127+16*1         ,1         , 42+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {7+127+16*1         ,1         , 32+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {8+127+16*1         ,1         , 34+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {9+127+16*1         ,1         , 36+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {10+127+16*1        ,1         , 38+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {11+127+16*1        ,1         , 44+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {12+127+16*1        ,1         , 46+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {13+127+16*1        ,1         , 52+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {14+127+16*1        ,1         , 54+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {15+127+16*1        ,1         , 60+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {16+127+16*1        ,1         , 62+64*1     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},

     /*line card 3 ,  refer to line card 7  */
     {1+127+16*2         ,1         , 24+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {2+127+16*2         ,1         , 26+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {3+127+16*2         ,1         , 16+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {4+127+16*2         ,1         , 18+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {5+127+16*2         ,1         , 8+64*2      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {6+127+16*2         ,1         , 10+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {7+127+16*2         ,1         , 0+64*2      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {8+127+16*2         ,1         , 2+64*2      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {9+127+16*2         ,1         , 4+64*2      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {10+127+16*2        ,1         , 6+64*2      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {11+127+16*2        ,1         , 12+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {12+127+16*2        ,1         , 14+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {13+127+16*2        ,1         , 20+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {14+127+16*2        ,1         , 22+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {15+127+16*2        ,1         , 28+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {16+127+16*2        ,1         , 30+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},

      /*line card 4 , line card 8  */
     {1+127+16*3         ,1         , 56+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {2+127+16*3         ,1         , 58+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {3+127+16*3         ,1         , 48+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {4+127+16*3         ,1         , 50+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {5+127+16*3         ,1         , 40+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {6+127+16*3         ,1         , 42+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {7+127+16*3         ,1         , 32+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {8+127+16*3         ,1         , 34+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {9+127+16*3         ,1         , 36+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {10+127+16*3        ,1         , 38+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {11+127+16*3        ,1         , 44+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {12+127+16*3        ,1         , 46+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {13+127+16*3        ,1         , 52+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {14+127+16*3        ,1         , 54+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {15+127+16*3        ,1         , 60+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {16+127+16*3        ,1         , 62+64*2     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},

    /*line card 5 ,  refer to line card 7  */
     {1+127+16*4         ,1         , 24+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {2+127+16*4         ,1         , 26+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {3+127+16*4         ,1         , 16+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {4+127+16*4         ,1         , 18+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {5+127+16*4         ,1         , 8+64*3      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {6+127+16*4         ,1         , 10+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {7+127+16*4         ,1         , 0+64*3      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {8+127+16*4         ,1         , 2+64*3      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {9+127+16*4         ,1         , 4+64*3      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {10+127+16*4        ,1         , 6+64*3      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {11+127+16*4        ,1         , 12+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {12+127+16*4        ,1         , 14+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {13+127+16*4        ,1         , 20+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {14+127+16*4        ,1         , 22+64*3     ,CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {15+127+16*4        ,1         , 28+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {16+127+16*4        ,1         , 30+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},

     /*line card 6 , line card 8  */
     {1+127+16*5         ,1         , 56+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {2+127+16*5         ,1         , 58+64*3     ,  CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {3+127+16*5         ,1         , 48+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {4+127+16*5         ,1         , 50+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {5+127+16*5         ,1         , 40+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {6+127+16*5         ,1         , 42+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {7+127+16*5         ,1         , 32+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {8+127+16*5         ,1         , 34+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {9+127+16*5         ,1         , 36+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {10+127+16*5        ,1         , 38+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {11+127+16*5        ,1         , 44+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {12+127+16*5        ,1         , 46+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {13+127+16*5        ,1         , 52+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {14+127+16*5        ,1         , 54+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {15+127+16*5        ,1         , 60+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {16+127+16*5        ,1         , 62+64*3     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},

    /*line card 7 , every odd card refer to line card 7  */
     {1+127+16*6         ,1         , 24          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {2+127+16*6         ,1         , 26          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {3+127+16*6         ,1         , 16          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {4+127+16*6         ,1         , 18          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {5+127+16*6         ,1         , 8           , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {6+127+16*6         ,1         , 10          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {7+127+16*6         ,1         , 0           , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {8+127+16*6         ,1         , 2           , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {9+127+16*6         ,1         , 4           , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {10+127+16*6        ,1         , 6           , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {11+127+16*6        ,1         , 12          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {12+127+16*6        ,1         , 14          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {13+127+16*6        ,1         , 20          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {14+127+16*6        ,1         , 22          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {15+127+16*6        ,1         , 28          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {16+127+16*6        ,1         , 30          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},

     /*line card 8 , every even card 0,2,4,6 refer to line card 8  */
     {1+127+16*7         ,1         , 56          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {2+127+16*7         ,1         , 58          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {3+127+16*7         ,1         , 48          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {4+127+16*7         ,1         , 50          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {5+127+16*7         ,1         , 40          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {6+127+16*7         ,1         , 42          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {7+127+16*7         ,1         , 32          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {8+127+16*7         ,1         , 34          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {9+127+16*7         ,1         , 36          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {10+127+16*7        ,1         , 38          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {11+127+16*7        ,1         , 44          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {12+127+16*7        ,1         , 46          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {13+127+16*7        ,1         , 52          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {14+127+16*7        ,1         , 54          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {15+127+16*7        ,1         , 60          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2},
     {16+127+16*7        ,1         , 62          , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DMA_STEP_OF_2}
};
static GT_U32   actualNum_falcon_12_8_rd_defaultMap_256_port_mode = sizeof(falcon_12_8_rd_defaultMap_256_port_mode)/sizeof(falcon_12_8_rd_defaultMap_256_port_mode[0]) ;


/* mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_12_8_defaultMap[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 273 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           ,  273/*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* due to CPSS bug : CPSS-7899 : Falcon : CPSS fail when 'first DMA' in DP was not mapped in 'port mapping' but others from that DP mapped */
    /* we need for the additional 7 CPU ports , the existence of next ports : in DP[8..11].
    meaning DMA ports : 64,72,80,88 */
    /* physical ports 64..109 , mapped to MAC 64..154 (in steps of 2) */
    ,{64              ,NUM_PORTS_64_TO_119-10,  64  , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, DMA_STEP_OF_2}
    /* physical ports 110..119 , mapped to MAC 176..216 (in steps of 4) */
    ,{110             ,                    10,  176 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, DMA_STEP_OF_4}

    /* map 'CPU SDMA ports' , DMA 50..52 of MG [3..5] */
    ,{121              , 3           , 275/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' , DMA 54..57 of MG [10..13] */
    ,{124              , 4          , 282/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_12_8_defaultMap = sizeof(falcon_12_8_defaultMap)/sizeof(falcon_12_8_defaultMap[0]);

/* mapping of falcon physical ports to MAC ports (and TXQ ports) */
static FALCON_PORT_MAP_STC falcon_12_8_defaultMap_128_modeTwoNetCpuPorts[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 273 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           ,  273/*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* due to CPSS bug : CPSS-7899 : Falcon : CPSS fail when 'first DMA' in DP was not mapped in 'port mapping' but others from that DP mapped */
    /* we need for the additional 7 CPU ports , the existence of next ports : in DP[8..11].
    meaning DMA ports : 64,72,80,88 */
    /* physical ports 64..109 , mapped to MAC 64..154 (in steps of 2) */
    ,{64              ,NUM_PORTS_64_TO_119-10,  64  , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, DMA_STEP_OF_2}
    /* physical ports 110..118 , mapped to MAC 176..212 (in steps of 4) */
    ,{110             ,                    9,  176 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, DMA_STEP_OF_4}

    /* map 'Ethernet CPU ports' (non-SDMA) */
    /* physical port 119 , mapped to MAC 257 */
    ,{119             , 1           ,  257    , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* physical port 120 , mapped to MAC 258 NOTE: muxed with DMA 272 !  */
    ,{120             , 1           ,  258    , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

    /* map 'CPU SDMA ports' , DMA 50..52 of MG [3..5] */
    ,{121              , 3           , 275/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' , DMA 54..57 of MG [10..13] */
    ,{124              , 4          , 282/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_12_8_defaultMap_128_modeTwoNetCpuPorts = sizeof(falcon_12_8_defaultMap_128_modeTwoNetCpuPorts)/sizeof(falcon_12_8_defaultMap_128_modeTwoNetCpuPorts[0]);


/* 1024 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports). include remote ports configurations */
static FALCON_PORT_MAP_STC falcon_12_8_defaultMap_1024_port_mode_remote_ports[] =
{
    /* native ports: first 29 MACs : physical ports 0..28 , mapped to MAC 0..28 */
     {0              , 28          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP}
     /* skip physicals 29..64 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 273 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           ,  273/*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
      /* remote physical ports 600..609 , mapped to MAC 65 (DP 8) , with phisical port 75 */
    ,{600             , 10   ,  65     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,75}
    /* remote physical ports 610..619 , mapped to MAC 33 (DP 4) , with phisical port 76 */
    ,{610             , 10   ,  33       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,76}
    /* remote physical ports 620..629 , mapped to MAC 66 (DP 8), with phisical port 77 */
    ,{620             , 9   ,  160       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , 77}
    /* remote physical ports 630..639 , mapped to MAC 33 , with phisical port 78 */
    ,{630             , 9   ,  35       , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E , 78}
    /* next 67-77 MACs : physical ports 64..74 */
     ,{64              ,10     ,     67     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* next 77- 97 MACs : physical ports 100..119 */
     ,{100              ,20     ,     77     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' , DMA 50..52 of MG [3..5] */
    ,{121              , 3           , 275/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' , DMA 54..57 of MG [10..13] */
    ,{124              , 4          , 282/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};

static GT_U32   actualNum_falcon_12_8_defaultMap_1024_port_mode_remote_ports = sizeof(falcon_12_8_defaultMap_1024_port_mode_remote_ports)/sizeof(falcon_12_8_defaultMap_1024_port_mode_remote_ports[0]);

/* 512 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports). include remote ports configurations */
static FALCON_PORT_MAP_STC falcon_12_8_defaultMap_512_port_mode[] =
{
    /* physical ports 0..61 , mapped to MAC 0..61 */
     {0               , 61          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 273 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           ,  273/*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* due to CPSS bug : CPSS-7899 : Falcon : CPSS fail when 'first DMA' in DP was not mapped in 'port mapping' but others from that DP mapped */
    /* we need for the additional 15 CPU ports , the existence of next ports : in DP[8..11].*/
    /* physical ports 64..257 , mapped to MAC 59..255 (in steps of 1) */
    ,{64              ,(255 - 61 + 1),  61  , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, IGNORE_DMA_STEP}
    /* map 'Ethernet CPU ports' (non-SDMA) */
    /* physical port 258 , mapped to MAC 257 */
    ,{259             , 1           ,  257    , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' 260..273 (no MAC/SERDES) DMA 274..287 (MG[2..15])*/
    ,{261              , 14          ,  274/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' 274 (no MAC/SERDES) DMA 272 (MG[0])*/
    ,{275              , 1           ,  272/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};

static GT_U32   actualNum_falcon_12_8_defaultMap_512_port_mode =
    sizeof(falcon_12_8_defaultMap_512_port_mode)
    /sizeof(falcon_12_8_defaultMap_512_port_mode[0]);

/* 512 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports). include remote ports configurations */
static FALCON_PORT_MAP_STC falcon_12_8_defaultMap_512_port_modeTwoNetCpuPorts[] =
{
    /* physical ports 0..61 , mapped to MAC 0..61 */
     {0               , 61          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 273 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           ,  273/*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* due to CPSS bug : CPSS-7899 : Falcon : CPSS fail when 'first DMA' in DP was not mapped in 'port mapping' but others from that DP mapped */
    /* we need for the additional 15 CPU ports , the existence of next ports : in DP[8..11].*/
    /* physical ports 64..257 , mapped to MAC 59..255 (in steps of 1) */
    ,{64              ,(255 - 61 + 1),  61  , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, IGNORE_DMA_STEP}
    /* map 'Ethernet CPU ports' (non-SDMA) */
    /* physical port 258 , mapped to MAC 257 */
    ,{259             , 1           ,  257    , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    , {260             , 1           ,  258    , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' 260..273 (no MAC/SERDES) DMA 274..287 (MG[2..15])*/
    ,{261              , 14          ,  274/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
#if 0
        /* muxed with DMA 257 : map 'CPU SDMA ports' 280 (no MAC/SERDES) DMA 258 (MG[0])*/
    ,{274              , 1           ,  272/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
#endif
};

static GT_U32   actualNum_falcon_12_8_defaultMap_512_port_modeTwoNetCpuPorts =
    sizeof(falcon_12_8_defaultMap_512_port_modeTwoNetCpuPorts)
    /sizeof(falcon_12_8_defaultMap_512_port_modeTwoNetCpuPorts[0]);

static FALCON_PORT_MAP_STC falcon_12_8_port_mode_25[] =
{
    /* DP #0 -3  */
      {0               , 1          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{8               , 1          ,   8       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{16               , 1          ,   16       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{24               , 1          ,   24       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     /* DP #4 physical ports  4 (MAC 32) connected to 48 remote ports 200-347 */
    ,{255             , 48   ,         32     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,32}
     /* DP #5 physical ports  5 (MAC 40) connected to 48 remote ports 248-395 */
    ,{303             , 48   ,         40     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,40}
     /* DP #6 physical ports  6 (MAC 48) connected to 48 remote ports 296-343 */
    ,{351             , 48   ,         48     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,48}
     /* DP #7 physical ports  7 (MAC 56 ) connected to 48 remote ports 344-391 */
    ,{399             , 48   ,         56     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,56}
     /* DP #8 physical ports  8 (MAC 64 ) connected to 48 remote ports 392-439 */
    ,{447             , 48   ,         64     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,69}
     /* DP #9 physical ports 9 (MAC 72) connected to 48 remote ports 440-487 */
    ,{495             , 48   ,         72     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,77}
     /* DP #10 physical ports  10 (MAC 80) connected to 48 remote ports 488-535 */
    ,{543             , 48   ,         80     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,85}
     /* DP #11 physical ports  11 (MAC 88) connected to 48 remote ports 536-583 */
    ,{591             , 48   ,         88     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,93}
     /* DP #12 -15  */
     ,{101               , 1          , 96       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{109               , 1          , 104       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{117               , 1          , 112       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{125               , 1          , 120       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}

     ,{133               , 1          , 128       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{141               , 1          , 136       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{149               , 1          , 144       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{157               , 1          , 152       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}

     /* DP #20 physical ports  20 (MAC 160) connected to 48 remote ports 584-631 */
    ,{639             , 48   ,         160     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,165}
     /* DP #21 physical ports  21 (MAC 168) connected to 48 remote ports 632-679 */
    ,{687             , 48   ,         168     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,173}
     /* DP #22 physical ports  22 (MAC 176) connected to 48 remote ports 680-727 */
    ,{735             , 48   ,         176     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,181}
     /* DP #23 physical ports  23(MAC 184 ) connected to 48 remote ports 728-775 */
    ,{783             , 48   ,         184     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,189}
     /* DP #24 physical ports  24(MAC 192 ) connected to 48 remote ports 776-823 */
    ,{831             , 48   ,         192     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,197}
     /* DP #25 physical ports 25(MAC 200) connected to 48 remote ports 824-871 */
    ,{879             , 48   ,         200     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,205}
     /* DP #26 physical ports  26(MAC 208) connected to 48 remote ports 872-919 */
    ,{927             , 48   ,         208     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,213}
     /* DP #27 physical ports  27 (MAC 216) connected to 48 remote ports 920-967 */
    ,{975             , 48   ,         216     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,221}

     ,{229               , 1          , 224      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{237               , 1          , 232       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{245               , 1          , 240       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{253               , 1          , 248       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}



    ,{CPU_PORT/*63*/  , 1           ,  273 /*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    ,{190             , 3          ,  274/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};

static GT_U32   actualNum_falcon_12_8_port_mode_25 =
    sizeof(falcon_12_8_port_mode_25)
    /sizeof(falcon_12_8_port_mode_25[0]);

#define PRV_B2B_PREPARE_MAC(_oldMac) ((256-32+ _oldMac)%256)

static FALCON_PORT_MAP_STC falcon_12_8_port_mode_25_b2b[] =
{
    /* DP #0 -3  */
      {0               , 1          ,   PRV_B2B_PREPARE_MAC(0)       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{8               , 1          ,   PRV_B2B_PREPARE_MAC(8)       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{16               , 1          ,  PRV_B2B_PREPARE_MAC(16)       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{24               , 1          ,  PRV_B2B_PREPARE_MAC(24)       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     /* DP #4 physical ports  4 (MAC 32) connected to 48 remote ports 200-347 */
    ,{255             , 48   ,         PRV_B2B_PREPARE_MAC(32)     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,32}
     /* DP #5 physical ports  5 (MAC 40) connected to 48 remote ports 248-395 */
    ,{303             , 48   ,         PRV_B2B_PREPARE_MAC(40)     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,40}
     /* DP #6 physical ports  6 (MAC 48) connected to 48 remote ports 296-343 */
    ,{351             , 48   ,         PRV_B2B_PREPARE_MAC(48)     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,48}
     /* DP #7 physical ports  7 (MAC 56 ) connected to 48 remote ports 344-391 */
    ,{399             , 48   ,         PRV_B2B_PREPARE_MAC(56)     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,56}
     /* DP #8 physical ports  8 (MAC 64 ) connected to 48 remote ports 392-439 */
    ,{447             , 48   ,         PRV_B2B_PREPARE_MAC(64)     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,69}
     /* DP #9 physical ports 9 (MAC 72) connected to 48 remote ports 440-487 */
    ,{495             , 48   ,         PRV_B2B_PREPARE_MAC(72)     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,77}
     /* DP #10 physical ports  10 (MAC 80) connected to 48 remote ports 488-535 */
    ,{543             , 48   ,         PRV_B2B_PREPARE_MAC(80)     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,85}
     /* DP #11 physical ports  11 (MAC 88) connected to 48 remote ports 536-583 */
    ,{591             , 48   ,         PRV_B2B_PREPARE_MAC(88)     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,93}
     /* DP #12 -15  */
     ,{101               , 1          , PRV_B2B_PREPARE_MAC(96)      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{109               , 1          , PRV_B2B_PREPARE_MAC(104)       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{117               , 1          , PRV_B2B_PREPARE_MAC(112)       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{125               , 1          , PRV_B2B_PREPARE_MAC(120)       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}

     ,{133               , 1          , PRV_B2B_PREPARE_MAC(128)       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{141               , 1          , PRV_B2B_PREPARE_MAC(136)       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{149               , 1          , PRV_B2B_PREPARE_MAC(144)       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{157               , 1          , PRV_B2B_PREPARE_MAC(152)       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}

     /* DP #20 physical ports  20 (MAC 160) connected to 48 remote ports 584-631 */
    ,{639             , 48   ,         PRV_B2B_PREPARE_MAC(160)     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,165}
     /* DP #21 physical ports  21 (MAC 168) connected to 48 remote ports 632-679 */
    ,{687             , 48   ,         PRV_B2B_PREPARE_MAC(168)     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,173}
     /* DP #22 physical ports  22 (MAC 176) connected to 48 remote ports 680-727 */
    ,{735             , 48   ,         PRV_B2B_PREPARE_MAC(176)     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,181}
     /* DP #23 physical ports  23(MAC 184 ) connected to 48 remote ports 728-775 */
    ,{783             , 48   ,         PRV_B2B_PREPARE_MAC(184)     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,189}
     /* DP #24 physical ports  24(MAC 192 ) connected to 48 remote ports 776-823 */
    ,{831             , 48   ,         PRV_B2B_PREPARE_MAC(192)     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,197}
     /* DP #25 physical ports 25(MAC 200) connected to 48 remote ports 824-871 */
    ,{879             , 48   ,         PRV_B2B_PREPARE_MAC(200)     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,205}
     /* DP #26 physical ports  26(MAC 208) connected to 48 remote ports 872-919 */
    ,{927             , 48   ,         PRV_B2B_PREPARE_MAC(208)     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,213}
     /* DP #27 physical ports  27 (MAC 216) connected to 48 remote ports 920-967 */
    ,{975             , 48   ,         PRV_B2B_PREPARE_MAC(216)     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,221}

      ,{229               , 1          , PRV_B2B_PREPARE_MAC(224)       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{237               , 1          , PRV_B2B_PREPARE_MAC(232)       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{245               , 1          , PRV_B2B_PREPARE_MAC(240)       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}
     ,{253               , 1          , PRV_B2B_PREPARE_MAC(248)       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,1}


    ,{CPU_PORT/*63*/  , 1           ,  273/*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    ,{190             , 3          ,  274/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};

static GT_U32   actualNum_falcon_12_8_port_mode_25_b2b =
    sizeof(falcon_12_8_port_mode_25_b2b)
    /sizeof(falcon_12_8_port_mode_25_b2b[0]);


/* active ravens are 0,2,5,7 */
static FALCON_PORT_MAP_STC falcon_64X50G_defaultMap[] =
{
    /* physical ports 0..15 , mapped to MAC 0..15 */
     {0               , 16          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* physical ports 16..31 , mapped to MAC 32..47 */
    ,{16              , 16          ,  32       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* physical ports 32..47 , mapped to MAC 80..95 */
    ,{32              , 16          ,  80       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* physical ports 48..59 , mapped to MAC 112..123 */
    ,{48              , 12          , 112       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* physical ports 64..69 , mapped to MAC 124..127 */
    ,{64              , 4           , 124       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* 10G MACs 133,135 used, MAC 131 in missed Raven3, MAC 132 in missed Raven4 */
    ,{90              , 1           , 133       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E    ,IGNORE_DMA_STEP}
    ,{91              , 1           , 135       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 136 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           , 136, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* 138 muxed with 131 of missed Raven3 - can be used */
    /* map 'CPU SDMA ports' 100 - 106 to DMA 137 - 143 */
    ,{100             ,  7          , 137/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}

};

static GT_U32   actualNum_falcon_64X50G_defaultMap =
    sizeof(falcon_64X50G_defaultMap)
    /sizeof(falcon_64X50G_defaultMap[0]);

/* active ravens are 0,2,5,7 */
static FALCON_PORT_MAP_STC falcon_6_4_max_ports_64_defaultMap[] =
{
    /* physical ports 0..15 , mapped to MAC 0..15 */
     {0               , 16          , 0        , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP}
    /* physical ports 16..31 , mapped to MAC 32..47 */
    ,{16              , 16          , 32       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP}
    /* physical ports 32..46 , mapped to MAC 80..94 */
    ,{32              , 15          , 80       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,IGNORE_DMA_STEP}
    /* physical ports 47..53 , mapped to MAC 112..118 */
    ,{47              , 7           , 112       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 136 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           , 136, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E           ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' 54 - 60 to DMA 137 - 143 */
    ,{54              , 7           , 137/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};

static GT_U32   actualNum_falcon_6_4_max_ports_64_defaultMap =
    sizeof(falcon_6_4_max_ports_64_defaultMap)
    /sizeof(falcon_6_4_max_ports_64_defaultMap[0]);

/* active ravens are 0,2,5,6,7 */
static FALCON_PORT_MAP_STC armstrong2_80x25g_defaultMap[] =
{
    /* physical ports 0..15 , mapped to MAC 0..15 */
     {0               , 16          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* physical ports 16..31 , mapped to MAC 32..47 */
    ,{16              , 16          ,  32       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* physical ports 32..59 , mapped to MAC 80..107 */
    ,{32              , 28          ,  80       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* skip physicals 60..62 */
    /* physical ports 64..83 , mapped to MAC 108..127 */
    ,{64              , 20          , 108       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* 10G MACs 128,135 used, MAC 131 in missed Raven3, MAC 132 in missed Raven4 */
    ,{90              ,  1          , 128       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    ,{91              ,  1          , 135       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 136 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           ,  136, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* 138 muxed with 131 of missed Raven3 - can be used */
    /* map 'CPU SDMA ports' 100 - 106 to DMA 137 - 143 */
    ,{100             ,  7          , 137/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};

static GT_U32   actualNum_armstrong2_80x25g_defaultMap =
    sizeof(armstrong2_80x25g_defaultMap)
    /sizeof(armstrong2_80x25g_defaultMap[0]);


static FALCON_PORT_MAP_STC armstrong2_6_4_max_ports_64_defaultMap[] =
{
    /* physical ports 0..15 , mapped to MAC 0..15 */
     {0               , 16          , 0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, IGNORE_DMA_STEP}
    /* physical ports 16..30 , mapped to MAC 32..46 */
    ,{16              , 15          , 32       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, IGNORE_DMA_STEP}
    /* physical ports 31..53 , mapped to MAC 80..103 */
    ,{31              , 23          , 80       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, IGNORE_DMA_STEP}
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 136 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           , 136, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E           ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' 54 - 60 to DMA 137 - 143 */
    ,{54              , 7           , 137/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};

static GT_U32   actualNum_armstrong2_6_4_max_ports_64_defaultMap =
    sizeof(armstrong2_6_4_max_ports_64_defaultMap)
    /sizeof(armstrong2_6_4_max_ports_64_defaultMap[0]);

/* active ravens are 0,2,4,5,7 */
static FALCON_PORT_MAP_STC falcon_80X50G_max_ports_64_defaultMap[] =
{
    /* physical ports 0..15 ,   mapped to MAC 0..15 */
     {0               , 16          , 0         , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, IGNORE_DMA_STEP}
    /* physical ports 16..31 ,  mapped to MAC 32..47 */
    ,{16              , 16          , 32        , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, IGNORE_DMA_STEP}
    /* physical ports 32..47 ,  mapped to MAC 80..95 */
    ,{32              , 16          , 80        , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, IGNORE_DMA_STEP}
    /* physical ports 48..53,   mapped to MAC 112..117 */
    ,{48              , 6           , 112        , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, IGNORE_DMA_STEP}
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 136 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           , 136, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E           ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' 54 - 60 to DMA 137 - 143 */
    ,{54              , 7           , 137/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};

static GT_U32   actualNum_falcon_80X50G_max_ports_64_defaultMap =
    sizeof(falcon_80X50G_max_ports_64_defaultMap)
    /sizeof(falcon_80X50G_max_ports_64_defaultMap[0]);

/* active ravens are 7, 0, 2, 4, 5 */
static FALCON_PORT_MAP_STC falcon_80X50G_max_ports_128_defaultMap[] =
{
    /* physical ports 0..15 , mapped to MAC 0..15 */
     {0               ,16          , 0*16       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* physical ports 16..31 , mapped to MAC 32..47 */
    ,{16              ,16          , 2*16       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* physical ports 32..47 , mapped to MAC 64..79 */
    ,{32              ,16          , 4*16       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* physical ports 48..59 , mapped to MAC 80..91 */
    ,{48              ,12          , 5*16       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* skip physicals 60..62 */
    /* physical ports 64..67 , mapped to MAC 92..95 */
    ,{64              , 4          , 5*16+12       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* physical ports 68..83 , mapped to MAC 112..127 */
    ,{68              ,16          , 7*16       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* 10G CPU network ports: MACs 133, 135  */
    ,{90              , 1          , 133       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    ,{91              , 1          , 135       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 136 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1          , 136       , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* 138 muxed with 131 of missed Raven3 - can be used */
    /* map 'CPU SDMA ports' 100 - 106 to DMA 137 - 143 */
    ,{100             , 7          , 137/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};

static GT_U32   actualNum_falcon_80X50G_max_ports_128_defaultMap =
    sizeof(falcon_80X50G_max_ports_128_defaultMap)
    /sizeof(falcon_80X50G_max_ports_128_defaultMap[0]);

static FALCON_PORT_MAP_STC falcon_12_8_port_mode_256_b2b_modeSonic128MacPorts[] =
{
    /* physicals 128..255 (128 ports) mapped to Macs 0,2,4,5...254 */
     {128              , 128        ,   0      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E ,DMA_STEP_OF_2}
    /* physical 0,1 (2 ports) are mapped to Macs 257,258 (the reduces ports) */
    ,{0               ,  2          ,  257     , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    ,{CPU_PORT  /*63*/  , 1          ,  273    , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports'  : 274....287 only 14 , because totally up to 16 ,but 272 of MG0 muxed with MAC 258 */
    ,{64              , 14          ,  274     , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    , IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_12_8_port_mode_256_b2b_modeSonic128MacPorts =
    sizeof(falcon_12_8_port_mode_256_b2b_modeSonic128MacPorts)
    /sizeof(falcon_12_8_port_mode_256_b2b_modeSonic128MacPorts[0]);

static PortInitList_STC falcon_12_8_portInitlist_port_mode_256_b2b_modeSonic128MacPorts[] =
{
     /* ports 0,1 (the MAC ports that supports 10G) - the reduced ports */
     { PORT_LIST_TYPE_INTERVAL,  {0/*startPort*/,1/*stopPort*/,1/*step*/, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }
     /* ports 128..255 in steps of 8 set as 400G : 32 ports of 400G that each
        can break to :
        1. 400G                                     max of 32 *400 = 12.8 T
        2. 4 * 10           (only even MACs)        max of 128*10  =  1.28T
        3. 4 * 25           (only even MACs)        max of 128*25  =  3.2 T
        4. 2 * 100G R4                              max of 64*100  =  6.4 T
        5. 2 * 40G R4                               max of 64*40   =  2.56T
    */
    ,{ PORT_LIST_TYPE_INTERVAL,  {128/*startPort*/,255/*stopPort*/, 4/*step*/, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_400G_E,  CPSS_PORT_INTERFACE_MODE_KR8_E }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* CPU NW port as cascade with 8 remotes (8 queues each).Ports 300 and 400 are NW CPU ports with 64 queues(8 ports * 8 queues)*/
static FALCON_PORT_MAP_STC falcon_12_8_Hqos_Map[] =
{
     /* physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 59          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 273 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           ,  273/*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}

    /* Below 2 CPU NW ports with 8 remotes*/

    /* DP #2*/
    ,{300             , 8   ,         256     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,256}
    /* DP #4*/
    ,{400             , 8   ,         257     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,257}


    /* physical ports 64..109 , mapped to MAC 64..154 (in steps of 2) */
    ,{64              ,NUM_PORTS_64_TO_119-10,  64  , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, DMA_STEP_OF_2}

    /* physical ports 110..119 , mapped to MAC 176..216 (in steps of 4) */
    ,{110             ,                    10,  176 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, DMA_STEP_OF_4}


   /*the below is required for port delete WA*/

     /* map 'CPU SDMA ports' , DMA 50..52 of MG [3..5] */
    ,{121              , 3           , 275/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' , DMA 54..57 of MG [10..13] */
    ,{124              , 4          , 282/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}

};
static GT_U32   actualNum_falcon_12_8_Hqos_Map = sizeof(falcon_12_8_Hqos_Map)/sizeof(falcon_12_8_Hqos_Map[0]);


static FALCON_PORT_MAP_STC falcon_6_4_Hqos_Map[] =
{
    /* physical ports 0..58 , mapped to MAC 0..58 */
     {0               , 58          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* skip physicals 59..62 */


     /* Below 2 CPU NW ports with 8 remotes*/

     /* DP #2*/
     ,{300             , 8   ,         129     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,129}
     /* DP #4*/
     ,{400             , 8   ,         131     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,131}

    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 136 */
    ,{CPU_PORT/*63*/  , 1           ,  136/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* physical ports 64..119 , mapped to MAC 59..116 */
    ,{64              ,NUM_PORTS_64_TO_119,  59 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
};

static GT_U32   actualNum_falcon_6_4_Hqos_Map = sizeof(falcon_6_4_Hqos_Map)/sizeof(falcon_6_4_Hqos_Map[0]);

static FALCON_PORT_MAP_STC falcon_rpp_with_empty_tile_Map[] =
{
    /* DP #0*/
    {300             , 8   ,         0     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,0}
    /* DP #2*/
    ,{400             , 8   ,         16     , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E ,16}

     /* skip physicals 59..62 */
    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 273 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           ,  273/*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}

   /*the below is required for port delete WA*/

     /* map 'CPU SDMA ports' , DMA 50..52 of MG [3..5] */
    ,{121              , 3           , 275/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports' , DMA 54..57 of MG [10..13] */
    ,{124              , 2          , 282/*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}

};

static GT_U32   actualNum_falcon_rpp_with_empty_tile_Map = sizeof(falcon_rpp_with_empty_tile_Map)/sizeof(falcon_rpp_with_empty_tile_Map[0]);

/* active ravens are 0,1,2,5,7,8,10,13,14,15 */
static FALCON_PORT_MAP_STC falcon_160X50G_64_defaultMap[] =
{
    /* physical ports   0..47 , mapped to MAC  0..47        */
     {0               , 48         ,   0      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
     /* physical ports  48..51 , mapped to MAC 80..83        */
    ,{48              , 4          ,  80      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 273 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           , 273 /*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E, IGNORE_DMA_STEP}

    /* 10G CPU network ports: MACs 257, 258  */
    ,{54              , 1           , 257       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    ,{55              , 1           , 258       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

    /* map 'CPU SDMA ports', DMA 278..279 of MG [6..7]      */
    ,{56              , 2           , 278 /*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E  ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports', DMA 284..287 of MG [12..15]    */
    ,{58              , 4           , 284 /*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E  ,IGNORE_DMA_STEP}
};
static GT_U32   actualNum_falcon_160X50G_64_defaultMap =
    sizeof(falcon_160X50G_64_defaultMap)/sizeof(falcon_160X50G_64_defaultMap[0]);


/* active ravens are 0,1,2,5,7,8,10,13,14,15 */
static FALCON_PORT_MAP_STC falcon_160X50G_128_defaultMap[] =
{
    /* physical ports   0..47 , mapped to MAC  0..47        */
     {0               , 48          ,   0      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* physical ports  48..59 , mapped to MAC 80..91        */
    ,{48              , 12          ,  80      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 273 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           , 273 /*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E, IGNORE_DMA_STEP}

    /* physical ports  64..67 ,     mapped to MAC 92..95    */
    ,{64              , 4           ,  92      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* physical ports  68..99 , mapped to MAC 112..143      */
    ,{68              , 32          , 112      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* physical ports  100..115 , mapped to MAC 160..175      */
    ,{100             , 16          , 160      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

    /* 10G CPU network ports: MACs 257, 258  */
    ,{116             , 1           , 257       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    ,{117             , 1           , 258       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

    /* map 'CPU SDMA ports', DMA 278..279 of MG [6..7]      */
    ,{120             , 2           , 278 /*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E  ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports', DMA 284..287 of MG [12..15]    */
    ,{122             , 4           , 284 /*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E  ,IGNORE_DMA_STEP}
};

static GT_U32   actualNum_falcon_160X50G_128_defaultMap =
    sizeof(falcon_160X50G_128_defaultMap)/sizeof(falcon_160X50G_128_defaultMap[0]);

/* active ravens are 0,1,2,5,7,8,10,13,14,15 */
static FALCON_PORT_MAP_STC falcon_160X50G_max_ports_defaultMap[] =
{
    /* physical ports   0..47 , mapped to MAC  0..47        */
     {0               , 48          ,   0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* physical ports  48..59 , mapped to MAC 80..91        */
    ,{48              , 12          ,  80      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

    /* map 'CPU SDMA port' 63 (no MAC/SERDES) DMA 273 of MG1 !!! not MG0 , because DP[4] (connected to MG0) may serve 'CPU network port' */
    ,{CPU_PORT/*63*/  , 1           , 273 /*DMA-MG1!!!*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E, IGNORE_DMA_STEP}

    /* physical ports  64..67 ,     mapped to MAC 92..95    */
    ,{64              , 4           ,  92      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* physical ports  68..99 , mapped to MAC 112..143      */
    ,{68              , 32          , 112      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* physical ports  100..115 , mapped to MAC 160..175      */
    ,{100             , 16          , 160      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    /* physical ports  116..163 , mapped to MAC 208..255      */
    ,{116             , 48          , 208      , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

    /* 10G CPU network ports: MACs 257, 258  */
    ,{164             , 1           , 257       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    ,{165             , 1           , 258       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}

    /* map 'CPU SDMA ports', DMA 278..279 of MG [6..7]      */
    ,{166             , 2           , 278 /*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E  ,IGNORE_DMA_STEP}
    /* map 'CPU SDMA ports', DMA 284..287 of MG [12..15]    */
    ,{168             , 4           , 284 /*DMA*/, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E  ,IGNORE_DMA_STEP}
};

static GT_U32   actualNum_falcon_160X50G_max_ports_defaultMap =
    sizeof(falcon_160X50G_max_ports_defaultMap)/sizeof(falcon_160X50G_max_ports_defaultMap[0]);

#define MAX_MAPPED_PORTS_CNS    1024
static  GT_U32  actualNum_cpssApi_falcon_defaultMap = 0;/*actual number of valid entries in cpssApi_falcon_defaultMap[] */
static CPSS_DXCH_PORT_MAP_STC   cpssApi_falcon_defaultMap[MAX_MAPPED_PORTS_CNS];

#ifdef GM_USED
GT_U32  useSinglePipeSingleDp = 1;
#else /*!GM_USED*/
GT_U32  useSinglePipeSingleDp = 0;
#endif /*!GM_USED*/
/* allow to modify the flag from the terminal */
GT_STATUS falcon_useSinglePipeSingleDp_Set(IN GT_BOOL useSingleDp)
{
    useSinglePipeSingleDp = useSingleDp;
    return GT_OK;
}
/* check if the mac number exists in the list */
static GT_BOOL isMacPortInList(
    IN FALCON_PORT_MAP_STC *currAppDemoInfoPtr ,
    IN GT_U32 numEntries,
    IN GT_U32 macNumber
)
{
    GT_U32  ii,jj;
    GT_U32  temp_startGlobalDmaNumber;
    GT_U32  dmaPortOffset;

    /* remove from the mapping the SDMAs ports that collide with the 'needed' CPU ports */
    for (ii = 0 ; ii < numEntries; ii++ , currAppDemoInfoPtr++)
    {
        dmaPortOffset = 0;
        for(jj = 0 ; jj < currAppDemoInfoPtr->numOfPorts; jj++)
        {
            if(currAppDemoInfoPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
            {
                /* not needed */
                continue;
            }

            temp_startGlobalDmaNumber       = currAppDemoInfoPtr->startGlobalDmaNumber + dmaPortOffset;

            if(temp_startGlobalDmaNumber == macNumber)
            {
                return GT_TRUE;
            }

            /* calculate dmaPortOffset */
            if((currAppDemoInfoPtr->jumpDmaPorts != DMA_NO_STEP)&&(currAppDemoInfoPtr->mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E))
            {
                if(currAppDemoInfoPtr->jumpDmaPorts >= 2)
                {
                    dmaPortOffset += currAppDemoInfoPtr->jumpDmaPorts;
                }
                else
                {
                    dmaPortOffset ++;
                }
            }
        }
    }

    return GT_FALSE;/* not found*/
}

static GT_U32   maxCpuSdma = 0xFFFFFFFF;
void falcon_max_cpu_sdma_set(IN GT_U32 value)
{
    maxCpuSdma = value;
}

static GT_U32   modeSonic32MacPorts = 0;

/* 64 ports mode: mapping of falcon physical ports to MAC ports (and TXQ ports).
   for B2B of Sonic */
static FALCON_PORT_MAP_STC falcon_12_8_defaultMap_64_port_mode_sonic[] =
{
     {0               , 4          ,    0       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    ,{4               , 4          ,    8       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    ,{8               , 4          ,   16       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    ,{12              , 4          ,   24       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    ,{16              , 4          ,   32       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    ,{20              , 4          ,   40       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    ,{24              , 4          ,   48       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    ,{28              , 4          ,   56       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    ,{32              , 1          ,  258       , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,IGNORE_DMA_STEP}
    ,{CPU_PORT/*63*/  , 1          ,  276       , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
    ,{60              , 3          ,  273       , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    ,IGNORE_DMA_STEP}
};

static GT_U32   actualNum_falcon_12_8_defaultMap_64_port_mode_sonic =
    sizeof(falcon_12_8_defaultMap_64_port_mode_sonic)
    /sizeof(falcon_12_8_defaultMap_64_port_mode_sonic[0]);

static PortInitList_STC falcon_12_8_portInitlist_64_port_mode_sonic[] =
{
     /* ports 0,1 (the MAC ports that supports 10G) - the reduced ports */
     { PORT_LIST_TYPE_INTERVAL,  {0/*startPort*/,32/*stopPort*/,1/*step*/, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


static PortInitList_STC falcon_12_8_portInitlist_hqos[] =
{
     { PORT_LIST_TYPE_LIST,      {0,1,2,3,11,18,20,32,35,36,54,55,56,57,58, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
     /* ports 64..127*/
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,127,1,                            APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }

    ,{ PORT_LIST_TYPE_LIST,      {256,257, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

     /* ports 300..307*/
    ,{ PORT_LIST_TYPE_INTERVAL,  {300,307,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,  CPSS_PORT_INTERFACE_MODE_REMOTE_E}

     /* ports 400..407*/
    ,{ PORT_LIST_TYPE_INTERVAL,  {400,407,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,  CPSS_PORT_INTERFACE_MODE_REMOTE_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }

};

static PortInitList_STC falcon_12_8_rpp_empty_tile[] =
{
     { PORT_LIST_TYPE_LIST,      {0,16,APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }

     /* ports 300..307*/
    ,{ PORT_LIST_TYPE_INTERVAL,  {300,307,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,  CPSS_PORT_INTERFACE_MODE_REMOTE_E}

     /* ports 400..407*/
    ,{ PORT_LIST_TYPE_INTERVAL,  {400,407,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,  CPSS_PORT_INTERFACE_MODE_REMOTE_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }

};




/* the number of CPU SDMAs that we need resources for */
static GT_U32   numCpuSdmas = 0;

/* ports in group1 can not co-exists with those of group2 */
typedef struct{
    GT_U32  macPortGroup1[2];
    GT_U32  macPortGroup2[2];
}NON_DUAL_CPU_PORTS_STC;

static NON_DUAL_CPU_PORTS_STC      falcon_6_4_non_dual_cpu_ports[]={{{131,132},{128,135}}};
static GT_U32                      use_falcon_6_4_cpu_ports_group1 = 1;
static GT_U32                      use_falcon_6_4_cpu_ports_group2 = 0;

/* fill array of cpssApi_falcon_defaultMap[] and update actualNum_cpssApi_falcon_defaultMap and numCpuSdmas */
static GT_STATUS fillDbForCpssPortMappingInfo(IN GT_U32 devNum)
{
    GT_STATUS   rc;
    FALCON_PORT_MAP_STC *currAppDemoInfoPtr = NULL;
    CPSS_DXCH_PORT_MAP_STC *currCpssInfoPtr;
    GT_U32  ii,jj;
    GT_U32  dmaPortOffset;
    GT_U32 numEntries = 0;/* number of entries in currAppDemoInfoPtr */
    GT_U32 maxPhyPorts;
    GT_U32 maxEthMacs;
    GT_U32 currBoardType;
    CPSS_PORTS_BMP_STC      skipEntriesInAppDemoPorts;/* bmp of 'entries' to SKIP (not port numbers !)*/
    CPSS_PORTS_BMP_STC      skipBmpInterface;      /* bmp of interfaces port numbers to SKIP from adding to the list of the CPSS */
    CPSS_PP_DEVICE_TYPE     devType;
    GT_U32                  value;
    GT_BOOL                 portModeExcelAllignedMapping = GT_FALSE;

    rc = appDemoBoardTypeGet(&currBoardType);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(useSinglePipeSingleDp == 1)
    {
        /*************************************************************************************/
        /* the array for the GM is already in 'CPSS format' , and not relate to below arrays */
        /* see singlePipe_singleDp_cpssApi_falcon_defaultMap[]                               */
        /*************************************************************************************/
        numCpuSdmas = 1;
        return GT_OK;
    }

    if(actualNum_cpssApi_falcon_defaultMap)
    {
        /* already initialized */
        return GT_OK;
    }

    devType = PRV_CPSS_PP_MAC(devNum)->devType;
    maxPhyPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        default:
        case 0:
        case 1:
            if((appDemoDbEntryGet("portModeExcelAllignedMapping", &value) == GT_OK) && (value != 0))
            {
                if (devType != CPSS_98EX5610_CNS ||
                    devType != CPSS_98EX5614_CNS ||
                    devType != CPSS_98CX8512_CNS ||
                    devType != CPSS_98CX8514_CNS ||
                    devType != CPSS_98CX8520_CNS ||
                    devType != CPSS_98CX8525_CNS ||
                    devType != CPSS_98CX8530_CNS ||
                    devType != CPSS_98CX8535_CNS ||
                    devType != CPSS_98CX8535_H_CNS)
                {
                    switch(value)
                    {
                       case 9:
                        portModeExcelAllignedMapping = GT_TRUE;
                        break;
                       default:
                        break;
                    }
                }
            }

            if(GT_TRUE == portModeExcelAllignedMapping)
            {
                switch(value)
                {
                   case 9:
                    currAppDemoInfoPtr = falcon_3_2_port_mode_9;
                    numEntries = actualNum_falcon_3_2_port_mode_9;
                    modeMaxMac=0;
                    break;
                   default:
                    break;
                }
            }
            else
            if (maxPhyPorts == 64)
            {
                 currAppDemoInfoPtr = &falcon_3_2_defaultMap_64_port_mode[0];
                 numEntries = actualNum_falcon_3_2_defaultMap_64_port_mode;
            }
            else if (maxPhyPorts == 256)
            {
                currAppDemoInfoPtr = &falcon_3_2_defaultMap_256_port_mode[0];
                numEntries = actualNum_falcon_3_2_defaultMap_256_port_mode;
            }
            else if (maxPhyPorts == 1024)
            {
                currAppDemoInfoPtr = &falcon_3_2_defaultMap_1024_port_mode_remote_ports[0];
                numEntries = actualNum_falcon_3_2_defaultMap_1024_port_mode_remote_ports;
            }
            else /* 128, 512 */
            {
                if(cpuPortMux2UseSdma == 0)
                {
                    currAppDemoInfoPtr = &falcon_3_2_defaultMap_TwoNetCpuPorts[0];
                    numEntries = actualNum_falcon_3_2_defaultMap_TwoNetCpuPorts;
                }
                else
                {
                    currAppDemoInfoPtr = &falcon_3_2_defaultMap[0];
                    numEntries = actualNum_falcon_3_2_defaultMap;
                }
            }
            break;
        case 2:
            if((appDemoDbEntryGet("portModeExcelAllignedMapping", &value) == GT_OK) && (value != 0))
            {
                if (devType != CPSS_98EX5610_CNS ||
                    devType != CPSS_98EX5614_CNS ||
                    devType != CPSS_98CX8512_CNS ||
                    devType != CPSS_98CX8514_CNS ||
                    devType != CPSS_98CX8520_CNS ||
                    devType != CPSS_98CX8525_CNS ||
                    devType != CPSS_98CX8530_CNS ||
                    devType != CPSS_98CX8535_CNS ||
                    devType != CPSS_98CX8535_H_CNS)
                {
                    switch(value)
                    {
                       case 9:
                        portModeExcelAllignedMapping = GT_TRUE;
                        break;
                       default:
                        break;
                    }
                }
            }

            if(GT_TRUE == portModeExcelAllignedMapping)
            {
                switch(value)
                {
                   case 9:
                    currAppDemoInfoPtr = falcon_6_4_port_mode_9;
                    numEntries = actualNum_falcon_6_4_port_mode_9;
                    modeMaxMac=0;
                    break;
                   default:
                    break;
                }
            }
            else
            if (devType == CPSS_98EX5610_CNS ||
                devType == CPSS_98CX8512_CNS ||
                devType == CPSS_98CX8530_CNS)
            {
                if (maxPhyPorts == 64)
                {
                    /* armstrong2 80X25G - 64 physical ports */
                    currAppDemoInfoPtr = armstrong2_6_4_max_ports_64_defaultMap;
                    numEntries = actualNum_armstrong2_6_4_max_ports_64_defaultMap;
                }
                else
                {
                    /* armstrong2 80X25G */
                    currAppDemoInfoPtr = &armstrong2_80x25g_defaultMap[0];
                    numEntries = actualNum_armstrong2_80x25g_defaultMap;
                }
                use_falcon_6_4_cpu_ports_group1 = 1;
                use_falcon_6_4_cpu_ports_group2 = 1;
                break;
            }
            else
            if (devType == CPSS_98CX8535_CNS ||
                devType == CPSS_98CX8535_H_CNS ||
                devType == CPSS_98CX8514_CNS ||
                devType == CPSS_98EX5614_CNS)
            {
                if (maxPhyPorts == 64)
                {
                    /* Falcon 80X25G - 64 physical ports */
                    currAppDemoInfoPtr = falcon_80X50G_max_ports_64_defaultMap;
                    numEntries = actualNum_falcon_80X50G_max_ports_64_defaultMap;
                }
                else
                {
                    /* Falcon 80X25G */
                    currAppDemoInfoPtr = falcon_80X50G_max_ports_128_defaultMap;
                    numEntries = actualNum_falcon_80X50G_max_ports_128_defaultMap;
                }
                use_falcon_6_4_cpu_ports_group1 = 1;
                use_falcon_6_4_cpu_ports_group2 = 1;
                break;
            }
            if (devType == CPSS_98CX8520_CNS ||
                devType == CPSS_98CX8525_CNS)
            {
                if (maxPhyPorts == 64)
                {
                    /* falcon_64X50G - 64 physical ports */
                    currAppDemoInfoPtr = &falcon_6_4_max_ports_64_defaultMap[0];
                    numEntries = actualNum_falcon_6_4_max_ports_64_defaultMap ;
                }
                else
                {
                    /* falcon_64X50G */
                    currAppDemoInfoPtr = &falcon_64X50G_defaultMap[0];
                    numEntries = actualNum_falcon_64X50G_defaultMap;
                }
                use_falcon_6_4_cpu_ports_group1 = 1;
                use_falcon_6_4_cpu_ports_group2 = 1;
                break;
            }

            if (mode100G)
            {
                currAppDemoInfoPtr = &falcon_6_4_100G_defaultMap[0];
                numEntries = actualNum_falcon_6_4_100G_defaultMap;
            }
            else if (maxPhyPorts == 64)
            {
                currAppDemoInfoPtr = &falcon_6_4_defaultMap_64_port_mode[0];
                numEntries = actualNum_falcon_6_4_defaultMap_64_port_mode;
            }
            else if (maxPhyPorts == 256)
            {
                currAppDemoInfoPtr = &falcon_6_4_defaultMap_256_port_mode[0];
                numEntries = actualNum_falcon_6_4_defaultMap_256_port_mode;
            }
            else if (maxPhyPorts == 1024)
            {
                currAppDemoInfoPtr = &falcon_6_4_defaultMap_1024_port_mode_remote_ports[0];
                numEntries = actualNum_falcon_6_4_defaultMap_1024_port_mode_remote_ports;
            }
            else /* 128, 512*/
            {
                currAppDemoInfoPtr = &falcon_6_4_defaultMap[0];
                numEntries = actualNum_falcon_6_4_defaultMap;
            }

            if(modeMaxMac)
            {
                if (maxPhyPorts == 256)
                {
                    currAppDemoInfoPtr = &falcon_6_4_256_port_maxMAC[0];
                    numEntries = actualNum_falcon_6_4_256_port_maxMAC;
                }
                else
                {
                    if (maxPhyPorts == 128)
                    {
                        currAppDemoInfoPtr = &falcon_6_4_maxMAC[0];
                        numEntries = actualNum_falcon_6_4_maxMAC;
                    }
                }
            }

            if((appDemoDbEntryGet("hqos", &value) == GT_OK) && (value != 0))
            {
                currAppDemoInfoPtr = &falcon_6_4_Hqos_Map[0];
                numEntries = actualNum_falcon_6_4_Hqos_Map;
            }
            break;
    case 4:
            if((appDemoDbEntryGet("portModeExcelAllignedMapping", &value) == GT_OK) && (value != 0))
            {
                switch(value)
                {
                   case 25:
                    portModeExcelAllignedMapping = GT_TRUE;
                    break;
                   default:
                    break;
                }
            }

            if((appDemoDbEntryGet("hqos", &value) == GT_OK) && (value != 0))
            {
                currAppDemoInfoPtr = &falcon_12_8_Hqos_Map[0];
                numEntries = actualNum_falcon_12_8_Hqos_Map;
            }
            else if((appDemoDbEntryGet("rppEmptyTile", &value) == GT_OK) && (value != 0))
            {
                currAppDemoInfoPtr = &falcon_rpp_with_empty_tile_Map[0];
                numEntries = actualNum_falcon_rpp_with_empty_tile_Map;
            }
            else if(modeSonic32MacPorts)
            {
                currAppDemoInfoPtr = &falcon_12_8_defaultMap_64_port_mode_sonic[0];
                numEntries = actualNum_falcon_12_8_defaultMap_64_port_mode_sonic;
            }
            else
            if(modeSonic128MacPorts)
            {
                /* only B2B supported , as this is the board in India for Sonic */
                currAppDemoInfoPtr = falcon_12_8_port_mode_256_b2b_modeSonic128MacPorts;
                numEntries = actualNum_falcon_12_8_port_mode_256_b2b_modeSonic128MacPorts;
            }
            else
            if(GT_TRUE == portModeExcelAllignedMapping)
            {
                switch(value)
                {
                   case 25:
                    if(currBoardType == APP_DEMO_FALCON_BOARD_12_8_B2B_CNS)
                    {
                        currAppDemoInfoPtr = falcon_12_8_port_mode_25_b2b;
                        numEntries = actualNum_falcon_12_8_port_mode_25_b2b;
                    }
                    else
                    {
                        currAppDemoInfoPtr = falcon_12_8_port_mode_25;
                        numEntries = actualNum_falcon_12_8_port_mode_25;
                    }

                    modeMaxMac=0;
                    break;
                   default:
                    break;
                }
            }
            else
            if(PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98CX8550_CNS)
            {
                switch (maxPhyPorts)
                {
                    case 64:
                        currAppDemoInfoPtr = &falcon_160X50G_64_defaultMap[0];
                        numEntries = actualNum_falcon_160X50G_64_defaultMap;
                        break;
                    case 128:
                        currAppDemoInfoPtr = &falcon_160X50G_128_defaultMap[0];
                        numEntries = actualNum_falcon_160X50G_128_defaultMap;
                        break;
                    default:
                        currAppDemoInfoPtr = &falcon_160X50G_max_ports_defaultMap[0];
                        numEntries = actualNum_falcon_160X50G_max_ports_defaultMap;
                        break;
                }
            }
            else
            if (maxPhyPorts == 64)
            {
                currAppDemoInfoPtr = &falcon_12_8_defaultMap_64_port_mode[0];
                numEntries = actualNum_falcon_12_8_defaultMap_64_port_mode;
            }
            else if (maxPhyPorts == 256)
            {
                currAppDemoInfoPtr = &falcon_12_8_defaultMap_256_port_mode[0];
                numEntries = actualNum_falcon_12_8_defaultMap_256_port_mode;

                if(currBoardType == APP_DEMO_FALCON_BOARD_12_8_RD_CNS)
                {
                    currAppDemoInfoPtr = &falcon_12_8_rd_defaultMap_256_port_mode[0];
                    numEntries = actualNum_falcon_12_8_rd_defaultMap_256_port_mode;
                }
            }
            else if (maxPhyPorts == 512)
            {
                if(cpuPortMux2UseSdma == 0)
                {
                    currAppDemoInfoPtr = &falcon_12_8_defaultMap_512_port_modeTwoNetCpuPorts[0];
                    numEntries = actualNum_falcon_12_8_defaultMap_512_port_modeTwoNetCpuPorts;
                }
                else
                {
                    currAppDemoInfoPtr = &falcon_12_8_defaultMap_512_port_mode[0];
                    numEntries = actualNum_falcon_12_8_defaultMap_512_port_mode;
                }
            }
            else if (maxPhyPorts == 1024)
            {
                currAppDemoInfoPtr = &falcon_12_8_defaultMap_1024_port_mode_remote_ports[0];
                numEntries = actualNum_falcon_12_8_defaultMap_1024_port_mode_remote_ports;
            }
            else /* 128 */
            {
                if(cpuPortMux2UseSdma == 0)
                {
                    currAppDemoInfoPtr = &falcon_12_8_defaultMap_128_modeTwoNetCpuPorts[0];
                    numEntries = actualNum_falcon_12_8_defaultMap_128_modeTwoNetCpuPorts;
                }
                else
                {
                    currAppDemoInfoPtr = &falcon_12_8_defaultMap[0];
                    numEntries = actualNum_falcon_12_8_defaultMap;
                }
            }
            break;
    }

    if(currAppDemoInfoPtr == NULL || numEntries == 0)
    {
        if(numEntries == 0)
        {
            cpssOsPrintf("ERROR : numEntries == 0 \n");
        }

        if(currAppDemoInfoPtr == NULL)
        {
            cpssOsPrintf("ERROR : currAppDemoInfoPtr == NULL \n");
        }

        return GT_BAD_STATE;
    }

    numCpuSdmas = 0;
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&skipEntriesInAppDemoPorts);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&skipBmpInterface);

    if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 2)
    {
        FALCON_PORT_MAP_STC *temp_currAppDemoInfoPtr;
        GT_U32              temp_startGlobalDmaNumber;

        use_falcon_6_4_cpu_ports_group1 = 1;/* default : use CPU ports MACs 130,131 */
        if(modeMaxMac)
        {
            use_falcon_6_4_cpu_ports_group2 = 1;/* default : use also CPU ports MACs 128,135 */
        }

        appDemoDbEntryGet("use_falcon_6_4_cpu_ports_group1", &use_falcon_6_4_cpu_ports_group1);
        appDemoDbEntryGet("use_falcon_6_4_cpu_ports_group2", &use_falcon_6_4_cpu_ports_group2);

        temp_currAppDemoInfoPtr = currAppDemoInfoPtr;

        /* remove from the mapping the CPU ports that are not part of it */
        for (ii = 0 ; ii < numEntries; ii++ , currAppDemoInfoPtr++)
        {
            temp_startGlobalDmaNumber = currAppDemoInfoPtr->startGlobalDmaNumber;
            for(jj = 0; jj < 2 ; jj++)
            {
                if(!use_falcon_6_4_cpu_ports_group1)
                {
                    if(temp_startGlobalDmaNumber == falcon_6_4_non_dual_cpu_ports[0].macPortGroup1[jj])
                    {
                        /* need to state to skip this entry */
                        CPSS_PORTS_BMP_PORT_ENABLE_MAC(&skipEntriesInAppDemoPorts,ii,GT_TRUE);
                        break;
                    }
                }

                if(!use_falcon_6_4_cpu_ports_group2)
                {
                    if(temp_startGlobalDmaNumber == falcon_6_4_non_dual_cpu_ports[0].macPortGroup2[jj])
                    {
                        /* need to state to skip this entry */
                        CPSS_PORTS_BMP_PORT_ENABLE_MAC(&skipEntriesInAppDemoPorts,ii,GT_TRUE);
                        break;
                    }
                }
            }
        }

        /* restore position */
        currAppDemoInfoPtr = temp_currAppDemoInfoPtr;
    }

    /* remove from the mapping the SDMAs ports that collide with the 'needed' CPU ports */
    {
        MUX_DMA_STC                 *muxed_SDMAs_Ptr;
        FALCON_PORT_MAP_STC *temp_currAppDemoInfoPtr;
        GT_U32              temp_startGlobalDmaNumber;
        GT_U32              kk;

        temp_currAppDemoInfoPtr = currAppDemoInfoPtr;

        muxed_SDMAs_Ptr = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 4 ? falcon_12_8_muxed_SDMAs :
                          PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 2 ? falcon_6_4_muxed_SDMAs  :
                                                       /* single tile*/        falcon_3_2_muxed_SDMAs  ;

        /* remove from the mapping the SDMAs ports that collide with the 'needed' CPU ports */
        for (ii = 0 ; ii < numEntries; ii++ , currAppDemoInfoPtr++)
        {
            if(currAppDemoInfoPtr->mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                continue;
            }

            dmaPortOffset = 0;
            for(jj = 0 ; jj < currAppDemoInfoPtr->numOfPorts; jj++)
            {
                temp_startGlobalDmaNumber       = currAppDemoInfoPtr->startGlobalDmaNumber + dmaPortOffset;

                /* check if this DMA is muxed */
                for(kk = 0 ; muxed_SDMAs_Ptr[kk].sdmaPort != GT_NA ; kk++)
                {
                    if(temp_startGlobalDmaNumber != muxed_SDMAs_Ptr[kk].sdmaPort)
                    {
                        continue;
                    }
                    /* it is muxed ... check if forbidden */
                    if(GT_TRUE == isMacPortInList(temp_currAppDemoInfoPtr,numEntries,muxed_SDMAs_Ptr[kk].macPort))
                    {
                        cpssOsPrintf("Using MAC[%d] instead of SDMA[%d] that is muxed with it \n",
                            muxed_SDMAs_Ptr[kk].macPort ,
                            muxed_SDMAs_Ptr[kk].sdmaPort);

                        /* the muxed MAC is used ! so need to remove this DMA */
                        CPSS_PORTS_BMP_PORT_ENABLE_MAC(&skipBmpInterface,temp_startGlobalDmaNumber,GT_TRUE);
                    }
                }
                /* calculate dmaPortOffset */
                if((currAppDemoInfoPtr->jumpDmaPorts != DMA_NO_STEP)&&(currAppDemoInfoPtr->mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E))
                {
                    if(currAppDemoInfoPtr->jumpDmaPorts >= 2)
                    {
                        dmaPortOffset += currAppDemoInfoPtr->jumpDmaPorts;
                    }
                    else
                    {
                        dmaPortOffset ++;
                    }
                }
            }
        }

        /* restore position */
        currAppDemoInfoPtr = temp_currAppDemoInfoPtr;
    }


    cpssOsMemSet(cpssApi_falcon_defaultMap,0,sizeof(cpssApi_falcon_defaultMap));
    currCpssInfoPtr    = &cpssApi_falcon_defaultMap[0];

    for (ii = 0 ; ii < numEntries; ii++ , currAppDemoInfoPtr++)
    {
        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&skipEntriesInAppDemoPorts,ii))
        {
            /* skip those entries that are 'not relevant to this run' */
            continue;
        }

        if ((currAppDemoInfoPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)&&(actualNum_cpssApi_falcon_defaultMap < maxPhyPorts))
        {
            /* first generate line that map between the mac number to physical port in the device.
            for remote ports the physical port number ( cascade number) is in field of "dma step"*/
            currCpssInfoPtr->physicalPortNumber = currAppDemoInfoPtr->jumpDmaPorts;
            currCpssInfoPtr->mappingType        = CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;
            currCpssInfoPtr->interfaceNum       = currAppDemoInfoPtr->startGlobalDmaNumber;
            currCpssInfoPtr->txqPortNumber      = 0;

            actualNum_cpssApi_falcon_defaultMap++;
            currCpssInfoPtr++;
            /* second, generate lines that map between remote physical port to the mac number*/
        }
        dmaPortOffset = 0;
        for(jj = 0 ; jj < currAppDemoInfoPtr->numOfPorts; jj++ , currCpssInfoPtr++)
        {
            if(actualNum_cpssApi_falcon_defaultMap >= maxPhyPorts)
            {
                rc = GT_FULL;
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssApi_falcon_defaultMap is FULL, maxPhy port is [%d]\n", maxPhyPorts);
                return rc;
            }

            currCpssInfoPtr->physicalPortNumber = currAppDemoInfoPtr->startPhysicalPortNumber + jj;
            currCpssInfoPtr->mappingType        = currAppDemoInfoPtr->mappingType;
            currCpssInfoPtr->interfaceNum       = currAppDemoInfoPtr->startGlobalDmaNumber + dmaPortOffset;
            /* convert the DMA to TXQ port number , because those need to be in consecutive range and in 8 TC steps */
            currCpssInfoPtr->txqPortNumber      = 0;

            if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&skipBmpInterface,currCpssInfoPtr->interfaceNum))
            {
                currCpssInfoPtr --;/* to return to this entry on the next iteration */

                /* allow to increment the dmaPortOffset */
            }
            else
            if(currAppDemoInfoPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E &&
               maxCpuSdma == numCpuSdmas)
            {
                /* not allow this CPU SDMA interface to be used */

                currCpssInfoPtr --;/* to return to this entry on the next iteration */
            }
            else
            {
                if(currAppDemoInfoPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
                {
                    numCpuSdmas++;
                }

                actualNum_cpssApi_falcon_defaultMap++;
            }

            if((currAppDemoInfoPtr->jumpDmaPorts != DMA_NO_STEP)&&(currAppDemoInfoPtr->mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E))
            {
                if(currAppDemoInfoPtr->jumpDmaPorts >= 2)
                {
                    dmaPortOffset += currAppDemoInfoPtr->jumpDmaPorts;
                }
                else
                {
                    dmaPortOffset ++;
                }
            }
        }
    }

    if(PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98CX8550_CNS)
    {
        if (maxPhyPorts <= 128)
        {
            /* 8T board 128 ports mode - no need special treatment */
            return GT_OK;
        }
        else
        {
            /* In 8T board's front-panel - the Ravens order is 2,5,7,8,10,13,14,15,0,1. Shift of two ravens (32 ports) is needed */
            maxEthMacs = 64 * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
            for ( ii = 0; ii < actualNum_cpssApi_falcon_defaultMap; ii++ )
            {
                 if(cpssApi_falcon_defaultMap[ii].interfaceNum < maxEthMacs)
                 {
                     GT_U8 skippedRavens;
                     switch (cpssApi_falcon_defaultMap[ii].interfaceNum/16)
                     {
                         case 0:
                             skippedRavens = 2;
                             break;
                         case 1:
                             skippedRavens = 4;
                             break;
                         case 2:
                             skippedRavens = 5;
                             break;
                         case 5:
                             skippedRavens = 3;
                             break;
                         case 7:
                             skippedRavens = 3;
                             break;
                         case 8:
                             skippedRavens = 5;
                             break;
                         case 10:
                             skippedRavens = 4;
                             break;
                         case 13:
                         case 14:
                         case 15:
                             skippedRavens = 2;
                             break;
                         default:
                             return GT_BAD_PARAM;
                     }

                     cpssApi_falcon_defaultMap[ii].interfaceNum = (cpssApi_falcon_defaultMap[ii].interfaceNum + (16 * skippedRavens)) % maxEthMacs;
                 }
            }
        }

        return GT_OK;
    }

    if(currBoardType == APP_DEMO_FALCON_BOARD_12_8_B2B_CNS)
    {/* In Belly-to-Belly board's front-panel - the Ravens order is 2-15,0-1. Shift of 32 ports is needed */
        maxEthMacs = 64 * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
        for(ii = 0 ; ii < actualNum_cpssApi_falcon_defaultMap ; ii++)
        {
             if(cpssApi_falcon_defaultMap[ii].interfaceNum < maxEthMacs)
             {
                 cpssApi_falcon_defaultMap[ii].interfaceNum = (cpssApi_falcon_defaultMap[ii].interfaceNum + 32) % maxEthMacs;
             }
        }

    }

    if(currBoardType == APP_DEMO_FALCON_BOARD_2T_4T_CNS)
    {/* In 2T4T board's front-panel - the Ravens order is 7,0,2,4,5. Shift of 16 ports is needed,
        in 3.2T, raven 4 does not exist as well */
        maxEthMacs = 64 * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
        for ( ii = 0; ii < actualNum_cpssApi_falcon_defaultMap; ii++ )
        {
             if(cpssApi_falcon_defaultMap[ii].interfaceNum < maxEthMacs)
             {
                 GT_U8 skippedRavens;
                 switch (cpssApi_falcon_defaultMap[ii].interfaceNum/16)
                 {
                 case 0:
                     skippedRavens = 0;
                     break;
                 case 2:
                     skippedRavens = 1;
                     break;
                 case 4:
                     if ( CPSS_98CX8525_CNS != devType )
                     {
                         skippedRavens = 1;
                     }
                     else
                     {
                         return GT_BAD_PARAM;
                     }
                     break;
                 case 5:
                     if ( CPSS_98CX8525_CNS != devType )
                     {
                         skippedRavens = 0;
                     }
                     else
                     {
                         skippedRavens = 2;
                     }
                     break;
                 case 7:
                     skippedRavens = 1;
                     break;
                 default :
                     return GT_BAD_PARAM;
                 }
                 cpssApi_falcon_defaultMap[ii].interfaceNum = (cpssApi_falcon_defaultMap[ii].interfaceNum + maxEthMacs - (16*(skippedRavens+1))) % maxEthMacs;
             }
        }
    }

    return GT_OK;
}

#define GM_FALCON_MAC_AND_TXQ_PORT_MAC(macPort) macPort,(8)

/* NOTE: the GM supports only single DP units , and single TXQ-DQ due to memory issues. */
/* so use only ports of DMA 0..8 , CPU port that uses the DMA 8                         */
static CPSS_DXCH_PORT_MAP_STC singlePipe_singleDp_cpssApi_falcon_defaultMap[] =
{ /* Port,            mappingType                           portGroupm,         intefaceNum, txQPort,      trafficManegerEnable , tmPortInd*/
/*0*/  {   0, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       GM_FALCON_MAC_AND_TXQ_PORT_MAC( 0),  GT_FALSE,   GT_NA,GT_FALSE}
/*1*/ ,{  18, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       GM_FALCON_MAC_AND_TXQ_PORT_MAC( 1),  GT_FALSE,   GT_NA,GT_FALSE}
/*2*/ ,{  36, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       GM_FALCON_MAC_AND_TXQ_PORT_MAC( 2),  GT_FALSE,   GT_NA,GT_FALSE}
/*3*/ ,{  54, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       GM_FALCON_MAC_AND_TXQ_PORT_MAC( 3),  GT_FALSE,   GT_NA,GT_FALSE}
/*4*/ ,{  58, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       GM_FALCON_MAC_AND_TXQ_PORT_MAC( 4),  GT_FALSE,   GT_NA,GT_FALSE}
/*5*/ ,{  79, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       GM_FALCON_MAC_AND_TXQ_PORT_MAC( 5),  GT_FALSE,   GT_NA,GT_FALSE}
/*6*/ ,{  64, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       GM_FALCON_MAC_AND_TXQ_PORT_MAC( 6),  GT_FALSE,   GT_NA,GT_FALSE}
/*7*/ ,{  80, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,               0,       GM_FALCON_MAC_AND_TXQ_PORT_MAC( 7),  GT_FALSE,   GT_NA,GT_FALSE}
/*8*/ ,{CPU_PORT, CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,               0,       GM_FALCON_MAC_AND_TXQ_PORT_MAC( 8),  GT_FALSE,   GT_NA,GT_FALSE}
};


/* GM using single DP (only 8 MACs) */
static PortInitList_STC singlePipe_singleDp_portInitlist_falcon[] =
{
     { PORT_LIST_TYPE_LIST,      {0,18,36,58,54,64,79,80,    APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* 64 ports mode: Falcon ports in 100G mode */
static PortInitList_STC falcon_3_2_portInitlist_64_port_mode[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,49,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E/*CPSS_PORT_SPEED_100G_E*/,  CPSS_PORT_INTERFACE_MODE_KR_E/*CPSS_PORT_INTERFACE_MODE_KR2_E*/ }
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/* -- 50G PORT MODE -- */

/* 128 ports mode: Falcon ports in 50G mode */
static PortInitList_STC falcon_3_2_portInitlist[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,58,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }
    ,{ PORT_LIST_TYPE_LIST,      {64,65,66,67,68,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }

    /* Reduced/CPU port - mapped to global MAC_64 - resides in Raven_3 */
    ,{ PORT_LIST_TYPE_LIST,      {69,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

static PortInitList_STC falcon_3_2_portInitlist_TwoCpuPorts[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,58,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }
    ,{ PORT_LIST_TYPE_LIST,      {64,65,66,67,68,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }

    /* Reduced/CPU port - mapped to global MAC_64 - resides in Raven_3 */
    ,{ PORT_LIST_TYPE_LIST,      {69,70,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/* 256 ports mode: Falcon ports in 50G mode */
static PortInitList_STC falcon_3_2_portInitlist_256_port_mode[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,58,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }
    ,{ PORT_LIST_TYPE_LIST,      {128,130,131,132,133,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* 1024 ports mode: Falcon ports in 50G mode */
static PortInitList_STC falcon_3_2_portInitlist_1024_port_mode[] =
{

    /*{ PORT_LIST_TYPE_INTERVAL,  {128,186,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }*/
     { PORT_LIST_TYPE_INTERVAL,  {0,58,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }
    ,{ PORT_LIST_TYPE_INTERVAL,  {/*590*/600,620,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,  CPSS_PORT_INTERFACE_MODE_REMOTE_E }
    ,{ PORT_LIST_TYPE_LIST,      {621,622,623,624,625,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,  CPSS_PORT_INTERFACE_MODE_REMOTE_E } /* remote ports*/
    ,{ PORT_LIST_TYPE_LIST,      {65,66,67,68,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E } /* cascade ports*/
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/* -- 100G PORT MODE -- */

/* 128 ports mode: Falcon ports in 100G mode */
static PortInitList_STC falcon_3_2_100G_portInitlist[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,58,2, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }
    ,{ PORT_LIST_TYPE_LIST,      {65,67,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/* 256 ports mode: Falcon ports in 100G mode */
static PortInitList_STC falcon_3_2_100G_portInitlist_256_port_mode[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,58,2, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }
    ,{ PORT_LIST_TYPE_LIST,      {128,130,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* 1024 ports mode: Falcon ports in 100G mode */
static PortInitList_STC falcon_3_2_100G_portInitlist_1024_port_mode[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {512,570,2, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }
    ,{ PORT_LIST_TYPE_LIST,      {577,579,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E } /* remote ports */

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* 64 port mode: Falcon ports in 50G mode */
static PortInitList_STC falcon_6_4_portInitlist_64_port_mode[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,49,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/* Falcon ports in 50G mode */
static PortInitList_STC falcon_6_4_portInitlist_256_port_mode[] =
{
     { PORT_LIST_TYPE_LIST,      {0,1,2,3,11,18,20,32,35,36,54,55,56,57,58, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
     /* ports 64..127*/
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,127,1,                            APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }

};


/* 1024 ports mode: Falcon ports in 50G mode */
static PortInitList_STC falcon_6_4_portInitlist_1024_port_mode[] =
{

     { PORT_LIST_TYPE_INTERVAL,  {0,28,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,74,1,                            APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_LIST,      {75,76,77,78,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E } /* cascade ports*/
    ,{ PORT_LIST_TYPE_INTERVAL,  {600,620,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,  CPSS_PORT_INTERFACE_MODE_REMOTE_E }
    ,{ PORT_LIST_TYPE_LIST,      {621,622,623,624,625,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,  CPSS_PORT_INTERFACE_MODE_REMOTE_E } /* remote ports*/
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/* Falcon ports in 50G mode */
static PortInitList_STC falcon_6_4_portInitlist[] =
{
     { PORT_LIST_TYPE_LIST,      {0,1,2,3,18,20,32,35,36,54,55,56,57,58, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
     /* ports 64..127*/
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,127,1,                            APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }


    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

static PortInitList_STC falcon_6_4_portInitlist_hqos[] =
{
     { PORT_LIST_TYPE_LIST,      {0,1,2,3,18,20,32,35,36,54,55,56,57,58, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
     /* ports 64..127*/
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,127,1,                            APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_LIST,  {129,131       ,APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
      /* ports 300..307*/
    ,{ PORT_LIST_TYPE_INTERVAL,  {300,307,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,  CPSS_PORT_INTERFACE_MODE_REMOTE_E}

     /* ports 400..407*/
    ,{ PORT_LIST_TYPE_INTERVAL,  {400,407,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,  CPSS_PORT_INTERFACE_MODE_REMOTE_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/* Falcon ports in 100G mode */
static PortInitList_STC falcon_6_4_100G_portInitlist[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,58,2, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }
    ,{ PORT_LIST_TYPE_INTERVAL,  {65,69,2,APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/* Falcon ports in 50G mode */
static PortInitList_STC falcon_12_8_portInitlist_64_port_mode[] =
{
     { PORT_LIST_TYPE_INTERVAL,       {0,49,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E    }
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/* Falcon ports in 50G mode */
static PortInitList_STC falcon_12_8_portInitlist_256_port_mode[] =
{
    { PORT_LIST_TYPE_LIST,      {0,1,2,3,18,20,32,35,36,54,55,56,57,58, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
         /* ports 64..127*/
   ,{ PORT_LIST_TYPE_INTERVAL,  {64,127,1,                           APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
   ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

static PortInitList_STC falcon_12_8_rd_slot_portInitlist_256_port_mode[] =
{
   /*slot 1 to 8 */
    { PORT_LIST_TYPE_INTERVAL,  {128,        128+15         ,1,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }
   ,{ PORT_LIST_TYPE_INTERVAL,  {128 + 16*1, 128+16*1 + 15  ,1,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }
   ,{ PORT_LIST_TYPE_INTERVAL,  {128 + 16*2, 128+16*2 + 15  ,1,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }
   ,{ PORT_LIST_TYPE_INTERVAL,  {128 + 16*3, 128+16*3 + 15  ,1,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }
   ,{ PORT_LIST_TYPE_INTERVAL,  {128 + 16*4, 128+16*4 + 15  ,1,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }
   ,{ PORT_LIST_TYPE_INTERVAL,  {128 + 16*5, 128+16*5 + 15  ,1,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }
   ,{ PORT_LIST_TYPE_INTERVAL,  {128 + 16*6, 128+16*6 + 15  ,1,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }
   ,{ PORT_LIST_TYPE_INTERVAL,  {128 + 16*7, 128+16*7 + 15  ,1,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E }
   ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* 512 ports mode: Falcon ports in 50G mode */
static PortInitList_STC falcon_12_8_portInitlist_512_port_mode[] =
{

     { PORT_LIST_TYPE_INTERVAL,  {0,60,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,258,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
     /* ports 258,259 (the MAC ports that supports 10G) */
    ,{ PORT_LIST_TYPE_INTERVAL,  {259,260,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* 1024 ports mode: Falcon ports in 50G mode */
static PortInitList_STC falcon_12_8_portInitlist_1024_port_mode[] =
{

     { PORT_LIST_TYPE_INTERVAL,  {0,28,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E }
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,74,1,                            APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_LIST,      {75,76,77,78,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E } /* cascade ports*/
    ,{ PORT_LIST_TYPE_INTERVAL,  {600,620,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,  CPSS_PORT_INTERFACE_MODE_REMOTE_E}
    ,{ PORT_LIST_TYPE_LIST,      {621,622,623,624,625,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,   CPSS_PORT_INTERFACE_MODE_REMOTE_E  } /* remote ports*/
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* Falcon ports in 50G mode (no CPU reduced network ports) */
static PortInitList_STC falcon_12_8_portInitlist[] =
{
    /* ports 0..119*/
     { PORT_LIST_TYPE_INTERVAL,  {0,119,1,                           APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* Falcon ports in 50G mode + 2 CPU reduced network ports */
static PortInitList_STC falcon_12_8_portInitlist_twoCpuPorts[] =
{
    /* ports 0..118*/
     { PORT_LIST_TYPE_INTERVAL,  {0,118,1,   APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
     /* ports 119,120 (the MAC ports that supports 10G) */
    ,{ PORT_LIST_TYPE_INTERVAL,  {119,120,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};



static PortInitList_STC falcon_12_8_port_mode_25_portInitlist[] =
{
    /* ports 0..31*/
     { PORT_LIST_TYPE_LIST,      {0,8,16,24,32,40,48,56,69,77,85,93,101,109,117,125,133,141,149,157,165,173,
     181,189,197,205,213,221,APP_INV_PORT_CNS }, CPSS_PORT_SPEED_400G_E,  CPSS_PORT_INTERFACE_MODE_KR8_E      }
    ,{ PORT_LIST_TYPE_LIST,      {229,237,245,253,APP_INV_PORT_CNS }, CPSS_PORT_SPEED_400G_E,  CPSS_PORT_INTERFACE_MODE_KR8_E      }
    ,{ PORT_LIST_TYPE_INTERVAL,  {255,967,1,   APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E      }
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

static PortInitList_STC falcon_6_4_port_mode_9_portInitlist[] =
{
    /* ports 0..31*/
    { PORT_LIST_TYPE_LIST,      {0,8,16,24,32,40,48,56,69,77,85,93,101,109,117,125    ,APP_INV_PORT_CNS }, CPSS_PORT_SPEED_400G_E,  CPSS_PORT_INTERFACE_MODE_KR8_E      }
     /* ports 200..976*/
     ,{ PORT_LIST_TYPE_INTERVAL,  {255,638,1,   APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E      }
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


static PortInitList_STC falcon_3_2_port_mode_9_portInitlist[] =
{
    /* ports 0..31*/
    { PORT_LIST_TYPE_LIST,      {0,8,16,24,32,40,48,56,APP_INV_PORT_CNS }, CPSS_PORT_SPEED_400G_E,  CPSS_PORT_INTERFACE_MODE_KR8_E      }
     /* ports 200..976*/
     ,{ PORT_LIST_TYPE_INTERVAL,  {255,447,1,   APP_INV_PORT_CNS }, CPSS_PORT_SPEED_100G_E,  CPSS_PORT_INTERFACE_MODE_KR2_E      }
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};



/* mode 256 physical : max MACs Falcon ports in 50G mode */
static PortInitList_STC falcon_6_4_mode_256_maxMAC_portInitlist[] =
{
     /* ports 0..132 (the 128 MAC ports that supports 50G) */
     { PORT_LIST_TYPE_INTERVAL,  {0  ,132,1/*step of 1*/,APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
     /* ports 133,134,135,136 (the MAC ports that supports 10G) */
    ,{ PORT_LIST_TYPE_INTERVAL,  {133,136,1/*step of 1*/,APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* mode 128 physical : max MACs Falcon ports in 50G mode */
static PortInitList_STC falcon_6_4_mode_128_maxMAC_portInitlist[] =
{
     /* ports 0..123 (the 128 MAC ports that supports 50G) */
     { PORT_LIST_TYPE_INTERVAL,  {0  ,123,1/*step of 1*/,APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
     /* ports 124..127 (the MAC ports that supports 10G) */
    ,{ PORT_LIST_TYPE_INTERVAL,  {124,127,1/*step of 1*/,APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* Armstrong2 ports in 25G mode */
static PortInitList_STC armstrong2_80x25g_portInitlist[] =
{
    /* ports 0..59 */
     { PORT_LIST_TYPE_INTERVAL,  {0,59,1,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* ports 64..83 */
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,83,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* ports 90..92 */
    ,{ PORT_LIST_TYPE_INTERVAL,  {90,91,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* Falcon ports in 50G mode */
static PortInitList_STC falcon_80x50g_portInitlist[] =
{
    /* ports 0..59 */
     { PORT_LIST_TYPE_INTERVAL,  {0,59,1,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* ports 64..83 */
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,83,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* ports 90..92 */
    ,{ PORT_LIST_TYPE_INTERVAL,  {90,91,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

static PortInitList_STC falcon_64x50g_portInitlist[] =
{
    /* ports 0..59 */
     { PORT_LIST_TYPE_INTERVAL,  {0,59,1,  APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* ports 64..67 */
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,67,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* ports 90..92 */
    ,{ PORT_LIST_TYPE_INTERVAL,  {90,91,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* Falcon ports in 50G mode */
static PortInitList_STC falcon_160x50g_64_portInitlist[] =
{
    /* ports  0..51 */
     { PORT_LIST_TYPE_INTERVAL,  {  0,  51,  1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
     /* ports 54..55*/
     ,{ PORT_LIST_TYPE_INTERVAL, { 54,  55,  1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

     ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* Falcon ports in 50G mode */
static PortInitList_STC falcon_160x50g_128_portInitlist[] =
{
    /* ports  0..59 */
     { PORT_LIST_TYPE_INTERVAL,  {  0,  59,  1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* ports  64..67 */
    ,{ PORT_LIST_TYPE_INTERVAL,  { 64,  67,  1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* ports  68..99 */
    ,{ PORT_LIST_TYPE_INTERVAL,  { 68,  99,  1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* ports 100..115 */
    ,{ PORT_LIST_TYPE_INTERVAL,  { 100, 115, 1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    /* ports 116..117*/
    ,{ PORT_LIST_TYPE_INTERVAL,  { 116, 117, 1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* Falcon ports in 50G mode */
static PortInitList_STC falcon_160x50g_maxPorts_portInitlist[] =
{
    /* ports  0..59 */
     { PORT_LIST_TYPE_INTERVAL,  {  0,  59,  1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* ports  64..67 */
    ,{ PORT_LIST_TYPE_INTERVAL,  { 64,  67,  1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* ports  68..99 */
    ,{ PORT_LIST_TYPE_INTERVAL,  { 68,  99,  1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* ports 100..115 */
    ,{ PORT_LIST_TYPE_INTERVAL,  { 100, 115, 1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* ports 116..163 */
    ,{ PORT_LIST_TYPE_INTERVAL,  { 116, 163, 1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    /* ports 164..165 */
    ,{ PORT_LIST_TYPE_INTERVAL,  { 164, 165, 1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

static PortInitList_STC *falcon_force_PortsInitListPtr = NULL;

/**
* @internal falcon_getBoardInfo function
* @endinternal
*
* @brief   Return the board configuration info.
*
* @param[in] firstDev                 - GT_TRUE - first device, GT_FALSE - next device
* @param[in] devIndex                 - device index
*
* @param[out] hwInfoPtr                - HW info of found device.
* @param[out] pciInfo                  - PCI info of found device.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of getBoardInfo.
*
*/
static GT_STATUS falcon_getBoardInfo
(
    IN  GT_BOOL          firstDev,
    IN  GT_U8            devIndex,
    OUT CPSS_HW_INFO_STC *hwInfoPtr,
    OUT GT_PCI_INFO      *pciInfo
)
{
    GT_STATUS   rc;

    if(isUnderAddDeviceCatchup == GT_TRUE)
    {
        /* when : isUnderAddDeviceCatchup == GT_TRUE
           The gtPresteraGetPciDev was already done by appDemoHotInsRmvSysGetPciInfo
        */
        *pciInfo = appDemoPpConfigList[devIndex].pciInfo;
    }
    else
    if (appDemoCpssPciProvisonDone == GT_FALSE)
    {
        /* In our case we want to find just one prestera device on PCI bus*/
        rc = gtPresteraGetPciDev(firstDev, pciInfo);
        if (rc != GT_OK)
        {
            if (firstDev != GT_FALSE)
            {
                osPrintf("Could not recognize any device on PCI bus!\n");
            }
            return GT_NOT_FOUND;
        }

        /*copy info for later use*/
        appDemoPpConfigList[devIndex].pciInfo = *pciInfo;
    }
    else
    {
        *pciInfo = appDemoPpConfigList[devIndex].pciInfo;
        gtPresteraSetPciDevIdinDrv( appDemoPpConfigList[devIndex].pciInfo.pciDevVendorId.devId);
    }
    rc = extDrvPexConfigure(
            pciInfo->pciBusNum,
            pciInfo->pciIdSel,
            pciInfo->funcNo,
            MV_EXT_DRV_CFG_FLAG_EAGLE_E,
            hwInfoPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("extDrvPexConfigure", rc);

    appDemoPpConfigList[devIndex].hwInfo = *hwInfoPtr;

    return rc;
}

/**
* @internal falcon_getBoardInfoSimple function
* @endinternal
*
* @brief   Return the board configuration info.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of getBoardInfo.
*
*/
static GT_STATUS falcon_getBoardInfoSimple
(
    OUT CPSS_HW_INFO_STC *hwInfoPtr,
    OUT GT_PCI_INFO *pciInfo
)
{
    GT_STATUS   rc;

    rc = falcon_getBoardInfo(GT_TRUE/*firstDev*/,0, hwInfoPtr, pciInfo);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* In our case we want to find just one prestera device on PCI bus*/
    if (appDemoCpssPciProvisonDone == GT_FALSE)
    {
        rc = gtPresteraGetPciDev(GT_FALSE, pciInfo);
        if(rc == GT_OK)
        {
            osPrintf("More than one recognized device found on PCI bus!\n");
            return GT_INIT_ERROR;
        }
    }

    return GT_OK;
}

#ifdef ASIC_SIMULATION
extern GT_U32 simCoreClockOverwrittenGet(GT_VOID);
#endif

/**
* @internal falcon_boardTypePrint function
* @endinternal
*
* @brief   This function prints type of Falcon board and Cider version.
*
* @param[in] boardName                - board name
* @param[in] devName                  - device name
*                                       none
*/
static GT_VOID falcon_boardTypePrint
(
    IN GT_CHAR  *boardName,
    IN GT_CHAR *devName
)
{
    GT_CHAR *environment;

#ifdef GM_USED
    environment = "GM (Golden Model) - simulation";
#elif defined ASIC_SIMULATION
    if(cpssDeviceRunCheck_onEmulator())
    {
        environment = "WM (White Model) - simulation (special EMULATOR Mode)";
    }
    else
    {
        environment = "WM (White Model) - simulation";
    }
#else
    if(cpssDeviceRunCheck_onEmulator())
    {
        environment = "HW (on EMULATOR)";
    }
    else
    {
        environment = "HW (Hardware)";
    }
#endif

    cpssOsPrintf("%s Board Type: %s [%s]\n", devName , boardName , environment);

}

static GT_U32 falcon_special_deviceId = CPSS_98CX8540_CNS;
static GT_BOOL falconSpecialBoardType = GT_FALSE;
GT_STATUS falcon_special_board_prepare(GT_VOID)
{

    appDemoDbEntryAdd("numOfTiles", 2);
    appDemoDbEntryAdd("ravenDevBmp", 0x00FF);
    appDemoDebugDeviceIdSet(0, falcon_special_deviceId);

    falconSpecialBoardType = GT_TRUE;

    return GT_OK;
}

static GT_BOOL falcon_special_th_interposer_board = GT_FALSE;
GT_STATUS falcon_special_th_interposer_board_prepare(GT_VOID)
{
    falcon_special_th_interposer_board = GT_TRUE;
    return GT_OK;
}

/**
* @internal falcon_boardTypeInitDatabases function
* @endinternal
*
* @brief   This function sets the databases' pointers and their
*          size values according to the input board type.
*
* @param[in] currBoardType            - board type
*/
static GT_STATUS falcon_boardTypeInitDatabases
(
    GT_U32 boardType
)
{
    switch(boardType)
    {
        case APP_DEMO_FALCON_BOARD_12_8_TH_CNS:
            /* SerDes Muxing DB's */
            ARR_PTR_AND_SIZE_MAC(falcon_DB_MacToSerdesMap, falcon_MacToSerdesMap, falcon_MacToSerdesMap_arrSize);
            /* Polarity Swap DB's */
            ARR_PTR_AND_SIZE_MAC(falcon_DB_PolarityArray, falcon_PolarityArray, falcon_PolarityArray_arrSize);
#ifndef ASIC_SIMULATION
            /* PHY's DB's*/
            ARR_PTR_AND_SIZE_MAC(falcon_DB_RavenLaneToPhyLaneMap, falcon_RavenLaneToPhyLaneMap, falcon_RavenLaneToPhyLaneMap_arrSize);
            ARR_PTR_AND_SIZE_MAC(falcon_DB_PhyGearboxHostSidePolarityArray, falcon_PhyGearboxHostSidePolarityArray, falcon_PhyGearboxHostSidePolarityArray_arrSize);
            ARR_PTR_AND_SIZE_MAC(falcon_DB_PhyGearboxLineSidePolarityArray, falcon_PhyGearboxLineSidePolarityArray, falcon_PhyGearboxLineSidePolarityArray_arrSize);
            ARR_PTR_AND_SIZE_MAC(falcon_DB_PhyInfo, falcon_PhyInfo, falcon_PhyInfo_arrSize);
            ARR_PTR_AND_SIZE_MAC(falcon_DB_PhyTuneParams, falcon_PhyTuneParams, falcon_PhyTuneParams_arrSize);
#endif
            if(falconSpecialBoardType == GT_TRUE)
            {/* special TH board: 6.4T over 12.8T TH board */
                falcon_PolarityArray_arrSize = falcon_PolarityArray_arrSize >> 1;
            }
            if(falcon_special_th_interposer_board == GT_TRUE)
            {/* special TH board: 12.8T TH board with interposers instead of PHYs */
                ARR_PTR_AND_SIZE_MAC(falcon_DB_interposers_PolarityArray, falcon_PolarityArray, falcon_PolarityArray_arrSize);
            }
            break;


        case APP_DEMO_FALCON_BOARD_12_8_B2B_CNS:
            /* SerDes Muxing DB's */
            ARR_PTR_AND_SIZE_MAC(falcon_Belly2Belly_MacToSerdesMap, falcon_MacToSerdesMap, falcon_MacToSerdesMap_arrSize);
            /* Polarity Swap DB's */
            ARR_PTR_AND_SIZE_MAC(falcon_Belly2Belly_PolarityArray, falcon_PolarityArray, falcon_PolarityArray_arrSize);
            break;


        case APP_DEMO_FALCON_BOARD_12_8_RD_CNS:
            /* SerDes Muxing DB's */
            ARR_PTR_AND_SIZE_MAC(falcon_RD_MacToSerdesMap, falcon_MacToSerdesMap, falcon_MacToSerdesMap_arrSize);
            /* Polarity Swap DB's */
            ARR_PTR_AND_SIZE_MAC(falcon_RD_PolarityArray, falcon_PolarityArray, falcon_PolarityArray_arrSize);
#ifndef ASIC_SIMULATION
            /* PHY's DB's*/
            ARR_PTR_AND_SIZE_MAC(falcon_RD_PhyInfo, falcon_PhyInfo, falcon_PhyInfo_arrSize);
            ARR_PTR_AND_SIZE_MAC(falcon_RD_PhyTuneParams, falcon_PhyTuneParams, falcon_PhyTuneParams_arrSize);
#endif
            break;


        case APP_DEMO_FALCON_BOARD_6_4_TH_CNS:
            /* SerDes Muxing DB's */
            ARR_PTR_AND_SIZE_MAC(falcon_6_4T_MacToSerdesMap, falcon_MacToSerdesMap, falcon_MacToSerdesMap_arrSize);
            /* Polarity Swap DB's */
            ARR_PTR_AND_SIZE_MAC(falcon_6_4T_PolarityArray, falcon_PolarityArray, falcon_PolarityArray_arrSize);
            break;

        case APP_DEMO_FALCON_BOARD_2T_4T_CNS:
            /* SerDes Muxing DB's */
            ARR_PTR_AND_SIZE_MAC(falcon_2T4T_MacToSerdesMap, falcon_MacToSerdesMap, falcon_MacToSerdesMap_arrSize);
            /* Polarity Swap DB's */
            ARR_PTR_AND_SIZE_MAC(falcon_2T4T_PolarityArray, falcon_PolarityArray, falcon_PolarityArray_arrSize);
            break;

        default:
            return GT_NOT_SUPPORTED;
            break;
    }

    return GT_OK;
}

/**
* @internal getPpPhase1ConfigSimple function
* @endinternal
*
* @brief   Returns the configuration parameters for cpssDxChHwPpPhase1Init().
*
* @param[in] devNum                   - The CPSS devNum.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of configBoardAfterPhase1.
*
*/
static GT_STATUS getPpPhase1ConfigSimple
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_U8                               boardRevId,
    OUT CPSS_DXCH_PP_PHASE1_INIT_INFO_STC  *phase1Params
)
{
    GT_STATUS rc;
    GT_U32  tmpValue , numOfTiles,portsFalcor,maxNumOfPhyPortsToUse;

    phase1Params->devNum = CAST_SW_DEVNUM(devNum);

/* retrieve PP Core Clock from HW */
    phase1Params->coreClock             = CPSS_DXCH_AUTO_DETECT_CORE_CLOCK_CNS;

    phase1Params->ppHAState             = CPSS_SYS_HA_MODE_ACTIVE_E;


    phase1Params->numOfPortGroups       = 1;

    numOfTiles = 1;/* default : we can not know from the 'PCI scan' */
    if(GT_OK == appDemoDbEntryGet("numOfTiles", &tmpValue))
    {
        if(tmpValue == 1 ||
           tmpValue == 2 ||
           tmpValue == 4 )
        {
            numOfTiles = tmpValue;
        }
    }

    if(mode100G)
    {
        /************************************************/
        /* 1 of every 2 MACs needs physical port number */
        /************************************************/
        portsFalcor = 2;
    }
    else
    {
        portsFalcor = 1;
    }

    if(modeSonic32MacPorts)
    {
        maxNumOfPhyPortsToUse =  64;
        numOfTiles            =   4;
    }
    else
    if(modeSonic128MacPorts)
    {
        maxNumOfPhyPortsToUse = 256;
        numOfTiles            =   4;
    }
    else
    /* set proper 'default' mode that can support the 'max MACs'*/
    if(modeMaxMac)
    {
        /* single tile : to support 65 MACs + 4 SDMAs */
        /* 2 tiles : to support 130 MACs + 8 SDMAs */
        /* 4 tiles : to support 258 MACs + 16 SDMAs */
        maxNumOfPhyPortsToUse = (numOfTiles * 128) / portsFalcor;

        if(maxNumOfPhyPortsToUse < 64)
        {
            /****************/
            /* minimal mode */
            /****************/
            maxNumOfPhyPortsToUse = 64;
        }
    }
    else
    {
        maxNumOfPhyPortsToUse = 128;  /*use 128 physical ports that gives 8K vlans or 64, 256, 512, 1024. 128 is the default */
    }

    if((boardRevId == 11) || (boardRevId == 12)) /* Falcon Z boards */
    {
        phase1Params->mngInterfaceType      = CPSS_CHANNEL_PEX_FALCON_Z_E;
        numOfTiles = 4;
        maxNumOfPhyPortsToUse = 512;
    } else {
        /* Eagle access mode */
        phase1Params->mngInterfaceType      = CPSS_CHANNEL_PEX_EAGLE_E;
    }

    cpssOsPrintf("The device considered with [%d] tiles to select [%d] 'ports mode' \n",
        numOfTiles,
        maxNumOfPhyPortsToUse);

    phase1Params->maxNumOfPhyPortsToUse = maxNumOfPhyPortsToUse;

    /* add Db Entry */
    rc = appDemoDxHwPpPhase1DbEntryInit(CAST_SW_DEVNUM(devNum),phase1Params,
        CPSS_98CX8513_CNS,/* dummy to represent 'falcon' family (one of falcon values) */
        NULL,NULL,NULL);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxHwPpPhase1DbEntryInit", rc);
    if(rc != GT_OK)
        return rc;

    /* check if appDemoDxHwPpPhase1DbEntryInit modified the value , and give indication about it */
    if(phase1Params->maxNumOfPhyPortsToUse != maxNumOfPhyPortsToUse)
    {
        cpssOsPrintf("NOTE : The 'ports mode' was explicitly changed to [%d] (from [%d]) \n",
            phase1Params->maxNumOfPhyPortsToUse,
            maxNumOfPhyPortsToUse);
    }

    return GT_OK;
}

/**
* @internal getPpPhase2ConfigSimple function
* @endinternal
*
* @brief   Returns the configuration parameters for cpssDxChHwPpPhase2Init().
*
* @param[in] devNum                   - The device number to get the parameters for.
* @param[in] devType                  - The Pp device type
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getPpPhase2ConfigSimple
(
    IN GT_U32               devIndex,
    IN GT_SW_DEV_NUM       devNum,
    IN  CPSS_PP_DEVICE_TYPE         devType,
    OUT CPSS_DXCH_PP_PHASE2_INIT_INFO_STC  *phase2Params
)
{
    GT_STATUS                   rc;
    CPSS_PP_PHASE2_INIT_PARAMS  appDemo_ppPhase2Params;
    GT_U32                      tmpData;
    GT_U32                      auDescNum,fuDescNum;
    GT_U32 rxDescNum = RX_DESC_NUM_DEF;
    GT_U32 txDescNum = TX_DESC_NUM_DEF;
    GT_U32 numDevsFactor;
    GT_U32 overrideDmaReduceFactor = 0;

    appDemoPhase2DefaultInit(&appDemo_ppPhase2Params);
    osMemSet(phase2Params,0,sizeof(CPSS_DXCH_PP_PHASE2_INIT_INFO_STC));

    appDemo_ppPhase2Params.useDoubleAuq = GT_TRUE;

    if( appDemoDbEntryGet("useMultiNetIfSdma", &tmpData) == GT_NO_SUCH )
    {
        appDemo_ppPhase2Params.useMultiNetIfSdma = GT_TRUE;
        if( appDemoDbEntryGet("skipTxSdmaGenDefaultCfg", &tmpData) == GT_NO_SUCH )
        {

            /* Enable Tx queue 3 to work in Tx queue generator mode */
            rc = appDemoDbEntryAdd("txGenQueue_3", GT_TRUE);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* Enable Tx queue 6 to work in Tx queue generator mode */
            rc = appDemoDbEntryAdd("txGenQueue_6", GT_TRUE);
            if(rc != GT_OK)
            {
                 return rc;
            }
         }

         if(!falcon_initPortDelete_WA_disabled)
         {
            /* force SDMA packet generator when the WA is needed in last queue */
            /* NOTE: ignoring "skipTxSdmaGenDefaultCfg" !!!                    */
            /* if this need to be bypassed ... need new 'flag' from appDemoDbEntryGet(...) */
            GT_U32  ii;
            GT_CHAR falcon_initPortDelete_WA_SdmaGenQueueName[40];

            for(ii = 0 ; ii < 2 ; ii ++)
            {
                osSprintf(falcon_initPortDelete_WA_SdmaGenQueueName,
                          "txGenQueue_%d",
                          reservedCpuSdmaGlobalQueue[ii]);

                /* Enable Tx queue for the WA to work in Tx queue generator mode */
                rc = appDemoDbEntryAdd(falcon_initPortDelete_WA_SdmaGenQueueName, GT_TRUE);
                if(rc != GT_OK)
                {
                    return rc;
                }

                osPrintf("falcon_initPortDelete_WA : using SDMA queue [%d] as 'packet generator' \n",
                    reservedCpuSdmaGlobalQueue[ii]);

                /* set the second queue with 2 descriptors and (3000+16) buffer size */
                if(ii == 1)/* the queue for the long packets */
                {
                    osSprintf(falcon_initPortDelete_WA_SdmaGenQueueName,
                              "txGenBuffSize_%d",
                              reservedCpuSdmaGlobalQueue[ii]);

                    /* very long buffers */
                    rc = appDemoDbEntryAdd(falcon_initPortDelete_WA_SdmaGenQueueName, (3000+16));
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    osSprintf(falcon_initPortDelete_WA_SdmaGenQueueName,
                              "txGenNumOfDescBuff_%d",
                              reservedCpuSdmaGlobalQueue[ii]);

                    /* need only single buffer but the appDemoAllocateDmaMem(...) divide it by 2 */
                    rc = appDemoDbEntryAdd(falcon_initPortDelete_WA_SdmaGenQueueName, 2*1);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
         }
    }

    /* allow each tile with it's own number of AUQ size */
    auDescNum = FALCON_AU_DESC_NUM_DEF * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
    /* allow each pipe with it's own number of FUQ size (for CNC upload) */
    /* NOTE: Fdb Upload still happen only on 'per tile' base (like AUQ) ...
       it is not per pipe triggered */
    fuDescNum = AU_DESC_NUM_DEF * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes;

    phase2Params->newDevNum                  = CAST_SW_DEVNUM(devNum);
    phase2Params->noTraffic2CPU = GT_FALSE;
    phase2Params->netifSdmaPortGroupId = 0;
    phase2Params->auMessageLength = CPSS_AU_MESSAGE_LENGTH_8_WORDS_E;

    /* mapping for 6.4 Emulator that due to lack of SDMA on the Linux OS */
    /* there is not enough SDMA allocation */
    /* Emulator fails with next error message:
        Allocating 400 bytes for DMA
        Allocating 38701 bytes for DMA
        memory DMA exhausted base=0xfae00000 curr=0xfafffac0 size=0x9740 end=0xfb000000
        Failed to allocate 38701 bytes, exiting
    */
    if(appDemoDbEntryGet("dma_reduce_factor", &tmpData) == GT_OK && tmpData)
    {
        /* As there is logic of '/=' in appDemoAllocateDmaMem(...)
           we need to disable this value , and restore it once the
           appDemoAllocateDmaMem(...) finish
           since not only 'Falcon devices' are part of the system , and devices
           can be added one by one as 'hot insersion'
           and not all devices seen via appDemoPpConfigDevAmount , as this
           value increment per 'insert device'
        */
        numDevsFactor = tmpData;
        overrideDmaReduceFactor = tmpData;

        /* we don't want the appDemoAllocateDmaMem(...) to change the numbers that
           the current function do with the 'rxSdmaDescrFix_0_Add' ,
           'rxSdmaDescrFix_0_Sub' */
        appDemoDbEntryAdd("dma_reduce_factor", 0);
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles >= 2 || appDemoPpConfigDevAmount >= 2)
    {
        numDevsFactor = appDemoPpConfigDevAmount;
    }
    else
    {
        numDevsFactor = 0;
    }

    if(numDevsFactor)
    {
        GT_U32  sdmaFactor;

        /* there is not enough DMA allocation for all the multiples */
        /* so keep the same alloc as in single tile */
        auDescNum /= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles * numDevsFactor;
        fuDescNum /= PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles * numDevsFactor;

        /* let the CPU SDMAs ports get the same number as in single tile .    */
        /* we have max of 4 SDMAs per tile (so the numOfTiles >= sdmaFactor)  */
        /* currently in 12.8 we use only 8 out of 16 , so we need to allow them*/
        /* amount of resources like for 6.4                                   */
        /* otherwise test like : prvTgfIpv4NonExactMatchUcRouting will fail to*/
        /* send packets from the CPU because no resources (at least in WM win/linux) */
        if(numCpuSdmas)
        {
            sdmaFactor = (numCpuSdmas + 3)/4;

            rxDescNum /= sdmaFactor * numDevsFactor;
            txDescNum /= sdmaFactor * numDevsFactor;

            if(appDemoDbEntryGet("txGenNumOfDescBuff", &tmpData) == GT_OK)
            {
                /* explicitly stated .. do not override */
            }
            else
            {
                /* NOTE: the 512 is value used inside appDemoAllocateDmaMem(...)
                   if not set by the 'DB entry add' */

                rc = appDemoDbEntryAdd("txGenNumOfDescBuff", 512 / sdmaFactor);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    rc = appDemoDbEntryAdd("fuqUseSeparate", GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(appDemoDbEntryGet("fuDescNum", &tmpData) == GT_OK)
    {
        /* Someone already stated the number of fuDescNum, so we NOT override it ! */
    }
    else
    {
        /* Add the fuDescNum to the DB, to allow appDemoAllocateDmaMem(...) to use it ! */
        rc = appDemoDbEntryAdd("fuDescNum", fuDescNum);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* cause CPU port0 queues having enough amount of buffers */
    if ((appDemoDbEntryGet("rxSdmaDescrFix_0_Add", &tmpData) != GT_OK)
        && (appDemoDbEntryGet("rxSdmaDescrFix_0_Sub", &tmpData) != GT_OK))
    {
        GT_U32 maxPacketSize = 10268; /* 10K + 16 (DSA tag) + 16 (spare) */
        /* amount buffers needed for packet and one empty */
        GT_U32 neededBufAmountPerQueue =
            ((maxPacketSize + RX_BUFF_SIZE_DEF - 1) / RX_BUFF_SIZE_DEF) + 1; /* 8 */
        /* rxDescNum is amount of descriptors per 1 SDMA, i.e. per 8 queues */
        /* it is appDemoAllocateDmaMem use of this parameter                */
        GT_U32 defaultBufAmountPerQueue = (rxDescNum / 8);
        GT_U32 addedBufAmountPerQueue =
            (neededBufAmountPerQueue > defaultBufAmountPerQueue)
            ? (neededBufAmountPerQueue - defaultBufAmountPerQueue) : 0;
        GT_U32 subtractedBufAmountPerQueue;
        GT_U32 numberOfCpuSdmas;
        GT_U32 numberOfSubtractedQueues;
        GT_U32 value;
        GT_U32 finalAddedBufAmountPerQueue;

        if (addedBufAmountPerQueue > 0)
        {
            numberOfCpuSdmas = (numCpuSdmas != 0) ? numCpuSdmas : 4;
            numberOfSubtractedQueues = ((numberOfCpuSdmas - 1) * 8);
            subtractedBufAmountPerQueue =
                (((addedBufAmountPerQueue * 8) + (numberOfSubtractedQueues - 1))
                    / numberOfSubtractedQueues);
            if (subtractedBufAmountPerQueue > (defaultBufAmountPerQueue - 1))
            {
                subtractedBufAmountPerQueue = (defaultBufAmountPerQueue - 1);
            }

            finalAddedBufAmountPerQueue =
                ((subtractedBufAmountPerQueue * numberOfSubtractedQueues) / 8);

            /* sdmaBase=0, sdmaBmp=0x01, queueBmp=0xFF */
            value = 0x0001FF00 | (finalAddedBufAmountPerQueue & 0xFF);
            appDemoDbEntryAdd("rxSdmaDescrFix_0_Add", value);
            /* sdmaBase=0, sdmaBmp=0xFE, queueBmp=0xFF */
            value = 0x00FEFF00 | (subtractedBufAmountPerQueue & 0xFF);
            appDemoDbEntryAdd("rxSdmaDescrFix_0_Sub", value);
            /* sdmaBase=8, sdmaBmp=0xFF, queueBmp=0xFF */
            value = 0x08FFFF00 | (subtractedBufAmountPerQueue & 0xFF);
            appDemoDbEntryAdd("rxSdmaDescrFix_1_Sub", value);
        }
    }

    appDemoCpssCurrentDevIndex = devIndex;

    rc = appDemoAllocateDmaMem(devType,
                               /* Rx */
                               rxDescNum,RX_BUFF_SIZE_DEF,RX_BUFF_ALLIGN_DEF,
                               /* Tx */
                               txDescNum,
                               /* AUQ */
                               auDescNum,
                               &appDemo_ppPhase2Params);

    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoAllocateDmaMem", rc);
    if(rc != GT_OK)
        return rc;

    if(overrideDmaReduceFactor)
    {
        /* restore the value that we temporarily 'disabled' */
        appDemoDbEntryAdd("dma_reduce_factor", overrideDmaReduceFactor);
    }

    /* convert CPSS_PP_PHASE2_INIT_PARAMS to CPSS_DXCH_PP_PHASE2_INIT_INFO_STC */
    phase2Params->netIfCfg          = appDemo_ppPhase2Params.netIfCfg         ;
    phase2Params->auqCfg            = appDemo_ppPhase2Params.auqCfg           ;
    phase2Params->fuqUseSeparate    = appDemo_ppPhase2Params.fuqUseSeparate   ;
    phase2Params->fuqCfg            = appDemo_ppPhase2Params.fuqCfg           ;
    phase2Params->useSecondaryAuq   = appDemo_ppPhase2Params.useSecondaryAuq  ;
    phase2Params->useDoubleAuq      = appDemo_ppPhase2Params.useDoubleAuq     ;
    phase2Params->useMultiNetIfSdma = appDemo_ppPhase2Params.useMultiNetIfSdma;
    phase2Params->multiNetIfCfg     = appDemo_ppPhase2Params.multiNetIfCfg    ;

    return rc;
}

typedef struct{
    GT_U32   firstDma;
    GT_U32   numPorts;
}SDMA_RANGE_INFO_STC;

/**
* @internal falcon_initPortDelete_WA function
* @endinternal
*
*   purpose : state CPSS what reserved resources it should use for the 'port delete' WA.
*   actually the WA is activated during 'cpssDxChPortModeSpeedSet(...,powerUp = GT_FALSE)'
*   NOTE: the function gets the same parameters as the cpssDxChPortPhysicalPortMapSet(...)
*       because we need to find non-used 'physical port number' and to 'steal'
*       one SDMA queue number.
*/
static GT_STATUS falcon_initPortDelete_WA
(
    IN GT_SW_DEV_NUM       devNum,
    IN  GT_U32                      portMapArraySize,
    IN  CPSS_DXCH_PORT_MAP_STC      portMapArrayPtr[]
)
{
    GT_STATUS   rc;
    GT_U32  ii,jj;
    CPSS_DXCH_PORT_MAP_STC      *currPtr = &portMapArrayPtr[0];
    CPSS_PORTS_BMP_STC          usedPhyPorts;
    MUX_DMA_STC                 *muxed_SDMAs_Ptr;
    SDMA_RANGE_INFO_STC         falcon_3_2_SDMAs_available  = { 64 +  4 ,  4};
    SDMA_RANGE_INFO_STC         falcon_6_4_SDMAs_available  = {128 +  8 ,  8};
    SDMA_RANGE_INFO_STC         falcon_12_8_SDMAs_available = {256 + 16 , 16};
    SDMA_RANGE_INFO_STC         *SDMAs_available_Ptr;
    CPSS_PORTS_BMP_STC          availableSDMAPorts;/* note : this is BMP of MAC/DMA ... not of physical ports */
    GT_U32                      maxPhyPorts;
    CPSS_DXCH_HW_PP_IMPLEMENT_WA_INIT_FALCON_PORT_DELETE_STC    waInfo;
    GT_U32                      numOfMappedSdma;

    maxPhyPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);

    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&usedPhyPorts);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&availableSDMAPorts);

    muxed_SDMAs_Ptr = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 4 ? falcon_12_8_muxed_SDMAs :
                      PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 2 ? falcon_6_4_muxed_SDMAs  :
                                                   /* single tile*/        falcon_3_2_muxed_SDMAs  ;

    SDMAs_available_Ptr = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 4 ? &falcon_12_8_SDMAs_available :
                          PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 2 ? &falcon_6_4_SDMAs_available  :
                                                       /* single tile*/        &falcon_3_2_SDMAs_available  ;

    /**********************************************************************/
    /********** start by search for CPU SDMA to be used by the WA *********/
    /**********************************************************************/

    /* build BMP of available SDMA ports that can be used */
    for(ii = SDMAs_available_Ptr->firstDma ;
        ii < SDMAs_available_Ptr->firstDma + SDMAs_available_Ptr->numPorts ;
        ii++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&availableSDMAPorts, ii);
    }

    for(ii = 0 ; ii < portMapArraySize; ii++,currPtr++)
    {
        /* state that this physical port is used */
        CPSS_PORTS_BMP_PORT_SET_MAC(&usedPhyPorts, currPtr->physicalPortNumber);

        /* for CPU SDMA , find the highest 'physical port number' and still it's 'queue 7' */
        if(currPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
        {
            /* state that this DMA port is used as CPU SDMA */
            /* note : this is BMP of DMAs ... not of physical ports */
            CPSS_PORTS_BMP_PORT_CLEAR_MAC(&availableSDMAPorts, currPtr->interfaceNum);
        }
        else
        {
            for(jj = 0 ; muxed_SDMAs_Ptr[jj].sdmaPort != GT_NA ; jj++)
            {
                if(currPtr->interfaceNum == muxed_SDMAs_Ptr[jj].macPort)
                {
                    /* since the application uses this MAC that is muxed with the SDMA ...
                       it is not free for the WA */
                    CPSS_PORTS_BMP_PORT_CLEAR_MAC(&availableSDMAPorts, muxed_SDMAs_Ptr[jj].sdmaPort);
                    break;
                }
            }
        }
    }

    if(CPSS_PORTS_BMP_IS_ZERO_MAC(&availableSDMAPorts))
    {
        /* the application uses ALL the SDMAs that the device can offer */
        /* so we will steal the 'queue 7,6' of the 'last one' */
        waInfo.reservedCpuSdmaGlobalQueue[0] = (numCpuSdmas * 8) - 1;
        waInfo.reservedCpuSdmaGlobalQueue[1] = (numCpuSdmas * 8) - 2;
    }
    else
    {
        rc = prvCpssDxChNetIfMultiNetIfNumberGet(devNum, &numOfMappedSdma);
        CPSS_ENABLER_DBG_TRACE_RC_MAC(
            "cpssDxChHwPpImplementWaInit_FalconPortDelete: prvCpssDxChNetIfMultiNetIfNumberGet", rc);
        if (GT_OK != rc)
        {
            return rc;
        }

        /* the highest used SDMAs, not used SDMAs not supporte by WA implementation */
        waInfo.reservedCpuSdmaGlobalQueue[0] = (numOfMappedSdma * 8) - 1;
        waInfo.reservedCpuSdmaGlobalQueue[1] = (numOfMappedSdma * 8) - 2;
        /*  next not supported (yet) by the CPSS
            waInfo.reservedCpuSdmaGlobalQueue[0]         = 0xFFFFFFFF;
            waInfo.reservedCpuSdmaGlobalQueue[1]         = 0xFFFFFFFF;
        */
    }

    if(waInfo.reservedCpuSdmaGlobalQueue[0] == 7)/* single CPU port */
    {
        waInfo.reservedCpuSdmaGlobalQueue[0] = 6;/* the '7' is reserved for all 'from cpu' tests (LUA/enh-UT) */
        waInfo.reservedCpuSdmaGlobalQueue[1] = 5;
    }

    /* saved for later use */
    reservedCpuSdmaGlobalQueue[0] = waInfo.reservedCpuSdmaGlobalQueue[0];
    reservedCpuSdmaGlobalQueue[1] = waInfo.reservedCpuSdmaGlobalQueue[1];

    if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&usedPhyPorts , 61))
    {
        /* check to use 61 as 'default' because it stated as 'special port' that
           should not be used by the application in the FS */
        /* but it is for the L2 traffic that is used by the WA */
        waInfo.reservedPortNum = 61;/* use 61 regardless to the port mode (64/128/512/1024) */
    }
    else
    {
        /**********************************************************************/
        /********** start the search for physical port to be used by the WA ***/
        /**********************************************************************/
        for(ii = maxPhyPorts-1 ; ii ; ii--)
        {
            if(ii == CPSS_CPU_PORT_NUM_CNS ||
               ii == CPSS_NULL_PORT_NUM_CNS)
            {
                /* not valid numbers to use ! */
                continue;
            }

            if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&usedPhyPorts , ii))
            {
                /* not used */
                break;
            }
        }

        if(ii == 0)
        {
            DBG_TRACE((" falcon_initPortDelete_WA : not found free physical port number ?! \n"));
            return GT_FULL;
        }

        waInfo.reservedPortNum = ii;
    }

    /*************************************************/
    /********** State the tail drop profile to use ***/
    /*************************************************/
    waInfo.reservedTailDropProfile = 15;


    /**********************************/
    /* call the CPSS to have the info */
    /**********************************/
    rc = cpssDxChHwPpImplementWaInit_FalconPortDelete(CAST_SW_DEVNUM(devNum),&waInfo);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpImplementWaInit_FalconPortDelete", rc);
    if (GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}
#if !defined(ASIC_SIMULATION) && defined(INCLUDE_MPD)

/**
 * mydHwCtrlRWMutexCreate
 *
 *
 * @return MYD_STATUS
 */
MYD_STATUS appDemo_phy_RWMutexCreate()
{
    char buf[64]={0};

    cpssOsMutexCreate(buf, &xsmi_extn_mtx);
    return MYD_OK;
}

/**
 * mydHwCtrlRWMutexDestroy
 *
 *
 * @return MYD_STATUS
 */
MYD_STATUS appDemo_phy_RWMutexDestroy()
{
    cpssOsMutexDelete(xsmi_extn_mtx);
    return MYD_OK;
}

/**
* @internal falcon_initPhys function
* @endinternal
*
*/
GT_STATUS falcon_phy_init
(
   IN GT_SW_DEV_NUM      devNum,
   IN GT_BOOL            loadImage
)
{
    GT_STATUS                    rc;
    GT_U32                       phyIndex;

     GT_U32                       ravenDevBmp, cardsBmp /*tmpData=0*/;
    GT_U32                       currBoardType;

    rc = appDemoBoardTypeGet(&currBoardType);
    if (rc != GT_OK)
    {
        return rc;
    }

    if ( ( (currBoardType != APP_DEMO_FALCON_BOARD_12_8_TH_CNS) &&
           (currBoardType != APP_DEMO_FALCON_BOARD_12_8_RD_CNS) ) ||
            (falcon_special_th_interposer_board) )
    {
        return GT_OK;
    }

    if(appDemoDbEntryGet("ravenDevBmp", &ravenDevBmp) != GT_OK)
    {
        ravenDevBmp = 0xffff;
    }
    if(appDemoDbEntryGet("cardsBmp", &cardsBmp) != GT_OK)
    {
        cardsBmp = 0xff;
    }

    for (phyIndex = 0; phyIndex < falcon_PhyInfo_arrSize; phyIndex++)
    {

        if (currBoardType == APP_DEMO_FALCON_BOARD_12_8_TH_CNS)
        {
            if(ravenDevBmp & (1<<falcon_PhyInfo[phyIndex].portGroupId))
            {
                falcon_PhyInfo[phyIndex].hostDevNum = devNum; /* validate phy according to Raven's bmp */
            }
        }
        else if (currBoardType == APP_DEMO_FALCON_BOARD_12_8_RD_CNS)
        {
            if(cardsBmp & (1<<falcon_PhyInfo[phyIndex].portGroupId))
            {
                falcon_PhyInfo[phyIndex].hostDevNum = devNum; /* validate phy according to Slots's bmp */
            }
        }
        rc=cpssDxChPhyXsmiMdcDivisionFactorSet(devNum, falcon_PhyInfo[phyIndex].xsmiInterface,
                                                         CPSS_PHY_SMI_MDC_DIVISION_FACTOR_32_E); /* Speed-up XSMI */
        if (rc != GT_OK)
        {
            return rc;
        }

    }
    if(loadImage ==GT_TRUE)
    {
        rc = gtAppDemoPhyMpdInit(devNum, 0);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    appDemo_phy_RWMutexCreate();

    return GT_OK;
}
#endif

/**
* @internal falcon_initPortMappingStage function
* @endinternal
*
*/
static GT_STATUS falcon_initPortMappingStage
(
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PP_DEVICE_TYPE      devType
)
{
    GT_STATUS   rc;
    CPSS_DXCH_PORT_MAP_STC *mapArrPtr;
    GT_U32 mapArrLen;
    CPSS_DXCH_PORT_MAP_STC *mapUpdatedArrPtr;
    GT_U32 mapArrUpdatedLen;

    switch(devType)
    {
        case CPSS_FALCON_ALL_DEVICES_CASES_MAC:
        default:
            if(useSinglePipeSingleDp == 0)
            {
                ARR_PTR_AND_SIZE_MAC(cpssApi_falcon_defaultMap, mapArrPtr, mapArrLen);
                mapArrLen = actualNum_cpssApi_falcon_defaultMap;
            }
            else
            {
                ARR_PTR_AND_SIZE_MAC(singlePipe_singleDp_cpssApi_falcon_defaultMap, mapArrPtr, mapArrLen);
            }
            break;
    }

    if(appDemoPortMapPtr && appDemoPortMapSize)
    {
        /* allow to 'FORCE' specific array by other logic */
        mapArrPtr = appDemoPortMapPtr;
        mapArrLen = appDemoPortMapSize;
    }

    rc = appDemoDxChPortPhysicalPortUpdatedMapCreate(
        mapArrLen, mapArrPtr, &mapArrUpdatedLen, &mapUpdatedArrPtr);
    if (GT_OK != rc)
    {
        return rc;
    }
    if (mapUpdatedArrPtr != NULL)
    {
        mapArrPtr = mapUpdatedArrPtr;
        mapArrLen = mapArrUpdatedLen;
    }

    rc = cpssDxChPortPhysicalPortMapSet(CAST_SW_DEVNUM(devNum), mapArrLen, mapArrPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPhysicalPortMapSet", rc);
    if (GT_OK != rc)
    {
        if (mapUpdatedArrPtr != NULL)
        {
            cpssOsFree(mapUpdatedArrPtr);
        }
        return rc;
    }

    if(appDemoPortMapPtr && appDemoPortMapSize)
    {
        /* update number of CPU ports */
        rc = prvCpssDxChNetIfMultiNetIfNumberGet(CAST_SW_DEVNUM(devNum),&numCpuSdmas);
        if (rc != GT_OK)
        {
            if (mapUpdatedArrPtr != NULL)
            {
                cpssOsFree(mapUpdatedArrPtr);
            }
            return rc;
        }
    }

    if(!falcon_initPortDelete_WA_disabled)/* if the WA initialization is not disabled */
    {
        /****************************************************************************/
        /* do init for the 'port delete' WA.                                        */
        /* with the same parameters as the cpssDxChPortPhysicalPortMapSet(...) used */
        /****************************************************************************/
        rc = falcon_initPortDelete_WA(devNum, mapArrLen, mapArrPtr);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_initPortDelete_WA", rc);
        if (rc != GT_OK)
        {
            if (mapUpdatedArrPtr != NULL)
            {
                cpssOsFree(mapUpdatedArrPtr);
            }
            return rc;
        }
    }

    /* it's for appdemo DB only */
    rc = appDemoDxChMaxMappedPortSet(CAST_SW_DEVNUM(devNum), mapArrLen, mapArrPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxChMaxMappedPortSet", rc);

    if (mapUpdatedArrPtr != NULL)
    {
        cpssOsFree(mapUpdatedArrPtr);
    }
    return rc;
}

static PortInitList_STC falcon_PortReducedIntListArr[16];
static GT_U32 falcon_PortReducedIntListSize =
    (sizeof(falcon_PortReducedIntListArr) / sizeof(falcon_PortReducedIntListArr[0]));


/**
* @internal falcon_PortModeSpeedTableReguceTo25G function
* @endinternal
*
*/
static GT_STATUS falcon_PortModeSpeedTableReguceTo25G
(
    IN PortInitList_STC *portInitListPtr
)
{
    GT_U32 i;
    GT_U32 portInitListSize;
    GT_U32 numOfSerdes = 0;

    for (i = 0; (i < falcon_PortReducedIntListSize); i++)
    {
        falcon_PortReducedIntListArr[i].entryType = PORT_LIST_TYPE_EMPTY;
    }

    for (i = 0; (portInitListPtr[i].entryType != PORT_LIST_TYPE_EMPTY); i++){};
    portInitListSize = i;
    if (portInitListSize > falcon_PortReducedIntListSize)
    {
        cpssOsPrintf("falcon_PortModeSpeedTableReguceTo25G table is too big\n");
        return GT_FAIL;
    }

    for (i = 0; (i < portInitListSize); i++)
    {
        falcon_PortReducedIntListArr[i] = portInitListPtr[i];
        switch (falcon_PortReducedIntListArr[i].interfaceMode)
        {
            case CPSS_PORT_INTERFACE_MODE_KR_E:  numOfSerdes = 1; break;
            case CPSS_PORT_INTERFACE_MODE_KR2_E: numOfSerdes = 2; break;
            case CPSS_PORT_INTERFACE_MODE_KR4_E: numOfSerdes = 4; break;
            case CPSS_PORT_INTERFACE_MODE_REMOTE_E: break;
            default:
                cpssOsPrintf("falcon_PortModeSpeedTableReguceTo25G not supported port mode\n");
                return GT_FAIL;
        }
        switch (falcon_PortReducedIntListArr[i].speed)
        {
            case CPSS_PORT_SPEED_NA_E:
            case CPSS_PORT_SPEED_10000_E:
            case CPSS_PORT_SPEED_25000_E:
            case CPSS_PORT_SPEED_REMOTE_E:
                /* good speeds - leave as is */
                break;
            case CPSS_PORT_SPEED_50000_E:
                if (numOfSerdes == 1)
                {
                    falcon_PortReducedIntListArr[i].speed = CPSS_PORT_SPEED_25000_E;
                }
                /* otherwise - leave as is */
                break;
            case CPSS_PORT_SPEED_100G_E:
                if (numOfSerdes == 1)
                {
                    cpssOsPrintf("falcon_PortModeSpeedTableReguceTo25G not supported port speed\n");
                    return GT_FAIL;
                }
                if (numOfSerdes == 2)
                {
                    falcon_PortReducedIntListArr[i].speed = CPSS_PORT_SPEED_50000_E;
                }
                /* otherwise - leave as is */
                break;
            default:
                cpssOsPrintf("falcon_PortModeSpeedTableReguceTo25G not supported port speed\n");
                return GT_FAIL;
        }
    }
    falcon_PortReducedIntListArr[portInitListSize] = portInitListPtr[portInitListSize];

    return GT_OK;
}

/**
* @internal falcon_PortModeConfiguration function
* @endinternal
*
*/
static GT_STATUS falcon_PortModeConfiguration
(
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PP_DEVICE_TYPE      devType
)
{
    GT_STATUS                       rc;
    PortInitList_STC                *portInitListPtr = NULL;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery;
    GT_U32                          maxPhyPorts;
    GT_U32                          currBoardType;
    GT_BOOL                         portModeExcelAllignedMapping = GT_FALSE;
    GT_U32                          value;

    rc = appDemoBoardTypeGet(&currBoardType);
    if (rc != GT_OK)
    {
        return rc;
    }

    maxPhyPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    switch(devType)
    {
        case CPSS_FALCON_ALL_DEVICES_CASES_MAC:
        default:
            if(useSinglePipeSingleDp == 0)
            {
                switch(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
                {
                    default:
                    case 0:
                    case 1:

                        if((appDemoDbEntryGet("portModeExcelAllignedMapping", &value) == GT_OK) && (value != 0))
                        {
                            if (PRV_CPSS_PP_MAC(devNum)->devType != CPSS_98EX5610_CNS &&
                                PRV_CPSS_PP_MAC(devNum)->devType != CPSS_98EX5614_CNS &&
                                PRV_CPSS_PP_MAC(devNum)->devType != CPSS_98CX8512_CNS &&
                                PRV_CPSS_PP_MAC(devNum)->devType != CPSS_98CX8514_CNS &&
                                PRV_CPSS_PP_MAC(devNum)->devType != CPSS_98CX8520_CNS &&
                                PRV_CPSS_PP_MAC(devNum)->devType != CPSS_98CX8525_CNS &&
                                PRV_CPSS_PP_MAC(devNum)->devType != CPSS_98CX8530_CNS)
                            {
                                switch(value)
                                {
                                   case 9:
                                    portModeExcelAllignedMapping = GT_TRUE;
                                    break;
                                   default:
                                    break;
                                }
                             }
                        }

                        if(GT_TRUE == portModeExcelAllignedMapping)
                        {
                            switch(value)
                            {
                               case 9:
                                portInitListPtr = falcon_3_2_port_mode_9_portInitlist;
                                break;
                               default:
                                 break;
                            }
                        }
                        else
                        if (maxPhyPorts == 64)
                        {
                            /* 100G mode*/
                             portInitListPtr = falcon_3_2_portInitlist_64_port_mode;
                        }
                        else if (maxPhyPorts == 256)
                        {
                           portInitListPtr = falcon_3_2_portInitlist_256_port_mode;
                             if(mode100G)
                            {
                                portInitListPtr = falcon_3_2_100G_portInitlist_256_port_mode;
                            }
                        }
                        else if (maxPhyPorts == 1024)
                        {
                            /* 50G mode, remote ports*/
                             portInitListPtr = falcon_3_2_portInitlist_1024_port_mode;
                             if(mode100G)
                            {
                                portInitListPtr = falcon_3_2_100G_portInitlist_1024_port_mode;
                            }
                        }
                        else /* 128, 512*/
                        {
                            if(cpuPortMux2UseSdma == 0)
                            {
                                portInitListPtr = falcon_3_2_portInitlist_TwoCpuPorts;
                            }
                            else
                            {
                                portInitListPtr = falcon_3_2_portInitlist;
                                if(mode100G)
                                {
                                    portInitListPtr = falcon_3_2_100G_portInitlist;
                                }
                            }
                        }
                        break;

                    case 2:
                        if((appDemoDbEntryGet("portModeExcelAllignedMapping", &value) == GT_OK) && (value != 0))
                        {
                            if (PRV_CPSS_PP_MAC(devNum)->devType != CPSS_98EX5610_CNS &&
                                PRV_CPSS_PP_MAC(devNum)->devType != CPSS_98EX5614_CNS &&
                                PRV_CPSS_PP_MAC(devNum)->devType != CPSS_98CX8512_CNS &&
                                PRV_CPSS_PP_MAC(devNum)->devType != CPSS_98CX8514_CNS &&
                                PRV_CPSS_PP_MAC(devNum)->devType != CPSS_98CX8520_CNS &&
                                PRV_CPSS_PP_MAC(devNum)->devType != CPSS_98CX8525_CNS &&
                                PRV_CPSS_PP_MAC(devNum)->devType != CPSS_98CX8530_CNS)
                            {
                                switch(value)
                                {
                                   case 9:
                                    portModeExcelAllignedMapping = GT_TRUE;
                                    break;
                                   default:
                                    break;
                                }
                             }
                        }

                        if((appDemoDbEntryGet("hqos", &value) == GT_OK) && (value != 0))
                        {
                            portInitListPtr = falcon_6_4_portInitlist_hqos;
                        }
                        else
                        if(GT_TRUE == portModeExcelAllignedMapping)
                        {
                            switch(value)
                            {
                               case 9:
                                portInitListPtr = falcon_6_4_port_mode_9_portInitlist;
                                break;
                               default:
                                 break;
                            }
                        }
                        else
                        if (PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98EX5610_CNS ||
                            PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98EX5614_CNS ||
                            PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98CX8512_CNS ||
                            PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98CX8514_CNS)
                        {
                            portInitListPtr = armstrong2_80x25g_portInitlist;
                        }
                        else if (PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98CX8520_CNS ||
                                 PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98CX8525_CNS)
                        {
                            portInitListPtr = falcon_64x50g_portInitlist;
                        }
                        else if (PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98CX8530_CNS ||
                                 PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98CX8535_CNS ||
                                 PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98CX8535_H_CNS)
                        {
                            portInitListPtr = falcon_80x50g_portInitlist;
                        }
                        else if(mode100G)
                        {
                            portInitListPtr = falcon_6_4_100G_portInitlist;
                        }
                        else if(modeMaxMac)
                        {
                            if (maxPhyPorts == 256)
                            {
                                portInitListPtr = falcon_6_4_mode_256_maxMAC_portInitlist;
                            }
                            else
                            {
                                portInitListPtr = falcon_6_4_mode_128_maxMAC_portInitlist;
                            }
                        }
                        else if (maxPhyPorts == 64)
                        {
                            /* 100G mode*/
                             portInitListPtr = falcon_6_4_portInitlist_64_port_mode;
                        }
                        else if (maxPhyPorts == 256)
                        {
                             portInitListPtr = falcon_6_4_portInitlist_256_port_mode;
                        }
                        else if (maxPhyPorts == 1024)
                        {
                             /* 50G mode, remote ports*/
                             portInitListPtr = falcon_6_4_portInitlist_1024_port_mode;
                        }
                        else /*128, 512*/
                        {
                            portInitListPtr = falcon_6_4_portInitlist;
                        }

                        break;

                    case 4:

                         if((appDemoDbEntryGet("portModeExcelAllignedMapping", &value) == GT_OK) && (value != 0))
                         {
                             switch(value)
                             {
                                case 25:
                                 portModeExcelAllignedMapping = GT_TRUE;
                                 break;
                                default:
                                 break;
                             }
                         }

                        if((appDemoDbEntryGet("hqos", &value) == GT_OK) && (value != 0))
                        {
                            portInitListPtr = falcon_12_8_portInitlist_hqos;
                        }
                        else  if((appDemoDbEntryGet("rppEmptyTile", &value) == GT_OK) && (value != 0))
                        {
                            portInitListPtr = falcon_12_8_rpp_empty_tile;
                        }
                        else  if(modeSonic32MacPorts)
                        {
                            portInitListPtr = falcon_12_8_portInitlist_64_port_mode_sonic;
                        }
                        else
                        if(modeSonic128MacPorts)
                        {
                            portInitListPtr = falcon_12_8_portInitlist_port_mode_256_b2b_modeSonic128MacPorts;
                        }
                        else
                        if(GT_TRUE == portModeExcelAllignedMapping)
                        {
                            switch(value)
                            {
                               case 25:
                                portInitListPtr = falcon_12_8_port_mode_25_portInitlist;
                                break;
                               default:
                                 break;
                            }
                        }
                        else
                        if(PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98CX8550_CNS)
                        {
                            switch(maxPhyPorts)
                            {
                                case 64:
                                    portInitListPtr = falcon_160x50g_64_portInitlist;
                                    break;
                                case 128:
                                    portInitListPtr = falcon_160x50g_128_portInitlist;
                                    break;
                                default:
                                    portInitListPtr = falcon_160x50g_maxPorts_portInitlist;
                                    break;
                            }
                        }
                        else
                        if (maxPhyPorts == 64)
                        {
                            /* 100G mode*/
                             portInitListPtr = falcon_12_8_portInitlist_64_port_mode;
                        }
                        else if (maxPhyPorts == 256)
                        {
                            /* 50G mode*/
                            portInitListPtr = falcon_12_8_portInitlist_256_port_mode;
                        }
                        else if (maxPhyPorts == 512)
                        {
                            /* 50G mode*/
                            portInitListPtr = falcon_12_8_portInitlist_512_port_mode;
                        }
                        else if (maxPhyPorts == 1024)
                        {
                             /* 50G mode, remote ports*/
                             portInitListPtr = falcon_12_8_portInitlist_1024_port_mode;
                        }
                        else /*128*/
                        {
                            if(cpuPortMux2UseSdma == 0)
                            {
                                portInitListPtr = falcon_12_8_portInitlist_twoCpuPorts;
                            }
                            else
                            {
                                portInitListPtr = falcon_12_8_portInitlist;
                            }
                        }

                        break;
                }

            }
            else
            {
                portInitListPtr = singlePipe_singleDp_portInitlist_falcon;
            }
            break;
    }

    if ((PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98CX8522_CNS)
        || (PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98CX8542_CNS))
    {
        if(portInitListPtr == NULL)
        {
            cpssOsPrintf("ERROR portInitListPtr == NULL \n");
            return GT_BAD_STATE;
        }

        rc = falcon_PortModeSpeedTableReguceTo25G(portInitListPtr);
        if (rc != GT_OK)
        {
            cpssOsPrintf("falcon_PortModeSpeedTableReguceTo25G failed\n");
            return rc;
        }
        portInitListPtr = falcon_PortReducedIntListArr;
    }

    if(currBoardType == APP_DEMO_FALCON_BOARD_12_8_RD_CNS)
    { /* force Falcon RD port mode list*/
        falcon_force_PortsInitListPtr = falcon_12_8_rd_slot_portInitlist_256_port_mode;
    }


    if(falcon_force_PortsInitListPtr)
    {
        /* allow to 'FORCE' specific array by other logic */
        portInitListPtr = falcon_force_PortsInitListPtr;
    }

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E == system_recovery.systemRecoveryProcess)
    {
        if ( portInitListPtr == NULL )
        {
            return GT_FAIL;
        }
        rc = appDemoBc2PortListInit(CAST_SW_DEVNUM(devNum), portInitListPtr, GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* indication for function appDemoBc2PortInterfaceInit(...) to
           not call appDemoBc2PortListInit(...) */
        appDemo_PortsInitList_already_done = 1;
    }

    return GT_OK;
}

/* flag to control if SW reset needed on GT_HW_ERROR_NEED_RESET returned by 'phase 1 init' */
/* when flag not set and getting GT_HW_ERROR_NEED_RESET the cpssInitSystem breaks and allow
    read registers and some APIs */
static GT_U32 falcon_do_soft_reset_on_GT_HW_ERROR_NEED_RESET_phase_1_init  = 1;
GT_STATUS falcon_do_soft_reset_on_GT_HW_ERROR_NEED_RESET_phase_1_init_set(IN GT_U32 allowSoftReset)
{
    falcon_do_soft_reset_on_GT_HW_ERROR_NEED_RESET_phase_1_init = allowSoftReset;
    return GT_OK;
}

/* flag to force soft reset regardless to PASS/FAIL of 'phase 1 init' */
static GT_U32 falcon_force_soft_reset_after_phase1_init = 0;
GT_STATUS falcon_force_soft_reset_after_phase1_init_set(void)
{
    falcon_force_soft_reset_after_phase1_init = 1;
    return GT_OK;
}

/* flag to run 'phase 1 init' in loop (with soft reset)! to allow to stress the HW */
static GT_U32 falcon_force_phase_1_init_in_loop = 0;
GT_STATUS falcon_force_phase_1_init_in_loop_set(void)
{
    falcon_force_phase_1_init_in_loop = 1;
    return GT_OK;
}

extern GT_U32 falcon_force_stop_init_after_d2d_init_get(void);

/* flag to allow kill the D2D of Raven 3 so we can test soft reset taht recover from it */
static GT_U32 falcon_force_kill_d2d_link_Raven_3 = 0;
GT_STATUS falcon_force_kill_d2d_link_Raven_3_set(void)
{
    falcon_force_kill_d2d_link_Raven_3 = 5;
    return GT_OK;
}

extern GT_STATUS    doAtomicDeviceResetAndRemove(IN GT_U8   devNum);
GT_STATUS falcon_force_early_check_for_device_not_reset_set(void);
/**
* @internal falcon_initPhase1AndPhase2 function
* @endinternal
*
*/
static GT_STATUS falcon_initPhase1AndPhase2
(
    IN GT_SW_DEV_NUM       devNum,
    IN GT_U8              boardRevId,
    IN CPSS_HW_INFO_STC   *hwInfoPtr,
    IN GT_U32             devIndex,
    OUT CPSS_PP_DEVICE_TYPE     *devTypePtr
)
{
    GT_STATUS   rc;
    CPSS_DXCH_PP_PHASE1_INIT_INFO_STC       cpssPpPhase1Info;     /* CPSS phase 1 PP params */
    CPSS_DXCH_PP_PHASE2_INIT_INFO_STC       cpssPpPhase2Info;     /* CPSS phase 2 PP params */
    GT_HW_DEV_NUM           hwDevNum;
    CPSS_PP_DEVICE_TYPE     devType;
    GT_U32                  waIndex;
    static CPSS_DXCH_IMPLEMENT_WA_ENT falcon_WaList[] =
    {
        /* no current WA that need to force CPSS */
        CPSS_DXCH_IMPLEMENT_WA_LAST_E
    };
    GT_U32  softResetAfterPhase1Init = 0;
    GT_U32  phase1InitCounter = 0;

    osMemSet(&cpssPpPhase1Info, 0, sizeof(cpssPpPhase1Info));
    osMemSet(&cpssPpPhase2Info, 0, sizeof(cpssPpPhase2Info));

#if 0
    /* MUST not call falcon_force_early_check_for_device_not_reset_set()
      regular setup as it kills LUA test of 'gtShutdownAndCoreRestart' */
    /* but for 'sonic' like environment , this is needed for quick recognition
       that the Falcon did not do reset before this init
       and it 'saves' time for SONIC to load
    */
    falcon_force_early_check_for_device_not_reset_set();
#endif

    rc = getPpPhase1ConfigSimple(devNum, boardRevId, &cpssPpPhase1Info);
    if (rc != GT_OK)
        return rc;

    cpssPpPhase1Info.hwInfo[0] = *hwInfoPtr;

phase_1_init_lbl:
    phase1InitCounter++;
    /* devType is retrieved in hwPpPhase1Part1*/
    rc = cpssDxChHwPpPhase1Init(&cpssPpPhase1Info, &devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpPhase1Init", rc);
    if((rc == GT_HW_ERROR_NEED_RESET && falcon_do_soft_reset_on_GT_HW_ERROR_NEED_RESET_phase_1_init))
    {
        cpssOsPrintf("iteration [%d] ERROR : 'phase 1 init' failed on 'GT_HW_ERROR_NEED_RESET' , do soft reset and retry 'phase 1 init' \n",
            phase1InitCounter);
        softResetAfterPhase1Init = 1;
    }
    else
    if(falcon_force_soft_reset_after_phase1_init || falcon_force_phase_1_init_in_loop)
    {
        if(rc == GT_OK)
        {
            cpssOsPrintf("-- 'phase 1 init' PASSED , but FORCED to do soft reset and retry \n");
        }
        else
        {
            cpssOsPrintf("iteration [%d] ERROR : 'phase 1 init' failed , and FORCED to do soft reset and retry \n",
                phase1InitCounter);
        }
        softResetAfterPhase1Init = 1;
    }
    else
    {
        softResetAfterPhase1Init = 0;
    }

    if(softResetAfterPhase1Init)
    {
        if(rc == GT_OK &&
            falcon_force_kill_d2d_link_Raven_3 &&
            ((falcon_force_kill_d2d_link_Raven_3-1) == (phase1InitCounter % falcon_force_kill_d2d_link_Raven_3)))
        {
            GT_U32  regValue;

            cpssOsPrintf("NOTE : killing D2D on Raven 3 (to check soft reset) every [%d] iterations \n",
                falcon_force_kill_d2d_link_Raven_3);

            appDemoEventFatalErrorEnable(CPSS_ENABLER_FATAL_ERROR_NOTIFY_ONLY_TYPE_E);
            cpssDrvPpHwRegisterRead(CAST_SW_DEVNUM(devNum),0,0x03300050,&regValue);
            cpssDrvPpHwRegBitMaskWrite(CAST_SW_DEVNUM(devNum),0,0x03687000,0x3,0x2);
            cpssDrvPpHwRegBitMaskWrite(CAST_SW_DEVNUM(devNum),0,0x03687000,0x3,0x1);
            cpssDrvPpHwRegisterRead(CAST_SW_DEVNUM(devNum),0,0x03300050,&regValue);
            cpssDrvPpHwRegisterRead(CAST_SW_DEVNUM(devNum),0,0x03300050,&regValue);
            if(regValue != 0xFFFFFFFF)
            {
                cpssOsPrintf("WARNING : the Raven not lost link on the D2D and get value [0x%8.8x] (expected 0xFFFFFFFF)\n",
                    regValue);
            }

            appDemoEventFatalErrorEnable(CPSS_ENABLER_FATAL_ERROR_NOTIFY_ONLY_TYPE_E);
            cpssDrvPpHwRegisterRead(CAST_SW_DEVNUM(devNum),0,0x23300050,&regValue);
            cpssDrvPpHwRegBitMaskWrite(CAST_SW_DEVNUM(devNum),0,0x23687000,0x3,0x2);
            cpssDrvPpHwRegBitMaskWrite(CAST_SW_DEVNUM(devNum),0,0x23687000,0x3,0x1);
            cpssDrvPpHwRegisterRead(CAST_SW_DEVNUM(devNum),0,0x23300050,&regValue);
            cpssDrvPpHwRegisterRead(CAST_SW_DEVNUM(devNum),0,0x23300050,&regValue);
            if(regValue != 0xFFFFFFFF)
            {
                cpssOsPrintf("WARNING : the Raven 7 not lost link on the D2D and get value [0x%8.8x] (expected 0xFFFFFFFF)\n",
                    regValue);
            }
        }

        doAtomicDeviceResetAndRemove(CAST_SW_DEVNUM(devNum));
        osTimerWkAfter(100);

        if(falcon_force_phase_1_init_in_loop)
        {
            cpssOsPrintf("loop iteration [%d] : call again to 'phase-1' init \n",
                phase1InitCounter);
            goto  phase_1_init_lbl;
        }


        cpssOsPrintf("call again to 'phase-1' init \n");

        if(phase1InitCounter < 5)
        {
            cpssOsPrintf("loop iteration [%d] : call again to 'phase-1' init \n",
                phase1InitCounter);
            goto  phase_1_init_lbl;
        }
        else
        {
            rc = cpssDxChHwPpPhase1Init(&cpssPpPhase1Info, &devType);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpPhase1Init - LAST time", rc);
        }

    }

    if (rc != GT_OK)
        return rc;

    if(falcon_force_stop_init_after_d2d_init_get())
    {
        cpssOsPrintf("the 'phase-1' PASSED , stop after D2D init \n");
        return GT_OK;
    }


    /* check if the device did HW reset (soft/hard) */
    appDemoDxChCheckIsHwDidHwReset(devNum);

    rc = appDemoDxChDeviceIdAndRevisionPrint(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxChDeviceIdAndRevisionPrint", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    *devTypePtr = devType;

    /* save phase 1 parameters to local DB */
    ppPhase1ParamsDb = cpssPpPhase1Info;

    /* update the appDemo DB */
    rc = falcon_appDemoDbUpdate(devIndex, devNum , devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_appDemoDbUpdate", rc);
    if (rc != GT_OK)
        return rc;

    if (appDemoInitRegDefaults != GT_FALSE)
    {
        return GT_OK;
    }

    rc = appDemo_configure_dma_per_devNum(CAST_SW_DEVNUM(devNum) , GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }

    waIndex = 0;
    while(falcon_WaList[waIndex] != CPSS_DXCH_IMPLEMENT_WA_LAST_E)
    {
        waIndex++;
    }

    if(waIndex)
    {
        rc = cpssDxChHwPpImplementWaInit(CAST_SW_DEVNUM(devNum),waIndex,falcon_WaList, NULL);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpImplementWaInit", rc);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /*************************************************/
    /* fill once the DB of 'port mapping' to be used */
    /*************************************************/
    rc = fillDbForCpssPortMappingInfo(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("fillDbForCpssPortMappingInfo", rc);
    if (rc != GT_OK)
        return rc;

    rc = falcon_initPortMappingStage(devNum,devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_initPortMappingStage", rc);
    if (GT_OK != rc)
        return rc;

    /* memory related data, such as addresses and block lenghts, are set in this funtion*/
    rc = getPpPhase2ConfigSimple(devIndex , devNum, devType, &cpssPpPhase2Info);
    if (rc != GT_OK)
        return rc;

    rc = cpssDxChHwPpPhase2Init(CAST_SW_DEVNUM(devNum),  &cpssPpPhase2Info);
    if (rc != GT_OK)
        return rc;
    if (appDemoCpssPciProvisonDone == GT_FALSE)
    {
        hwDevNum = (devNum + appDemoHwDevNumOffset) & 0x3FF;
    }
    else
    {
        hwDevNum = appDemoPpConfigList[devIndex].hwDevNum;
    }


    /* set HWdevNum related values */
    rc = appDemoDxHwDevNumChange(CAST_SW_DEVNUM(devNum), hwDevNum);
    if (GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal falcon_getNeededLibs function
* @endinternal
*
*/
static GT_STATUS falcon_getNeededLibs(
    OUT  APP_DEMO_LIB_INIT_PARAMS    *libInitParamsPtr,
    OUT  CPSS_PP_CONFIG_INIT_STC     *sysConfigParamsPtr
)
{
    osMemSet(libInitParamsPtr,0,sizeof(*libInitParamsPtr));
    osMemSet(sysConfigParamsPtr,0,sizeof(*sysConfigParamsPtr));

    libInitParamsPtr->initBridge = GT_TRUE;/*prvBridgeLibInit*/
    libInitParamsPtr->initIpv4   = GT_TRUE;/*prvIpLibInit*/
    libInitParamsPtr->initIpv6   = GT_TRUE;/*prvIpLibInit*/
    libInitParamsPtr->initPcl    = GT_TRUE;/*prvPclLibInit*/
    libInitParamsPtr->initTcam   = GT_TRUE;/*prvTcamLibInit*/
    libInitParamsPtr->initPort   = GT_TRUE;/*prvPortLibInit*/
    libInitParamsPtr->initPhy    = GT_TRUE;/*prvPhyLibInit*/
    libInitParamsPtr->initNetworkIf = GT_TRUE;/*prvNetIfLibInit*/
    libInitParamsPtr->initMirror = GT_TRUE;/*prvMirrorLibInit*/
    libInitParamsPtr->initTrunk  = GT_TRUE;/*prvTrunkLibInit*/
    libInitParamsPtr->initPha    = GT_TRUE;/*prvPhaLibInit*/

    sysConfigParamsPtr->numOfTrunks         = _4K;    /*used by prvTrunkLibInit*/
    sysConfigParamsPtr->lpmDbSupportIpv4    = GT_TRUE;/*used by prvIpLibInit*/
    sysConfigParamsPtr->lpmDbSupportIpv6    = GT_TRUE;/*used by prvIpLibInit*/
    /*NOTYE: info sysConfigParamsPtr->lpmRamMemoryBlocksCfg is set later : used by prvIpLibInit*/

    sysConfigParamsPtr->pha_packetOrderChangeEnable = GT_FALSE;/*used by prvPhaLibInit*/

    return GT_OK;
}



/**
* @internal appDemoFalconIpLpmRamDefaultConfigCalc function
* @endinternal
*
* @brief   This function calculate the default RAM LPM DB configuration for LPM management.
*
* @param[in] devNum                   - The Pp device number to get the parameters for.
* @param[in] sharedTableMode          - shared tables mode
* @param[in] maxNumOfPbrEntries       - number of PBR entries to deduct from the LPM memory calculations
*
* @param[out] ramDbCfgPtr              - (pointer to) ramDbCfg structure to hold the defaults calculated
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoFalconIpLpmRamDefaultConfigCalc
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT      sharedTableMode,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 value;
    GT_U32 i=0;

    CPSS_DXCH_CFG_DEV_INFO_STC   devInfo;
    rc = cpssDxChCfgDevInfoGet(devNum, &devInfo);
    if (rc!=GT_OK)
    {
        return rc;
    }

    if (appDemoLpmRamConfigInfoSet==GT_TRUE)
    {
        for (i=0;i<appDemoLpmRamConfigInfoNumOfElements;i++)
        {
            ramDbCfgPtr->lpmRamConfigInfo[i].devType = appDemoLpmRamConfigInfo[i].devType;
            ramDbCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg = appDemoLpmRamConfigInfo[i].sharedMemCnfg;
        }
        ramDbCfgPtr->lpmRamConfigInfoNumOfElements=appDemoLpmRamConfigInfoNumOfElements;
    }
    else
    {
        ramDbCfgPtr->lpmRamConfigInfo[0].devType=devInfo.genDevInfo.devType;
        ramDbCfgPtr->lpmRamConfigInfo[0].sharedMemCnfg=sharedTableMode;
        ramDbCfgPtr->lpmRamConfigInfoNumOfElements=1;
    }

    if(appDemoDbEntryGet("maxNumOfPbrEntries", &value) == GT_OK)
        ramDbCfgPtr->maxNumOfPbrEntries = value;
    else
        ramDbCfgPtr->maxNumOfPbrEntries = maxNumOfPbrEntries;

    if(appDemoDbEntryGet("sharedTableMode", &value) == GT_OK)
    {
        for (i=0; i<ramDbCfgPtr->lpmRamConfigInfoNumOfElements; i++)
        {
            ramDbCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg = value;
        }
    }

    ramDbCfgPtr->lpmMemMode = CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;/*the only mode for Falcon*/
    if(appDemoDbEntryGet("lpmRamMemoryBlocksCfg.lpmRamBlocksAllocationMethod", &value) == GT_OK)
        ramDbCfgPtr->blocksAllocationMethod  = (CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT)value;
    else
        ramDbCfgPtr->blocksAllocationMethod  = CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E;

    return GT_OK;
}

/**
* @internal falcon_initPpLogicalInit function
* @endinternal
*
*/
static GT_STATUS falcon_initPpLogicalInit
(
    IN GT_SW_DEV_NUM       devNum,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC    *lpmRamMemoryBlocksCfgPtr
)
{
    GT_STATUS   rc;
    CPSS_DXCH_PP_CONFIG_INIT_STC    ppLogInitParams;
    GT_U32 value = 0;

    osMemSet(&ppLogInitParams ,0, sizeof(ppLogInitParams));

    ppLogInitParams.maxNumOfPbrEntries = _8K;/* same as SIP5 devices */
    ppLogInitParams.lpmMemoryMode = CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
    ppLogInitParams.sharedTableMode = CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E;

    if(appDemoDbEntryGet("maxNumOfPbrEntries", &value) == GT_OK)
    {
        ppLogInitParams.maxNumOfPbrEntries = value;
    }

    if(appDemoDbEntryGet("sharedTableMode", &value) == GT_OK)
    {
        ppLogInitParams.sharedTableMode = value;
    }

    rc = cpssDxChCfgPpLogicalInit(CAST_SW_DEVNUM(devNum), &ppLogInitParams);
    if(rc != GT_OK)
        return rc;

    /* function must be called after cpssDxChCfgPpLogicalInit that update the value of :
        PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.lpmRam */
    /* calculate info needed by 'prvDxCh2Ch3IpLibInit' (init of LPM manager) */
    rc = appDemoFalconIpLpmRamDefaultConfigCalc(CAST_SW_DEVNUM(devNum),
                                                ppLogInitParams.sharedTableMode,
                                                ppLogInitParams.maxNumOfPbrEntries,
                                                lpmRamMemoryBlocksCfgPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal falcon_appDemoDbUpdate function
* @endinternal
*
* @brief   Update the AppDemo DB (appDemoPpConfigList[devIndex]) about the device
*
* @param[in] devIndex                 - the device index
* @param[in] devNum                   - the SW devNum
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS falcon_appDemoDbUpdate
(
    IN GT_U32             devIndex,
    IN GT_SW_DEV_NUM      devNum,
    IN CPSS_PP_DEVICE_TYPE     devType
)
{
    appDemoPpConfigList[devIndex].valid = GT_TRUE;
    appDemoPpConfigList[devIndex].devNum = CAST_SW_DEVNUM(devNum);
    appDemoPpConfigList[devIndex].deviceId = devType;
    appDemoPpConfigList[devIndex].devFamily = CPSS_PP_FAMILY_DXCH_FALCON_E;
    appDemoPpConfigList[devIndex].apiSupportedBmp = APP_DEMO_DXCH_FUNCTIONS_SUPPORT_CNS;

    return GT_OK;
}

/**
* @internal falcon_initStaticSerdesMuxing function
* @endinternal
*
* @brief   init static serdes muxing
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS falcon_initStaticSerdesMuxing
(
    IN GT_U8    devNum,
    IN GT_U32   ravensBmp,
    IN GT_BOOL  supportPhyGearbox
)
{
    GT_U32                       portNum, portMacNum, ravenIndex;
    CPSS_DXCH_PORT_MAP_STC       portMap;
    GT_STATUS                    rc;
    GT_U32                       currBoardType;
#ifndef ASIC_SIMULATION
    GT_U32                       *ravenLanesToPhyLanesMap, laneIndex, phyIndex=SKIP_PHY;
    CPSS_PORT_MAC_TO_SERDES_STC  gearboxModeMacToSerdesMap;
#endif
    rc = appDemoBoardTypeGet(&currBoardType);
    if (rc != GT_OK)
    {
        return rc;
    }

    if ((currBoardType == APP_DEMO_FALCON_BOARD_12_8_TH_CNS) && (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles != 4))
    {
        return GT_OK;
    }

    portNum = (currBoardType == APP_DEMO_FALCON_BOARD_12_8_RD_CNS)? 128 : 0;
    for(; portNum < appDemoPpConfigList[devNum].maxPortNumber ; portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);
        if (prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            continue;
        }

        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, /*OUT*/&portMap);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPhysicalPortMapGet", rc);
        if(rc != GT_OK)
        {
            continue;
        }

        portMacNum = portMap.interfaceNum;
        if(falcon_MacToSerdesMap_arrSize <= (portMacNum/8))
        {
            break;
        }
        if((portMacNum % 8) != 0)
        {
            continue;
        }
        ravenIndex = portMacNum/16;
        if( (ravensBmp & (1<<ravenIndex)) == 0)
        {
            continue;
        }
#ifndef ASIC_SIMULATION
        if(supportPhyGearbox && (NULL != falcon_RavenLaneToPhyLaneMap) && (currBoardType == APP_DEMO_FALCON_BOARD_12_8_TH_CNS))
        {   /* Gearbox mode on TH board should use different SD muxing settings
               to align the Falcon's lanes with the Phy's lanes*/
            GET_PHY_INDEX_FROM_GROUP_ID_MAC(ravenIndex,phyIndex);
            if(phyIndex != SKIP_PHY)
            {
                ravenLanesToPhyLanesMap = (portMacNum % 16) ? &falcon_RavenLaneToPhyLaneMap[phyIndex].lanes[NUM_OF_LANES_PER_PHY / 2]/*High 8 ports*/ :
                    &falcon_RavenLaneToPhyLaneMap[phyIndex].lanes[0]/*Low 8 ports */;

                for (laneIndex = 0; laneIndex < NUM_OF_LANES_PER_PHY/2; laneIndex++)
                {/* Build SD muxing map for the current port */
                    gearboxModeMacToSerdesMap.serdesLanes[ravenLanesToPhyLanesMap[laneIndex]%8] = laneIndex;
                }
                rc = cpssDxChPortLaneMacToSerdesMuxSet(devNum,portNum,&gearboxModeMacToSerdesMap);
                if (rc != GT_OK)
                {
                    return rc;
                }

                continue;
            }
        }
#else
        GT_UNUSED_PARAM(supportPhyGearbox);
#endif
        rc = cpssDxChPortLaneMacToSerdesMuxSet(devNum,portNum,&falcon_MacToSerdesMap[portMacNum/8]);
        if (rc != GT_OK)
        {
            return rc;
        }

    }

    return GT_OK;
}

/**
* @internal falcon_initStaticSerdesMuxingValidation function
* @endinternal
*
* @brief   init static serdes muxing
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS falcon_initStaticSerdesMuxingValidation
(
    IN GT_U8 devNum
)
{
    GT_U32                      portNum, portMacNum;
    CPSS_DXCH_PORT_MAP_STC      portMap;
    CPSS_PORT_MAC_TO_SERDES_STC   macToSerdesMuxStc;
    GT_STATUS                   rc;
    GT_U32                      currBoardType;

    rc = appDemoBoardTypeGet(&currBoardType);
    if (rc != GT_OK)
    {
        return rc;
    }

    for(portNum = 0; portNum < appDemoPpConfigList[devNum].maxPortNumber ; portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);
        if (prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            continue;
        }

        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, /*OUT*/&portMap);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPhysicalPortMapGet", rc);
        if(rc != GT_OK)
        {
            continue;
        }

        portMacNum = portMap.interfaceNum;
        if(falcon_MacToSerdesMap_arrSize <= (portMacNum/8))
        {
            break;
        }
        if((portMacNum % 8) != 0)
        {
            continue;
        }

        rc = cpssDxChPortLaneMacToSerdesMuxGet(devNum,portNum,&macToSerdesMuxStc);
        if (rc != GT_OK)
        {
            osPrintf("falcon_initStaticSerdesMuxingValidation: FIRST GET portNum = %d, rc = %d\n",portNum,rc);
        }

        if(cpssOsMemCmp(&macToSerdesMuxStc, &(falcon_MacToSerdesMap[portMacNum/8]), sizeof(CPSS_PORT_MAC_TO_SERDES_STC)) != 0)
        {
            osPrintf("falcon_initStaticSerdesMuxingValidation: FAILURE for portNum = %d, trying to configure again ...\n",portNum);
            rc = cpssDxChPortLaneMacToSerdesMuxSet(devNum,portNum,&falcon_MacToSerdesMap[portNum / 8]);
            if (rc != GT_OK)
            {
                osPrintf("falcon_initStaticSerdesMuxingValidation: MUX SET ERROR for portNum = %d, rc = %d\n",portNum,rc);
            }

            rc = cpssDxChPortLaneMacToSerdesMuxGet(devNum,portNum,&macToSerdesMuxStc);
            if (rc != GT_OK)
            {
                osPrintf("falcon_initStaticSerdesMuxingValidation: SECOND GET portNum = %d, rc = %d\n",portNum,rc);
            }
            if(cpssOsMemCmp(&macToSerdesMuxStc, &(falcon_MacToSerdesMap[portMacNum/8]), sizeof(CPSS_PORT_MAC_TO_SERDES_STC)) != 0)
            {
                osPrintf("falcon_initStaticSerdesMuxingValidation: SECOND MUXSET ERROR for portNum = %d !!!!!!!\n\n",portNum);
            }
        }
    }
    return GT_OK;
}

/**
* @internal falcon_configPolaritySwap function
* @endinternal
*
* @brief   Config polarity swap
*
* @param[in] devNum                - The CPSS devNum.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS falcon_configPolaritySwap
(
    IN  GT_U8    devNum
)
{
    GT_STATUS   rc;
    GT_U32      i;
    GT_U32      polarityArraySize;
    GT_U32      currBoardType;

    rc = appDemoBoardTypeGet(&currBoardType);
    if (rc != GT_OK)
    {
        return rc;
    }

    if ((currBoardType == APP_DEMO_FALCON_BOARD_12_8_TH_CNS) && (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles != 4))
    {
        return GT_OK;
    }

    if(cpssDeviceRunCheck_onEmulator())
    {
        /* must not run on emulator because it is not connected like the board */
        /* it hold no 'swap' of SERDESes                                       */

        /* see also call to function hwsFalconInitStaticSerdesMuxing(...) */

        return GT_OK;
    }

    /* NOTE: this array is supports LANEs of the 'CPU ports' !!! */
    polarityArraySize = (64+4) * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
#ifdef GM_USED
    polarityArraySize = 8;/* no real meaning , as any way only 8 MAC ports supported */
#endif /*GM_USED*/

    for (i = 0; (i < falcon_PolarityArray_arrSize) && (i < polarityArraySize); i++)
    {
        rc = cpssDxChPortSerdesLanePolaritySet(devNum, 0,
                                               falcon_PolarityArray[i].laneNum,
                                               falcon_PolarityArray[i].invertTx,
                                               falcon_PolarityArray[i].invertRx);

        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortSerdesLanePolaritySet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal falcon_appDemoInitSequence function
* @endinternal
*
* @brief   This is the 'simple' board initialization function for Falcon device.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
static GT_STATUS falcon_appDemoInitSequence
(
    IN  GT_U8             boardRevId,
    IN GT_U32             devIndex,
    IN GT_SW_DEV_NUM      devNum,
    IN CPSS_HW_INFO_STC   *hwInfoPtr
)
{
    GT_STATUS   rc;
    CPSS_PP_DEVICE_TYPE     devType;
    APP_DEMO_LIB_INIT_PARAMS    libInitParams;
    CPSS_PP_CONFIG_INIT_STC     sysConfigParams;
    GT_BOOL supportSystemReset = GT_TRUE;

    /* Set debug devType if needed */
    rc = appDemoDebugDeviceIdGet(CAST_SW_DEVNUM(devNum), &devType);
    if (GT_OK == rc)
    {
        rc = prvCpssDrvDebugDeviceIdSet(CAST_SW_DEVNUM(devNum), devType);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvCpssDrvDebugDeviceIdSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = falcon_initPhase1AndPhase2(devNum,boardRevId,hwInfoPtr,devIndex,&devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_initPhase1AndPhase2", rc);
    if (GT_OK != rc)
        return rc;

    if(falcon_force_stop_init_after_d2d_init_get())
    {
        return GT_OK;
    }

    if (appDemoInitRegDefaults != GT_FALSE)
    {
        return GT_OK;
    }

    if(!cpssDeviceRunCheck_onEmulator())
    {
        /* must not run on emulator because it is not connected like the board */
        /* it hold no 'swap' of SERDESes and no PHYs*/
#if !defined(ASIC_SIMULATION) && defined(INCLUDE_MPD)

        rc = falcon_phy_init(devNum,GT_TRUE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_initPhys", rc);
#endif

#ifndef GM_USED
        rc = falcon_initStaticSerdesMuxing(CAST_SW_DEVNUM(devNum),0xffff,phyGearboxMode);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_initStaticSerdesMuxing", rc);
#endif
        if (rc != GT_OK)
            return rc;
    }

    rc = falcon_configPolaritySwap(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_configPolaritySwap", rc);
    if (rc != GT_OK)
        return rc;

    rc = falcon_PortModeConfiguration(devNum,devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_PortModeConfiguration", rc);
    if (rc != GT_OK)
        return rc;

    rc = falcon_getNeededLibs(&libInitParams,&sysConfigParams);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_getNeededLibs", rc);
    if(rc != GT_OK)
        return rc;

    rc = falcon_initPpLogicalInit(devNum,&sysConfigParams.lpmRamMemoryBlocksCfg);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_initPpLogicalInit", rc);
    if(rc != GT_OK)
        return rc;

    rc = appDemoDxLibrariesInit(CAST_SW_DEVNUM(devNum),&libInitParams,&sysConfigParams);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxLibrariesInit", rc);
    if(rc != GT_OK)
        return rc;

    rc = appDemoDxPpGeneralInit(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxPpGeneralInit", rc);
    if(rc != GT_OK)
        return rc;

    rc = appDemoBc2PIPEngineInit(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoBc2PIPEngineInit", rc);
    if(rc != GT_OK)
        return rc;

    rc = appDemoB2PtpConfig(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoB2PtpConfig", rc);
    if(rc != GT_OK)
        return rc;

    #ifndef ASIC_SIMULATION
    if (boardRevId == 1 || boardRevId == 3)
    {
        falconDB_ThermalTaskInit(devNum);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("falconDB_ThermalTaskInit", rc);
    }
    #endif

    rc = appDemoDxTrafficEnable(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxTrafficEnable", rc);
    if(rc != GT_OK)
        return rc;

    #ifdef GM_USED
    /* the GM not supports the 'soft reset' so we can not support system with it */
    supportSystemReset = GT_FALSE;
    #endif /*GM_USED*/
    if(cpssDeviceRunCheck_onEmulator())
    {
        CPSS_TBD_BOOKMARK_FALCON_EMULATOR
        /* until we make it happen ... do not try it on emulator ! */
        supportSystemReset = GT_FALSE;
    }
#ifndef ASIC_SIMULATION

    /* not support gtShutdownAndCoreRestart */
    appDemoPpConfigList[devIndex].devSupportSystemReset_forbid_gtShutdownAndCoreRestart = GT_TRUE;
    /*cpssOsPrintf("NOTE: the Falcon HW crash the gtShutdownAndCoreRestart(...) \n");*/
#endif /* ASIC_SIMULATION */

    /* not support 'system reset' */
    rc = cpssDevSupportSystemResetSet(CAST_SW_DEVNUM(devNum), supportSystemReset);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDevSupportSystemResetSet", rc);
    if (GT_OK != rc)
        return rc;

    return GT_OK;
}

/**
* @internal falcon_EventHandlerInit function
* @endinternal
*
*/
static GT_STATUS falcon_EventHandlerInit
(
        IN GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32 value;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */
    GT_U8                   devIndex;

    /* appDemoEventHandlerPreInit:
        needed to be called before:
        boardCfgFuncs.boardAfterInitConfig(...)
        and before:
        appDemoEventRequestDrvnModeInit()
        */
    rc = appDemoEventHandlerPreInit();
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEventHandlerPreInit", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (GT_OK == appDemoDbEntryGet("initSystemWithoutInterrupts", &value) && value)
    {
        /* we not want interrupts handling */
        PRINT_SKIP_DUE_TO_DB_FLAG("event handling","initSystemWithoutInterrupts");
    }
    else
    {
        rc = cpssSystemRecoveryStateGet(&system_recovery);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
        {
            PRINT_SKIP_DUE_TO_DB_FLAG("appDemoEventRequestDrvnModeInit ","system recovery HA process");
            return GT_OK;
        }

        if(isUnderAddDeviceCatchup == GT_TRUE)
        {
            GT_U32  lastIndexInLoop = appDemoPpConfigDevAmount;
            /* the appDemoDeviceEventRequestDrvnModeInit , assumes that the
                appDemoPpConfigDevAmount , should be 0 to start the tasks of the event.

                but the logic of START_LOOP_ALL_DEVICES() requires the
                appDemoPpConfigDevAmount to be >= 1 , to 'see' the current device

                therefore the gtDbFalconBoardReg_deviceSimpleInit is doing
                appDemoPpConfigDevAmount++ , before entering the logic.
            */
            appDemoPpConfigDevAmount--;

            while ((GT_U32)(1 << lastIndexInLoop) <= appDemoSysConfig.appDemoActiveDeviceBmp)
            {
                lastIndexInLoop ++;
            }

            for (devIndex = SYSTEM_DEV_NUM_MAC(0);
                 (devIndex < SYSTEM_DEV_NUM_MAC(lastIndexInLoop)); devIndex++)
            {
                SYSTEM_SKIP_NON_ACTIVE_DEV(devIndex)

                appDemoDeviceEventRequestDrvnModeInit(devIndex);
            }

            appDemoPpConfigDevAmount++;
        }
        else
        {
            rc = appDemoEventRequestDrvnModeInit();
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEventRequestDrvnModeInit", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal falcon_localUtfInit function
* @endinternal
*
*/
static GT_STATUS falcon_localUtfInit
(
    IN GT_SW_DEV_NUM      devNum
)
{
    GT_STATUS rc = GT_OK;

#ifdef INCLUDE_UTF

    if(isUnderAddDeviceCatchup == GT_FALSE)
    {
        /* Initialize unit tests for CPSS */
        rc = utfPreInitPhase();
        if (rc != GT_OK)
        {
            utfPostInitPhase(rc);
            return rc;
        }
    }

    rc = utfInit(CAST_SW_DEVNUM(devNum));
    if (rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("utfInit", rc);
        utfPostInitPhase(rc);
        return rc;
    }

    if(isUnderAddDeviceCatchup == GT_FALSE)
    {
        utfPostInitPhase(rc);
    }

#endif /* INCLUDE_UTF */

    GT_UNUSED_PARAM(devNum);
    if(isUnderAddDeviceCatchup == GT_FALSE)
    {
        appDemoFalconMainUtForbidenTests();
    }

    return rc;
}

/**
* @internal falcon_EgfEftPortFilter function
* @endinternal
*
*/
static GT_STATUS falcon_EgfEftPortFilter
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           devIdx
)
{
    GT_STATUS   rc;
    GT_U32  portNum;
    GT_BOOL isLinkUp;

    if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles < 2)
    {
        return GT_OK;
    }

/*       as the interrupts from the device are not implemented , check manually
 *       if port is UP and set it as 'link UP' in the EGF-EFT unit */

    for(portNum = 64 ; portNum < appDemoPpConfigList[devIdx].maxPortNumber ; portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(CAST_SW_DEVNUM(devNum),portNum);
        if (!appDemoDxChIsLocalPort_or_RemotePort_with_MAC_PHY_OBJ(CAST_SW_DEVNUM(devNum),portNum))
        {
            continue;
        }
        rc = cpssDxChPortLinkStatusGet(CAST_SW_DEVNUM(devNum),
            portNum,&isLinkUp);
        if(rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortLinkStatusGet", rc);
            return rc;
        }

        if(isLinkUp == GT_TRUE)
        {
            rc = cpssDxChBrgEgrFltPortLinkEnableSet(CAST_SW_DEVNUM(devNum),
                portNum,
                    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E  /*don't filter*/);
            if(rc != GT_OK)
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgEgrFltPortLinkEnableSet", rc);
                return rc;
            }
        }
    }


    return GT_OK;
}

#ifndef GM_USED
/**
* @internal led_cpu_port_mac_check function
* @endinternal
*
* @brief   Checks if current MAC port is CPU port
*
* @param[in] devNum                - The CPSS devNum.
*
* @retval GT_TRUE                  - the mac port is CPU port
* @retval GT_FAIL                  - otherwise.
*
*/
static GT_BOOL led_cpu_port_mac_check(GT_U32 portMacNum, GT_U32 *cpuPortsArray)
{
    GT_U32 ii;
    for (ii = 0; ii < CPSS_CHIPLETS_MAX_NUM_CNS; ii++)
    {
        if (cpuPortsArray[ii] == portMacNum)
        {
            return GT_TRUE;
        }
    }
    return GT_FALSE;
}

/**
* @internal led_port_mac_raven_with_cpu
* @endinternal
*
* @brief   Checks if current MAC port and CPU port together in the same Raven
*
* @param[in] portMacNum            - current MAC to check.
* @param[in] cpuPortsArray         - CPU ports array
*
* @retval GT_TRUE                  - the mac port and CPU ports are in the same Raven
* @retval GT_FAIL                  - the mac port and CPU ports are NOT in the same Raven.
*
*/
static GT_BOOL led_port_mac_raven_with_cpu(GT_U32 portMacNum, GT_U32 *cpuPortsArray)
{
    GT_U32 raven = portMacNum / CPSS_CHIPLETS_MAX_NUM_CNS;
    /* current mac not in Raven with connected CPU port */
    if (cpuPortsArray[raven] == 0xFFFFFFFF)
    {
        return GT_FALSE;
    }
    return GT_TRUE;
}

/**
* @internal first_port_in_raven
* @endinternal
*
* @brief   Checks if current MAC port is the first 400G port in Raven
*
* @param[in] portMacNum            - current MAC to check.
*
* @retval GT_TRUE                  - the mac port is the first 400G port in Raven.
* @retval GT_FAIL                  - the mac port is NOT the first 400G port in Raven.
*
*/
static GT_BOOL first_port_in_raven(GT_U32 portMacNum)
{
    GT_STATUS rc = (((portMacNum / 8) % 2) == 0) ? GT_TRUE : GT_FALSE;

    return rc;
}

/**
* @internal led_port_position_get_mac
* @endinternal
*
* @brief   Gets LED ports position for the current MAC port
*
* @param[in] devNum                - device number
* @param[in] portMacNum            - current MAC to check
* @param[in] cpuPortsArray         - CPU ports array
*
* @retval GT_U32                   - the LED ports position for the current MAC port.
*
*/
static GT_U32 led_port_position_get_mac(GT_U8 devNum, GT_U32 portMacNum, GT_U32 *cpuPortsArray)
{
    GT_U32 firstLedPosition, secondLedPosition;


    switch(PRV_CPSS_PP_MAC(devNum)->devType)
    {
        case CPSS_FALCON_2T_4T_PACKAGE_DEVICES_CASES_MAC:
            return portMacNum % 16;
        default:
            if (portMacNum % 8 == 0)
            {
                if (led_port_mac_raven_with_cpu(portMacNum, cpuPortsArray) == GT_FALSE)
                {
                    if (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 4)
                    {
                        firstLedPosition =  0;
                        secondLedPosition = 1;
                    }
                    else
                    {
                        firstLedPosition =  1;
                        secondLedPosition = 0;
                    }
                }
                else
                {
                    if (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 4)
                    {
                        firstLedPosition =  0;
                        secondLedPosition = 1;
                    }
                    else
                    {
                        firstLedPosition =  2;
                        secondLedPosition = 1;
                    }
                }

                return first_port_in_raven(portMacNum) ? firstLedPosition : secondLedPosition;
            }
            else
            {
                return APPDEMO_BAD_VALUE;
            }
        }
}

/**
* @internal led_cpu_port_position_get_mac
* @endinternal
*
* @brief   Gets CPU LED ports position for the current CPU port
*
* @param[in] devNum                - device number
*
* @retval GT_U32                   - the LED ports position for the current CPU port.
*
*/
static GT_U32 led_cpu_port_position_get_mac(GT_U8 devNum)
{
    GT_U32 cpuLedPosition;

    switch(PRV_CPSS_PP_MAC(devNum)->devType)
    {
        case CPSS_FALCON_2T_4T_PACKAGE_DEVICES_CASES_MAC:
            cpuLedPosition = 16;
            break;
        default:
            if (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 4)
            {
                cpuLedPosition = 2;
            }
            else
            {
                cpuLedPosition = 0;
            }
    }

    return cpuLedPosition;
}

/**
* @internal falconLedInit function
* @endinternal
*
* @brief   LED configurations
*
* @param[in] devNum                - The CPSS devNum.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
static GT_STATUS falcon_LedInit
(
    GT_U8    devNum
)
{
    GT_STATUS                       rc;
    GT_U32                          classNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_LED_CONF_STC               ledConfig;
    CPSS_LED_CLASS_MANIPULATION_STC ledClassManip;
    GT_U32                          position;
    GT_U32                          tileIndex;
    GT_U32                          ravenIndex;
    GT_U32                          ledUnit;
    CPSS_DXCH_PORT_MAP_STC          portMap;
    GT_U32                          portMacNum;
    CPSS_FALCON_LED_STREAM_INDICATIONS_STC * ledStreamIndication;
    GT_U32                          cpuPorts[CPSS_CHIPLETS_MAX_NUM_CNS] = {0};
    GT_U32                          cpuPortMacFirst;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery; /* holds system recovery information */

    #define FIRST_PORT_IN_RAVEN(portMacNum) \
            first_port_in_raven(portMacNum)

    /* Macro returns LED position per MTI400 ports
      (two ports per Raven, one LED per port) */
    #define LED_PORT_POSITION_GET_MAC(portMacNum) \
            led_port_position_get_mac(devNum, portMacNum, cpuPorts);

    /* Macro returns true if mac port is CPU and false otherwise */
    #define LED_CPU_PORT_MAC(portMacNum)    \
            led_cpu_port_mac_check(portMacNum, cpuPorts)

    /* Macro returns LED position per CPU ports */
    #define LED_CPU_PORT_POSITION_GET_MAC(portMacNum) \
            led_cpu_port_position_get_mac(devNum)

    cpssOsMemSet(&ledConfig, 0, sizeof(CPSS_LED_CONF_STC));
    cpssOsMemSet(&cpuPorts,  0xFF, sizeof(cpuPorts));

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

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    switch(PRV_CPSS_PP_MAC(devNum)->devType)
    {
        case CPSS_98CX8540_CNS:
        case CPSS_98CX8540_H_CNS:
        case CPSS_98CX8522_CNS:
            ledStreamIndication = falcon_6_4_led_indications;
            cpuPortMacFirst     = 128;
            break;
        case CPSS_98EX5610_CNS:
            ledStreamIndication = falcon_6_4_reduced_led_indications;
            cpuPortMacFirst     = 128;
            break;
        case CPSS_FALCON_2T_4T_PACKAGE_DEVICES_CASES_MAC:
            ledStreamIndication = falcon_2T_4T_led_indications;
            cpuPortMacFirst     = 128;
            break;
        default:
            ledStreamIndication = falcon_12_8_led_indications;
            cpuPortMacFirst     = 256;
    }

    for (tileIndex = 0; tileIndex < PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles; tileIndex++)
    {
        for(ravenIndex = 0; ravenIndex < FALCON_RAVENS_PER_TILE; ravenIndex++)
        {
            ledUnit = PRV_DXCH_LED_NUM_MAC(tileIndex, ravenIndex);

            /* Start of LED stream location - Class 2 */
            ledConfig.sip6LedConfig.ledStart[ledUnit] = ledStreamIndication[ledUnit].ledStart;    /* LedControl.LedStart[5:12] */
            /* End of LED stream location: LED ports 0, 1 or 0, 1 and 2*/
            ledConfig.sip6LedConfig.ledEnd[ledUnit] = ledStreamIndication[ledUnit].ledEnd;        /* LedControl.LedEnd[5:12] */

            if (ledStreamIndication[ledUnit].cpuPort == GT_TRUE)
            {
                cpuPorts[ledUnit] = cpuPortMacFirst + ledUnit;
            }
            /* The LED unit is included into LED chain */
            ledConfig.sip6LedConfig.ledChainBypass[ledUnit] = GT_FALSE;
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

    for (classNum = 0 ; classNum < 6; classNum++)
    {
        if(classNum == 2)
        {
            ledClassManip.disableOnLinkDown       = GT_TRUE;                                  /* classGlobalControl.disable on link down [ 0- 5] = pos = classNum = 0*/
            ledClassManip.blinkEnable             = GT_TRUE;                                  /* blinkGlobalControl.blinkEn     [18-25] = pos = classNum val = 1 */
        }                                                                                     /* blinkGlobalControl.Blink Select[10-17] = pos = classNum val = 0 */
        else
        {
            ledClassManip.disableOnLinkDown       = GT_FALSE;
            ledClassManip.blinkEnable             = GT_FALSE;
        }
        rc = cpssDxChLedStreamClassManipulationSet(devNum, 0,
                                                   CPSS_DXCH_LED_PORT_TYPE_XG_E /* don't care , not applicable for Falcon */
                                                   ,classNum ,&ledClassManip);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamClassManipulationSet", rc);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    /*--------------------------------------------------------------------------------------------------------*
     * LED. classesAndGroupConfig uses default setting , don't configure   cpssDxChLedStreamGroupConfigSet()  *
     *--------------------------------------------------------------------------------------------------------*
     * now configure port dependent LED configurations                                                        *
     *       - port led position (stream is hard wired                                                        *
     *           (One LED per 400G port, two 400G per Raven)                                                  *
     *           The port LED positions for full chip is as follows:                                          *
     *           P31,                   Raven 15    Position 1                                                *
     *           P30,                   Raven 15    Position 0                                                *
     *           P29,                   Raven 14    Position 1                                                *
     *           P28,                   Raven 14    Position 0                                                *
     *           ...,                                                                                         *
     *           HOST1,                 Raven 2     Position 2                                                *
     *           P5,                    Raven 2     Position 1                                                *
     *           P4,                    Raven 2     Position 0                                                *
     *           HOST0,                 Raven 1     Position 2                                                *
     *           P3,                    Raven 1     Position 1                                                *
     *           P2,                    Raven 1     Position 0                                                *
     *           P1,                    Raven 0     Position 1                                                *
     *           P0                     Raven 0     Position 0                                                *
     *                                                                                                        *
     *       - invert polarity                                                                                *
     *--------------------------------------------------------------------------------------------------------*/
    for(portNum = 0; portNum < (appDemoPpConfigList[devNum].maxPortNumber); portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);

        if (prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            continue;
        }

        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, /*OUT*/&portMap);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPhysicalPortMapGet", rc);
        if(rc != GT_OK)
        {
            continue;
        }

        portMacNum = portMap.interfaceNum;
        position = (LED_CPU_PORT_MAC(portMacNum)) ? LED_CPU_PORT_POSITION_GET_MAC(portMacNum) :  LED_PORT_POSITION_GET_MAC(portMacNum);

        if (position != APPDEMO_BAD_VALUE)
        {
            rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamPortPositionSet", rc);
            if(rc != GT_OK)
            {
                return rc;
            }

            /*skip if in recovery mode HA */
            if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
            {
                rc = cpssDxChLedStreamPortClassPolarityInvertEnableSet(devNum, portNum, /*classNum*/2, /*invertEnable*/1);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamPortClassPolarityInvertEnableSet", rc);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}
#endif

extern GT_STATUS prvCpssHwInitSip5IsInitTcamDefaultsDone(IN GT_U8   devNum, GT_BOOL tcamDaemonEnable);
/**
* @internal falcon_waitForTcamInitDone function
* @endinternal
*
*/
static GT_STATUS falcon_waitForTcamInitDone
(
    IN GT_SW_DEV_NUM    devNum
)
{
    if(!cpssDeviceRunCheck_onEmulator())
    {
        return GT_OK;
    }

    /* on the emulator , in order not to wait for it to finish all 36K entries ,
       we allow it to run in parallel , and only now we check if the default values
       finished to load */
    return prvCpssHwInitSip5IsInitTcamDefaultsDone(CAST_SW_DEVNUM(devNum), GT_TRUE);
}

/**
* @internal gtDbFalconBoardReg_SimpleInit function
* @endinternal
*
* @brief   This is the 'simple' board initialization function for Falcon device.
*
* @param[in] boardRevId               - Board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
static GT_STATUS gtDbFalconBoardReg_SimpleInit
(
    IN  GT_U8  boardRevId
)
{
    GT_STATUS   rc;
    GT_U32      start_sec  = 0;
    GT_U32      start_nsec = 0;
    GT_U32      end_sec  = 0;
    GT_U32      end_nsec = 0;
    GT_U32      diff_sec;
    GT_U32      diff_nsec;
    CPSS_HW_INFO_STC   hwInfo[APP_DEMO_CPSS_MAX_NUM_PORT_GROUPS_CNS];
    GT_SW_DEV_NUM           devNum;
    GT_U32      devIndex;/* device index in the array of appDemoPpConfigList[devIndex] */
    GT_PCI_INFO pciInfo;
    GT_U32      tmpData;
    GT_U32      boardType = APP_DEMO_FALCON_BOARD_12_8_B2B_CNS;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery;
    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of init */
    CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC   globalParamsStc;

    rc = cpssOsTimeRT(&secondsStart,&nanoSecondsStart);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssOsTimeRT", rc);
    if(rc != GT_OK)
        return rc;

#if (defined ASIC_SIMULATION) &&  (defined RTOS_ON_SIM)
    /*simulation initialization*/
    appDemoRtosOnSimulationInit();

#endif /*(defined ASIC_SIMULATION) &&  (defined RTOS_ON_SIM)*/

#ifdef ASIC_SIMULATION
    /* allow setting clock after the SOFT-RESET during the 'system reset'
       and 'system init' */
    if(simCoreClockOverwrittenGet())
    {
        simCoreClockOverwrite(simCoreClockOverwrittenGet());
    }
#endif

    /* Enable printing inside interrupt routine - supplied by extrernal drive */
    extDrvUartInit();

    /* Call to fatal_error initialization, use default fatal error call_back - supplied by mainOs */
    rc = osFatalErrorInit((FATAL_FUNC_PTR)NULL);
    if (rc != GT_OK)
        return rc;

    /* On FalconZ board no real PCI exits */

        /* this function finds all Prestera devices on PCI bus */
    rc = falcon_getBoardInfoSimple(
            &hwInfo[0],
            &pciInfo);
    if( (boardRevId == 11) || (boardRevId == 12))  /* Falcon Z boards */
    {
        hwInfo[0].driver = NULL;
    } else {
        if (rc != GT_OK)
            return rc;
    }

    if(cpssDeviceRunCheck_onEmulator())
    {
        /* the 'NO KN' is doing polling in this interrupt global cause register */
        /* but this polling should not be seen during 'trace'                   */
        appDemoTrace_skipTrace_onAddress(PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS + 0x30 /*0x1d000030*//*address*/,0/*index*/);
    }

    appDemoPpConfigDevAmount = 1;

    devIndex = SYSTEM_DEV_NUM_MAC(0);
    devNum =   devIndex;
    /* TODO: userForceBoardType should be deleted after creating an automatic board recognition in appDemoDxChBoardTypeGet.*/
    switch(boardRevId)
    {
        case 1:
            /* Add option to force board type while using cpssInitSystem 35,1 */
            if(appDemoDbEntryGet("boardType", &tmpData) == GT_OK)
            {
                switch(tmpData)
                {
                    case 0:
                        boardType = APP_DEMO_FALCON_BOARD_12_8_TH_CNS;
                        falcon_boardTypePrint("12.8T TH" /*boardName*/, "Falcon" /*devName*/);
                        break;
                    case 1:
                        boardType = APP_DEMO_FALCON_BOARD_12_8_B2B_CNS;
                        falcon_boardTypePrint("12.8T Belly-to-Belly" /*boardName*/, "Falcon" /*devName*/);
                        break;
                    case 2:
                        boardType = APP_DEMO_FALCON_BOARD_12_8_RD_CNS;
                        falcon_boardTypePrint("12.8T RD" /*boardName*/, "Falcon" /*devName*/);
                        break;
                    case 3:
                        boardType = APP_DEMO_FALCON_BOARD_6_4_TH_CNS;
                        falcon_boardTypePrint("6.4T TH" /*boardName*/, "Falcon" /*devName*/);
                        break;
                    default:
                        break;
                }
            }
            break;
        case 2:
        case 12: /* Falcon Z2 */
            boardType = APP_DEMO_FALCON_BOARD_12_8_B2B_CNS;
            falcon_boardTypePrint("12.8T Belly-to-Belly" /*boardName*/, "Falcon" /*devName*/);
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("modeMaxMac", 1);
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("numOfTiles", 4);
            break;
        case 3:
        case 13: /* Falcon Z2 */
            boardType = APP_DEMO_FALCON_BOARD_12_8_TH_CNS;
            falcon_boardTypePrint("12.8T TH" /*boardName*/, "Falcon" /*devName*/);
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("modeMaxMac", 1);
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("numOfTiles", 4);
            break;
        case 4:
            boardType = APP_DEMO_FALCON_BOARD_12_8_RD_CNS;
            falcon_boardTypePrint("12.8T RD" /*boardName*/, "Falcon" /*devName*/);
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("portMgr", 1);        /* Force Port Manager*/
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("phyGearboxMode", 1); /* Force only PHY's gearbox modes */
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("modeMaxMac", 1);
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("numOfTiles", 4);
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("mode100G", 1);
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("useCpuPort0", 0);
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("useCpuPort1", 0);
            break;
        case 5:
            boardType = APP_DEMO_FALCON_BOARD_6_4_TH_CNS;
            falcon_boardTypePrint("6.4T TH" /*boardName*/, "Falcon" /*devName*/);
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("modeMaxMac", 1);
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("numOfTiles", 2);
            break;
        case 6:
            boardType = APP_DEMO_FALCON_BOARD_2_TH_CNS;
            falcon_boardTypePrint("2T TH" /*boardName*/, "Falcon" /*devName*/);
            break;
        case 7:
            boardType = APP_DEMO_FALCON_BOARD_2T_4T_CNS;
            falcon_boardTypePrint("2T/4T TH" /*boardName*/, "Falcon" /*devName*/);
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("numOfTiles", 2);
            break;
        default:
            break;
    }
    userForceBoardType(boardType);
    rc = appDemoDxChBoardTypeGet(CAST_SW_DEVNUM(devNum), boardRevId, &bc2BoardType);
    if (rc != GT_OK)
    {
        return rc;
    }
    rc = falcon_boardTypeInitDatabases(boardType);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* take time from the 'phase1 init' stage (not including the 'PCI scan' operations) */
    cpssOsTimeRT(&start_sec, &start_nsec);

    if(appDemoDbEntryGet("modeMaxMac", &tmpData) == GT_OK)
    {
        modeMaxMac = 1;
    }
    else
    {
        modeMaxMac = 0;
    }

    if(appDemoDbEntryGet("mode100G", &tmpData) == GT_OK)
    {
        mode100G = 1;
    }
    else
    {
        mode100G = 0;
    }

    if(appDemoDbEntryGet("useCpuPort1", &tmpData) == GT_OK)
    {
        cpuPortMux1UseSdma = 0;
    }
    else
    {
        cpuPortMux1UseSdma = 1;
    }

    if(appDemoDbEntryGet("useCpuPort2", &tmpData) == GT_OK)
    {
        cpuPortMux2UseSdma = 0;
    }
    else
    {
        cpuPortMux2UseSdma = 1;
    }

    if(appDemoDbEntryGet("ravenDevBmp", &tmpData) == GT_OK)
    {
        hwsFalconSetRavenDevBmp(tmpData);
    }

    if(appDemoDbEntryGet("serdesExternalFirmware", &tmpData) == GT_OK)
    {
        PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_SET(serdesExternalFirmware, (GT_BOOL)tmpData);
    }

    if(appDemoDbEntryGet("ravenCm3Uart", &tmpData) == GT_OK)
    {
        hwsFalconSetRavenCm3Uart(tmpData);
    }

    if (appDemoDbEntryGet("portMgr", &tmpData) != GT_OK)
    {
        portMgr = GT_FALSE;
    }
    else
    {
        portMgr = (tmpData == 1) ? GT_TRUE : GT_FALSE;
    }

    if (appDemoDbEntryGet("tsenEagleRead", &tmpData) == GT_OK)
    {
        tsenEagleRead = GT_TRUE;
    }
    else
    {
        tsenEagleRead = GT_FALSE;
    }

    if (appDemoDbEntryGet("phyGearboxMode", &tmpData) != GT_OK)
    {
        phyGearboxMode = GT_FALSE;
    }
    else
    {
        phyGearboxMode = (tmpData == 1) ? GT_TRUE : GT_FALSE;
    }

    modeSonic128MacPorts = 0;
    modeSonic32MacPorts  = 0;

    if(appDemoDbEntryGet("modeSonic32MacPorts", &tmpData) == GT_OK)
    {
        modeSonic32MacPorts = 1;

        /* reset those flags that may collide with the needed mode */
        modeMaxMac = 0;
        cpuPortMux1UseSdma = 0;
        cpuPortMux2UseSdma = 0;
    }

    if(appDemoDbEntryGet("modeSonic128MacPorts", &tmpData) == GT_OK)
    {
        GT_U32 currBoardType;
        rc = appDemoBoardTypeGet(&currBoardType);
        if (rc != GT_OK)
        {
            return rc;
        }
        if(currBoardType != APP_DEMO_FALCON_BOARD_12_8_B2B_CNS)
        {
            cpssOsPrintf("modeSonic128MacPorts supported only on B2B 12.8T device (missing 'b2b' indication)\n");
            return GT_BAD_STATE;
        }
        modeSonic128MacPorts = 1;

        /* reset those flags that may collide with the needed mode */
        modeMaxMac = 0;
        cpuPortMux1UseSdma = 0;
        cpuPortMux2UseSdma = 0;
    }

    rc = falcon_appDemoInitSequence(boardRevId, devIndex, devNum, hwInfo);
    if (rc != GT_OK)
        return rc;
    if(falcon_force_stop_init_after_d2d_init_get())
    {
        return GT_OK;
    }
    if (appDemoInitRegDefaults != GT_FALSE)
    {
        return GT_OK;
    }

    /* init the event handlers */
    rc = falcon_EventHandlerInit();
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_EventHandlerInit", rc);
    if (rc != GT_OK)
        return rc;

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E == system_recovery.systemRecoveryProcess)
    {
        /* Data Integrity module initialization. It should be done after events init. */
        rc = appDemoDataIntegrityInit(devNum);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDataIntegrityInit", rc);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    rc = falcon_localUtfInit(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_localUtfInit", rc);
    if (rc != GT_OK)
        return rc;
    rc = falcon_EgfEftPortFilter(devNum,devIndex);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_EgfEftPortFilter", rc);
    if (rc != GT_OK)
        return rc;

    rc = falcon_waitForTcamInitDone(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_waitForTcamInitDone", rc);
    if (rc != GT_OK)
        return rc;

#ifndef GM_USED
    rc = falcon_LedInit(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_LedInit", rc);
    if (rc != GT_OK)
        return rc;
#endif

    if(boardType == APP_DEMO_FALCON_BOARD_12_8_RD_CNS)
    {
        rc = falcon_RD_ledInit (CAST_SW_DEVNUM(devNum));
        CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_RD_ledInit ", rc);
        if (rc != GT_OK)
            return rc;
    }

    /* CPSSinit time measurement */
    rc = cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd);
    if(rc != GT_OK)
    {
        return rc;
    }

    seconds = secondsEnd-secondsStart;
    if(nanoSecondsEnd >= nanoSecondsStart)
    {
        nanoSec = nanoSecondsEnd-nanoSecondsStart;
    }
    else
    {
        nanoSec = (1000000000 - nanoSecondsStart) + nanoSecondsEnd;
        seconds--;
    }
    cpssOsPrintf("cpssInitSystem time: %d sec., %d nanosec.\n", seconds, nanoSec);


    cpssOsTimeRT(&end_sec, &end_nsec);
    if(end_nsec < start_nsec)
    {
        end_nsec += 1000000000;
        end_sec  -= 1;
    }
    diff_sec  = end_sec  - start_sec;
    diff_nsec = end_nsec - start_nsec;

    cpssOsPrintf("Time processing the cpssInitSimple (from 'phase1 init') is [%d] seconds + [%d] nanoseconds \n" , diff_sec , diff_nsec);

    systemInitialized = GT_TRUE;

    if (tsenEagleRead)
    {
        rc = appDemoEagleTsenTemperatureGetCreateTask(CAST_SW_DEVNUM(devNum));
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEagleTsenTemperatureGetCreateTask", rc);
        if (rc != GT_OK)
            return rc;
    }

    if (portMgr)
    {
        rc = cpssDxChPortManagerInit(CAST_SW_DEVNUM(devNum));
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortMgrInit", rc);
        if(rc != GT_OK)
        {
            return rc;
        }

       if ((boardType == APP_DEMO_FALCON_BOARD_12_8_TH_CNS )  ||
           (boardType == APP_DEMO_FALCON_BOARD_12_8_B2B_CNS ) ||
           (boardType == APP_DEMO_FALCON_BOARD_6_4_TH_CNS )   ||
           (boardType == APP_DEMO_FALCON_BOARD_2_TH_CNS )     ||
           (boardType == APP_DEMO_FALCON_BOARD_2T_4T_CNS))
        {
            /* RX temination default is Floating - all our DB's should have AVDD as default */
            cpssOsMemSet(&globalParamsStc, 0, sizeof(CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC));
            globalParamsStc.rxTermination = 1; /* AVDD */
            globalParamsStc.globalParamsBitmapType = CPSS_PORT_MANAGER_GLOBAL_PARAMS_RX_TERM_E;
            rc = cpssDxChPortManagerGlobalParamsOverride(devNum, &globalParamsStc);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortManagerGlobalParamsOverride", rc);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        rc = appDemoPortManagerTaskCreate(CAST_SW_DEVNUM(devNum));
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoPortManagerTaskCreate", rc);
        if (rc != GT_OK)
            return rc;
    }

    /* allow interrupts / appDemo tasks to stable */
    osTimerWkAfter(500);

    return rc;
}

/**
* @internal gtDbFalconBoardReg_AllDevicesSimpleInit function
* @endinternal
*
* @brief   This is the 'simple' board initialization function for two Falcon 6.4T/12.8T devices.
*
* @param[in] boardRevId               - Board revision Id.
* @param[in] allowCommonInit          - indication to allow common init
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
static GT_STATUS gtDbFalconBoardReg_AllDevicesSimpleInit
(
    IN  GT_U8  boardRevId,
    IN  GT_BOOL allowCommonInit
)
{
    GT_STATUS   rc;
    GT_U32      start_sec  = 0;
    GT_U32      start_nsec = 0;
    GT_U32      end_sec  = 0;
    GT_U32      end_nsec = 0;
    GT_U32      diff_sec;
    GT_U32      diff_nsec;
    CPSS_HW_INFO_STC   hwInfo[APP_DEMO_CPSS_MAX_NUM_PORT_GROUPS_CNS];
    GT_SW_DEV_NUM           devNum;
    GT_U8        devIndex;/* device index in the array of appDemoPpConfigList[devIndex] */
    GT_PCI_INFO pciInfo;
    GT_U32      tmpData;
    GT_U32      boardType;

    GT_U32      secondsStart, secondsEnd,
                nanoSecondsStart, nanoSecondsEnd,
                seconds, nanoSec; /* time of init */
    CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC   globalParamsStc;
    GT_BOOL     firstDev;
    GT_U32      numOfDevs;
    GT_CHAR     boardName[160];
    GT_BOOL     selectedDeviceFound = GT_FALSE;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery;

    rc = cpssOsTimeRT(&secondsStart,&nanoSecondsStart);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssOsTimeRT", rc);
    if(rc != GT_OK)
        return rc;

    if(GT_TRUE == allowCommonInit)
    {
#if (defined ASIC_SIMULATION) &&  (defined RTOS_ON_SIM)
        /*simulation initialization*/
        appDemoRtosOnSimulationInit();

#endif /*(defined ASIC_SIMULATION) &&  (defined RTOS_ON_SIM)*/

#ifdef ASIC_SIMULATION
        /* allow setting clock after the SOFT-RESET during the 'system reset'
           and 'system init' */
        if(simCoreClockOverwrittenGet())
        {
            simCoreClockOverwrite(simCoreClockOverwrittenGet());
        }
#endif

        /* Enable printing inside interrupt routine - supplied by extrernal drive */
        extDrvUartInit();

        /* Call to fatal_error initialization, use default fatal error call_back - supplied by mainOs */
        rc = osFatalErrorInit((FATAL_FUNC_PTR)NULL);
        if (rc != GT_OK)
            return rc;

        if(cpssDeviceRunCheck_onEmulator())
        {
            /* the 'NO KN' is doing polling in this interrupt global cause register */
            /* but this polling should not be seen during 'trace'                   */
            appDemoTrace_skipTrace_onAddress(PRV_CPSS_FALCON_MG0_BASE_ADDRESS_CNS + 0x30 /*0x1d000030*//*address*/,0/*index*/);
        }
    }

    switch(boardRevId)
    {
        case 1:
            boardType = APP_DEMO_FALCON_BOARD_6_4_TH_CNS;
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("modeMaxMac", 1);
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("numOfTiles", 2);
            break;

        case 2:
            boardType = APP_DEMO_FALCON_BOARD_12_8_B2B_CNS;
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("modeMaxMac", 1);
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("numOfTiles", 4);
            break;

        case 3:
            boardType = APP_DEMO_FALCON_BOARD_12_8_TH_CNS;
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("modeMaxMac", 1);
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("numOfTiles", 4);
            break;

        default:
            boardType = APP_DEMO_FALCON_BOARD_12_8_B2B_CNS;
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("modeMaxMac", 1);
            APP_DEMO_DB_ENTRY_ADD_PREVENT_OVERRIDE_MAC("numOfTiles", 4);
            break;
   }

    rc = falcon_boardTypeInitDatabases(boardType);
    if (rc != GT_OK)
    {
        return rc;
    }

    userForceBoardType(boardType);

    if(appDemoDbEntryGet("modeMaxMac", &tmpData) == GT_OK)
    {
        modeMaxMac = 1;
    }
    else
    {
        modeMaxMac = 0;
    }

    if(appDemoDbEntryGet("mode100G", &tmpData) == GT_OK)
    {
        mode100G = 1;
    }
    else
    {
        mode100G = 0;
    }

    if(appDemoDbEntryGet("useCpuPort1", &tmpData) == GT_OK)
    {
        cpuPortMux1UseSdma = 0;
    }
    else
    {
        cpuPortMux1UseSdma = 1;
    }

    if(appDemoDbEntryGet("useCpuPort2", &tmpData) == GT_OK)
    {
        cpuPortMux2UseSdma = 0;
    }
    else
    {
        cpuPortMux2UseSdma = 1;
    }

    if(appDemoDbEntryGet("ravenDevBmp", &tmpData) == GT_OK)
    {
        hwsFalconSetRavenDevBmp(tmpData);
    }

    if(appDemoDbEntryGet("ravenCm3Uart", &tmpData) == GT_OK)
    {
        hwsFalconSetRavenCm3Uart(tmpData);
    }

    if (appDemoDbEntryGet("portMgr", &tmpData) != GT_OK)
    {
        portMgr = GT_FALSE;
    }
    else
    {
        portMgr = (tmpData == 1) ? GT_TRUE : GT_FALSE;
    }

    if (appDemoDbEntryGet("tsenEagleRead", &tmpData) == GT_OK)
    {
        tsenEagleRead = GT_TRUE;
    }
    else
    {
        tsenEagleRead = GT_FALSE;
    }

    if (appDemoDbEntryGet("phyGearboxMode", &tmpData) != GT_OK)
    {
        phyGearboxMode = GT_FALSE;
    }
    else
    {
        phyGearboxMode = (tmpData == 1) ? GT_TRUE : GT_FALSE;
    }

    if(appDemoDbEntryGet("modeSonic128MacPorts", &tmpData) == GT_OK)
    {
        GT_U32 currBoardType;
        rc = appDemoBoardTypeGet(&currBoardType);
        if (rc != GT_OK)
        {
            return rc;
        }
        if(currBoardType != APP_DEMO_FALCON_BOARD_12_8_B2B_CNS)
        {
            cpssOsPrintf("modeSonic128MacPorts supported only on B2B 12.8T device (missing 'b2b' indication)\n");
            return GT_BAD_STATE;
        }
        modeSonic128MacPorts = 1;

        /* reset those flags that may collide with the needed mode */
        modeMaxMac = 0;
        cpuPortMux1UseSdma = 0;
        cpuPortMux2UseSdma = 0;
    }
    else
    {
        modeSonic128MacPorts = 0;
    }

    /* take time from the 'phase1 init' stage (not including the 'PCI scan' operations) */
    cpssOsTimeRT(&start_sec, &start_nsec);

    firstDev = GT_TRUE;
    for (devIndex = 0; (1); devIndex++)
    {
        SYSTEM_SKIP_NON_ACTIVE_DEV(SYSTEM_DEV_NUM_MAC(devIndex));
        if(devIndex > 0)
        {
            /* support skip of devIndex = 0 */
            firstDev = GT_FALSE;
        }

        /* this function finds all Prestera devices on PCI bus */
        rc = falcon_getBoardInfo(firstDev, devIndex, &hwInfo[devIndex], &pciInfo);
        if (rc != GT_OK)
        {
            break;
        }

        devNum = SYSTEM_DEV_NUM_MAC(devIndex);
        /* In case a specific device requested in command-line, skip if
           this is not the one */
        if (appDemoSysIsDeviceSelected())
        {
            if (!appDemoSysIsSelectedDevice(devNum))
            {
                continue;
            }
            cpssOsPrintf("Using Device %d (PCI ID %x:%x:%x.%x)\n", devNum,
                         pciInfo.pciBusNum >> 8, pciInfo.pciBusNum & 0xff,
                         pciInfo.pciIdSel, pciInfo.funcNo);
            osMemCpy(&hwInfo[0], &hwInfo[devIndex], sizeof(hwInfo[0]));
            devIndex = 0;
            selectedDeviceFound = GT_TRUE;
        }

        if( boardType == APP_DEMO_FALCON_BOARD_6_4_TH_CNS)
        {
            osSprintf(boardName, "6.4T TH Device[%d]", devIndex);
            falcon_boardTypePrint(boardName, "Falcon" /*devName*/);
        }else if ( boardType == APP_DEMO_FALCON_BOARD_12_8_B2B_CNS)
        {
            osSprintf(boardName, "12.8T Belly-to-Belly Device[%d]", devIndex);
            falcon_boardTypePrint(boardName, "Falcon" /*devName*/);
        }else if ( boardType == APP_DEMO_FALCON_BOARD_12_8_TH_CNS)
        {
            osSprintf(boardName, "12.8T TH Device[%d]", devIndex);
            falcon_boardTypePrint(boardName, "Falcon" /*devName*/);
        }else
        {
            return GT_FAIL;
        }

        devNum = SYSTEM_DEV_NUM_MAC(devIndex);

        rc = appDemoDxChBoardTypeGet(CAST_SW_DEVNUM(devNum), boardRevId, &bc2BoardType);
        if (rc != GT_OK)
        {
            break;
        }

        rc = falcon_appDemoInitSequence(boardRevId, devNum, devNum, &hwInfo[devIndex]);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (appDemoInitRegDefaults != GT_FALSE)
        {
            return GT_OK;
        }

        /* not support 'system reset' */
        rc = cpssDevSupportSystemResetSet(CAST_SW_DEVNUM(devNum), GT_FALSE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDevSupportSystemResetSet", rc);
        if (GT_OK != rc)
            return rc;

        rc = falcon_EgfEftPortFilter(devNum,devNum);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_EgfEftPortFilter", rc);
        if (rc != GT_OK)
            return rc;

        rc = falcon_waitForTcamInitDone(devNum);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_waitForTcamInitDone", rc);
        if (rc != GT_OK)
            return rc;

    #ifndef GM_USED
        rc = falcon_LedInit(CAST_SW_DEVNUM(devNum));
        CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_LedInit", rc);
        if (rc != GT_OK)
            return rc;
    #endif
        if (appDemoPpConfigDevAmount == 1)
        {
            break;
        }

        if (selectedDeviceFound)
        {
            devIndex = 1; /* So numOfDevs, few lines below, will get 1 */
            break;
        }

        if(GT_TRUE == isUnderAddDeviceCatchup)
        {
            /* we did insert of the single device , not continue the while (1) */
            devIndex ++; /* so numOfDevs will hold this index */
            break;
        }
    }


    if (appDemoSysIsDeviceSelected() && !selectedDeviceFound)
    {
        cpssOsPrintf("Warning: Selected device number was not detected\n");
    }

    if (appDemoCpssPciProvisonDone == GT_FALSE)
    {
        numOfDevs = devIndex;
    }
    else
    {
        numOfDevs = appDemoPpConfigDevAmount;
    }
    /* init the event handlers */
    rc = falcon_EventHandlerInit();
    CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_EventHandlerInit", rc);
    if (rc != GT_OK)
        return rc;

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E == system_recovery.systemRecoveryProcess)
    {
        for (devIndex = SYSTEM_DEV_NUM_MAC(0); devIndex < SYSTEM_DEV_NUM_MAC(numOfDevs); devIndex++)
        {
            SYSTEM_SKIP_NON_ACTIVE_DEV(devIndex);

            devNum = appDemoPpConfigList[devIndex].devNum;
            /* Data Integrity module initialization. It should be done after events init. */
            rc = appDemoDataIntegrityInit(devNum);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDataIntegrityInit", rc);
            if(GT_OK != rc)
            {
                return rc;
            }
        }
    }


    for (devIndex = SYSTEM_DEV_NUM_MAC(0); devIndex < SYSTEM_DEV_NUM_MAC(numOfDevs); devIndex++)
    {
        SYSTEM_SKIP_NON_ACTIVE_DEV(devIndex);

        devNum = appDemoPpConfigList[devIndex].devNum;
        rc = falcon_localUtfInit(devNum);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_localUtfInit", rc);
        if (rc != GT_OK)
            return rc;
    }

    /* CPSSinit time measurement */
    rc = cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd);
    if(rc != GT_OK)
    {
        return rc;
    }

    seconds = secondsEnd-secondsStart;
    if(nanoSecondsEnd >= nanoSecondsStart)
    {
        nanoSec = nanoSecondsEnd-nanoSecondsStart;
    }
    else
    {
        nanoSec = (1000000000 - nanoSecondsStart) + nanoSecondsEnd;
        seconds--;
    }
    cpssOsPrintf("cpssInitSystem time: %d sec., %d nanosec.\n", seconds, nanoSec);


    cpssOsTimeRT(&end_sec, &end_nsec);
    if(end_nsec < start_nsec)
    {
        end_nsec += 1000000000;
        end_sec  -= 1;
    }
    diff_sec  = end_sec  - start_sec;
    diff_nsec = end_nsec - start_nsec;

    cpssOsPrintf("Time processing the gtDbDxFalconBoardReg_AllDevicesSimpleInit (from 'phase1 init') is [%d] seconds + [%d] nanoseconds \n" , diff_sec , diff_nsec);

    systemInitialized = GT_TRUE;

    if (tsenEagleRead)
    {
        for (devIndex = SYSTEM_DEV_NUM_MAC(0); (devIndex < SYSTEM_DEV_NUM_MAC(numOfDevs)); devIndex++)
        {
            SYSTEM_SKIP_NON_ACTIVE_DEV(devIndex);

            devNum = appDemoPpConfigList[devIndex].devNum;
            rc = appDemoEagleTsenTemperatureGetCreateTask(CAST_SW_DEVNUM(devNum));
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEagleTsenTemperatureGetCreateTask", rc);
            if (rc != GT_OK)
                return rc;
        }
    }

    if (portMgr)
    {
        for (devIndex = SYSTEM_DEV_NUM_MAC(0); (devIndex < SYSTEM_DEV_NUM_MAC(numOfDevs)); devIndex++)
        {
            SYSTEM_SKIP_NON_ACTIVE_DEV(devIndex);

            devNum = appDemoPpConfigList[devIndex].devNum;
            rc = cpssDxChPortManagerInit(CAST_SW_DEVNUM(devNum));
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortMgrInit", rc);
            if(rc != GT_OK)
            {
                return rc;
            }

           if ((boardType == APP_DEMO_FALCON_BOARD_12_8_TH_CNS )  ||
               (boardType == APP_DEMO_FALCON_BOARD_12_8_B2B_CNS ) ||
               (boardType == APP_DEMO_FALCON_BOARD_6_4_TH_CNS )   ||
               (boardType == APP_DEMO_FALCON_BOARD_2_TH_CNS )     ||
               (boardType == APP_DEMO_FALCON_BOARD_2T_4T_CNS))
            {
                /* RX temination default is Floating - all our DB's should have AVDD as default */
                cpssOsMemSet(&globalParamsStc, 0, sizeof(CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC));
                globalParamsStc.rxTermination = 1; /* AVDD */
                globalParamsStc.globalParamsBitmapType = CPSS_PORT_MANAGER_GLOBAL_PARAMS_RX_TERM_E;
                rc = cpssDxChPortManagerGlobalParamsOverride(devNum, &globalParamsStc);
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortManagerGlobalParamsOverride", rc);
                if(rc != GT_OK)
                {
                    return rc;
                }

            }
            rc = appDemoPortManagerTaskCreate(CAST_SW_DEVNUM(devNum));
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoPortManagerTaskCreate", rc);
            if (rc != GT_OK)
                return rc;
        }
    }

    /* allow interrupts / appDemo tasks to stable */
    osTimerWkAfter(500);

    return rc;
}

extern void cpssInitSystemGet
(
    OUT GT_U32  *boardIdxPtr,
    OUT GT_U32  *boardRevIdPtr,
    OUT GT_U32  *multiProcessAppPtr
);
extern void cpssInitSystemSet
(
    IN GT_U32  boardIdx,
    IN GT_U32  boardRevId
);

/**
* @internal gtDbFalconBoardReg_2Devices_AllDevicesSimpleInit function
* @endinternal
*
* @brief   This is the 'simple' board initialization function for two Falcon 6.4T/12.8T devices.
*
* @param[in] boardRevId               - Board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
static GT_STATUS multiDevices_cpssPpInsert_SimpleInit
(
    IN  GT_U8  boardRevId,
    IN const GT_U8 busDevNumArr[3]
)
{
    GT_STATUS   rc;
    GT_U32 orig_boardIdx, orig_boardRevId, orig_reMultiProcessApp;
    GT_U32  tmpData,full_init_skip_device_bmp;
    GT_U32  currDevNum;

    GT_UNUSED_PARAM(boardRevId);

    if(isHirApp)
    {
        /* the appDemo already runs with the '-hir' indication
           so adjust the missing init part that needed for '-hir'
        */
        cpssSystemBaseInit();
    }

    cpssInitSystemGet(&orig_boardIdx, &orig_boardRevId, &orig_reMultiProcessApp);

    /* need  to split the DMA between 3 devices */
    appDemoDbEntryAdd ("dma_reduce_factor",3);

    if(GT_OK == appDemoDbEntryGet ("full_init_skip_device_bmp",&tmpData) && tmpData)
    {
        full_init_skip_device_bmp = tmpData;
    }
    else
    {
        full_init_skip_device_bmp = 0;
    }

    /*
        NOTE: we insert the 3 devices as :
        devNum = 0 --> Aldrin2
        devNum = 1 --> Falcon
        devNum = 2 --> Aldrin2

        we init the Falcon 'between' the other 2 Aldrin2 devices , to cover
        the bug of tables DB of CPSS of :
        JIRA : CPSS-13946 : CPSS table initialization with multiple device Aldrin2 + Falcon failed
    */

    isHirApp = 1;/* allow to insert device */

    /* set the 0,0 so the Aldrin2 init will not see 'previous cpssInitSystem'
       that is the 'current cpssInitSystem'  */
    cpssInitSystemSet(0, 0);

    currDevNum = SYSTEM_DEV_NUM_MAC(0);
    if(0 == (full_init_skip_device_bmp & BIT_0))
    {
        rc = cpssPpInsert(CPSS_CHANNEL_PCI_E ,
            /*busNum*/ 0,
            /*busDevNum*/ busDevNumArr[1],
            /*cpssDevNum*/ currDevNum ,
            0xFF,APP_DEMO_CPSS_HIR_INSERTION_FULL_INIT_E);
        if(rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPpInsert FAILED : on first Aldrin2", rc);
            return rc;
        }

        currDevNum++;
    }

    /* not support full_init_skip_device_bmp of the middle device , as the appdemo
       expect continues device numbers , from first to last */
    rc = cpssPpInsert(CPSS_CHANNEL_PEX_E ,
        /*busNum*/ 0,
        /*busDevNum*/ busDevNumArr[0],
        /*cpssDevNum*/ currDevNum ,
        0xFF,APP_DEMO_CPSS_HIR_INSERTION_FULL_INIT_E);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPpInsert FAILED : on the Falcon", rc);
        return rc;
    }

    currDevNum++;

    if(0 == (full_init_skip_device_bmp & BIT_2))
    {
        /*init second Aldrin2*/
        rc = cpssPpInsert(CPSS_CHANNEL_PCI_E ,
            /*busNum*/ 0,
            /*busDevNum*/ busDevNumArr[2],
            /*cpssDevNum*/ currDevNum ,
            0xFF,APP_DEMO_CPSS_HIR_INSERTION_FULL_INIT_E);
        if(rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPpInsert FAILED : on second Aldrin2", rc);
            return rc;
        }

        currDevNum++;
    }

    isHirApp = 0;/*restore non 'hir' mode */

    /* restore falcon default */
    cpssInitSystemSet(orig_boardIdx, orig_boardRevId);

    return rc;
}
/**
* @internal gtDbFalconBoardReg_2Devices_AllDevicesSimpleInit function
* @endinternal
*
* @brief   This is the 'simple' board initialization function for two Falcon 6.4T/12.8T devices.
*
* @param[in] boardRevId               - Board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
static GT_STATUS gtDbFalconBoardReg_2Devices_AllDevicesSimpleInit
(
    IN  GT_U8  boardRevId
)
{
    if(boardRevId == 4)/* (3 devices) single Falcon , 2 Aldrin2 : Falcon first on PCIe */
    {
        static const GT_U8 busDevNumArr[3] =
                              {0, /* Falcon */
                               1, /* first Aldrin2 */
                               2};/* second Aldrin2*/
        return multiDevices_cpssPpInsert_SimpleInit(boardRevId,busDevNumArr);
    }
    else
    if(boardRevId == 5)/* (3 devices) single Falcon , 2 Aldrin2 : Falcon second on PCIe */
    {
        static const GT_U8 busDevNumArr[3] =
                              {1, /* Falcon */
                               0, /* first Aldrin2 */
                               2};/* second Aldrin2*/
        return multiDevices_cpssPpInsert_SimpleInit(boardRevId,busDevNumArr);
    }
    else
    if(boardRevId == 6)/* (3 devices) single Falcon , 2 Aldrin2 :  : Falcon third on PCIe*/
    {
        static const GT_U8 busDevNumArr[3] =
                              {2, /* Falcon */
                               0, /* first Aldrin2 */
                               1};/* second Aldrin2*/
        return multiDevices_cpssPpInsert_SimpleInit(boardRevId,busDevNumArr);
    }

    if (appDemoCpssPciProvisonDone == GT_FALSE)
    {
        appDemoPpConfigDevAmount = 2;
    }

    return gtDbFalconBoardReg_AllDevicesSimpleInit(boardRevId,GT_TRUE);
}

/**
* @internal gtDbFalconBoardReg_BoardCleanDbDuringSystemReset function
* @endinternal
*
* @brief   clear the DB of the specific board config file , as part of the 'system rest'
*         to allow the 'cpssInitSystem' to run again as if it is the first time it runs
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS gtDbFalconBoardReg_BoardCleanDbDuringSystemReset
(
    IN  GT_U8   boardRevId
)
{
    UNUSED_PARAM_MAC(boardRevId);

    appDemo_PortsInitList_already_done = 0;
    actualNum_cpssApi_falcon_defaultMap = 0;

    return GT_OK;
}
/**
* @internal gtDbPxFalconBoardReg function
* @endinternal
*
* @brief   Registration function for the Falcon board .
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbPxFalconBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
)
{
    UNUSED_PARAM_MAC(boardRevId);

    /* set the actual pointer to use for this board */
    boardCfgFuncs->boardSimpleInit               = gtDbFalconBoardReg_SimpleInit;
    boardCfgFuncs->boardCleanDbDuringSystemReset = gtDbFalconBoardReg_BoardCleanDbDuringSystemReset;
    boardCfgFuncs->deviceSimpleInit              = gtDbFalconBoardReg_deviceSimpleInit;

    return GT_OK;
}

/**
* @internal gtDbPxFalconBoard2DevsReg function
* @endinternal
*
* @brief   Registration function for two Falcon 6.4T devices board .
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbPxFalconBoard2DevsReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
)
{
    UNUSED_PARAM_MAC(boardRevId);

    /* set the actual pointer to use for this board */
    boardCfgFuncs->boardSimpleInit               = gtDbFalconBoardReg_2Devices_AllDevicesSimpleInit;
    boardCfgFuncs->boardCleanDbDuringSystemReset = gtDbFalconBoardReg_BoardCleanDbDuringSystemReset;
    boardCfgFuncs->deviceSimpleInit              = gtDbFalconBoardReg_deviceSimpleInit;

    return GT_OK;
}

/************************************************************************/
/* Tracing specific registers access */
/************************************************************************/
#if 0
    /** Range parameteres: Base, size */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,
    /** Range parameteres: base, step, amount, size  */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E,
    /** Range parameteres: addr */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E,
    /** Range parameteres: base, step, amount  */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E,
    /** marks end of array */
    APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_LAST_E
#endif
static APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_STC falconPakcetBufferParseRegsArr[] =
{
     {0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E, 0x1, "TILE", {0x00000000, 0x20000000, 4, 0x20000000}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x2  , "PB_CENTER", {0x19000000, 0x00010000, 0}}
    ,{0x2, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,   0x100, "center",    {0x1000, 0x1000, 0}}
    ,{0x2, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,   0x200, "counter", {0x0000, 0x1000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x300, "PB_SMB_WRITE_ARBITER", {0x19030000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x3  , "PB_GPC_GRP_PACKET_WRITE_0", {0x19040000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x4,   "PB_GPC_GRP_CELL_READ_0", {0x19060000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x4,   "PB_GPC_GRP_CELL_READ_1", {0x19070000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x5,   "PB_GPC_GRP_PACKET_READ_0", {0x19080000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x5,   "PB_GPC_GRP_PACKET_READ_1", {0x19090000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x3,   "PB_GPC_GRP_PACKET_WRITE_1", {0x190A0000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x4,   "PB_GPC_GRP_CELL_READ_2", {0x190C0000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x4,   "PB_GPC_GRP_CELL_READ_3", {0x190D0000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x5,   "PB_GPC_GRP_PACKET_READ_2", {0x190E0000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x5,   "PB_GPC_GRP_PACKET_READ_3", {0x190F0000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x400, "PB_NEXT_POINTER_MEMO_0", {0x19100000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x400, "PB_NEXT_POINTER_MEMO_1", {0x19110000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x400, "PB_NEXT_POINTER_MEMO_2", {0x19120000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x500, "PB_SHARED_MEMO_BUF_0_0", {0x19130000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x500, "PB_SHARED_MEMO_BUF_0_1", {0x19140000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x500, "PB_SHARED_MEMO_BUF_1_0", {0x19150000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x500, "PB_SHARED_MEMO_BUF_1_1", {0x19160000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x500, "PB_SHARED_MEMO_BUF_2_0", {0x19170000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x500, "PB_SHARED_MEMO_BUF_2_1", {0x19180000, 0x00010000, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x1000, "CTRL_PIPE0", {0x08000000, 0x08000000, 0, 0}}
    ,{0x1,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E, 0x1000, "CTRL_PIPE1", {0x10000000, 0x08000000, 0, 0}}
    ,{0x3,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x600, "main",         {0x1000, 0x1000}}
    ,{0x3,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E, 0x700, "packetWrite",  {0x0000, 0x0400, 4, 0x0400}}
    ,{0x4,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x800, "main",         {0x0200, 0x0100}}
    ,{0x4,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E, 0x900, "cellRead",     {0x0000, 0x0100, 2, 0x0100}}
    ,{0x5,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0xA00, "main",         {0x2000, 0x1000}}
    ,{0x5,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E, 0xB00, "packetRead",   {0x0000, 0x1000, 2, 0x1000}}
    /* pbCenter*/
    ,{0x100, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "npmRandomizerEnable", {0x0000}}
    ,{0x100, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "tileId",              {0x0404}}
    ,{0x100, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "tileMode",            {0x0408}}
    ,{0x100, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "mswCollapseCfg",      {0x0410}}
    ,{0x100, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "badAddress",          {0x0414}}
    /* pbCointer in  pbCenter*/
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "enablePbc",           {0x000C}}
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgTile",             {0x002C}}
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "enableMasterMode",    {0x0010}}
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "enableInterTileTransact",{0x0014}}
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "resizeSmbCells",      {0x001C}}
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "resizeSmbMcCells",    {0x0020}}
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "resizeSmbSumCells",   {0x0030}}
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "resizeNpmWords",      {0x0024}}
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "resizeNpmMcWords",    {0x0028}}
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "resizeNpmSumWords",   {0x0034}}
    ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgFillThreshold",    {0x0038}}
    /* pbSmbWriteArbiter */
    ,{0x300, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgEnable",           {0x0004}}
    /* gpcGrpPacketWrite */
    ,{0x600, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "npmRandomizerEnable", {0x0014}}
    ,{0x600, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "badAddress",          {0x0018}}
    /* gpcPacketWrite */
    ,{0x700, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgEnable",           {0x0000}}
    ,{0x700, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgList",             {0x0004}}
    ,{0x700, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgNxp",              {0x0008}}
    ,{0x700, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgWrt",              {0x000C}}
    ,{0x700, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgStat",             {0x0010}}
    /* gpcGrpCellRead */
    ,{0x800, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "badRegister",         {0x0014}}
    /* gpcCellRead */
    ,{0x900, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "initialization",      {0x0000}}
    ,{0x900, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "configuration",       {0x0004}}
    ,{0x900, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "aging",               {0x0008}}
    /* gpcGrpPacketRead */
    ,{0xA00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "badAddress",          {0x0014}}
    /* gpcPacketRead */
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "initialization",      {0x0000}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "miscEnable",          {0x0004}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "aging",               {0x0008}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "channelEnable",{0x0080, 4, 10}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "channelReset", {0x0180, 4, 10}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "channelShaper",{0x0280, 4, 10}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "packetCountEnable",   {0x0380}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "packetCountReset",    {0x0384}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "fifoSegmentStart",{0x0700, 4, 10}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "fifoSegmentNext", {0x0400, 4, 20}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgUpdateTrigger",    {0x0800}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E,        0, "arbiterPointHighSpeedChannel0En", {0x0804}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "arbiterPointGroup1", {0x0814, 4, 5}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "arbiterPointGroup2", {0x0828, 4, 5}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "arbiterCalendarSlot",{0x083C, 4, 32}}
    ,{0xB00, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "arbiterCalendarMax",  {0x08BC}}
    /* npmMc */
    ,{0x400, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgEnable",           {0x0000}}
    ,{0x400, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgInit",             {0x0004}}
    ,{0x400, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgUnit",             {0x0300}}
    ,{0x400, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgRefs",             {0x0400}}
    ,{0x400, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "clrRefs",      {0x0410, 4, 4}}
    ,{0x400, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "insList",      {0x0210, 4, 4}}
    ,{0x400, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "intReinsertMask",     {0x011C}}
    /* smbMc */
    ,{0x500, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgEnable",           {0x0004}}
    ,{0x500, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "cfgInit",             {0x0000}}
    ,{0x500, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "dbgPayloadCtrl",      {0x0040}}
    ,{0x500, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "dbgPayloadData", {0x0050, 4, 8}}
    /* TX_DMA*/
    ,{0x1000, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E, 0x1100, "TX_DMA",      {0x05560000, 0x20000, 4, 0x10000}}
    ,{0x1100, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E, 0, "TXD Channel Reset", {0x2100, 4, 9}}
    ,{0x1100, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E, 0, "TDS Channel Reset",        {0x4004}}

    /* end of list */
    ,{0, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_LAST_E, 0, "", {0}}
};

void appDemoHwPpDrvRegTracingFalconPb()
{
    appDemoHwPpDrvRegTracingRegNameBind(falconPakcetBufferParseRegsArr);
    appDemoHwPpDrvRegTracingRegAddressesRangeSet(
        0xDFE00000, 0x19000000, /* Packet Buffer */
        0xDFFF0000, 0x0D560000, /* Pipe0 TX_DMA0 */ /* TX_FIFO 0x0D570000 sipped */
        0xDFFF0000, 0x0D580000, /* Pipe0 TX_DMA1 */
        0xDFFF0000, 0x0D5A0000, /* Pipe0 TX_DMA2 */
        0xDFFF0000, 0x0D5C0000, /* Pipe0 TX_DMA3 */
        0xDFFF0000, 0x15560000, /* Pipe1 TX_DMA0 */
        0xDFFF0000, 0x15580000, /* Pipe1 TX_DMA1 */
        0xDFFF0000, 0x155A0000, /* Pipe1 TX_DMA2 */
        0xDFFF0000, 0x155C0000, /* Pipe1 TX_DMA3 */
        0);
    appDemoHwPpDrvRegTracingBind(1);
}

GT_STATUS appDemoTxqDebugHelp
(
  GT_VOID
)
{
    cpssOsPrintf ("  1.Dump scheduler errors count and type - prvCpssDxChTxqFalconDumpSchedErrors (devNum)\n");
    cpssOsPrintf ("  2.Dump queue token bucket level and deficit - prvCpssDxChTxqDumpQueueStatus (devNum,tileNum,queueStart,queueEnd)\n");
    cpssOsPrintf ("  3.Dump A node (port) token bucket level and deficit - prvCpssDxChTxqDumpAlevelStatus(devNum,tileNum,aLevelStart,aLevelEnd)\n");
    cpssOsPrintf ("  4.Dump queue shaping configuration parameters - prvCpssDxChTxqDumpQueueShapingParameters(devNum,tileNum,queueStart,queueEnd)\n");
    cpssOsPrintf ("  5.Dump A node (port)  shaping configuration parameters - prvCpssDxChTxqDumpAnodeShapingParameters(devNum,tileNum,aLevelStart,aLevelEnd)\n");
    cpssOsPrintf ("  6.Dump PDX routing table and PDX pizza arbiter -  prvCpssFalconTxqUtilsPdxTablesDump(devNum,tileNum)\n");
    cpssOsPrintf ("  7.Dump PDQ queue to SDQ/PDS mapping -  prvCpssFalconTxqUtilsPsiConfigDump(devNum,tileNum)\n");
    cpssOsPrintf ("  8.Dump TxQ descriptor counters for specific tile and PDS  -  prvCpssDxChTxqFalconDescCounterDump(devNum,tileNum,pdsNum,queueStart,queueEnd)\n");
    cpssOsPrintf ("  9.Dump schedule profile attributes  -  prvCpssFalconTxqUtilsDumpSchedDbProfiles(devNum)\n");
    cpssOsPrintf ("  10.Dump mapping of local port to queues at SDQ -  prvCpssFalconTxqUtilsSdqPortRangeDump(devNum,tileNum,sdqNum)\n");
    cpssOsPrintf ("  11.Dump  dump queue attributes -  prvCpssFalconTxqUtilsSdqQueueAttributesDump(devNum,tileNum,sdqNum,num of q to dump)\n");
    cpssOsPrintf ("  12.Print port to schedule profile binding -  prvCpssFalconTxqUtilsDumpBindPortsToSchedDb(devNum)\n");
     cpssOsPrintf (" 13.Dump  physical port to P node and A node mapping(also PDQ Q numbers). -  prvCpssFalconTxqUtilsPhysicalPortsToSwShadowDump(devNum,tileNum)\n");
     cpssOsPrintf (" 14.Print the configure tree under a specific port (SW data base ,no read from HW) -  prvCpssFalconTxqPdqDumpPortSw(devNum,tileNum,portNum)\n");
     cpssOsPrintf (" 15.Print the configure tree under a specific port ( read from HW) -  prvCpssFalconTxqPdqDumpPortHw(devNum,tileNum,portNum)\n");

    return GT_OK;
}

void appDemoForbidenTestsAdd(IN FORBIDEN_TESTS   forbidenTests[])
{
#ifdef INCLUDE_UTF

    GT_U32  ii = 0;

    while(forbidenTests[ii].testName)
    {
        utfAddPreSkippedRule(
            "",
            "",
            forbidenTests[ii].testName,
            forbidenTests[ii].testName);

        ii++;
    }
#else
   GT_UNUSED_PARAM(forbidenTests);
#endif
    return;
}

void appDemoForbidenSuitesAdd(IN FORBIDEN_SUITES   forbidenSuites[])
{
#ifdef INCLUDE_UTF
    GT_U32  ii = 0;

    while(forbidenSuites[ii].suiteName)
    {
        utfAddPreSkippedRule(
            forbidenSuites[ii].suiteName,
            forbidenSuites[ii].suiteName,
            "",
            "");

        ii++;
    }
#else
   GT_UNUSED_PARAM(forbidenSuites);
#endif
    return;
}

/*
    MainUT tests:
    list of tests/suites that forbidden because crash the CPSS or the device
   (fatal error in simulation on unknown address)
*/
GT_STATUS   appDemoFalconMainUtForbidenTests(void)
{
    static FORBIDEN_TESTS mainUt_forbidenTests_CRASH[] =
    {
        /* test CRASH or FATAL ERROR ! */

        /* test fail an may fail others */
        /*doing remap that fail in TQX scheduler :
            ERROR GT_BAD_PARAM in function: falcon_xel_to_cpss_err_code, file prvcpssdxchtxqpdq.c, line[137].
            [prvCpssFalconTxqPdqPeriodicSchemeConfig , line[415]] failed : in pdqNum[0] (retCode_xel[-1])

            but this test may fail others
            */
        TEST_NAME(cpssDxChPortPhysicalPortMapSet                  )

        ,{NULL}/* must be last */
    };

    /* suites that most or all tests CRASH or FATAL ERROR ! */
    static FORBIDEN_SUITES mainUt_forbidenSuites_CRASH [] =
    {
         SUITE_NAME(cpssDxChPortCtrl                               )
        ,SUITE_NAME(cpssDxChPortStat                               )
        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS mainUt_forbidenTests_many_tests_fail [] =
    {
         TEST_NAME(/*cpssDxChCscd.*/cpssDxChCscdHyperGPortCrcModeSet                             )
        ,TEST_NAME(/*cpssDxChCscd.*/cpssDxChCscdHyperGPortCrcModeGet                             )
        ,TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagPrbsSerdesStatusGet                              )
        ,TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagPrbsSerdesTestEnableSet                          )
        ,TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagPrbsSerdesTestEnableGet                          )
        ,TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagPrbsSerdesTransmitModeSet                        )
        ,TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagPrbsSerdesTransmitModeGet                        )
        ,TEST_NAME(/*cpssDxChHwInit.*/cpssDxChHwAccessInfoStoreTest                              )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhyPortAddrSet                                     )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhyPortAddrGet                                     )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhyAutonegSmiGet                                   )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhyAutonegSmiSet                                   )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhySmiAutoMediaSelectSet                           )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhySmiAutoMediaSelectGet                           )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhySmiMdcDivisionFactorGet                         )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhySmiMdcDivisionFactorSet                         )
        ,TEST_NAME(/*cpssDxChPhySmiPreInit.*/cpssDxChPhyAutoPollNumOfPortsSet                    )
        ,TEST_NAME(/*cpssDxChPhySmiPreInit.*/cpssDxChPhyAutoPollNumOfPortsGet                    )
        ,TEST_NAME(/*cpssDxChPortIfModeCfg.*/cpssDxChPortModeSpeedSet                            )

        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxQShaperProfileSet                            )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTx4TcTailDropProfileSet                        )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTx4TcTailDropProfileGet                        )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTailDropProfileSet                           )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxTailDropProfileGet                           )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxBindPortToDpSet                              )
        ,TEST_NAME(/*cpssDxChPortTx.*/cpssDxChPortTxBindPortToDpGet                              )

        ,TEST_NAME(/*cpssDxChPortBufMg.*/cpssDxChPortFcHolSysModeSet                             )
        ,TEST_NAME(/*cpssDxChPortBufMg.*/cpssDxChPortGlobalPacketNumberGet                       )
        ,TEST_NAME(/*cpssDxChPortBufMg.*/cpssDxChPortRxBufNumberGet                              )

        ,TEST_NAME(/*cpssDxChPortPfc.*/cpssDxChPortPfcCounterGet                                 )


        ,{NULL}/* must be last */
    };

#ifdef IMPL_TGF

    /* suites that most or all tests fail ! */
    /* those tests are about 'timing' */
    static FORBIDEN_SUITES onEmulator_enhUt_forbidenSuites_many_tests_fail [] =
    {
         SUITE_NAME(tgfNetIf/* this suite must be 'debugged' before running it on emulator */)
        ,{NULL}/* must be last */
    };

#endif /*IMPL_TGF*/

    appDemoForbidenTestsAdd(mainUt_forbidenTests_CRASH);
    appDemoForbidenSuitesAdd(mainUt_forbidenSuites_CRASH);
    appDemoForbidenTestsAdd(mainUt_forbidenTests_many_tests_fail);

#ifdef IMPL_TGF

    if(cpssDeviceRunCheck_onEmulator())
    {
        appDemoForbidenSuitesAdd(onEmulator_enhUt_forbidenSuites_many_tests_fail);
    }



#endif /*IMPL_TGF*/

    return GT_OK;
}

/* compare values of MG registers in the different MG units */
GT_STATUS   appDemoFalconMgRegCompare(void)
{
  static GT_U32   falconMgRegisters[] =
  {
       0x1D000034
      ,0x1D00003C
      ,0x1D00004C
      ,0x1D000050
      ,0x1D000058
      ,0x1D000060
      ,0x1D000084
      ,0x1D0000A8
      ,0x1D0000C0
      ,0x1D0000C4
      ,0x1D0000C8
      ,0x1D0000CC
      ,0x1D000154
      ,0x1D00015C
      ,0x1D000204
      ,0x1D00020C
      ,0x1D000210
      ,0x1D00023C
      ,0x1D000254
      ,0x1D0003F0
      ,0x1D0003F8
      ,0x1D0003FC
      ,0x1D00051C
      ,0x1D00260C
      ,0x1D00261C
      ,0x1D00262C
      ,0x1D00263C
      ,0x1D00264C
      ,0x1D00265C
      ,0x1D00266C
      ,0x1D00267C
      ,0x1D002680
      ,0x1D0026C0
      ,0x1D0026C4
      ,0x1D0026C8
      ,0x1D0026CC
      ,0x1D0026D0
      ,0x1D0026D4
      ,0x1D0026D8
      ,0x1D0026DC
      ,0x1D002700
      ,0x1D002704
      ,0x1D002708
      ,0x1D002710
      ,0x1D002714
      ,0x1D002718
      ,0x1D002720
      ,0x1D002724
      ,0x1D002728
      ,0x1D002730
      ,0x1D002734
      ,0x1D002738
      ,0x1D002740
      ,0x1D002744
      ,0x1D002748
      ,0x1D002750
      ,0x1D002754
      ,0x1D002758
      ,0x1D002760
      ,0x1D002764
      ,0x1D002768
      ,0x1D002770
      ,0x1D002774
      ,0x1D002778
      ,0x1D002800
      ,0x1D002814
      ,0x1D002818
      ,0x1D002860
      ,0x1D002864
      ,0x1D002870
      ,0x1D002874
      ,0x1D002878
      ,0x1D00287C
      ,0x1D002880
      ,0x1D002884
      ,0x1D002888
      ,0x1D00288C
      ,0x1D0028BC
      ,0x1D0028C8
      /* must be last*/
      ,GT_NA
    };
    GT_STATUS   rc;
    GT_U32  ii,jj;
    GT_U8 devNum = 0;
    GT_U32  offsetFromMg0BaseAddr[CPSS_MAX_SDMA_CPU_PORTS_CNS];
    GT_U32  readValue[CPSS_MAX_SDMA_CPU_PORTS_CNS];
    GT_U32  numMgs = PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgNumOfUnits;
    GT_U32  holdDiff;

    offsetFromMg0BaseAddr[0] = 0;
    cpssOsMemSet(readValue, 0, sizeof(readValue));

    cpssOsPrintf("RegAddr       MG[ 0]");

    for(jj = 1 ; jj < numMgs; jj++)
    {
        offsetFromMg0BaseAddr[jj] = prvCpssSip6OffsetFromFirstInstanceGet(devNum,jj,PRV_CPSS_DXCH_UNIT_MG_E);
        cpssOsPrintf("       MG[%1.2d]",jj);
    }
    cpssOsPrintf("\n");


    for(ii = 0 ; falconMgRegisters[ii] != GT_NA ; ii++)
    {
        holdDiff = 0;
        /*******************************************************/
        /* read the register from MG0 and compare to other MGs */
        /*******************************************************/
        for(jj = 0; jj < numMgs ; jj++)
        {
            rc = cpssDrvPpHwRegisterRead(devNum,0,falconMgRegisters[ii]+offsetFromMg0BaseAddr[jj],&readValue[jj]);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(jj > 0)
            {
                /* compare the values MGj to MG0 */
                if(readValue[jj] != readValue[0])
                {
                    holdDiff = 1;
                }
            }
        }

        if(holdDiff == 0)
        {
            cpssOsPrintf("[0x%8.8x] [0x%8.8x] same value all MGs \n",falconMgRegisters[ii],readValue[0]);
        }
        else
        {
            cpssOsPrintf("[0x%8.8x] ",falconMgRegisters[ii]);

            for(jj = 0 ; jj < numMgs ; jj++)
            {
                cpssOsPrintf("[0x%8.8x] ",readValue[jj]);
            }
            cpssOsPrintf("\n");
        }
    }

    return GT_OK;
}

static GT_STATUS falcon_getMappings
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   portNum,
    OUT GT_U32                   *macNum,
    OUT GT_U32                   *serdesNum
)
{
    /*GT_U32                          firstPortNumInGop;*/
    CPSS_DXCH_PORT_MAP_STC          portMap;
    /*CPSS_PORT_MAC_TO_SERDES_STC     macToSerdesMuxStc;*/
    CPSS_DXCH_ACTIVE_LANES_STC        portParams;
    GT_STATUS rc;

    /* Get Raven's lane number (0-15) */
    rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, /*OUT*/&portMap);
    if (rc != GT_OK)
    {
        return rc;
    }

    *macNum = portMap.interfaceNum;
    rc = cpssDxChPortSerdesActiveLanesListGet(devNum,portNum,&portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    *serdesNum = portParams.activeLaneList[0];
    return GT_OK;
}

#define ALL_PORTS 0xFFFF
/**
* @internal falcon_printMappings function
* @endinternal
*
* @brief   Print all internal mappings: Port-Mac-SerDes
*
* @param[in] devNum                - The CPSS devNum.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS falcon_printMappings
(
    IN  GT_U8            devNum,
    IN  GT_U32           portNum
)
{
    GT_U32                          portIterator, macNum=0,serdesNum = 0;
    GT_U32                          ravenId, ravenDevBmp;
    GT_BOOL                         invertTx;
    GT_BOOL                         invertRx;
    GT_STATUS                       rc;

    if(appDemoDbEntryGet("ravenDevBmp", &ravenDevBmp) != GT_OK)
    {
        ravenDevBmp = 0xffff;
    }

    cpssOsPrintf("+------+------+--------+-------+-------+-------+\n");
    cpssOsPrintf("| Port | Mac  | SerDes | Raven |Inv.Tx |Inv.Rx |\n");
    cpssOsPrintf("+------+------+--------+-------+-------+-------+\n");

    for(portIterator = 0; portIterator < appDemoPpConfigList[devNum].maxPortNumber ; portIterator++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portIterator);
        if (prvCpssDxChPortRemotePortCheck(devNum,portIterator))
        {
            continue;
        }

        rc = falcon_getMappings(devNum,portIterator,&macNum,&serdesNum);
        if (rc != GT_OK)
        {
            return rc;
        }


        if((64 * PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles) <= macNum)
        {
            continue;
        }

        if((ALL_PORTS != portNum) && (portIterator != portNum))
        {
            continue;
        }

        ravenId = macNum / 16;
        if((ravenDevBmp&(1<<ravenId))==0)
        {
            continue;
        }

        rc = cpssDxChPortSerdesLanePolarityGet(devNum,0,serdesNum,&invertTx,&invertRx);
        if (rc != GT_OK)
        {
            return rc;
        }


        cpssOsPrintf("| %4d | %4d |  %4d  | %4d  | %5s | %5s |\n",portIterator,
                                                                  macNum,
                                                                  serdesNum,
                                                                  ravenId,
                                                                  invertTx?"TRUE ":"FALSE",
                                                                  invertRx?"TRUE ":"FALSE");
        cpssOsPrintf("+------+------+--------+-------+-------+-------+\n");
    }
    return GT_OK;
}

#ifndef ASIC_SIMULATION

/**
 * falcon_overrideTxParams function
 *
 *
 * @param devNum
 * @param portNum
 * @param laneNum
 *
 * @return GT_STATUS
 */
GT_STATUS falcon_overrideTxParams
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               laneNum
)
{
    GT_U32                          portIterator, laneIterator;
    CPSS_DXCH_ACTIVE_LANES_STC      activeLanes;
    GT_STATUS                       rc;

    if(ALL_PORTS == portNum)
    {
        for(portIterator = 0; portIterator < appDemoPpConfigList[devNum].maxPortNumber ; portIterator++)
        {
            CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portIterator);
            if (prvCpssDxChPortRemotePortCheck(devNum,portIterator))
            {
                continue;
            }
            rc = prvCpssDxChPortParamsGet(devNum,portIterator,&activeLanes);
            if(rc != GT_OK)
            {
                return rc;
            }
            for(laneIterator = 0; laneIterator < activeLanes.numActiveLanes; laneIterator++)
            {
                rc = cpssDxChPortSerdesManualTxConfigSet(devNum,portIterator,laneIterator,&falcon_RD_serdesTxParams[RD_FALCON_SHORT_OR_LONG_TRACE_PORT_PARAMS(portIterator)]);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    else
    {
        return cpssDxChPortSerdesManualTxConfigSet(devNum,portNum,laneNum,&falcon_RD_serdesTxParams[RD_FALCON_SHORT_OR_LONG_TRACE_PORT_PARAMS(portNum)]);
    }
    return GT_OK;
}
#endif

/*********************************** PHY APIs ************************************/

#if !defined(ASIC_SIMULATION) && defined(INCLUDE_MPD)

/**
 * falcon_phy_convertMultilanesToSingleLaneOpMode function
 *
 *
 * @param devNum
 * @param opMode
 * @param laneCount
 * @param singleLaneOpMode
 *
 * @return GT_STATUS
 */
GT_STATUS falcon_phy_convertMultilanesToSingleLaneOpMode
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_APPDEMO_XPHY_OP_MODE  opMode,
    OUT GT_U32                   *laneCount,
    OUT GT_APPDEMO_XPHY_OP_MODE  *singleLaneOpMode
)
{
    GT_UNUSED_PARAM(devNum);

    if(NULL == laneCount || NULL == singleLaneOpMode)
    {
        return GT_BAD_PTR;
    }

    switch(opMode)
    {
        case RETIMER_HOST_50G_KR2_LINE_50G_KR2:
            *singleLaneOpMode = RETIMER_HOST_25G_KR_LINE_25G_KR;
            *laneCount = 2;
            break;
        case RETIMER_HOST_100G_KR2_LINE_100G_KR2:
            *singleLaneOpMode = RETIMER_HOST_50G_KR_LINE_50G_KR;
            *laneCount = 2;
            break;
        case RETIMER_HOST_100G_KR4_LINE_100G_KR4:
            *singleLaneOpMode = RETIMER_HOST_25G_KR_LINE_25G_KR;
            *laneCount = 4;
            break;
        case RETIMER_HOST_200G_KR4_LINE_200G_KR4:
            *singleLaneOpMode = RETIMER_HOST_50G_KR_LINE_50G_KR;
            *laneCount = 4;
            break;
        case RETIMER_HOST_200G_KR8_LINE_200G_KR8:
            *singleLaneOpMode = RETIMER_HOST_25G_KR_LINE_25G_KR;
            *laneCount = 8;
            break;
        case RETIMER_HOST_400G_KR8_LINE_400G_KR8:
            *singleLaneOpMode = RETIMER_HOST_50G_KR_LINE_50G_KR;
            *laneCount = 8;
            break;
        default:
            /* no multilanes retimer mode*/
            *singleLaneOpMode = opMode;
            *laneCount = 1;
            break;
    }

    return GT_OK;
}

/**
 * falcon_phy_getPortNum function
 *
 *
 * @param devNum
 * @param falconPortNum
 * @param phyIndex
 * @param phyPortNum
 *
 * @return GT_STATUS
 */
GT_STATUS falcon_phy_getPortNum
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_U32                   falconPortNum,
    OUT GT_U32                   *phyIndex,
    OUT GT_U32                   *phyPortNum
)
{
    GT_U32      macNum, serdesNum, ravenIndex, slotNum = 0;
    GT_U32      currBoardType;
    GT_STATUS   rc;

    rc = falcon_getMappings(devNum,falconPortNum,&macNum,&serdesNum);
    if (rc != GT_OK)
    {
        return rc;
    }


    rc = appDemoBoardTypeGet(&currBoardType);
    if (rc != GT_OK)
    {
        return rc;
    }
    if(currBoardType == APP_DEMO_FALCON_BOARD_12_8_RD_CNS)
    {
        RD_GET_SLOT_NUM_FROM_PORT_NUM_MAC(falconPortNum,slotNum)
        GET_PHY_INDEX_FROM_GROUP_ID_MAC(slotNum,(*phyIndex));
    }
    else
    {
        ravenIndex = macNum / NUM_OF_LANES_PER_PHY;
        GET_PHY_INDEX_FROM_GROUP_ID_MAC(ravenIndex,(*phyIndex));
    }

    if(*phyIndex == SKIP_PHY)
    {
        /*CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_phy_getPortNum failed: PHY does not exist for this port", GT_BAD_PARAM);*/
        return GT_BAD_PARAM;
    }
    if (falcon_RavenLaneToPhyLaneMap != NULL)
    {
        *phyPortNum = falcon_RavenLaneToPhyLaneMap[*phyIndex].lanes[serdesNum % 16];
    }
    else if(currBoardType == APP_DEMO_FALCON_BOARD_12_8_RD_CNS)
    {
        /* PHY port's numbering in RD board has a pattern and can be calculated by formula instead of using mapping table */
        *phyIndex +=
                    (RD_NUM_OF_PHY_PER_SLOT - 1 - /* 0-3 number represents the PHY's index inside the specific slot */
                    (((falconPortNum - RD_FIRST_PORT_NUM) % RD_NUM_OF_PORTS_PER_SLOT) / 4));
        *phyPortNum = (falconPortNum - RD_FIRST_PORT_NUM) % RD_NUM_OF_PORTS_PER_SLOT % 4;
                       /* 0-3 represents the PHY port's index inside the specific PHY */
        switch(*phyPortNum)
        { /* Use the PHY port's index to calculate the phyPortNum according to the pattern */
        case 0:
            *phyPortNum = 2 * 4; /*  8 */
            break;
        case 1:
            *phyPortNum = 3 * 4; /* 12 */
            break;
        case 2:
            *phyPortNum = 0 * 4; /*  0 */
            break;
        case 3:
            *phyPortNum = 1 * 4; /*  4 */
            break;
        default:
            return GT_BAD_PARAM;
            break;
        }
    }
    else
    {
        *phyPortNum = serdesNum %  NUM_OF_LANES_PER_PHY;
    }
    return GT_OK;
}

GT_STATUS falcon_phy_get_laneBmp
(
   GT_U32                       portNum,
   GT_U32                       *hostSideLanesBmp,
   GT_U32                       *lineSideLanesBmp
)
{
    MPD_RESULT_ENT ret;
    GT_U32 rel_ifIndex;
    MPD_OPERATIONS_PARAMS_UNT params;
    cpssOsMemSet(&params, 0 , sizeof(params));

    rel_ifIndex = portNum;
    ret = mpdPerformPhyOperation(rel_ifIndex,
                                 PRV_MPD_OP_CODE_GET_LANE_BMP_E,
                                 &params);
    if(ret == MPD_OK_E)
    {
         if(hostSideLanesBmp != NULL)
         {
            *hostSideLanesBmp = params.internal.phyLaneBmp.hostSideLanesBmp;
         }
         if(lineSideLanesBmp != NULL)
         {
            *lineSideLanesBmp = params.internal.phyLaneBmp.lineSideLanesBmp;
         }
    }
    return (ret == MPD_OK_E)? GT_OK: GT_FAIL;
}

/**
* @internal falcon_phy_tunePort function
* @endinternal
*
* @brief   Tune PHY port
*
* @param[in] devNum                - The CPSS devNum.
* @param[in] falconPortNum         - The CPSS portNum.
* @param[in] hostOrLine            - Choose Host/Line/Both sides
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS falcon_phy_tunePort
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      falconPortNum,
    IN  GT_APPDEMO_XPHY_HOST_LINE   hostOrLine
)
{
    GT_U32                      phyPortNum, phyIndex;
    GT_U32                      laneBmpHost, lanesBmpLine;
    CPSS_PORT_SERDES_TUNE_STC   *hostSideTuneParamsPtr;
    CPSS_PORT_SERDES_TUNE_STC   *lineSideTuneParamsPtr;
    GT_STATUS                   rc;

    rc = falcon_phy_getPortNum(devNum,falconPortNum,&phyIndex,&phyPortNum);
    if (rc != GT_OK)
    {
        return rc;
    }


    if(falcon_PhyTuneParams == falcon_RD_PhyTuneParams) /* if RD board*/
    {
        hostSideTuneParamsPtr = &falcon_PhyTuneParams[GT_APPDEMO_XPHY_HOST_SIDE+RD_PHY_SHORT_OR_LONG_TRACE_PORT_PARAMS(falconPortNum)];
        lineSideTuneParamsPtr = &falcon_PhyTuneParams[GT_APPDEMO_XPHY_LINE_SIDE+RD_PHY_SHORT_OR_LONG_TRACE_PORT_PARAMS(falconPortNum)];
    }
    else
    {
        hostSideTuneParamsPtr = &falcon_PhyTuneParams[GT_APPDEMO_XPHY_HOST_SIDE];
        lineSideTuneParamsPtr = &falcon_PhyTuneParams[GT_APPDEMO_XPHY_LINE_SIDE];
    }

    if(!((hostOrLine == GT_APPDEMO_XPHY_HOST_SIDE) || (hostOrLine == GT_APPDEMO_XPHY_BOTH_SIDES)))
    {
         hostSideTuneParamsPtr = NULL;
    }

    if(!((hostOrLine == GT_APPDEMO_XPHY_LINE_SIDE) || (hostOrLine == GT_APPDEMO_XPHY_BOTH_SIDES)))
    {
         lineSideTuneParamsPtr = NULL;
    }


    rc = falcon_phy_get_laneBmp(falconPortNum, &laneBmpHost, &lanesBmpLine);
    if (rc != GT_OK)
    {
        return rc;
    }

    return gtAppDemoPhyPortTune(falconPortNum,laneBmpHost, lanesBmpLine, hostSideTuneParamsPtr,lineSideTuneParamsPtr);

}

/**
* @internal falcon_phy_setTxParams function
* @endinternal
*
* @brief   Set PHY port lane Tx parameters
*
* @param[in] devNum                - The CPSS devNum.
* @param[in] falconPortNum         - The CPSS portNum.
* @param[in] hostOrLine            - Choose Host/Line/Both sides
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS falcon_phy_setTxParams
(
    IN  GT_PHYSICAL_PORT_NUM            falconPortNum,
    IN  GT_APPDEMO_XPHY_HOST_LINE       hostOrLine,
    IN  GT_U32                          atten,
    IN  GT_U32                          post,
    IN  GT_U32                          pre,
    IN  GT_U32                          pre2,
    IN  GT_U32                          pre3
)
{
    CPSS_PORT_SERDES_TUNE_STC   tuneParams, *hostSideTuneParamsPtr = NULL, *lineSideTuneParamsPtr = NULL;
    GT_U32                      laneBmpHost, lanesBmpLine;
    GT_STATUS                   rc;

    tuneParams.txTune.avago.atten  = atten;
    tuneParams.txTune.avago.post   = post;
    tuneParams.txTune.avago.pre    = pre;
    tuneParams.txTune.avago.pre2   = pre2;
    tuneParams.txTune.avago.pre3   = pre3;

    if((hostOrLine == GT_APPDEMO_XPHY_HOST_SIDE) || (hostOrLine == GT_APPDEMO_XPHY_BOTH_SIDES))
    {
         hostSideTuneParamsPtr = &tuneParams;
    }

    if((hostOrLine == GT_APPDEMO_XPHY_LINE_SIDE) || (hostOrLine == GT_APPDEMO_XPHY_BOTH_SIDES))
    {
         lineSideTuneParamsPtr = &tuneParams;
    }
    rc = falcon_phy_get_laneBmp(falconPortNum, &laneBmpHost, &lanesBmpLine);
    if (rc != GT_OK)
    {
        return rc;
    }

    return gtAppDemoPhyPortTxConfigSet(falconPortNum,laneBmpHost, lanesBmpLine, hostSideTuneParamsPtr,lineSideTuneParamsPtr);
}

/**
 * falcon_phy_GetThermal function
 *
 *
 * @param devNum
 * @param falconPortNum
 * @param coreTemperature
 *
 * @return GT_STATUS
 */
GT_STATUS falcon_phy_GetThermal
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            falconPortNum,
    OUT GT_32                           *coreTemperature

)
{
    return gtAppDemoPhyGetTemperature(devNum, falconPortNum, coreTemperature);
}

/**
* @internal falcon_get_mpd_if_index function
* @endinternal
*
*/
GT_STATUS falcon_get_mpd_if_index
(
    IN GT_U32    devNum,
    IN GT_U32    portNum,
    OUT GT_U32   *mpd_ifIndex
)
{

    devNum = devNum;
    *mpd_ifIndex = mpdIfIndexArr[portNum];
    return GT_TRUE;
}


/**
 * @internal falcon_phy_deletePort
 * @endinternal
 *
 *  @brief   Power down PHY port
 *
 * @param devNum
 * @param falconPortNum
 *
 * @retval GT_OK                    - on success,
    * @retval GT_FAIL               - otherwise.
 */
GT_STATUS falcon_phy_deletePort
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_U32                   falconPortNum
)
{
    GT_STATUS                       rc;
    GT_U32 mpd_ifIndex;

    /*TODO power down PCS*/

    /*Power down PHY port*/
    falcon_get_mpd_if_index(devNum, falconPortNum, &mpd_ifIndex);
    rc = gtAppDemoPhyPortPowerdown(devNum, mpd_ifIndex, GT_TRUE);
    if(GT_OK != rc)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("gtAppDemoPhyPortPowerdown Failed", rc);
        return rc;
    }

    return GT_OK;
}

static GT_STATUS falcon_PhyModeGet
(
    GT_APPDEMO_XPHY_OP_MODE        opMode,
    GT_APPDEMO_XPHY_SPEED_EXT_STC  *speedParam
)
{
    switch(opMode)
    {
    case RETIMER_HOST_25G_KR_LINE_25G_KR:
        speedParam->hostSide.speed  = CPSS_PORT_SPEED_25000_E;
        speedParam->hostSide.ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
        break;
    case RETIMER_HOST_50G_KR_LINE_50G_KR:
        speedParam->hostSide.speed  = CPSS_PORT_SPEED_50000_E;
        speedParam->hostSide.ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
        break;
    case RETIMER_HOST_50G_KR2_LINE_50G_KR2:
        speedParam->hostSide.speed  = CPSS_PORT_SPEED_50000_E;
        speedParam->hostSide.ifMode = CPSS_PORT_INTERFACE_MODE_KR2_E;
        break;
    case RETIMER_HOST_100G_KR2_LINE_100G_KR2:
        speedParam->hostSide.speed  = CPSS_PORT_SPEED_100G_E;
        speedParam->hostSide.ifMode = CPSS_PORT_INTERFACE_MODE_KR2_E;
        break;
    case RETIMER_HOST_100G_KR4_LINE_100G_KR4:
        speedParam->hostSide.speed  = CPSS_PORT_SPEED_100G_E;
        speedParam->hostSide.ifMode = CPSS_PORT_INTERFACE_MODE_KR4_E;
        break;
    case RETIMER_HOST_200G_KR4_LINE_200G_KR4:
        speedParam->hostSide.speed  = CPSS_PORT_SPEED_200G_E;
        speedParam->hostSide.ifMode = CPSS_PORT_INTERFACE_MODE_KR4_E;
        break;
    case RETIMER_HOST_200G_KR8_LINE_200G_KR8:
        speedParam->hostSide.speed  = CPSS_PORT_SPEED_200G_E;
        speedParam->hostSide.ifMode = CPSS_PORT_INTERFACE_MODE_KR8_E;
        break;
    case RETIMER_HOST_400G_KR8_LINE_400G_KR8:
        speedParam->hostSide.speed  = CPSS_PORT_SPEED_400G_E;
        speedParam->hostSide.ifMode = CPSS_PORT_INTERFACE_MODE_KR8_E;
        break;
    case  PCS_HOST_25G_KR_LINE_25G_KR:
        speedParam->hostSide.speed  = CPSS_PORT_SPEED_25000_E;
        speedParam->hostSide.ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
        speedParam->lineSide.speed  = CPSS_PORT_SPEED_25000_E;
        speedParam->lineSide.ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
        break;
    case  PCS_HOST_50G_KR_LINE_50G_KR:
        speedParam->hostSide.speed  = CPSS_PORT_SPEED_50000_E;
        speedParam->hostSide.ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
        speedParam->lineSide.speed  = CPSS_PORT_SPEED_50000_E;
        speedParam->lineSide.ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
        break;
    case  PCS_HOST_100G_KR2_LINE_100G_CR4:
        speedParam->hostSide.speed  = CPSS_PORT_SPEED_100G_E;
        speedParam->hostSide.ifMode = CPSS_PORT_INTERFACE_MODE_KR2_E;
        speedParam->lineSide.speed  = CPSS_PORT_SPEED_100G_E;
        speedParam->lineSide.ifMode = CPSS_PORT_INTERFACE_MODE_CR4_E;
        break;
    case  PCS_HOST_100G_KR2_LINE_100G_SR4:
    case  PCS_HOST_100G_KR2_AP_LINE_100G_SR4:
        speedParam->hostSide.speed  = CPSS_PORT_SPEED_100G_E;
        speedParam->hostSide.ifMode = CPSS_PORT_INTERFACE_MODE_KR2_E;
        speedParam->lineSide.speed  = CPSS_PORT_SPEED_100G_E;
        speedParam->lineSide.ifMode = CPSS_PORT_INTERFACE_MODE_SR_LR4_E;
        break;
    case  PCS_HOST_100G_KR2_LINE_100G_LR4:
    case  PCS_HOST_100G_KR2_AP_LINE_100G_LR4_AP:
        speedParam->hostSide.speed  = CPSS_PORT_SPEED_100G_E;
        speedParam->hostSide.ifMode = CPSS_PORT_INTERFACE_MODE_KR2_E;
        speedParam->lineSide.speed  = CPSS_PORT_SPEED_100G_E;
        speedParam->lineSide.ifMode = CPSS_PORT_INTERFACE_MODE_SR_LR4_E;
        break;
    case  PCS_HOST_100G_KR2_LINE_100G_KR2:
        speedParam->hostSide.speed  = CPSS_PORT_SPEED_100G_E;
        speedParam->hostSide.ifMode = CPSS_PORT_INTERFACE_MODE_KR2_E;
        speedParam->lineSide.speed  = CPSS_PORT_SPEED_100G_E;
        speedParam->lineSide.ifMode = CPSS_PORT_INTERFACE_MODE_KR2_E;
        break;
    case  PCS_HOST_100G_KR2_LINE_100G_CR2:
        speedParam->hostSide.speed  = CPSS_PORT_SPEED_100G_E;
        speedParam->hostSide.ifMode = CPSS_PORT_INTERFACE_MODE_KR2_E;
        speedParam->lineSide.speed  = CPSS_PORT_SPEED_100G_E;
        speedParam->lineSide.ifMode = CPSS_PORT_INTERFACE_MODE_CR2_E;
        break;
    default:
        break;
    }
    return GT_OK;
}

/**
* @internal falcon_phy_createPort function
* @endinternal
*
* @brief   Create PHY port
*
* @param[in] devNum                - The CPSS devNum.
* @param[in] falconPortNum         - The CPSS portNum.
* @param[in] opMode                - PHY port's operation mode.
* @param[in] fecMode               - PHY port's FEC mode
* @param[in] shortChannel          - short/long channel boolean.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS falcon_phy_createPort
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  GT_U32                   falconPortNum,
    IN  GT_APPDEMO_XPHY_OP_MODE  opMode,
    IN  GT_APPDEMO_XPHY_FEC_MODE fecModeHost,
    IN  GT_APPDEMO_XPHY_FEC_MODE fecModeLine
)
{
    GT_STATUS                       rc;
    GT_U32                          phyPortNum, phyIndex;
    GT_U32                          hostSideActiveLanesBmp, lineSideActiveLanesBmp;
    GT_APPDEMO_XPHY_OP_MODE         singleLaneOpMode;
    GT_U32                          multiLaneCount, laneIndex;

    CPSS_PORT_SERDES_TUNE_STC      *hostSideTuneParamsPtr;
    CPSS_PORT_SERDES_TUNE_STC      *lineSideTuneParamsPtr;
    GT_BOOL                        hostOpModeAp = GT_FALSE, lineOpModeAp = GT_FALSE;
    GT_U32 mpd_ifIndex;
    GT_APPDEMO_XPHY_SPEED_EXT_STC  speedParam;

    cpssOsMemSet(&speedParam, 0 , sizeof(speedParam));

    if(!gtAppDemoPhyIsPcsMode(opMode) && (phyGearboxMode == GT_TRUE))
    {
        cpssOsPrintf("Cannot set PHY to retimer mode. Board was initialized to support gearbox modes only.\n");
        return GT_BAD_STATE;
    }
    if(gtAppDemoPhyIsPcsMode(opMode) && (phyGearboxMode == GT_FALSE))
    {
        cpssOsPrintf("Cannot set PHY to gearbox mode. Board was initialized to support retimer modes only.\n");
        return GT_BAD_STATE;
    }

    falcon_get_mpd_if_index(devNum, falconPortNum, &mpd_ifIndex);

    speedParam.hostSide.fecMode = fecModeHost;
    speedParam.lineSide.fecMode = fecModeLine;

    if(phyGearboxMode)
    {   /* Gearbox mode */

        rc = falcon_PhyModeGet(opMode, &speedParam);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = falcon_phy_getPortNum(devNum,falconPortNum,&phyIndex,&phyPortNum);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Skip Tx config and port tune if AP mode */
        rc = gtAppDemoPhyApOpModeDetect(opMode, &hostOpModeAp, &lineOpModeAp);
        if(GT_OK != rc)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_opModeDetect Failed", rc);
        }
        speedParam.isRetimerMode = GT_FALSE;
        rc = gtAppDemoPhySetPortMode(mpd_ifIndex,
                                     speedParam,
                                     hostOpModeAp,
                                     lineOpModeAp);
        if(GT_OK != rc)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("gtAppDemoPhySetPortMode Failed", rc);
        }

        if(falcon_PhyTuneParams == falcon_RD_PhyTuneParams) /* if RD board*/
        {
            hostSideTuneParamsPtr = &falcon_PhyTuneParams[GT_APPDEMO_XPHY_HOST_SIDE+RD_PHY_SHORT_OR_LONG_TRACE_PORT_PARAMS(falconPortNum)];
            lineSideTuneParamsPtr = &falcon_PhyTuneParams[GT_APPDEMO_XPHY_LINE_SIDE+RD_PHY_SHORT_OR_LONG_TRACE_PORT_PARAMS(falconPortNum)];
        }
        else
        {
            hostSideTuneParamsPtr = &falcon_PhyTuneParams[GT_APPDEMO_XPHY_HOST_SIDE];
            lineSideTuneParamsPtr = &falcon_PhyTuneParams[GT_APPDEMO_XPHY_LINE_SIDE];
        }

        rc = falcon_phy_get_laneBmp(mpd_ifIndex, &hostSideActiveLanesBmp,&lineSideActiveLanesBmp);
        if (rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_phy_createPort failed: gtAppDemoPhyGetPortLanesBmp", rc);
            return rc;
        }

        rc = gtAppDemoPhyPortTxConfigSet(mpd_ifIndex, hostSideActiveLanesBmp, lineSideActiveLanesBmp,
                                                 (hostOpModeAp ? hostSideTuneParamsPtr : NULL),(lineOpModeAp ? lineSideTuneParamsPtr : NULL));
        if (rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_phy_createPort failed: gtAppDemoPhyPortTxConfigSet", rc);
        }

        rc = gtAppDemoPhyPortTune(mpd_ifIndex, hostSideActiveLanesBmp, lineSideActiveLanesBmp,
                                                 (hostOpModeAp ? hostSideTuneParamsPtr : NULL),(lineOpModeAp ? lineSideTuneParamsPtr : NULL));
        if (rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_phy_createPort failed: gtAppDemoPhyPortSetAndTune", rc);
        }

        if (falcon_PhyGearboxHostSidePolarityArray != NULL)
        {
            rc = gtAppDemoPhyPolarityArraySet(mpd_ifIndex, GT_APPDEMO_XPHY_HOST_SIDE,hostSideActiveLanesBmp,
                                              (APPDEMO_SERDES_LANE_POLARITY_STC*)&falcon_PhyGearboxHostSidePolarityArray[phyIndex], NUM_OF_LANES_PER_PHY);
            if (rc != GT_OK)
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_phy_createPort failed: gtAppDemoPhyPolarityArraySet Host-Side", rc);
                return rc;
            }
        }
        if (falcon_PhyGearboxLineSidePolarityArray != NULL)
        {
            rc = gtAppDemoPhyPolarityArraySet(mpd_ifIndex, GT_APPDEMO_XPHY_LINE_SIDE,lineSideActiveLanesBmp,
                                              (APPDEMO_SERDES_LANE_POLARITY_STC*)&falcon_PhyGearboxLineSidePolarityArray[phyIndex], NUM_OF_LANES_PER_PHY);
            if (rc != GT_OK)
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_phy_createPort failed: gtAppDemoPhyPolarityArraySet Line-Side", rc);
                return rc;
            }
        }
    }
    else
    {   /* Retimer Mode*/
        /* For multilanes retimer opModes - configure each lane separately */

        falcon_phy_convertMultilanesToSingleLaneOpMode(devNum,opMode,&multiLaneCount,&singleLaneOpMode);
        rc = falcon_PhyModeGet(opMode, &speedParam);
        if (rc != GT_OK)
        {
            return rc;
        }


        rc = falcon_phy_getPortNum(devNum,falconPortNum,&phyIndex,&phyPortNum);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Skip Tx config and port tune if AP mode */
        rc = gtAppDemoPhyApOpModeDetect(opMode, &hostOpModeAp, &lineOpModeAp);
        if(GT_OK != rc)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_opModeDetect Failed", rc);
        }

        speedParam.isRetimerMode    = GT_TRUE;

        for(laneIndex = 0 ; laneIndex < multiLaneCount ; laneIndex++)
        {
            rc = gtAppDemoPhySetPortMode(mpd_ifIndex,
                                         speedParam,
                                         hostOpModeAp,
                                         lineOpModeAp);
            if(GT_OK != rc)
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("gtAppDemoPhySetPortMode Failed", rc);
            }
        }
    }

    return GT_OK;
}

/**
 * @internal falcon_phy_getPortLinkStatus
 * @endinternal
 *
 * @brief Get PCS link status
 *
 * @param devNum
 * @param falconPortNum
 * @param linkStatus
 *
 * @return GT_STATUS
 */
GT_STATUS falcon_phy_getPortLinkStatus
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            falconPortNum,
    OUT GT_U16                          *linkStatus
)
{
    GT_STATUS                   rc;
    GT_U32 mpd_ifIndex;

    falcon_get_mpd_if_index(devNum, falconPortNum, &mpd_ifIndex);
    rc = gtAppDemoPhyGetPortLinkStatus(mpd_ifIndex, linkStatus);
    if(GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}

/*********************************** Debugging functions (TEMP) ************************************/

/**
 * printPhy function
 *
 *
 * @return GT_STATUS
 */
GT_STATUS printPhy()
{
    GT_U32 i = 128, phyIndex, phyPortNum;

    for(; i < 256 ; i++)
    {
        falcon_phy_getPortNum(0,i,&phyIndex,&phyPortNum);
        osPrintf("-falconPortNum[%3d]--phyIndex[%3d]--phyPortNum[%3d]--phyAddr[0x%x]\n",i,phyIndex,phyPortNum,falcon_PhyInfo[phyIndex].phyAddr);
    }
    return GT_OK;
}

/**
 * phyPortStatus function
 *
 *
 * @param devNum
 * @param falconPortNum
 * @param laneNum
 * @param lineSide
 *
 * @return GT_STATUS
 */
GT_STATUS phyPortStatus
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            falconPortNum,
    IN  GT_PHYSICAL_PORT_NUM            laneNum,
    IN  GT_BOOL                         lineSide
)
{
    MYD_STATUS                      rc;
    GT_U32                          phyPortNum, phyIndex;
    MYD_DEV_PTR                     pDev;
    MYD_STATE_DUMP                  info;
    GT_STATUS                       cpssRc;

    cpssRc = falcon_phy_getPortNum(devNum,falconPortNum,&phyIndex,&phyPortNum);
    if (cpssRc != GT_OK)
    {
        return cpssRc;
    }

    pDev = (MYD_DEV_PTR)falcon_PhyInfo[phyIndex].driverObj;

    rc = mydDiagStateDump(pDev, phyPortNum/4 + pDev->mdioPort,
                                     lineSide ? 3 : 4, phyPortNum % 4 + laneNum, 0, &info);
    if(MYD_OK != rc)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("falcon_phy_portStatus Failed", rc);
        return GT_FAIL;
    }

    osPrintf("attenuation=[%d] postCursor=[%d] preCursor=[%d] pre2Cursor=[%d] pre3Cursor=[%d]\n",
             info.txFFE.postCursor,
             info.txFFE.attenuation,
             info.txFFE.preCursor,
             info.txFFE.pre2Cursor,
             info.txFFE.pre3Cursor);
    return GT_OK;
}

#endif

#ifndef ASIC_SIMULATION
/**
 * falconDebugEgfStatusGet function
 *
 *
 * @param devNum
 *
 * @return GT_STATUS
 */
GT_STATUS falconDebugEgfStatusGet(GT_U8 devNum)
{
    GT_STATUS rc;
    GT_U32 portNum;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT state;

    for(portNum = 0; portNum < appDemoPpConfigList[devNum].maxPortNumber ; portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);
        if (prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            continue;
        }

        rc = cpssDxChBrgEgrFltPortLinkEnableGet(devNum,portNum, &state);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPhysicalPortMapGet", rc);

        if(CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E != state)
        {
            cpssOsPrintf("EGF FORCE LINK DOWN: port %d\n", portNum);
        }
    }

    return GT_OK;
}
#endif /*ASIC_SIMULATION*/

#define                     DLB_TASK_MAX_PRIO   250
/* Local routines */
static unsigned __TASKCONV  appDemoDlbWaTask(GT_VOID * param);  /* DLB WA task */
static GT_U32               dlbTaskCreated      = 0;            /* indication that task created */
static GT_TASK              l2DlbEventHandlerTid= (GT_TASK)0;   /* task id array for reset */
static GT_BOOL              dlbWaEnable         = GT_FALSE;
static GT_U32               dlbWaDelay          = 500;          /* 4 seconds for 8 control pipes */

/**
* @internal appdemoDlbWaEnable function
* @endinternal
*
* @brief   Enable/Disable DLB WA and creates a task to call waExecute periodically.
*
* @param[in] devNum                - device number
* @param[in] waEnable              - status of DLB WA
*                                    GT_TRUE     - DLB WA is enabled
*                                    GT_FALSE    - DLB WA is disabled
* @param[in] newDelay              - Interval between consecutive DLB WA call in milisec.
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
* @note none
*
*/
GT_STATUS appDemoDlbWaEnable
(
    GT_U8                               devNum,
    GT_BOOL                             waEnable,
    GT_U32                              newDelay
)
{
    GT_STATUS                       rc = GT_OK;
    char                            *name= "DLB_WA";/* The task name */
    GT_U32                          timer;

    /* Update the global flag to indicate the DLB wa status */
    dlbWaEnable = waEnable;

    /* Update delay */
    dlbWaDelay = newDelay;

    /* Return if no need to create task OR already created */
    if(((dlbWaEnable == GT_TRUE) && (dlbTaskCreated == 1)) ||
        (dlbWaEnable == GT_FALSE))
    {
        return rc;
    }

    /* Create task */
    if(l2DlbEventHandlerTid == 0)
    {
        dlbTaskCreated = 0;
        rc = osTaskCreate(name,
                DLB_TASK_MAX_PRIO,
                _32KB,
                appDemoDlbWaTask,
                (GT_VOID*)((GT_UINTPTR)devNum),
                &l2DlbEventHandlerTid);
        if (rc != GT_OK)
        {
            return GT_FAIL;
        }

        /* maximal time to wait - 100 Seconds */
        timer = 100000;
        while((dlbTaskCreated == 0) && (--timer))
        {
            /* wait for indication that task created */
            osTimerWkAfter(1);
        }
        if (!timer)
        {
            /* something went wrong with task */
            return GT_TIMEOUT;
        }
    }
    return rc;
}


/*******************************************************************************
* appDemoDlbWaTask
*
* DESCRIPTION:  App demo task for DLB WA.
*
* INPUTS:
*       param - The process data structure.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static unsigned __TASKCONV appDemoDlbWaTask
(
    GT_VOID * param
)
{
    GT_STATUS           rc = GT_OK;
    GT_U8               devNum = (GT_U8)((GT_UINTPTR)param);
    dlbTaskCreated      = 1;

    /* wait for indication that task created */
    osTimerWkAfter(1);

    while (1)
    {
        if(dlbWaEnable  == GT_TRUE)
        {
            rc = cpssDxChBrgL2DlbWaExecute(devNum);
            if(rc != GT_OK)
            {
                /* Reset the global params related to DLB before exiting */
                l2DlbEventHandlerTid    = 0;
                dlbWaEnable             = GT_FALSE;
                cpssOsPrintf("L2_DLB: appDemoDlbWaTask Exited\n");
                return rc;
            }
        }
        osTimerWkAfter(dlbWaDelay);
    }

    /* to avoid warnings in gcc */
    return rc;
}

typedef GT_STATUS (*TRACE_FUNC)(IN GT_BOOL enable);
extern GT_STATUS   falcon_dev_init_trace_bind( IN TRACE_FUNC trace_func);
extern void smiAccessTraceEnable(GT_BOOL enable);
GT_STATUS trace_appDemo_and_smi(IN GT_BOOL enable)
{
    smiAccessTraceEnable(enable);
    appDemoTraceHwAccessEnable(0,CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_BOTH_E,enable);
    return GT_OK;
}


/* open trace in start of hwsFalconStep0 and closed at the end of it */
GT_STATUS   falcon_dev_init_trace_enable(void)
{
    falcon_dev_init_trace_bind(trace_appDemo_and_smi);

    return GT_OK;
}
#if (defined ASIC_SIMULATION && !defined ASIC_SIMULATION_ENV_FORBIDDEN)
#include <gtExtDrv/drivers/gtDmaDrv.h>
#include <asicSimulation/SCIB/scib.h>
#include <cm3FileOps.h>
#define SUPER_IMAGE_NAME  "super_image_harrier.bin"
#define FW_IMAGE_NAME     "firmware_cm3_falcon.bin"/*do not really care regarding  FW for correct device ,
                                                                                                        lets take official Falcon*/

extern GT_VOID simCm3ImageFilePexAdressSet
(   IN GT_U32   addr_high,
    IN GT_U32   addr_low
);
extern GT_STATUS simCm3CurrentDeviceIdSet(IN GT_U32 deviceId/*the deviceIde as in the INI file*/);

static GT_U32   Uboot_wmDevNum = 0;
/* set the appDemo to work with uboot on specific CM3 (in multi-devices system) */
GT_STATUS appDemoUbootWmDevNumSet(IN GT_U32   wmDevNum)
{
    Uboot_wmDevNum = wmDevNum;

    simCm3CurrentDeviceIdSet(wmDevNum);

    return GT_OK;
}

static char wm_cm3FwFileName[50] = {0};
static GT_BOOL wm_cm3FwFile_set = GT_FALSE;
static char wm_cm3SupperImageFileName[50] = {0};
static GT_BOOL wm_cm3SupperImageFile_set = GT_FALSE;
GT_STATUS appDemoWmCm3FwFileNameSet
(
    IN char*    cm3FwFileName
)
{
    if(cm3FwFileName)
    {
        osStrCpy(wm_cm3FwFileName,cm3FwFileName);
        wm_cm3FwFile_set = GT_TRUE;
    }
    else
    {
        wm_cm3FwFile_set = GT_FALSE;
    }
    return GT_OK;
}
GT_STATUS appDemoWmCm3SupperImageFileNameSet
(
    IN char*    cm3SupperImageFileName
)
{
    if(cm3SupperImageFileName)
    {
        osStrCpy(wm_cm3SupperImageFileName,cm3SupperImageFileName);
        wm_cm3SupperImageFile_set = GT_TRUE;
    }
    else
    {
        wm_cm3SupperImageFile_set = GT_FALSE;
    }
    return GT_OK;
}


GT_STATUS appDemoUbootStart
(   IN GT_U32   addrHigh,
    IN GT_U32   addrLow
)
{
    GT_U32 ubootSize,i,j,fsize;
    GT_U32 offset,value,size;
    GT_CHAR_PTR tmp;
    FILE * fp = NULL;
    void * buffer = NULL;
    size_t sizeRead;
    char* fw_filename = wm_cm3FwFile_set ? &wm_cm3FwFileName[0] : FW_IMAGE_NAME;

    GT_CHAR_PTR uboot[] =
    {
     /*Open 1M iATU to MG0 at base address 0xf6400000*/
     "mw 0xf6001300 0;",
     "mw 0xf6001304 0x80000000;",
     "mw 0xf6001308 0xf6400000;",
     "mw 0xf600130c 0;",
     "mw 0xf6001310 0xf64fffff;",
     "mw 0xf6001314 0x3C200000;",/*MG0 base adress*/
     "mw 0xf6001318 0;",/*iATU*/
     /*In MG0 confi processor window settings, configure window #1: (Address decoding for CM3)*/
     "mw 0xf6400490 0x10000803;",  /*Base address for CM3*/
     "mw 0xf6400494 0x00ff0000;",    /*size 16MB*/
     "mw 0xf6400498 0x00000000;",
     "mw 0xf640049c 0xf200000e;",    /*remap address 0xf2000000 (higher 2G to access oATU)*/
     /*oATU to host CPU DDR with remap to 0x3000000, where u-boot should store the super image*/
     "mw 0xf6001200 0;",
     "mw 0xf6001204 0x80000000;",
     "mw 0xf6001208 0xf2000000;",
     "mw 0xf600120c 0;",
     "mw 0xf6001210 0xf23fffff;",
     "mw 0xf6001214 0x03000000;",
     "mw 0xf6001218 0;",
     /*Completion registers */
     "mw 0xf6400124 0x1;",
     "mw.l 0xf6400140 0x8102",
     /*Load MI FW to 0x2000000 by tftp, then copy to SRAM */
     "cp.l 0x2000000 0xf6480000 0x30000",
     /*Register 0x500 - read-modify-write gets you this value*/
     "mw 0xf6400500 0x300e1a80"
    };

    /*BAR 0/2 configuration in PCI config space*/
    value =0xF6000000;
    scibPciRegWrite(Uboot_wmDevNum,0x10/*BAR0 Low*/,1,&value);

#if __WORDSIZE == 64
    value =0x0;
    scibPciRegWrite(Uboot_wmDevNum,0x14/*BAR0 High*/,1,&value);
#endif

    value =0xF6400000;
    scibPciRegWrite(Uboot_wmDevNum,0x18/*BAR2 Low*/,1,&value);

#if __WORDSIZE == 64
    value =0x0;
    scibPciRegWrite(Uboot_wmDevNum,0x1C/*BAR2 High*/,1,&value);
#endif

    ubootSize = sizeof(uboot)/sizeof(uboot[0]);
    cpssOsPrintf("Loading uboot configuration with size %d lines.\n",ubootSize);
    for(i=0;i<ubootSize;i++)
    {
      cpssOsPrintf("Line %d : %s\n",i,uboot[i]);

      if(NULL!=cpssOsStrStr(uboot[i],"mw"))
      {
         tmp =cpssOsStrStr(uboot[i],"0x");
         if(tmp==NULL)
         {
            return GT_FAIL;
         }

         sscanf(tmp,"%x %x",&offset,&value);
         if(0xf6001214==offset)
         {
            value =addrLow;
         }
         else if(0xf6001218==offset)
         {
            value =addrHigh;
         }
         cpssOsPrintf("Write [adress 0x%x data 0x%x]\n",offset,value);

         ASIC_SIMULATION_WriteMemory(Uboot_wmDevNum, offset, 1, &value , offset);
      }
      else if(NULL!=cpssOsStrStr(uboot[i],"cp"))
      {
         tmp =cpssOsStrStr(uboot[i],"0x");
         if(tmp==NULL)
         {
            return GT_FAIL;
         }

         sscanf(tmp,"%x %x %x",&offset,&value,&size);

         cpssOsPrintf("Copy from adress 0x%x  to adress  0x%x [size %d bytes]\n",offset,value,size);

         fp = fopen(fw_filename,"rb");
         if(fp==NULL)
         {
             cpssOsPrintf("Error opening %s file\n",fw_filename);
             return GT_FAIL;
         }

         fseek(fp, 0, SEEK_END);
         fsize = ftell(fp);
         fseek(fp, 0, SEEK_SET);  /* rewind(f) */

         if(fsize>size)
         {
           cpssOsPrintf ("File size is more then copied size.File will be truncated.");
         }

         buffer = cpssOsMalloc(size);
         if(NULL==buffer)
         {
           cpssOsPrintf ("Allocation failure");
           return GT_FAIL;
         }

         cpssOsPrintf("Allocating  %d bytes in order to emulate  flash for %s \n",size,fw_filename);
         cpssOsPrintf("Actual file size %d\n",fsize);

         sizeRead = fread(buffer,fsize,1,fp);
         if (sizeRead != 1)
         {
           cpssOsPrintf ("Reading error");
           return GT_FAIL;
         }

         for(j=0;j<fsize/4;j++)
         {
            ASIC_SIMULATION_WriteMemory(0, value+4*j, 1, &(((GT_U32*)buffer)[j]), value+4*j);
            /*print last 5 words*/
            if(fsize/4-j<5)
            {
             cpssOsPrintf("addr [0x%x ] word [0x%x]\n",value+4*j,((GT_U32*)buffer)[j]);
            }
         }

         cpssOsPrintf("Copied %d words.First word is 0x%x ,last word is 0x%x \n",j,((GT_U32*)buffer)[0],((GT_U32*)buffer)[j-1]);

      }
      else
      {
        cpssOsPrintf("Line %d :Unknown command %s\n",i,uboot[i]);
      }
    }

    return GT_OK;
}

GT_STATUS appDemoLoadSuperImageToDdr
(
    GT_BOOL runUboot
)
{
   FILE * fp = NULL;
   long fsize,sizeToAllocate;
   void * buffer = NULL;
   size_t sizeRead;
   void * superImageStart = NULL;
   GT_STATUS rc;
   char* supperImage_filename = wm_cm3SupperImageFile_set ? &wm_cm3SupperImageFileName[0] : SUPER_IMAGE_NAME;

   {
       GT_U32   value = 0xabcdef99;
       GT_U32   deviceId = 1;
       /**/
       scibPciRegRead(deviceId,0x00,1,&value);
       if(value != 0xFFFFFFFF && value != 0xabcdef99)
       {
            /* patch for Harrier + Aldrin3M , where the Harrier is deviceId = 1 and not 0 */
            appDemoUbootWmDevNumSet(deviceId);
       }
   }

   fp = fopen(supperImage_filename,"rb");
   if(fp==NULL)
   {
       cpssOsPrintf("Error opening %s file\n",supperImage_filename);
       return GT_FAIL;
   }

   fseek(fp, 0, SEEK_END);
   fsize = ftell(fp);
   fseek(fp, 0, SEEK_SET);  /* rewind(f) */

   sizeToAllocate = fsize+HEADER_FLASH_OFFSET_FOR_CM3_USAGE+1;

   rc = extDrvDdrAllocate(sizeToAllocate,&buffer);
   if(rc!=GT_OK)
   {
       cpssOsPrintf("Error %d allocating  %d bytes in order to emulate  flash for %s \n",rc,sizeToAllocate,supperImage_filename);
       return rc;
   }

   /*super-image must reside 0x40000 after base offset*/

   superImageStart = (void *)((uintptr_t)buffer+HEADER_FLASH_OFFSET_FOR_CM3_USAGE);

   cpssOsMemSet(buffer,0,sizeToAllocate);

   cpssOsPrintf("Allocating  %d bytes in order to emulate  flash for %s \n",sizeToAllocate,supperImage_filename);

   sizeRead = fread(superImageStart,fsize,1,fp);
   if (sizeRead != 1)
   {
     cpssOsPrintf ("Reading error");
     return GT_FAIL;
   }
   /* *image_ptr_ptr = (BOOTONP_image_header_STC *)(buffer);*/
   cpssOsPrintf("DDR adress id %p\n",buffer);
   cpssOsPrintf("Super image adress is  %p\n",superImageStart);

   if(GT_TRUE==runUboot)
   {
#if __WORDSIZE == 64
           appDemoUbootStart(((uintptr_t)buffer)>>32,((uintptr_t)buffer)&0xFFFFFFFF);
#else
           appDemoUbootStart(0,(GT_U32)buffer);
#endif
   }

   fclose(fp);

   return GT_OK;

}
#endif

extern GT_STATUS debug_appDemoDxChFecEventPortCounterGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  APP_DEMO_FEC_COUNTER_TYPES_ENT  counterType,
    IN  GT_BOOL                         printHeader
);

/**
* @internal falcon_fecCountersValidation function
* @endinternal
*
* @brief   FEC counters validation
*
*/
GT_STATUS falcon_fecCountersValidation
(
    IN GT_U8 devNum,
    IN GT_U32 counterId
)
{
    GT_U32                      portNum, portMacNum;
    GT_U32                      ravenIndex, tileIndex, fecUnit, fecLaneBit;
    CPSS_DXCH_PORT_MAP_STC      portMap;
    GT_U32                      currentRegAddress, lastRegAddress;
    GT_U32                      regValue, currentBit;
    GT_U32                      ravenOffset, tileOffset;
    GT_U32                      fecVirtualLaneIndex, fecVirtualLanesNumbers;
    GT_STATUS                   rc;

    #define FALCON_MTI_PORT_BASE_MAC(unit, _tileId, _ravenId)                   \
        (0x00478000 + 0x80000*(unit))

    /* Global FEC CE - cause */
    #define FALCON_MTI_GLOBAL_FEC_CE_INTERRUPT_CAUSE_MAC(unit, _tileId, _ravenId) \
            (FALCON_MTI_PORT_BASE_MAC(unit,_tileId, _ravenId) + 0x3c)

    /* Global FEC NCE - cause */
    #define FALCON_MTI_GLOBAL_FEC_NCE_INTERRUPT_CAUSE_MAC(unit, _tileId, _ravenId) \
            (FALCON_MTI_PORT_BASE_MAC(unit,_tileId, _ravenId) + 0x44)

    /* Init values before cycle */
    lastRegAddress = 0;
    regValue = 0;

    /* Generate FEC interrupts for all existing ports */
    for(portNum = 0; portNum < appDemoPpConfigList[devNum].maxPortNumber ; portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);
        if (prvCpssDxChPortRemotePortCheck(devNum, portNum))
        {
            continue;
        }

        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, /*OUT*/&portMap);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPhysicalPortMapGet", rc);
        if (rc != GT_OK)
        {
            continue;
        }

        rc = appDemoDxChPortFecLaneIndexGet(devNum, portNum, &fecVirtualLaneIndex, &fecVirtualLanesNumbers);
        if (rc == GT_NOT_INITIALIZED)
        {
            continue;
        }
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        portMacNum  = portMap.interfaceNum;
        tileIndex   = portMacNum / 64;
        ravenIndex  = ((portMacNum / 16) % 4);
        /* Tiles 1,3 */
        if ((tileIndex % 2) != 0)
        {
            ravenIndex = 3 - ravenIndex;
        }
        fecUnit     = ((fecVirtualLaneIndex / 16) % 2);

        ravenOffset = prvCpssDxChHwUnitBaseAddrGet(devNum, (PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E + ravenIndex), NULL);
        tileOffset = prvCpssSip6TileOffsetGet(devNum, tileIndex);

        if (counterId == 0)
        {
            currentRegAddress = tileOffset + ravenOffset + FALCON_MTI_GLOBAL_FEC_CE_INTERRUPT_CAUSE_MAC(fecUnit, tileIndex, ravenIndex);
        }
        else
        {
            currentRegAddress = tileOffset + ravenOffset + FALCON_MTI_GLOBAL_FEC_NCE_INTERRUPT_CAUSE_MAC(fecUnit, tileIndex, ravenIndex);
        }

        /* Check if cause registers is ready to write */
        if ((lastRegAddress != 0) &&
            (lastRegAddress != currentRegAddress))
        {
            /* Write summary bit */
            regValue |= 0x1;
            cpssDrvPpHwRegisterWrite(CAST_SW_DEVNUM(devNum), 0, lastRegAddress, regValue);

            /* Reset value for next cause register */
            regValue = 0;

            cpssOsTimerWkAfter(50);
        }

        /* Set register data */
        currentBit = 1 << ((fecVirtualLaneIndex % 16) + 1);
        for (fecLaneBit = 0; fecLaneBit < fecVirtualLanesNumbers;  fecLaneBit+=2)
        {
            regValue |= currentBit << fecLaneBit;
        }

        /* Save the last register address */
        lastRegAddress = currentRegAddress;

    }

    /* Write last register */
    regValue |= 0x1;
    cpssDrvPpHwRegisterWrite(CAST_SW_DEVNUM(devNum), 0, lastRegAddress, regValue);

    cpssOsTimerWkAfter(5000);

    /* Read FEC counters for all existing ports*/
    for(portNum = 0; portNum < appDemoPpConfigList[devNum].maxPortNumber ; portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);
        if (prvCpssDxChPortRemotePortCheck(devNum, portNum))
        {
            continue;
        }

        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, /*OUT*/&portMap);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPhysicalPortMapGet", rc);
        if(rc != GT_OK)
        {
            continue;
        }

        rc = appDemoDxChPortFecLaneIndexGet(devNum, portNum, &fecVirtualLaneIndex, &fecVirtualLanesNumbers);
        if (rc == GT_NOT_INITIALIZED)
        {
            continue;
        }
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        rc = debug_appDemoDxChFecEventPortCounterGet(devNum, portNum,
                                                     (counterId == 0) ? APP_DEMO_FEC_CE_COUNTER_TYPE_E : APP_DEMO_FEC_NCE_COUNTER_TYPE_E,
                                                     (portNum == 0) ? GT_TRUE : GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}


/**
* @internal appDemoLpmRamConfigSet function
* @endinternal
*
* @brief  Set given list of device type and Shared memory configuration mode.
*         Should be called before cpssInitSystem().
*
* @param[in] lpmRamConfigInfoArray          - array of pairs: devType+Shared memory configuration mode
* @param[in] lpmRamConfigInfoNumOfElements  - number of valid pairs

* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on lpmRamConfigInfoNumOfElements bigger than array size
*/
GT_STATUS appDemoLpmRamConfigSet
(
    IN CPSS_DXCH_LPM_RAM_CONFIG_INFO_STC    lpmRamConfigInfoArray[CPSS_DXCH_CFG_NUM_OF_DEV_TYPES_MANAGED_CNS],
    IN GT_U32                               lpmRamConfigInfoNumOfElements
)
{
    GT_U32 i=0;

    CPSS_NULL_PTR_CHECK_MAC(lpmRamConfigInfoArray);

    if(lpmRamConfigInfoNumOfElements > CPSS_DXCH_CFG_NUM_OF_DEV_TYPES_MANAGED_CNS)
    {
        return GT_BAD_PARAM;
    }

    for (i=0;i<lpmRamConfigInfoNumOfElements;i++)
    {
        appDemoLpmRamConfigInfo[i].devType =  lpmRamConfigInfoArray[i].devType;
        appDemoLpmRamConfigInfo[i].sharedMemCnfg = lpmRamConfigInfoArray[i].sharedMemCnfg;
    }

    appDemoLpmRamConfigInfoNumOfElements = lpmRamConfigInfoNumOfElements;
    appDemoLpmRamConfigInfoSet = GT_TRUE;
    return GT_OK;
}

/**
* @internal appDemoLpmRamConfigGet function
* @endinternal
*
* @brief  Get given list of device type and Shared memory configuration mode.
*
* @param[out] lpmRamConfigInfoArray             - array of pairs: devType+Shared memory configuration mode
* @param[out] lpmRamConfigInfoNumOfElementsPtr  - (pointer to) number of valid pairs
* @param[out] lpmRamConfigInfoSetFlagPtr        - (pointer to) Flag specifying that the configuration was set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS appDemoLpmRamConfigGet
(
    OUT CPSS_DXCH_LPM_RAM_CONFIG_INFO_STC    lpmRamConfigInfoArray[CPSS_DXCH_CFG_NUM_OF_DEV_TYPES_MANAGED_CNS],
    OUT GT_U32                               *lpmRamConfigInfoNumOfElementsPtr,
    OUT GT_BOOL                              *lpmRamConfigInfoSetFlagPtr
)
{
    GT_U32 i=0;

    CPSS_NULL_PTR_CHECK_MAC(lpmRamConfigInfoArray);
    CPSS_NULL_PTR_CHECK_MAC(lpmRamConfigInfoNumOfElementsPtr);
    CPSS_NULL_PTR_CHECK_MAC(lpmRamConfigInfoSetFlagPtr);

    *lpmRamConfigInfoSetFlagPtr=appDemoLpmRamConfigInfoSet;

    if (appDemoLpmRamConfigInfoSet==GT_FALSE)
    {
        return GT_OK;
    }

    for (i = 0; i < appDemoLpmRamConfigInfoNumOfElements; i++)
    {
        lpmRamConfigInfoArray[i].devType=appDemoLpmRamConfigInfo[i].devType;
        lpmRamConfigInfoArray[i].sharedMemCnfg=appDemoLpmRamConfigInfo[i].sharedMemCnfg;
    }

    *lpmRamConfigInfoNumOfElementsPtr = appDemoLpmRamConfigInfoNumOfElements;

    return GT_OK;
}

/**
* @internal dbgLedTestTask function
* @endinternal
*
* @brief  Logic of register data monitoring task.
*         It's used for LED raw data register monitoring and calculation
*         of positive pulse duration (time when registers data > 0)
*
* @param[in] param             - address of register to monitor
*
* @retval GT_OK                 - on success
* @retval GT_FAIL               - on fail to create task
*/
static unsigned __TASKCONV dbgLedTestTask
(
    GT_VOID *param
)
{
    GT_U32      regAddr = (GT_U32)(GT_UINTPTR) param;
    GT_U32      data;
    GT_U8       devNum = 0;
    GT_U32      ii;         /*iterator*/
    GT_STATUS   rc = GT_OK; /*return code*/

    cpssOsPrintf("\ndbgLedTestTask: Started for Read reg 0x%08X\n", regAddr);

    while (1)
    {
        osTimerWkAfter(1);

        rc = cpssDrvPpHwRegisterRead(devNum,0,regAddr,&data);
        if (rc != GT_OK)
        {
            cpssOsPrintf("\ndbgLedTestTask: Read reg fail: 0x%08X rc %d\n", regAddr, rc);
            return 1;
        }

        if (data)
        {
            for (ii = 0; ii < 300000; ii++)
            {
                rc = cpssDrvPpHwRegisterRead(devNum,0,regAddr,&data);
                if (rc != GT_OK)
                {
                    cpssOsPrintf("\ndbgLedTestTask: Read reg fail: 0x%08X rc %d\n", regAddr, rc);
                    return 1;
                }

                 if (data == 0)
                 {
                     cpssOsPrintf("\ndbgLedTestTask: Signal Duration %d mili\n", ii);

                     break;
                 }
                 osTimerWkAfter(1);
            }

            if (ii == 300000)
            {
                cpssOsPrintf("\ndbgLedTestTask: Signal Duration too long\n");
                return 3;
            }
        }

    }

    cpssOsPrintf("\ndbgLedTestTask: Abort\n");
    return 0;
}

/**
* @internal dbgLedTest function
* @endinternal
*
* @brief  Debug function that creates register data monitoring task.
*         It's used for LED raw data register monitoring and calculation
*         of positive pulse duration (time when registers data > 0)
*
* @param[in] regAddr             - address of register to monitor
*
* @retval GT_OK                 - on success
* @retval GT_FAIL               - on fail to create task
*/
GT_STATUS  dbgLedTest(GT_U32 regAddr)
{
    GT_STATUS rc;
    GT_TASK   taskId;

    rc = cpssOsTaskCreate("led_test_task",
                      200,
                     _8KB,
                     dbgLedTestTask,
                     (GT_VOID*)((GT_UINTPTR)(regAddr)),
                     &taskId );
    if(rc != GT_OK)
    {
        cpssOsPrintf("spawn led_test_task fail \n");
    }
    else
    {
        cpssOsPrintf("spawn led_test_task OK for register 0x%08X\n", regAddr);
    }
    return rc;

}

/**
* @internal falcon_CatchUpSecondDevice function
* @endinternal
*
* @brief   the complementary operations that need to be done after the first device
*          finished the 'HS' , and second device can start its initialization.
*
* @param[in] devIndex                 - the device index
* @param[in] devNum                   - the SW devNum
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS gtDbFalconBoardReg_deviceSimpleInit
(
    IN  GT_U8  devIndex,
    IN  GT_U8  boardRevId
)
{
    GT_STATUS rc;

    /* increment the number of devices as we insert it */
    /* and the logic use loop of the updated number    */
    appDemoPpConfigDevAmount++;

    /* state the single device to work on */
    appDemoSysConfig.appDemoActiveDeviceBmp = 1 << devIndex;
    appDemoDbEntryAdd("appDemoActiveDeviceBmp", appDemoSysConfig.appDemoActiveDeviceBmp);
    isUnderAddDeviceCatchup = GT_TRUE;

    rc = gtDbFalconBoardReg_AllDevicesSimpleInit(boardRevId,GT_FALSE);

    appDemoSysConfig.appDemoActiveDeviceBmp = 0x0;/* reset the indications */
    appDemoDbEntryAdd("appDemoActiveDeviceBmp", appDemoSysConfig.appDemoActiveDeviceBmp);
    isUnderAddDeviceCatchup = GT_FALSE;

    return rc;
}

