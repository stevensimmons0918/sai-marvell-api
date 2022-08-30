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
* @file gtAppDemoPhy7120Config.c
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
#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>

#include <cpss/common/smi/cpssGenSmi.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>

#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#endif /*CHX_FAMILY*/

#ifndef ASIC_SIMULATION
#include <appDemo/phy/gtAppDemoPhyConfig.h>

#include <88x7120/mydApiTypes.h>
#include <88x7120/mydAPI.h>
#include <88x7120/mydHwCntl.h>
#include <88x7120/mydInitialization.h>
#include <88x7120/mydDiagnostics.h>
#include <88x7120/mydApiRegs.h>
#include <88x7120/mydHwSerdesCntl.h>
#include <88x7120/mydIntrIOConfig.h>
#include <88x7120/mydAPIInternal.h>

#include "88x7120/serdes.0x1093_2085_064.h"
#include "88x7120/sbus_master.0x1021_2001.h"
#include "88x7120/z80_AutoCal_0x010A.NIMB.h"

#define GT_APPDEMO_PHY_7120_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(pDev,laneNum,mdioPort,laneOffset) \
                       if((pDev != NULL) && (pDev->devEnabled == MYD_TRUE))                      \
                       {                                                                         \
                           mdioPort   = (laneNum / pDev->portCount) + pDev->mdioPort;            \
                           laneOffset = (laneNum % MYD_NUM_LANES);                               \
                       }

MYD_OP_MODE gtAppDemoPhy7120ModeTranslationTable[GT_APPDEMO_XPHY_OP_MODE_LAST][GT_APPDEMO_XPHY_FEC_MODE_LAST][GT_APPDEMO_XPHY_BOTH_SIDES]=
{                                         /*  NO FEC                                      FC-FEC                                      RS-FEC                                      RS-FEC (544, 514)                          */
                                          /*  {hostSide            ,lineSide           }, {hostSide            ,lineSide           }, {hostSide            ,lineSide           }, {hostSide            ,lineSide           } */
 /* RETIMER_HOST_25G_KR_LINE_25G_KR       */ {{MYD_R25L            ,MYD_R25L           }, {MYD_R25L            ,MYD_R25L           }, {MYD_R25L            ,MYD_R25L           }, {MYD_R25L            ,MYD_R25L           }},
 /* RETIMER_HOST_50G_KR_LINE_50G_KR       */ {{MYD_R50U            ,MYD_R50U           }, {MYD_R50U            ,MYD_R50U           }, {MYD_R50U            ,MYD_R50U           }, {MYD_R50U            ,MYD_R50U           }},
 /* RETIMER_HOST_50G_KR2_LINE_50G_KR2     */ {{MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}},
 /* RETIMER_HOST_100G_KR2_LINE_100G_KR2   */ {{MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}},
 /* RETIMER_HOST_100G_KR4_LINE_100G_KR4   */ {{MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}},
 /* RETIMER_HOST_200G_KR4_LINE_200G_KR4   */ {{MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}},
 /* RETIMER_HOST_200G_KR8_LINE_200G_KR8   */ {{MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}},
 /* RETIMER_HOST_400G_KR8_LINE_400G_KR8   */ {{MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}},
 /* PCS_HOST_25G_KR_LINE_25G_KR           */ {{MYD_P25LN           ,MYD_P25LN          }, {MYD_P25LF           ,MYD_P25LF          }, {MYD_P25LR           ,MYD_P25LR          }, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}},
 /* PCS_HOST_50G_KR_LINE_50G_KR           */ {{MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_P50UP           ,MYD_P50UP          }},
 /* PCS_HOST_100G_KR2_LINE_100G_CR4       */ {{MYD_P100UP          ,MYD_P100LN         }, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_P100UP          ,MYD_P100CR         }, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}},
 /* PCS_HOST_100G_KR2_LINE_100G_SR4       */ {{MYD_P100UP          ,MYD_P100LN         }, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_P100UP          ,MYD_P100LR         }, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}},
 /* PCS_HOST_100G_KR2_LINE_100G_LR4       */ {{MYD_P100UP          ,MYD_P100LN         }, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_P100UP          ,MYD_P100LR         }, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}},
 /* PCS_HOST_100G_KR2_AP_LINE_100G_LR4_AP */ {{MYD_OP_MODE_UNKNOWN, MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_P100QP          ,MYD_P100CR         }, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}},
 /* PCS_HOST_100G_KR2_AP_LINE_100G_SR4    */ {{MYD_P100QP          ,MYD_P100LN         }, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_P100QP          ,MYD_P100LR         }, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}},
 /* PCS_HOST_100G_KR2_LINE_100G_KR2       */ {{MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_P100UP          ,MYD_P100UP         }},
 /* PCS_HOST_100G_KR2_LINE_100G_CR2       */ {{MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_OP_MODE_UNKNOWN ,MYD_OP_MODE_UNKNOWN}, {MYD_P100UP          ,MYD_P100UP         }},
};

/*********************************** private APIs ************************************/

GT_STATUS gtAppDemoPhy7120TranslateOpAndFecModeToAppDemo
(
   IN  GT_APPDEMO_XPHY_OP_MODE          opMode,
   IN  GT_APPDEMO_XPHY_FEC_MODE         fecMode,
   OUT MYD_OP_MODE                      *mydOpModeHost,
   OUT MYD_OP_MODE                      *mydOpModeLine
)
{
    if(mydOpModeHost == NULL || mydOpModeLine == NULL)
    {
        return GT_BAD_PTR;
    }
    if(opMode >= GT_APPDEMO_XPHY_OP_MODE_LAST || fecMode >= GT_APPDEMO_XPHY_FEC_MODE_LAST)
    {
        return GT_BAD_PARAM;
    }

    *mydOpModeHost = gtAppDemoPhy7120ModeTranslationTable[opMode][fecMode][GT_APPDEMO_XPHY_HOST_SIDE];
    *mydOpModeLine = gtAppDemoPhy7120ModeTranslationTable[opMode][fecMode][GT_APPDEMO_XPHY_LINE_SIDE];

    if(*mydOpModeHost == MYD_OP_MODE_UNKNOWN || *mydOpModeLine == MYD_OP_MODE_UNKNOWN)
    {
        return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}

GT_STATUS gtAppDemoPhy7120TranslateAppDemoToOpAndFecMode
(
   IN  MYD_OP_MODE                      mydOpModeHost,
   IN  MYD_OP_MODE                      mydOpModeLine,
   OUT GT_APPDEMO_XPHY_OP_MODE          *opMode,
   OUT GT_APPDEMO_XPHY_FEC_MODE         *fecMode
)
{
    GT_APPDEMO_XPHY_OP_MODE          opModeIterator;
    GT_APPDEMO_XPHY_FEC_MODE         fecModeIterator;

    if(opMode == NULL || fecMode == NULL)
    {
        return GT_BAD_PTR;
    }
    if(mydOpModeHost >= MYD_OP_MODE_COUNT || mydOpModeLine >= MYD_OP_MODE_COUNT)
    {
        return GT_BAD_PARAM;
    }

    for(opModeIterator = 0 ; opModeIterator < GT_APPDEMO_XPHY_OP_MODE_LAST; opModeIterator++)
    {
        for(fecModeIterator = 0 ; fecModeIterator < GT_APPDEMO_XPHY_FEC_MODE_LAST; fecModeIterator++)
        {
            if((gtAppDemoPhy7120ModeTranslationTable[opModeIterator][fecModeIterator][GT_APPDEMO_XPHY_HOST_SIDE] == mydOpModeHost)
            && (gtAppDemoPhy7120ModeTranslationTable[opModeIterator][fecModeIterator][GT_APPDEMO_XPHY_LINE_SIDE] == mydOpModeLine))

            {
                *opMode  = opModeIterator;
                *fecMode = fecModeIterator;
                return GT_OK;
            }
        }
    }
    return GT_NOT_FOUND;
}

/*********************************** APIs ************************************/

/**
 * gtAppDemoPhy7120PortPowerdown
 *
 *
 * @param phyInfo
 * @param portNum
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhy7120PortPowerdown
(
   GT_APPDEMO_XPHY_INFO_PTR     phyInfo,
   GT_U32                       portNum
)
{
    MYD_STATUS      mydRc;
    MYD_DEV_PTR     pDev = NULL;
    MYD_U16         mdioPort=0;
    MYD_U16         laneOffset=0;

    GT_APPDEMO_PHY_INFO_DRIVER_CHECK_AND_RETURN(phyInfo);
    pDev = (MYD_DEV_PTR)phyInfo->driverObj;

    GT_APPDEMO_PHY_7120_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(pDev,portNum,mdioPort,laneOffset);
    (void)laneOffset;

    mydRc = mydLanePowerdown(pDev,mdioPort,MYD_HOST_SIDE,MYD_ALL_LANES);
    if(mydRc != MYD_OK)
    {
        return GT_FAIL;
    }

    mydRc = mydLanePowerdown(pDev,mdioPort,MYD_LINE_SIDE,MYD_ALL_LANES);
    if(mydRc != MYD_OK)
    {
        return GT_FAIL;
    }

    return GT_OK;
}

/**
 * gtAppDemoPhy7120GetPortLinkStatus
 *
 *
 * @param phyInfo
 * @param portNum
 * @param linkStatus
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhy7120GetPortLinkStatus
(
   GT_APPDEMO_XPHY_INFO_PTR     phyInfo,
   GT_U32                       portNum,
   GT_U16                       *linkStatus
)
{
    MYD_STATUS                  mydRc;
    MYD_DEV_PTR                 pDev = NULL;
    MYD_U16                     mdioPort=0;
    MYD_U16                     laneOffset=0;

    MYD_PCS_LINK_STATUS         statusDetail;
    MYD_U16                     latchedStatus;

    GT_APPDEMO_PHY_INFO_DRIVER_CHECK_AND_RETURN(phyInfo);
    pDev = (MYD_DEV_PTR)phyInfo->driverObj;

    GT_APPDEMO_PHY_7120_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(pDev,portNum,mdioPort,laneOffset);

    if( NULL != linkStatus )
    {
        mydRc = mydReadPCSLinkStatus(pDev,mdioPort,laneOffset,linkStatus,&latchedStatus,&statusDetail);
        /*osPrintf("\ncurrentStatus#%d latchedStatus#%d hCur#%d hLat#%d lCur#%d lLat#%d\n",*linkStatus,latchedStatus, \
                 statusDetail.hostCurrent,statusDetail.hostLatched,statusDetail.lineCurrent,statusDetail.lineLatched);*/

        if( mydRc != MYD_OK )
        {
            return GT_FAIL;
        }
    }
    return GT_OK;
}

/**
 * gtAppDemoPhy7120GetPortMode
 *
 *
 * @param phyInfo
 * @param portNum
 * @param opMode
 * @param fecMode
 * @param args
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhy7120GetPortMode
(
   GT_APPDEMO_XPHY_INFO_PTR     phyInfo,
   GT_U32                       portNum,
   GT_APPDEMO_XPHY_OP_MODE      *opMode,
   GT_APPDEMO_XPHY_FEC_MODE     *fecMode,
   GT_VOID_PTR                  args
)
{
    MYD_STATUS                  mydRc;
    MYD_DEV_PTR                 pDev = NULL;
    MYD_OP_MODE                 mydOpModeHost;
    MYD_OP_MODE                 mydOpModeLine;
    MYD_U16                     mdioPort=0;
    MYD_U16                     laneOffset=0;

    GT_UNUSED_PARAM(args);

    GT_APPDEMO_PHY_INFO_DRIVER_CHECK_AND_RETURN(phyInfo);
    pDev = (MYD_DEV_PTR)phyInfo->driverObj;

    GT_APPDEMO_PHY_7120_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(pDev,portNum,mdioPort,laneOffset);

    mydRc = mydGetOpMode(pDev,mdioPort,MYD_HOST_SIDE,laneOffset,&mydOpModeHost);
    if(mydRc != MYD_OK)
    {
        return GT_FAIL;
    }

    mydRc = mydGetOpMode(pDev,mdioPort,MYD_LINE_SIDE,laneOffset,&mydOpModeLine);
    if(mydRc != MYD_OK)
    {
        return GT_FAIL;
    }

    return gtAppDemoPhy7120TranslateAppDemoToOpAndFecMode(mydOpModeHost,mydOpModeLine,opMode,fecMode);
}

GT_STATUS gtAppDemoPhy7120InitDriver
(
    GT_APPDEMO_XPHY_INFO_PTR     phyInfo,
    GT_BOOL                      loadImage
)
{
    GT_STATUS       rc = GT_OK;

    MYD_STATUS      mydRc;
    MYD_DEV_PTR     pDev = NULL;
    MYD_U16         *pBusMasterImage = NULL;
    MYD_U16         busMasterSize;
    MYD_U16         *pSerdesImage = NULL;
    MYD_U16         serdesSize;
    MYD_U16         *pZ80Image = NULL;
    MYD_U16         z80Size;

    if(phyInfo == NULL)
    {
        return GT_NOT_INITIALIZED;
    }

    if(phyInfo->driverObj == NULL)
    {
        phyInfo->driverObj = (GT_VOID_PTR)cpssOsMalloc(sizeof(MYD_DEV));
        if(phyInfo->driverObj == NULL)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("mydInitDriver Failed", GT_NO_RESOURCE);
        }
    }
    else
    {
        return GT_ALREADY_EXIST;
    }
    pDev = (MYD_DEV_PTR)phyInfo->driverObj;

    /* To protect CPLD MUX channel until XSmi bus in use */
    mydHwCtrlRWMutexCreate();

    if (loadImage)
    {
        pSerdesImage    = (MYD_U16  *)myd_serdes0x1093_2085Data;
        serdesSize      = AVAGO_SERDES_FW_IMAGE_SIZE;

        pBusMasterImage = (MYD_U16  *)myd_sbusMaster0x1021_2001Data;
        busMasterSize   = AVAGO_SBUS_MASTER_FW_IMAGE_SIZE;

        pZ80Image       = (MYD_U16  *)myd_z80_0x010aData;
        z80Size         = Z80_0x010A_IMAGE_SIZE;

        mydRc = mydInitDriver(wrMydCtrlXsmiRead, wrMydCtrlXsmiWrite, phyInfo->phyAddr,
                            pZ80Image, z80Size, pBusMasterImage, busMasterSize, pSerdesImage, serdesSize,
                            (MYD_PVOID)phyInfo, pDev);
    }
    else
    {
        mydRc = mydInitDriver(wrMydCtrlXsmiRead, wrMydCtrlXsmiWrite, phyInfo->phyAddr,
                            NULL, 0, NULL, 0, NULL, 0,
                            (MYD_PVOID)phyInfo, pDev);
    }

    if(mydRc != MYD_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("mydInitDriver Failed", mydRc);
        rc = GT_FAIL;
    }
    return rc;
}

GT_STATUS gtAppDemoPhy7120BroadcastInit
(
    GT_APPDEMO_XPHY_INFO_PTR     phyInfoArr,
    GT_U32                       phyInfoArrSize,
    CPSS_PHY_XSMI_INTERFACE_ENT  broadcastInterface,
    GT_U16                       broadcastAddr,
    GT_BOOL                      loadImage
)
{
    GT_STATUS                   rc = GT_OK;
    GT_APPDEMO_XPHY_INFO_STC    broadcastPhyInfo;
    GT_U32                      phyIndex;
    MYD_DEV_PTR                 currPDev;

    osMemCpy(&broadcastPhyInfo, &phyInfoArr[0], sizeof(GT_APPDEMO_XPHY_INFO_STC));
    broadcastPhyInfo.xsmiInterface = broadcastInterface;
    broadcastPhyInfo.phyAddr       = broadcastAddr;
    broadcastPhyInfo.portGroupId   = 0xF;
    broadcastPhyInfo.hostDevNum    = 0;
    rc = gtAppDemoPhy7120InitDriver(&broadcastPhyInfo,loadImage); /* Broadcast image */
    if(GT_OK != rc)
    {
        return rc;
    }
    for (phyIndex = 0; phyIndex < phyInfoArrSize ; phyIndex++)
    { /* Manually create driver objects */
        if(phyInfoArr[phyIndex].hostDevNum != SKIP_PHY)
        {
            if(phyInfoArr[phyIndex].driverObj == NULL)
            {
                phyInfoArr[phyIndex].driverObj = (GT_VOID_PTR)cpssOsMalloc(sizeof(MYD_DEV));
                if(phyInfoArr[phyIndex].driverObj == NULL)
                {
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("gtAppDemoPhy7120ArrayInit mem alloc failed", GT_NO_RESOURCE);
                }
                osMemCpy(phyInfoArr[phyIndex].driverObj,broadcastPhyInfo.driverObj,sizeof(MYD_DEV));
                currPDev = phyInfoArr[phyIndex].driverObj;
                currPDev->mdioPort = phyInfoArr[phyIndex].phyAddr;
                currPDev->hostContext = (MYD_PVOID)&phyInfoArr[phyIndex];
                mydInitSerdesDev(currPDev);
                cpssOsPrintf("Init PHY driver done (broadcast). hostDevNum=%d portGroupId=%d phyAddr=0x%x phyType=0x%x\n",
                             phyInfoArr[phyIndex].hostDevNum, phyInfoArr[phyIndex].portGroupId, phyInfoArr[phyIndex].phyAddr, phyInfoArr[phyIndex].phyType);

            }
        }
    }
    return GT_OK;
}

GT_STATUS gtAppDemoPhy7120UnloadDriver
(
    GT_APPDEMO_XPHY_INFO_PTR     phyInfo
)
{
    GT_STATUS       rc = GT_OK;

    MYD_STATUS      mydRc;
    MYD_DEV_PTR     pDev = NULL;

    GT_APPDEMO_PHY_INFO_DRIVER_CHECK_AND_RETURN(phyInfo);
    pDev = (MYD_DEV_PTR)phyInfo->driverObj;

    /* To protect CPLD MUX channel until XSmi bus in use */
    mydHwCtrlRWMutexDestroy();

    if (!(pDev && (pDev->devEnabled == MYD_TRUE)))
    {
        rc = GT_NOT_INITIALIZED;
    }
    else
    {
        mydRc = mydUnloadDriver(pDev);
        if(mydRc != MYD_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("mydUnloadDriver Failed", mydRc);
            rc = GT_FAIL;
        }
        cpssOsFree(pDev);
        phyInfo->driverObj = NULL;
    }

    return rc;
}

GT_STATUS gtAppDemoPhy7120SetPortMode
(
   GT_APPDEMO_XPHY_INFO_PTR     phyInfo,
   GT_U32                       portsBmp,
   GT_APPDEMO_XPHY_OP_MODE      opMode,
   GT_APPDEMO_XPHY_FEC_MODE     fecMode,
   GT_VOID_PTR                  args
)
{
    GT_STATUS       rc;
    GT_U32          portNum;
    GT_BOOL         *shortChannel=args;

    MYD_STATUS      mydRc;
    MYD_U16         result;
    MYD_DEV_PTR     pDev = NULL;
    MYD_OP_MODE     mydOpModeHost;
    MYD_OP_MODE     mydOpModeLine;
    MYD_U16         mdioPort=0;
    MYD_U16         laneOffset=0;
    MYD_U16         iCalNIMBStatus;
    MYD_U32         modeOption;

    GT_APPDEMO_PHY_INFO_DRIVER_CHECK_AND_RETURN(phyInfo);
    pDev = (MYD_DEV_PTR)phyInfo->driverObj;

    if(shortChannel == NULL)
    {
        return GT_BAD_PTR;
    }

    rc = gtAppDemoPhy7120TranslateOpAndFecModeToAppDemo(opMode,fecMode,&mydOpModeHost,&mydOpModeLine);
    if(rc != GT_OK)
    {
        return rc;
    }

    for (portNum = 0; portNum < (pDev->portCount*MYD_NUM_LANES); portNum++)
    {
        /* portsBmp contains only valid ports - checked by original function */
        if ((portsBmp & (1<<portNum)) == 0) continue;

        GT_APPDEMO_PHY_7120_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(pDev,portNum,mdioPort,laneOffset);
        /*osPrintf("-portNum[%d]---mdioPort[%d]---laneOffset[%d]-\n",portNum,mdioPort,laneOffset);*/

        /* TBD needed due to previous call to mydLanePowedown */
        mydRc = mydLanePowerup(pDev,mdioPort,MYD_HOST_SIDE,MYD_ALL_LANES);
        if(mydRc != MYD_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("mydLanePowerup Failed", mydRc);
            rc = GT_FAIL;
            continue;
        }
        mydRc = mydLanePowerup(pDev,mdioPort,MYD_LINE_SIDE,MYD_ALL_LANES);
        if(mydRc != MYD_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("mydLanePowerup Failed", mydRc);
            rc = GT_FAIL;
            continue;
        }

        if(mydOpModeHost > MYD_PCS_RESERVE_29) /* Workaroud for retimer modes */
        {
            mydNIMBGetICalStatus(pDev, mdioPort, laneOffset, &iCalNIMBStatus);
            if (iCalNIMBStatus) /* Stop the iCal polling and auto calibration is started */
            {
                mydRc = mydNIMBSetICal(pDev, mdioPort, laneOffset, 0);
                if(mydRc != MYD_OK)
                {
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("mydNIMBSetICal Failed", mydRc);
                    rc = GT_FAIL;
                    continue;
                }
                mydWait(pDev, 500);
            }
        }

        modeOption = MYD_MODE_FORCE_RECONFIG | ((*shortChannel)?MYD_MODE_SHORT_CHANNEL:0);
        mydRc = mydSetModeSelection(pDev, mdioPort, laneOffset, mydOpModeHost, mydOpModeLine, modeOption, &result);
        if(mydRc != MYD_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("mydSetModeSelection Failed", mydRc);
            rc = GT_FAIL;
            continue;
        }

        if(mydOpModeHost > MYD_PCS_RESERVE_29) /* Workaroud for retimer modes */
        {
            /* Set the eye range to trigger an iCal calibration; MYD_NIMB_EYE_DEFAULT will use the suggested values */
            mydRc = mydNIMBSetEyeCoeff(pDev, mdioPort, laneOffset, 0, 0, 0, MYD_NIMB_EYE_DEFAULT);
            if(rc != MYD_OK)
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("mydNIMBSetEyeCoeff Failed", mydRc);
                rc = GT_FAIL;
                continue;
            }
            /* mydNIMBSetICal must be stopped when calling API that issues interrupt calls to the SerDes
               and SBus master. All APIs that required MYD_ENABLE_SERDES_API definition issued interrupt. */
            /* start the iCal polling and auto calibration */
            mydRc = mydNIMBSetICal(pDev, mdioPort, laneOffset, 1);
            if(mydRc != MYD_OK)
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("mydNIMBSetICal Failed", mydRc);
                rc = GT_FAIL;
                continue;
            }
        }
    }

    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("One or more ports configuration failed", rc);
    }
    return rc;
}

GT_STATUS gtAppDemoPhy7120GetPortLanesBmp
(
   GT_APPDEMO_XPHY_INFO_PTR     phyInfo,
   GT_U32                       portNum,
   GT_U32                       *hostSideLanesBmp,
   GT_U32                       *lineSideLanesBmp
)
{
    MYD_DEV_PTR     pDev = NULL;
    MYD_U16         mdioPort=0;
    MYD_U16         laneOffset=0;
    MYD_U16         laneCount, laneIndex;

    GT_APPDEMO_PHY_INFO_DRIVER_CHECK_AND_RETURN(phyInfo);
    pDev = (MYD_DEV_PTR)phyInfo->driverObj;

    GT_APPDEMO_PHY_7120_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(pDev,portNum,mdioPort,laneOffset);


    if(NULL != hostSideLanesBmp)
    {
        *hostSideLanesBmp = 0;
        laneCount = pDev->hostConfig[MYD_GET_PORT_IDX(pDev, mdioPort)][laneOffset].laneCount;
        for(laneIndex = 0; laneIndex < laneCount ; laneIndex++)
        {
            *hostSideLanesBmp |= 1 << (portNum+laneIndex);
        }
    }

    if(NULL != lineSideLanesBmp)
    {
        *lineSideLanesBmp = 0;
        laneCount = pDev->lineConfig[MYD_GET_PORT_IDX(pDev, mdioPort)][laneOffset].laneCount;
        for(laneIndex = 0; laneIndex < laneCount ; laneIndex++)
        {
            *lineSideLanesBmp |= 1 << (portNum+laneIndex);
        }
    }

    return GT_OK;
}

GT_STATUS gtAppDemoPhy7120SetLanePolaritySwap
(
   GT_APPDEMO_XPHY_INFO_PTR             phyInfo,
   GT_APPDEMO_XPHY_HOST_LINE            hostOrLineSide,
   APPDEMO_SERDES_LANE_POLARITY_STC*    polaritySwapParams
)
{
    GT_STATUS       rc = GT_OK;

    MYD_STATUS      mydRc;
    MYD_DEV_PTR     pDev = NULL;
    MYD_U16         mdioPort=0;
    MYD_U16         laneOffset=0;
    MYD_U16         mydHostOrLineSide;

    GT_APPDEMO_PHY_INFO_DRIVER_CHECK_AND_RETURN(phyInfo);
    pDev = (MYD_DEV_PTR)phyInfo->driverObj;

    if(polaritySwapParams == NULL)
    {
        return GT_BAD_PTR;
    }

    mydHostOrLineSide = (hostOrLineSide == GT_APPDEMO_XPHY_HOST_SIDE) ? MYD_HOST_SIDE : MYD_LINE_SIDE;

    GT_APPDEMO_PHY_7120_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(pDev,polaritySwapParams->laneNum,mdioPort,laneOffset);

    mydRc = mydSetTxPolarity(pDev, mdioPort, mydHostOrLineSide, laneOffset, polaritySwapParams->invertTx,1);
    if(mydRc != MYD_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("mydSetTxPolarity Failed", mydRc);
        rc = GT_FAIL;
    }
    mydRc = mydSetRxPolarity(pDev, mdioPort, mydHostOrLineSide, laneOffset, polaritySwapParams->invertRx,1);
    if(mydRc != MYD_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("mydSetRxPolarity Failed", mydRc);
        rc = GT_FAIL;
    }

    return rc;
}

GT_STATUS gtAppDemoPhy7120SerdesTune
(
   GT_APPDEMO_XPHY_INFO_PTR                 phyInfo,
   GT_APPDEMO_XPHY_HOST_LINE                hostOrLineSide,
   GT_U32                                   lanesBmp,
   CPSS_PORT_SERDES_TUNE_STC                *tuneParamsPtr
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          laneNum;

    MYD_STATUS     mydRc;
    MYD_DEV_PTR    pDev = NULL;
    MYD_U16        mdioPort=0;
    MYD_U16        laneOffset=0;
    MYD_U16        mydHostOrLineSide;
    MYD_U16        sAddr;

    MYD_U16        dc;
    MYD_U16        lf;
    MYD_U16        hf;
    MYD_U16        bw;


    GT_APPDEMO_PHY_INFO_DRIVER_CHECK_AND_RETURN(phyInfo);
    pDev = (MYD_DEV_PTR)phyInfo->driverObj;

    if(tuneParamsPtr == NULL)
    {
        return GT_BAD_PTR;
    }
    tuneParamsPtr->type = CPSS_PORT_SERDES_AVAGO_E;
    dc          = (MYD_U16)tuneParamsPtr->rxTune.avago.DC;
    lf          = (MYD_U16)tuneParamsPtr->rxTune.avago.LF;
    hf          = (MYD_U16)tuneParamsPtr->rxTune.avago.HF;
    bw          = (MYD_U16)tuneParamsPtr->rxTune.avago.BW;

    switch (hostOrLineSide)
    {
        case GT_APPDEMO_XPHY_HOST_SIDE:
            mydHostOrLineSide = MYD_HOST_SIDE;
            break;
        case GT_APPDEMO_XPHY_LINE_SIDE:
            mydHostOrLineSide = MYD_LINE_SIDE;
            break;
        default:
            return GT_NOT_SUPPORTED;
            break;
    }

    for (laneNum = 0; laneNum < (pDev->portCount*MYD_NUM_LANES); laneNum++)
    {
        /* portsBmp contains only valid ports - must be verfied by caller function */
        if ((lanesBmp & (1<<laneNum)) == 0) continue;

        GT_APPDEMO_PHY_7120_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(pDev,laneNum,mdioPort,laneOffset);

        if(mydGetSerdesAddress(pDev,mdioPort,mydHostOrLineSide,laneOffset,&sAddr) == MYD_OK)
        {
            /* Check ical pcal order */
            mydRc = mydSerdesTuneCTLE(pDev,sAddr,dc,lf,hf,bw,MYD_FIXED_CTLE_DISABLED);
            if(mydRc != MYD_OK)
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("mydSerdesTuneCTLE Failed", mydRc);
                rc = GT_FAIL;
                continue;
            }
            mydRc = mydSerdesTune_pCal(pDev,sAddr,MYD_DFE_START_ADAPTIVE);
            if(mydRc != MYD_OK)
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("mydSerdesTune_pCal Failed", mydRc);
                rc = GT_FAIL;
                continue;
            }
        }
    }

    return rc;
}

/**
 * gtAppDemoPhy7120TxConfigSet
 *
 *
 * @param phyInfo
 * @param hostOrLineSide
 * @param lanesBmp
 * @param tuneParamsPtr
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhy7120TxConfigSet
(
   GT_APPDEMO_XPHY_INFO_PTR                 phyInfo,
   GT_APPDEMO_XPHY_HOST_LINE                hostOrLineSide,
   GT_U32                                   lanesBmp,
   CPSS_PORT_SERDES_TUNE_STC                *tuneParamsPtr
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          laneNum;

    MYD_STATUS     mydRc;
    MYD_DEV_PTR    pDev = NULL;
    MYD_U16        mdioPort=0;
    MYD_U16        laneOffset=0;
    MYD_U16        mydHostOrLineSide;
    MYD_U16        sAddr;

    MYD_16         preCursor;
    MYD_16         pre2Cursor;
    MYD_16         pre3Cursor;
    MYD_16         attenuation;
    MYD_16         postCursor;

    GT_APPDEMO_PHY_INFO_DRIVER_CHECK_AND_RETURN(phyInfo);
    pDev = (MYD_DEV_PTR)phyInfo->driverObj;

    if(tuneParamsPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    tuneParamsPtr->type = CPSS_PORT_SERDES_AVAGO_E;
    preCursor   = (MYD_16)tuneParamsPtr->txTune.avago.pre;
    pre2Cursor  = (MYD_16)tuneParamsPtr->txTune.avago.pre2;
    pre3Cursor  = (MYD_16)tuneParamsPtr->txTune.avago.pre3;
    attenuation = (MYD_16)tuneParamsPtr->txTune.avago.atten;
    postCursor  = (MYD_16)tuneParamsPtr->txTune.avago.post;

    switch (hostOrLineSide)
    {
        case GT_APPDEMO_XPHY_HOST_SIDE:
            mydHostOrLineSide = MYD_HOST_SIDE;
            break;
        case GT_APPDEMO_XPHY_LINE_SIDE:
            mydHostOrLineSide = MYD_LINE_SIDE;
            break;
        default:
            return GT_NOT_SUPPORTED;
            break;
    }

    for (laneNum = 0; laneNum < (pDev->portCount*MYD_NUM_LANES); laneNum++)
    {
        /* portsBmp contains only valid ports - must be verfied by caller function */
        if ((lanesBmp & (1<<laneNum)) == 0) continue;

        GT_APPDEMO_PHY_7120_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(pDev,laneNum,mdioPort,laneOffset);

        if(mydGetSerdesAddress(pDev,mdioPort,mydHostOrLineSide,laneOffset,&sAddr) == MYD_OK)
        {
            mydRc = mydSerdesSetTxFFE(pDev,sAddr,preCursor,pre2Cursor,pre3Cursor,attenuation,postCursor);
            if(mydRc != MYD_OK)
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("mydSerdesSetTxFFE Failed", mydRc);
                rc = GT_FAIL;
                continue;
            }
        }
    }

    return rc;
}

/**
 * gtAppDemoPhy7120Loopback
 *
 *
 * @param phyInfo
 * @param lanesBmp
 * @param hostOrLineSide
 * @param loopback_type
 * @param enable
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhy7120Loopback
(
    GT_APPDEMO_XPHY_INFO_PTR                 phyInfo,
    GT_U32                                   lanesBmp,
    GT_APPDEMO_XPHY_HOST_LINE                hostOrLineSide,
    GT_U16                                   loopback_type,
    GT_U16                                   enable
)
{
    GT_U32         laneNum;
    MYD_STATUS     mydRc = GT_OK;
    MYD_DEV_PTR    pDev = NULL;
    MYD_U16        mdioPort=0;
    MYD_U16        laneOffset=0;

    GT_APPDEMO_PHY_INFO_DRIVER_CHECK_AND_RETURN(phyInfo);
    pDev = (MYD_DEV_PTR)phyInfo->driverObj;

    for (laneNum = 0; laneNum < (pDev->portCount*MYD_NUM_LANES); laneNum++)
    {
        /* portsBmp contains only valid ports - must be verfied by caller function */
        if ((lanesBmp & (1<<laneNum)) == 0) continue;

        GT_APPDEMO_PHY_7120_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(pDev,laneNum,mdioPort,laneOffset);
        if(hostOrLineSide == GT_APPDEMO_XPHY_HOST_SIDE)
        {
            mydRc = mydSetPCSHostLoopback(pDev,mdioPort,laneOffset,loopback_type, enable) ;
        }
        if(hostOrLineSide == GT_APPDEMO_XPHY_LINE_SIDE)
        {
            mydRc = mydSetPCSLineLoopback(pDev,mdioPort,laneOffset,loopback_type, enable) ;
        }
        if( mydRc != MYD_OK)
        {
            osPrintf("mydSetPCSLineLoopback fail rc =%d \n",mydRc);
        }
    }

    return GT_OK ;
}

/**
 * gtAppDemoPhy7120ConfigurePktGeneratorChecker
 *
 *
 * @param phyInfo
 * @param lanesBmp
 * @param hostOrLineSide
 * @param readToClear
 * @param frameLengthControl
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhy7120ConfigurePktGeneratorChecker
(
   GT_APPDEMO_XPHY_INFO_PTR                 phyInfo,
   GT_U32                                   lanesBmp,
   GT_APPDEMO_XPHY_HOST_LINE               hostOrLineSide,
   GT_BOOL  readToClear,
   GT_U16 frameLengthControl

)
{
    GT_U32          laneNum;
    MYD_U16        mydHostOrLineSide;
    MYD_STATUS     mydRc = GT_OK;
    MYD_DEV_PTR    pDev = NULL;
    MYD_U16        mdioPort=0;
    MYD_U16        laneOffset=0;



    MYD_BOOL dontuseSFDinChecker = MYD_TRUE;
    MYD_U16 pktPatternControl = 0;
    MYD_BOOL generateCRCoff = MYD_FALSE;
    MYD_U16 initialPayload = 0;

    MYD_U16 numPktsToSend = 0xFFFF;
    MYD_BOOL randomIPG = MYD_FALSE;
    MYD_U16 ipgDuration = 2;

    GT_APPDEMO_PHY_INFO_DRIVER_CHECK_AND_RETURN(phyInfo);
    pDev = (MYD_DEV_PTR)phyInfo->driverObj;

    switch (hostOrLineSide)
    {
        case GT_APPDEMO_XPHY_HOST_SIDE:
            mydHostOrLineSide = MYD_HOST_SIDE;
            break;
        case GT_APPDEMO_XPHY_LINE_SIDE:
            mydHostOrLineSide = MYD_LINE_SIDE;
            break;
        default:
            return GT_NOT_SUPPORTED;
            break;
    }

    for (laneNum = 0; laneNum < (pDev->portCount*MYD_NUM_LANES); laneNum++)
    {
        /* portsBmp contains only valid ports - must be verfied by caller function */
        if ((lanesBmp & (1<<laneNum)) == 0) continue;

        GT_APPDEMO_PHY_7120_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(pDev,laneNum,mdioPort,laneOffset);
        mydRc = mydConfigurePktGeneratorChecker(pDev, mdioPort, mydHostOrLineSide, laneOffset, readToClear,
                            dontuseSFDinChecker, pktPatternControl, generateCRCoff, initialPayload,
                            frameLengthControl, numPktsToSend, randomIPG, ipgDuration);
        if( mydRc != MYD_OK)
        {
            osPrintf("mydConfigurePktGeneratorChecker fail rc =%d \n", mydRc);
        }
    }

    return GT_OK ;
}

/**
 * gtAppDemoPhy7120EnablePktGeneratorChecker
 *
 *
 * @param phyInfo
 * @param lanesBmp
 * @param hostOrLineSide
 * @param enableGenerator
 * @param enableChecker
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhy7120EnablePktGeneratorChecker
(
   GT_APPDEMO_XPHY_INFO_PTR                 phyInfo,
   GT_U32                                   lanesBmp,
   GT_APPDEMO_XPHY_HOST_LINE               hostOrLineSide,
   GT_BOOL  enableGenerator ,
   GT_BOOL  enableChecker

)
{
    GT_U32          laneNum;
    MYD_U16        mydHostOrLineSide;
    MYD_STATUS     mydRc;
    MYD_DEV_PTR    pDev = NULL;
    MYD_U16        mdioPort=0;
    MYD_U16        laneOffset=0;

    GT_APPDEMO_PHY_INFO_DRIVER_CHECK_AND_RETURN(phyInfo);
    pDev = (MYD_DEV_PTR)phyInfo->driverObj;

    switch (hostOrLineSide)
    {
        case GT_APPDEMO_XPHY_HOST_SIDE:
            mydHostOrLineSide = MYD_HOST_SIDE;
            break;
        case GT_APPDEMO_XPHY_LINE_SIDE:
            mydHostOrLineSide = MYD_LINE_SIDE;
            break;
        default:
            return GT_NOT_SUPPORTED;
            break;
    }

    for (laneNum = 0; laneNum < (pDev->portCount*MYD_NUM_LANES); laneNum++)
    {
        /* portsBmp contains only valid ports - must be verfied by caller function */
        if ((lanesBmp & (1<<laneNum)) == 0) continue;

        GT_APPDEMO_PHY_7120_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(pDev,laneNum,mdioPort,laneOffset);
        mydRc = mydEnablePktGeneratorChecker(pDev, mdioPort, mydHostOrLineSide, laneOffset,
                                             enableGenerator, enableChecker);
        if( mydRc != MYD_OK)
        {
            osPrintf("mydEnablePktGeneratorChecker fail rc =%d \n", mydRc);
        }
    }

    return GT_OK ;
}

/**
 * gtAppDemoPhy7120PhyGetThermal
 *
 *
 * @param phyInfo
 * @param lanesBmp
 * @param thermal
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhy7120PhyGetThermal
(
   GT_APPDEMO_XPHY_INFO_PTR                 phyInfo,
   GT_U32                                   lanesBmp,
   GT_32   *thermal
)
{

    GT_U32         laneNum, laneOffset;
    MYD_STATUS     mydRc = GT_OK;
    MYD_DEV_PTR    pDev = NULL;
    MYD_U16        mdioPort=0;


    GT_APPDEMO_PHY_INFO_DRIVER_CHECK_AND_RETURN(phyInfo);
    pDev = (MYD_DEV_PTR)phyInfo->driverObj;

    for (laneNum = 0; laneNum < (pDev->portCount*MYD_NUM_LANES); laneNum++)
    {
        /* portsBmp contains only valid ports - must be verfied by caller function */
        if ((lanesBmp & (1<<laneNum)) == 0) continue;

        GT_APPDEMO_PHY_7120_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(pDev,laneNum,mdioPort,laneOffset);
        (void)laneOffset;
        mydRc = mydSerdesGetTemperature(pDev, mdioPort, (MYD_32*) thermal );
    }

    return mydRc;
}

/**
 * gtAppDemoPhy7120DiagStateDump
 *
 *
 * @param phyInfo
 * @param lanesBmp
 * @param hostOrLineSide
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhy7120DiagStateDump
(
   GT_APPDEMO_XPHY_INFO_PTR                phyInfo,
   GT_U32                                  lanesBmp,
   GT_APPDEMO_XPHY_HOST_LINE               hostOrLineSide

)
{
    GT_U32         laneNum;
    MYD_U16        mydHostOrLineSide;
    MYD_STATUS     mydRc;
    MYD_DEV_PTR    pDev = NULL;
    MYD_U16        mdioPort=0;
    MYD_U16        laneOffset=0;

    MYD_32  coreTemperature;
    MYD_U16 voltageCore;
    MYD_U16 voltageDVDD;
    MYD_U16 voltageAVDD;

    MYD_16 preCursor;
    MYD_16 pre2Cursor;
    MYD_16 pre3Cursor;
    MYD_16 attenuation;
    MYD_16 postCursor;

    MYD_U16 mdioPortIndex;
    MYD_U16 sAddr;

    MYD_SER_EYE_DATA_TYPE *pEyeDataBuf = NULL;

    GT_APPDEMO_PHY_INFO_DRIVER_CHECK_AND_RETURN(phyInfo);
    pDev = (MYD_DEV_PTR)phyInfo->driverObj;

    switch (hostOrLineSide)
    {
        case GT_APPDEMO_XPHY_HOST_SIDE:
            mydHostOrLineSide = MYD_HOST_SIDE;
            break;
        case GT_APPDEMO_XPHY_LINE_SIDE:
            mydHostOrLineSide = MYD_LINE_SIDE;
            break;
        default:
            return GT_NOT_SUPPORTED;
            break;
    }

    for (laneNum = 0; laneNum < (pDev->portCount*MYD_NUM_LANES); laneNum++)
    {
        /* portsBmp contains only valid ports - must be verfied by caller function */
        if ((lanesBmp & (1<<laneNum)) == 0) continue;

        GT_APPDEMO_PHY_7120_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(pDev,laneNum,mdioPort,laneOffset);
        mydSerdesGetTemperature(pDev, mdioPort, &coreTemperature);
        mydSerdesGetVoltage(pDev, mdioPort, &voltageCore, &voltageDVDD, &voltageAVDD);

        osPrintf("coreTemperature =%d \n", coreTemperature);
        osPrintf("voltageCore =%d  voltageDVDD=%d voltageAVDD=%d \n", voltageCore, voltageDVDD , voltageAVDD);

        mdioPortIndex = MYD_GET_PORT_IDX(pDev, mdioPort);

        if (mydHostOrLineSide == MYD_HOST_SIDE)
        {
            sAddr = MYD_MAPPED_SERDES_ID(mdioPortIndex, MYD_HOST_MODE_IDX, laneOffset);
            /*
            controlANEGReg = MYD_AUTONEG_CTRL_REG_HOST+(0x0200*laneOffset);
            */
        }
        else
        {
            sAddr = MYD_MAPPED_SERDES_ID(mdioPortIndex, MYD_LINE_MODE_IDX, laneOffset);
            /*
            controlANEGReg = MYD_AUTONEG_CTRL_REG_LINE+(0x0200*laneOffset);
            */
        }

        mydRc = mydSerdesGetTxFFE(pDev, sAddr,  &preCursor, &pre2Cursor, &pre3Cursor, &attenuation, &postCursor);
        if (mydRc == MYD_OK)
        {

            osPrintf("phyInfo->portGroupId =%d mdioPort=%d laneOffset=%d \n", phyInfo->portGroupId, mdioPort, laneOffset);
            osPrintf("preCursor =%u pre2Cursor=%u pre3Cursor=%u attenuation=%u postCursor=%u \n", \
            preCursor, pre2Cursor, pre3Cursor, attenuation, postCursor);
        }
        else
        {
            osPrintf("mydSerdesGetTxFFE error code =%d \n", mydRc);
        }

        /* call mydWrapperEyeAllocDataSt to malloc the memory for the eye data structure */
        pEyeDataBuf = mydWrapperEyeAllocDataSt((MYD_SER_DEV_PTR)pDev->serdesDev[MYD_GET_PORT_IDX(pDev,mdioPort)]);

        mydRc = mydSerdesGetEye(pDev, sAddr, 6, MYD_DISABLE, 2, pEyeDataBuf);
        if (mydRc == MYD_OK)
        {
            osPrintf("x.UI:         %3d\n",pEyeDataBuf->ed_x_UI);
            osPrintf("x.resolution: %3d\n",pEyeDataBuf->ed_x_resolution);
            osPrintf("x.points:     %3d\n",pEyeDataBuf->ed_x_points);
            osPrintf("x.min:        %3d\n",pEyeDataBuf->ed_x_min);
            osPrintf("x.max:        %3d\n",pEyeDataBuf->ed_x_max);
            osPrintf("x.step:       %3d\n",pEyeDataBuf->ed_x_step);
            osPrintf("x.center:     %3d\n",(pEyeDataBuf->ed_x_min + pEyeDataBuf->ed_x_max) / 2);
            osPrintf("x.width:      %3d\n",pEyeDataBuf->ed_width);
            osPrintf("x.width_mUI:  %3d\n",pEyeDataBuf->ed_width_mUI);
            osPrintf("\n");

            osPrintf("y.resolution: %3d (DAC points)\n",pEyeDataBuf->ed_y_resolution);
            osPrintf("y.points:     %3d\n",pEyeDataBuf->ed_y_points);
            osPrintf("y.step:       %3d\n",pEyeDataBuf->ed_y_step);
            osPrintf("y.min:        %3d (DAC for y=0)\n",pEyeDataBuf->ed_y_min);
            osPrintf("y.mission_points: %3d\n",pEyeDataBuf->ed_y_mission_points);
            osPrintf("y.mission_step:   %3d\n",pEyeDataBuf->ed_y_mission_step);
            osPrintf("y.mission_min:    %3d (DAC for y=0)\n",pEyeDataBuf->ed_y_mission_min);

            osPrintf("y.center:     %3d (DAC middle)\n",pEyeDataBuf->ed_y_min + pEyeDataBuf->ed_y_points / 2 * pEyeDataBuf->ed_y_step);
            osPrintf("y.height:     %3d\n",pEyeDataBuf->ed_height);
            osPrintf("y.height_mV:  %3d\n",pEyeDataBuf->ed_height_mV);
            osPrintf("\n");

            osPrintf("dc_balance:    %6.2g\n", pEyeDataBuf->ed_dc_balance);
            osPrintf("trans_density: %6.2g\n", pEyeDataBuf->ed_trans_density);
            osPrintf("error_threshold: %d\n",  pEyeDataBuf->ed_error_threshold);
            osPrintf("min_dwell_bits:  %g\n",  pEyeDataBuf->ed_min_dwell_bits*1.0);
            osPrintf("max_dwell_bits:  %g\n",  pEyeDataBuf->ed_max_dwell_bits*1.0);
            osPrintf("fast_dynamic:    %d\n",  pEyeDataBuf->ed_fast_dynamic);
            osPrintf("\n");
        }
        else
        {
            osPrintf("mydSerdesGetEye error code =%d \n", mydRc);
        }

        /* pEyeDataBuf must be freed */
        mydWrapperEyeFreeDataSt((MYD_SER_DEV_PTR)pDev->serdesDev[MYD_GET_PORT_IDX(pDev,mdioPort)], pEyeDataBuf);
    }

    return GT_OK ;
}

/**
 * gtAppDemoPhy7120PktGeneratorGetCounter
 *
 *
 * @param phyInfo
 * @param lanesBmp
 * @param hostOrLineSide
 * @param whichCounter
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhy7120PktGeneratorGetCounter
(
   GT_APPDEMO_XPHY_INFO_PTR                 phyInfo,
   GT_U32                                   lanesBmp,
   GT_APPDEMO_XPHY_HOST_LINE               hostOrLineSide,
   GT_U16  whichCounter

)
{

    GT_U32          laneNum;
    MYD_U16        mydHostOrLineSide;
    MYD_STATUS     mydRc;
    MYD_DEV_PTR    pDev = NULL;
    MYD_U16        mdioPort=0;
    MYD_U16        laneOffset=0;

    MYD_U64 rxPktCount ;
    MYD_U64 rxByteCount ;

    GT_APPDEMO_PHY_INFO_DRIVER_CHECK_AND_RETURN(phyInfo);
    pDev = (MYD_DEV_PTR)phyInfo->driverObj;

     switch (hostOrLineSide)
    {
        case GT_APPDEMO_XPHY_HOST_SIDE:
            mydHostOrLineSide = MYD_HOST_SIDE;
            break;
        case GT_APPDEMO_XPHY_LINE_SIDE:
            mydHostOrLineSide = MYD_LINE_SIDE;
            break;
        default:
            return GT_NOT_SUPPORTED;
            break;
    }

    for (laneNum = 0; laneNum < (pDev->portCount*MYD_NUM_LANES); laneNum++)
    {
        /* portsBmp contains only valid ports - must be verfied by caller function */
        if ((lanesBmp & (1<<laneNum)) == 0) continue;

        GT_APPDEMO_PHY_7120_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(pDev,laneNum,mdioPort,laneOffset);
        mydRc = mydPktGeneratorGetCounter(pDev, mdioPort, mydHostOrLineSide, laneOffset,
                                          whichCounter, &rxPktCount, &rxByteCount);
        if( mydRc != MYD_OK)
        {
            osPrintf("mydEnablePktGeneratorChecker fail rc =%d \n", mydRc);
        }
        else
        {
            osPrintf(" PktCount = %I64u \n", rxPktCount);

        }
    }

    return GT_OK ;
}

/**
 * gtAppDemoPhy7120PktGeneratorCounterReset
 *
 *
 * @param phyInfo
 * @param lanesBmp
 * @param hostOrLineSide
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhy7120PktGeneratorCounterReset
(
   GT_APPDEMO_XPHY_INFO_PTR                 phyInfo,
   GT_U32                                   lanesBmp,
   GT_APPDEMO_XPHY_HOST_LINE               hostOrLineSide

)
{

    GT_U32         laneNum;
    MYD_U16        mydHostOrLineSide;
    MYD_STATUS     mydRc;
    MYD_DEV_PTR    pDev = NULL;
    MYD_U16        mdioPort=0;
    MYD_U16        laneOffset=0;

    GT_APPDEMO_PHY_INFO_DRIVER_CHECK_AND_RETURN(phyInfo);
    pDev = (MYD_DEV_PTR)phyInfo->driverObj;

     switch (hostOrLineSide)
    {
        case GT_APPDEMO_XPHY_HOST_SIDE:
            mydHostOrLineSide = MYD_HOST_SIDE;
            break;
        case GT_APPDEMO_XPHY_LINE_SIDE:
            mydHostOrLineSide = MYD_LINE_SIDE;
            break;
        default:
            return GT_NOT_SUPPORTED;
            break;
    }

    for (laneNum = 0; laneNum < (pDev->portCount*MYD_NUM_LANES); laneNum++)
    {
        /* portsBmp contains only valid ports - must be verfied by caller function */
        if ((lanesBmp & (1<<laneNum)) == 0) continue;

        GT_APPDEMO_PHY_7120_LANENUM_TO_MDIOPORT_AND_LANEOFFSET(pDev,laneNum,mdioPort,laneOffset);
        mydRc = mydPktGeneratorCounterReset(pDev, mdioPort, mydHostOrLineSide, laneOffset);

        if( mydRc != MYD_OK)
        {
            osPrintf("mydEnablePktGeneratorChecker fail rc =%d \n", mydRc);
        }
    }

    return GT_OK ;
}

/**
 * gtAppDemoPhy7120BindFuncPtr
 *
 *
 * @param funcPtrArray
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhy7120BindFuncPtr(GT_APPDEMO_XPHY_FUNC_PTRS *funcPtrArray)
{
    funcPtrArray[GT_APPDEMO_XPHY_88X7120].gtAppDemoPhyInitDriverFunc          = gtAppDemoPhy7120InitDriver;
    funcPtrArray[GT_APPDEMO_XPHY_88X7120].gtAppDemoPhyArrayInitFunc           = gtAppDemoPhy7120BroadcastInit;
    funcPtrArray[GT_APPDEMO_XPHY_88X7120].gtAppDemoPhyUnloadDriverFunc        = gtAppDemoPhy7120UnloadDriver;
    funcPtrArray[GT_APPDEMO_XPHY_88X7120].gtAppDemoPhySetPortModeFunc         = gtAppDemoPhy7120SetPortMode;
    funcPtrArray[GT_APPDEMO_XPHY_88X7120].gtAppDemoPhyGetPortModeFunc         = gtAppDemoPhy7120GetPortMode;
    funcPtrArray[GT_APPDEMO_XPHY_88X7120].gtAppDemoPhyPortPowerdownFunc       = gtAppDemoPhy7120PortPowerdown;
    funcPtrArray[GT_APPDEMO_XPHY_88X7120].gtAppDemoPhyGetPortLinkStatusFunc   = gtAppDemoPhy7120GetPortLinkStatus;
    funcPtrArray[GT_APPDEMO_XPHY_88X7120].gtAppDemoPhyGetPortLanesBmpFunc     = gtAppDemoPhy7120GetPortLanesBmp;
    funcPtrArray[GT_APPDEMO_XPHY_88X7120].gtAppDemoPhySetLanePolaritySwapFunc = gtAppDemoPhy7120SetLanePolaritySwap;
    funcPtrArray[GT_APPDEMO_XPHY_88X7120].gtAppDemoPhySerdesTuneFunc          = gtAppDemoPhy7120SerdesTune;
    funcPtrArray[GT_APPDEMO_XPHY_88X7120].gtAppDemoPhySerdesTxConfigSetFunc   = gtAppDemoPhy7120TxConfigSet;
    funcPtrArray[GT_APPDEMO_XPHY_88X7120].gtAppDemoPhyPktGeneratorGetCounterFunc   = gtAppDemoPhy7120PktGeneratorGetCounter;
    funcPtrArray[GT_APPDEMO_XPHY_88X7120].gtAppDemoPhyPktGeneratorCounterResetFunc   = gtAppDemoPhy7120PktGeneratorCounterReset;
    funcPtrArray[GT_APPDEMO_XPHY_88X7120].gtAppDemoPhyEnablePktGeneratorCheckerFunc   = gtAppDemoPhy7120EnablePktGeneratorChecker;
    funcPtrArray[GT_APPDEMO_XPHY_88X7120].gtAppDemoPhyConfigurePktGeneratorCheckerFunc   = gtAppDemoPhy7120ConfigurePktGeneratorChecker;
    funcPtrArray[GT_APPDEMO_XPHY_88X7120].gtAppDemoPhyLoopbackFunc   = gtAppDemoPhy7120Loopback;
    funcPtrArray[GT_APPDEMO_XPHY_88X7120].gtAppDemoPhyDiagStateDumpFunc = gtAppDemoPhy7120DiagStateDump ;
    funcPtrArray[GT_APPDEMO_XPHY_88X7120].gtAppDemoPhyGetTemperatureFunc = gtAppDemoPhy7120PhyGetThermal ;
    funcPtrArray[GT_APPDEMO_XPHY_88X7120].gtAppDemoPhyXSmiExtentionFunc = NULL;

    return GT_OK;
}

/**
 *
 * gtAppDemoPhy7120BindXSmiExtnFuncPtr
 *
 * @param funcPtrArray
 *
 * @return GT_STATUS
 */
GT_STATUS gtAppDemoPhy7120BindXSmiExtnFuncPtr(GT_APPDEMO_XPHY_FUNC_PTRS *funcPtrArray, const void * funcPtr)
{
    funcPtrArray[GT_APPDEMO_XPHY_88X7120].gtAppDemoPhyXSmiExtentionFunc = funcPtr;
    return GT_OK;
}
#endif
