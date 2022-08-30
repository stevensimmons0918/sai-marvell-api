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
* @file gtAppDemoPhyConfig.c
*
* @brief Generic support for PHY init.
*
* @version   1
********************************************************************************
*/
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/sysHwConfig/gtAppDemoTmConfig.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/boardConfig/gtBoardsConfigFuncs.h>
#include <appDemo/boardConfig/gtDbDxFalcon.h>
#include <appDemo/boardConfig/appDemoBoardConfig_mpdTools.h>
#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>
#include <cpss/common/smi/cpssGenSmi.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <appDemo/phy/gtAppDemoPhyConfig.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <appDemo/boardConfig/appDemoBoardConfig_ezBringupTools.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#endif /*CHX_FAMILY*/

#ifndef ASIC_SIMULATION
#ifndef INCLUDE_MPD

#include <88x3240/mtdApiTypes.h>
#include <88x3240/mtdHwCntl.h>
#include <88x3240/mtdAPI.h>
#include <88x3240/mtdFwDownload.h>
#include <88x3240/mtdHunit.h>
#include <88x3240/mtdCunit.h>

/* for Lewis PHY driver */
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#else /*INCLUDE_MPD */


#undef MIN
#include <mpdPrv.h>
#define __USE_POSIX199309
#include <sched.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>



extern GT_U16 portToPhyArrayAldrinRd[][2];
extern GT_U16 portToPhyArrayAldrin2Rd[][2];

GT_U16 portToPhyArrayAc3xTb2_0[][2] = {
    {0,1},
    {1,0},
    {0xff, 0xff}
};

GT_U16 portToPhyArrayAc3xTb2_1[][2] = {
    {8, 0},   /*device 0 */
    {9, 1},
    {10,2},
    {11,3},
    {8, 4},  /*device 1*/
    {9, 5},
    {10,6},
    {11,7},
    {0xff, 0xff}
};

/*same as port2PhySmiAddrTable */
GT_U16 portToPhyArrayCetusLewisRd[][2] = {
    {56,3},
    {57,0},
    {58,1},
    {59,2},
    {64,6},
    {65,4},
    {66,5},
    {67,7},
    {68,8},
    {69,9},
    {70,0xa},
    {71,0xb},
    {0xff, 0xff}
};

GT_U16 portToPhyArrayAc5xRd1780[][2] = {
    {0, 0x0},
    {1, 0x1},
    {2, 0x2},
    {3, 0x3},
    {4, 0x4},
    {5, 0x5},
    {6, 0x6},
    {7, 0x7},
    {8, 0x8},
    {9, 0x9},
    {10,0xa},
    {11,0xb},
    {12,0xc},
    {13,0xd},
    {14,0xe},
    {15,0xf},
    {16,0x10},
    {17,0x11},
    {18,0x12},
    {19,0x13},
    {20,0x14},
    {21,0x15},
    {22,0x16},
    {23,0x17},
    {24,0x18},
    {25,0x19},
    {26,0x1a},
    {27,0x1b},
    {28,0x1c},
    {29,0x1d},
    {30,0x1e},
    {31,0x1f},
    {0xff, 0xff}
};

GT_U16 portToPhyArrayAc5xRd2540_SR1[][2] = {
    {32,0x0},
    {33,0x1},
    {34,0x2},
    {35,0x3},
    {36,0x4},
    {37,0x5},
    {38,0x6},
    {39,0x7},
    {40,0x1c},
    {41,0x1d},
    {42,0x1e},
    {43,0x1f},
    {44,0xc},
    {45,0xd},
    {46,0xe},
    {47,0xf},
    {0xff, 0xff}
};

GT_U16 portToPhyArrayAc5pRd2580_SR1[][2] = {
    {16, 0x1}, /* PanelPort[33] E2580_#1 SRDS[5] 20G_QXGMII */
    {14, 0x0}, /* PanelPort[34] E2580_#1 SRDS[5] 20G_QXGMII */
    {17, 0x3}, /* PanelPort[35] E2580_#1 SRDS[5] 20G_QXGMII */
    {15, 0x2}, /* PanelPort[36] E2580_#1 SRDS[5] 20G_QXGMII */
    {12, 0x5}, /* PanelPort[37] E2580_#1 SRDS[4] 20G_QXGMII */
    {10, 0x4}, /* PanelPort[38] E2580_#1 SRDS[4] 20G_QXGMII */
    {13, 0x7}, /* PanelPort[39] E2580_#1 SRDS[4] 20G_QXGMII */
    {11, 0x6}, /* PanelPort[40] E2580_#1 SRDS[4] 20G_QXGMII */

    { 8, 0x9}, /* PanelPort[41] E2580_#2 SRDS[3] 20G_QXGMII */
    { 6, 0x8}, /* PanelPort[42] E2580_#2 SRDS[3] 20G_QXGMII */
    { 9, 0xb}, /* PanelPort[43] E2580_#2 SRDS[3] 20G_QXGMII */
    { 7, 0xa}, /* PanelPort[44] E2580_#2 SRDS[3] 20G_QXGMII */
    { 4, 0xd}, /* PanelPort[45] E2580_#2 SRDS[2] 20G_QXGMII */
    { 2, 0xc}, /* PanelPort[46] E2580_#2 SRDS[2] 20G_QXGMII */
    { 5, 0xf}, /* PanelPort[47] E2580_#2 SRDS[2] 20G_QXGMII */
    { 3, 0xe}, /* PanelPort[47] E2580_#2 SRDS[2] 20G_QXGMII */
    {0xff, 0xff}
};

GT_U16 portToPhyArrayAc5pRd1780[][2] = {
    {30, 0x0},
    {28, 0x1},
    {31, 0x2},
    {29, 0x3},
    {34, 0x4},
    {32, 0x5},
    {35, 0x6},
    {33, 0x7},
    {38, 0x8},
    {36, 0x9},
    {39,0xa},
    {37,0xb},
    {42,0xc},
    {40,0xd},
    {43,0xe},
    {41,0xf},
    {46,0x10},
    {44,0x11},
    {47,0x12},
    {45,0x13},
    {50,0x14},
    {48,0x15},
    {51,0x16},
    {49,0x17},
    {20,0x18},
    {18,0x19},
    {21,0x1a},
    {19,0x1b},
    {24,0x1c},
    {22,0x1d},
    {25,0x1e},
    {23,0x1f},
    {0xff, 0xff}
};

GT_U16 portToPhyArrayAc5xRd2580_SR2[][2] = {
    {32,0x0},
    {33,0x1},
    {34,0x2},
    {35,0x3},
    {36,0x4},
    {37,0x5},
    {38,0x6},
    {39,0x7},
    {0xff, 0xff}
};

GT_U16 portToPhyArrayAc5xRd2540_SR2[][2] = {
    {40,0x8},
    {41,0x9},
    {42,0xa},
    {43,0xb},
    {44,0xc},
    {45,0xd},
    {46,0xe},
    {47,0xf},
    {0xff, 0xff}
};


static struct{
    GT_U8       devNum;
    GT_U32      macNum;
}ifIndexInfoArr[MPD_MAX_PORT_NUMBER_CNS];

static GT_U32 nextFreeGlobalIfIndex = 1;
/* ******* DEBUG ********/
#define MPD_SAMPLE_NUM_OF_DEBUG_FLAGS_CNS       (10)


typedef struct {
        BOOLEAN   isFree;
        char    * flagName_PTR;
        char    * flagHelp_PTR;
        BOOLEAN   flagStatus; /* TRUE - On */
}MPD_SAMPLE_DEBUG_FLAG_INFO_STC;
static MPD_SAMPLE_DEBUG_FLAG_INFO_STC mpdSampleDebugFlags[MPD_SAMPLE_NUM_OF_DEBUG_FLAGS_CNS];
static UINT_32 MPD_SAMPLE_DEBUG_FREE_INDEX = 0;
#endif/* INCLUDE_MPD */
#endif/* ASIC_SIMULATION */

#define MAX_PHY_ADDR_IN_SMI_BUS              32
#define PHY_ID_REG_DEV_NUM_CNS                1
#define PHY_VENDOR_ID_REG_ADDR_CNS            2
#define PHY_ID_REG_ADDR_CNS                   3

/* Marvell PHYs unique vendor ID - register 2 */
#define MRVL_PHY_REV_A_UID_CNS 0x2B
#define MRVL_PHY_UID_CNS 0x141
#define MRVL_PHY_1780_MODEL_NUM_CNS 0x2D
#define MRVL_PHY_2540_MODEL_NUM_CNS 0x3C1 /* 4.2003=0xBC1, bit[9:4]=Model_Num */
#define MRVL_PHY_2580_MODEL_NUM_CNS 0x3C3 /* 4.2003=0xBC3, bit[9:4]=Model_Num */
#define MRVL_PHY_3240_MODEL_NUM_CNS 0x18
#define MRVL_PHY_3340_MODEL_NUM_CNS 0x1A
#define MRVL_PHY_2180_MODEL_NUM_CNS 0x1B
#define MRVL_PHY_7120_MODEL_NUM_CNS 0x3B


#define PHY_TIMEOUT        10000
GT_U16 vendorId;
GT_BOOL checkEnable = GT_TRUE;
GT_U16 phyModelNum;

extern GT_STATUS appDemoBoardTypeGet
(
    OUT GT_U32 *boardTypePtr
);

typedef enum
{
    GT_APPDEMO_PHY_RAM_E = 0,
    GT_APPDEMO_PHY_FLASH_E
}GT_APPDEMO_PHY_DOWNLOAD_MODE_ENT;

GT_STATUS gtAppDemoXPhyVendorIdGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    IN  GT_U32                      xsmiAddr,
    OUT  GT_U16                    *vendorIdPtr
)
{
    GT_STATUS rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC saved_system_recovery;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery;

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }
    saved_system_recovery = system_recovery;

    system_recovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E;
    system_recovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    rc =  cpssSystemRecoveryStateSet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* read vendor ID */
    rc = cpssXsmiPortGroupRegisterRead(devNum, portGroupsBmp, xsmiInterface,
                                        xsmiAddr, PHY_VENDOR_ID_REG_ADDR_CNS,
                                        PHY_ID_REG_DEV_NUM_CNS,
                                        /*OUT*/vendorIdPtr);
    if (rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssXsmiPortGroupRegisterRead", rc);
    }
    rc =  cpssSystemRecoveryStateSet(&saved_system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }
    return rc;
}

GT_STATUS gtAppDemoXPhyIdGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    IN  GT_U32                      xsmiAddr,
    OUT GT_U16                     *phyIdPtr
)
{
    GT_STATUS rc;
    GT_U16    data;
    /*GT_U16    phyRev = 0;  future use for 2180 phy*/

    /* read PHY ID */
    rc = cpssXsmiPortGroupRegisterRead(devNum, portGroupsBmp, xsmiInterface,
                                        xsmiAddr, PHY_ID_REG_ADDR_CNS,
                                        PHY_ID_REG_DEV_NUM_CNS, &data);
    if (rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssXsmiPortGroupRegisterRead", rc);
        return rc;
    }

    if (data != 0xFFFF)
    {
        switch(data){
            case 0xBC1: /* E2540, 0x3C1 */
            case 0xBC3: /* E2580, 0x3C3 */
                *phyIdPtr = (data & 0x3FF);
                phyModelNum = *phyIdPtr;
                break;
            default:
                *phyIdPtr = (data >> 4) & 0x3F;
                phyModelNum = *phyIdPtr;
                break;
        }
    }
    else
        *phyIdPtr = 0;

    return GT_OK;
}

GT_STATUS gtAppDemoPhyIdGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_PHY_SMI_INTERFACE_ENT  smiInterface,
    IN  GT_U32                      smiAddr,
    OUT GT_U16                     *phyIdPtr
)
{
    GT_STATUS rc;
    GT_U16    data;

    /* read PHY ID */
    rc = cpssSmiRegisterReadShort(devNum, portGroupsBmp, smiInterface,
                                        smiAddr, PHY_ID_REG_ADDR_CNS, &data);
    if (rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssSmiRegisterReadShort", rc);
        return rc;
    }

    if (data != 0xFFFF)
    {
        *phyIdPtr = (data >> 4) & 0x3F;
        phyModelNum = *phyIdPtr;
    }
    else
        *phyIdPtr = 0;

    return GT_OK;
}

GT_VOID gtAppDemoXSmiScan
(
    IN  GT_U8   devNum
)
{
    GT_STATUS   rc;                     /* return code */
    GT_U32      portGroupId;            /* local core number */
    GT_PORT_GROUPS_BMP  portGroupsBmp;  /* port groups bitmap */
    CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface;  /* SMI i/f iterator */
    GT_U32      xsmiAddr;               /* SMI Address iterator */
    GT_U16      data;                   /* register data */
    GT_U32      currBoardType;
    GT_U8       maxXSMIinterface=CPSS_PHY_XSMI_INTERFACE_0_E;
    GT_U8       minXSMIinterface=CPSS_PHY_XSMI_INTERFACE_0_E;

    rc = appDemoBoardTypeGet(&currBoardType);
    if (rc != GT_OK)
    {
        cpssOsPrintf("fail to get the BoardType=%d\n", currBoardType);
    }
    else
    {
        /* enable to load PHY_XSMI_INTERFACE_3 */
        if(currBoardType == APP_DEMO_ALDRIN2_BOARD_RD_CNS)
        {
            cpssDrvHwPpResetAndInitControllerWriteReg(0, 0xf8d24, 0x18200ce);
            maxXSMIinterface = CPSS_PHY_XSMI_INTERFACE_3_E;
        }
        else if(currBoardType == APP_DEMO_FALCON_BOARD_12_8_TH_CNS)
        {
            minXSMIinterface = CPSS_PHY_XSMI_INTERFACE_4_E;
            maxXSMIinterface = CPSS_PHY_XSMI_INTERFACE_5_E;
        }
        else if (currBoardType == APP_DEMO_XCAT5X_A0_BOARD_RD_CNS)
        {
            maxXSMIinterface = CPSS_PHY_XSMI_INTERFACE_0_E;
        }
        else
        {
            maxXSMIinterface = CPSS_PHY_XSMI_INTERFACE_1_E;
        }
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        portGroupsBmp = 1<<portGroupId;

        for(xsmiInterface = minXSMIinterface; xsmiInterface <= maxXSMIinterface; xsmiInterface++)
        {
            for(xsmiAddr = 0; xsmiAddr < BIT_5; xsmiAddr++)
            {
                rc = gtAppDemoXPhyVendorIdGet(devNum, portGroupsBmp, xsmiInterface, xsmiAddr, &data);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("gtAppDemoXPhyVendorIdGet FAIL:devNum=%d,portGroupId=%d,xsmiInterface=%d,xsmiAddr=%d,\n",
                                devNum, portGroupId, xsmiInterface, xsmiAddr);
                    continue;
                }
                else
                {
                    cpssOsPrintf("gtAppDemoXPhyVendorIdGet:devNum=%d,portGroupId=%d,xsmiInterface=%d,xsmiAddr=%d,VendorId=0x%x\n",
                        devNum, portGroupId, xsmiInterface, xsmiAddr, data);
                }

                rc = gtAppDemoXPhyIdGet(devNum, portGroupsBmp, xsmiInterface, xsmiAddr, &data);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("gtAppDemoXPhyIdGet FAIL:devNum=%d,portGroupId=%d,xsmiInterface=%d,xsmiAddr=%d\n",
                                 devNum, portGroupId, xsmiInterface, xsmiAddr);
                    continue;
                }
                else
                {
                    cpssOsPrintf("gtAppDemoXPhyIdGet:devNum=%d,portGroupId=%d,xsmiInterface=%d,xsmiAddr=%d,PhyId=0x%x\n",
                                 devNum, portGroupId, xsmiInterface, xsmiAddr, data);
                }
            }
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    return;
}

GT_VOID gtAppDemoSmiPhyAddrArrayBuild
(
    IN  GT_U8                       devNum,
    OUT GT_APPDEMO_XPHY_INFO_STC    *phyInfoArray,
    OUT GT_U32                      *phyInfoArrayLenPtr
)
{
    GT_STATUS   rc;                     /* return code */
    GT_U32      portGroupId;            /* local core number */
    GT_PORT_GROUPS_BMP  portGroupsBmp;  /* port groups bitmap */
    GT_U32      smiAddr;                /* SMI Address iterator */
    GT_U16      data;                   /* register data */
    GT_U32      currBoardType;
    CPSS_PHY_SMI_INTERFACE_ENT smiInterfaceEnum = 0;
    *phyInfoArrayLenPtr = 0;

    rc = appDemoBoardTypeGet(&currBoardType);
    if (rc != GT_OK)
    {
        cpssOsPrintf("fail to get the BoardType=%d\n", currBoardType);
    }

    if ((currBoardType != APP_DEMO_XCAT5P_A0_BOARD_RD_CNS) && (currBoardType != APP_DEMO_XCAT5X_A0_BOARD_RD_CNS))
    {
        /* support only ac5x and ac5p */
        return;
    }

    if (currBoardType == APP_DEMO_XCAT5P_A0_BOARD_RD_CNS)
    {
        smiInterfaceEnum = CPSS_PHY_SMI_INTERFACE_0_E;
    }
    else if (currBoardType == APP_DEMO_XCAT5X_A0_BOARD_RD_CNS)
    {
        smiInterfaceEnum = CPSS_PHY_SMI_INTERFACE_1_E;
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
    {
        portGroupsBmp = 1<<portGroupId;

        for(smiAddr = 0; smiAddr < BIT_5; smiAddr++)
        {
            if (currBoardType == APP_DEMO_XCAT5P_A0_BOARD_RD_CNS)
            {
                data = 0x2D;
            }
            else
            {
                rc = gtAppDemoPhyIdGet(devNum, portGroupsBmp, smiInterfaceEnum, smiAddr, &data);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("gtAppDemoPhyIdGet FAIL:devNum=%d,portGroupId=%d,smiInterface=%d,smiAddr=%d\n",
                                 devNum, portGroupId, smiInterfaceEnum, smiAddr);
                    continue;
                }
            }

            phyInfoArray[*phyInfoArrayLenPtr].hostDevNum = devNum;
            phyInfoArray[*phyInfoArrayLenPtr].portGroupId = portGroupId;
            phyInfoArray[*phyInfoArrayLenPtr].phyAddr = (GT_U16)smiAddr;
            phyInfoArray[*phyInfoArrayLenPtr].xsmiInterface = smiInterfaceEnum;
            phyInfoArray[*phyInfoArrayLenPtr].phyType = data;
            cpssOsPrintf("%d)hostDevNum=%d,portGroupId=%d,phyAddr=0x%x,smiInterface=%d,phyType=0x%x\n",
                         *phyInfoArrayLenPtr,
                         phyInfoArray[*phyInfoArrayLenPtr].hostDevNum,
                         phyInfoArray[*phyInfoArrayLenPtr].portGroupId,
                         phyInfoArray[*phyInfoArrayLenPtr].phyAddr,
                         phyInfoArray[*phyInfoArrayLenPtr].xsmiInterface,
                         phyInfoArray[*phyInfoArrayLenPtr].phyType);
            (*phyInfoArrayLenPtr)++;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    return;
}

GT_VOID gtAppDemoXSmiPhyAddrArrayBuild
(
    IN  GT_U8                       devNum,
    OUT GT_APPDEMO_XPHY_INFO_STC    *phyInfoArray,
    OUT GT_U32                      *phyInfoArrayLenPtr
)
{
    GT_STATUS   rc;                     /* return code */
    GT_U32      portGroupId;            /* local core number */
    GT_PORT_GROUPS_BMP  portGroupsBmp;  /* port groups bitmap */
    CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface;  /* SMI i/f iterator */
    GT_U32      xsmiAddr;               /* SMI Address iterator */
    GT_U16      data;                   /* register data */
    GT_U32      currBoardType;
    GT_U8       maxXSMIinterface=CPSS_PHY_XSMI_INTERFACE_0_E;
    GT_U8       minXSMIinterface=CPSS_PHY_XSMI_INTERFACE_0_E;

    *phyInfoArrayLenPtr = 0;

    rc = appDemoBoardTypeGet(&currBoardType);
    if (rc != GT_OK)
    {
        cpssOsPrintf("fail to get the BoardType=%d\n", currBoardType);
    }
    else
    {
        switch(currBoardType){
            case APP_DEMO_XCAT5P_A0_BOARD_RD_CNS:
                /* RD-AC5P use XSMI[0] + SMI[0] (conflict to XSMI[1])
                 * when code scan to none exist XSMI[1], appDemo crashed.
                 * */
                maxXSMIinterface = CPSS_PHY_XSMI_INTERFACE_0_E;
                break;
            case APP_DEMO_XCAT5X_A0_BOARD_RD_CNS:
                /*RD-AC5X use SMI[1] + XSMI[0] */
                maxXSMIinterface = CPSS_PHY_XSMI_INTERFACE_0_E;
                break;
            case APP_DEMO_ALDRIN2_BOARD_RD_CNS:
                /* enable to load PHY_XSMI_INTERFACE_3 */
                cpssDrvHwPpResetAndInitControllerWriteReg(0, 0xf8d24, 0x18200ce);
                maxXSMIinterface = CPSS_PHY_XSMI_INTERFACE_3_E;
                break;
            default:
                maxXSMIinterface = CPSS_PHY_XSMI_INTERFACE_1_E;
        }
    }

    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        portGroupsBmp = 1<<portGroupId;

        for(xsmiInterface = minXSMIinterface; xsmiInterface <= maxXSMIinterface; xsmiInterface++)
        {
            for(xsmiAddr = 0; xsmiAddr < BIT_5; xsmiAddr++)
            {
                rc = gtAppDemoXPhyVendorIdGet(devNum, portGroupsBmp, xsmiInterface, xsmiAddr, &data);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("gtAppDemoXPhyVendorIdGet FAIL:devNum=%d,portGroupId=%d,xsmiInterface=%d,xsmiAddr=%d,\n",
                                devNum, portGroupId, xsmiInterface, xsmiAddr);
                    continue;
                }

                if ((data != MRVL_PHY_UID_CNS) && (data != MRVL_PHY_REV_A_UID_CNS))
                {
                    continue;
                }
                else if ((data == MRVL_PHY_UID_CNS || data == MRVL_PHY_REV_A_UID_CNS) &&
                         checkEnable == GT_TRUE)
                {
                    vendorId = data;
                    checkEnable = GT_FALSE;
                }
                rc = gtAppDemoXPhyIdGet(devNum, portGroupsBmp, xsmiInterface, xsmiAddr, &data);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("gtAppDemoXPhyIdGet FAIL:devNum=%d,portGroupId=%d,xsmiInterface=%d,xsmiAddr=%d\n",
                                 devNum, portGroupId, xsmiInterface, xsmiAddr);
                    continue;
                }

                phyInfoArray[*phyInfoArrayLenPtr].hostDevNum = devNum;
                phyInfoArray[*phyInfoArrayLenPtr].portGroupId = portGroupId;
                phyInfoArray[*phyInfoArrayLenPtr].phyAddr = (GT_U16)xsmiAddr;
                phyInfoArray[*phyInfoArrayLenPtr].xsmiInterface = xsmiInterface;
                phyInfoArray[*phyInfoArrayLenPtr].phyType = data;
                cpssOsPrintf("%d)hostDevNum=%d,portGroupId=%d,phyAddr=0x%x,xsmiInterface=%d,phyType=0x%x\n",
                             *phyInfoArrayLenPtr,
                             phyInfoArray[*phyInfoArrayLenPtr].hostDevNum,
                             phyInfoArray[*phyInfoArrayLenPtr].portGroupId,
                             phyInfoArray[*phyInfoArrayLenPtr].phyAddr,
                             phyInfoArray[*phyInfoArrayLenPtr].xsmiInterface,
                             phyInfoArray[*phyInfoArrayLenPtr].phyType);
                (*phyInfoArrayLenPtr)++;
            }
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    return;
}

#ifndef ASIC_SIMULATION
#ifndef INCLUDE_MPD
GT_STATUS gtAppDemoXPhyFwDownloadSingle
(
    GT_U32  devNum,
    GT_U32  portGroup,
        CPSS_PHY_XSMI_INTERFACE_ENT  xsmiInterface,
    GT_U32  phyType,
    GT_U16  startPhyAddr,
    GT_U16  endPhyAddr,
    GT_APPDEMO_PHY_DOWNLOAD_MODE_ENT mode
)
{
    GT_U8           *buffer = NULL, *buffer_slave = NULL;
    GT_U32          nmemb, nmemb_slave;
    MTD_BOOL        isPhyRead;
    GT_U32          timeout;
    int             ret;
    GT_U16          status;
    MTD_BOOL        tunitReady;
    CPSS_OS_FILE_TYPE_STC        *fp, *fp_slave;

    GT_U16 phyAddr[32] = {  0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0 };

    GT_U16 errorPorts[32] = {   0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0 };

    GT_PHYSICAL_PORT_NUM numPort = 0;
    GT_U32 i;
    GT_U8 major, minor, inc, test;
    PRESTERA_INFO switchInfo;

    char file_name[64];
    char file_name_slave[64];

    fp = cpssOsMalloc (sizeof(CPSS_OS_FILE_TYPE_STC));
    fp_slave = cpssOsMalloc (sizeof(CPSS_OS_FILE_TYPE_STC));
    /* !!! Pay attention - if you change names of files here you must change
        them in Makefile too for standalone version build !!! */
    if(MRVL_PHY_3240_MODEL_NUM_CNS == phyType)
    {/* important - see comment about file name above */
        cpssOsSprintf(file_name, "%s", "88X32xx-FW.hdr");
        cpssOsSprintf(file_name_slave, "%s", "3240_0400_6270.bin");
    }
    else if(MRVL_PHY_3340_MODEL_NUM_CNS == phyType && MRVL_PHY_UID_CNS == vendorId)
    {/* important - see comment about file name above */
        cpssOsSprintf(file_name, "%s", "88X33xx-Z2-FW.hdr");
        cpssOsSprintf(file_name_slave, "%s", "flashdownloadslave.bin");
    }
    else if (MRVL_PHY_3340_MODEL_NUM_CNS == phyType && MRVL_PHY_REV_A_UID_CNS == vendorId)
    {
        cpssOsSprintf(file_name, "%s", "x3310fw_0_2_8_0_8850.hdr");
    }

    else if (MRVL_PHY_2180_MODEL_NUM_CNS == phyType && MRVL_PHY_REV_A_UID_CNS == vendorId)
    {
        cpssOsSprintf(file_name, "%s", "e21x0fw_8_0_0_36_8923.hdr");
    }
    else
    {
        cpssOsPrintf("not supported PHY type\n");
        return GT_NOT_SUPPORTED;
    }

    /* create phy address array */
    if (GT_APPDEMO_PHY_RAM_E == mode)
    {
        numPort = endPhyAddr - startPhyAddr + 1;
        for (i = 0; i < numPort; i++)
        {
            phyAddr[i] = (i + startPhyAddr);
        }
    }
    else /* for SPI flash, only first address of each PHY */
    {
        /* This option not functional on Lewis RD board with PHY 3240.
            Get: "Expected 0x0100 from slave got 0x0400 on port" for
            every port we tried
         */
        numPort = (endPhyAddr - startPhyAddr + 1) / 4;
        for (i = 0; i < numPort; i++)
        {
            phyAddr[i] = (i * 4) + startPhyAddr;
            if(3240 == phyType)
            {
                phyAddr[i] += 2;
            }
        }
    }

    if (numPort >= 32)
    {
        cpssOsPrintf("numPort[%d]>=32\n", numPort);
        return GT_BAD_PARAM;
    }

    if (0 == numPort)
    {
        cpssOsPrintf("0 == numPort\n");
        return GT_BAD_PARAM;
    }

    cpssOsPrintf("Load firmware file %s\n", file_name);

    fp->type = CPSS_OS_FILE_REGULAR;
    fp->fd = cpssOsFopen(file_name, "r",fp);
    if (0 == fp->fd)
    {
        cpssOsPrintf("open %s fail \n", file_name);
        return GT_ERROR;
    }

    nmemb = cpssOsFgetLength(fp->fd);
    if (nmemb <= 0)
    {
        cpssOsPrintf("open %s fail \n", file_name);
        return GT_ERROR;
    }

    /* for store image */
    buffer = cpssOsMalloc(216 * 1024);
    if (NULL == buffer)
    {
        cpssOsPrintf(" buffer allocation fail \n");
        return GT_ERROR;
    }

    ret = cpssOsFread(buffer, 1, nmemb, fp);
    if (ret < 0)
    {
        cpssOsPrintf("Read from file fail\n");
        return GT_ERROR;
    }

    switchInfo.devNum = devNum;
    switchInfo.portGroup = portGroup;
        switchInfo.xsmiInterface = xsmiInterface;
    fp_slave->type = CPSS_OS_FILE_REGULAR;
    if (GT_APPDEMO_PHY_FLASH_E == mode)   /* load slave .bin for SPI download */
    {
        cpssOsPrintf("Load slave bin file %s\n", file_name_slave);
        fp_slave->fd = cpssOsFopen(file_name_slave, "r", fp_slave);
        if (0 == fp_slave->fd)
        {
            cpssOsPrintf("open %s fail \n", file_name_slave);
            return GT_ERROR;
        }

        nmemb_slave = cpssOsFgetLength(fp_slave->fd);
        if (nmemb_slave <= 0)
        {
            cpssOsPrintf("slave bin length fail \n");
            return GT_ERROR;
        }

        /* 20KB for store slave image */
        buffer_slave = cpssOsMalloc(20 * 1024);
        if (NULL == buffer_slave)
        {
            cpssOsPrintf(" slave buffer allocation fail \n");
            return GT_ERROR;
        }

        ret = cpssOsFread(buffer_slave, 1, nmemb_slave, fp_slave);
        if (ret < 0)
        {
            cpssOsPrintf("Read from slave bin file fail\n");
            return GT_ERROR;
        }

        cpssOsPrintf("MTL downloading firmware to SPI flash phyAddr=%x,numPort=%d ...", phyAddr[0], numPort);
        (GT_VOID)mtdParallelUpdateFlashImage(&switchInfo, phyAddr, buffer, nmemb,
                                                buffer_slave, nmemb_slave,
                                                (GT_U16)numPort, errorPorts,
                                                &status);
    }
    else            /* RAM */
    {
        cpssOsPrintf("MTL downloading firmware to ram phyAddr=%x,numPort=%d ...", phyAddr[0], numPort);
        (GT_VOID)mtdParallelUpdateRamImage(&switchInfo, phyAddr, buffer, nmemb,
                                            (GT_U16)numPort, errorPorts,
                                            &status);
    }

    status &= 0xFFFF;
    if (status == 0)
    {
        cpssOsPrintf("OK");
        (GT_VOID)mtdGetFirmwareVersion(&switchInfo, phyAddr[0], &major, &minor, &inc, &test);
        cpssOsPrintf(" - firmware ver = %u.%u.%u.%u\n", major, minor, inc, test);

        timeout = 0;
        while (timeout <= PHY_TIMEOUT)
        {
            if (timeout == PHY_TIMEOUT)
            {
                cpssOsPrintf("phy app is not ready before timeout \n ");
                return GT_TIMEOUT;
            }
            /* check phy code status, ready = MTD_TRUE */
            mtdDidPhyAppCodeStart(&switchInfo, phyAddr[0], &isPhyRead);
            if (isPhyRead == MTD_TRUE)
                    break;
            timeout++;
            cpssOsTimerWkAfter(1);
        }
    }
    else
    {
        cpssOsPrintf("FAIL , status = 0x%x\n", status);
        cpssOsPrintf("MTL phy status = ");
        for (i = 0; i < numPort; i++)
            cpssOsPrintf("%4x, ", errorPorts[i]);
        cpssOsPrintf("\n");
        return GT_ERROR;
    }

    cpssOsFree(buffer);
    cpssOsFclose(fp);
    cpssOsFree(fp);
    cpssOsFree(fp_slave);

    if (GT_APPDEMO_PHY_FLASH_E == mode)
    {
        cpssOsFree(buffer_slave);
        cpssOsFclose(fp_slave);
    }

        for(i=0;i < numPort;i++)
        {
                /*
                        set Host side XFI / SGMII-AN
                        active lane 0
                        MAC side always power up
                        do software reset after config
                */
                ret = mtdSetMacInterfaceControl(&switchInfo,
                                                                                        phyAddr[i],
                                                                                MTD_MAC_TYPE_XFI_SGMII_AN_EN,
                                                                                MTD_TRUE, /* TRUE = mac interface always power up */
                                                                                MTD_MAC_SNOOP_OFF,
                                                                                0,
                                                                                MTD_MAC_SPEED_10_MBPS,
                                                                                MTD_TRUE);
                if (ret != GT_OK)
                {
                                cpssOsPrintf("mtdSetMacInterfaceControl port %u fail\n", phyAddr[i]);
                        return ret;
                }

                /*
                                set Line side AMD, prefer copper
                                F2R mode = b'00  (OFF)
                                no energy detect
                                no max power AMD
                                do software reset after config
                */
                ret = mtdSetCunitTopConfig(&switchInfo,
                                        phyAddr[i],
                                                                        MTD_F2R_OFF,
                                                                        MTD_MS_AUTO_PREFER_CU,
                                                                        MTD_FT_10GBASER,
                                                                        MTD_FALSE,
                                                                        MTD_FALSE,
                                                                        MTD_TRUE);
                if (ret != GT_OK)
                {
                                cpssOsPrintf("mtdSetCunitTopConfig port %u fail\n", phyAddr[i]);
                        return ret;
                }

                /* LED control */
                        ret = cpssXsmiPortGroupRegisterWrite(0, 0, xsmiInterface,  phyAddr[i],  0xf022, 31,  0x29);
                if (ret != GT_OK)
                {
                        cpssOsPrintf("cpssXsmiPortGroupRegisterWrite(0, 0, %u,  %u,  0xf022, 31,  0x29):ret=%d\n",
                                                xsmiInterface, phyAddr[i], ret);
                        return ret;
                }
                        ret = cpssXsmiPortGroupRegisterWrite(0, 0, xsmiInterface,  phyAddr[i],  0xf023, 31,  0x31);
                if (ret != GT_OK)
                {
                        cpssOsPrintf("cpssXsmiPortGroupRegisterWrite(0, 0, %u,  %u,  0xf023, 31,  0x31):ret=%d\n",
                                                xsmiInterface, phyAddr[i], ret);
                        return ret;
                }
                ret = cpssXsmiPortGroupRegisterWrite(0, 0, xsmiInterface,  phyAddr[i],  0xf015, 31,  0x8800);
                if (ret != GT_OK)
                {
                        cpssOsPrintf("cpssXsmiPortGroupRegisterWrite(0, 0, %u,  %u,  0xf015, 31,  0x31):ret=%d\n",
                                                xsmiInterface, phyAddr[i], ret);
                        return ret;
                }
                /* power up phy */
                        if ((ret = mtdRemovePhyLowPowerMode(&switchInfo, phyAddr[i])) != GT_OK)
                {
                                cpssOsPrintf("mtdRemovePhyLowPowerMode of port %u fail\n", phyAddr[i]);
                        return ret;
                }

                /* release note 4.6  do T-unit reset, (1.0x0.15) */
                for (timeout = 0; timeout < PHY_TIMEOUT; timeout++)
                {
                        ret = mtdIsTunitResponsive(&switchInfo, phyAddr[i], &tunitReady);
                        if (tunitReady == MTD_TRUE)
                                        break;

                        cpssOsTimerWkAfter(1);
                }
                /* after flash burn on 3240 PHY timeout happens, but FW is burned and functional after HW reset */
                if (timeout == PHY_TIMEOUT)
                {
                                cpssOsPrintf("mtdIsTunitResponsive timeout:phyAddr=0x%x\n", phyAddr[i]);
                        return GT_TIMEOUT;
                }

                ret = mtdSoftwareReset(&switchInfo, phyAddr[i],100);
                if (ret != GT_OK)
                {
                        cpssOsPrintf("mtdSoftwareReset port %u fail\n", phyAddr[i]);
                        return ret;
                }
        }
    return GT_OK;
}

GT_STATUS gtAppDemoXPhyFwDownload
(
    IN  GT_U8   devNum
)
{
    GT_STATUS                     rc;
    GT_APPDEMO_XPHY_INFO_STC    phyInfoArray[256];
    GT_APPDEMO_XPHY_INFO_STC    phyInfo3340[256];
    GT_U32                      i;
    GT_U32                      phyInfoArrayLen = 0;
    GT_U32                      phyInfo3340Len = 0;
    GT_U32                      xSmiInterface = 0;
    GT_U32                      startAddr, endAddr;
    GT_U32                      initPhy;
    GT_APPDEMO_PHY_DOWNLOAD_MODE_ENT phyFwLoadMode;

    rc = appDemoDbEntryGet("initPhy", &initPhy);
    if (rc != GT_OK)
    {
        initPhy = 1;
    }

    if (!initPhy)
    {
        return GT_OK;
    }

    rc = appDemoDbEntryGet("phyFwLoadMode", &phyFwLoadMode);
    if (rc != GT_OK)
    {
        phyFwLoadMode = GT_APPDEMO_PHY_RAM_E;
    }
    if (GT_APPDEMO_PHY_FLASH_E == phyFwLoadMode)
    {
        return GT_NOT_IMPLEMENTED;
    }
    gtAppDemoXSmiPhyAddrArrayBuild(devNum, phyInfoArray, &phyInfoArrayLen);

    for (i = 0; i < phyInfoArrayLen; i++)
    {
        if(MRVL_PHY_3240_MODEL_NUM_CNS == phyInfoArray[i].phyType)
        {
            rc = gtAppDemoXPhyFwDownloadSingle(devNum, 0 /* portGroup */, phyInfoArray[i].xsmiInterface,
                                                MRVL_PHY_3240_MODEL_NUM_CNS,
                                                phyInfoArray[i].phyAddr,
                                                phyInfoArray[i].phyAddr,
                                                phyFwLoadMode);
            if (rc != GT_OK)
            {
                cpssOsPrintf("gtAppDemoXPhyFwDownloadSingle:rc=%d\n", rc);
                return rc;
            }
        }
        /* phy 88E2180 is a sub family of 3340*/
        else if((MRVL_PHY_3340_MODEL_NUM_CNS == phyInfoArray[i].phyType) ||
                (MRVL_PHY_2180_MODEL_NUM_CNS == phyInfoArray[i].phyType))
        {
            osMemCpy(&(phyInfo3340[phyInfo3340Len]), &(phyInfoArray[i]), sizeof(GT_APPDEMO_XPHY_INFO_STC));
            phyInfo3340Len++;
        }
        else
        {
            return GT_NOT_SUPPORTED;
        }
    }
    if (phyInfo3340Len != 0) {
        for (i=0; i<phyInfo3340Len;) {
            xSmiInterface = phyInfo3340[i].xsmiInterface;
            startAddr = 0;
            endAddr = 0;
            while ((phyInfo3340[i].xsmiInterface == xSmiInterface) && (i<phyInfo3340Len)) {
                if (startAddr>=phyInfo3340[i].phyAddr)
                {
                    startAddr = phyInfo3340[i].phyAddr;
                }
                if (endAddr<=phyInfo3340[i].phyAddr)
                {
                    endAddr = phyInfo3340[i].phyAddr;
                }
                i++;
            }
            rc = gtAppDemoXPhyFwDownloadSingle(devNum, 0 /* portGroup */, xSmiInterface,
                                                phyModelNum,
                                                startAddr,
                                                endAddr,
                                                phyFwLoadMode);
            if (rc != GT_OK)
            {
                cpssOsPrintf("gtAppDemoXPhyFwDownloadSingle:rc=%d\n", rc);
                return rc;
            }
        }
    }

    return GT_OK;
}
#else /* !INCLUDE_MPD */

/*********************************** NEW APIs ************************************/

BOOLEAN gtAppDemoPhyOsDelay
(
     IN UINT_32 delay
)
{
    struct timespec sleep, remain;
    if (1000000000 < delay) {
    /* sleep time to big */
        return 0;
    }

    sleep.tv_nsec = delay*1000000;
    sleep.tv_sec = 0;
    if (nanosleep(  &sleep, &remain)) {
        return TRUE;
    }

    return FALSE;
}

void * gtAppDemoPhyOsMalloc
(
     UINT_32  size
)
{
        return cpssOsMalloc(size);
}
void gtAppDemoPhyOsFree
(
     void* data
)
{
    return cpssOsFree(data);
}

BOOLEAN gtAppDemoPhyXsmiMdioWrite (
    IN UINT_32      rel_ifIndex,
    IN UINT_8       mdioAddress,
    IN UINT_16      deviceOrPage,   /* PRV_MPD_IGNORE_PAGE_CNS */
    IN UINT_16      address,
    IN UINT_16      value
)
{
    UINT_8          dev, port;
    UINT_8          xsmiInterface, smiInterface;
    UINT_32         portGroup, portGroupsBmp;
    PRV_MPD_PORT_HASH_ENTRY_STC * port_entry_PTR;
    GT_STATUS       status = GT_OK;

    port_entry_PTR = prvMpdGetPortEntry(rel_ifIndex);
    if (port_entry_PTR == NULL) {
        return FALSE;
    }

    if ((UINT_32)port_entry_PTR->initData_PTR->phyType == MPD_TYPE_INVALID_E) {
        return FALSE;
    }

    port = port_entry_PTR->initData_PTR->port;
    dev = port_entry_PTR->initData_PTR->mdioInfo.mdioDev;
    portGroup = MPD_PORT_NUM_TO_GROUP_MAC(port);
    portGroupsBmp = MPD_PORT_GROUP_TO_PORT_GROUP_BMP_MAC(portGroup);

    if (port_entry_PTR->initData_PTR->phyType != MPD_TYPE_88E1780_E)
    {
        xsmiInterface = port_entry_PTR->initData_PTR->mdioInfo.mdioBus;
        status = cpssXsmiPortGroupRegisterWrite( dev,
                                                 portGroupsBmp,
                                                 xsmiInterface,
                                                 mdioAddress,
                                                 address,
                                                 deviceOrPage,
                                                 value);
    }
    else
    {
        smiInterface = port_entry_PTR->initData_PTR->mdioInfo.mdioBus;
        status = cpssSmiRegisterWriteShort( dev,
                                        portGroupsBmp,
                                        smiInterface,
                                        mdioAddress,
                                        address,
                                        value);
    }
    if (status != GT_OK){
        return FALSE;
    }
    return TRUE;
}

BOOLEAN gtAppDemoPhyXsmiMdioRead (
        IN  UINT_32      rel_ifIndex,
        IN  UINT_8       mdioAddress,
        IN  UINT_16      deviceOrPage,
        IN  UINT_16      address,
        OUT UINT_16    * value_PTR
)
{
    UINT_8          dev, port;
    UINT_8          xsmiInterface, smiInterface;
    UINT_32         portGroup, portGroupsBmp;
    PRV_MPD_PORT_HASH_ENTRY_STC * port_entry_PTR;
    GT_STATUS       status = GT_OK;

    port_entry_PTR = prvMpdGetPortEntry(rel_ifIndex);
    if (port_entry_PTR == NULL) {
        return FALSE;
    }

    if ((UINT_32)port_entry_PTR->initData_PTR->phyType == MPD_TYPE_INVALID_E) {
        return FALSE;
    }

    port = port_entry_PTR->initData_PTR->port;
    dev = port_entry_PTR->initData_PTR->mdioInfo.mdioDev;
    xsmiInterface = port_entry_PTR->initData_PTR->mdioInfo.mdioBus;
    portGroup = MPD_PORT_NUM_TO_GROUP_MAC(port);
    portGroupsBmp = MPD_PORT_GROUP_TO_PORT_GROUP_BMP_MAC(portGroup);

    if (port_entry_PTR->initData_PTR->phyType != MPD_TYPE_88E1780_E)
    {
         xsmiInterface = port_entry_PTR->initData_PTR->mdioInfo.mdioBus;
         status = cpssXsmiPortGroupRegisterRead( dev,
                                            portGroupsBmp,
                                            xsmiInterface,
                                            mdioAddress,
                                            address,
                                            deviceOrPage,
                                            value_PTR);
    }
    else
    {
       smiInterface = port_entry_PTR->initData_PTR->mdioInfo.mdioBus;
       status = cpssSmiRegisterReadShort( dev,
                                            portGroupsBmp,
                                            smiInterface,
                                            mdioAddress,
                                            address,
                                            value_PTR);

        if (status != GT_OK){
            return FALSE;
        }
    }
    if (status != GT_OK){
        return FALSE;
    }
    return TRUE;
}

/* mpdSampleGetFwFiles */
BOOLEAN gtAppDemoPhyGetFwFiles
(
    IN  MPD_TYPE_ENT          phyType,
    OUT MPD_FW_FILE_STC   * mainFile_PTR
)
{
    char * fwFileName_PTR = NULL;
    FILE * fp;
    int ret;
    UINT_32 fileSize;

    switch(phyType)
    {
        case MPD_TYPE_88E2540_E:
            fwFileName_PTR = "v0A0A0000_11508_11488_e2540.hdr";
            break;
        case MPD_TYPE_88E2580_E:
            fwFileName_PTR = "v0A0A0000_11508_11488_e2580.hdr";
            break;
        case MPD_TYPE_88X3540_E:
            fwFileName_PTR = "samples/resources/v0A030000_10979_10954_x3540.hdr";
            break;
        case MPD_TYPE_88X32x0_E:
            fwFileName_PTR    = "88X32xx-FW.hdr";
            break;
        case MPD_TYPE_88X33x0_E:
             if(vendorId == MRVL_PHY_UID_CNS)
             {
                fwFileName_PTR    = "88X33xx-Z2-FW.hdr";
             }
             else if(vendorId == MRVL_PHY_REV_A_UID_CNS)
             {
                 fwFileName_PTR = "x3310fw_0_2_8_0_8850.hdr";
             }
             break;
        case MPD_TYPE_88X2180_E:
            if(vendorId == MRVL_PHY_REV_A_UID_CNS)
            {
                fwFileName_PTR = "e21x0fw_8_0_0_36_8923.hdr";
            }
            break;
        default:
            cpssOsPrintf("not supported PHY type\n");
            break;
    }
    if (fwFileName_PTR == NULL)
    {
            return FALSE;
    }

    fp = fopen(fwFileName_PTR,"r");
    if (fp)
    {
        fseek(fp, 0 , SEEK_END);
        fileSize = ftell(fp);
        fseek(fp, 0 , SEEK_SET);
        mainFile_PTR->dataSize = fileSize;
        mainFile_PTR->data_PTR = gtAppDemoPhyOsMalloc(fileSize);

        if (mainFile_PTR->data_PTR == NULL)
        {
            fclose(fp);
            return FALSE;
        }
        ret = fread(mainFile_PTR->data_PTR, sizeof(char), fileSize, fp);
        fclose(fp);
        if (ret <= 0)
        {
            cpssOsPrintf("Read from file fail\n");
            return GT_ERROR;
        }
    }
    return TRUE;
}

MPD_RESULT_ENT gtAppDemoPhyHandleFailure(
        IN UINT_32                              rel_ifIndex,
        IN MPD_ERROR_SEVERITY_ENT       severity,
        IN UINT_32                                      line,
        IN const char                           * calling_func_PTR,
        IN const char                           * error_text_PTR
)
{
     char * severityText[MPD_ERROR_SEVERITY_MINOR_E+1] = {"Fatal", "Error", "Minor"};
     cpssOsPrintf("Failure Level [%s] on port [%d] line [%d] called by [%s].  %s\n",
                (severity<=MPD_ERROR_SEVERITY_MINOR_E)?severityText[severity]:"Unknown",
                 rel_ifIndex,
                 line,
                 calling_func_PTR,
                 error_text_PTR);

        if (severity == MPD_ERROR_SEVERITY_FATAL_E){
            cpssOsPrintf("Fatal\n");
        }

        return MPD_OK_E;
}

MPD_RESULT_ENT gtAppDemoPhyPrintLog(
    const char              * log_text_PTR
)
{
    cpssOsPrintf(log_text_PTR);
    return MPD_OK_E;
}

BOOLEAN gtAppDemoPhyDebugBind (
    IN const char   * comp_PTR,
        IN const char   * pkg_PTR,
        IN const char   * flag_PTR,
        IN const char   * help_PTR,
    OUT UINT_32         * flagId_PTR
)
{
    UINT_32 len = 0,index;

    if (MPD_SAMPLE_DEBUG_FREE_INDEX >= MPD_SAMPLE_NUM_OF_DEBUG_FLAGS_CNS) {
            /* no more debug flags */
            return FALSE;
    }
    index = MPD_SAMPLE_DEBUG_FREE_INDEX++;
    mpdSampleDebugFlags[index].isFree = FALSE;
    mpdSampleDebugFlags[index].flagStatus = FALSE;
#if 0  /* Enable for logs */
    if(!strcmp(flag_PTR, "write") || !strcmp(flag_PTR, "read"))
    {
        mpdSampleDebugFlags[index].flagStatus = TRUE;
    }
#endif
    if(strcmp(flag_PTR,"fw") == 0)
    {
        /*cpssOsPrintf("Firmware logs enabled\n");*/
        mpdSampleDebugFlags[index].flagStatus = TRUE;
    }
    len += (comp_PTR != NULL)?strlen(comp_PTR):0;
    len += (pkg_PTR != NULL)?strlen(pkg_PTR):0;
    len += (flag_PTR != NULL)?strlen(flag_PTR):0;

    if (len)
    {
        len += 6;
        mpdSampleDebugFlags[index].flagName_PTR = gtAppDemoPhyOsMalloc(len+1/*room for '\0'*/);
        mpdSampleDebugFlags[index].flagName_PTR[0] = '\0';
        strcat(mpdSampleDebugFlags[index].flagName_PTR,comp_PTR);
        strcat(mpdSampleDebugFlags[index].flagName_PTR,"-->");
        strcat(mpdSampleDebugFlags[index].flagName_PTR,pkg_PTR);
        strcat(mpdSampleDebugFlags[index].flagName_PTR,"-->");
        strcat(mpdSampleDebugFlags[index].flagName_PTR,flag_PTR);

    }
    len = (help_PTR != NULL)?strlen(help_PTR):0;
    if (len)
    {
        mpdSampleDebugFlags[index].flagHelp_PTR = gtAppDemoPhyOsMalloc(len);
        mpdSampleDebugFlags[index].flagHelp_PTR[0] = '\0';
        strncpy(mpdSampleDebugFlags[index].flagHelp_PTR,help_PTR,len);
    }
    *flagId_PTR = index;
    return TRUE;
}

/**
 * @brief   example implementation of debug check is Flag active
 *
 * @return Active / InActive
 */
BOOLEAN gtAppDemoPhyDebugIsActive (
    IN UINT_32  flagId
)
{
    if (flagId >= MPD_SAMPLE_NUM_OF_DEBUG_FLAGS_CNS)
    {
        return FALSE;
    }
    if (mpdSampleDebugFlags[flagId].isFree == TRUE)
    {
        return FALSE;
    }
    return mpdSampleDebugFlags[flagId].flagStatus;
}

/**
 * @brief   example implementation of debug log (print)
 *
 */

void gtAppDemoPhyDebugLog (
    IN const char    * funcName_PTR,
    IN const char    * format_PTR,
    IN ...
)
{

        va_list      argptr;

        printf("%s:",funcName_PTR);
        va_start(argptr, format_PTR);
        vprintf( format_PTR, argptr);
        va_end(argptr);
        printf("\n");
}

static GT_STATUS gtAppDemoPhyPortNumGet
(
    IN  GT_U32                      currBoardType,
    IN  GT_U8                       boardRevId,
    IN  GT_APPDEMO_XPHY_INFO_STC    phyInfo,
    OUT GT_U32                      *portNum
)
{
    GT_U32  i;
    GT_U32  portStart = 0;
    GT_U32  portEnd = 0;
    GT_U16  *portToPhyArray = NULL;

    switch(currBoardType)
    {
       case APP_DEMO_XCAT5P_A0_BOARD_RD_CNS:
             if(phyInfo.phyType == MRVL_PHY_2580_MODEL_NUM_CNS)
             {
                 portToPhyArray = &portToPhyArrayAc5pRd2580_SR1[0][0];
                 portStart = 2;
                 portEnd   = 17;
             }
             if(phyInfo.phyType == MRVL_PHY_1780_MODEL_NUM_CNS)
             {
                 portToPhyArray = &portToPhyArrayAc5pRd1780[0][0];
                 portStart = 18;
                 portEnd   = 51;
             }
            break;

        case APP_DEMO_XCAT5X_A0_BOARD_RD_CNS:
             if(phyInfo.phyType == MRVL_PHY_1780_MODEL_NUM_CNS)
             {
                 portToPhyArray = &portToPhyArrayAc5xRd1780[0][0];
                 portStart = 0;
                 portEnd   = 31;
             }

             if(phyInfo.phyType == MRVL_PHY_2580_MODEL_NUM_CNS)
             {
                 if (boardRevId == 3)
                 {
                      portToPhyArray = &portToPhyArrayAc5xRd2580_SR2[0][0];
                      portStart = 32;
                      portEnd   = 39;
                 }
             }
             if(phyInfo.phyType == MRVL_PHY_2540_MODEL_NUM_CNS)
             {
                 if (boardRevId == 3)
                 {
                     portToPhyArray = &portToPhyArrayAc5xRd2540_SR2[0][0];
                     portStart = 40;
                     portEnd   = 47;
                 }
                 else if (boardRevId == 2)
                 {
                     portToPhyArray = &portToPhyArrayAc5xRd2540_SR1[0][0];
                     portStart = 32;
                     portEnd   = 47;
                 }
             }
            break;
        case APP_DEMO_ALDRIN_BOARD_RD_CNS:
             portToPhyArray = &portToPhyArrayAldrinRd[0][0];
             switch(phyInfo.xsmiInterface)
             {
                 case CPSS_PHY_XSMI_INTERFACE_0_E:
                     portStart = 20;
                     portEnd   = 31;
                     break;
                 case CPSS_PHY_XSMI_INTERFACE_1_E:
                     portStart = 8;
                     portEnd   = 19;
                     break;
                 default:
                     break;
             }
            break;
        case APP_DEMO_ALDRIN2_BOARD_RD_CNS:
             portToPhyArray = &portToPhyArrayAldrin2Rd[0][0];
             switch(phyInfo.xsmiInterface)
             {
                 case CPSS_PHY_XSMI_INTERFACE_0_E:
                     portStart = 24;
                     portEnd   = 35;
                     break;
                 case CPSS_PHY_XSMI_INTERFACE_1_E:
                     portStart = 36;
                     portEnd   = 47;
                     break;
                 case CPSS_PHY_XSMI_INTERFACE_2_E:
                     portStart = 48;
                     portEnd   = 64;
                     break;
                 case CPSS_PHY_XSMI_INTERFACE_3_E:
                     portStart = 65;
                     portEnd   = 76;
                     break;
                 default:
                     break;
             }
            break;
        case APP_DEMO_XCAT3X_TB2_48_CNS:
             if(phyInfo.phyType == MRVL_PHY_2180_MODEL_NUM_CNS)
             {
                 portToPhyArray = &portToPhyArrayAc3xTb2_1[0][0];
                 portStart = 8;
                 portEnd = 11;
             }
             else
             {
                 portToPhyArray = &portToPhyArrayAc3xTb2_0[0][0];
                 portStart = 0;
                 portEnd = 1;
             }
            break;
        case APP_DEMO_CETUS_BOARD_LEWIS_RD_CNS:
        case APP_DEMO_CETUS_BOARD_DB_CNS:
             portToPhyArray = &portToPhyArrayCetusLewisRd[0][0];
             portStart = 56;
             portEnd = 71;
            break;
        default:
             return GT_FAIL;
    }
    for(i = 0; portToPhyArray[i] != 0xff ; i=i+2)
    {
        if(portToPhyArray[i] < portStart || portToPhyArray[i]  > portEnd)
        {
            continue;
        }
        if(portToPhyArray[i+1] == phyInfo.phyAddr)
        {
            *portNum = portToPhyArray[i];
        }
     }
    return GT_OK;
}

GT_U32 gtAppDemoPhyMpdIndexGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portNum
)
{
   GT_U32 index;

   for(index = 1; index < nextFreeGlobalIfIndex; index++)
   {
       if(ifIndexInfoArr[index].devNum == devNum &&
          ifIndexInfoArr[index].macNum == portNum)
       {
           return index;
       }
   }
   ifIndexInfoArr[nextFreeGlobalIfIndex].devNum  = devNum;
   ifIndexInfoArr[nextFreeGlobalIfIndex].macNum = portNum;
   return nextFreeGlobalIfIndex++;
}

MPD_TYPE_ENT gtAppDemoMpdPhyTypeGet
(
    GT_U32 phyType
)
{
    switch(phyType)
    {
        case MRVL_PHY_2540_MODEL_NUM_CNS:
            return MPD_TYPE_88E2540_E;
        case MRVL_PHY_2580_MODEL_NUM_CNS:
            return MPD_TYPE_88E2580_E;
        case MRVL_PHY_1780_MODEL_NUM_CNS:
            return MPD_TYPE_88E1780_E;
        case MRVL_PHY_3240_MODEL_NUM_CNS:
            return MPD_TYPE_88X32x0_E;
        case MRVL_PHY_3340_MODEL_NUM_CNS:
            return MPD_TYPE_88X33x0_E;
        case MRVL_PHY_2180_MODEL_NUM_CNS:
            return MPD_TYPE_88X2180_E;
        case MRVL_PHY_7120_MODEL_NUM_CNS:
            return MPD_TYPE_88X7120_E;
        default:
            return MPD_TYPE_INVALID_E;
    }
}

static GT_BOOL appDemoPhyMpdInitDone = GT_FALSE;
static GT_APPDEMO_XPHY_INFO_STC phyInfoArray_Ac3x_Tb2[16];
static GT_U32 phyInfoArrayLen_Ac3x_Tb2 = 0;

GT_STATUS gtAppDemoPhyMpdInit
(
    GT_U8 devNum,
    GT_U8 boardRevId
)
{
    UINT_32                                     rel_ifIndex, i;
    MPD_RESULT_ENT                              rc = MPD_OK_E;
    MPD_CALLBACKS_STC                           phy_callbacks;
    MPD_PORT_INIT_DB_STC                        phy_entry;
    GT_U32                                      portNum = 0;
    GT_APPDEMO_XPHY_INFO_STC                    xsmiPhyInfoArray[256];
    GT_APPDEMO_XPHY_INFO_STC                    smiPhyInfoArray[256];
    GT_U32                                      xsmiPhyInfoArrayLen = 0, smiPhyInfoArrayLen = 0;
    GT_U32                                      initPhy;
    GT_U32                                      currBoardType;
    MPD_TYPE_ENT                                phyType;
    GT_U32                                      phyNumber = 0;
    MPD_OPERATIONS_PARAMS_UNT                   opParams;

    if (appDemoPhyMpdInitDone)
    {
        return GT_OK;
    }

    memset(xsmiPhyInfoArray, 0, sizeof(xsmiPhyInfoArray));
    memset(smiPhyInfoArray, 0, sizeof(smiPhyInfoArray));

    gtAppDemoXSmiPhyAddrArrayBuild(devNum, xsmiPhyInfoArray, &xsmiPhyInfoArrayLen);
    gtAppDemoSmiPhyAddrArrayBuild(devNum,  smiPhyInfoArray, &smiPhyInfoArrayLen);

    rc = appDemoDbEntryGet("initPhy", &initPhy);
    if (rc != GT_OK)
    {
        initPhy = 1;
    }

    if((!initPhy) || (xsmiPhyInfoArrayLen == 0 && smiPhyInfoArrayLen == 0))
    {
        cpssOsPrintf("Skip Phy Init.... \n");
        return GT_OK;
    }

    rc = appDemoBoardTypeGet(&currBoardType);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(currBoardType == APP_DEMO_XCAT3X_TB2_48_CNS)
    {
        for(i = 0; i < xsmiPhyInfoArrayLen; i++)
        {
            phyType = gtAppDemoMpdPhyTypeGet(xsmiPhyInfoArray[i].phyType);
            if(phyType == MPD_TYPE_88X2180_E)
            {
                phyNumber = 1;
            }
            phyInfoArray_Ac3x_Tb2[phyInfoArrayLen_Ac3x_Tb2] = xsmiPhyInfoArray[i];
            phyInfoArray_Ac3x_Tb2[phyInfoArrayLen_Ac3x_Tb2].hostDevNum = devNum;
            phyInfoArrayLen_Ac3x_Tb2++;
        }

        if(devNum == 0)
        {
            /* we do MPD init only after we have the phy info on both the devices */
            return GT_OK;
        }
        xsmiPhyInfoArrayLen = phyInfoArrayLen_Ac3x_Tb2;
        osMemCpy(&xsmiPhyInfoArray, &phyInfoArray_Ac3x_Tb2, sizeof(GT_APPDEMO_XPHY_INFO_STC)*xsmiPhyInfoArrayLen);
    }

    /* bind basic callbacks needed from host application */
    memset(&phy_callbacks,0,sizeof(phy_callbacks));

    phy_callbacks.sleep_PTR               = gtAppDemoPhyOsDelay;
    phy_callbacks.alloc_PTR               = gtAppDemoPhyOsMalloc;
    phy_callbacks.free_PTR                = gtAppDemoPhyOsFree;
    phy_callbacks.debug_bind_PTR          = gtAppDemoPhyDebugBind;
    phy_callbacks.is_active_PTR           = gtAppDemoPhyDebugIsActive;
    phy_callbacks.debug_log_PTR           = gtAppDemoPhyDebugLog;
    phy_callbacks.txEnable_PTR            = NULL;
    phy_callbacks.handle_failure_PTR      = gtAppDemoPhyHandleFailure;
    phy_callbacks.getFwFiles_PTR          = gtAppDemoPhyGetFwFiles;
    phy_callbacks.mdioRead_PTR            = gtAppDemoPhyXsmiMdioRead;
    phy_callbacks.mdioWrite_PTR           = gtAppDemoPhyXsmiMdioWrite;
    phy_callbacks.logging_PTR             = gtAppDemoPhyPrintLog;

    if (currBoardType == APP_DEMO_XCAT5X_A0_BOARD_RD_CNS)
    {
        rc = genRegisterSet(devNum, 0, 0x8002010c, 0x22111133, 0xFFFFFFFF);
        if (rc != GT_OK)
        {
            cpssOsPrintf("genRegisterSet(0, 0, 0x8002010c, 0x22111133, 0xFFFFFFFF): rc = %d\n", rc);
            return rc;
        }
    }
    else if (currBoardType == APP_DEMO_XCAT5P_A0_BOARD_RD_CNS)
    {
        rc = genRegisterSet(devNum, 0, 0x3c00010c, 0x01111220, 0xFFFFFFFF);
        if (rc != GT_OK)
        {
            cpssOsPrintf("genRegisterSet(0, 0, 0x3c00010c, 0x01111220, 0xFFFFFFFF): rc = %d\n", rc);
            return rc;
        }
    }

    rc = mpdDriverInitDb(&phy_callbacks);
    if (rc != MPD_OK_E) {
        cpssOsPrintf("gtAppDemoPhyMpdInit failed in mpdDriverInitDb \n");
        return GT_FAIL;
    }

    /* xsmi update */
    for(i = 0; i < xsmiPhyInfoArrayLen ; i++)
    {
        rc = gtAppDemoPhyPortNumGet(currBoardType, boardRevId, xsmiPhyInfoArray[i], &portNum);
        if(rc != GT_OK)
        {
            cpssOsPrintf("gtAppDemoPhyMpdInit failed \n");
            return GT_FAIL;
        }
        phyType = gtAppDemoMpdPhyTypeGet(xsmiPhyInfoArray[i].phyType);

        memset(&phy_entry,0,sizeof(phy_entry));
        phy_entry.port                          = portNum;
        phy_entry.phyNumber                     = phyNumber;
        phy_entry.phyType                       = phyType;

        phy_entry.transceiverType               = MPD_TRANSCEIVER_COPPER_E;

        phy_entry.mdioInfo.mdioAddress          = xsmiPhyInfoArray[i].phyAddr;
        phy_entry.mdioInfo.mdioDev              = xsmiPhyInfoArray[i].hostDevNum;

        phy_entry.mdioInfo.mdioBus              = xsmiPhyInfoArray[i].xsmiInterface;
        phy_entry.disableOnInit                 = GT_FALSE;

        switch(phy_entry.phyType){
            case MPD_TYPE_88E2540_E:
                phy_entry.phySlice                  = (xsmiPhyInfoArray[i].phyAddr % 4);
                phy_entry.usxInfo.usxType           = MPD_PHY_USX_TYPE_OXGMII;
                break;
            case MPD_TYPE_88E2580_E:
                phy_entry.phySlice                  = (xsmiPhyInfoArray[i].phyAddr % 8);
                if (currBoardType == APP_DEMO_XCAT5X_A0_BOARD_RD_CNS)
                {
                    phy_entry.usxInfo.usxType           = MPD_PHY_USX_TYPE_OXGMII;
                }
                else
                {
                    phy_entry.usxInfo.usxType           = MPD_PHY_USX_TYPE_20G_QXGMII;
                }
                break;
            default:
                phy_entry.phySlice                  = portNum;
        }

        if(currBoardType == APP_DEMO_XCAT3X_TB2_48_CNS)
        {
            if((phy_entry.phyType == MPD_TYPE_88X2180_E) &&
                 (xsmiPhyInfoArray[i].phyAddr >= 0x4))
            {
                devNum = 1;
            }
            else
            {
                devNum = 0;
            }
        }

        rel_ifIndex = gtAppDemoPhyMpdIndexGet(devNum, portNum);
        if (mpdPortDbUpdate(rel_ifIndex, &phy_entry) != MPD_OK_E) {
            cpssOsPrintf("gtAppDemoPhyMpdInit failed in mpdPortDbUpdate\n");
            return GT_FAIL;
        }
    }

    /* smi update */
    for(i = 0; i < smiPhyInfoArrayLen; i++)
    {
        rc = gtAppDemoPhyPortNumGet(currBoardType, boardRevId, smiPhyInfoArray[i], &portNum);
        if(rc != GT_OK)
        {
            cpssOsPrintf("gtAppDemoPhyMpdInit failed \n");
            return GT_FAIL;
        }
        phyType = gtAppDemoMpdPhyTypeGet(smiPhyInfoArray[i].phyType);
        memset(&phy_entry,0,sizeof(phy_entry));
        phy_entry.port                          = portNum;
        phy_entry.phyNumber                     = phyNumber /*portNum/8*/;
        phy_entry.phyType                       = phyType;

        phy_entry.transceiverType               = MPD_TRANSCEIVER_COPPER_E;

        phy_entry.mdioInfo.mdioAddress          = smiPhyInfoArray[i].phyAddr;
        phy_entry.mdioInfo.mdioDev              = smiPhyInfoArray[i].hostDevNum;
        phy_entry.mdioInfo.mdioBus              = smiPhyInfoArray[i].xsmiInterface;
        phy_entry.phySlice                      = (smiPhyInfoArray[i].phyAddr % 4);
        phy_entry.usxInfo.usxType               = MPD_PHY_USX_TYPE_OXGMII;
        phy_entry.disableOnInit                 = GT_FALSE;

        rel_ifIndex = gtAppDemoPhyMpdIndexGet(devNum, portNum);
        if (mpdPortDbUpdate(rel_ifIndex, &phy_entry) != MPD_OK_E) {
            cpssOsPrintf("gtAppDemoPhyMpdInit failed in mpdPortDbUpdate\n");
            return GT_FAIL;
        }
    }

    for (i=0; i<MPD_TYPE_NUM_OF_TYPES_E; i++)
    {
        prvMpdGlobalDb_PTR->fwDownloadType_ARR[i] = MPD_FW_DOWNLOAD_TYPE_RAM_E;
    }

    cpssOsPrintf("MPD PHY initialize........ \n");

    if (mpdDriverInitHw() != MPD_OK_E)
    {
        cpssOsPrintf("gtAppDemoPhyMpdInit failed in mpdDriverInitHw");
        return GT_FAIL;
    }

    /* Advertise all supported speeds */
    opParams.phyAutoneg.enable       = MPD_AUTO_NEGOTIATION_ENABLE_E;
    opParams.phyAutoneg.capabilities = MPD_AUTONEG_CAPABILITIES_DEFAULT_CNS;
    opParams.phyAutoneg.masterSlave  = MPD_AUTONEGPREFERENCE_UNKNOWN_E;

    for(i = 0; i < xsmiPhyInfoArrayLen ; i++)
    {
        rc = gtAppDemoPhyPortNumGet(currBoardType, boardRevId, xsmiPhyInfoArray[i], &portNum);
        if(rc != GT_OK)
        {
            cpssOsPrintf("gtAppDemoPhyMpdInit failed \n");
            return GT_FAIL;
        }

        rel_ifIndex = gtAppDemoPhyMpdIndexGet(devNum, portNum);
        rc = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_AUTONEG_E, &opParams);
        if ( MPD_OK_E != rc )
        {
            cpssOsPrintf("Error : devNum[%d]portNum[%d]rel_ifIndex[%d] : failed on [%s] status[%d]\n",
                devNum,portNum,rel_ifIndex,"MPD_OP_CODE_SET_AUTONEG_E",rc);
            return GT_FAIL;
        }
    }

    cpssOsPrintf("MPD PHY initialize is finished \n");

    appDemoPhyMpdInitDone = GT_TRUE;
    return GT_OK;
}

/**
 * gtAppDemoPhyApOpModeDetect
 *
 * @param opMode
 * @param isHostOpModeAp
 * @param isLineOpModeAp
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhyApOpModeDetect
(
    IN  GT_APPDEMO_XPHY_OP_MODE  opMode,
    OUT GT_BOOL *isHostOpModeAp,
    OUT GT_BOOL *isLineOpModeAp
)
{
    if ( opMode >= GT_APPDEMO_XPHY_OP_MODE_LAST ) {
        return GT_ERROR;
    }

    if ( opMode == PCS_HOST_100G_KR2_AP_LINE_100G_LR4_AP ) {
        *isHostOpModeAp = GT_TRUE;
        *isLineOpModeAp = GT_TRUE;

    }else if ( opMode == PCS_HOST_100G_KR2_AP_LINE_100G_SR4) {
        *isHostOpModeAp = GT_TRUE;
        *isLineOpModeAp = GT_FALSE;

    }else {
        *isHostOpModeAp = GT_FALSE;
        *isLineOpModeAp = GT_FALSE;
    }

    return GT_OK;
}

GT_STATUS gtAppDemoPhySetPortMode
(
   GT_U32                        portNum,
   GT_APPDEMO_XPHY_SPEED_EXT_STC speedParam,
   GT_U32                        apEnableHost,
   GT_U32                        apEnableLine
)
{
    MPD_RESULT_ENT ret;
    MPD_OPERATIONS_PARAMS_UNT params;

    cpssOsMemSet(&params, 0 , sizeof(params));

    params.phySpeedExt.isRetimerMode    = speedParam.isRetimerMode;
    params.phySpeedExt.hostSide.speed   = speedParam.hostSide.speed;
    params.phySpeedExt.hostSide.ifMode  = speedParam.hostSide.ifMode;
    params.phySpeedExt.hostSide.fecMode = speedParam.hostSide.fecMode;
    params.phySpeedExt.hostSide.apEnable = apEnableHost;

    params.phySpeedExt.lineSide.speed   = speedParam.lineSide.speed;
    params.phySpeedExt.lineSide.ifMode  = speedParam.lineSide.ifMode;
    params.phySpeedExt.lineSide.fecMode = speedParam.lineSide.fecMode;
    params.phySpeedExt.lineSide.apEnable = apEnableLine;

    ret = mpdPerformPhyOperation(portNum,
                                 MPD_OP_CODE_SET_SPEED_EXT_E,
                                 &params);
    return (ret == MPD_OK_E)? GT_OK: GT_FAIL;
}

GT_STATUS gtAppDemoPhyGetPortMode
(
   GT_U32                       portNum,
   GT_APPDEMO_XPHY_OP_MODE      *opMode,
   GT_APPDEMO_XPHY_FEC_MODE     *fecModeHost,
   GT_APPDEMO_XPHY_FEC_MODE     *fecModeLine,
   GT_U32                       *apEnableHost,
   GT_U32                       *apEnableLine
)
{
    MPD_RESULT_ENT ret;
    GT_U32 rel_ifIndex;
    MPD_OPERATIONS_PARAMS_UNT params;
    cpssOsMemSet(&params, 0 , sizeof(params));

    rel_ifIndex = portNum;
    ret = mpdPerformPhyOperation(rel_ifIndex,
                                 MPD_OP_CODE_GET_SPEED_EXT_E,
                                 &params);
    if(ret == MPD_OK_E)
    {
        *opMode       = params.phySpeedExt.hostSide.speed;
        *fecModeHost  = params.phySpeedExt.hostSide.fecMode;
        *apEnableHost = params.phySpeedExt.hostSide.apEnable;
        *fecModeLine = params.phySpeedExt.lineSide.fecMode;
        *apEnableLine = params.phySpeedExt.lineSide.apEnable;
    }
    return (ret == MPD_OK_E)? GT_OK: GT_FAIL;
}

GT_STATUS gtAppDemoPhyPortPowerdown
(
   GT_SW_DEV_NUM            devNum,
   GT_U32                   portNum,
   GT_BOOL                  enable
)
{
    MPD_RESULT_ENT ret;
    MPD_OPERATIONS_PARAMS_UNT params;
    cpssOsMemSet(&params, 0 , sizeof(params));
    devNum = devNum;

    params.phyDisable.forceLinkDown = enable;

    ret = mpdPerformPhyOperation(portNum,
                                 MPD_OP_CODE_SET_PHY_DISABLE_OPER_E,
                                 &params);

    return (ret == MPD_OK_E)? GT_OK: GT_FAIL;
}

GT_STATUS gtAppDemoPhyGetPortLinkStatus
(
   GT_U32                       portNum,
   GT_U16                       *linkStatus
)
{
    MPD_RESULT_ENT ret;
    GT_U32 rel_ifIndex;
    MPD_OPERATIONS_PARAMS_UNT params;
    cpssOsMemSet(&params, 0 , sizeof(params));

    rel_ifIndex = portNum;
    ret = mpdPerformPhyOperation(rel_ifIndex,
                                 MPD_OP_CODE_GET_SPEED_EXT_E,
                                 &params);
    if(ret == MPD_OK_E)
    {
    if(linkStatus != NULL)
    {
        *linkStatus  = params.phyInternalOperStatus.isOperStatusUp;
    }
    else
    {
            return GT_BAD_PTR;
    }
    }
    return (ret == MPD_OK_E)? GT_OK: GT_FAIL;
}

GT_STATUS gtAppDemoPhyGetTemperature
(
    IN  GT_U32                       devNum,
    IN  GT_U32                       portNum,
    OUT GT_32                       *coreTemperature
)

{
    MPD_RESULT_ENT ret;
    MPD_OPERATIONS_PARAMS_UNT   params;
    GT_U32                      mpd_ifIndex;

    cpssOsMemSet(&params, 0 , sizeof(params));
    if(GT_TRUE == appDemoEzbIsXmlWithDevNum(devNum))
    {
        appDemoEzbMpdDevMacToIfIndexConvert(devNum,portNum,&mpd_ifIndex);
    }
    else
    {
#if !defined(ASIC_SIMULATION)
        falcon_get_mpd_if_index(devNum, portNum, &mpd_ifIndex);
#endif
    }

    ret = mpdPerformPhyOperation(portNum,
                                 MPD_OP_CODE_GET_TEMPERATURE_E,
                                 &params);
    if(ret == MPD_OK_E)
    {
        *coreTemperature = params.phyTemperature.temperature;
    }

    return (ret == MPD_OK_E)? GT_OK: GT_FAIL;
}

GT_STATUS gtAppDemoPhySerdesTxConfigSet
(
    GT_U32                                   portNum,
    GT_U32                                   laneBmp,
    GT_APPDEMO_XPHY_HOST_LINE                hostOrLineSide,
    CPSS_PORT_SERDES_TUNE_STC                *tuneParamsPtr
)
{
    MPD_RESULT_ENT ret;
    MPD_OPERATIONS_PARAMS_UNT params;
    cpssOsMemSet(&params, 0 , sizeof(params));

    params.phyTune.tuneParams.paramsType = MPD_SERDES_PARAMS_TX_E;

    params.phyTune.tuneParams.txTune.atten =  tuneParamsPtr->txTune.avago.atten;
    params.phyTune.tuneParams.txTune.post  =  tuneParamsPtr->txTune.avago.post;
    params.phyTune.tuneParams.txTune.pre   =  tuneParamsPtr->txTune.avago.pre;
    params.phyTune.tuneParams.txTune.pre2  =  tuneParamsPtr->txTune.avago.pre2;
    params.phyTune.tuneParams.txTune.pre3  =  tuneParamsPtr->txTune.avago.pre3;

    params.phyTune.hostOrLineSide          = hostOrLineSide;
    params.phyTune.lanesBmp                = laneBmp;

    ret = mpdPerformPhyOperation(portNum,
                                 MPD_OP_CODE_SET_SERDES_TUNE_E,
                                 &params);

    return (ret == MPD_OK_E)? GT_OK: GT_FAIL;


}

GT_STATUS gtAppDemoPhyPortTxConfigSet
(
   GT_U32                                   portNum,
   GT_U32                                   lanesBmpHost,
   GT_U32                                   lanesBmpLine,
   CPSS_PORT_SERDES_TUNE_STC                *hostSideTuneParamsPtr,
   CPSS_PORT_SERDES_TUNE_STC                *lineSideTuneParamsPtr
)
{

    if(hostSideTuneParamsPtr!= NULL)
    {
        CHECK_STATUS(gtAppDemoPhySerdesTxConfigSet(portNum, lanesBmpHost, GT_APPDEMO_XPHY_HOST_SIDE, hostSideTuneParamsPtr));
    }
    if(lineSideTuneParamsPtr!= NULL)
    {
        CHECK_STATUS(gtAppDemoPhySerdesTxConfigSet(portNum, lanesBmpLine, GT_APPDEMO_XPHY_LINE_SIDE, lineSideTuneParamsPtr));
    }
    lanesBmpHost = lanesBmpHost;
    lanesBmpLine = lanesBmpLine;
    return GT_OK;
}

GT_STATUS gtAppDemoPhySerdesTune
(
    GT_U32                                   portNum,
    GT_U32                                   laneBmp,
    GT_APPDEMO_XPHY_HOST_LINE                hostOrLineSide,
    CPSS_PORT_SERDES_TUNE_STC                *tuneParamsPtr
)
{
    MPD_RESULT_ENT ret;
    MPD_OPERATIONS_PARAMS_UNT params;
    cpssOsMemSet(&params, 0 , sizeof(params));

    params.phyTune.tuneParams.paramsType = MPD_SERDES_PARAMS_RX_E;

    params.phyTune.tuneParams.rxTune.dc    =  tuneParamsPtr->rxTune.avago.DC;
    params.phyTune.tuneParams.rxTune.lf    =  tuneParamsPtr->rxTune.avago.LF;
    params.phyTune.tuneParams.rxTune.hf    =  tuneParamsPtr->rxTune.avago.HF;
    params.phyTune.tuneParams.rxTune.bw    =  tuneParamsPtr->rxTune.avago.BW;

    params.phyTune.hostOrLineSide          = hostOrLineSide;
    params.phyTune.lanesBmp                = laneBmp;

    ret = mpdPerformPhyOperation(portNum,
                                 MPD_OP_CODE_SET_SERDES_TUNE_E,
                                 &params);

    return (ret == MPD_OK_E)? GT_OK: GT_FAIL;
}

GT_BOOL gtAppDemoPhyIsPcsMode(GT_APPDEMO_XPHY_OP_MODE opMode)
{
    return (opMode > GT_APPDEMO_XPHY_OP_MODE_LAST_RETIMER) ? GT_TRUE : GT_FALSE;
}

GT_STATUS gtAppDemoPhyPolarityArraySet
(
   GT_U32                               portNum,
   GT_APPDEMO_XPHY_HOST_LINE            hostOrLineSide,
   GT_U32                               lanesBmp,
   APPDEMO_SERDES_LANE_POLARITY_STC     *polaritySwapParamsArr,
   GT_U32                               polaritySwapParamsArrSize
)
{
    GT_U32      laneIndex;
    MPD_RESULT_ENT ret;
    MPD_OPERATIONS_PARAMS_UNT params;
    cpssOsMemSet(&params, 0 , sizeof(params));

    for (laneIndex = 0; laneIndex < polaritySwapParamsArrSize ; laneIndex++)
    {
        if(lanesBmp & (1<<laneIndex))
        {
            params.phySerdesPolarity.laneNum = polaritySwapParamsArr[laneIndex].laneNum;
            params.phySerdesPolarity.hostOrLineSide = hostOrLineSide;
            params.phySerdesPolarity.invertTx = polaritySwapParamsArr[laneIndex].invertTx;
            params.phySerdesPolarity.invertRx = polaritySwapParamsArr[laneIndex].invertRx;
            ret = mpdPerformPhyOperation(portNum,
                                         MPD_OP_CODE_SET_SERDES_LANE_POLARITY_E,
                                         &params);
            if(ret != MPD_OK_E)
                return GT_FAIL;

        }
    }
    return (ret == MPD_OK_E)? GT_OK: GT_FAIL;
}

GT_STATUS gtAppDemoPhyPortTune
(
    GT_U32                                   portNum,
    GT_U32                                   lanesBmpHost,
    GT_U32                                   lanesBmpLine,
    CPSS_PORT_SERDES_TUNE_STC                *hostSideTuneParamsPtr,
    CPSS_PORT_SERDES_TUNE_STC                *lineSideTuneParamsPtr
)
{

    if(NULL != lineSideTuneParamsPtr)
    {
        CHECK_STATUS(gtAppDemoPhySerdesTune(portNum,lanesBmpLine,GT_APPDEMO_XPHY_LINE_SIDE,lineSideTuneParamsPtr));
    }

    cpssOsTimerWkAfter(100);

    if(NULL != hostSideTuneParamsPtr)
    {
        CHECK_STATUS(gtAppDemoPhySerdesTune(portNum,lanesBmpHost,GT_APPDEMO_XPHY_HOST_SIDE,hostSideTuneParamsPtr));
    }
    lanesBmpHost = lanesBmpHost;
    lanesBmpLine = lanesBmpLine;
    return GT_OK;
}


#endif /* INCLUDE_MPD */
#endif /* ASIC_SIMULATION */


