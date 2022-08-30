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
* @file gtDbDxHawk.c
*
* @brief Initialization functions for the Hawk - SIP6.10 - board.
*
* @version   1
********************************************************************************
*/

#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>
#include <appDemo/userExit/userEventHandler.h>
#include <gtExtDrv/drivers/gtUartDrv.h>
#ifdef INCLUDE_UTF
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#endif /* INCLUDE_UTF */
#include <gtOs/gtOsExc.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <appDemo/utils/mainUT/appDemoUtils.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#ifdef INCLUDE_MPD
#include <appDemo/phy/gtAppDemoPhyConfig.h>
/*static GT_U32 mpdIfIndexArr[512];
CPSS_OS_MUTEX  xsmi_extn_mtx;*/
#endif

/* offset used during HW device ID calculation formula */
extern GT_U8 appDemoHwDevNumOffset;
/* cpssInitSystem has been run */
extern GT_BOOL systemInitialized;
/* port manager boolean variable */
extern GT_BOOL portMgr;

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

extern GT_STATUS userForceBoardType(IN GT_U32 boardType);

#define PRINT_SKIP_DUE_TO_DB_FLAG(reasonPtr , flagNamePtr) \
        cpssOsPrintf("NOTE: '%s' skipped ! (due to flag '%s') \n",  \
            reasonPtr , flagNamePtr)

#define ARR_PTR_AND_SIZE_MAC(myArr , arrPtr , arrLen) \
    arrPtr = &myArr[0];                               \
    arrLen = sizeof(myArr)/sizeof(myArr[0])

#define UNUSED_PARAM_MAC(x) x = x

#define CPU_PORT    CPSS_CPU_PORT_NUM_CNS

extern GT_STATUS appDemoTrace_skipTrace_onAddress(
    IN GT_U32   address,
    IN GT_U32   index /* up to 16 indexes supported 0..15 */
);

extern GT_STATUS phoenix_phyInit
(
    IN  GT_U8  devNum,
    IN CPSS_PHY_SMI_INTERFACE_ENT smiInterface
);

/**
* @internal prvAppDemoHawkSerdesConfigSet function
* @endinternal
*
* @brief   Per DB/RD board configure the Tx/Rx Polarity values on Serdeses
*
* @note   APPLICABLE DEVICES: AC5P.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Harrier; Ironman; xCat3; AC5.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvAppDemoHawkSerdesConfigSet
(
    IN GT_U8 devNum,
    IN GT_U8 boardRevId
)
{
    GT_STATUS                           rc;
    GT_U32                              i;
    GT_U32                              polarityArraySize;
    APPDEMO_SERDES_LANE_POLARITY_STC    *currentPolarityArrayPtr;
    CPSS_PP_DEVICE_TYPE                 devType;
    CPSS_DXCH_CFG_DEV_INFO_STC          devInfo;
    CPSS_PORT_MAC_TO_SERDES_STC         *currentMacToSerdesMapPtr;
    GT_U32                              ac5p_MacToSerdesMap_arrSize;
    GT_U32                              portNum, portMacNum;
    CPSS_DXCH_PORT_MAP_STC              portMap;

    static APPDEMO_SERDES_LANE_POLARITY_STC  ac5p_DB_PolarityArray[] =
    {
        /* laneNum  invertTx    invertRx */
         { 0,   GT_TRUE ,   GT_TRUE   }
        ,{ 1,   GT_FALSE,   GT_TRUE   }
        ,{ 2,   GT_FALSE,   GT_FALSE  }
        ,{ 3,   GT_FALSE,   GT_TRUE   }
        ,{ 4,   GT_FALSE,   GT_TRUE   }
        ,{ 5,   GT_TRUE ,   GT_FALSE  }
        ,{ 6,   GT_TRUE ,   GT_FALSE  }
        ,{ 7,   GT_TRUE ,   GT_FALSE  }
        ,{ 8,   GT_TRUE ,   GT_TRUE   }
        ,{ 9,   GT_FALSE,   GT_FALSE  }
        ,{ 10,  GT_FALSE,   GT_FALSE  }
        ,{ 11,  GT_TRUE ,   GT_TRUE   }
        ,{ 12,  GT_TRUE ,   GT_TRUE   }
        ,{ 13,  GT_FALSE,   GT_FALSE  }
        ,{ 14,  GT_FALSE,   GT_FALSE  }
        ,{ 15,  GT_FALSE,   GT_TRUE   }
        ,{ 16,  GT_TRUE ,   GT_TRUE   }
        ,{ 17,  GT_FALSE,   GT_FALSE  }
        ,{ 18,  GT_FALSE,   GT_TRUE   }
        ,{ 19,  GT_FALSE,   GT_TRUE   }
        ,{ 20,  GT_FALSE,   GT_TRUE   }
        ,{ 21,  GT_TRUE ,   GT_FALSE  }
        ,{ 22,  GT_TRUE ,   GT_TRUE   }
        ,{ 23,  GT_TRUE ,   GT_FALSE  }
        ,{ 24,  GT_FALSE,   GT_FALSE  }
        ,{ 25,  GT_FALSE,   GT_TRUE   }
        ,{ 26,  GT_FALSE,   GT_FALSE  }
        ,{ 27,  GT_FALSE,   GT_TRUE   }
        ,{ 28,  GT_FALSE,   GT_TRUE   }
        ,{ 29,  GT_FALSE,   GT_TRUE   }
        ,{ 30,  GT_FALSE,   GT_TRUE   }
        ,{ 31,  GT_FALSE,   GT_FALSE  }
    };

    static APPDEMO_SERDES_LANE_POLARITY_STC  ac5p_RD_PolarityArray[] =
    {
        /* laneNum  invertTx    invertRx */
          { 0,   GT_FALSE,   GT_FALSE   }
         ,{ 1,   GT_FALSE,   GT_FALSE   }
         ,{ 2,   GT_FALSE,   GT_FALSE   }
         ,{ 3,   GT_FALSE,   GT_FALSE   }
         ,{ 4,   GT_FALSE,   GT_FALSE   }
         ,{ 5,   GT_FALSE,   GT_FALSE   }
         ,{ 6,   GT_FALSE,   GT_FALSE   }
         ,{ 7,   GT_FALSE,   GT_FALSE   }
         ,{ 8,   GT_FALSE,   GT_FALSE   }
         ,{ 9,   GT_FALSE,   GT_FALSE   }
         ,{ 10,  GT_FALSE,   GT_FALSE   }
         ,{ 11,  GT_FALSE,   GT_FALSE   }
         ,{ 12,  GT_FALSE,   GT_FALSE   }
         ,{ 13,  GT_FALSE,   GT_FALSE   }
         ,{ 14,  GT_FALSE,   GT_FALSE   }
         ,{ 15,  GT_FALSE,   GT_FALSE   }
         ,{ 16,  GT_FALSE,   GT_FALSE   }
         ,{ 17,  GT_TRUE ,   GT_TRUE    }
         ,{ 18,  GT_FALSE,   GT_TRUE    }
         ,{ 19,  GT_FALSE,   GT_FALSE   }
         ,{ 20,  GT_FALSE,   GT_TRUE    }
         ,{ 21,  GT_FALSE,   GT_FALSE   }
         ,{ 22,  GT_TRUE ,   GT_TRUE    }
         ,{ 23,  GT_TRUE ,   GT_TRUE    }
         ,{ 24,  GT_TRUE ,   GT_FALSE   }
         ,{ 25,  GT_FALSE,   GT_TRUE    }
         ,{ 26,  GT_FALSE,   GT_FALSE   }
         ,{ 27,  GT_FALSE,   GT_TRUE    }
         ,{ 28,  GT_FALSE,   GT_FALSE   }
         ,{ 29,  GT_FALSE,   GT_FALSE   }
         ,{ 30,  GT_TRUE ,   GT_FALSE   }
         ,{ 31,  GT_FALSE,   GT_TRUE    }
    };

    static CPSS_PORT_MAC_TO_SERDES_STC  ac5p_DB_MacToSerdesMap[] =
    {
        {{0,6,1,7,2,4,3,5}},    /* SD: 0-7    MAC: 0...25  */
        {{3,2,0,1,7,6,4,5}},    /* SD: 8-15   MAC: 26..51  */
        {{7,1,6,0,5,3,4,2}},    /* SD: 16-23  MAC: 52..77  */
        {{0,6,1,7,2,4,3,5}},    /* SD: 24-31  MAC: 78..103 */
    };

    static CPSS_PORT_MAC_TO_SERDES_STC  ac5p_RD_MacToSerdesMap[] =
    {
        {{0,1,2,3,4,5,6,7}},    /* SRDS: 0-7    MAC: 0...25  */
        {{0,1,2,3,4,5,6,7}},    /* SRDS: 8-15   MAC: 26..51  */
        {{5,0,4,1,6,7,3,2}},    /* SRDS: 16-23  MAC: 52..77  */
        {{1,6,2,5,0,7,3,4}},    /* SRDS: 24-31  MAC: 78..103 */
    };

    rc = cpssDxChCfgDevInfoGet(devNum, &devInfo);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChCfgDevInfoGet", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    devType = devInfo.genDevInfo.devType;

    switch(devType)
    {
    case CPSS_AC5P_ALL_DEVICES_CASES_MAC:
        if (boardRevId == 2) /* RD board */
        {
            currentPolarityArrayPtr = ac5p_RD_PolarityArray;
            polarityArraySize = sizeof(ac5p_RD_PolarityArray)/sizeof(APPDEMO_SERDES_LANE_POLARITY_STC);
            currentMacToSerdesMapPtr = ac5p_RD_MacToSerdesMap;
            ac5p_MacToSerdesMap_arrSize = sizeof(ac5p_RD_MacToSerdesMap) / sizeof(CPSS_PORT_MAC_TO_SERDES_STC);
        }
        else /* DB board */
        {
            currentPolarityArrayPtr = ac5p_DB_PolarityArray;
            polarityArraySize = sizeof(ac5p_DB_PolarityArray)/sizeof(APPDEMO_SERDES_LANE_POLARITY_STC);
            currentMacToSerdesMapPtr = ac5p_DB_MacToSerdesMap;
            ac5p_MacToSerdesMap_arrSize = sizeof(ac5p_DB_MacToSerdesMap) / sizeof(CPSS_PORT_MAC_TO_SERDES_STC);

        }
            break;

        default:
            return GT_OK;
    }
    for (i = 0; i < polarityArraySize; i++)
    {
        rc = cpssDxChPortSerdesLanePolaritySet(devNum, 0,
                currentPolarityArrayPtr[i].laneNum,
                currentPolarityArrayPtr[i].invertTx,
                currentPolarityArrayPtr[i].invertRx);

        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortSerdesLanePolaritySet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /**************************/
    /* handle 'Serdes muxing' */
    /**************************/
#if 0
    for(i = 0 ; i < ac5p_MacToSerdesMap_arrSize ; i++)
    {
        rc = cpssDxChPortLaneMacToSerdesMuxSet(CAST_SW_DEVNUM(devNum), i*26, &ac5p_DB_MacToSerdesMap[i]);
        if (rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortLaneMacToSerdesMuxSet", rc);
            return rc;
        }
    }
#endif

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
        if(ac5p_MacToSerdesMap_arrSize <= (portMacNum/26))
        {
            break;
        }
        if((portMacNum % 26) != 0)
        {
            continue;
        }
        rc = cpssDxChPortLaneMacToSerdesMuxSet(devNum,portNum,&currentMacToSerdesMapPtr[portMacNum/26]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }


    return GT_OK;
}


#define PORT_NUMBER_ARR_SIZE    9
typedef struct
{
    GT_PHYSICAL_PORT_NUM                portNumberArr  [PORT_NUMBER_ARR_SIZE];
    GT_U32                              globalDmaNumber[PORT_NUMBER_ARR_SIZE];
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT     mappingType;
    GT_BOOL                             reservePreemptiveChannel;
}HAWK_PORT_MAP_STC;

/* the physical port numbers are restricted to 0..74 due to 'my physical ports' table (TTI unit) */
/* mapping of Hawk physical ports to MAC ports (and TXQ ports). */
static HAWK_PORT_MAP_STC hawk_port_mode[] =
{
    /* DP[1] ports */
     {{ 0, 1, 2, 6,10,14,18,22 ,GT_NA} , {26,27,28,32,36,40,44,48 ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}
        /* USX_1 part */
    ,{{ 3, 4, 5, 7, 8, 9, GT_NA} ,       { 29, 30, 31, 33, 34, 35, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
    ,{{ 11, 12, 13, 15, 16, 17, GT_NA} , { 37, 38, 39, 41, 42, 43, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
    ,{{ 19, 20, 21, 23, 24, 25, GT_NA} , { 45, 46, 47, 49, 50, 51, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}

    /* DP[0] ports */
    ,{{26,27,28,32,36,40,44,48 ,GT_NA} , { 0, 1, 2, 6,10,14,18,22 ,GT_NA}, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}
        /* USX_0 part */
    ,{{ 29, 30, 31, 33, 34, 35, GT_NA} , { 3, 4, 5, 7, 8, 9, GT_NA} ,       CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
    ,{{ 37, 38, 39, 41, 42, 43, GT_NA} , { 11, 12, 13, 15, 16, 17, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
    ,{{ 45, 46, 47, 49, 50, 51, GT_NA} , { 19, 20, 21, 23, 24, 25, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}

    /* DP[3] ports - part 1 */
    ,{{52,53,54,55,56,57,58,59 ,GT_NA} , {78,79,80,84,88,92,96,100,GT_NA}, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}

     /*reserved for preemption
    ,{{61,75,76,77,78,79,80,81,GT_NA} , {57,61,55,59,63,67,71,75,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}*/

     /* skip physical 62 */
    ,{{CPU_PORT/*63*/          ,GT_NA} , {106/*DMA*/              ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E ,GT_FALSE   }
    /* DP[2] ports */
    ,{{64,65,66,67,68,69,70,71 ,GT_NA} ,  {52,53,54,58,62,66,70,74 ,GT_NA}, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}
      /*reserved for preemption
    ,{{82,83,84,85,86,87,88,89,GT_NA} , {83,87,81,85,89,93,97,101,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}*/
     /* map 'CPU network port' */
    ,{{72                      ,GT_NA} , {105/*MAC*/              ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
     /* map 'CPU SDMA ports' */
    ,{{73,74,60                ,GT_NA} , {107,108,109/*DMA*/      ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,GT_FALSE    }
};
static GT_U32   actualNum_hawk_port_mode = sizeof(hawk_port_mode)/sizeof(hawk_port_mode[0]);

static HAWK_PORT_MAP_STC hawk_port_mode_rpp[] =
{
    /* DP[0] ports */
     {{ 1, 2, 6,10,14,18,22 ,GT_NA} , {1, 2, 6,10,14,18,22 ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}
     /*cascade*/
     ,{{0 ,GT_NA} , {0 ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
     /* RPP */
    ,{{ 101, 102, 103, 104, 105, 106, GT_NA} ,       { 0, 0, 0, 0, 0, 0, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,GT_FALSE}


    /* DP[1] ports */
    ,{{26,27,28,32,36,40,44,48 ,GT_NA} , {26,27,28,32,36,40,44,48 ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}
        /* USX_1 part */
    ,{{ 29, 30, 31, 33, 34, 35, GT_NA}  , { 29, 30, 31, 33, 34, 35, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
    ,{{ 37, 38, 39, 41, 42, 43, GT_NA}  , { 37, 38, 39, 41, 42, 43, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
    ,{{ 45, 46, 47, 49, 50, 51, GT_NA}  , { 45, 46, 47, 49, 50, 51, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}

    /* DP[2] ports - part 1 */
    ,{{52,53,54,55,56,57,58,59 ,GT_NA} , {52,53,54,58,62,66,70,74 ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}

     /*reserved for preemption
    ,{{61,75,76,77,78,79,80,81,GT_NA} , {57,61,55,59,63,67,71,75,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}*/

     /* skip physical 62 */
    ,{{CPU_PORT/*63*/          ,GT_NA} , {106/*DMA*/              ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E ,GT_FALSE   }
    /* DP[3] ports */
    ,{{64,65,66,67,68,69,70,71 ,GT_NA} , {78,79,80,84,88,92,96,100,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}
      /*reserved for preemption
    ,{{82,83,84,85,86,87,88,89,GT_NA} , {83,87,81,85,89,93,97,101,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}*/
     /* map 'CPU network port' */
    ,{{72                      ,GT_NA} , {105/*MAC*/              ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
     /* map 'CPU SDMA ports' */
    ,{{73,74,60                ,GT_NA} , {107,108,109/*DMA*/      ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,GT_FALSE    }
};
static GT_U32   actualNum_hawk_port_mode_rpp = sizeof(hawk_port_mode_rpp)/sizeof(hawk_port_mode_rpp[0]);


static HAWK_PORT_MAP_STC hawk_AC5P_port_mode[] =
{
    /* DP[1] ports */
     {{ 0, 1, 2, 6,10,14,18,22 ,GT_NA} , {26,27,28,32,36,40,44,48 ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}
        /* USX_1 part */
    ,{{ 3, 4, 5, 7, 8, 9, GT_NA} ,       { 29, 30, 31, 33, 34, 35, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
    ,{{ 11, 12, 13, 15, 16, 17, GT_NA} , { 37, 38, 39, 41, 42, 43, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
    ,{{ 19, 20, 21, 23, 24, 25, GT_NA} , { 45, 46, 47, 49, 50, 51, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}

    /* DP[0] ports */
    ,{{26,27,28,32,36,40,44,48 ,GT_NA} , { 0, 1, 2, 6,10,14,18,22 ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}
        /* USX_0 part */
    ,{{ 29, 30, 31, 33, 34, 35, GT_NA} , { 3, 4, 5, 7, 8, 9, GT_NA} ,       CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
    ,{{ 37, 38, 39, 41, 42, 43, GT_NA} , { 11, 12, 13, 15, 16, 17, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
    ,{{ 45, 46, 47, 49, 50, 51, GT_NA} , { 19, 20, 21, 23, 24, 25, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}

     /* DP[2] ports */
    ,{{52,53,54,55,56,57,58,59 ,GT_NA} , { 52,53,54,58,62,66,70,74,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}

     /* skip physical 62 */
    ,{{CPU_PORT/*63*/          ,GT_NA} , {106/*DMA*/              ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E ,GT_FALSE   }
     /* map 'CPU network port' */
    ,{{72                      ,GT_NA} , {105/*MAC*/              ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
     /* map 'CPU SDMA ports' */
    ,{{73,74,60                ,GT_NA} , {107,108,109/*DMA*/      ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,GT_FALSE    }
};
static GT_U32   actualNum_hawk_AC5P_port_mode = sizeof(hawk_AC5P_port_mode)/sizeof(hawk_AC5P_port_mode[0]);

static HAWK_PORT_MAP_STC hawk_armstrong_2S_port_mode[] =
{
    /* DP[1] ports */
     {{ 0, 1, 2, 6,10,14,18,22 ,GT_NA} , {26,27,28,32,36,40,44,48 ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}

    /* DP[0] ports */
    ,{{26,27,28,32,36,40,44,48 ,GT_NA} , { 0, 1, 2, 6,10,14,18,22 ,GT_NA}, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}

    /* DP[3] ports - part 1 */
    ,{{52,53,54,55,56,57,58,59 ,GT_NA} , {78,79,80,84,88,92,96,100,GT_NA}, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}

     /* skip physical 62 */
    ,{{CPU_PORT/*63*/          ,GT_NA} , {106/*DMA*/              ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E ,GT_FALSE   }

    /* DP[2] ports */
    ,{{64,65,66,67,68,69,70,71 ,GT_NA} ,  {52,53,54,58,62,66,70,74 ,GT_NA}, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}

    /* map 'CPU network port' */
    ,{{72                      ,GT_NA} , {105/*MAC*/              ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
     /* map 'CPU SDMA ports' */
    ,{{73,74,60                ,GT_NA} , {107,108,109/*DMA*/      ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,GT_FALSE    }
};
static GT_U32   actualNum_hawk_armstrong_2S_port_mode = sizeof(hawk_armstrong_2S_port_mode)/sizeof(hawk_armstrong_2S_port_mode[0]);

static HAWK_PORT_MAP_STC hawk_Aldrin3_port_mode[] =
{
    /* DP[1] ports */
     {{ 0, 1, 2, 6,10,14,18,22 ,GT_NA} , {26,27,28,32,36,40,44,48 ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}

    /* DP[0] ports */
    ,{{26,27,28,32,36,40,44,48 ,GT_NA} , { 0, 1, 2, 6,10,14,18,22 ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}

     /* DP[2] ports */
    ,{{52,53,54,55,56,57,58,59 ,GT_NA} , { 52,53,54,58,62,66,70,74,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}

     /* skip physical 62 */
    ,{{CPU_PORT/*63*/          ,GT_NA} , {106/*DMA*/              ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E ,GT_FALSE   }
     /* map 'CPU network port' */
    ,{{72                      ,GT_NA} , {105/*MAC*/              ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
     /* map 'CPU SDMA ports' */
    ,{{73,74,60                ,GT_NA} , {107,108,109/*DMA*/      ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,GT_FALSE    }
};
static GT_U32   actualNum_hawk_Aldrin3_port_mode = sizeof(hawk_Aldrin3_port_mode)/sizeof(hawk_Aldrin3_port_mode[0]);

static HAWK_PORT_MAP_STC hawk_Aldrin3_QSGMII_port_mode[] =
{
    /* DP[1] ports */
     {{ 0, 1, 2, 6,10,14,18,22 ,GT_NA} , {26,27,28,32,36,40,44,48 ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}
        /* USX_1 part */
    ,{{  3,  4,  5, GT_NA} , { 38, 37, 39, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
    ,{{ 11, 12, 13, GT_NA} , { 42, 41, 43, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}

    /* DP[0] ports */
    ,{{26,27,28,32,36,40,44,48 ,GT_NA} , { 0, 1, 2, 6,10,14,18,22 ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}
        /* USX_0 part */
    ,{{ 29, 30, 31, GT_NA} , { 12, 11, 13, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
    ,{{ 37, 38, 39, GT_NA} , { 16, 15, 17, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}

     /* DP[2] ports */
    ,{{52,53,54,55,56,57,58,59 ,GT_NA} , { 52,53,54,58,62,66,70,74,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}

     /* skip physical 62 */
    ,{{CPU_PORT/*63*/          ,GT_NA} , {106/*DMA*/              ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E ,GT_FALSE   }
     /* map 'CPU network port' */
    ,{{72                      ,GT_NA} , {105/*MAC*/              ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
     /* map 'CPU SDMA ports' */
    ,{{73,74                   ,GT_NA} , {107,108/*DMA*/      ,GT_NA} ,     CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,GT_FALSE    }
};
static GT_U32   actualNum_hawk_Aldrin3_QSGMII_port_mode = sizeof(hawk_Aldrin3_QSGMII_port_mode)/sizeof(hawk_Aldrin3_QSGMII_port_mode[0]);

static HAWK_PORT_MAP_STC hawk_Aldrin3_XL_port_mode[] =
{
    /* DP[1] ports */
     {{ 0, 1, 2, 6,10,14,18,22 ,GT_NA} , {26,27,28,32,36,40,44,48 ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}

    /* DP[0] ports */
    ,{{26,27,28,32,36,40,44,48 ,GT_NA} , { 0, 1, 2, 6,10,14,18,22 ,GT_NA}, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}

    /* DP[3] ports - part 1 */
    ,{{52,53,54,55,56,57,58,59 ,GT_NA} , {78,79,80,84,88,92,96,100,GT_NA}, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}

     /* skip physical 62 */
    ,{{CPU_PORT/*63*/          ,GT_NA} , {106/*DMA*/              ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E ,GT_FALSE   }

    /* DP[2] ports */
    ,{{64,65,66,67,68,69,70,71 ,GT_NA} ,  {52,53,54,58,62,66,70,74 ,GT_NA}, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}
     /* map 'CPU network port' */
    ,{{72                      ,GT_NA} , {105/*MAC*/              ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
     /* map 'CPU SDMA ports' */
    ,{{73,74,60                ,GT_NA} , {107,108,109/*DMA*/      ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,GT_FALSE    }
};
static GT_U32   actualNum_hawk_Aldrin3_XL_port_mode = sizeof(hawk_Aldrin3_XL_port_mode)/sizeof(hawk_Aldrin3_XL_port_mode[0]);

static HAWK_PORT_MAP_STC hawk_Aldrin3_XL_continuous_port_mode[] =
{
    /* DP[1] ports */
     {{ 0, 1, 2, 3, 4, 5, 6, 7 ,GT_NA} , {26,27,28,32,36,40,44,48 ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}

    /* DP[0] ports */
    ,{{ 8, 9,10,11,12,13,14,15 ,GT_NA} , { 0, 1, 2, 6,10,14,18,22 ,GT_NA}, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}

    /* DP[3] ports - part 1 */
    ,{{16,17,18,19,20,21,22,23 ,GT_NA} , {78,79,80,84,88,92,96,100,GT_NA}, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}

    /* DP[2] ports */
    ,{{24,25,26,27,28,29,30,31 ,GT_NA} ,  {52,53,54,58,62,66,70,74 ,GT_NA}, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
     /* map 'CPU network port' */
    ,{{32                      ,GT_NA} , {105/*MAC*/              ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}

     /* skip physical 62 */
    ,{{CPU_PORT/*63*/          ,GT_NA} , {106/*DMA*/              ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E ,GT_FALSE   }
 };
static GT_U32   actualNum_hawk_Aldrin3_continuous_XL_port_mode = sizeof(hawk_Aldrin3_XL_continuous_port_mode)/sizeof(hawk_Aldrin3_XL_continuous_port_mode[0]);

static HAWK_PORT_MAP_STC hawk_AC5P_RD_port_mode[] =
{
    /* USX_1 */
     {{  0, 1, 2, 3, 4, 5, 6, 7, GT_NA} , { 30, 28, 31, 29, 34, 32, 35, 33 ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
    ,{{  8, 9,10,11,12,13,14,15, GT_NA} , { 38, 36, 39, 37, 42, 40, 43, 41 ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
    ,{{ 16,17,18,19,20,21,22,23, GT_NA} , { 46, 44, 47, 45, 50, 48, 51, 49 ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
    /* USX_0 */
    ,{{ 24,25,26,27,28,29,30,31, GT_NA} , { 20, 18, 21, 19, 24, 22, 25, 23 ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
    ,{{ 32,33,34,35,36,37,38,39, GT_NA} , { 16, 14, 17, 15, 12, 10, 13, 11 ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
    ,{{ 40,41,42,43,44,45,46,47, GT_NA} , {  8,  6,  9,  7,  4,  2,  5,  3 ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}

    /* DP[1] ports */
    /* DP[0] ports */
    ,{{48,49,50,51 ,GT_NA} , { 27, 26, 1, 0 ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}

     /* DP[2] ports */
    ,{{52,53,54,55,56,57,58,59 ,GT_NA} , {78,79,80,84,88,92,96,100,GT_NA}, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}
    /* DP[3] ports */
    ,{{64,65,66,67,68,69,70,71 ,GT_NA} , {52,53,54,58,62,66,70,74 ,GT_NA}, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_TRUE}

     /* skip physical 62 */
    ,{{CPU_PORT/*63*/          ,GT_NA} , {106/*DMA*/              ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E ,GT_FALSE   }
     /* map 'CPU network port' */
#if 0
    ,{{72                      ,GT_NA} , {105/*MAC*/              ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,GT_FALSE}
#endif
     /* map 'CPU SDMA ports' */
    ,{{73,74,60                ,GT_NA} , {107,108,109/*DMA*/      ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,GT_FALSE    }
};
static GT_U32   actualNum_hawk_AC5P_RD_port_mode = sizeof(hawk_AC5P_RD_port_mode)/sizeof(hawk_AC5P_RD_port_mode[0]);

/***************************************************************/
/* flag to use the MAX number of macs that the device supports */
/* AND max SDMAs                                               */
/***************************************************************/
static GT_U32   cpuPortMuxUseSdma = 0; /* default mode - CPU network port */

typedef struct{
    GT_U32  sdmaPort;
    GT_U32  macPort;
}MUX_DMA_STC;
static MUX_DMA_STC hawkCpuMuxInfo[1] = {{107,105}};

/* the number of CPU SDMAs that we need resources for */
static GT_U32   numCpuSdmas = 0;

#define MAX_MAPPED_PORTS_CNS    128
static  GT_U32  actualNum_cpssApi_hawk_defaultMap = 0;/*actual number of valid entries in cpssApi_hawk_defaultMap[] */
static CPSS_DXCH_PORT_MAP_STC   cpssApi_hawk_defaultMap[MAX_MAPPED_PORTS_CNS];
    /* filled at run time , by info from falcon_3_2_defaultMap[] */

static PortInitList_STC *hawk_force_PortsInitListPtr = NULL;

/* Hawk ports in 50G mode */
static PortInitList_STC hawk_portInitlist[] =
{
    /* 16 ports in range of 0..51 - in 50G */
     { PORT_LIST_TYPE_LIST    ,  {0, 1, 2, 6,10,14,18,22,26,27,28,32,36,40,44,48, APP_INV_PORT_CNS}, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* 8 ports in range of 52..59 - in 50G */
    ,{ PORT_LIST_TYPE_INTERVAL,  {52, 59, 1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* 8 ports in range of 64..71 - in 50G */
    ,{ PORT_LIST_TYPE_INTERVAL,  {64, 71, 1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
     /* port 72 - 'CPU port' (reduced MAC) in 10G */
    ,{ PORT_LIST_TYPE_LIST    ,  {72,        APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* Hawk ports in 50G mode */
static PortInitList_STC hawk_portInitlist_rpp[] =
{
    /* 16 ports in range of 0..51 - in 50G */
     { PORT_LIST_TYPE_LIST    ,  {0, 1, 2, 6,10,14,18,22,26,27,28,32,36,40,44,48, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* 16 ports in range of 52..71 - in 50G */
    ,{ PORT_LIST_TYPE_INTERVAL,  {52,60,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {62,71,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {101,106,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,  CPSS_PORT_INTERFACE_MODE_REMOTE_E}
     /* port 72 - 'CPU port' (reduced MAC) in 10G */
    ,{ PORT_LIST_TYPE_LIST    ,  {72,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* Hawk AC5P ports in 50G mode */
static PortInitList_STC hawk_AC5P_portInitlist[] =
{
    /* 16 ports in range of 0..48 - in 50G */
     { PORT_LIST_TYPE_LIST    ,  {0, 1, 2, 6,10,14,18,22,26,27,28,32,36,40,44,48, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* 8 ports in range of 52..59 - in 50G */
    ,{ PORT_LIST_TYPE_INTERVAL,  {52,59,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
     /* port 72 - 'CPU port' (reduced MAC) in 10G */
    ,{ PORT_LIST_TYPE_LIST    ,  {72,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* Hawk AC5P ports in 25G mode */
static PortInitList_STC hawk_Aldrin3_portInitlist[] =
{
    /* 16 ports in range of 0..48 - in 25G */
     { PORT_LIST_TYPE_LIST    ,  {0, 1, 2, 6,10,14,18,22,26,27,28,32,36,40,44,48, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* 8 ports in range of 52..59 - in 25G */
    ,{ PORT_LIST_TYPE_INTERVAL,  {52,59,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
     /* port 72 - 'CPU port' (reduced MAC) in 10G */
    ,{ PORT_LIST_TYPE_LIST    ,  {72,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* Hawk AC5P ports in 25G mode */
static PortInitList_STC hawk_Aldrin3_XL_portInitlist[] =
{
    /* 16 ports in range of 0..48 - in 25G */
     { PORT_LIST_TYPE_LIST    ,  {0, 1, 2, 6,10,14,18,22,26,27,28,32,36,40,44,48, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* 8 ports in range of 52..59 - in 25G */
    ,{ PORT_LIST_TYPE_INTERVAL,  {52,59,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* 8 ports in range of 64..71 - in 25G */
    ,{ PORT_LIST_TYPE_INTERVAL,  {64, 71, 1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* port 72 - 'CPU port' (reduced MAC) in 10G */
    ,{ PORT_LIST_TYPE_LIST    ,  {72,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};
static PortInitList_STC hawk_Aldrin3_XL_continuous_portInitlist[] =
{
     /* 32 ports in range of  0..31 - in 25G */
     { PORT_LIST_TYPE_INTERVAL,  { 0,31,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    /* port 32 - 'CPU port' (reduced MAC) in 10G */
    ,{ PORT_LIST_TYPE_LIST    ,  {32,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

static PortInitList_STC hawk_AC5P_RD_portInitlist[] =
{
    /* 2 ports in range of 0..1 - in 50G */
     { PORT_LIST_TYPE_LIST    ,  {48, 49, 50, 51, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

     /* 8 ports in range of 52..59 - in 50G */
    ,{ PORT_LIST_TYPE_INTERVAL,  {0,31,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_1000_E,  CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E}

     /* 8 ports in range of 52..59 - in 50G */
    ,{ PORT_LIST_TYPE_INTERVAL,  {32, 47, 1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_5000_E,  CPSS_PORT_INTERFACE_MODE_USX_20G_QXGMII_E}

    /* 8 ports in range of 52..59 - in 50G */
    ,{ PORT_LIST_TYPE_INTERVAL,  {52,59,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {64,71,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
#if 0
    /* port 72 - 'CPU port' (reduced MAC) in 10G */
    ,{ PORT_LIST_TYPE_LIST    ,  {72,     APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
#endif
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/* fill array of cpssApi_hawk_defaultMap[] and update actualNum_cpssApi_hawk_defaultMap and numCpuSdmas */
static GT_STATUS fillDbForCpssPortMappingInfo(IN GT_U32 devNum)
{
    GT_STATUS   rc;
    HAWK_PORT_MAP_STC *currAppDemoInfoPtr;
    CPSS_DXCH_PORT_MAP_STC *currCpssInfoPtr;
    GT_U32  ii,jj;
    GT_U32 numEntries;
    GT_U32 maxPhyPorts;
    GT_U32 interfaceNum;
    GT_U32 value;
    CPSS_PP_DEVICE_TYPE     devType;
    GT_U32 currBoardType;

    if(actualNum_cpssApi_hawk_defaultMap)
    {
        /* already initialized */
        return GT_OK;
    }

    rc = appDemoBoardTypeGet(&currBoardType);
    if (rc != GT_OK)
    {
        return rc;
    }

    devType = PRV_CPSS_PP_MAC(devNum)->devType;
    maxPhyPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);


    if ( currBoardType ==  APP_DEMO_XCAT5P_A0_BOARD_RD_CNS)
    {
        currAppDemoInfoPtr = &hawk_AC5P_RD_port_mode[0];
        numEntries = actualNum_hawk_AC5P_RD_port_mode;
        hawk_force_PortsInitListPtr = hawk_AC5P_RD_portInitlist;
    }
    else if ( (appDemoDbEntryGet("rppMap", &value) == GT_OK) && (value != 0) )
    {
         currAppDemoInfoPtr = &hawk_port_mode_rpp[0];
         numEntries = actualNum_hawk_port_mode_rpp;
         hawk_force_PortsInitListPtr = hawk_portInitlist_rpp;
    }
    else if ( (appDemoDbEntryGet("continuousMap", &value) == GT_OK) && (value != 0) )
    {
         currAppDemoInfoPtr = &hawk_Aldrin3_XL_continuous_port_mode[0];
         numEntries = actualNum_hawk_Aldrin3_continuous_XL_port_mode;
         hawk_force_PortsInitListPtr = hawk_Aldrin3_XL_continuous_portInitlist;
    }
    else
    {
        switch (devType)
        {
            case CPSS_98DX4504_CNS:
            case CPSS_98DX4504M_CNS:
            case CPSS_98DX4510_CNS:
            case CPSS_98DX4510M_CNS:
            case CPSS_98DX4510_H_CNS:
            case CPSS_98DX4510M_H_CNS:
            case CPSS_98DX4530_CNS:
            case CPSS_98DX4530M_CNS:
            case CPSS_98DX4550_CNS:
            case CPSS_98DX4550M_CNS:
            case CPSS_98DX4550_H_CNS:
            case CPSS_98DX4550M_H_CNS:
                currAppDemoInfoPtr = &hawk_AC5P_port_mode[0];
                numEntries = actualNum_hawk_AC5P_port_mode;
                break;
            case CPSS_98EX5604_CNS:
            case CPSS_98EX5604M_CNS:
                currAppDemoInfoPtr = &hawk_armstrong_2S_port_mode[0];
                numEntries = actualNum_hawk_armstrong_2S_port_mode;
                break;
            case CPSS_98DX7324_CNS:
            case CPSS_98DX7324M_CNS:
            case CPSS_98DX7325_CNS:
            case CPSS_98DX7325M_CNS:
                currAppDemoInfoPtr = &hawk_Aldrin3_port_mode[0];
                numEntries = actualNum_hawk_Aldrin3_port_mode;
                break;
            case CPSS_98DXC725_CNS:
                currAppDemoInfoPtr = &hawk_Aldrin3_QSGMII_port_mode[0];
                numEntries = actualNum_hawk_Aldrin3_QSGMII_port_mode;
                break;
            case CPSS_98DX7332_CNS:
            case CPSS_98DX7332M_CNS:
            case CPSS_98DX7335_CNS:
            case CPSS_98DX7335M_CNS:
            case CPSS_98DX7335_H_CNS:
            case CPSS_98DX7335M_H_CNS:
                currAppDemoInfoPtr = &hawk_Aldrin3_XL_port_mode[0];
                numEntries = actualNum_hawk_Aldrin3_XL_port_mode;
                break;
            default:
                currAppDemoInfoPtr = &hawk_port_mode[0];
                numEntries = actualNum_hawk_port_mode;
        }
    }

    cpssOsMemSet(cpssApi_hawk_defaultMap,0,sizeof(cpssApi_hawk_defaultMap));

    currCpssInfoPtr    = &cpssApi_hawk_defaultMap[0];

    for (ii = 0 ; ii < numEntries; ii++ , currAppDemoInfoPtr++)
    {
        for(jj = 0 ; currAppDemoInfoPtr->portNumberArr[jj] != GT_NA ; jj++ , currCpssInfoPtr++)
        {
            if(actualNum_cpssApi_hawk_defaultMap >= maxPhyPorts)
            {
                rc = GT_FULL;
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssApi_hawk_defaultMap is FULL, maxPhy port ", rc);
                return rc;
            }

            interfaceNum = currAppDemoInfoPtr->globalDmaNumber[jj];

            if(((hawkCpuMuxInfo[0].sdmaPort == interfaceNum) && (cpuPortMuxUseSdma == 0)) ||
               ((hawkCpuMuxInfo[0].macPort  == interfaceNum) && (cpuPortMuxUseSdma == 1)))
            {
                /* skip this MAC/SDMA , because it is not according to the CPU SDMA mux mode */
                currCpssInfoPtr--;
                continue;
            }
            currCpssInfoPtr->physicalPortNumber = currAppDemoInfoPtr->portNumberArr[jj];
            currCpssInfoPtr->mappingType        = currAppDemoInfoPtr->mappingType;
            currCpssInfoPtr->interfaceNum       = interfaceNum;
            currCpssInfoPtr->txqPortNumber      = 0;/* CPSS will give us 8 TC queues per port */
            currCpssInfoPtr->reservePreemptiveChannel = currAppDemoInfoPtr->reservePreemptiveChannel;


            if(currAppDemoInfoPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                if(PRV_CPSS_PP_MAC(devNum)->isGmDevice && currCpssInfoPtr->physicalPortNumber != CPU_PORT)
                {
                    /* skip this SDMA - the GM currently supports single SDMA */
                    currCpssInfoPtr--;
                    continue;
                }

                numCpuSdmas++;
            }

            actualNum_cpssApi_hawk_defaultMap++;
        }
    }

    return GT_OK;
}

/**
* @internal hawk_getBoardInfo function
* @endinternal
*
* @brief   Return the board configuration info.
*
* @param[in] firstDev                 - GT_TRUE - first device, GT_FALSE - next device
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
static GT_STATUS hawk_getBoardInfo
(
    IN  GT_BOOL          firstDev,
    OUT CPSS_HW_INFO_STC *hwInfoPtr,
    OUT GT_PCI_INFO      *pciInfo
)
{
    GT_STATUS   rc;

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
    appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].pciInfo = *pciInfo;

    rc = extDrvPexConfigure(
            pciInfo->pciBusNum,
            pciInfo->pciIdSel,
            pciInfo->funcNo,
            MV_EXT_DRV_CFG_FLAG_EAGLE_E,
            hwInfoPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("extDrvPexConfigure", rc);

    appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].hwInfo = *hwInfoPtr;

    return rc;
}

/**
* @internal hawk_getBoardInfoSimple function
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
static GT_STATUS hawk_getBoardInfoSimple
(
    OUT CPSS_HW_INFO_STC *hwInfoPtr,
    OUT GT_PCI_INFO *pciInfo,
    OUT GT_U32 *numOfDevicesPtr
)
{
    GT_STATUS   rc;
    GT_U32      numOfDevices;

    rc = hawk_getBoardInfo(GT_TRUE/*firstDev*/, hwInfoPtr++, pciInfo);
    if (rc != GT_OK) {return rc;}

    numOfDevices = 1;

    /* In our case we want to find just one prestera device on PCI bus*/
    while (GT_OK == hawk_getBoardInfo(GT_FALSE/*firstDev*/, hwInfoPtr++, pciInfo))
    {
        numOfDevices++;
        if (numOfDevices == APP_DEMO_CPSS_MAX_NUM_PORT_GROUPS_CNS)
        {
            /* this is max number of supported devices defined by hwInfoPtr array size */
            break;
        }
    }

    osPrintf("Recognized [%d] devices found on PCI bus!\n", numOfDevices);

    *numOfDevicesPtr = numOfDevices;
    return GT_OK;
}

#ifdef ASIC_SIMULATION
extern GT_U32 simCoreClockOverwrittenGet(GT_VOID);
#endif

/**
* @internal hawk_boardTypePrint function
* @endinternal
*
* @brief   This function prints type of Hawk board and Cider version.
*
* @param[in] boardName                - board name
* @param[in] devName                  - device name
*                                       none
*/
static GT_VOID hawk_boardTypePrint
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
/*    cpssOsPrintf("Using Cider Version [%s] \n", ciderVersionName);*/

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

    UNUSED_PARAM_MAC(boardRevId);

    phase1Params->devNum = CAST_SW_DEVNUM(devNum);

/* retrieve PP Core Clock from HW */
    phase1Params->coreClock             = CPSS_DXCH_AUTO_DETECT_CORE_CLOCK_CNS;

    phase1Params->ppHAState             = CPSS_SYS_HA_MODE_ACTIVE_E;


    phase1Params->numOfPortGroups       = 1;

    /* Eagle access mode */
    phase1Params->mngInterfaceType      = CPSS_CHANNEL_PEX_EAGLE_E;

    phase1Params->maxNumOfPhyPortsToUse = 128;/* single mode of 128 physical ports ,
                                            so supports value 128 or 0 */

    phase1Params->serdesRefClock        = CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E;

    /* add Db Entry */
    rc = appDemoDxHwPpPhase1DbEntryInit(CAST_SW_DEVNUM(devNum),phase1Params,
        CPSS_98DX4504_CNS ,/* dummy to represent 'hawk' family (one of falcon values) */
        NULL,NULL,NULL);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxHwPpPhase1DbEntryInit", rc);
    if(rc != GT_OK)
        return rc;

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
    IN GT_SW_DEV_NUM       devNum,
    IN  CPSS_PP_DEVICE_TYPE         devType,
    OUT CPSS_DXCH_PP_PHASE2_INIT_INFO_STC  *phase2Params
)
{
    GT_STATUS                   rc;
    CPSS_PP_PHASE2_INIT_PARAMS  appDemo_ppPhase2Params;
    GT_U32                      tmpData;
    GT_U32                      auDescNum,fuDescNum;
    GT_U32 rxDescNum = (RX_DESC_NUM_DEF / appDemoPpConfigDevAmount);
    GT_U32 txDescNum = (TX_DESC_NUM_DEF / appDemoPpConfigDevAmount);

    appDemoPhase2DefaultInit(&appDemo_ppPhase2Params);
    osMemSet(phase2Params,0,sizeof(CPSS_DXCH_PP_PHASE2_INIT_INFO_STC));

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
    }

    /* allow each tile with it's own number of AUQ size */
    auDescNum = AU_DESC_NUM_DEF;
    /* allow each pipe with it's own number of FUQ size (for CNC upload) */
    /* NOTE: Fdb Upload still happen only on 'per tile' base (like AUQ) ...
       it is not per pipe triggered */
    fuDescNum = AU_DESC_NUM_DEF;

    phase2Params->newDevNum                  = CAST_SW_DEVNUM(devNum);
    phase2Params->noTraffic2CPU = GT_FALSE;
    phase2Params->netifSdmaPortGroupId = 0;
    phase2Params->auMessageLength = CPSS_AU_MESSAGE_LENGTH_8_WORDS_E;

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

    appDemoCpssCurrentDevIndex = devNum;

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

/**
* @internal hawk_initPortMappingStage function
* @endinternal
*
*/
static GT_STATUS hawk_initPortMappingStage
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
        case CPSS_AC5P_ALL_DEVICES_CASES_MAC:
        default:
            ARR_PTR_AND_SIZE_MAC(cpssApi_hawk_defaultMap, mapArrPtr, mapArrLen);
            mapArrLen = actualNum_cpssApi_hawk_defaultMap;
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

    /* it's for appdemo DB only */
    rc = appDemoDxChMaxMappedPortSet(CAST_SW_DEVNUM(devNum), mapArrLen, mapArrPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxChMaxMappedPortSet", rc);

    if (mapUpdatedArrPtr != NULL)
    {
        cpssOsFree(mapUpdatedArrPtr);
    }
    return rc;
}

/**
* @internal hawk_PortModeConfiguration function
* @endinternal
*
*/
static GT_STATUS hawk_PortModeConfiguration
(
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PP_DEVICE_TYPE      devType
)
{
    GT_STATUS   rc;
    PortInitList_STC *portInitListPtr;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery;

    UNUSED_PARAM_MAC(devType);

    if(hawk_force_PortsInitListPtr)
    {
        /* allow to 'FORCE' specific array by other logic */
        portInitListPtr = hawk_force_PortsInitListPtr;
    }
    else
    {
        switch (devType)
        {
            case CPSS_98DX4504_CNS:
            case CPSS_98DX4504M_CNS:
            case CPSS_98DX4510_CNS:
            case CPSS_98DX4510M_H_CNS:
            case CPSS_98DX4510_H_CNS:
            case CPSS_98DX4510M_CNS:
            case CPSS_98DX4530_CNS:
            case CPSS_98DX4530M_CNS:
            case CPSS_98DX4550_CNS:
            case CPSS_98DX4550M_CNS:
            case CPSS_98DX4550_H_CNS:
            case CPSS_98DX4550M_H_CNS:
                portInitListPtr = hawk_AC5P_portInitlist;
                break;
            case CPSS_98DX7324_CNS:
            case CPSS_98DX7324M_CNS:
            case CPSS_98DX7325_CNS:
            case CPSS_98DX7325M_CNS:
            case CPSS_98DXC725_CNS:
                portInitListPtr = hawk_Aldrin3_portInitlist;
                break;
            case CPSS_98DX7332_CNS:
            case CPSS_98DX7332M_CNS:
            case CPSS_98DX7335_CNS:
            case CPSS_98DX7335M_CNS:
            case CPSS_98DX7335_H_CNS:
            case CPSS_98DX7335M_H_CNS:
                portInitListPtr = hawk_Aldrin3_XL_portInitlist;
                break;
            default:
                portInitListPtr = hawk_portInitlist;
        }
    }

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E == system_recovery.systemRecoveryProcess)
    {
        rc = appDemoBc2PortListInit(CAST_SW_DEVNUM(devNum),portInitListPtr,GT_TRUE);
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

/**
* @internal hawk_appDemoDbUpdate function
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
static GT_STATUS hawk_appDemoDbUpdate
(
    IN GT_U32             devIndex,
    IN GT_SW_DEV_NUM      devNum,
    IN CPSS_PP_DEVICE_TYPE     devType
)
{
    appDemoPpConfigList[devIndex].valid = GT_TRUE;
    appDemoPpConfigList[devIndex].devNum = CAST_SW_DEVNUM(devNum);
    appDemoPpConfigList[devIndex].deviceId = devType;
    appDemoPpConfigList[devIndex].devFamily = CPSS_PP_FAMILY_DXCH_AC5P_E;
    appDemoPpConfigList[devIndex].apiSupportedBmp = APP_DEMO_DXCH_FUNCTIONS_SUPPORT_CNS;  CPSS_TBD_BOOKMARK_AC5P

    return GT_OK;
}

/**
* @internal hawk_initPhase1AndPhase2 function
* @endinternal
*
*/
static GT_STATUS hawk_initPhase1AndPhase2
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
    GT_U32                  boardType;
    static CPSS_DXCH_IMPLEMENT_WA_ENT falcon_WaList[] =
    {
        /* no current WA that need to force CPSS */
        CPSS_DXCH_IMPLEMENT_WA_LAST_E
    };

    osMemSet(&cpssPpPhase1Info, 0, sizeof(cpssPpPhase1Info));
    osMemSet(&cpssPpPhase2Info, 0, sizeof(cpssPpPhase2Info));

    switch(boardRevId)
    {
        case 1:
            boardType = APP_DEMO_XCAT5P_A0_BOARD_DB_CNS;
            break;
        case 2:
            boardType = APP_DEMO_XCAT5P_A0_BOARD_RD_CNS;
            break;
        default:
            boardType = APP_DEMO_XCAT5P_A0_BOARD_DB_CNS;
            break;
    }

    userForceBoardType(boardType);

    rc = appDemoDxChBoardTypeGet(SYSTEM_DEV_NUM_MAC(0), boardRevId, &bc2BoardType);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = getPpPhase1ConfigSimple(devNum, boardRevId, &cpssPpPhase1Info);
    if (rc != GT_OK)
        return rc;


    cpssPpPhase1Info.hwInfo[0] = *hwInfoPtr;

    /* devType is retrieved in hwPpPhase1Part1*/
    rc = cpssDxChHwPpPhase1Init(&cpssPpPhase1Info, &devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpPhase1Init", rc);
    if (rc != GT_OK)
        return rc;

    *devTypePtr = devType;

    /* check if the device did HW reset (soft/hard) */
    appDemoDxChCheckIsHwDidHwReset(devNum);

    /* update the appDemo DB */
    rc = hawk_appDemoDbUpdate(devIndex, devNum , devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("hawk_appDemoDbUpdate", rc);
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

    rc = hawk_initPortMappingStage(devNum,devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("hawk_initPortMappingStage", rc);
    if (GT_OK != rc)
        return rc;

    /* memory related data, such as addresses and block lenghts, are set in this funtion*/
    rc = getPpPhase2ConfigSimple(devNum, devType, &cpssPpPhase2Info);
    if (rc != GT_OK)
        return rc;

    rc = cpssDxChHwPpPhase2Init(CAST_SW_DEVNUM(devNum),  &cpssPpPhase2Info);
    if (rc != GT_OK)
        return rc;

    hwDevNum = (devNum + appDemoHwDevNumOffset) & 0x3FF;

    /* set HWdevNum related values */
    rc = appDemoDxHwDevNumChange(CAST_SW_DEVNUM(devNum), hwDevNum);
    if (GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal hawk_getNeededLibs function
* @endinternal
*
*/
static GT_STATUS hawk_getNeededLibs(
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
* @internal hawk_initPpLogicalInit function
* @endinternal
*
*/
static GT_STATUS hawk_initPpLogicalInit
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
* @internal hawk_appDemoInitSequence function
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
static GT_STATUS hawk_appDemoInitSequence
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
    GT_U32      tmpData;

    if (appDemoDbEntryGet("portMgr", &tmpData) != GT_OK)
    {
        portMgr = GT_FALSE;
    }
    else
    {
        portMgr = (tmpData == 1) ? GT_TRUE : GT_FALSE;
    }

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
#ifdef GM_USED
    else
    {
        /* Force Hawk GM to use value that allow traffic test to pass ok */
        rc = prvCpssDrvDebugDeviceIdSet(CAST_SW_DEVNUM(devNum),CPSS_98DX9410_CNS/*0x941011AB*/);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvCpssDrvDebugDeviceIdSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
#endif /*GM_USED*/

    rc = hawk_initPhase1AndPhase2(devNum,boardRevId,hwInfoPtr,devIndex,&devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("hawk_initPhase1AndPhase2", rc);
    if (GT_OK != rc)
        return rc;

    if (appDemoInitRegDefaults != GT_FALSE)
    {
        return GT_OK;
    }

    if(!cpssDeviceRunCheck_onEmulator())
    {
        rc = prvAppDemoHawkSerdesConfigSet(CAST_SW_DEVNUM(devNum),boardRevId);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvAppDemoHawkSerdesConfigSet", rc);
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

        rc = appDemoPortManagerTaskCreate(CAST_SW_DEVNUM(devNum));
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoPortManagerTaskCreate", rc);
        if (rc != GT_OK)
            return rc;
    }

#if !defined(ASIC_SIMULATION) && defined(INCLUDE_MPD)
    if ( boardRevId == 2 /* RD */ )
    {
            rc = gtAppDemoPhyMpdInit(devNum,boardRevId);  /* pass 0 means switch chip #0  */
            if (rc != GT_OK)
                return rc;
            CPSS_ENABLER_DBG_TRACE_RC_MAC("RD_AC5P_MPD_initPhys", rc);
    }
#endif

    rc = hawk_PortModeConfiguration(devNum,devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("hawk_PortModeConfiguration", rc);
    if (rc != GT_OK)
        return rc;

    rc = hawk_getNeededLibs(&libInitParams,&sysConfigParams);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("hawk_getNeededLibs", rc);
    if(rc != GT_OK)
        return rc;

    rc = hawk_initPpLogicalInit(devNum,&sysConfigParams.lpmRamMemoryBlocksCfg);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("hawk_initPpLogicalInit", rc);
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

#if 0
    CPSS_TBD_BOOKMARK_AC5P /* cpss not ready yet */
    rc = appDemoB2PtpConfig(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoB2PtpConfig", rc);
    if(rc != GT_OK)
        return rc;
#endif /*0*/


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
    cpssOsPrintf("NOTE: the Hawk HW crash the gtShutdownAndCoreRestart(...) \n");
#endif /* ASIC_SIMULATION */

    rc = appDemoB2PtpConfig(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoB2PtpConfig", rc);
    if(rc != GT_OK)
        return rc;
    /* not support 'system reset' */
    rc = cpssDevSupportSystemResetSet(CAST_SW_DEVNUM(devNum), supportSystemReset);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDevSupportSystemResetSet", rc);
    if (GT_OK != rc)
        return rc;

    return GT_OK;
}

/**
* @internal hawk_EventHandlerInit function
* @endinternal
*
*/
static GT_STATUS hawk_EventHandlerInit
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32 value;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */

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
        if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E ||
            system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E)
        {
            PRINT_SKIP_DUE_TO_DB_FLAG("appDemoEventRequestDrvnModeInit ","system recovery in HA/HS process");
            return GT_OK;
        }

        rc = appDemoEventRequestDrvnModeInit();
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEventRequestDrvnModeInit", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

extern GT_STATUS   appDemoFalconMainUtForbidenTests(void);
/*
    MainUT tests:
    list of tests/suites that forbidden because crash the CPSS or the device
   (fatal error in simulation on unknown address)
*/
static GT_STATUS   appDemoHawkMainUtForbidenTests(void)
{
#ifdef IMPL_TGF
    static FORBIDEN_TESTS mainUt_forbidenTests_FatalError [] =
    {
         TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagPortGroupTablePrint       )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhyXsmiMdcDivisionFactorSet )
        ,{NULL}/* must be last */
    };

    /* suites that most or all tests CRASH or FATAL ERROR or Fail ! */
    static FORBIDEN_SUITES mainUt_forbidenSuites_CRASH_FAIL [] =
    {

         SUITE_NAME(cpssDxChDiagBist           )/* lib is not aligned yet */
        ,SUITE_NAME(cpssDxChPortSyncEther      )/* lib is not aligned yet */
        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS enhancedUt_forbidenTests_Emulator_LongerThanOneMinute [] =
    {
         TEST_NAME(tgfIOamKeepAliveFlowHashVerificationBitSelection)
        ,TEST_NAME(tgfPortTxSchedulerSp)
        ,TEST_NAME(tgfPortTxTailDropDbaPortResources)
        ,TEST_NAME(prvTgfBrgVplsBasicTest1)
        ,TEST_NAME(tgfBasicTrafficSanity)
        ,TEST_NAME(tgfBasicDynamicLearning_fromCascadePortDsaTagForward)
        ,TEST_NAME(tgfVlanManipulationEthernetOverMpls)
        ,TEST_NAME(prvTgfFdbIpv4UcPointerRouteByIndex)
        ,TEST_NAME(prvTgfFdbIpv4UcPointerRouteByMsg)
        ,TEST_NAME(prvTgfFdbIpv4UcRoutingLookupMask)
        ,TEST_NAME(prvTgfFdbIpv4UcRoutingAgingEnable)
        ,TEST_NAME(prvTgfFdbIpv6UcRoutingLookupMask)
        ,TEST_NAME(prvTgfFdbIpv6UcRoutingAgingEnable)
        ,TEST_NAME(prvTgfBrgGenMtuCheck)
        ,TEST_NAME(tgfBasicIpv4UcEcmpRouting)
        ,TEST_NAME(tgfBasicIpv4UcEcmpRoutingRandomEnable)
        ,TEST_NAME(tgfBasicIpv4UcEcmpRoutingIndirectAccess)
        ,TEST_NAME(prvTgfIpv4NonExactMatchUcRouting)
        ,TEST_NAME(prvTgfIpv4NonExactMatchMcRouting)
        ,TEST_NAME(tgfIpv4UcRoutingHiddenPrefixes)
        ,TEST_NAME(tgfBasicIpv4UcEcmpRoutingHashCheck)
        ,TEST_NAME(prvTgfPclMetadataPacketType)
        ,TEST_NAME(prvTgfPclMetadataPacketTypeIpOverMpls)
        ,TEST_NAME(prvTgfIngressMirrorEportVsPhysicalPort)
        ,TEST_NAME(prvTgfEgressMirrorEportVsPhysicalPort)
        ,TEST_NAME(tgfTrunkSaLearning)
        ,TEST_NAME(tgfTrunkSaLearning_globalEPortRepresentTrunk)
        ,TEST_NAME(tgfTrunkSimpleHashMacSaDa)
        ,TEST_NAME(tgfTrunkSimpleHashMacSaDa_globalEPortRepresentTrunk)
        ,TEST_NAME(tgfTrunkWithRemoteMembers)
        ,TEST_NAME(tgfTrunkCrcHashMaskPriority)
        ,TEST_NAME(tgfTrunkCascadeTrunk)
        ,TEST_NAME(tgfTrunkDesignatedTableModes)
        ,TEST_NAME(tgfTrunkCascadeTrunkWithWeights)
        ,TEST_NAME(tgfTrunkSortMode)
        ,TEST_NAME(tgfTrunkCrcHash_liteMode_CRC32_cpssPredictEgressPort)
        ,TEST_NAME(tgfTrunk_80_members_flood_EPCL)
        ,TEST_NAME(tgfTrunkPacketTypeHashModeTest)
        ,TEST_NAME(tgfIpfixSamplingToCpuTest)
        ,TEST_NAME(tgfIpfixAlarmEventsTest)
        ,TEST_NAME(prvTgfUnucUnregmcBcFiltering)
        ,TEST_NAME(prvTgfEgressInterface)
        ,TEST_NAME(tgfPortTxSip6QcnQueuePool0ResourcesWithoutIngressHdr)
        ,TEST_NAME(tgfPortTxSip6QcnQueuePool1ResourcesWithoutIngressHdr)
        ,TEST_NAME(tgfPortTxSip6QcnQueuePool0ResourcesWithIngressHdr)
        ,TEST_NAME(tgfPortTxSip6QcnQueuePool1ResourcesWithIngressHdr)
        ,TEST_NAME(tgfTunnelTermEtherOverMplsPwLabelParallelLookup)
        ,TEST_NAME(prvTgfTunnelTermPbrDualLookup)
        ,{NULL}/* must be last */
    };

#endif /*IMPL_TGF*/


    /* tests that defined forbidden by Hawk */

#ifdef IMPL_TGF
    appDemoForbidenTestsAdd (mainUt_forbidenTests_FatalError);
    appDemoForbidenSuitesAdd(mainUt_forbidenSuites_CRASH_FAIL);

    if (cpssDeviceRunCheck_onEmulator())
    {
        /* very slow suites */
        utfAddPreSkippedRule("cpssDxChBridgeFdbManager","*","","");
        utfAddPreSkippedRule("tgfBridgeFdbManager","*","","");

        {
            GT_U32 skip = 0;
            appDemoDbEntryGet("hawkOnEmulatorSkipSlowEnhancedUt", &skip);
            if (skip)
            {
                appDemoForbidenTestsAdd(
                    enhancedUt_forbidenTests_Emulator_LongerThanOneMinute);
            }
        }
    }
#endif /*IMPL_TGF*/

    /* add tests that defined forbidden by Falcon */
    return appDemoFalconMainUtForbidenTests();
}

/**
* @internal hawk_localUtfInit function
* @endinternal
*
*/
static GT_STATUS hawk_localUtfInit
(
    IN GT_SW_DEV_NUM      devNum
)
{
    GT_STATUS rc = GT_OK;

#ifdef INCLUDE_UTF

    /* Initialize unit tests for CPSS */
    rc = utfPreInitPhase();
    if (rc != GT_OK)
    {
        utfPostInitPhase(rc);
        return rc;
    }

    rc = utfInit(CAST_SW_DEVNUM(devNum));
    if (rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("utfInit", rc);
        utfPostInitPhase(rc);
        return rc;
    }

    utfPostInitPhase(rc);
#else
    devNum = devNum;
#endif /* INCLUDE_UTF */

    appDemoHawkMainUtForbidenTests();

    return rc;
}

#ifndef GM_USED

static APP_DEMO_LED_UNIT_CONFIG_STC ac5p_XL_50G_led_indications[CPSS_LED_UNITS_MAX_NUM_CNS] =
{
    {64, 71,    GT_FALSE},     /* 8 LED ports (LED Unit 0 - PIPE1 400G)     */
    { 0,  0,    GT_TRUE },     /* Not used    (LED Unit 1 - PIPE1 USX  )    */
    {64, 64+3,  GT_FALSE},     /* 1 LED port  (LED Unit 2 - PIPE1 CPU PORT) + 3 dummy indication */
    {64, 71,    GT_FALSE},     /* 8 LED ports (LED Unit 3 - PIPE0 400G)     */
    { 0,  0,    GT_TRUE },     /* Not used    (LED Unit 4 - PIPE0 USX)      */
    {64, 71,    GT_FALSE},     /* 8 LED ports (LED Unit 5 - PIPE3 400G)     */
    {64, 71+20, GT_FALSE}      /* 8 LED ports  (LED Unit 6 - PIPE2 400G) +
                                    20 indications for dummy USX ports (LED stream alignment to 56 bits) */
};

static APP_DEMO_LED_UNIT_CONFIG_STC ac5p_RD_led_indications[CPSS_LED_UNITS_MAX_NUM_CNS] =
{
    {64, 65,    GT_FALSE},     /* 2 LED ports (LED Unit 0 - PIPE1 400G)     */
    { 0,  0,    GT_TRUE },     /* Not used    (LED Unit 1 - PIPE1 USX  )    */
    { 0,  0,    GT_TRUE},      /* Not used    (LED Unit 2 - PIPE1 CPU PORT) */
    {64, 65,    GT_FALSE},     /* 2 LED ports (LED Unit 3 - PIPE0 400G)     */
    { 0,  0,    GT_TRUE },     /* Not used    (LED Unit 4 - PIPE0 USX)      */
    {64, 64,    GT_FALSE},     /* 1 LED port  (LED Unit 5 - PIPE3 400G)     */
    {64, 64,    GT_FALSE}      /* 1 LED port  (LED Unit 6 - PIPE2 400G)     */
};

static APP_DEMO_LED_PORT_MAC_POSITION_STC ac5p_50G_ports_map[] =
{
    { 0, 0},  { 1, 1}, { 2, 2}, { 6, 3}, {10, 4}, {14, 5}, {18, 6}, {22, 7},
    {26, 0},  {27, 1}, {28, 2}, {32, 3}, {36, 4}, {40, 5}, {44, 6}, {48, 7},
    {52, 0},  {53, 1}, {54, 2}, {58, 3}, {62, 4}, {66, 5}, {70, 6}, {74, 7},
    {78, 0},  {79, 1}, {80, 2}, {84, 3}, {88, 4}, {92, 5}, {96, 6}, {100,7}
};

static APP_DEMO_LED_PORT_MAC_POSITION_STC ac5p_USX_ports_map[] =
{
    { 2, 0},  { 3, 0}, { 4, 0}, { 5, 0}, { 6, 0}, { 7, 0}, { 8, 0}, { 9, 0},
    {10, 2},  {11, 2}, {12, 2}, {13, 2}, {14, 2}, {15, 2}, {16, 2}, {17, 2},
    {18, 4},  {19, 4}, {20, 4}, {21, 4}, {22, 4}, {23, 4}, {24, 4}, {25, 4},

    {28, 0},  {29, 0}, {30, 0}, {31, 0}, {32, 0}, {33, 0}, {34, 0}, {35, 0},
    {36, 2},  {37, 2}, {38, 2}, {39, 2}, {40, 2}, {41, 2}, {42, 2}, {43, 2},
    {44, 4},  {45, 4}, {46, 4}, {47, 4}, {48, 4}, {49, 4}, {50, 4}, {51, 4}
};

static APP_DEMO_LED_PORT_MAC_POSITION_STC ac5p_RD_ports_map[] =
{
    { 0, 0},  { 1, 1},
    {26, 0},  {27, 1},
    {52, 0},
    {78, 0}
};

/**
* @internal led_port_position_get_mac
* @endinternal
*
* @brief   Gets LED ports position for the current MAC port
*
* @param[in] devNum                - device number
* @param[in] boardRevId            - board revision ID
* @param[in] portMacNum            - current MAC to check
*
* @retval GT_U32                   - the LED ports position for the current MAC port
*
*/
static GT_U32 led_port_position_get_mac
(
    IN  GT_U8  devNum,
    IN  GT_U8  boardRevId,
    IN  GT_U32 portMacNum
)
{
    GT_U32 ledPosition = 0;
    PRV_CPSS_PORT_TYPE_ENT  portMacType;
    APP_DEMO_LED_PORT_MAC_POSITION_STC * ac5pPortsMapArrPtr;
    GT_U32 ac5pPortsMapArrLength;
    GT_U32 ii;

    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum);
    switch (portMacType)
    {
        case PRV_CPSS_PORT_MTI_100_E:
            if (boardRevId == 2) /* RD board */
            {
                ac5pPortsMapArrPtr = &ac5p_RD_ports_map[0];
                ac5pPortsMapArrLength = sizeof(ac5p_RD_ports_map) / sizeof(ac5p_RD_ports_map[0]);
            }
            else
            {
                ac5pPortsMapArrPtr = &ac5p_50G_ports_map[0];
                ac5pPortsMapArrLength = sizeof(ac5p_50G_ports_map) / sizeof(ac5p_50G_ports_map[0]);
            }
            break;
        case PRV_CPSS_PORT_MTI_CPU_E:
            ac5pPortsMapArrPtr = NULL;
            break;
        case PRV_CPSS_PORT_MTI_USX_E:
            ac5pPortsMapArrPtr = &ac5p_USX_ports_map[0];
            ac5pPortsMapArrLength = sizeof(ac5p_USX_ports_map) / sizeof(ac5p_USX_ports_map[0]);
            break;
        default:
            ac5pPortsMapArrPtr = NULL;
            break;
    }

    if (ac5pPortsMapArrPtr)
    {
        for (ii = 0; ii < ac5pPortsMapArrLength; ii++)
        {
            if (ac5pPortsMapArrPtr[ii].macPortNumber == portMacNum)
            {
                ledPosition = ac5pPortsMapArrPtr[ii].ledPortPosition;
                break;
            }
        }
    }

    return ledPosition;
}

/**
* @internal hawk_LedInit function
* @endinternal
*
* @brief   LED configurations
*
* @param[in] devNum                - device number
* @param[in] boardRevId            - board revision ID
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
*/
static GT_STATUS hawk_LedInit
(
    IN  GT_U8    devNum,
    IN  GT_U8    boardRevId
)
{
    GT_STATUS                       rc;
    GT_U32                          classNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_LED_CONF_STC               ledConfig;
    CPSS_LED_CLASS_MANIPULATION_STC ledClassManip;
    GT_U32                          position;
    GT_U32                          ledUnit;
    CPSS_DXCH_PORT_MAP_STC          portMap;
    APP_DEMO_LED_UNIT_CONFIG_STC * ledStreamIndication;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery; /* holds system recovery information */

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

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    if (boardRevId == 2) /* RD board */
    {
        ledStreamIndication = ac5p_RD_led_indications;
        ledConfig.blink0Duration          = CPSS_LED_BLINK_DURATION_0_E;    /* 32 ms */                     /* BlinkDivision0.blink0Duration  [ 0: 29]  = 0x32DCD40 (32/app_clock in mSec)*/
        ledConfig.blink0DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E;  /* 50%   */                     /* blinkGlobalControl.Blink0-Duty [ 3: 4]  = 1 25%  */
        ledConfig.blink1Duration          = CPSS_LED_BLINK_DURATION_0_E;    /* 32 ms */                     /* BlinkDivision1.blink0Duration  [ 0: 29]  = 0x32DCD40 (32/app_clock in mSec)*/
        ledConfig.blink1DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E;  /* 50%   */                     /* blinkGlobalControl.Blink1-Duty [ 8: 9]  = 1 25%  */

    }
    else
    {
        ledStreamIndication = ac5p_XL_50G_led_indications;
    }

    for(ledUnit = 0; ledUnit < PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum; ledUnit++)
    {
        /* Start of LED stream location - Class 2 */
        ledConfig.sip6LedConfig.ledStart[ledUnit] = ledStreamIndication[ledUnit].ledStart;    /* LedControl.LedStart[5:12] */
        /* End of LED stream location: LED ports 0, 1 or 0 */
        ledConfig.sip6LedConfig.ledEnd[ledUnit] = ledStreamIndication[ledUnit].ledEnd;        /* LedControl.LedEnd[5:12] */
        /* LED unit may be bypassed in the LED chain */
        ledConfig.sip6LedConfig.ledChainBypass[ledUnit] = ledStreamIndication[ledUnit].bypassLedUnit;
    }

    rc = cpssDxChLedStreamConfigSet(devNum, 0, &ledConfig);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamConfigSet", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    ledClassManip.invertEnable            = GT_FALSE; /* not relevant for Hawk */
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
                                                   CPSS_DXCH_LED_PORT_TYPE_XG_E /* don't care , not applicable for Hawk */
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
     *       - port led position (stream is hard wired)                                                       *
     *           The port LED positions for full chip is as follows:                                          *
     *                                                                                                        *
     *           P0,    LED Server 1,  Positions 0-7   50G Ports  26, 27, 28, 32, 36, 40, 44, 48              *
     *                                                                                                        *
     *           P1,    LED Server 2   Positions 2     USX Ports  28, 29, 30, 31, 32, 33, 34, 35              *
     *           P1,    LED Server 2   Positions 4     USX Ports  36, 37, 38, 39, 40, 41, 42, 43              *
     *           P1,    LED Server 2   Positions 6     USX Ports  44, 45, 46, 47, 48, 49, 50, 51              *
     *                                                                                                        *
     *           P2,    LED Server 3,  Positions 0     CPU Ports  105                                         *
     *                                                                                                        *
     *           P3,    LED Server 4,  Positions 0-7   50G Ports  0,   1,  2,  6, 10, 14, 18, 22,             *
     *                                                                                                        *
     *           P4     LED Server 5   Positions 2     USX Ports   2,  3,  4,  5,  6,  7,  8,  9              *
     *           P4     LED Server 5   Positions 4     USX Ports  10, 11, 12, 13, 14, 15, 16, 17              *
     *           P4     LED Server 5   Positions 6     USX Ports  18, 19, 20, 21, 22, 23, 24, 25              *
     *                                                                                                        *
     *           P5,    LED Server 6,  Positions 0-7   50G Ports  78, 79, 80, 84, 88, 92, 96, 100             *
     *           P6,    LED Server 7,  Positions 0-7   50G Ports  52, 53, 54, 58, 62, 66, 70, 74              *
     *                                                                                                        *
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

        position = led_port_position_get_mac(devNum, boardRevId, portMap.interfaceNum);

        if (position != APPDEMO_BAD_VALUE)
        {
            rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamPortPositionSet", rc);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* Skip if in recovery mode HA */
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
* @internal gtDbHawkBoardReg_SimpleInit function
* @endinternal
*
* @brief   This is the 'simple' board initialization function for Hawk device.
*
* @param[in] boardRevId               - Board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
static GT_STATUS gtDbHawkBoardReg_SimpleInit
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
    GT_U32      ii;
    CPSS_HW_INFO_STC   hwInfo[APP_DEMO_CPSS_MAX_NUM_PORT_GROUPS_CNS];
    GT_SW_DEV_NUM           devNum;
    GT_U32      devIndex;/* device index in the array of appDemoPpConfigList[devIndex] */
    GT_PCI_INFO pciInfo;
    GT_U32      tmpData;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery;
    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of init */

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

    /* this function finds all Prestera devices on PCI bus */
    rc = hawk_getBoardInfoSimple(
            &hwInfo[0],
            &pciInfo,
            &appDemoPpConfigDevAmount);
    if (rc != GT_OK)
        return rc;

    if(cpssDeviceRunCheck_onEmulator())
    {
        /* the 'NO KN' is doing polling in this interrupt global cause register */
        /* but this polling should not be seen during 'trace'                   */
        appDemoTrace_skipTrace_onAddress(PRV_CPSS_AC5P_MG0_BASE_ADDRESS_CNS + 0x30 /*0x3d000030*//*address*/,0/*index*/);
    }

    /* take time from the 'phase1 init' stage (not including the 'PCI scan' operations) */
    cpssOsTimeRT(&start_sec, &start_nsec);

    hawk_boardTypePrint("DB" /*boardName*/, "Hawk" /*devName*/);

    if(appDemoDbEntryGet("useCpuPort", &tmpData) != GT_OK)
    {
        cpuPortMuxUseSdma = 0;
    }
    else
    {
        /* useCpuPort + 0 means SDMA, all other value means CPU MAC is used */
        cpuPortMuxUseSdma = tmpData?0:1;
    }

    for (ii = 0; ii < appDemoPpConfigDevAmount; ii++ )
    {
        devIndex = SYSTEM_DEV_NUM_MAC(ii);
        devNum = devIndex;

        rc = hawk_appDemoInitSequence(boardRevId, devIndex, devNum, &hwInfo[ii]);
        if (rc != GT_OK)
            return rc;
#ifndef GM_USED
        if (boardRevId == 2 /* RD */)
        {
            rc = phoenix_phyInit(devNum, CPSS_PHY_SMI_INTERFACE_0_E);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("hawk_phyInit", rc);
            if (rc != GT_OK)
                return rc;
        }
        rc = hawk_LedInit(devNum, boardRevId);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("hawk_LedInit", rc);
        if (rc != GT_OK)
            return rc;
#endif
    }
    if (appDemoInitRegDefaults != GT_FALSE)
    {
        return GT_OK;
    }

    /* init the event handlers */
    rc = hawk_EventHandlerInit();
    CPSS_ENABLER_DBG_TRACE_RC_MAC("hawk_EventHandlerInit", rc);
    if (rc != GT_OK)
        return rc;

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E == system_recovery.systemRecoveryProcess)
    {
        for (ii = 0; ii < appDemoPpConfigDevAmount; ii++ )
        {
            devIndex = SYSTEM_DEV_NUM_MAC(ii);
            devNum =   appDemoPpConfigList[devIndex].devNum;

            /* Data Integrity module initialization. It should be done after events init. */
            rc = appDemoDataIntegrityInit(devNum);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDataIntegrityInit", rc);
            if(GT_OK != rc)
            {
                return rc;
            }
        }
    }

    rc = hawk_localUtfInit(SYSTEM_DEV_NUM_MAC(0));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("hawk_localUtfInit", rc);
    if (rc != GT_OK)
        return rc;

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

    /* allow interrupts / appDemo tasks to stable */
    osTimerWkAfter(500);

    return rc;
}

/**
* @internal gtDbHawkBoardReg_BoardCleanDbDuringSystemReset function
* @endinternal
*
* @brief   clear the DB of the specific board config file , as part of the 'system rest'
*         to allow the 'cpssInitSystem' to run again as if it is the first time it runs
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS gtDbHawkBoardReg_BoardCleanDbDuringSystemReset
(
    IN  GT_U8   boardRevId
)
{
    UNUSED_PARAM_MAC(boardRevId);

    appDemo_PortsInitList_already_done = 0;
    actualNum_cpssApi_hawk_defaultMap = 0;

    return GT_OK;
}
/**
* @internal gtDbDxHawkBoardReg function
* @endinternal
*
* @brief   Registration function for the Hawk board .
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDxHawkBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
)
{
    UNUSED_PARAM_MAC(boardRevId);

    /* set the actual pointer to use for this board */
    boardCfgFuncs->boardSimpleInit               = gtDbHawkBoardReg_SimpleInit;
    boardCfgFuncs->boardCleanDbDuringSystemReset = gtDbHawkBoardReg_BoardCleanDbDuringSystemReset;

    return GT_OK;
}

static APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_STC hawkPakcetBufferParseRegsArr[] =
{
     {0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,         0x100, "PB_CENTER",  {0x00000000, 0x00008000, 0, 0}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x110, "PB_COUNTER", {0x00200000, 0x00008000, 0, 0}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E, 0x140, "PB_SMB_WR_ARB",  {0x00400000, 0x00200000, 4, 0x00010000}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E, 0x150, "PB_GPC_PKT_WR",  {0x00C00000, 0x00200000, 4, 0x00010000}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E, 0x160, "PB_GPC_CEL_RD",  {0x01400000, 0x00200000, 4, 0x00010000}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E, 0x170, "PB_GPC_PKT_RD",  {0x01C00000, 0x00200000, 4, 0x00010000}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E, 0x180, "PB_NXT_PTR_MEM", {0x02400000, 0x00200000, 2, 0x00010000}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E, 0x190, "PB_SHARED_MEMO", {0x02800000, 0x00400000, 3, 0x00010000}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E, 0x1A0, "RX_DMA",  {0x08000000, 0x00400000, 4, 0x00010000}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E, 0x1B0, "TX_DMA",  {0x09800000, 0x00800000, 4, 0x00010000}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_SERIAL_E, 0x1C0, "TX_FIFO", {0x09C00000, 0x00800000, 4, 0x00010000}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x200, "PCA_SFF0", {0x34800000, 0x00004000, 0, 0}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x200, "PCA_SFF1", {0x35000000, 0x00004000, 0, 0}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x200, "PCA_SFF2", {0x32300000, 0x00004000, 0, 0}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x200, "PCA_SFF3", {0x33300000, 0x00004000, 0, 0}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x210, "PCA_BRG0", {0x34000000, 0x00004000, 0, 0}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x210, "PCA_BRG1", {0x34400000, 0x00004000, 0, 0}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x210, "PCA_BRG2", {0x32000000, 0x00004000, 0, 0}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x210, "PCA_BRG3", {0x33000000, 0x00004000, 0, 0}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x220, "PCA_PZ_ARB_E0", {0x34200000, 0x00004000, 0, 0}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x220, "PCA_PZ_ARB_E1", {0x34600000, 0x00004000, 0, 0}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x220, "PCA_PZ_ARB_E2", {0x32200000, 0x00004000, 0, 0}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x220, "PCA_PZ_ARB_E3", {0x33200000, 0x00004000, 0, 0}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x220, "PCA_PZ_ARB_I0", {0x36000000, 0x00004000, 0, 0}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x220, "PCA_PZ_ARB_I1", {0x36400000, 0x00004000, 0, 0}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x220, "PCA_PZ_ARB_I2", {0x32100000, 0x00004000, 0, 0}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x220, "PCA_PZ_ARB_I3", {0x33100000, 0x00004000, 0, 0}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x230, "PCA_CTSU0", {0x34B00000, 0x000010000, 0, 0}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x230, "PCA_CTSU1", {0x35300000, 0x000010000, 0, 0}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x230, "PCA_CTSU2", {0x32500000, 0x000010000, 0, 0}}
     ,{0x0,   APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_RANGE_E,        0x230, "PCA_CTSU3", {0x33500000, 0x000010000, 0, 0}}
     ,{0x170, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E,  0, "PzSlotCfg",     {0x0200, 0x4, 92, 0}}
     ,{0x170, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E,         0, "PzControl",     {0x0100, 0, 0, 0}}
     ,{0x170, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E,  0, "ChanEnable",    {0x0500, 0x4, 28, 0}}
     ,{0x170, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E,  0, "ChanReset",     {0x0600, 0x4, 28, 0}}
     ,{0x170, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E,  0, "ChanProfile",   {0x0700, 0x4, 28, 0}}
     ,{0x1A0, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E,  0, "ChanCfCfg",     {0x0120, 0x4, 27, 0}}
     ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E,  0, "ChanCtrlEn",    {0x0600, 0x4, 0x020, 0}}
     ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E,  0, "ChanCtrlCfg",   {0x0800, 0x4, 0x020, 0}}
     ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E,  0, "LlNextPtr",     {0x0A00, 0x4, 136, 0}}
     ,{0x200, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E,         0, "globalCfg",     {0x0000, 0, 0, 0}}
     ,{0x210, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E,  0, "RxChannel",     {0x0100, 0x4, 32, 0}}
     ,{0x210, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E,  0, "TxChannel",     {0x0800, 0x4, 32, 0}}
     ,{0x220, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_SERIAL_E,  0, "PzSlotCfg",     {0x0300, 0x4, 0x100, 0}}
     ,{0x220, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_ADDR_E,         0, "PzControl",     {0x0B0C, 0, 0, 0}}

     /* end of list */
     ,{0, APP_DEMO_HW_PP_TRACING_ADDR_PARSE_RULE_LAST_E, 0, "", {0}}
};

void appDemoHwPpDrvRegTracingHawkPb()
{
    appDemoHwPpDrvRegTracingRegNameBind(hawkPakcetBufferParseRegsArr);
    /* pairs of base and size */
    appDemoHwPpDrvRegTracingRegAddressesBoundsSet(
        0x00000000, 0x03010000, /* Packet Buffer */
        0x08000000, 0x00C10000, /* RXDMA0-3 */
        0x09800000, 0x01C10000, /* TX_DMA0-3, TX_FIFO0-3 */
        0x32000000, 0x04E10000, /* all PCA units 0-3 */
        0, 0);
    appDemoHwPpDrvRegTracingBind(1);
}

#if 0
    ,{PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E              ,0x00000000,     32  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_COUNTER_E                 ,0x00200000,     32  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_E       ,0x00400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_1_E     ,0x00600000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_2_E     ,0x00800000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_3_E     ,0x00A00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_0_E  ,0x00C00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_1_E  ,0x00E00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_2_E  ,0x01000000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_3_E  ,0x01200000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_0_E     ,0x01400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_1_E     ,0x01600000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_2_E     ,0x01800000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_3_E     ,0x01A00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E   ,0x01C00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_1_E   ,0x01E00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_2_E   ,0x02000000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_3_E   ,0x02200000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E     ,0x02400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_1_E     ,0x02600000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E     ,0x02800000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_1_0_E     ,0x02C00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_2_0_E     ,0x03000000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_RXDMA_E                      ,0x08000000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_RXDMA1_E                     ,0x08400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_RXDMA2_E                     ,0x08800000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_RXDMA3_E                     ,0x08C00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXDMA_E                      ,0x09800000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TX_FIFO_E                    ,0x09C00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXDMA1_E                     ,0x0A000000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TX_FIFO1_E                   ,0x0A400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXDMA2_E                     ,0x0A800000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TX_FIFO2_E                   ,0x0AC00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TXDMA3_E                     ,0x0B000000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_TX_FIFO3_E                   ,0x0B400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_2_E                  ,0x32000000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_2_E         ,0x32100000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_2_E         ,0x32200000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_SFF_2_E                  ,0x32300000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_2_E                  ,0x32400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_CTSU_2_E                 ,0x32500000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_2_E     ,0x32600000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_2_E     ,0x32800000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_2_E     ,0x32A00000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_2_E     ,0x32C00000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_3_E                  ,0x33000000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_3_E         ,0x33100000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_3_E         ,0x33200000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_SFF_3_E                  ,0x33300000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_3_E                  ,0x33400000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_CTSU_3_E                 ,0x33500000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_3_E     ,0x33600000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_3_E     ,0x33800000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_3_E     ,0x33A00000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_3_E     ,0x33C00000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_0_E                  ,0x34000000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_0_E         ,0x34200000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_BRG_1_E                  ,0x34400000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_1_E         ,0x34600000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_SFF_0_E                  ,0x34800000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_0_E                  ,0x34A00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_CTSU_0_E                 ,0x34B00000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_0_E     ,0x34C00000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_0_E     ,0x34D00000,      1  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCA_SFF_1_E                  ,0x35000000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_LMU_1_E                  ,0x35200000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_CTSU_1_E                 ,0x35300000,     64  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_1_E     ,0x35400000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_E_1_E     ,0x35500000,      1  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_0_E     ,0x35800000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_0_E     ,0x35A00000,      1  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_1_E     ,0x35C00000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_E_1_E     ,0x35E00000,      1  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_0_E         ,0x36000000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_0_E     ,0x36100000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_0_E     ,0x36200000,      1  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_1_E         ,0x36400000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_1_E     ,0x36500000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_163_I_1_E     ,0x36600000,      1  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_0_E     ,0x36800000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_0_E     ,0x36A00000,      1  * _1M }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_1_E     ,0x36C00000,     16  * _1K }
    ,{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_164_I_1_E     ,0x36E00000,      1  * _1M }
#endif /*0*/

extern void pexAccessTracePreWriteCallbackBind(void (*pCb)(GT_U32, GT_U32));

static void hawkPreWriteSimpleTrace(GT_U32 address, GT_U32 data)
{
    if (appDemoHwPpDrvRegTracingIsTraced(address) == 0) return;

    osPrintf("0x%08X 0x%08X\n", address, data);
}

void appDemoHwPpDrvRegTracingHawkAbsAddrDpPreWriteEnable()
{
    /* pairs of base and size */
    appDemoHwPpDrvRegTracingRegAddressesBoundsSet(
        0x00000000, 0x03010000, /* Packet Buffer */
        0x08000000, 0x00C10000, /* RXDMA0-3 */
        0x09800000, 0x01C10000, /* TX_DMA0-3, TX_FIFO0-3 */
        0x32000000, 0x04E10000, /* all PCA units 0-3 */
        0, 0);
    pexAccessTracePreWriteCallbackBind(&hawkPreWriteSimpleTrace);
}

void appDemoHwPpDrvRegTracingHawkAbsAddrDpPca0PreWriteEnable()
{
    /* pairs of base and size */
    appDemoHwPpDrvRegTracingRegAddressesBoundsSet(
        /*{PRV_CPSS_DXCH_UNIT_PCA_BRG_0_E                  */0x34000000,    0x3FFF/*16  * _1K*/
       ,/*{PRV_CPSS_DXCH_UNIT_PCA_SFF_0_E                  */0x34800000,    0x3FFF/*16  * _1K*/
       ,/*{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_0_E         */0x36000000,    0x3FFF/*16  * _1K*/
       ,/*{PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_0_E         */0x34200000,    0x3FFF/*16  * _1K*/
       ,/*{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_0_E     */0x34C00000,    0x3FFF/*16  * _1K*/
       ,/*{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_0_E     */0x36100000,    0x3FFF/*16  * _1K*/
       ,/*{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_0_E     */0x35800000,    0x3FFF/*16  * _1K*/
       ,/*{PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_0_E     */0x36800000,    0x3FFF/*16  * _1K*/
       ,0, 0);
    pexAccessTracePreWriteCallbackBind(&hawkPreWriteSimpleTrace);
}

/********************************************************************/
static void appDemoHwPpDrvRegTracingHawkDpTracingRegAddressesBoundsSet
(
    IN GT_U8  devNum,
    IN GT_U32 pbUnitsBitmap,
    IN GT_U32 dpBitmap,
    IN GT_U32 dpUnitsBitmap
)
{
    GT_STATUS                rc;
    GT_U32                   baseArr[64];
    GT_U32                   sizeArr[64];
    GT_U32                   i;
    GT_U32                   dp;
    GT_U32                   unitsNum;
    PRV_CPSS_DXCH_UNIT_ENT   unitId;
    GT_BOOL                  error;

    cpssOsMemSet(baseArr, 0, sizeof(baseArr));
    cpssOsMemSet(sizeArr, 0, sizeof(sizeArr));

    unitsNum = 0;
    for (i = 0; (i < 8); i++)
    {
        if ((pbUnitsBitmap & (1 << i)) == 0) continue;
        switch (i)
        {
            case 0: unitId = PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E; break;
            case 1: unitId = PRV_CPSS_DXCH_UNIT_PB_COUNTER_E; break;
            case 2: unitId = PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_E; break;
            case 3: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_0_E; break;
            case 4: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_CELL_READ_0_E; break;
            case 5: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E; break;
            case 6: unitId = PRV_CPSS_DXCH_UNIT_PB_NEXT_POINTER_MEMO_0_E; break;
            case 7: unitId = PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E; break;
            default:
                /* unused bits */
                continue;
        }
        error = GT_FALSE;
        baseArr[unitsNum] = prvCpssDxChHwUnitBaseAddrGet(
            devNum, unitId, &error);
        if (error != GT_FALSE)
        {
            cpssOsPrintf("prvCpssDxChHwUnitBaseAddrGet error\n");
            return;
        }
        rc = prvCpssSip6UnitIdSizeInByteGet(
            devNum, unitId, &(sizeArr[unitsNum]));
        if (rc != GT_OK)
        {
            cpssOsPrintf("prvCpssSip6UnitIdSizeInByteGet error\n");
            return;
        }
        unitsNum ++;
    }
    for (dp = 0; (dp < 4); dp++)
    {
        if ((dpBitmap & (1 << dp)) == 0) continue;

        for (i = 0; (i < 11); i++)
        {
            if ((dpUnitsBitmap & (1 << i)) == 0) continue;

            switch (dp)
            {
                case 0:
                    switch (i)
                    {
                        case  0: unitId = PRV_CPSS_DXCH_UNIT_PCA_SFF_0_E; break;
                        case  1: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_0_E; break;
                        case  2: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_0_E; break;
                        case  3: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_0_E; break;
                        case  4: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_0_E; break;
                        case  5: unitId = PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_0_E; break;
                        case  6: unitId = PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_0_E; break;
                        case  7: unitId = PRV_CPSS_DXCH_UNIT_PCA_BRG_0_E; break;
                        case  8: unitId = PRV_CPSS_DXCH_UNIT_TX_FIFO_E; break;
                        case  9: unitId = PRV_CPSS_DXCH_UNIT_TXDMA_E; break;
                        case 10: unitId = PRV_CPSS_DXCH_UNIT_RXDMA_E; break;
                        default:
                            /* unused bits */
                            continue;
                    }
                    break;
                case 1:
                    switch (i)
                    {
                        case  0: unitId = PRV_CPSS_DXCH_UNIT_PCA_SFF_1_E; break;
                        case  1: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_1_E; break;
                        case  2: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_1_E; break;
                        case  3: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_1_E; break;
                        case  4: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_1_E; break;
                        case  5: unitId = PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_1_E; break;
                        case  6: unitId = PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_1_E; break;
                        case  7: unitId = PRV_CPSS_DXCH_UNIT_PCA_BRG_1_E; break;
                        case  8: unitId = PRV_CPSS_DXCH_UNIT_TX_FIFO1_E; break;
                        case  9: unitId = PRV_CPSS_DXCH_UNIT_TXDMA1_E; break;
                        case 10: unitId = PRV_CPSS_DXCH_UNIT_RXDMA1_E; break;
                        default:
                            /* unused bits */
                            continue;
                    }
                    break;

                case 2:
                    switch (i)
                    {
                        case  0: unitId = PRV_CPSS_DXCH_UNIT_PCA_SFF_2_E; break;
                        case  1: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_2_E; break;
                        case  2: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_2_E; break;
                        case  3: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_2_E; break;
                        case  4: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_2_E; break;
                        case  5: unitId = PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_2_E; break;
                        case  6: unitId = PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_2_E; break;
                        case  7: unitId = PRV_CPSS_DXCH_UNIT_PCA_BRG_2_E; break;
                        case  8: unitId = PRV_CPSS_DXCH_UNIT_TX_FIFO2_E; break;
                        case  9: unitId = PRV_CPSS_DXCH_UNIT_TXDMA2_E; break;
                        case 10: unitId = PRV_CPSS_DXCH_UNIT_RXDMA2_E; break;
                        default:
                            /* unused bits */
                            continue;
                    }
                    break;
                case 3:
                    switch (i)
                    {
                        case  0: unitId = PRV_CPSS_DXCH_UNIT_PCA_SFF_3_E; break;
                        case  1: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_3_E; break;
                        case  2: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_3_E; break;
                        case  3: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_3_E; break;
                        case  4: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_3_E; break;
                        case  5: unitId = PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_3_E; break;
                        case  6: unitId = PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_E_3_E; break;
                        case  7: unitId = PRV_CPSS_DXCH_UNIT_PCA_BRG_3_E; break;
                        case  8: unitId = PRV_CPSS_DXCH_UNIT_TX_FIFO3_E; break;
                        case  9: unitId = PRV_CPSS_DXCH_UNIT_TXDMA3_E; break;
                        case 10: unitId = PRV_CPSS_DXCH_UNIT_RXDMA3_E; break;
                        default:
                            /* unused bits */
                            continue;
                    }
                    break;
                default:
                    /* unused bits */
                    continue;
            }
            error = GT_FALSE;
            baseArr[unitsNum] = prvCpssDxChHwUnitBaseAddrGet(
                devNum, unitId, &error);
            if (error != GT_FALSE)
            {
                cpssOsPrintf("prvCpssDxChHwUnitBaseAddrGet error\n");
                return;
            }
            rc = prvCpssSip6UnitIdSizeInByteGet(
                devNum, unitId, &(sizeArr[unitsNum]));
            if (rc != GT_OK)
            {
                cpssOsPrintf("prvCpssSip6UnitIdSizeInByteGet error\n");
                return;
            }
            unitsNum ++;
        }
    }

    /* pairs of base and size */
    appDemoHwPpDrvRegTracingRegAddressesBoundsFromArraySet(
        baseArr, sizeArr);
}

static GT_BOOL hawkPreWriteTrace_Stopped = GT_FALSE;
static GT_BOOL hawkPreWriteTrace_FirstCall = GT_TRUE;
static GT_U8   hawkPreWriteTrace_devNum = 0;
static GT_U32  hawkPreWriteTrace_pbUnitsBitmap = 0;
static GT_U32  hawkPreWriteTrace_dpBitmap = 0;
static GT_U32  hawkPreWriteTrace_dpUnitsBitmap = 0;

static void hawkPreWriteTrace(GT_U32 address, GT_U32 data)
{
    if (hawkPreWriteTrace_Stopped != GT_FALSE) return;

    if (hawkPreWriteTrace_FirstCall != GT_FALSE)
    {
        if (PRV_CPSS_PP_MAC(hawkPreWriteTrace_devNum) == 0)
        {
            /* no DB yet */
            return;
        }
        if (PRV_CPSS_DXCH_PP_MAC(hawkPreWriteTrace_devNum)->accessTableInfoSize == 0)
        {
            /* no table DB yet */
            return;
        }

        /* bind at first call when unit addresses DB already configured */
        hawkPreWriteTrace_FirstCall = GT_FALSE;
        appDemoHwPpDrvRegTracingHawkDpTracingRegAddressesBoundsSet(
            hawkPreWriteTrace_devNum,
            hawkPreWriteTrace_pbUnitsBitmap,
            hawkPreWriteTrace_dpBitmap,
            hawkPreWriteTrace_dpUnitsBitmap);
    }

    if (appDemoHwPpDrvRegTracingIsTraced(address) == 0) return;

    osPrintf("0x%08X 0x%08X\n", address, data);
}

void appDemoHwPpDrvRegTracingHawkDpPreWriteEnable
(
    IN GT_U8  devNum,
    IN GT_U32 pbUnitsBitmap,
    IN GT_U32 dpBitmap,
    IN GT_U32 dpUnitsBitmap
)
{
    hawkPreWriteTrace_devNum      = devNum;
    hawkPreWriteTrace_pbUnitsBitmap  = pbUnitsBitmap;
    hawkPreWriteTrace_dpBitmap       = dpBitmap;
    hawkPreWriteTrace_dpUnitsBitmap  = dpUnitsBitmap;
    pexAccessTracePreWriteCallbackBind(&hawkPreWriteTrace);
}

/********************************************************************/


/* used to prepare manual coded table */
/* slot_index ==> resource index      */
void appDemoHwPpPizzaResourcesCalculator
(
    GT_U32 totalSlots,
    ...
    /* repeated group po parameters */
    /* GT_U32 resGroup0Amount,    amount of resources in Group0, 0 - end off list */
    /* GT_U32 resGroup0Size,      Size in slots of each resource in Group0        */
    /* GT_U32 resGroup0IndexBase, index of the first resource in Group0           */
)
{
    GT_U32  *slorsArrPtr;
    GT_U32  resGroupAmount;
    GT_U32  resGroupSize;
    GT_U32  resGroupIndexBase;
    va_list argP;
    GT_U32  i;
    GT_U32  resIndex;
    GT_U32  fillUnitSize=0;
    GT_U32  fillUnitBase;
    GT_U32  fillUnitBaseMax;
    GT_U32  fillMin;
    GT_U32  fillCurrent;
    GT_U32  startSlot;
    GT_U32  aproxSlot;
    GT_U32  nextSlot=0;
    GT_U32  slotInResIndex;

    va_start(argP, totalSlots);

    slorsArrPtr = (GT_U32 *)cpssOsMalloc(totalSlots * sizeof(GT_U32));
    if (slorsArrPtr == NULL)
    {
        cpssOsPrintf("Memory allocation failed\n");
        va_end(argP);
        return;
    }
    for (i = 0; (i < totalSlots); i++)
    {
        /* mark all cells as free */
        slorsArrPtr[i] = 0xFFFFFFFF;
    }
    /* fillUnitSize = SQRT(totalSlots)*/
    for (i = 0; ((i * i) <= totalSlots); i++) {fillUnitSize = i;}
    fillUnitBaseMax = totalSlots - (totalSlots % fillUnitSize);

    while (1)
    {
        resGroupAmount     = va_arg(argP, GT_U32);
        if (resGroupAmount == 0) {break; };
        resGroupSize       = va_arg(argP, GT_U32);
        resGroupIndexBase  = va_arg(argP, GT_U32);
        for (resIndex = resGroupIndexBase;
              (resIndex < (resGroupIndexBase + resGroupAmount));
              resIndex++)
        {
            /* look for the minimal used fillUnit */
            /* 1. determinate the fillMin value*/
            fillMin = fillUnitSize; /* maximal possible to decrease */
            for (fillUnitBase = 0; (fillUnitBase < fillUnitBaseMax);  fillUnitBase += fillUnitSize)
            {
                fillCurrent = 0;
                for (i = 0; (i < fillUnitSize); i++)
                {
                    if (slorsArrPtr[fillUnitBase + i] != 0xFFFFFFFF) {fillCurrent++;}
                }
                if (fillMin > fillCurrent) {fillMin = fillCurrent;}
                if (fillMin == 0) {break;}
            }
            /* 2. find fillUnitBase */
            for (fillUnitBase = 0; (fillUnitBase < fillUnitBaseMax);  fillUnitBase += fillUnitSize)
            {
                fillCurrent = 0;
                for (i = 0; (i < fillUnitSize); i++)
                {
                    if (slorsArrPtr[fillUnitBase + i] != 0xFFFFFFFF) {fillCurrent++;}
                }
                if (fillMin == fillCurrent) {break;}
            }
            /* find free startSlot */
            startSlot = 0xFFFFFFFF;
            if (fillMin < fillUnitSize)
            {
                for (i = 0; (i < fillUnitSize); i++)
                {
                    if (slorsArrPtr[fillUnitBase + i] == 0xFFFFFFFF)
                    {
                        startSlot = fillUnitBase + i;
                        break;
                    }
                }
            }
            else
            {
                /* fearch free place in the reminder */
                for (i = fillUnitBaseMax; (i < totalSlots); i++)
                {
                    if (slorsArrPtr[i] == 0xFFFFFFFF)
                    {
                        startSlot = i;
                        break;
                    }
                }
            }
            if (startSlot == 0xFFFFFFFF)
            {
                cpssOsFree(slorsArrPtr);
                va_end(argP);
                cpssOsPrintf("Required more slots, than pizza has\n");
                return;
            }

            /* allocate resource */
            slorsArrPtr[startSlot] = resIndex;
            for (slotInResIndex = 1; (slotInResIndex < resGroupSize); slotInResIndex++)
            {
                aproxSlot = (startSlot + ((totalSlots * slotInResIndex) / resGroupSize)) % totalSlots;
                for (i = 0; (i <= (totalSlots / 2)); i++)
                {
                    nextSlot = (aproxSlot + i) % totalSlots;
                    if (slorsArrPtr[nextSlot] == 0xFFFFFFFF) break;
                    nextSlot = (aproxSlot + totalSlots - i) % totalSlots;
                    if (slorsArrPtr[nextSlot] == 0xFFFFFFFF) break;
                }
                if (slorsArrPtr[nextSlot] != 0xFFFFFFFF)
                {
                    cpssOsFree(slorsArrPtr);
                    va_end(argP);
                    cpssOsPrintf("ERROR, no free space in pizza found\n");
                    return;
                }
                slorsArrPtr[nextSlot] = resIndex;
            }
        }
    }

    cpssOsPrintf("---! --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---!\n");
    cpssOsPrintf("---!   0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15!\n");
    cpssOsPrintf("---! --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---!\n");
    for (i = 0; (i < totalSlots); i++)
    {
        if ((i % 16) == 0)
        {
            cpssOsPrintf("%03d!", i);
        }
        if (slorsArrPtr[i] == 0xFFFFFFFF) {cpssOsPrintf(" ***");}
        else {cpssOsPrintf(" %03d", slorsArrPtr[i]);}
        if ((i % 16) == 15)
        {
            cpssOsPrintf("!\n");
        }
    }
    if ((totalSlots % 16) != 0) {cpssOsPrintf("\n");}
    cpssOsPrintf("---! --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---!\n");

    cpssOsFree(slorsArrPtr);
    va_end(argP);
}


GT_STATUS  ac5p_clearMappingDb
(
    GT_VOID
)
{
    actualNum_cpssApi_hawk_defaultMap =0;
    hawk_force_PortsInitListPtr = NULL;
    return GT_OK;
}

#define ALL_PORTS 0xFFFF
/**
* @internal hawk_printMappings function
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
GT_STATUS hawk_printMappings
(
    IN  GT_U8            devNum,
    IN  GT_U32           portNum
)
{
    GT_STATUS                       rc;
    GT_U32                          portIterator, macNum=0,serdesNum = 0;
    GT_BOOL                         invertTx;
    GT_BOOL                         invertRx;
    CPSS_DXCH_PORT_MAP_STC          portMap;
    CPSS_DXCH_ACTIVE_LANES_STC      portParams;

    cpssOsPrintf("+------+------+--------+--------+-------+-------+\n");
    cpssOsPrintf("| Port | Mac  | SerDes | SD Mux |Inv.Tx |Inv.Rx |\n");
    cpssOsPrintf("+------+------+--------+--------+-------+-------+\n");

    for(portIterator = 0; portIterator < appDemoPpConfigList[devNum].maxPortNumber ; portIterator++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portIterator);
        if (prvCpssDxChPortRemotePortCheck(devNum,portIterator))
        {
            continue;
        }

        if((ALL_PORTS != portNum) && (portIterator != portNum))
        {
            continue;
        }

        rc = cpssDxChPortPhysicalPortMapGet(devNum, portIterator, 1, /*OUT*/&portMap);
        if(rc != GT_OK)
        {
            continue;
        }

        macNum = portMap.interfaceNum;
        rc = cpssDxChPortSerdesActiveLanesListGet(devNum,portIterator,&portParams);
        if(rc != GT_OK)
        {
            continue;
        }
        serdesNum = portParams.activeLaneList[0];


        rc = cpssDxChPortSerdesLanePolarityGet(devNum,0,serdesNum,&invertTx,&invertRx);
        if(rc != GT_OK)
        {
            continue;
        }

        cpssOsPrintf("| %4d | %4d |  %4d  |  %4d  | %5s | %5s |\n",portIterator,
                                                                   macNum,
                                                                   serdesNum,
                                                                   serdesNum % 8,
                                                                   invertTx?"TRUE ":"FALSE",
                                                                   invertRx?"TRUE ":"FALSE");
        cpssOsPrintf("+------+------+--------+--------+-------+-------+\n");
    }
    return GT_OK;
}

