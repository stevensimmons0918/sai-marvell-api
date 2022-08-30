/********************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file cpssDxChPortIfModeCfg.c
*
* @brief CPSS implementation for Port interface mode configuration.
*
*
* @version   116
********************************************************************************
*/
/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>

#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitLedCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInitLedCtrl.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBcat2Resource.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgBobKResource.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortIfModeCfgResource.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortDynamicPAPortSpeedDB.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortInterlaken.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssDxChPortTxQHighSpeedPortsDrv.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqPreemptionUtils.h>

#include <cpss/dxCh/dxChxGen/ptp/cpssDxChPtp.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>

#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/DynamicPizzaArbiter/cpssDxChPortDynamicPizzaArbiterWorkConserving.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsLion2.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>

/* TM glue */
#include <cpss/dxCh/dxChxGen/tmGlue/private/prvCpssDxChTmGlue.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortAp.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceFalcon.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceHawk.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortDpIronman.h>
#include <cpss/common/port/cpssPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortPacketBuffer.h>

#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/falcon/pipe/prvCpssFalconTxqSdqRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqSdq.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgEgrFlt.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrg.h>
#include <cpss/dxCh/dxChxGen/cutThrough/private/prvCpssDxChCutThrough.h>
#include <cpss/dxCh/dxChxGen/port/macCtrl/prvCpssDxChMacCtrl.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/generic/labservices/port/gop/silicon/falcon/mvHwsFalconPortIf.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqSearchUtils.h>
#include <cpss/dxCh/dxChxGen/cutThrough/cpssDxChCutThrough.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/*global variables macros*/

#define PRV_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portCtrlSrc._var)


#define PRV_SHARED_PORT_DIR_PORT_SERDES_CFG_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portSerdesCfgSrc._var)


#define NEW_PORT_INIT_SEQ

/* array defining serdes speed used in given interface mode for given port data speed */
/* APPLICABLE DEVICES:  DxChXcat and above. */
extern CPSS_DXCH_PORT_SERDES_SPEED_ENT serdesFrequency[CPSS_PORT_INTERFACE_MODE_NA_E][CPSS_PORT_SPEED_NA_E];

extern GT_VOID prvCpssDxChPortIlknPrMemInit
(
    GT_VOID
);

/* Perform WA to Rxaui port in AC3X device, to overcome link flapping issue
   found during power cycle test */
extern GT_STATUS prvCpssDxChAc3xRxauiPortWA
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum
);

/* Check whether given port is connected to 1690 PHY device */
extern GT_STATUS prvCpssDxChAc3xIsPortConnectedTo1690Phy
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM physicalPort,
    OUT GT_BOOL              *portConnectedTo1690Phy
);

extern GT_STATUS prvCpssDxChTxRequestMaskSet
(
    IN GT_U8    devNum,
    IN GT_U32   maskIndex,
    IN GT_U32   maskValue
);

extern GT_STATUS prvCpssDxChPortTxQUnitDQHighSpeedPortEnable
(
    IN GT_U8       devNum,
    IN GT_U32      highSpeedPortIdx,
    IN GT_PORT_NUM portNum
);

extern GT_STATUS prvCpssDxChPortTxQUnitDQHighSpeedPortDisable
(
    IN GT_U8    devNum,
    IN GT_U32   highSpeedPortIdx
);


GT_STATUS prvSpeedPortSpeedChangeAllowOsPrintSet(IN GT_U32 allowPrint)
{
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portIfModeCfgSrc.prvPrintSpeedChangeTime) = allowPrint;
    return GT_OK;
}

GT_STATUS prvSpeedPortSpeedChangeAllowOsPrintGet(GT_VOID)
{
    return PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portIfModeCfgSrc.prvPrintSpeedChangeTime);
}

#define OS_SPEED_CHANGE_PRINTF(_x)    do\
                                       {\
                                           if(prvSpeedPortSpeedChangeAllowOsPrintGet())\
                                           {\
                                             cpssOsPrintf _x;\
                                           }\
                                       }while(0);

#define supportedPortsModesList PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portIfModeCfgSrc.supportedPortsModesList)

/* Convert cpss fec mode to hws mode */
#define PRV_CPSS_CONVERT_CPSS_FEC_MODE_VAL_TO_HWS_MAC(_fecMode, _hwsFecMode)      \
    switch (_fecMode)                                                   \
    {                                                                   \
        case CPSS_DXCH_PORT_FEC_MODE_ENABLED_E:                         \
            _hwsFecMode = FC_FEC;                                       \
            break;                                                      \
        case CPSS_DXCH_PORT_FEC_MODE_DISABLED_E:                        \
            _hwsFecMode = FEC_OFF;                                      \
            break;                                                      \
        case CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E:                      \
            _hwsFecMode = RS_FEC;                                       \
            break;                                                      \
        case CPSS_DXCH_PORT_RS_FEC_544_514_MODE_ENABLED_E:              \
            _hwsFecMode = RS_FEC_544_514;                               \
            break;                                                      \
        default:                                                        \
            _hwsFecMode =  (FEC_NA);                                    \
    }


static GT_STATUS prvCpssDxChPortIsAlreadyConfigured
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    prevIfMode,
    IN  CPSS_PORT_SPEED_ENT             prevSpeed,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    newIfMode,
    IN  CPSS_PORT_SPEED_ENT             newSpeed,
    OUT GT_BOOL                        *isPortAlreadyConfigured
)
{
    GT_STATUS rc;
    GT_BOOL   linkUp;

    if(NULL == isPortAlreadyConfigured)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    *isPortAlreadyConfigured = GT_FALSE;
    if (GT_FALSE == powerUp)
        return GT_OK;
    /* remote port in falcon not have mac so the ifmode and speed is not relevant to link*/
    if ((GT_TRUE == prvCpssDxChPortRemotePortCheck(devNum,portNum)) &&(PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
    {
        return GT_OK;
    }
    if ((prevIfMode == newIfMode) && (prevSpeed == newSpeed))
    {
        rc = cpssDxChPortLinkStatusGet(devNum, portNum, &linkUp);
        if (rc != GT_OK)
        {
            return rc;
        }
         if(linkUp)
         {
             *isPortAlreadyConfigured = GT_TRUE;
         }
    }

 /*   if(linkUp)
    {
        if((prevIfMode == newIfMode) && (prevSpeed == newSpeed))
        {
            *isPortAlreadyConfigured = GT_TRUE;
        }
    }*/
    return GT_OK;
}




/*static*/ GT_STATUS prvCpssDxChPortImplementWaSGMII2500
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS rc = GT_OK;  /* result status */
    GT_U32    regAddr;     /* register address */
    GT_U32    portMacMap;  /* port Mac Map */

    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_IMPLEMENT_WA_2_5G_SGMII_LINK_UP_E))
    {
        PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum,  portMacMap);
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacMap].serdesCnfg;
        if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if((CPSS_PORT_SPEED_2500_E == speed) && ((CPSS_PORT_INTERFACE_MODE_SGMII_E == ifMode)||(CPSS_PORT_INTERFACE_MODE_2500BASE_X_E == ifMode)))
        {
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 14, 2, 0x3);
        }
        else
        {
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 14, 2, 0x0);
        }
    }
    return rc;
}

static GT_STATUS prvCpssDxChPortWa10gXlgEnable
(   IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN GT_BOOL                          enable
)
{
#if 1
    devNum  = devNum;
    portNum = portNum;
    speed   = speed;
    enable  = enable;
#else
        /* This Errata implementation cuases the link to not link up under traffic
           so currently the decision is to discard this Errata */
    GT_U32 portMacNum;
    GT_U32 portGroupId;
    GT_U32 portMacType;
    GT_U32 regAddr;
    GT_STATUS rc;

    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_CAELUM_10G_XLG_LINK_UP_E))
    {
        PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

        portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum);

        portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacNum);

        if((PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) && (portMacType == PRV_CPSS_PORT_XLG_E))
        {
            PRV_CPSS_DXCH_PORT_MAC_CTRL4_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E, &regAddr);

            if(regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            if(CPSS_PORT_SPEED_10000_E == speed)
            {
                rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 14, 1, enable?1:0);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                }
            }
        }
    }
#endif
    return GT_OK;

}


CPSS_PORT_INTERFACE_MODE_ENT * prvCpssDxChPortTypeSupportedModeListGet
(
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT portType
)
{
    prvIfModeSupportedPortsModes_STC *listPtr;
    GT_U32 i;

    listPtr = &supportedPortsModesList[0];
    for (i = 0 ; listPtr->supportedIfList != NULL; i++,listPtr++)
    {
        if (listPtr->portType == portType)
        {
            return listPtr->supportedIfList;
        }
    }
    return (CPSS_PORT_INTERFACE_MODE_ENT *)NULL;
}


GT_BOOL prvCpssDxChPortTypeSupportedModeCheck
(
    IN PRV_CPSS_PORT_TYPE_OPTIONS_ENT portType,
    IN CPSS_PORT_INTERFACE_MODE_ENT   ifMode
)
{
    CPSS_PORT_INTERFACE_MODE_ENT * listPtr;
    GT_U32 i;

    listPtr = prvCpssDxChPortTypeSupportedModeListGet(portType);
    if (listPtr == NULL)
    {
        return GT_FALSE;
    }
    for (i = 0 ; *listPtr != CPSS_PORT_INTERFACE_MODE_NA_E ; i++, listPtr++)
    {
        if (*listPtr == ifMode)
        {
            return GT_TRUE;
        }
    }
    return GT_FALSE;
}

GT_STATUS prvCpssDxChPortSupportedModeCheck
(
    IN  GT_U8                         devNum,
    IN  GT_PHYSICAL_PORT_NUM          portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT  ifMode,
    OUT GT_BOOL                      *isSupportedPtr
)
{
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT portType;
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* check if interface mode supported by given port */
    portType = PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum,portMacNum);

    /* resOld = supportedPortsModes[PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum,portNum)][ifMode]; */
    *isSupportedPtr = prvCpssDxChPortTypeSupportedModeCheck(portType,ifMode);
    return GT_OK;
}


static GT_STATUS lion2FlexLinkIfModeCfg
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode
);

static GT_STATUS prvCpssDxChPortXcat3ModeSpeedSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORTS_BMP_STC              portsBmp,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

static GT_STATUS prvCpssDxChPortCaelumModeSpeedSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORTS_BMP_STC              portsBmp,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

static GT_STATUS prvCpssDxChPortFalconModeSpeedSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORTS_BMP_STC              portsBmp,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
);

static GT_STATUS prvCpssDxChPort_SIP6_10_ModeSpeedSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORTS_BMP_STC                  portsBmp,
    IN  GT_BOOL                             powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT        ifMode,
    IN  CPSS_PORT_SPEED_ENT                 speed,
    IN  PRV_CPSS_PORT_MNG_PORT_SM_DB_STC    *portMgrDbPtr
);

static GT_STATUS cpuPortIfModeGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
);

static GT_STATUS lionIfModeGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
);


/* Array bellow is table of pointers to ifMode configuration functions per DXCH ASIC family per interface mode
 * if mode not supported NULL pointer provided
 */
#if 0
    PRV_CPSS_DXCH_PORT_INTERFACE_MODE_SET_FUN portIfModeFuncPtrArray[CPSS_PP_FAMILY_END_DXCH_E-CPSS_PP_FAMILY_START_DXCH_E-1][CPSS_PORT_INTERFACE_MODE_NA_E]=
    {                                   /* REDUCED_10BIT_E  REDUCED_GMII_E              MII_E                           SGMII                           XGMII                         MGMII       1000BASE_X_E                    GMII                        MII_PHY_E                  QX                             HX                              RXAUI           100BASE_FX_E         QSGMII_E                XLG                    LOCAL_XGMII              KR                      HGL                     CHGL_12                  ILK   SR_LR                   XHGS*/
    /* CPSS_PP_FAMILY_CHEETAH_E     */  {      NULL,       cheetah1_3_GePortIfModeCfg,  cheetah1_3_GePortIfModeCfg, cheetah1_3_GePortIfModeCfg,      cheetah1_3_XgPortIfModeCfg,    NULL,   cheetah1_3_GePortIfModeCfg,     cheetah1_3_GePortIfModeCfg, cheetah1_3_GePortIfModeCfg, NULL,                       NULL,                       NULL,                   NULL,                   NULL,                   NULL,                   NULL,                    NULL,                   NULL,                    NULL,                   NULL, NULL,                   NULL                   },
    /* CPSS_PP_FAMILY_CHEETAH2_E    */  {      NULL,       cheetah1_3_GePortIfModeCfg,  cheetah1_3_GePortIfModeCfg, cheetah1_3_GePortIfModeCfg,      cheetah1_3_XgPortIfModeCfg,    NULL,   cheetah1_3_GePortIfModeCfg,     cheetah1_3_GePortIfModeCfg, cheetah1_3_GePortIfModeCfg, cheetah1_3_XgPortIfModeCfg, cheetah1_3_XgPortIfModeCfg, NULL,                   NULL,                   NULL,                   NULL,                   NULL,                    NULL,                   NULL,                    NULL,                   NULL, NULL,                   NULL                   },
    /* CPSS_PP_FAMILY_CHEETAH3_E    */  {      NULL,       NULL,                        cheetah1_3_GePortIfModeCfg, cheetah1_3_GePortIfModeCfg,      cheetah1_3_XgPortIfModeCfg,    NULL,   cheetah1_3_GePortIfModeCfg,     cheetah1_3_GePortIfModeCfg, NULL,                       NULL,                       NULL,                       NULL,                   NULL,                   NULL,                   NULL,                   NULL,                    NULL,                   NULL,                    NULL,                   NULL, NULL,                   NULL                   },
    /* CPSS_PP_FAMILY_DXCH_XCAT_E   */  {      NULL,       NULL,                        xCatFePortIfModeCfg,        xCatGePortIfModeCfg,             xCatXgPortIfModeCfg,           NULL,   xCatGePortIfModeCfg,            NULL,                       NULL,                       xCatXgPortIfModeCfg,        xCatXgPortIfModeCfg,        xCatXgPortIfModeCfg,    xCatGePortIfModeCfg,    xCatGePortIfModeCfg,    NULL,                   NULL,                    NULL,                   NULL,                    NULL,                   NULL, NULL,                   NULL                   },
    /* CPSS_PP_FAMILY_DXCH_XCAT3_E  */  {      NULL,       NULL,                        NULL,                       NULL,                            NULL,                          NULL,   NULL,                           NULL,                       NULL,                       NULL,                       NULL,                       NULL,                   NULL,                   NULL,                   NULL,                   NULL,                    NULL,                   NULL,                    NULL,                   NULL, NULL,                   NULL                   },
    /* CPSS_PP_FAMILY_DXCH_AC5_E    */  {      NULL,       NULL,                        NULL,                       NULL,                            NULL,                          NULL,   NULL,                           NULL,                       NULL,                       NULL,                       NULL,                       NULL,                   NULL,                   NULL,                   NULL,                   NULL,                    NULL,                   NULL,                    NULL,                   NULL, NULL,                   NULL                   },
    /* CPSS_PP_FAMILY_DXCH_LION_E   */  {      NULL,       NULL,                        NULL,                       lionFlexLinkIfModeCfg,           lionFlexLinkIfModeCfg,         NULL,   lionFlexLinkIfModeCfg,          NULL,                       NULL,                       NULL,                       lionFlexLinkIfModeCfg,      lionFlexLinkIfModeCfg,  NULL,                   NULL,                   lionFlexLinkIfModeCfg,  lionFlexLinkIfModeCfg,   NULL,                   NULL,                    NULL,                   NULL, NULL,                   NULL                   },
    /* CPSS_PP_FAMILY_DXCH_XCAT2_E  */  {      NULL,       NULL,                        xCatFePortIfModeCfg,        xCatGePortIfModeCfg,             NULL,                          NULL,   xCatGePortIfModeCfg,            NULL,                       NULL,                       NULL,                       NULL,                       NULL,                   xCatGePortIfModeCfg,    xCatGePortIfModeCfg,    NULL,                   NULL,                    NULL,                   NULL,                    NULL,                   NULL, NULL,                   NULL                   },
    /* CPSS_PP_FAMILY_DXCH_LION2_E  */  {      NULL,       NULL,                        NULL,                       lion2FlexLinkIfModeCfg,          lion2FlexLinkIfModeCfg,        NULL,   lion2FlexLinkIfModeCfg,         NULL,                       NULL,                       NULL,                       lion2FlexLinkIfModeCfg,     lion2FlexLinkIfModeCfg, lion2FlexLinkIfModeCfg, NULL,                   NULL,                   lion2FlexLinkIfModeCfg,  lion2FlexLinkIfModeCfg, lion2FlexLinkIfModeCfg,  NULL,                   NULL, lion2FlexLinkIfModeCfg, lion2FlexLinkIfModeCfg },
    /* CPSS_PP_FAMILY_DXCH_LION3_E  */  {      NULL,       NULL,                        NULL,                       lion2FlexLinkIfModeCfg,          lion2FlexLinkIfModeCfg,        NULL,   lion2FlexLinkIfModeCfg,         NULL,                       NULL,                       NULL,                       lion2FlexLinkIfModeCfg,     lion2FlexLinkIfModeCfg, NULL,                   NULL,                   NULL,                   lion2FlexLinkIfModeCfg,  lion2FlexLinkIfModeCfg, lion2FlexLinkIfModeCfg,  lion2FlexLinkIfModeCfg, NULL, lion2FlexLinkIfModeCfg, lion2FlexLinkIfModeCfg },
    /* CPSS_PP_FAMILY_DXCH_BOBCAT2_E  */{      NULL,       NULL,                        NULL,                       NULL,                            NULL,                          NULL,   NULL,                           NULL,                       NULL,                       NULL,                       NULL,                       NULL,                   NULL,                   NULL,                   NULL,                   NULL,                    NULL,                   NULL,                    NULL,                   NULL,                         NULL                   }
    };

    /* array of pointers to port interface mode get functions per DXCH ASIC family */
    PRV_CPSS_DXCH_PORT_INTERFACE_MODE_GET_FUN portIfModeGetFuncPtrArray[CPSS_PP_FAMILY_END_DXCH_E-CPSS_PP_FAMILY_START_DXCH_E-1]=
    {
    /* CPSS_PP_FAMILY_CHEETAH_E     */  cheetah1_3_PortIfModeGet,
    /* CPSS_PP_FAMILY_CHEETAH2_E    */  cheetah1_3_PortIfModeGet,
    /* CPSS_PP_FAMILY_CHEETAH3_E    */  cheetah1_3_PortIfModeGet,
    /* CPSS_PP_FAMILY_DXCH_XCAT_E   */  xcatPortIfModeGet,
    /* CPSS_PP_FAMILY_DXCH_XCAT3_E  */  lionIfModeGet,/*xcatPortIfModeGet,*/
    /* CPSS_PP_FAMILY_DXCH_AC5_E    */  lionIfModeGet,/*xcatPortIfModeGet,*/
    /* CPSS_PP_FAMILY_DXCH_LION_E   */  lionIfModeGet,
    /* CPSS_PP_FAMILY_DXCH_XCAT2_E  */  xcatPortIfModeGet,
    /* CPSS_PP_FAMILY_DXCH_LION2_E  */  lionIfModeGet,
    /* CPSS_PP_FAMILY_DXCH_LION3_E  */  lionIfModeGet,
    /* CPSS_PP_FAMILY_DXCH_BOBCAT2_E*/  lionIfModeGet
    };

#endif

typedef struct prvCpssIfModeSetFun_STC
{
    CPSS_PORT_INTERFACE_MODE_ENT                ifMode;
    PRV_CPSS_DXCH_PORT_INTERFACE_MODE_SET_FUN   fun;
}prvCpssIfModeSetFun_STC;

typedef struct prvCpssDev_IfMode_Fun_STC
{
    CPSS_PP_FAMILY_TYPE_ENT                   familyType;
    prvCpssIfModeSetFun_STC                  *ifModeSetFuncListPtr;
    PRV_CPSS_DXCH_PORT_INTERFACE_MODE_GET_FUN ifModeGetFunc;
}prvCpssDev_IfMode_Fun_STC;


prvCpssIfModeSetFun_STC prvCpssIfModeFun_Lion2_List[] =
{
     { CPSS_PORT_INTERFACE_MODE_SGMII_E,        lion2FlexLinkIfModeCfg }
    ,{ CPSS_PORT_INTERFACE_MODE_XGMII_E,        lion2FlexLinkIfModeCfg }
    ,{ CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,   lion2FlexLinkIfModeCfg }
    ,{ CPSS_PORT_INTERFACE_MODE_HX_E,           lion2FlexLinkIfModeCfg }
    ,{ CPSS_PORT_INTERFACE_MODE_RXAUI_E,        lion2FlexLinkIfModeCfg }
    ,{ CPSS_PORT_INTERFACE_MODE_100BASE_FX_E,   lion2FlexLinkIfModeCfg }
    ,{ CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E,  lion2FlexLinkIfModeCfg }
    ,{ CPSS_PORT_INTERFACE_MODE_KR_E,           lion2FlexLinkIfModeCfg }
    ,{ CPSS_PORT_INTERFACE_MODE_HGL_E,          lion2FlexLinkIfModeCfg }
    ,{ CPSS_PORT_INTERFACE_MODE_SR_LR_E,        lion2FlexLinkIfModeCfg }
    ,{ CPSS_PORT_INTERFACE_MODE_XHGS_E,         lion2FlexLinkIfModeCfg }
    ,{ CPSS_PORT_INTERFACE_MODE_XHGS_SR_E,      lion2FlexLinkIfModeCfg }
    ,{ CPSS_PORT_INTERFACE_MODE_KR4_E,          lion2FlexLinkIfModeCfg }
    ,{ CPSS_PORT_INTERFACE_MODE_SR_LR4_E,       lion2FlexLinkIfModeCfg }
    ,{ CPSS_PORT_INTERFACE_MODE_KR2_E,           lion2FlexLinkIfModeCfg }
    ,{ CPSS_PORT_INTERFACE_MODE_SR_LR2_E,        lion2FlexLinkIfModeCfg }
    ,{ CPSS_PORT_INTERFACE_MODE_NA_E,           NULL                   }
};


prvCpssIfModeSetFun_STC prvCpssIfModeFun_Bobcat2_List[] =
{
    { CPSS_PORT_INTERFACE_MODE_NA_E,           NULL                   }
};


prvCpssIfModeSetFun_STC prvCpssIfModeFun_xcat3_List[] =
{
    { CPSS_PORT_INTERFACE_MODE_NA_E,           NULL                   }
};


prvCpssDev_IfMode_Fun_STC prvCpssIfModeSetFunList[] =
{
     {  CPSS_PP_FAMILY_DXCH_LION2_E,    &prvCpssIfModeFun_Lion2_List[0]   , lionIfModeGet             }
    ,{  CPSS_PP_FAMILY_DXCH_BOBCAT2_E,  &prvCpssIfModeFun_Bobcat2_List[0] , lionIfModeGet             }
    ,{  CPSS_PP_FAMILY_DXCH_XCAT3_E,    &prvCpssIfModeFun_xcat3_List[0]   , lionIfModeGet             }
    ,{  CPSS_MAX_FAMILY,                NULL                              , NULL                      }
};


prvCpssDev_IfMode_Fun_STC * prvCpssDxChFillPortIfFunctionsFind
(
    CPSS_PP_FAMILY_TYPE_ENT devFamily
)
{
    GT_U32 devIdx;

    if(CPSS_DXCH_ALL_SIP5_FAMILY_MAC(devFamily))
    {
        /* All sip5/6 treated the same here */
        devFamily = CPSS_PP_FAMILY_DXCH_BOBCAT2_E;
    }
    if(devFamily == CPSS_PP_FAMILY_DXCH_AC5_E)
    {
        devFamily = CPSS_PP_FAMILY_DXCH_XCAT3_E;
    }

    for (devIdx = 0; prvCpssIfModeSetFunList[devIdx].familyType != CPSS_MAX_FAMILY; devIdx++)
    {
        if (prvCpssIfModeSetFunList[devIdx].familyType == devFamily)
        {
            return &prvCpssIfModeSetFunList[devIdx];
        }
    }
    return (prvCpssDev_IfMode_Fun_STC*)NULL;
}


PRV_CPSS_DXCH_PORT_INTERFACE_MODE_SET_FUN prvCpssDxChFillPortIfFunctionsSetFind
(
    IN CPSS_PP_FAMILY_TYPE_ENT     devFamily,
    IN CPSS_PORT_INTERFACE_MODE_ENT ifMode
)
{
    prvCpssDev_IfMode_Fun_STC * ifFunsPtr;
    prvCpssIfModeSetFun_STC  *setFunlistPtr;
    GT_U32 ifIdx;

    ifFunsPtr = prvCpssDxChFillPortIfFunctionsFind(devFamily);
    if (ifFunsPtr == NULL)
    {
        return (PRV_CPSS_DXCH_PORT_INTERFACE_MODE_SET_FUN)NULL;
    }
    setFunlistPtr = ifFunsPtr->ifModeSetFuncListPtr;
    for (ifIdx = 0; setFunlistPtr->ifMode != CPSS_PORT_INTERFACE_MODE_NA_E; ifIdx++,setFunlistPtr++)
    {
        if (setFunlistPtr->ifMode == ifMode)
        {
            return setFunlistPtr->fun;
        }
    }

    return (PRV_CPSS_DXCH_PORT_INTERFACE_MODE_SET_FUN)NULL;
}

GT_STATUS prvCpssDxChPortIfFunctionsObjInit
(
    IN GT_U8 devNum,
    INOUT PRV_CPSS_DXCH_PORT_OBJ_STC *objPtr,
    IN    CPSS_PP_FAMILY_TYPE_ENT     devFamily
)
{
    GT_U32 ifIdx;
    prvCpssIfModeSetFun_STC   *setFunlistPtr;
    prvCpssDev_IfMode_Fun_STC *ifFunsPtr;

    /*-------------------------------------------------*/
    /* search for setPortInterfaceMode() function list */
    /*-------------------------------------------------*/

    ifFunsPtr = prvCpssDxChFillPortIfFunctionsFind(devFamily);
    if (ifFunsPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    /*-------------------------------------------------*/
    /*  fill setPortInterfaceMode() functions          */
    /*-------------------------------------------------*/
    for (ifIdx = 0; ifIdx < sizeof(objPtr->setPortInterfaceMode)/sizeof(objPtr->setPortInterfaceMode[0]);  ifIdx++)
    {
        objPtr->setPortInterfaceMode[ifIdx] = (PRV_CPSS_DXCH_PORT_INTERFACE_MODE_SET_FUN)NULL;
    }
    setFunlistPtr = ifFunsPtr->ifModeSetFuncListPtr;
    for (ifIdx = 0; setFunlistPtr->ifMode != CPSS_PORT_INTERFACE_MODE_NA_E; ifIdx++,setFunlistPtr++)
    {
        objPtr->setPortInterfaceMode[setFunlistPtr->ifMode] = setFunlistPtr->fun;
    }


    objPtr->getPortInterfaceMode = ifFunsPtr->ifModeGetFunc;

    /* common functions bind */
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortModeSpeedSetFunc = prvCpssDxChPortModeSpeedSetWrapper;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortHwUnresetFunc = prvCpssDxChPortHwUnresetWrapper;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortHwResetFunc = prvCpssDxChPortHwResetWrapper;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppFecModeSetFunc = prvCpssDxChPortFecModeSetWrapper;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppFecModeGetFunc = prvCpssDxChPortFecModeGetWrapper;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppIfModeGetFromExtFunc = prvCpssDxChPortInterfaceModeGetWrapper;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortBwTxFifoSetFunc = cpssDxChPortPizzaArbiterBWModeSet;

    return GT_OK;
}




PRV_CPSS_LION2_INT_CAUSE_ENT lion2PortUnitHglSumIndex
(
    IN GT_PHYSICAL_PORT_NUM localPort
)
{
    switch(localPort)
    {
        case 0:
            return PRV_CPSS_LION2_PORT0_UNITS_SUM_HGL_SUMMARY_E;
        case 1:
            return PRV_CPSS_LION2_PORT1_UNITS_SUM_HGL_SUMMARY_E;
        case 2:
            return PRV_CPSS_LION2_PORT2_UNITS_SUM_HGL_SUMMARY_E;
        case 3:
            return PRV_CPSS_LION2_PORT3_UNITS_SUM_HGL_SUMMARY_E;
        case 4:
            return PRV_CPSS_LION2_PORT4_UNITS_SUM_HGL_SUMMARY_E;
        case 5:
            return PRV_CPSS_LION2_PORT5_UNITS_SUM_HGL_SUMMARY_E;
        case 6:
            return PRV_CPSS_LION2_PORT6_UNITS_SUM_HGL_SUMMARY_E;
        case 7:
            return PRV_CPSS_LION2_PORT7_UNITS_SUM_HGL_SUMMARY_E;
        case 8:
            return PRV_CPSS_LION2_PORT8_UNITS_SUM_HGL_SUMMARY_E;
        case 9:
            return PRV_CPSS_LION2_PORT9_UNITS_SUM_HGL_SUMMARY_E;
        case 10:
            return PRV_CPSS_LION2_PORT10_UNITS_SUM_HGL_SUMMARY_E;
        case 11:
            return PRV_CPSS_LION2_PORT11_UNITS_SUM_HGL_SUMMARY_E;
        case 12:
            return PRV_CPSS_LION2_PORT12_UNITS_SUM_HGL_SUMMARY_E;
        case 14:
            return PRV_CPSS_LION2_PORT14_UNITS_SUM_HGL_SUMMARY_E;
        default:
            return PRV_CPSS_LION2_LAST_INT_E;
    }
}

/**
* @internal prvCpssDxChAc3xRxauiSerdesTxParametersSet function
* @endinternal
*
* @brief   Set serdes TX parameters for a RXAUI port of AC3X device.
*
* @note   APPLICABLE DEVICES:      AC3X.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - physical port number
* @param[in] portMacMap               - mac port number related to the physical port number
* @param[in] hwsPortMode              - hws port mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failed
* @retval GT_NOT_SUPPORTED         - not supported
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChAc3xRxauiSerdesTxParametersSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 portMacMap,
    IN MV_HWS_PORT_STANDARD hwsPortMode
)
{
    GT_U32 serdesLane;
    GT_STATUS rc;
    CPSS_PORT_SERDES_TX_CONFIG_STC serdesTxCfg;
    MV_HWS_PORT_INIT_PARAMS   curPortParams;  /* current port parameters */
    GT_U32 portIndex;
    GT_U32 curPortNum =0xff;
    CPSS_DXCH_PORT_SERDES_AC3X1690TX_PARAM_CONFIG_STC   *serdesRxauiTxConfigPtr = PRV_SHARED_PORT_DIR_PORT_SERDES_CFG_SRC_GLOBAL_VAR_GET(serdesRxauiTxConfig);


    /* sanity checks */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if ( CPSS_PP_FAMILY_DXCH_AC3X_E != PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    /* getting hws port parameters */
    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portMacMap, hwsPortMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /*check if port exists*/
    for (portIndex=0; portIndex<PRV_SHARED_PORT_DIR_PORT_SERDES_CFG_SRC_GLOBAL_VAR_GET(numOfSerdesRxauiConfig); portIndex++)
    {
        if (serdesRxauiTxConfigPtr[portIndex].portMac == portMacMap)
        {
            curPortNum=portIndex;
            break;
        }
    }

    if (curPortNum == 0xff)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "port mac not matching AC3X RXAUI port");
    }

    for (serdesLane=0; serdesLane < curPortParams.numOfActLanes; serdesLane++)
    {
        CPSS_LOG_INFORMATION_MAC("calling AC3X serdes TX parameters get API");
        /* getting current serdes TX parameters */
        rc = cpssDxChPortSerdesManualTxConfigGet(devNum, portNum, serdesLane, &serdesTxCfg);

        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        /* assigning the proper TX values */
        serdesTxCfg.type= CPSS_PORT_SERDES_AVAGO_E;
        serdesTxCfg.txTune.avago.post = serdesRxauiTxConfigPtr[curPortNum].post;
        serdesTxCfg.txTune.avago.pre = serdesRxauiTxConfigPtr[curPortNum].pre;
        serdesTxCfg.txTune.avago.atten = serdesRxauiTxConfigPtr[curPortNum].atten;

        CPSS_LOG_INFORMATION_MAC("setting AC3X TX params- post %d, pre %d, atten %d, lane %d",
                                  serdesRxauiTxConfigPtr[curPortNum].post,serdesRxauiTxConfigPtr[curPortNum].pre,
                                  serdesRxauiTxConfigPtr[curPortNum].atten, serdesLane);
        /* setting the values */
        rc = cpssDxChPortSerdesManualTxConfigSet(devNum, portNum, serdesLane, &serdesTxCfg);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortSerdesLoopbackActivate function
* @endinternal
*
* @brief   Set internal loopback enable and
*         Set power down/up of Tx and Rx on Serdeses.
*         Not Valid for port speed 10G and 20G.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - physical port number
* @param[in] speed                    - port data speed
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failed
* @retval GT_NOT_SUPPORTED         - not supported
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortSerdesLoopbackActivate
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_SPEED_ENT      speed
)
{
    GT_STATUS                    rc;

    rc = prvCpssDxChPortLion2InternalLoopbackEnableSet(devNum, portNum, GT_TRUE);
    if (rc != GT_OK)
    {
         return rc;
    }
    if(CPSS_PORT_SPEED_1000_E == speed)
    {
        rc = prvCpssDxChPortSerdesPartialPowerDownSet(devNum, portNum, GT_TRUE, GT_TRUE);
        if (rc != GT_OK)
        {
             return rc;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortSerdesPartialPowerDownSet function
* @endinternal
*
* @brief   Set power down/up of Tx and Rx on Serdeses.
*         Not Valid for port speed 10G and 20G
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - system device number
* @param[in] portNum                  - physical port number
* @param[in] powerDownRx              - Status of Serdes Rx (TRUE - power down, FALSE - power up).
* @param[in] powerDownTx              - Status of Serdes Tx (TRUE - power down, FALSE - power up).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failed.
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssDxChPortSerdesPartialPowerDownSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  powerDownRx,
    IN GT_BOOL                  powerDownTx
)
{
    GT_STATUS                    rc;
    GT_UOPT                      portGroup;
    MV_HWS_PORT_STANDARD         portMode;/* interface modes of port which use this configuration in Black Box format */
    CPSS_PORT_SPEED_ENT          speed; /* current speed of port */
    GT_UOPT                      phyPortNum; /* number of port in local core */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode; /* port interface mode */
    GT_U32                       firstInQuadruplet;  /* mac number of first port in quadruplet */
    GT_U32                       maxGEonly = 24;     /* maximal GE only ports for device */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    speed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portNum);
    phyPortNum = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);
    ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portNum);
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if (rc != GT_OK)
    {
        return rc;
    }
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        maxGEonly = 48;
    }
    if ((PRV_CPSS_GE_PORT_GE_ONLY_E == PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum, phyPortNum))
        && (phyPortNum < maxGEonly))
    {/* provide to HWS first port in quadruplet and it will configure
                all other ports if needed */
         firstInQuadruplet = phyPortNum & 0xFFFFFFFC;
    }
    else
    {
        firstInQuadruplet = phyPortNum;
    }
    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortPartialPowerDown(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], powerDownRx[%d], powerDownTx[%d])", devNum, portGroup, firstInQuadruplet, portMode, powerDownRx, powerDownTx);
    rc = mvHwsPortPartialPowerDown(devNum, portGroup, firstInQuadruplet, portMode, powerDownRx, powerDownTx);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }

    return rc;
}

/**
* @internal prvCpssDxChPortModeToPortTypeConvert function
* @endinternal
*
* @brief   Convert interface mode + speed to MAC type.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] ifMode                   - Interface mode
* @param[in] speed                    - port speed
*
* @param[out] portTypePtr             - (pointer to) MAC type
*/
GT_STATUS prvCpssDxChPortModeToPortTypeConvert
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    OUT PRV_CPSS_PORT_TYPE_ENT          *portTypePtr
)
{
    GT_STATUS rc;
    GT_U32 portMacNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_XCAT3_E | CPSS_AC5_E| CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E);

    *portTypePtr = PRV_CPSS_PORT_NOT_EXISTS_E;

    rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(devNum, portNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &portMacNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    switch (ifMode)
    {
        case CPSS_PORT_INTERFACE_MODE_QSGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_2_5G_SXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_5G_SXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_10G_SXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_5G_DXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_10G_DXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_20G_DXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_QUSGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_10G_QXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_20G_QXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E:
            if (HWS_DEV_SILICON_TYPE(devNum) == Harrier)
            {
                *portTypePtr = PRV_CPSS_PORT_MTI_100_E;
            }
            else
            {
                *portTypePtr = PRV_CPSS_PORT_MTI_USX_E;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
        case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
        case CPSS_PORT_INTERFACE_MODE_2500BASE_X_E:
        case CPSS_PORT_INTERFACE_MODE_KR_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
        case CPSS_PORT_INTERFACE_MODE_KR_C_E:
        case CPSS_PORT_INTERFACE_MODE_KR2_C_E:
        case CPSS_PORT_INTERFACE_MODE_CR_C_E:
        case CPSS_PORT_INTERFACE_MODE_CR2_C_E:
        case CPSS_PORT_INTERFACE_MODE_KR_S_E:
        case CPSS_PORT_INTERFACE_MODE_CR_S_E:
        case CPSS_PORT_INTERFACE_MODE_KR2_E:
        case CPSS_PORT_INTERFACE_MODE_CR_E:
        case CPSS_PORT_INTERFACE_MODE_CR2_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR2_E:
            if(speed == CPSS_PORT_SPEED_10000_E)
            {
                if(mvHwsMtipIsReducedPort(devNum, portMacNum) == GT_TRUE)
                {
                    *portTypePtr = PRV_CPSS_PORT_MTI_CPU_E;
                    break;
                }
            }
            *portTypePtr = PRV_CPSS_PORT_MTI_100_E;
            break;

        case CPSS_PORT_INTERFACE_MODE_KR4_E:
        case CPSS_PORT_INTERFACE_MODE_CR4_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR4_E:
            if ((speed == CPSS_PORT_SPEED_200G_E) || (speed == CPSS_PORT_SPEED_212G_E))
            {
                *portTypePtr = PRV_CPSS_PORT_MTI_400_E;
            }
            else
            {
                *portTypePtr = PRV_CPSS_PORT_MTI_100_E;
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_KR8_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR8_E:
        case CPSS_PORT_INTERFACE_MODE_CR8_E:
            *portTypePtr = PRV_CPSS_PORT_MTI_400_E;
            break;

        case CPSS_PORT_INTERFACE_MODE_NA_E:
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

extern GT_U32 isSupportUsxPort(
    IN  GT_U8   devNum,
    IN  GT_U32  macNum
);
extern GT_U32 isForceUsxPort(
    IN  GT_U8   devNum,
    IN  GT_U32  macNum
);


/**
* @internal prvCpssDxChPortTypeSet function
* @endinternal
*
* @brief   Sets port type (mostly means which mac unit used) on a specified port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] ifMode                   - Interface mode
* @param[in] speed                    - port speed
*                                       None.
*/
GT_VOID prvCpssDxChPortTypeSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_U32 macNum;

    if(PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        /* port number already checked in caller function */
        (GT_VOID)prvCpssDxChPortPhysicalPortMapCheckAndConvert(devNum, portNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &macNum);
    }
    else
    {
        macNum = portNum;
    }

    if((CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        cpssOsPrintf("prvCpssDxChPortTypeSet: Not applicable device");
        return;
    }

    switch (ifMode)
    {
        case CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_1000_E:
                    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                    {
                        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_MTI_100_E;
                    }
                    else
                    {
                        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_GE_E;
                    }
                    break;
                case CPSS_PORT_SPEED_10000_E:
                case CPSS_PORT_SPEED_20000_E:
                    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_XG_E;
                    break;
                default:
                    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_XG_E;
                    break;
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
        case CPSS_PORT_INTERFACE_MODE_QSGMII_E:
        case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
        case CPSS_PORT_INTERFACE_MODE_2500BASE_X_E:
        case CPSS_PORT_INTERFACE_MODE_100BASE_FX_E:
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_MTI_100_E;
                if(mvHwsMtipIsReducedPort(devNum, macNum) == GT_TRUE)
                {
                    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_MTI_CPU_E;
                    break;
                }

                if(isSupportUsxPort(devNum,macNum) && (ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E))
                {
                    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_MTI_USX_E;
                }
                else if(isForceUsxPort(devNum,macNum))
                {
                    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_MTI_USX_E;
                }

            }
            else
            {
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_GE_E;
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_QX_E:
        case CPSS_PORT_INTERFACE_MODE_HX_E:
        case CPSS_PORT_INTERFACE_MODE_XGMII_E:
        case CPSS_PORT_INTERFACE_MODE_RXAUI_E:
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_XG_E;
            break;
        case CPSS_PORT_INTERFACE_MODE_SR_LR4_E:
        case CPSS_PORT_INTERFACE_MODE_KR4_E:
        case CPSS_PORT_INTERFACE_MODE_CR4_E:
            if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                if ((speed == CPSS_PORT_SPEED_200G_E) || (speed == CPSS_PORT_SPEED_212G_E))
                {
                    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_MTI_400_E;
                }
                else
                {
                    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_MTI_100_E;
                }
            }
            else
            {
                if ((speed == CPSS_PORT_SPEED_100G_E) || (speed == CPSS_PORT_SPEED_107G_E) || (speed == CPSS_PORT_SPEED_102G_E))
                {
                    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_CG_E;
                }
                else if(speed == CPSS_PORT_SPEED_29090_E || speed == CPSS_PORT_SPEED_40000_E || speed == CPSS_PORT_SPEED_50000_E)
                {
                    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_XLG_E;
                }
            }
            break;

        case CPSS_PORT_INTERFACE_MODE_KR_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
        case CPSS_PORT_INTERFACE_MODE_CR_E:
        case CPSS_PORT_INTERFACE_MODE_KR_C_E:
        case CPSS_PORT_INTERFACE_MODE_CR_C_E:
        case CPSS_PORT_INTERFACE_MODE_KR_S_E:
        case CPSS_PORT_INTERFACE_MODE_CR_S_E:
            if(PRV_CPSS_PP_MAC(devNum)->isGmDevice && !PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
            {
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_XLG_E;
                break;
            }

            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                if((speed == CPSS_PORT_SPEED_10000_E) || (speed == CPSS_PORT_SPEED_25000_E) || (speed == CPSS_PORT_SPEED_5000_E))
                {
                    if(mvHwsMtipIsReducedPort(devNum, macNum) == GT_TRUE)
                    {
                        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_MTI_CPU_E;
                        break;
                    }
                }
                if(isForceUsxPort(devNum,macNum))
                {
                    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_MTI_USX_E;
                }
                else
                {
                    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_MTI_100_E;
                }
                break;
            }
            GT_ATTR_FALLTHROUGH;
        case CPSS_PORT_INTERFACE_MODE_KR2_E:
        case CPSS_PORT_INTERFACE_MODE_XLG_E:
        case CPSS_PORT_INTERFACE_MODE_CR2_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR2_E:
        case CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E:
        case CPSS_PORT_INTERFACE_MODE_HGL_E: /* pay attention this definition for
        HGL relevant for Bobcat2 XLHGL_KR4 mode, in other cases could be required
        PRV_CPSS_PORT_HGL_E */
        case CPSS_PORT_INTERFACE_MODE_XHGS_E: /* If with Serdes   of 12.1875Ghz */
        case CPSS_PORT_INTERFACE_MODE_XHGS_SR_E:
        case CPSS_PORT_INTERFACE_MODE_KR2_C_E:
        case CPSS_PORT_INTERFACE_MODE_CR2_C_E:
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_MTI_100_E;
            }
            else
            {
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_XLG_E;
            }
            break;


        case CPSS_PORT_INTERFACE_MODE_ILKN4_E:
        case CPSS_PORT_INTERFACE_MODE_ILKN8_E:
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_ILKN_E;
            break;
        case CPSS_PORT_INTERFACE_MODE_NA_E:
            /* leave CPSS init default value, see hwPpPhase1Part1 */
            break;
        case CPSS_PORT_INTERFACE_MODE_CR8_E:
        case CPSS_PORT_INTERFACE_MODE_KR8_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR8_E:
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_MTI_400_E;
            break;

        case CPSS_PORT_INTERFACE_MODE_USX_2_5G_SXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_5G_SXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_10G_SXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_5G_DXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_10G_DXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_20G_DXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_QUSGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_10G_QXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_20G_QXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E:
            if (HWS_DEV_SILICON_TYPE(devNum) == Harrier)
            {
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_MTI_100_E;
            }
            else
            {
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_MTI_USX_E;
            }
            break;

        default:
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_FE_E;
            break;
    }

    return;
}

/**
* @internal internal_cpssDxChPortInterfaceModeSet function
* @endinternal
*
* @brief   Sets Interface mode on a specified port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] ifMode                   - Interface mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Depends on capability of the specific device. The port's speed
*       should be set after configuring Interface Mode.
*
*/
static GT_STATUS internal_cpssDxChPortInterfaceModeSet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode
)
{
    GT_STATUS rc;   /* return code */
    GT_U32 portMacNum;      /* MAC number */
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    PRV_CPSS_DXCH_PORT_INTERFACE_MODE_SET_FUN ifModeSetFuncPtr; /* pointer to [devFamily][ifMode] specific function */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,portNum);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum) == PRV_CPSS_PORT_NOT_EXISTS_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if((GT_U32)ifMode >= CPSS_PORT_INTERFACE_MODE_NA_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    ifModeSetFuncPtr =  (PORT_OBJ_FUNC(devNum)).setPortInterfaceMode[ifMode];
    if(ifModeSetFuncPtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    if(portNum != CPSS_CPU_PORT_NUM_CNS)
    {
        GT_BOOL resNew;
        /* check if interface mode supported by given port */

        /*
        //resOld = supportedPortsModes[PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum,portNum)][ifMode];
        */

        rc = prvCpssDxChPortSupportedModeCheck(devNum,portNum,ifMode,/*OUT*/&resNew);
        if (rc != GT_OK)
        {
            return GT_OK;
        }

        if (resNew == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    if((tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
        && (PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.hitlessWriteMethodEnable))
    {/* do only if we during recovery */
        CPSS_PORT_INTERFACE_MODE_ENT   currentIfMode;

        rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &currentIfMode);
        if(rc != GT_OK)
        {
            return rc;
        }
        if(ifMode == currentIfMode)
        {
            return GT_OK;
        }
    }

    /* configure requested interface mode on port */
    if((rc = ifModeSetFuncPtr(devNum, portNum, ifMode)) != GT_OK)
        return rc;

    /* cpu port has not entry in phyPortInfoArray - skip SW port info DB update */
    if(portNum == CPSS_CPU_PORT_NUM_CNS)
        return GT_OK;

    /* update current port type - used to define which mac unit currently in use by port
    */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->genInfo.devFamily < CPSS_PP_FAMILY_DXCH_XCAT_E)
    {
        prvCpssDxChPortTypeSet(devNum,portNum,ifMode,CPSS_PORT_SPEED_NA_E);
    }


    /* save new interface mode in DB */
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacNum) = ifMode;

    return GT_OK;
}

/**
* @internal cpssDxChPortInterfaceModeSet function
* @endinternal
*
* @brief   Sets Interface mode on a specified port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] ifMode                   - Interface mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Depends on capability of the specific device. The port's speed
*       should be set after configuring Interface Mode.
*
*/
GT_STATUS cpssDxChPortInterfaceModeSet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortInterfaceModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ifMode));

    rc = internal_cpssDxChPortInterfaceModeSet(devNum, portNum, ifMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ifMode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortGePortTypeSet function
* @endinternal
*
* @brief   Set port type and inband auto-neg. mode of GE MAC of port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] ifMode                   - port interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, ifMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortGePortTypeSet
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT ifMode
)
{
    GT_STATUS   rc;     /* return code */
    GT_U32      value;  /* value to set in register */
    GT_U32      regAddr; /* register address */
    GT_BOOL     linkDownStatus; /* current port force link down state */
    GT_U32      portGroupId;    /* port group ID - for multi-port-group devices */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

    /* Set interface mode */
    switch(ifMode)
    {
        CPSS_COVERITY_NON_ISSUE_BOOKMARK
        /* coverity[unterminated_case] */
        case CPSS_PORT_INTERFACE_MODE_QSGMII_E: /*No break*/
        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
            value = 0;
            break;
        case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
            value = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortForceLinkDownEnable(devNum,portNum,&linkDownStatus);
    if(rc != GT_OK)
    {
        return rc;
    }

    PRV_CPSS_DXCH_PORT_MAC_CTRL_REG_MAC(devNum, portNum, PRV_CPSS_PORT_GE_E,
                                        &regAddr);
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(
                                                                devNum, portNum);
    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 1, 1,
                                            value);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChPortForceLinkDownDisable(devNum,portNum,linkDownStatus);

    return rc;
}

/**
* @internal lion2FlexLinkIfModeCfg function
* @endinternal
*
* @brief   Sets Interface mode on a specified port of devices from DXCH
*         Lion2 family.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - Interface mode.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - on not supported interface or port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS lion2FlexLinkIfModeCfg
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode
)
{
    GT_STATUS            rc;                /* return code */
    MV_HWS_PORT_STANDARD portMode;          /* port i/f mode and speed translated to BlackBox enum */
    GT_U32               macNum;            /* number of MAC used by port for given ifMode */
    GT_PHYSICAL_PORT_NUM localPort;         /* number of port in port group */
    GT_BOOL              notEnoughInfo = GT_FALSE; /* not enough info to define validity */
    GT_U32               portGroup;         /* number of local core */
    GT_U32               sliceNumUsed;      /* number of pizza slices used by neighbour
                                            port */
    CPSS_PORT_INTERFACE_MODE_ENT    ifModeOld;
    CPSS_PORT_SPEED_ENT             speedOld;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    if(portNum == CPSS_CPU_PORT_NUM_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);
    if(localPort >= 12)
    {/* to prevent coverity warning OVERRUN_STATIC */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    switch(ifMode)
    {
        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
            /* no need to check SGMII2_5 it's same like SGMII */
            portMode = SGMII;
            break;
        case CPSS_PORT_INTERFACE_MODE_XGMII_E:
            /* other speeds similar to 10G */
            portMode = _10GBase_KX4;
            break;
        case CPSS_PORT_INTERFACE_MODE_KR4_E:
            /* check here just basic KR support, validity for KR20/40/100
                will be tested in speedSet */
            portMode = _40GBase_KR4;
            if((9 == localPort) || (11 == localPort))
            {/* local ports 9 and 11 can use regular and extended MAC
                till we not sure which one we can't check validity */
                notEnoughInfo = GT_TRUE;
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_KR_E:
            /* check here just basic KR support, validity for KR20/40/100
                will be tested in speedSet */
            portMode = _10GBase_KR;
            if((9 == localPort) || (11 == localPort))
            {/* local ports 9 and 11 can use regular and extended MAC
                till we not sure which one we can't check validity */
                notEnoughInfo = GT_TRUE;
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_KR2_E:
            /* check here just basic KR support, validity for KR20/40/100
                will be tested in speedSet */
            portMode = _20GBase_KR2;
            if((9 == localPort) || (11 == localPort))
            {/* local ports 9 and 11 can use regular and extended MAC
                till we not sure which one we can't check validity */
                notEnoughInfo = GT_TRUE;
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_HGL_E:
            if((9 == localPort) || (11 == localPort))
            {/* if core clock of device less than 480 it doesn't
                support more than 10G speed on ports 9 and 11 */
                if(PRV_CPSS_PP_MAC(devNum)->coreClock < 480)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            portMode = HGL; /* could be XLHGL, but here doesn't matter */
            break;
        case CPSS_PORT_INTERFACE_MODE_SR_LR4_E:
            /* check here just basic SR support, validity for SR20/40
                will be tested in speedSet */
            portMode = _40GBase_SR_LR4;
            if((9 == localPort) || (11 == localPort))
            {/* local ports 9 and 11 can use regular and extended MAC
                till we not sure which one we can't check validity */
                notEnoughInfo = GT_TRUE;
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_SR_LR2_E:
            /* check here just basic SR support, validity for SR20/40
                will be tested in speedSet */
            portMode = _20GBase_SR_LR2;
            if((9 == localPort) || (11 == localPort))
            {/* local ports 9 and 11 can use regular and extended MAC
                till we not sure which one we can't check validity */
                notEnoughInfo = GT_TRUE;
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
            /* check here just basic SR support, validity for SR20/40
                will be tested in speedSet */
            portMode = _10GBase_SR_LR;
            if((9 == localPort) || (11 == localPort))
            {/* local ports 9 and 11 can use regular and extended MAC
                till we not sure which one we can't check validity */
                notEnoughInfo = GT_TRUE;
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_XHGS_E:
        case CPSS_PORT_INTERFACE_MODE_XHGS_SR_E:
            if (PRV_CPSS_PP_MAC(devNum)->revision == 0) /* LION2 version 0 and clock less than 360 MHz are not supported */
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            if(PRV_CPSS_PP_MAC(devNum)->coreClock < 480)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }

            portMode = (CPSS_PORT_INTERFACE_MODE_XHGS_E == ifMode) ?
                                                       _12GBaseR : _12GBase_SR;
            if((9 == localPort) || (11 == localPort))
            {/* local ports 9 and 11 can use regular and extended MAC
                till we not sure which one we can't check validity */
                notEnoughInfo = GT_TRUE;
            }

            break;
        default:
            rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode,
                                /* in these cases just one option for speed */
                                CPSS_PORT_SPEED_NA_E,
                                &portMode);
            if(rc != GT_OK)
            {
                return rc;
            }
            break;
    }

    portGroup = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, localPort, portMode, &curPortParams);
    if (GT_OK != rc)
    {
        return rc;
    }
    macNum = curPortParams.portMacNumber;

    if(notEnoughInfo && (macNum > 11))
    {/* if configured local port 9 or 11 and it uses extended MAC
        for 1 serdes mode too, then we have enough info to check validity */
        notEnoughInfo = GT_FALSE;
    }

    switch(PRV_CPSS_PP_MAC(devNum)->devType)
    {
        case CPSS_LION_2_THREE_MINI_GOPS_DEVICES_CASES_MAC:
            if(macNum > 11)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

            break;

        default:
            break;
    }

    if((macNum%2 != 0) && (macNum < 12) && (!notEnoughInfo))
    {
        if(CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E == ifMode)
        {
            rc = cpssDxChPortPizzaArbiterIfPortStateGet(devNum, portGroup,
                                                        localPort-1, &sliceNumUsed);
            if(rc != GT_OK)
            {
                return rc;
            }
            if(sliceNumUsed > 1)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
        else if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portNum-1) >= PRV_CPSS_PORT_XLG_E)
        {/* i.e. odd port's serdeses occupied by neighbor even port */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    if((macNum%4 != 0) && (!notEnoughInfo))
    {
        GT_U32 localPortTmp;
        GT_U8  sdVecSizeTmp;
        GT_PHYSICAL_PORT_NUM firstInQuadruplet;

        firstInQuadruplet = (macNum != 14) ? (portNum&0xFFFFFFFC) :
                                            (portNum-2) /* i.e. local port 9 */;
        /* if first port of quadruplet not configured check not needed */
        if((PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,firstInQuadruplet) !=
                                            CPSS_PORT_INTERFACE_MODE_NA_E)
           && (PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,firstInQuadruplet) !=
                                                        CPSS_PORT_SPEED_NA_E))
        {
            rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                        PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,firstInQuadruplet),
                        PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,firstInQuadruplet),
                        &portMode);
            if(rc != GT_OK)
                return rc;
            localPortTmp = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,
                                                            firstInQuadruplet);
            if(localPortTmp >= 12)
            {/* to prevent coverity warning OVERRUN_STATIC */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, localPortTmp, portMode, &curPortParams);
            if (GT_OK != rc)
            {
                return rc;
            }
            sdVecSizeTmp    = curPortParams.numOfActLanes;
            if(CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E == ifMode)
            {
                rc = cpssDxChPortPizzaArbiterIfPortStateGet(devNum, portGroup,
                                                            localPortTmp, &sliceNumUsed);
                if(rc != GT_OK)
                {
                    return rc;
                }
                if(sliceNumUsed > 2)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }
            }
            else if(sdVecSizeTmp >= 4)
            {/* i.e. current port's serdeses occupied by first port in its quadruplet */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
    }

    speedOld = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portNum);
    ifModeOld = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portNum);
    if((!notEnoughInfo) && (ifModeOld != CPSS_PORT_INTERFACE_MODE_NA_E)
                                        && (speedOld != CPSS_PORT_SPEED_NA_E))
    {/* if we sure new mode is valid - reset port accordingly to old mode */
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifModeOld, speedOld, &portMode);
        if(rc != GT_OK)
        {
            return rc;
        }
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, portGroup, localPort, portMode, PORT_POWER_DOWN);
        rc = mvHwsPortReset(devNum, portGroup, localPort, portMode,
                            PORT_POWER_DOWN);
        if (rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }
    }

    if((GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,PRV_CPSS_DXCH_LION2_HGL_MAC_INT_WA_E))
       && (PRV_CPSS_DXCH_DEV_MODULE_CFG_MAC(devNum)->ppEventsCfg[portGroup].intVecNum !=
           CPSS_PP_INTERRUPT_VECTOR_NOT_USED_CNS))
    {/* mask the HGL MAC interrupts if the portInterface != HGL*/
        /* convert the 'Physical port' to portGroupId -- supporting multi-port-groups device */

        /* unmask - i.e. event enable.                */
        /* prvCpssDrvEventsMask may return not GT_OK  */
        /* if interrupt was not binded by application */
        /* or interrupts are not used.                */
        /* Need to ignore this fail.                  */
        (GT_VOID)prvCpssDrvEventsMask(devNum,
            lion2PortUnitHglSumIndex(localPort),
            (ifMode != CPSS_PORT_INTERFACE_MODE_HGL_E));
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortInterfaceModeGet function
* @endinternal
*
* @brief   Gets Interface mode on a specified port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] ifModePtr                - interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - wrong media interface mode value received
*/
static GT_STATUS internal_cpssDxChPortInterfaceModeGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
)
{
    GT_STATUS       rc = GT_OK;
    PRV_CPSS_DXCH_PORT_INTERFACE_MODE_GET_FUN ifModeGetFuncPtr; /* pointer to
            per device_family function returning current port interface mode */
    GT_U32  portMacNum;
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;  /* port object pointer */
    GT_BOOL             doPpMacConfig = GT_TRUE;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(ifModePtr);

    /* Get PHY MAC object ptr */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* check if MACPHY callback should run */
    if ((portMacObjPtr != NULL) &&
        (portMacObjPtr->macDrvMacPortInterfaceModeGetFunc))
    {

        rc = portMacObjPtr->macDrvMacPortInterfaceModeGetFunc(devNum,portNum,
                                  CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig,
                                  ifModePtr);
        if(rc!=GT_OK)
        {
            return rc;
        }

        if(doPpMacConfig == GT_FALSE)
        {
            return GT_OK;
        }
    }

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_NOT_EXISTS_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if((ifModeGetFuncPtr = PORT_OBJ_FUNC(devNum).getPortInterfaceMode) == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    return ifModeGetFuncPtr(devNum, portNum, ifModePtr);
}

/**
* @internal cpssDxChPortInterfaceModeGet function
* @endinternal
*
* @brief   Gets Interface mode on a specified port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] ifModePtr                - interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - wrong media interface mode value received
*/
GT_STATUS cpssDxChPortInterfaceModeGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortInterfaceModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ifModePtr));

    rc = internal_cpssDxChPortInterfaceModeGet(devNum, portNum, ifModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ifModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal lionIfModeGet function
* @endinternal
*
* @brief   Gets Interface mode on a specified port.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] ifModePtr                - interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS lionIfModeGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
)
{
    GT_STATUS   rc;
    GT_U32  portMacNum;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum,portMacNum);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        if(portNum == CPSS_CPU_PORT_NUM_CNS-1)
        {
CPSS_TBD_BOOKMARK_BOBCAT2
        }
    }

    if(portNum == CPSS_CPU_PORT_NUM_CNS)
        return cpuPortIfModeGet(devNum,portNum,ifModePtr);

    *ifModePtr = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacNum);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum,
                                                            &portMapShadowPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E == portMapShadowPtr->portMap.mappingType)
        {
            if((CPSS_PORT_INTERFACE_MODE_ILKN4_E != *ifModePtr) &&
                (CPSS_PORT_INTERFACE_MODE_ILKN8_E != *ifModePtr))
            {/* interlaken channel mapped to physical MAC that can serve not
                interlaken, but it's wrong to relate to channel anything
                else then ILKN mode */
                *ifModePtr = CPSS_PORT_INTERFACE_MODE_NA_E;
            }
        }
    }

    return GT_OK;
}

/**
* @internal cpuPortIfModeGet function
* @endinternal
*
* @brief   Gets Interface mode of CPU port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] ifModePtr                - interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS cpuPortIfModeGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
)
{
    GT_U32 value;   /* register value */
    GT_STATUS rc;   /* return code */

    if(portNum != CPSS_CPU_PORT_NUM_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, &portMapShadowPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E == portMapShadowPtr->portMap.mappingType)
        {/* return dummy ifMode to support legacy behavior */
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_MII_E;
            return GT_OK;
        }
        else
        {
            return prvCpssDxChPortLion2InterfaceModeHwGet(devNum,portNum,ifModePtr);
        }
    }

    if (prvCpssDrvHwPpPortGroupGetRegField(devNum,
            PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum),
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.cpuPortCtrlReg,
            1, 2, &value) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    switch(value)
    {
        case 0:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_MII_E;
            break;
        case 1:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_GMII_E;
            break;
        case 2:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_REDUCED_GMII_E;
            break;
        case 3:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_MII_PHY_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortForceLinkDownEnable function
* @endinternal
*
* @brief   Enable Force link down on a specified port on specified device and
*         read current force link down state of it.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
*
* @param[out] linkDownStatusPtr        - (ptr to) current force link down state of port:
*                                      GT_TRUE  - enable force link down on port,
*                                      GT_FALSE - disable force link down on port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortForceLinkDownEnable
(
    IN GT_U8    devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL *linkDownStatusPtr
)
{
    GT_STATUS rc; /* return code */

    /* Get force link down status */
    if((rc = cpssDxChPortForceLinkDownEnableGet(devNum, portNum, linkDownStatusPtr)) != GT_OK)
        return rc;

    if(*linkDownStatusPtr == GT_FALSE)
    {/* Set force link down */
        if((rc = cpssDxChPortForceLinkDownEnableSet(devNum,portNum,GT_TRUE)) != GT_OK)
            return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortForceLinkDownDisable function
* @endinternal
*
* @brief   Disable Force link down on a specified port on specified device if
*         it was previously disabled.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[in] linkDownStatus           - previous force link down state:
*                                      GT_TRUE  - enable force link down on port,
*                                      GT_FALSE - disable force link down on port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortForceLinkDownDisable
(
    IN GT_U8   devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL linkDownStatus
)
{
    GT_STATUS rc;   /* return code */

    if(linkDownStatus == GT_FALSE)
    {
        if((rc = cpssDxChPortForceLinkDownEnableSet(devNum,portNum,GT_FALSE)) != GT_OK)
            return rc;
    }

    return GT_OK;
}

#define PRV_CPSS_DXCH_PORT_BOBCAT2_MINIMAL_IPG_CNS  2
#define PRV_CPSS_DXCH_PORT_BOBCAT2_TXQ_CONSTANT_CNS 20
#define PRV_CPSS_DXCH_PORT_BOBCAT2_TXQ_DFB_CNS      16
#define PRV_CPSS_DXCH_PORT_BOBCAT2_TXQ_SPARE_CNS    100
#define PRV_CPSS_DXCH_PORT_BOBCAT2_DESC_FIFO_SIZE_CNS   206

#define PRV_CPSS_DXCH_PORT_BOBCAT2_MPPM_MIN_LATENCY_CNS         32
#define PRV_CPSS_DXCH_PORT_BOBCAT2_MAXIMAL_SINGLE_BANK_FIFO_FILL_LEVEL_CNS  70
#define PRV_CPSS_DXCH_PORT_BOBCAT2_NUMBER_OF_MPPM_CLIENTS_CNS   2
#define PRV_CPSS_DXCH_PORT_BOBCAT2_OTHERS_LATENCY_CNS           267

#define PRV_CPSS_DXCH_PORT_BOBCAT2_PAYLOAD_CREDITS_CNS  541
#define PRV_CPSS_DXCH_PORT_BOBCAT2_HEADER_CREDITS_CNS   541


/**
* @internal prvCpssDxChPortBcat2PtpInterfaceWidthSelect function
* @endinternal
*
* @brief   Set PTP interface width.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
*                                      portMacNum  - port MAC number (not CPU port)
* @param[in] speed                    - port data speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong speed
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChPortBcat2PtpInterfaceWidthSelect
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_U32      regAddr;            /* register address */
    GT_U32      interfaceWidth;     /* PTP interface width */
    GT_U32      portMacNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);


    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                 portNum,
                                                                 portMacNum);

    /* Interface width select */
    switch(speed)
    {
        case CPSS_PORT_SPEED_10_E:
        case CPSS_PORT_SPEED_100_E:
        case CPSS_PORT_SPEED_1000_E:
            interfaceWidth = 0;     /* width 8bit */
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                interfaceWidth = 1; /* width 64bit */
            }
            break;

        case CPSS_PORT_SPEED_2500_E:
        case CPSS_PORT_SPEED_5000_E:
        case CPSS_PORT_SPEED_10000_E:
        case CPSS_PORT_SPEED_11800_E:
        case CPSS_PORT_SPEED_12000_E:
        case CPSS_PORT_SPEED_12500_E:
        case CPSS_PORT_SPEED_13600_E:
        case CPSS_PORT_SPEED_16000_E:
        case CPSS_PORT_SPEED_15000_E:

            interfaceWidth = 1;
            if(PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum) &&
              (portMacNum == 32) && (speed == CPSS_PORT_SPEED_2500_E))
            {
                interfaceWidth = 0; /* width 8bit */
            }

            break;
        case CPSS_PORT_SPEED_29090_E:
        case CPSS_PORT_SPEED_40000_E:
        case CPSS_PORT_SPEED_47200_E:
        case CPSS_PORT_SPEED_50000_E:
        case CPSS_PORT_SPEED_52500_E:
            interfaceWidth = 2;
            break;
        case CPSS_PORT_SPEED_75000_E:
        case CPSS_PORT_SPEED_100G_E:
        case CPSS_PORT_SPEED_107G_E:
        case CPSS_PORT_SPEED_140G_E:
        case CPSS_PORT_SPEED_102G_E:
            interfaceWidth = 3;
            break;

        case CPSS_PORT_SPEED_20000_E:
        case CPSS_PORT_SPEED_23600_E:
        case CPSS_PORT_SPEED_25000_E:
        case CPSS_PORT_SPEED_26700_E:
            /* for Caelum devices 20G port should be treated as 256bit width */
            if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum) ||
               PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum) ||
               PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
            {
                interfaceWidth = 2;
            }
            else
            {
                interfaceWidth = 1;
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).PTPGeneralCtrl;

    /* Set interface width select bits[3:2] */
    return prvCpssDrvHwPpSetRegField(devNum, regAddr, 2, 2, interfaceWidth);
}

/**
* @internal prvCpssDxChPortBcat2PtpInterfaceWidthGet function
* @endinternal
*
* @brief   Get PTP interface width in bits
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not CPU port)
*
* @param[out] busWidthBitsPtr          - pointer to bus with in bits.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong speed
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChPortBcat2PtpInterfaceWidthGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                         *busWidthBitsPtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;            /* register address */
    GT_U32      interfaceWidth;     /* PTP interface width */
    GT_U32      portMacNum;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    static GT_U32 busWidthArr[] =
    {
        /* 0x0 = 8BIT;  */   8
        /* 0x1 = 64BIT; */ ,64
        /* 0x2 = 256BIT */,256
        /* 0x3 = 512BIT */,512
    };

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(busWidthBitsPtr);

    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum,portNum,/*OUT*/&portMapShadowPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (portMapShadowPtr->valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                 portNum,
                                                                 portMacNum);


    regAddr = PRV_DXCH_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).PTPGeneralCtrl;

    /* Set interface width select bits[3:2] */
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, 2, 2, /*OUT*/&interfaceWidth);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    *busWidthArr = (GT_U32)(~0);
    if (interfaceWidth >=  sizeof(busWidthArr)/sizeof(busWidthArr[0]))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    *busWidthBitsPtr = busWidthArr[interfaceWidth];
    return GT_OK;
}


/**
* @internal prvCpssDxChPortBcat2InterlakenDelete function
* @endinternal
*
* @brief   Reset ILKN physical layer.
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChPortBcat2InterlakenDelete
(
    IN  GT_U8                           devNum
)
{
    GT_STATUS               rc; /* return code */
    GT_U32                  i;  /* iterator */
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr; /* ptr to ports entry in
                                                            mapping shadow DB */

    /*-------------------------------------------------*/
    /* disable all channels and config them to 0 speed */
    /*-------------------------------------------------*/
    for(i = 0; i < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; i++)
    {
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, i,
                                                       &portMapShadowPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(portMapShadowPtr->valid)
        {
            if(CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E ==
                            portMapShadowPtr->portMap.mappingType)
            {
                rc = cpssDxChPortIlknChannelEnableSet(devNum, i,
                                                      CPSS_PORT_DIRECTION_BOTH_E,
                                                      GT_FALSE);
                if(rc != GT_OK)
                {
                    return rc;
                }

                rc = cpssDxChPortIlknChannelSpeedSet(devNum, i,
                                                     CPSS_PORT_DIRECTION_BOTH_E,
                                                     0, NULL);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortBcat2PortReset function
* @endinternal
*
* @brief   Power down port.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] ifMode                   - interface mode according to which power down port
* @param[in] speed                    - port data  according to which power down port
* @param[in] qsgmiiFullDelete         - GT_TRUE - power down QSGMII interface
*                                      GT_FALSE - just free resources of this specific
*                                      member of QSGMII quadruplet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortBcat2PortReset
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         qsgmiiFullDelete
)
{
    GT_STATUS               rc;         /* return code */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */
    MV_HWS_PORT_STANDARD    portMode;   /* port interface/speed in terms of HWS */

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                 portNum,
                                                                 portMacMap);

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_PP_MAC(devNum)->isGmDevice)
    {
        return GT_OK;
    }

    if(CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E == ifMode)
    {
        /* reset just units used for loopback */
        if(CPSS_PORT_SPEED_1000_E == speed)
        {
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsMacReset(devNum[%d], portGroup[%d], macNum[%d], macType[%d], action[%d])", devNum, 0, portMacMap, GEMAC_X, RESET);
            rc = mvHwsMacReset(devNum, 0, portMacMap, portMode, GEMAC_X, RESET);
        }
        else
        {
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsMacReset(devNum[%d], portGroup[%d], macNum[%d], macType[%d], action[%d])", devNum, 0, portMacMap, XLGMAC, RESET);
            rc = mvHwsMacReset(devNum, 0, portMacMap, portMode, XLGMAC, RESET);
        }
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Error in mvHwsMacReset, code %d for port[%d]\n", rc, portMacMap);
        }

        if(CPSS_PORT_SPEED_1000_E == speed)
        {
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPcsReset(devNum[%d], portGroup[%d], portMacMap[%d], pcsType[%d], action[%d])", devNum, 0, portMacMap, GPCS, RESET);
            rc = mvHwsPcsReset(devNum, 0, portMacMap, portMode, GPCS, RESET);
        }
        else
        {
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPcsReset(devNum[%d], portGroup[%d], portMacMap[%d], pcsType[%d], action[%d])", devNum, 0, portMacMap, MMPCS, RESET);
            rc = mvHwsPcsReset(devNum, 0, portMacMap, portMode, MMPCS, RESET);
        }
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Error in mvHwsPcsReset, code %d for port[%d]\n", rc, portMacMap);
        }
    }
    else
    {
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            if(portMode != NON_SUP_MODE)
            {
                CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, 0, portMacMap, portMode, PORT_POWER_DOWN);
                rc = mvHwsPortReset(devNum, 0, portMacMap, portMode, PORT_POWER_DOWN);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Error in mvHwsPortReset, code %d for port[%d], portMode[%d]\n", rc, portMacMap, portMode);
                }
            }
        }
        else
        {
            if((qsgmiiFullDelete) && (portMode != NON_SUP_MODE))
            {
                if(ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E)
                {
                    /* the QSGMII sets 4 ports on the same SERDES and those 4 MACs should be aligned */
                    /* NOTE: the qsgmiiFullDelete makes sure that 'last' port 'valid' in the 4 ports
                       will get into this code */
                    portMacMap = portMacMap & (~0x3);/* clear the 2 LSBits */
                }


                CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, 0, portMacMap, portMode, PORT_POWER_DOWN);
                rc = mvHwsPortReset(devNum, 0, portMacMap, portMode, PORT_POWER_DOWN);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Error in mvHwsPortReset, code %d for port[%d], portMode[%d]\n", rc, portMacMap, portMode);
                }
            }
        }
    }

    return GT_OK;
}

#if 0
/**
* @internal prvCpssDxChPortBcat2TxFifoThresholdsSet function
* @endinternal
*
* @brief   Set the thresholds in the port?s TXFIFO per it's current status/interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For B0 and above.
*
*/
static GT_STATUS prvCpssDxChPortBcat2TxFifoThresholdsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
#define PRV_CPSS_BOBCAT2_PORT_GE_FIFO_HIGH_WM_CNS   139

    GT_STATUS   rc;     /* return code */
    GT_U32      portMacMap; /* number of MAC mapped on this physical port */
    GT_U32      regAddr;/* register address */
    GT_U32      txFifoMinTh, /* GE TX FIFO minimum threshod */
                wmValue;     /* high and low water marks values */
    GT_U32      txRdThr;    /* Number of FIFO entries required to commence
                                reading from the FIFO.*/

    if(!PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {/* not needed for A0 */
        return GT_OK;
    }

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                 portNum,
                                                                 portMacMap);

    switch (ifMode)
    {
        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
        case CPSS_PORT_INTERFACE_MODE_QSGMII_E:
        case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
        case CPSS_PORT_INTERFACE_MODE_100BASE_FX_E:
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                                    perPortRegs[portMacMap].gePortFIFOConfigReg1;
            txFifoMinTh = (speed <= CPSS_PORT_SPEED_1000_E) ? 50 : 136;
            rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 6, 8, txFifoMinTh);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* When setting the GIGE TxFifoMinTh need to also set the WM fields
             *  so that: 140 > High_WM > Low_WM > MinTh.
             *  Any WM values that meet the above constrains are OK.
             */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                                    perPortRegs[portMacMap].gePortFIFOConfigReg0;
            wmValue = ((txFifoMinTh+1)<<8) | PRV_CPSS_BOBCAT2_PORT_GE_FIFO_HIGH_WM_CNS;
            rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 16, wmValue);
            if(rc != GT_OK)
            {
                return rc;
            }
            break;

        default:

            switch(speed)
            {
                case CPSS_PORT_SPEED_10000_E:
                case CPSS_PORT_SPEED_12000_E:
                    txRdThr = 22;
                    break;
                case CPSS_PORT_SPEED_20000_E:
                    txRdThr = 25;
                    break;

                case CPSS_PORT_SPEED_40000_E:
                    txRdThr = 31;
                    break;

                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }

            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.
                            perPortRegs[portMacMap].xlgPortFIFOsThresholdsConfig;
            rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 11, 5, txRdThr);
            if(rc != GT_OK)
            {
                return rc;
            }

            break;
    }

    return GT_OK;
}
#endif

/**
* @internal prvCpssDxChPortBc2PtpTimeStampFixWa function
* @endinternal
*
* @brief   Set the thresholds in ports Tx FIFO
*
* @note   APPLICABLE DEVICES:      Bobcat2
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note WA fix PTP timestamp problem
*
*/
GT_STATUS prvCpssDxChPortBc2PtpTimeStampFixWa
(
    IN  GT_U8                           devNum
)
{
    GT_STATUS rc;           /* return code */
    GT_U32    port;         /* port number */
    GT_U32    txFifoLoWm;   /* Tx FIFO low level value */
    GT_U32    txFifoHiWm;   /* Tx FIFO high level value */
    GT_U32    txFifoMin;    /* Tx FIFO minimal level value */
    GT_U32    txFifoSize;   /* Tx FIFO size */
    GT_U32    txRdThreshold;/* Tx FIFO read threshold */

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    txFifoLoWm = 0xC;
    txFifoHiWm = 0xE;
    txFifoMin = 0x9;
    txFifoSize = 0xB;
    txRdThreshold = 0xA;

    /* Port FIFO Thresholds Confiruration Registers */
    for (port= 0; (port < PRV_CPSS_PP_MAC(devNum)->numOfPorts); port++)
    {
        PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum, port);

        rc = prvCpssHwPpSetRegField(devNum,
              PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[port].gePortFIFOConfigReg0,
              0, 16, ((txFifoLoWm << 8) | txFifoHiWm));
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssHwPpSetRegField(devNum,
              PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[port].gePortFIFOConfigReg1,
              6, 8, txFifoMin);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(PRV_CPSS_GE_PORT_GE_ONLY_E != PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum, port))
        {
            rc = prvCpssHwPpSetRegField(devNum,
                  PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[port].xlgPortFIFOsThresholdsConfig,
                  5, 11, ((txRdThreshold << 6) | txFifoSize));
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->errata.info_CPSS_DXCH_IMPLEMENT_WA_BOBCAT2_PTP_TIMESTAMP_E.enabled = GT_TRUE;

    return GT_OK;
}

/**
* @internal prvCpssDxChPortAnDisableFixWa function
* @endinternal
*
* @brief   Disable the masking of pcs_rx_er when disabling the Auto-Neg on 10M/100M port speed, CRC error will be reported.
*         Due to missing byte in the packets, CRC errors will be reported
*
* @note   APPLICABLE DEVICES:      Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5.
*
* @param[in] devNum                   - physical device number
* @param[in] physicalPortNum          - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Set field rf_pcs_rx_er_mask_disable to 0 in register Gige_MAC_IP_Unit
*
*/
GT_STATUS prvCpssDxChPortAnDisableFixWa
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    physicalPortNum
)
{
    GT_STATUS   rc;
    GT_BOOL     isLinkUp;
    GT_U32      regAddr;
    GT_U32      rxErrMaskDisable;
    GT_U32      portMacNum;
    CPSS_PORT_SPEED_ENT     speed;

    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, physicalPortNum, portMacNum);

    if (prvCpssDxChPortRemotePortCheck(devNum,physicalPortNum))
    {
        /* WA is not applicable */
        return GT_OK;
    }

    /* for xCat3 device and above in port speed 10M/100M */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[portMacNum].eeeLpiControl[2];
    if (regAddr == PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        /* 1. the xCat3 did not initialize the eeeLpiControl registers!
           2. only sip5 initialize the eeeLpiControl registers!
           3. lion2 not have those registers in the gig-MAC */

        /* WA is not applicable . do not treat is as error.
           allow application to be unaware to when need to call it and when not needed.
        */
        return GT_OK;
    }

    rc = cpssDxChPortLinkStatusGet(devNum, physicalPortNum, &isLinkUp);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortLinkStatusGet command failed in port %d\n", physicalPortNum);
    }

    rc = cpssDxChPortSpeedGet(devNum, physicalPortNum, &speed);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortSpeedGet command failed in port %d\n", physicalPortNum);
    }

    if (isLinkUp == GT_TRUE)
    {
        if ((speed == CPSS_PORT_SPEED_10_E) || (speed == CPSS_PORT_SPEED_100_E))
        {
            /* Save the value of <rf_pcs_rx_er_mask_disable> in Gige MAC eeeLpiControl[2] register */
            if (prvCpssHwPpGetRegField(devNum, regAddr, 7, 1, &rxErrMaskDisable) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }

            CPSS_PORTS_BMP_PORT_ENABLE_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                                           info_PRV_CPSS_DXCH_MISCONFIG_100BASEX_AN_DISABLE_WA_E.
                                           portPcsRxErMaskDisableBmpPtr, portMacNum, BIT2BOOL_MAC(rxErrMaskDisable));

            /* Set <rf_pcs_rx_er_mask_disable> in Gige MAC eeeLpiControl[2] register to be 0 */
            if (prvCpssHwPpSetRegField(devNum, regAddr, 7, 1, BOOL2BIT_MAC(GT_FALSE)) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
        }
    }
    else
    { /* link down */
        if ((speed == CPSS_PORT_SPEED_10_E) || (speed == CPSS_PORT_SPEED_100_E))
        {
            /* Restore <rf_pcs_rx_er_mask_disable> in Gige MAC eeeLpiControl[2] register */
            rxErrMaskDisable = CPSS_PORTS_BMP_IS_PORT_SET_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
                                                              info_PRV_CPSS_DXCH_MISCONFIG_100BASEX_AN_DISABLE_WA_E.
                                                              portPcsRxErMaskDisableBmpPtr, portMacNum);

            if (prvCpssHwPpSetRegField(devNum, regAddr, 7, 1, BOOL2BIT_MAC(rxErrMaskDisable)) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortBcat2PortDelete function
* @endinternal
*
* @brief   Power down port and free all allocated for it resources.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] ifMode                   - interface mode according to which power down port
* @param[in] speed                    - port data  according to which power down port
* @param[in] qsgmiiFullDelete         - GT_TRUE - power down QSGMII interface
*                                      GT_FALSE - just free resources of this specific
*                                      member of QSGMII quadruplet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortBcat2PortDelete
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         qsgmiiFullDelete
)
{
    GT_STATUS               rc;         /* return code */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */
    CPSS_PORT_INTERFACE_MODE_ENT *portIfModePtr;/* interface configured on port now */
    CPSS_PORT_SPEED_ENT          *portSpeedPtr; /* speed configured on port now */

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                 portNum,
                                                                 portMacMap);

    portIfModePtr = &(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap));
    if((CPSS_PORT_INTERFACE_MODE_ILKN4_E == ifMode)
        ||  (CPSS_PORT_INTERFACE_MODE_ILKN8_E == ifMode)
        ||  (CPSS_PORT_INTERFACE_MODE_ILKN4_E == *portIfModePtr)
        ||  (CPSS_PORT_INTERFACE_MODE_ILKN8_E == *portIfModePtr))
    {/* if new or old mode is ILKN */
        rc = prvCpssDxChPortBcat2InterlakenDelete(devNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* if on port was configured interface - reset physical layer and free resources */
    if(*portIfModePtr != CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        portSpeedPtr = &(PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap));
        rc = prvCpssDxChPortBcat2PortReset(devNum, portNum, *portIfModePtr,
                                           *portSpeedPtr, qsgmiiFullDelete);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChBcat2PortResourcesConfig(devNum, portNum, ifMode,speed, GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* Set PTP interface width */
        rc = prvCpssDxChPortBcat2PtpInterfaceWidthSelect(devNum, portNum, CPSS_PORT_SPEED_10_E);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChPortPizzaArbiterIfDelete(devNum, portNum);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* reset port according to new interface */
    rc = prvCpssDxChPortBcat2PortReset(devNum, portNum, ifMode, speed,
                                       qsgmiiFullDelete);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortBcat2InterlakenConfigSet function
* @endinternal
*
* @brief   Configure Interlaken interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChPortBcat2InterlakenConfigSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      bwInGBit;   /* bandwidth of ILKN interface in Gb */
    GT_U32      estBWMBps;  /* real bandwidth in Bps based on allocation
                                                                granularity */
    GT_U32      regAddr;/* register address */

    /* init PR memory management */
    prvCpssDxChPortIlknPrMemInit();

    bwInGBit = (CPSS_PORT_SPEED_20000_E == speed) ? 20 : 40;
    rc = prvCpssDxChPortIlknIFBWSet(devNum, bwInGBit, /*OUT*/&estBWMBps);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.PR.PRFunctional.minMaxPkts;
    rc = prvCpssDrvHwPpWriteRegister(devNum, regAddr, 0x0A00003F);
    if(GT_OK != rc)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.PR.PRFunctional.general;

    /* enable to recalculate CRC in outgoing descriptor */
    return prvCpssDrvHwPpSetRegField(devNum, regAddr, 0, 1, 1);
}

/**
* @internal prvCpssDxChPortBcat2InterlakenTxDmaEnableSet function
* @endinternal
*
* @brief   Set Tx DMA enable state for Interlaken interface.
*
* @note   APPLICABLE DEVICES:      Bobcat2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChPortBcat2InterlakenTxDmaEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL enable
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;/* register address */


    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_ilkn.supported == GT_FALSE)
    {
        /* nothing needed */
        return GT_OK;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.ILKN_WRP.
                                                        txdmaConverterControl0;

    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 17, 1, BOOL2BIT_MAC(enable));

    return rc;
}

/**
* @internal prvCpssDxChPortBcat2FcaBusWidthSet function
* @endinternal
*
* @brief   FCA bus width configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported speed for given port
*/
GT_STATUS prvCpssDxChPortBcat2FcaBusWidthSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS rc; /* return code */
    GT_U32 regAddr; /* register address */
    GT_U32 regValue; /* register value */
    GT_U32 fieldOffset; /* register value */
    GT_U32 fieldLength; /* register value */
    GT_U32 portMacNum; /* number of mac mapped to this physical port */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
    fieldOffset = 4;
    fieldLength = 3;

    if(( ifMode == CPSS_PORT_INTERFACE_MODE_ILKN4_E  ) ||
       ( ifMode == CPSS_PORT_INTERFACE_MODE_ILKN8_E  ) ||
       ( ifMode == CPSS_PORT_INTERFACE_MODE_ILKN12_E ) ||
       ( ifMode == CPSS_PORT_INTERFACE_MODE_ILKN16_E ) ||
       ( ifMode == CPSS_PORT_INTERFACE_MODE_ILKN24_E ))
    {
        regValue = 4; /* width 512bit */
    }
    else
    {
        switch(speed)
        {
            case CPSS_PORT_SPEED_10_E:
            case CPSS_PORT_SPEED_100_E:
            case CPSS_PORT_SPEED_1000_E:
                regValue = 0; /* width 8bit */
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
                {
                    regValue = 1; /* width 64bit */
                }
                break;

            case CPSS_PORT_SPEED_10000_E:
            case CPSS_PORT_SPEED_12000_E:
            case CPSS_PORT_SPEED_12500_E:
            case CPSS_PORT_SPEED_2500_E:
            case CPSS_PORT_SPEED_5000_E:
            case CPSS_PORT_SPEED_11800_E:
                regValue = 1; /* width 64bit */
                if(PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum) &&
                  (portMacNum == 32) && (speed == CPSS_PORT_SPEED_2500_E))
                {
                    regValue = 0; /* width 8bit */
                }
                break;

            case CPSS_PORT_SPEED_29090_E:
            case CPSS_PORT_SPEED_40000_E:
            case CPSS_PORT_SPEED_47200_E:
            case CPSS_PORT_SPEED_50000_E:  /* Aldrin(4lanes) / BC3(2 or 4 lanes) */
            case CPSS_PORT_SPEED_52500_E:  /* Aldrin2(2lanes) */
                regValue = 3; /* width256 */
                break;

            case CPSS_PORT_SPEED_100G_E:   /* BC3 */
            case CPSS_PORT_SPEED_107G_E:   /* BC3 */
            case CPSS_PORT_SPEED_102G_E:   /* BC3 */
                regValue = 4; /* width512 */
                break;

            case CPSS_PORT_SPEED_20000_E:
            case CPSS_PORT_SPEED_23600_E:
            case CPSS_PORT_SPEED_25000_E:
            case CPSS_PORT_SPEED_26700_E: /* Aldrin(2lanes) / BC3(1 or 2 lanes) */
                if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum) ||
                   PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum) ||
                   PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum) )
                {
                    regValue = 3; /* width256 */
                }
                else
                {
                    regValue = 1; /* width 64bit */
                }
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }

    rc =  prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, regValue);
    return rc;
}

/**
* @internal prvCpssDxChPortBcat2FcaBusWidthGet function
* @endinternal
*
* @brief   FCA bus width configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported speed for given port
*/
GT_STATUS prvCpssDxChPortBcat2FcaBusWidthGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                         *widthBitsPtr
)
{
    GT_STATUS rc; /* return code */
    GT_U32 regAddr; /* register address */
    GT_U32 regValue; /* register value */
    GT_U32 fieldOffset; /* register value */
    GT_U32 fieldLength; /* register value */
    GT_U32 portMacNum; /* number of mac mapped to this physical port */
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    static GT_U32 fcaBusWidthBits[] =
    {
        /*   0x0 = Type A; BUS_8_BIT; 1G port         */   8
        /*   0x1 = Type B; BUS_64_BIT; 2.5G-10G port  */, 64
        /*   0x2 = Type C; BUS_128_BIT; No such port  */,128
        /*   0x3 = Type D; BUS_256_BIT; 40G port      */,256
        /*   0x4 = Type E; BUS_512_BIT; 100G port     */,512
    };
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(widthBitsPtr);

    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum,portNum,/*OUT*/&portMapShadowPtr);
    if (rc != GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if (portMapShadowPtr->valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                 portNum,
                                                                 portMacNum);

    regAddr = PRV_DXCH_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
    fieldOffset = 4;
    fieldLength = 3;


    *widthBitsPtr = (GT_U32)(~0);
    rc =  prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, /*OUT*/&regValue);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,  LOG_ERROR_NO_MSG);
    }

    if (regValue >=  sizeof(fcaBusWidthBits)/sizeof(fcaBusWidthBits[0]))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);/* out of array boundary */
    }
    *widthBitsPtr = fcaBusWidthBits[regValue];
    return GT_OK;
}


/**
* @internal bcat2QsgmiiConfig function
* @endinternal
*
* @brief   Check if port needs configuration on MAC/PCS/Serdes levels,
*         force link down on ports of its quadruplet if application not
*         configured them yet.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] fullConfigPtr            - call port delete/create or just configure pizza
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*
* @note In QSGMII can't configure only one port, because of internal physical
*       synchronization issues, but we don't want application get linkup interrupts
*       from ports it
*
*/
static GT_STATUS bcat2QsgmiiConfig
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 powerUp,
    OUT GT_BOOL                 *fullConfigPtr
)
{
    GT_STATUS                    rc; /* return code */
    GT_U32                       i;  /* iterator */
    GT_PHYSICAL_PORT_NUM         portMacNum; /* MAC number mapped to given port */
    GT_U32                       firstInQuadruplet;  /* mac number of first port in quadruplet */
    GT_U32                       regAddr;    /* address of register */
    GT_U32                       value;
    GT_U32                       portNumIter;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum,
                                                                 portMacNum);

    if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        value = ((powerUp) ? 1 /* force link up*/: 0/* force link down*/);

        if(value == 0)
        {
            /* the call is on 'physical port' (not on 'MAC port') */
            rc = prvCpssDxChHwEgfEftFieldSet(devNum,portNum,
                PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_PHYSICAL_PORT_LINK_STATUS_MASK_E,
                value);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            /* we can not force link UP ... we need to wait for interrupt
               from the MAC to know that ports is actually 'link UP'

               FIX JIRA : CPSS-4899 : Ports 0-11 in link up when working with higher ports
            */
        }
    }

    *fullConfigPtr = GT_TRUE;
    if((GT_TRUE == powerUp) && (CPSS_PORT_IF_MODE_QSGMII_USX_QUSGMII_CHECK_MAC(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacNum))))
    {
        *fullConfigPtr = GT_FALSE;
        return GT_OK;
    }

    firstInQuadruplet = portMacNum&0xFFFFFFFC;
    if(GT_TRUE == powerUp)
    {
        for(i = firstInQuadruplet; i < firstInQuadruplet+4; i++)
        {
            if(i == portMacNum)
            {
                /* set application configuration for force link down */
                rc = prvCpssDxChPortForceLinkDownEnableSetMac(devNum,portNum,
                            CPSS_PORTS_BMP_IS_PORT_SET_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.info_PRV_CPSS_DXCH_LION2_DISMATCH_PORT_LINK_WA_E.portForceLinkDownBmpPtr,portNum));
                if(rc != GT_OK)
                {
                    return rc;
                }

                continue;
            }
            if(CPSS_PORT_INTERFACE_MODE_NA_E == PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, i))
            {
                /* because HWS configures all four ports of QSGMII quadruplet
                   when application requires to configure one of them,
                   force link down on GE MAC of port to prevent link on ports
                   not configured yet by application */
                PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,i,&regAddr);
                if (prvCpssDrvHwPpPortGroupSetRegField(devNum,
                                                       CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                       regAddr, 0, 1, 1) != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                }
            }
            else if(CPSS_PORT_IF_MODE_QSGMII_USX_QUSGMII_CHECK_MAC(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, i)))
            {
                *fullConfigPtr = GT_FALSE;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
    }
    else
    {
        for(i = firstInQuadruplet; i < firstInQuadruplet+4; i++)
        {
            if(i != portMacNum)
            {
                if(CPSS_PORT_INTERFACE_MODE_NA_E !=
                                            PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, i))
                {
                    *fullConfigPtr = GT_FALSE;
                    break;
                }
            }
        }

        if(*fullConfigPtr == GT_TRUE)
        {
            for(i = firstInQuadruplet; i < firstInQuadruplet+4; i++)
            {
                rc = prvCpssDxChPortPhysicalPortMapReverseMappingGet(devNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, i, &portNumIter);
                if(GT_OK != rc)
                {
                    return rc;
                }

                /* set application configuration for force link down */
                rc = prvCpssDxChPortForceLinkDownEnableSetMac(devNum,portNumIter,
                            CPSS_PORTS_BMP_IS_PORT_SET_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.info_PRV_CPSS_DXCH_LION2_DISMATCH_PORT_LINK_WA_E.portForceLinkDownBmpPtr,portNumIter));
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
        else
        {
#if 0
            /* set force link down enable in case that SERDES is still in power up state*/
            rc = prvCpssDxChPortForceLinkDownEnableSetMac(devNum,portNum,GT_TRUE);
            if(rc != GT_OK)
            {
                return rc;
            }
#endif
            PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&regAddr);
            if (prvCpssDrvHwPpPortGroupSetRegField(devNum,
                                                   CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                   regAddr, 0, 1, 1) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
        }
    }

    /* because force link down doesn't prevent packets to enter queue,
           must set link down here */
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) && !PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        if (*fullConfigPtr == GT_TRUE)
        {
            value = ((powerUp) ? 2/* force link down*/ :0 /*use MAC indication*/);
            /*close all ports */
            for (i = firstInQuadruplet; i < firstInQuadruplet+4; i++)
            {
                rc = prvCpssDxChHwEgfEftFieldSet(devNum, portMacNum,
                                             PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_PHYSICAL_PORT_LINK_STATUS_MASK_E,
                                             value);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
            if (powerUp)
            {
                /*init the first */
                value = ((powerUp) ? 0 /*use MAC indication*/ : 2/* force link down*/);
                rc = prvCpssDxChHwEgfEftFieldSet(devNum,portMacNum,
                                                 PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_PHYSICAL_PORT_LINK_STATUS_MASK_E,
                                                 value);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
        else
        {
            value = ((powerUp) ? 0 /*use MAC indication*/ : 2/* force link down*/);
            rc = prvCpssDxChHwEgfEftFieldSet(devNum,portMacNum,
                                             PRV_CPSS_DXCH_HW_EGF_EFT_FIELD_EGR_FILTER_PHYSICAL_PORT_LINK_STATUS_MASK_E,
                                             value);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal checkMLGonePortConfigured function
* @endinternal
*
* @brief   Check if port one of four ports already configured to MLG mode.
*         force link down on ports of its quadruplet if application not
*         configured them yet.
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] powerUp                  - serdes power:
*                                      GT_TRUE - up;
*                                      GT_FALSE - down;
*
* @param[out] fullConfigPtr            - call port delete/create or just configure pizza
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS checkMLGonePortConfigured
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 powerUp,
    OUT GT_BOOL                 *fullConfigPtr,
    OUT GT_U8                   *configuredPortsBmpPtr
)
{
    GT_PHYSICAL_PORT_NUM    portMacNum; /* MAC number mapped to given port */
    GT_U32      firstPortInGop;  /* mac number of first port in GOP */
    GT_U32      i;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    *fullConfigPtr = GT_TRUE;
    *configuredPortsBmpPtr = 0;
    firstPortInGop = (portMacNum & 0xFFFFFFFC);

    if(powerUp == GT_TRUE)
    {
        /* check the port's mode in all four ports in the GOP */
        for(i = firstPortInGop; i < (firstPortInGop + 4); i++)
        {
            if(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, i) == CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E)
            {
                /* if one port in the GOP already configured to MLG mode,
                   no need to configure other ports */
                *fullConfigPtr = GT_FALSE;
                *configuredPortsBmpPtr = *configuredPortsBmpPtr | (1 << (i-firstPortInGop));
            }
            else if(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, i) != CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                /* if one port in the GOP already configured to other mode, the GOP can't be configured to
                   MLG mode for any port in this GOP */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
    }
    else
    { /* power Down */
        for(i = firstPortInGop; i < (firstPortInGop + 4); i++)
        {
            if(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, i) == CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E)
            {
                *configuredPortsBmpPtr = *configuredPortsBmpPtr | (1 << (i-firstPortInGop));
                /* if other ports in the GOP already configured to MLG mode, the GOP configuraition will be saved
                   for these ports */
                if(i != portMacNum)
                {
                    *fullConfigPtr = GT_FALSE;
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChLedPortTypeConfig function
* @endinternal
*
* @brief   Configures the type of the port connected to the LED.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] powerUp                  - port is powered Up(GT_TRUE) or powered down(GT_FALSE)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssDxChLedPortTypeConfig
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         powerUp
)
{
    GT_U32      portMacNum;      /* port MAC number */
    GT_U32      regAddr;         /* register address */
    GT_STATUS   rc;              /* return status*/
    GT_U32      ledInterfaceNum; /* LED interface number */
    GT_U32      ledPort;         /* LED port number */
    GT_U32      bitOffset;       /* register field offset */
    GT_U32      fieldValue;      /* register field value */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* port MAC type */
    GT_U32      class1Source;    /* source indication of class #1 - link status */
    GT_U32      portLedPosition;

    if(PRV_CPSS_PP_MAC(devNum)->isGmDevice)
    {
        /* GM not support it */
        return GT_OK;
    }

    if (PRV_CPSS_SIP_5_10_CHECK_MAC(devNum) == 0)
    {
        return GT_OK;
    }


    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                 portNum,
                                                                 portMacNum);

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* Update LED port position for current mac type */
        portLedPosition = PRV_CPSS_DXCH_PORT_LED_POSITION(devNum, portMacNum);
        rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, portLedPosition);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"cpssDxChLedStreamPortPositionSet, portNum = %d\n", portNum);
        }

        return GT_OK;
    }

    rc = cpssDxChLedStreamPortPositionGet(devNum, portNum, &ledPort);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (ledPort == 0x3f)
    {
        /* LED port not connected */
        return GT_OK;
    }


    rc = prvCpssDxChPortLedInterfaceGet(devNum, portNum, &ledInterfaceNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChDevLedInterfacePortCheck(devNum, ledInterfaceNum,ledPort);
    if (rc != GT_OK)
    {
        return rc;
    }


    /* use default source #1 for class 1 indication. */
    class1Source = 1;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].portTypeConfig[ledPort/16];
    portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum);
    switch (portMacType)
    {
        case PRV_CPSS_PORT_NOT_EXISTS_E:
        case PRV_CPSS_PORT_FE_E:
        case PRV_CPSS_PORT_GE_E:
            fieldValue = 0;
            /* use Link_test_fail indication for link status of GIG ports */
            class1Source = 0xa;
            break;
        case PRV_CPSS_PORT_XG_E:
        case PRV_CPSS_PORT_XLG_E:
            fieldValue = 1;
            break;
        case PRV_CPSS_PORT_CG_E:
            fieldValue = 2;
            break;
        case PRV_CPSS_PORT_ILKN_E:
            fieldValue = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    bitOffset = (ledPort%16) * 2;

    /* Ports type configuration */
    rc =  prvCpssDrvHwPpSetRegField(devNum, regAddr, bitOffset, 2, fieldValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (GT_FALSE != PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum,
            PRV_CPSS_DXCH_BOBCAT2_LED_LINK_GIG_INDICATION_WA_E))
    {
        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) &&
           (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E) &&
           (ledInterfaceNum > 3))
        {
            /* do nothing - no need WA for BobK for led interface 4,5 */
        }
        else
        {
            regAddr = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->GOP.LED[ledInterfaceNum].portIndicationSelect[ledPort];
            bitOffset = 5;
            rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, bitOffset, 5, class1Source);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    /* for BC3 if FE is applicable */
    if (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_BOBCAT3_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E))
    {
        PRV_CPSS_PORT_TYPE_ENT macType;

        if (powerUp == GT_TRUE)
        {
            macType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum);
        }
        else
        {
            macType = PRV_CPSS_PORT_NOT_EXISTS_E; /* disconnect Mac led indication */
        }
        rc = prvCpssDxChLedErrataPortLedMacTypeSet(devNum,portNum,macType);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal sip_6_10_prvCpssDxChPortSerdesFreeCheck function
* @endinternal
*
* @brief   Check that the Serdes for port Configuration is free and not used by another port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_STATE             - on port cannot be configured when the serdes, that port need
*                                       for configuration is not free
* @retval GT_NOT_SUPPORTED         - i/f mode/speed pair not supported
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS sip_6_10_prvCpssDxChPortSerdesFreeCheck
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS               rc;           /* return code */
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_STANDARD    portMode;
    GT_U32                  portRef;
    GT_U32                  portGroup;
    GT_U32                  portMacMap;
    PRV_CPSS_REG_DB_INFO_STC behind_regDbInfo;
    PRV_CPSS_REG_DB_INFO_STC ahead_regDbInfo;
    GT_U32                   ii,iiMax,jj;
    GT_U32                  iiRef;/* relative index in the DP of the port */

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    CPSS_TBD_BOOKMARK_AC5P
    if(CPSS_PORT_MULTIPLE_USX_PORT_MODE_CHECK_MAC(ifMode))
    {
        return GT_OK;
    }
    /*dont check cpu port */
    if(mvHwsMtipIsReducedPort(devNum, portMacMap) == GT_TRUE)
    {
        return GT_OK;
    }
    /** info for function to know which the BMP of MACs that their SERDESs may
       eclipse my SERDES */
    rc = prvCpssSip6RegDbInfoGet(devNum,portMacMap,PRV_CPSS_REG_DB_TYPE_MAC_ECLIPSED_BY_SERDES_BEHIND_E,&behind_regDbInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    portRef = behind_regDbInfo.startMacIn_macBmpEclipseSerdes;
    iiMax = behind_regDbInfo.startMacIn_macBmpEclipseSerdes + 32;
    iiRef = 0;
    for(ii = behind_regDbInfo.startMacIn_macBmpEclipseSerdes;ii < iiMax;ii ++,portRef++,iiRef++)
    {
        if(portRef == portMacMap)
        {
            /* no more */
            break;
        }

        if(0 == ((1<<iiRef) & behind_regDbInfo.macBmpEclipseSerdes))
        {
            continue;
        }

        /* skip not initialized ports */
        if (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portRef].portIfMode == CPSS_PORT_INTERFACE_MODE_NA_E)
        {
            continue;
        }
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portRef].portIfMode,
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portRef].portSpeed, &portMode);

        if(GT_NOT_INITIALIZED == rc)
        {
            continue;
        }

        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portRef);

        rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, portRef, portMode, &curPortParams);
        if (GT_OK != rc)
        {
            return rc;
        }

        ahead_regDbInfo.startMacIn_macBmpEclipseSerdes = curPortParams.numOfActLanes;
        /* get the bmp of MACs that the 'portRef' is eclipsing ahead with curPortParams.numOfActLanes */
        rc = prvCpssSip6RegDbInfoGet(devNum,portRef,PRV_CPSS_REG_DB_TYPE_MAC_ECLIPSE_SERDES_AHEAD_E,&ahead_regDbInfo);
        if(rc != GT_OK)
        {
            return rc;
        }

        for(jj = 0 ; jj < 32; jj++)
        {
            if(0 == ((1<<jj) & ahead_regDbInfo.macBmpEclipseSerdes))
            {
                continue;
            }

            if(portMacMap == (ahead_regDbInfo.startMacIn_macBmpEclipseSerdes + jj))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "physical port[%d] with MAC port[%d] not allow to use SERDESes because MAC port [%d] is holding [%d] SERDESes",
                    portNum,portMacMap,
                    portRef,curPortParams.numOfActLanes);
            }
        }
    }

    /* Checking is serdes, needs for current port don't used already by port with mac number AFTER current mac number */
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacMap);

    rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacMap, portMode, &curPortParams);
    if (GT_OK != rc)
    {
        return rc;
    }

    ahead_regDbInfo.startMacIn_macBmpEclipseSerdes = curPortParams.numOfActLanes;
    /* get the bmp of MACs that the 'portMacMap' is eclipsing ahead with curPortParams.numOfActLanes */
    rc = prvCpssSip6RegDbInfoGet(devNum,portMacMap,PRV_CPSS_REG_DB_TYPE_MAC_ECLIPSE_SERDES_AHEAD_E,&ahead_regDbInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    for(jj = 0 ; jj < 32; jj++)
    {
        if(0 == ((1<<jj) & ahead_regDbInfo.macBmpEclipseSerdes))
        {
            continue;
        }

        portRef = ahead_regDbInfo.startMacIn_macBmpEclipseSerdes + jj;

        if(portRef == portMacMap)
        {
            continue;
        }


        if(PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portRef].portIfMode == CPSS_PORT_INTERFACE_MODE_NA_E &&
           PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portRef].portSpeed  == CPSS_PORT_SPEED_NA_E)
        {
            /* avoid calling in this case to prvCpssCommonPortIfModeToHwsTranslate(...) because it will return 'GT_NOT_INITIALIZED'
               (to avoid it from generate 'CPSS error LOG' indication)
            */
            continue;
        }

        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portRef].portIfMode,
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portRef].portSpeed, &portMode);

        if(GT_NOT_INITIALIZED == rc)
        {
            continue;
        }

        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "physical port[%d] with MAC port[%d] not allow to use [%d] SERDESes because MAC port [%d] 'hold SERDES(es)'",
            portNum,portMacMap,
            curPortParams.numOfActLanes,
            portRef);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortSerdesFreeCheck function
* @endinternal
*
* @brief   Check that the Serdes for port Configuration is not free and not used
*         by another port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_STATE             - on port cannot be configured when the serdes, that port need
*                                       for configuration is not free
* @retval GT_NOT_SUPPORTED         - i/f mode/speed pair not supported
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortSerdesFreeCheck
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS               rc;           /* return code */
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U8                   sdVecSize;
    MV_HWS_PORT_STANDARD    portMode;
    GT_U32                  portRef;
    GT_U32                  divider;
    GT_U32                  difference;
    GT_U32                  portGroup;
    GT_BOOL                 extendedMode;
    GT_U32                  portMacMap;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        return sip_6_10_prvCpssDxChPortSerdesFreeCheck(devNum,portNum,ifMode,speed);
    }

    /* in BC3 any port can be set to MLG mode */
    if((PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) &&
       (ifMode == CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E))
    {
        return GT_OK;
    }

    if(PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum,portMacMap) == PRV_CPSS_GE_PORT_GE_ONLY_E)
    {
        return GT_OK;
    }

    /* Checking is serdes, needs for current port don't used already by port with mac number BEFORE current mac number */
    if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        rc = cpssDxChPortExtendedModeEnableGet(devNum, portNum, &extendedMode);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        if(GT_TRUE == extendedMode)
        {
            return GT_OK;
        }
    }

    for(divider = 2; divider <= 4; divider += 2)
    {
        portRef = (portMacMap / divider) * divider;
        difference = portMacMap - portRef;
        if(difference > 0)
        {
            /* skip not initialized ports */
            if (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portRef].portIfMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                continue;
            }
            rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portRef].portIfMode,
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portRef].portSpeed, &portMode);

            if(GT_NOT_INITIALIZED == rc)
            {
                continue;
            }

            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portRef);

            rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, portRef, portMode, &curPortParams);
            if (GT_OK != rc)
            {
                return rc;
            }
            sdVecSize = curPortParams.numOfActLanes;

            if(sdVecSize > difference)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "physical port[%d] with MAC port[%d] not allow to use SERDESes because MAC port [%d] is holding [%d] SERDESes",
                    portNum,portMacMap,
                    portRef,curPortParams.numOfActLanes);
            }
        }
    }

    /* Checking is serdes, needs for current port don't used already by port with mac number AFTER current mac number */
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }


    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacMap);

    rc = hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacMap, portMode, &curPortParams);
    if (GT_OK != rc)
    {
        return rc;
    }
    sdVecSize = curPortParams.numOfActLanes;

    for(divider = 1; divider < sdVecSize; divider++)
    {

        if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
        {
            rc = cpssDxChPortExtendedModeEnableGet(devNum, portNum + divider, &extendedMode);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            if(GT_TRUE == extendedMode)
            {
                continue;
            }
        }

        if(PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap + divider].portIfMode == CPSS_PORT_INTERFACE_MODE_NA_E &&
           PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap + divider].portSpeed  == CPSS_PORT_SPEED_NA_E)
        {
            /* avoid calling in this case to prvCpssCommonPortIfModeToHwsTranslate(...) because it will return 'GT_NOT_INITIALIZED'
               (to avoid it from generate 'CPSS error LOG' indication)
            */
            continue;
        }


        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap + divider].portIfMode,
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap + divider].portSpeed, &portMode);

        if(GT_NOT_INITIALIZED == rc)
        {
            continue;
        }

        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "physical port[%d] with MAC port[%d] not allow to use [%d] SERDESes because MAC port [%d] 'hold SERDES(es)'",
            portNum,portMacMap,
            curPortParams.numOfActLanes,
            portMacMap + divider);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortBcat2ModeSpeedSet function
* @endinternal
*
* @brief   Configure Interface mode and speed on a specified port and execute
*         on port's serdeses power up sequence; or configure power down on port's
*         serdeses.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Caelum; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portsBmp                 - physical port number (or CPU port)
* @param[in] powerUp                  - serdes power:
*                                      GT_TRUE - up;
*                                      GT_FALSE - down;
* @param[in] ifMode                   - interface mode (related only for serdes power up [powerUp==GT_TRUE])
* @param[in] speed                    - port data  (related only for serdes power up [powerUp==GT_TRUE])
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortBcat2ModeSpeedSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORTS_BMP_STC              portsBmp,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS               rc;         /* return code */
    GT_PHYSICAL_PORT_NUM    portNum;    /* iterator */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */
    GT_BOOL                 supported;  /* is ifMode/speed supported on port */
    MV_HWS_PORT_STANDARD    portMode;
    GT_BOOL                 lbPort;
    MV_HWS_REF_CLOCK_SUP_VAL refClock;
    CPSS_PORT_INTERFACE_MODE_ENT *portIfModePtr;
    CPSS_PORT_SPEED_ENT          *portSpeedPtr;
    GT_BOOL                  fullConfig; /* because in QSGMII mode one serdes serves four ports
                                                    when one port of the quadruplete configured three
                                                    others configured too, so when application asks
                                                    to configure another port from quadruplete we don't need
                                                    to execute full configuretion of new port */
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    PRV_CPSS_DXCH_PORT_STATE_STC      portStateStc;  /* current port state */
    GT_BOOL                  isPortAlreadyConfigured;
    MV_HWS_PORT_INIT_INPUT_PARAMS   portInitInParam;

    if(powerUp == GT_TRUE)
    {
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChSerdesRefClockTranslateCpss2Hws(devNum, &refClock);
        if (rc != GT_OK)
        {
            return rc;
        }

        lbPort = (CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E == ifMode) ?
                                                        GT_TRUE : GT_FALSE;
    }
    else
    {/* to prevent warnings */
        portMode = NON_SUP_MODE;
        refClock = MHz_156;
        lbPort = GT_FALSE;
    }

    for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {

        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portsBmp, portNum))
        {
            PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                         portNum,
                                                                         portMacMap);
        }
        else
        {
            continue;
        }

        if ((PRV_CPSS_GE_PORT_GE_ONLY_E == PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum, portMacMap))
                && (portMacMap < 48) && (CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E == ifMode)
                && (portMacMap != (portMacMap & 0xFFFFFFFC)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        portIfModePtr = &(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap));
        portSpeedPtr = &(PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap));

        rc = prvCpssDxChPortIsAlreadyConfigured(devNum, portNum, powerUp, *portIfModePtr, *portSpeedPtr, ifMode, speed, &isPortAlreadyConfigured);
        if (rc != GT_OK)
        {
            return rc;
        }
        if(isPortAlreadyConfigured)
        {
            continue;
        }

        rc = prvCpssDxChPortSerdesFreeCheck(devNum,portNum,ifMode,speed);
        if(rc != GT_OK)
        {
             return rc;
        }

        if(powerUp == GT_FALSE)
        {
            /* we remove the ports, so need to use current interface and speed
             * if configured, not the values from the caller */
            if((CPSS_PORT_INTERFACE_MODE_NA_E != *portIfModePtr) &&
                                        (CPSS_PORT_SPEED_NA_E != *portSpeedPtr))
            {
                ifMode = *portIfModePtr;
                speed =  *portSpeedPtr;
            }
            else
            {
                continue; /* port is already power down */
            }
        }

        rc = prvCpssCommonPortInterfaceSpeedGet(devNum, portMacMap,
                                                              ifMode, speed,
                                                              &supported);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(!supported)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssDxChPortStateDisableAndGet(devNum,portNum,&portStateStc);
        if (rc != GT_OK)
        {
            return rc;
        }

        fullConfig = GT_TRUE;
        if(CPSS_PORT_IF_MODE_QSGMII_USX_QUSGMII_CHECK_MAC(ifMode))
        {
            rc = bcat2QsgmiiConfig(devNum, portNum, powerUp, &fullConfig);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        rc = prvCpssDxChPortBcat2PortDelete(devNum, portNum, ifMode, speed,
                                            fullConfig);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* save new interface mode in DB after prvCpssDxChPortBcat2PortDelete */
        *portIfModePtr = CPSS_PORT_INTERFACE_MODE_NA_E;
        *portSpeedPtr = CPSS_PORT_SPEED_NA_E;

        if(!powerUp)
        {
            rc = prvCpssDxChPortStateRestore(devNum, portNum, &portStateStc);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = prvCpssDxChPortBcat2InterlakenTxDmaEnableSet(devNum, GT_FALSE);
            if (rc != GT_OK)
            {
                return rc;
            }

            continue;
        }

        rc = prvCpssDxChPortPizzaArbiterIfConfigure(devNum, portNum,((speed < CPSS_PORT_SPEED_1000_E) ? CPSS_PORT_SPEED_1000_E : speed));
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortPizzaArbiterIfConfigure command failed in port %d\n", portNum);
        }
        if(!PRV_CPSS_PP_MAC(devNum)->isGmDevice)
        {
            rc = prvCpssDxChBcat2PortResourcesConfig(devNum, portNum, ifMode, speed,GT_TRUE);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChBcat2PortResourcesConfig command failed in port %d\n", portNum);
            }
        }
        /* Set FCA interface width */
        rc = prvCpssDxChPortBcat2FcaBusWidthSet(devNum, portNum, ifMode, speed);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortBcat2FcaBusWidthSet command failed in port %d\n", portNum);
        }

        /* Set PTP interface width */
        rc = prvCpssDxChPortBcat2PtpInterfaceWidthSelect(devNum, portNum, speed);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortBcat2PtpInterfaceWidthSelect command failed in port %d\n", portNum);
        }

        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, &portMapShadowPtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortPhysicalPortMapShadowDBGet command failed in port %d\n", portNum);
        }

        if(portMapShadowPtr->portMap.trafficManagerEn)
        {
            /* Port speed calibration value for TM Flow Control */
            #if defined (INCLUDE_TM)
                        rc = prvCpssDxChTmGlueFlowControlPortSpeedSet(devNum,
                                                          portMapShadowPtr->portMap.tmPortIdx,
                                                                      speed);
            #else
                rc = GT_NOT_SUPPORTED;
            #endif /*#if defined (INCLUDE_TM)*/


            if (rc != GT_OK)
            {
                return rc;
            }
        }

        if((CPSS_PORT_INTERFACE_MODE_ILKN4_E == ifMode)
            || (CPSS_PORT_INTERFACE_MODE_ILKN8_E == ifMode))
        {/* if new mode is ILKN */
            rc = prvCpssDxChPortBcat2InterlakenConfigSet(devNum, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortBcat2InterlakenConfigSet command failed in port %d\n", portNum);
            }

            rc = prvCpssDxChPortBcat2InterlakenTxDmaEnableSet(devNum, GT_TRUE);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortBcat2InterlakenTxDmaEnableSet command failed in port %d\n", portNum);
            }
        }

        if(fullConfig)
        {
            if(CPSS_PORT_IF_MODE_QSGMII_USX_QUSGMII_CHECK_MAC(ifMode))
            {/* provide to HWS first port in quadruplet and it will configure
                all other ports if needed */
                portMacMap &= 0xFFFFFFFC;
            }

            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortFlavorInit(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], lbPort[%d], refClock[%d], refClockSource[%d]})", devNum, 0, portMacMap, portMode, lbPort, refClock, PRIMARY_LINE_SRC);
            cpssOsMemSet(&portInitInParam, 0,sizeof(portInitInParam));
            portInitInParam.lbPort = lbPort;
            portInitInParam.refClock = refClock;
            portInitInParam.refClockSource = PRIMARY_LINE_SRC;
            rc = mvHwsPortFlavorInit(devNum, 0, portMacMap, portMode, &portInitInParam);
            if (rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
                return rc;
            }
        }

        prvCpssDxChPortTypeSet(devNum, portNum, ifMode, speed);

        /* update addresses of mac registers accordingly to used MAC GE/XLG/etc. */
        rc = prvCpssDxChHwRegAddrPortMacUpdate(devNum, portNum,ifMode);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChHwRegAddrPortMacUpdate command failed in port %d\n", portNum);
        }

        if((CPSS_PORT_SPEED_10_E == speed) || (CPSS_PORT_SPEED_100_E == speed))
        {/* HWS doesn't support 10/100M, so after 1G configured by HWS, fix speed here */
            rc = geMacUnitSpeedSet(devNum, portNum, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "geMacUnitSpeedSet command failed in port %d\n", portNum);
            }
        }

        /* Configures type of the port connected to the LED */
        rc = prvCpssDxChLedPortTypeConfig(devNum, portNum,powerUp);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChLedPortTypeConfig command failed in port %d\n", portNum);
        }

        rc = prvCpssDxChPortImplementWaSGMII2500(devNum, portNum, ifMode, speed);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortImplementWaSGMII2500 command failed in port %d\n", portNum);
        }

        /* save new interface mode in DB */
        *portIfModePtr = ifMode;
        *portSpeedPtr = speed;

         /*Creating Port LoopBack*/
        if ((PRV_CPSS_PP_MAC(devNum)->revision > 0) && lbPort)
        {
            rc = prvCpssDxChPortSerdesLoopbackActivate (devNum, portNum, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortSerdesLoopbackActivate command failed in port %d\n", portNum);
            }
        }

        rc = prvCpssDxChPortStateRestore(devNum, portNum, &portStateStc);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChPortStateRestore command failed in port %d\n", portNum);
        }

    } /* for(portNum = 0; */

    return GT_OK;
}

/**
* @internal prvCpssDxChPortLoopbackDisableRestore function
* @endinternal
*
* @brief   Takes care to Set loopback to disable if it was
*         previousely set to enable.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - physical device number
* @param[in] portsBmp                 - physical port number
* @param[in] powerUp                  - serdes power:
*                                      GT_TRUE - up;
*                                      GT_FALSE - down;
* @param[in] ifMode                   - interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortLoopbackDisableRestore
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORTS_BMP_STC              portsBmp,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      i;  /* iterator */
    CPSS_PORT_INTERFACE_MODE_ENT prevIfMode; /* interface mode before setting */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);

    for(i = 0; i < PRV_CPSS_PP_MAC(devNum)->numOfPorts; i++)
    {
        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portsBmp, i))
        {
            if(powerUp == GT_TRUE)
            {
                prevIfMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, i);
                if(CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E == prevIfMode)
                {
                    rc = prvCpssDxChPortLion2InternalLoopbackEnableSet(devNum, i, GT_FALSE);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
            else
            {
                if(CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E == ifMode)
                {
                    rc = prvCpssDxChPortLion2InternalLoopbackEnableSet(devNum, i, GT_FALSE);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
        }
        else
        {
            continue;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChPortMoveDetect function
* @endinternal
*
* @brief   Detect if port moved from regular to extended MAC or back. which means
*         that although no change was in interface/speed and link on old
*         connection still up - port must be completely reconfigured.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] movedPtr                 - GT_TRUE - change happend
*                                      GT_FALSE - no change
*
* @retval GT_OK                    - on success,
*                                       otherwise fail.
*/
static GT_STATUS prvCpssDxChPortMoveDetect
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *movedPtr
)
{
    GT_STATUS               rc; /* return code */
    CPSS_PP_FAMILY_TYPE_ENT devFamily;  /* device family */

    devFamily = PRV_CPSS_PP_MAC(devNum)->devFamily;

    *movedPtr = GT_FALSE;

    if(CPSS_PP_FAMILY_DXCH_LION2_E == devFamily)
    {
        GT_BOOL extendedMode;   /* is port use extended MAC */
        GT_U32  macStatusRegAddr;/* current MAC status register */
        GT_U32  currentMacOffset; /* current offset of MAC registers of given
                                                                        port */
        GT_U32  localPort;

        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portNum);
        if((localPort != 9) && (localPort != 11))
        {
            return GT_OK;
        }

        rc = cpssDxChPortExtendedModeEnableGet(devNum, portNum, &extendedMode);
        if(rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_DXCH_PORT_STATUS_CTRL_REG_MAC(devNum, portNum, &macStatusRegAddr);
        currentMacOffset = macStatusRegAddr & 0xF000;
        if(((GT_FALSE == extendedMode) && (currentMacOffset > 0xB000))
            || ((GT_TRUE == extendedMode) && (currentMacOffset <= 0xB000)))
        {/* i.e. extended mode changed before portModeSpeedSet engaged and it
            must be completely executed although link is still up and no change
            was in interface/speed */
            *movedPtr = GT_TRUE;
        }
    }

    return GT_OK;
}


static GT_U32   debug_enable_prvFalconPortCreateWa_start = 1;
GT_STATUS debug_enable_prvFalconPortCreateWa_start_set(GT_U32   enable)
{
    debug_enable_prvFalconPortCreateWa_start = enable;
    return GT_OK;
}

/**
* @internal prvFalconPortDeleteWa_enable function
* @endinternal
*
* @brief   enable flag to protect the port delete
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                  - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvFalconPortDeleteWa_enable
(
    IN  GT_U8                  devNum
)
{
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr)
    {
        /* state that we are doing 'port disable' from 'port delete' context , and we may need the 'port delete WA' */
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr->doingPortDisableFromPortDelete = GT_TRUE;
        return GT_OK;
    }
    return GT_OK;
}

/**
* @internal prvFalconPortDeleteWa_disable function
* @endinternal
*
* @brief   disable flag that protect the port delete of
*          this port;
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - physical port number
* @param[in] portIsEnable          - port is currently enable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvFalconPortDeleteWa_disable
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_BOOL                portIsEnable
)
{
    GT_STATUS rc;
    /* reset the flag (before checking rc !) */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr)
    {
     /*
             For SIP6.0 in order to perform port power down WA when port
             already in disabled state it's needed to enable/disable it.
        */
        if(portIsEnable == GT_FALSE)
        {
            rc = cpssDxChPortEnableSet(devNum, portNum, GT_TRUE);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            rc = cpssDxChPortEnableSet(devNum, portNum, GT_FALSE);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr->doingPortDisableFromPortDelete = GT_FALSE;
    }
    return GT_OK;
}

/**
* @internal prvFalconPortCreateWa_start function
* @endinternal
*
* @brief   check if we need to protect the port create of this port with interaction with OTHER ports in the DP[] !
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                  - physical device number
* @param[in] portNum                 - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS prvFalconPortCreateWa_start
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum
)
{

    if ((PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr == NULL) ||
        (PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr->isWaNeeded == GT_FALSE))
    {
        return GT_OK;
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr->waAlreadyDoneArr[portNum] = GT_FALSE;

    if(!debug_enable_prvFalconPortCreateWa_start)
    {
        /* bypass the 'FORCE' to forget , for debug purposes */
    }
    else
    if (0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(PRV_CPSS_PP_MAC(devNum)->portEgfForceStatusBitmapPtr, portNum))
    {
        /* FORCE to 'forget' about the application request to 'EGF link UP' on this port !             */
        /* we want to make sure the application explicitly set 'EGF link up' after every 'port create' */
        /* (due to the 'port create' potential bug that we are dealing with here)                      */
        CPSS_PORTS_BMP_PORT_SET_MAC(PRV_CPSS_PP_MAC(devNum)->portEgfForceStatusBitmapPtr, portNum);
    }

    return GT_OK;
}

/**
* @internal prvCpssSetSpecialSpeedConfiguration function
* @endinternal
*
* @brief   check if we need to set IPG/CRC/Preamble for special
*          speeds (106G-R4/42G-R4/53G-R2
*
* @note   APPLICABLE DEVICES:      Falcon;
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                  - physical device number
* @param[in] portNum                 - physical port number
* @param[in] speed                   - port Speed
* @param[in] powerUp                 - Enable or disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS prvCpssSetSpecialSpeedConfiguration
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_SPEED_ENT     speed,
    IN  GT_BOOL                 powerUp
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 crcValue = (GT_TRUE == powerUp) ? 0 : 4;
    GT_U32 preambleValue = (GT_TRUE == powerUp) ? 1 : 8;
    GT_U32 ipgValue = (GT_TRUE == powerUp) ? 1 : 12;

        /* Falcon 106G AP connect to TD3 High2 header need fomal support for packet forward and send to BCM correctly.
        Requirement - JIRA 10600 */
    if ((CPSS_PORT_SPEED_106G_E == speed) ||
        (CPSS_PORT_SPEED_42000_E == speed) ||
        (CPSS_PORT_SPEED_53000_E == speed))
    {
        rc = prvCpssDxChFalconPortMacCrcModeSet(CAST_SW_DEVNUM(devNum), portNum, CPSS_PORT_DIRECTION_BOTH_E, crcValue);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"calling prvCpssDxChFalconPortMacCrcModeSet from wrapper failed=%d",rc);
        }

        rc = prvCpssDxChFalconPortMacPreambleLengthSet(CAST_SW_DEVNUM(devNum), portNum, preambleValue);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"calling prvCpssDxChFalconPortMacPreambleLengthSet from wrapper failed=%d",rc);
        }

        /* deal with frame loss */
        rc = cpssDxChPortIpgSet(CAST_SW_DEVNUM(devNum), portNum, ipgValue);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"calling cpssDxChPortIpgSet from wrapper failed=%d",rc);
        }
    }
    return rc;
}

/**
* @internal prvCpssDxChPortFecModeGet function
* @endinternal
*
* @brief   Read current Forward error correction (FEC) mode of port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portMode                 - port speed and mode
* @param[out] modePtr                 - current Forward error correction mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortFecModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  MV_HWS_PORT_STANDARD        portMode,
    OUT CPSS_DXCH_PORT_FEC_MODE_ENT *modePtr
)
{
    GT_STATUS               rc;
    GT_U32                  localPort;  /* number of port in GOP */
    GT_U32                  portGroupId;/* iterator of port groups */
    GT_U32                  portMacNum; /* MAC number */
    MV_HWS_PORT_FEC_MODE    fecMode; /* FEC state on port */

    CPSS_NULL_PTR_CHECK_MAC(modePtr);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    switch(portMode)
    {
        case HWS_10G_MODE_CASE:
        case HWS_25G_MODE_CASE:
        case HWS_40G_R4_MODE_CASE:
        case HWS_50G_R2_MODE_CASE:
        case HWS_100G_R4_MODE_CASE:
        case HWS_PAM4_MODE_CASE:
        case HWS_200G_R8_MODE_CASE:
        case HWS_D_USX_MODE_CASE:
        case HWS_Q_USX_MODE_CASE:
        case HWS_SX_MODE_CASE:
        case _12GBaseR:
        case _20GBase_KR2:
        case _20GBase_SR_LR2:
        case _26_7GBase_KR:
        case _40GBase_KR2:
        case _50GBase_KR4:

            break;
        default:
            *modePtr = CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
            return GT_OK;
    }

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portMacNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portMacNum);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortFecCofigGet(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], *portFecEn)", devNum, portGroupId, localPort, portMode);
    rc = mvHwsPortFecCofigGet(devNum, portGroupId, localPort, portMode, &fecMode);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }

    switch(fecMode)
    {
        case FC_FEC:
            *modePtr = CPSS_DXCH_PORT_FEC_MODE_ENABLED_E;
            break;

        case FEC_OFF:
            *modePtr = CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
            break;

        case RS_FEC:
            *modePtr = CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E;
            break;

        case RS_FEC_544_514:
            *modePtr = CPSS_DXCH_PORT_RS_FEC_544_514_MODE_ENABLED_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal internal_original_cpssDxChPortModeSpeedSet function
* @endinternal
*
* @brief   Configure Interface mode and speed on a specified port and execute
*         on port's serdeses power up sequence; or configure power down on port's
*         serdeses.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portsBmpPtr              - (pointer to) physical ports bitmap (or CPU port)
* @param[in] powerUp                  - serdes power:
*                                      GT_TRUE - up;
*                                      GT_FALSE - down;
* @param[in] ifMode                   - interface mode (related only for serdes power up [powerUp==GT_TRUE])
* @param[in] speed                    - port data  (related only for serdes power up [powerUp==GT_TRUE])
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if port was not initialized
*                                     (does not exist in port
*                                     DB)
*
* @note Supposed to replace old API's:
*       cpssDxChPortInterfaceModeSet
*       cpssDxChPortSpeedSet
*       cpssDxChPortSerdesPowerStatusSet
*       The API rolls back a port's mode and speed to their last values
*       if they cannot be set together on the device.
*       Pay attention! Before configure CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E
*       MUST execute power down for port with this interface.
*       Pay attention! Unlike other interfaces which are ready to forward traffic
*       after this API pass, interlaken interfaces require call afterwards
*       cpssDxChPortIlknChannelSpeedSet to configure channel.
*
*/
static GT_STATUS internal_original_cpssDxChPortModeSpeedSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORTS_BMP_STC             *portsBmpPtr,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      i;  /* iterator */
    GT_U32      apPortIndex;
    PRV_CPSS_DXCH_PORT_INTERFACE_MODE_SET_FUN ifModeSetFuncPtr; /* pointer to
                                    [devFamily][ifMode] specific function */
    PRV_CPSS_DXCH_PORT_SERDES_POWER_STATUS_SET_FUN serdesPowerSetFuncPtr;
                            /* pointer to [devFamily] specific function */
    CPSS_PORT_INTERFACE_MODE_ENT prevIfMode =CPSS_PORT_INTERFACE_MODE_NA_E;
                            /* stores port's last interface mode*/
    CPSS_PORT_SPEED_ENT          prevSpeed; /* stores port's last speed*/
    PRV_CPSS_DXCH_PORT_INTERFACE_MODE_GET_FUN ifModeGetFuncPtr; /* pointer to
                                    [devFamily] specific function */
    GT_BOOL originalPortState; /* port enable state defined by application */
    GT_BOOL linkUp;             /* is link currently up */
    GT_BOOL moved;              /* port exit moved from regular to extended MAC */

    GT_BOOL apEnabled = GT_FALSE; /* indicate ap enabled and ap port enabled */
    GT_U32 portMacMap; /* number of mac mapped to this physical port */
    GT_BOOL isCpu;  /* is physical port mapped to CPU */
    GT_BOOL forceLinkState; /* is force link pass set on port */
    GT_BOOL                         apEnable;   /* AP enabled on port */
    MV_HWS_PORT_STANDARD            portMode = NON_SUP_MODE;
    GT_BOOL                         havingNotRegularTxDmaBwModePort; /* having Not Regular Port*/
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U8  sdVecSize;
    GT_U16 *sdVectorPtr;
    GT_U32 portGroupId;
    GT_U32 localPort;  /* number of port in GOP */
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
    GT_BOOL extMode = GT_FALSE;
    GT_U32 extPortMacMap;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    if(ifMode == CPSS_PORT_INTERFACE_MODE_KR_S_E ||
       ifMode == CPSS_PORT_INTERFACE_MODE_CR_S_E ||
       ifMode == CPSS_PORT_INTERFACE_MODE_CR2_C_E ||
       ifMode == CPSS_PORT_INTERFACE_MODE_KR2_C_E ||
       ifMode == CPSS_PORT_INTERFACE_MODE_CR_C_E ||
       ifMode == CPSS_PORT_INTERFACE_MODE_KR_C_E )
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }



/* check if AP port - in case of AP port cpssDxChPortModeSpeedSet is called with ifMode and speed with addition of _NA_E value to the values requested*/
    if (ifMode > CPSS_PORT_INTERFACE_MODE_NA_E && speed > CPSS_PORT_SPEED_NA_E)
    {
        if ((PRV_CPSS_SIP_5_CHECK_MAC(devNum)) || (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
        {
            /* check if AP is enabled */
            rc = cpssDxChPortApEnableGet(devNum, 0, &apEnabled);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "cpssDxChPortModeSpeedSet - try to set AP port but can not get FW status");
            }
            if (apEnabled)
            {
                portMacMap = 0; /* avoid compiler warning */
                for(apPortIndex = 0; apPortIndex < CPSS_MAX_PORTS_NUM_CNS; apPortIndex++)
                {
                    if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(portsBmpPtr, apPortIndex))
                    {
                        PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, apPortIndex, portMacMap);
                        portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacMap);

                        extPortMacMap = portMacMap;
                        CHECK_STATUS(hwsExtendedPortNumGet(devNum, portGroupId,  &extMode, &extPortMacMap));
                        if (extMode == GT_TRUE)
                        {
                            CHECK_STATUS(mvHwsExtendedPortSerdesTxIfSelectSet(devNum, portGroupId, portMacMap, _10GBase_KR));
                        }
                        break;
                    }
                }


                if(apPortIndex < CPSS_MAX_PORTS_NUM_CNS)
                {
                    rc = prvCpssDxChPortApHighLevelConfigurationSet(devNum, apPortIndex, powerUp, (ifMode - CPSS_PORT_INTERFACE_MODE_NA_E), (speed - CPSS_PORT_SPEED_NA_E));
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortModeSpeedSet - fail to set pizza for AP port");
                    }
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "cpssDxChPortModeSpeedSet - try to set AP port but wrong port number");
                }


                rc = prvCpssSetSpecialSpeedConfiguration(devNum, apPortIndex, (speed-CPSS_PORT_SPEED_NA_E), powerUp);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"calling prvCpssSetSpecialSpeedConfiguration from wrapper failed=%d",rc);
                }

                if ((PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) || (CPSS_PP_FAMILY_DXCH_AC5_E == PRV_CPSS_PP_MAC(devNum)->devFamily) )
                {
                    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum, ifMode - CPSS_PORT_INTERFACE_MODE_NA_E, speed - CPSS_PORT_SPEED_NA_E, &portMode);
                    if(rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortModeSpeedSet - failed to translate port mode");
                    }
                    rc = hwsPortModeParamsSetMode(devNum,0, portMacMap, powerUp ? portMode : NON_SUP_MODE);
                    if(rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortModeSpeedSet - failed to update Elements databse on host CPU");
                    }
                }

                if(powerUp)
                {
                    if(CPSS_PP_FAMILY_DXCH_FALCON_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
                    {
                        rc = prvFalconPortCreateWa_start(devNum,apPortIndex);
                        if(rc != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvFalconPortCreateWa_start:rc=%d,portNum=%d\n",rc, portMacMap);
                        }
                        rc = mvHwsFalconPortCommonInit(devNum, 0, portMacMap, portMode);
                        if(rc != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsFalconPortCommonInit:rc=%d,portNum=%d\n",rc, portMacMap);
                        }
                    }
                    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    /* send ack to SrvCPU that Pizza was configure*/
                    if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
                    {
                        rc = prvCpssDxChPortApPortModeAckSet(devNum, apPortIndex, (ifMode - CPSS_PORT_INTERFACE_MODE_NA_E), (speed - CPSS_PORT_SPEED_NA_E));
                        if (rc != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortModeSpeedSet - fail to send ack to srvCPU that pizza was set");
                        }
                    }
                }
                else
                {
                    if(CPSS_PP_FAMILY_DXCH_FALCON_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
                    {
                        rc = mvHwsFalconPortApReset(devNum, 0, portMacMap, portMode);
                        if(rc != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "mvHwsFalconPortApReset:rc=%d,portNum=%d\n",rc, portMacMap);
                        }
                    }
                }
                return rc;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "cpssDxChPortModeSpeedSet - try to set AP port but FW was not loaded");
            }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "cpssDxChPortModeSpeedSet - try to ser AP port but AP is not supported on this device");
        }
    }


    if ((PRV_CPSS_SIP_5_CHECK_MAC(devNum)) || (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        /* before configuring cpssDxChPortModeSpeedSet check that ap is not running on port */
        for(i = 0; i < PRV_CPSS_PP_MAC(devNum)->numOfPorts; i++)
        {
            if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(portsBmpPtr, i))
            {
                if(prvCpssDxChPortRemotePortCheck(devNum,i))
                    continue;
                PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, i, portMacMap);
                rc = cpssDxChPortApPortEnableGet(devNum, i, &apEnable);
                if (rc != GT_OK)
                {
                    return rc;
                }
                if (apEnable == GT_TRUE)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
        }
    }



    if(powerUp == GT_TRUE)
    {
        if((GT_U32)ifMode >= CPSS_PORT_INTERFACE_MODE_NA_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if((GT_U32)speed >= CPSS_PORT_SPEED_NA_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

            /* check datapath bandwidth overbooking by new configuration */
        if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) && (! PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
        {
            /* check only when there is at least one port with not regulal */
            /* TX_DMA Bandwidth mode.                                      */
            rc = prvCpssDxChPortDynamicPAPortSpeedDBHavePortWithNotRegularFactor(
                devNum, &havingNotRegularTxDmaBwModePort);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            if (havingNotRegularTxDmaBwModePort != GT_FALSE)
            {
                rc = prvCpssDxChPortDynamicPizzaArbiterBWOverbookDevicePortsBitmapCheck(
                    devNum, portsBmpPtr, speed);
                if (rc != GT_OK)
                {
                    if (rc == GT_NO_RESOURCE)
                    {
                        cpssOsPrintf("\nNo Enough Bandwidth for required configuration\n");
                    }
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
          }
    }
    else
    {
        /*Clear fast link configuration if already set by application*/
        /*Applies to BC3 & Aldrin2 devices only*/
        if((PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) && (!PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
        {
            for(i = 0; i < CPSS_MAX_PORTS_NUM_CNS; i++)
            {
                if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(portsBmpPtr, i))
                {
                    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                                i,
                                                                                portMacMap);

                    /*Check if fastlink is configured & clear */
                    if (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap].isFastLink)
                    {
                        rc = cpssDxChPortFastLinkDownEnableSet(devNum, i/*portNum*/, GT_FALSE);
                        if(rc != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                        }
                    }
                }
                else
                {
                    continue;
                }
            }
        }
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            for(i = 0; i < CPSS_MAX_PORTS_NUM_CNS; i++)
            {
                if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(portsBmpPtr, i))
                {
                    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                                i,
                                                                                portMacMap);
                    ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap);
                    speed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap);
                    if ((speed == CPSS_PORT_SPEED_NA_E) || (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E))
                    {
                        /* port already deleted */
                        return GT_OK;
                    }
                }
            }
        }
        else if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            /* for all other port modes: we not care about those ifMode,
               speed from the caller ! as the port is going down
               set dummy values that fit sip6 devices */
            ifMode = CPSS_PORT_INTERFACE_MODE_KR_E;
            speed  = CPSS_PORT_SPEED_10000_E;
        }
        else
        /* in MLG mode we want to power down only the specific port number
           from the group */
        if (ifMode != CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E)
        {
            /* for all other port modes: we not care about those ifMode,
               speed from the caller ! as the port is going down
               set dummy values */
            ifMode = CPSS_PORT_INTERFACE_MODE_SGMII_E;
            speed  = CPSS_PORT_SPEED_1000_E;
        }
    }

    if((PRV_CPSS_SIP_5_10_CHECK_MAC(devNum)) ||
        (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
        {
            rc = prvCpssDxChPortLoopbackDisableRestore(devNum, *portsBmpPtr, powerUp, ifMode);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    if ((PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) || (CPSS_PP_FAMILY_DXCH_AC5_E == PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        for(i = 0; i < CPSS_MAX_PORTS_NUM_CNS; i++)
        {
            if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(portsBmpPtr, i))
            {
                PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                             i,
                                                                             portMacMap);
                rc = prvCpssCommonPortIfModeToHwsTranslate(devNum, ifMode, speed, &portMode);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssCommonPortIfModeToHwsTranslate - failed ");
                }
                rc = hwsPortModeParamsSetMode(devNum,0, portMacMap, powerUp ? portMode : NON_SUP_MODE);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwsPortModeParamsSetMode - failed ");
                }
            }
            else
            {
                continue;
            }
        }
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        return prvCpssDxChPortFalconModeSpeedSet(devNum, *portsBmpPtr, powerUp, ifMode,speed);
    }
    else if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        return prvCpssDxChPortCaelumModeSpeedSet(devNum, *portsBmpPtr, powerUp, ifMode,speed);
    }

    if(PRV_CPSS_DXCH_BOBCAT2_CHECK_MAC(devNum))
    {
        return prvCpssDxChPortBcat2ModeSpeedSet(devNum, *portsBmpPtr, powerUp, ifMode,speed);
    }

    if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        return prvCpssDxChPortXcat3ModeSpeedSet(devNum, *portsBmpPtr, powerUp, ifMode,speed);
    }

    if(powerUp == GT_TRUE)
    {
        ifModeSetFuncPtr = PORT_OBJ_FUNC(devNum).setPortInterfaceMode[ifMode];
        ifModeGetFuncPtr = PORT_OBJ_FUNC(devNum).getPortInterfaceMode;
        if(ifModeSetFuncPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        ifModeGetFuncPtr = NULL;
        ifModeSetFuncPtr = NULL;
    }

    if((serdesPowerSetFuncPtr = PORT_OBJ_FUNC(devNum).
                                                setSerdesPowerStatus) == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for(i = 0; i < PRV_CPSS_PP_MAC(devNum)->numOfPorts; i++)
    {
        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(portsBmpPtr, i))
        {
            PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum,i);
        }
        else
        {
            continue;
        }
        rc = cpssDxChPortPhysicalPortMapIsCpuGet(devNum, i, &isCpu);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(isCpu == GT_FALSE)
        {
            GT_BOOL resNew;
            /*
            // resOld = supportedPortsModes[PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum,i)][ifMode];
            */

            rc = prvCpssDxChPortSupportedModeCheck(devNum,i,ifMode,/*OUT*/&resNew);
            if (rc != GT_OK)
            {
                return GT_OK;
            }

            if (resNew == GT_FALSE)
            {

                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
        }

        /* save port's previous interface mode for rollback*/
        if ((ifModeGetFuncPtr != NULL) &&
                            (ifModeGetFuncPtr(devNum,i,&prevIfMode) != GT_OK))
        {
            prevIfMode = CPSS_PORT_INTERFACE_MODE_NA_E;
        }

        /* Get previous port speed, if failed set to N/A*/
        rc = prvCpssDxChPortSpeedGet(devNum,i,&prevSpeed);
        if(rc != GT_OK)
            prevSpeed = CPSS_PORT_SPEED_NA_E;

        /* if port already configured to required interface and link is up
            no need to reconfigure it */
        if((GT_TRUE == powerUp) && (prevIfMode == ifMode) && (prevSpeed == speed))
        {
            rc = cpssDxChPortLinkStatusGet(devNum, i, &linkUp);
            if (rc != GT_OK)
            {
                return rc;
            }

            if(linkUp)
            {
                rc = cpssDxChPortForceLinkPassEnableGet(devNum, i, &forceLinkState);
                if (rc != GT_OK)
                {
                    return rc;
                }

                rc = cpssDxChPortApPortEnableGet(devNum, i, &apEnable);
                if (rc != GT_OK)
                {
                    return rc;
                }

                /* check if new configuration done on different MAC used for
                    same physical port i.e. port move from regular MAC to
                    extended or back */
                rc = prvCpssDxChPortMoveDetect(devNum, i, &moved);
                if(rc != GT_OK)
                {
                    return rc;
                }

                /* if link up skip configuration if port not uses now
                    different MAC and no force link pass defined on port (i.e. link up not fake)
                    and no AP enabled on port */
                if((!moved) && (!forceLinkState) && (!apEnable))
                {/* move to next port if needed otherwise return OK */
                    continue;
                }
            }
        }

        /* disable port before port's units reset/power down to prevent traffic stuck */
        originalPortState = GT_FALSE;
        if(((CPSS_PORT_INTERFACE_MODE_HGL_E != prevIfMode) &&
                                (CPSS_PORT_INTERFACE_MODE_NA_E != prevIfMode))
            || ((CPSS_PORT_SPEED_15000_E != prevSpeed) &&
                                        (CPSS_PORT_SPEED_16000_E != prevSpeed)))
        {
            rc = prvCpssDxChPortEnableGet(devNum, i, &originalPortState);
            if (rc != GT_OK)
            {
                return rc;
            }

            if(originalPortState != GT_FALSE)
            {
                rc = prvCpssDxChPortEnableSet(devNum, i, GT_FALSE);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }

        if(GT_TRUE == powerUp)
        {
            /* configure requested interface mode on port */
            if((rc = ifModeSetFuncPtr(devNum, i, ifMode)) != GT_OK)
            {
                goto rollBackSpeedMode;
            }
            /* cpu port has not entry in phyPortInfoArray - skip SW port info DB update */
            if(isCpu == GT_FALSE)
            {
                /* update current port type - used to define which mac unit currently
                    in use by port */
                if(PRV_CPSS_PP_MAC(devNum)->devFamily < CPSS_PP_FAMILY_DXCH_XCAT_E)
                {
                    prvCpssDxChPortTypeSet(devNum,i,ifMode,speed);
                }

                /* save new interface mode in DB */
                PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, i) = ifMode;
            }

            /* Set port speed, if failed roll back changes*/
            rc = prvCpssDxChPortSpeedSet(devNum,i,speed);
            if(rc != GT_OK)
                goto rollBackSpeedMode;

            if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                rc = prvCpssDxChPortForceLinkDownEnableSetMac(devNum, i, GT_FALSE);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }

        }
        else
        {
            if(PRV_CPSS_PORT_FE_E == PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,i))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG); /* Fast Ethernet ports have no serdes,
                                            so power down not applicable for them */

            if((CPSS_PORT_INTERFACE_MODE_NA_E == prevIfMode)
                                        || (CPSS_PORT_SPEED_NA_E == prevSpeed))
            {
                PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, i) = ifMode;
                PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, i) = speed;
            }

            if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E) ||
                (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT2_E))
            {
                rc = prvCpssDxChPortForceLinkDownEnableSetMac(devNum, i, GT_TRUE);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }

        }

        if((isCpu == GT_TRUE) ||
            (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,i) == PRV_CPSS_PORT_FE_E))
        {
            continue;
        }

        rc = serdesPowerSetFuncPtr(devNum, i, CPSS_PORT_DIRECTION_BOTH_E, 0xFF, powerUp);
        if(rc != GT_OK)
        {
            goto rollBackSpeedMode;
        }

        if (powerUp)
        {
            if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

                portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,i);
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,i);

                rc = hwsPortModeParamsGetToBuffer(devNum, portGroupId, localPort, portMode, &curPortParams);
                if (GT_OK != rc)
                {
                    return rc;
                }
                sdVecSize   = curPortParams.numOfActLanes;
                sdVectorPtr = curPortParams.activeLanesList;
                /* set the Polarity values on Serdeses if SW DB values initialized */
                rc = prvCpssDxChPortSerdesPolaritySet(devNum, portGroupId, sdVectorPtr, sdVecSize);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortSerdesPolaritySet, portNum = %d\n", i);
                }
            }
        }

        if(originalPortState != GT_FALSE)
        {
            rc = prvCpssDxChPortEnableSet(devNum,i,GT_TRUE);
            if (rc != GT_OK)
            {
                goto rollBackSpeedMode;
            }
        }
    }

    return GT_OK;

    /* Rolls back changes to port and returns the rc which was retruned*/
rollBackSpeedMode:
     if ((ifModeGetFuncPtr != NULL) && (prevSpeed != CPSS_PORT_SPEED_NA_E)
         &&   (prevIfMode != CPSS_PORT_INTERFACE_MODE_NA_E))
     {
         ifModeSetFuncPtr(devNum, i, prevIfMode);
         if(isCpu ==GT_FALSE)
         {
            /* roll back port type */
            if(PRV_CPSS_PP_MAC(devNum)->devFamily < CPSS_PP_FAMILY_DXCH_XCAT_E)
            {
                prvCpssDxChPortTypeSet(devNum,i,prevIfMode,speed);
            }

            /* return old interface mode to DB */
            PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, i) = prevIfMode;
         }
         prvCpssDxChPortSpeedSet(devNum,i,prevSpeed);
         if((isCpu == GT_FALSE) && (PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,i) != PRV_CPSS_PORT_FE_E))
             serdesPowerSetFuncPtr(devNum, i, CPSS_PORT_DIRECTION_BOTH_E, 0xFF, powerUp);
     }

    return rc;
}

/**
* @internal internal_cpssDxChPortModeSpeedSet function
* @endinternal
*
* @brief   Configure Interface mode and speed on a specified port and execute
*         on port's serdeses power up sequence; or configure power down on port's
*         serdeses.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; AC5; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portsBmpPtr              - (pointer to) physical ports bitmap (or CPU port)
* @param[in] powerUp                  - serdes power:
*                                      GT_TRUE - up;
*                                      GT_FALSE - down;
* @param[in] ifMode                   - interface mode (related only for serdes power up [powerUp==GT_TRUE])
* @param[in] speed                    - port data  (related only for serdes power up [powerUp==GT_TRUE])
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if port was not initialized
*                                     (does not exist in port
*                                     DB)
*
*
*/
static GT_STATUS internal_cpssDxChPortModeSpeedSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORTS_BMP_STC              *portsBmpPtr,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS               rc,rcTimeMeassure;
    CPSS_PORTS_BMP_STC      *capatiblePortsBmpPtr;
    GT_PHYSICAL_PORT_NUM    port;
    GT_U32                  portMacNum;
    GT_U32                  portMacType;
    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of init */

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PM_NOT_APPLICABLE_DEVICE(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    rcTimeMeassure = cpssOsTimeRT(&secondsStart,&nanoSecondsStart);
    if(rcTimeMeassure != GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rcTimeMeassure, LOG_ERROR_NO_MSG);
    }


    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChPort_SIP6_10_ModeSpeedSet(devNum, *portsBmpPtr, powerUp, ifMode,speed, NULL);
    }
    else
    {
        rc = internal_original_cpssDxChPortModeSpeedSet(
            devNum, portsBmpPtr, powerUp, ifMode, speed);
    }

    if (rc != GT_OK) return rc;

    capatiblePortsBmpPtr =
        &(PRV_CPSS_DXCH_PP_MAC(devNum)->errata.
        info_PRV_CPSS_DXCH_PORTS_XLG_UNIDIRECTIONAL_WA_E.waCapablePortsBmp);

    for (port = 0; (port < CPSS_MAX_PORTS_NUM_CNS); port++)
    {
        /* portsBmp contains only valid ports - checked by original function */
        if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(portsBmpPtr, port) == 0) continue;
        if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(capatiblePortsBmpPtr, port) == 0) continue;
        /* power Down case */
        if (powerUp == GT_FALSE)
        {
            /* on power down disable XLG_UNIDIRECTIONAL_WA on each port */
            rc = prvCpssDxChPortXlgBufferStuckWaEnableSet(devNum, port, GT_FALSE);
            if (rc != GT_OK) return rc;
            continue;
        }
        /* power UP case */
        /* XLG ports only*/
        PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, port, portMacNum);
        portMacType = PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum, portMacNum);
        if (portMacType != PRV_CPSS_PORT_XLG_E) continue;

        /* on power down enable XLG_UNIDIRECTIONAL_WA on each XLG port */
        rc = prvCpssDxChPortXlgBufferStuckWaEnableSet(devNum, port, GT_TRUE);
        if (rc != GT_OK) return rc;
    }

    rcTimeMeassure = cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd);
    if(rcTimeMeassure != GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rcTimeMeassure, LOG_ERROR_NO_MSG);
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

    OS_SPEED_CHANGE_PRINTF(("Speed change (cpssDxChPortModeSpeedSet) finished after: %d sec., %d nanosec.\n",seconds, nanoSec));

    return GT_OK;
}

/**
* @internal cpssDxChPortModeSpeedSet function
* @endinternal
*
* @brief   Configure Interface mode and speed on a specified port and execute
*         on port's serdeses power up sequence; or configure power down on port's
*         serdeses.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portsBmpPtr              - (pointer to) physical ports bitmap (or CPU port)
* @param[in] powerUp                  - serdes power:
*                                      GT_TRUE - up;
*                                      GT_FALSE - down;
* @param[in] ifMode                   - interface mode (related only for serdes power up [powerUp==GT_TRUE])
* @param[in] speed                    - port data  (related only for serdes power up [powerUp==GT_TRUE])
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if port was not initialized
*                                     (does not exist in port
*                                     DB)
*
* @note Supposed to replace old API's:
*       cpssDxChPortInterfaceModeSet
*       cpssDxChPortSpeedSet
*       cpssDxChPortSerdesPowerStatusSet
*       The API rolls back a port's mode and speed to their last values
*       if they cannot be set together on the device.
*       Pay attention! Before configure CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E
*       MUST execute power down for port with this interface.
*       Pay attention! Unlike other interfaces which are ready to forward traffic
*       after this API pass, interlaken interfaces require call afterwards
*       cpssDxChPortIlknChannelSpeedSet to configure channel.
*
*/
GT_STATUS cpssDxChPortModeSpeedSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORTS_BMP_STC             *portsBmpPtr,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortModeSpeedSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portsBmpPtr, powerUp, ifMode, speed));

    rc = internal_cpssDxChPortModeSpeedSet(devNum, portsBmpPtr, powerUp, ifMode, speed);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portsBmpPtr, powerUp, ifMode, speed));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortExtendedModeEnableSet function
* @endinternal
*
* @brief   Define which GE and XG MAC ports 9 and 11 of every mini-GOP will use -
*         from local mini-GOP or extended from other mini-GOP
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
*                                      (APPLICABLE RANGES: Lion2: 9,11; xCat3; AC5: 25,27)
* @param[in] enable                   - extended mode:
*                                      GT_TRUE - use extended MAC;
*                                      GT_FALSE - use local MAC;
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For now if application interested to implement GE and XG modes of ports 9
*       and 11 of every mini-GOP over extended MAC's, it can call this function
*       at init stage once for port 9 and once for port 11 and enough
*
*/
static GT_STATUS internal_cpssDxChPortExtendedModeEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS               rc;         /* return code */
    GT_U32                  localPort;  /* number of port in GOP */
    GT_U32                  portGroupId;/* iterator of port groups */
    GT_U32                  regAddr;    /* address of register */
    GT_U32                  fieldData;  /* data to write to register field */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

    if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        if((localPort != 25) && (localPort != 27))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        CPSS_LOG_INFORMATION_MAC("Calling: hwsPortExtendedModeCfg(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], extendedMode[%d])", devNum, 0, portNum, _10GBase_KR/*don't care for XCATC3*/, enable);
        rc = hwsPortExtendedModeCfg(devNum, 0, portNum, _10GBase_KR/*don't care for XCATC3*/, enable);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /*
            In case of internal CPU - two SERDESes (10,11) are configured in UBOOT
            for out-of-band port mode. If NAND mode (non NFS) or USB-to-ETH
            adapter are used two lanes may be used for network ports, but should
            be powered down before usage.

        */
        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E) &&
           (enable == GT_TRUE) &&
           (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->hwInfo[0].busType == CPSS_HW_INFO_BUS_TYPE_MBUS_E))
        {
            MV_HWS_SERDES_CONFIG_STC    serdesConfig;

            serdesConfig.baudRate = _1_25G;
            serdesConfig.media = XAUI_MEDIA;
            serdesConfig.busWidth = _10BIT_ON;
            serdesConfig.refClock = _156dot25Mhz;
            serdesConfig.refClockSource = PRIMARY;
            serdesConfig.encoding = SERDES_ENCODING_NA;
            serdesConfig.serdesType = COM_PHY_C12GP41P2V;

            rc = mvHwsSerdesPowerCtrl(devNum,0,10,GT_FALSE,&serdesConfig);
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            rc = mvHwsSerdesPowerCtrl(devNum,0,11,GT_FALSE,&serdesConfig);
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }

        rc = prvCpssDxChXcat3PortMacRegAddrSwap(devNum, portNum, enable);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portNum);

        if((localPort != 9) && (localPort != 11))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if(enable)
        {
            switch(PRV_CPSS_PP_MAC(devNum)->devType)
            {
                case CPSS_LION_2_THREE_MINI_GOPS_DEVICES_CASES_MAC:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                default:
                    break;
            }
        }

        fieldData = BOOL2BIT_MAC(enable);

        /* use extended or regular MAC for RX DMA */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.rxdmaIfConfig[localPort/8];
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                             (localPort%8)*4+3, 1, fieldData);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /* use extended or regular MAC for TX DMA */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.txdmaExtendedPortsConfig;
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                             ((9 == localPort) ? 0 : 1), 1, fieldData);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortExtendedModeEnableSet function
* @endinternal
*
* @brief   Define which GE and XG MAC ports 9 and 11 of every mini-GOP will use -
*         from local mini-GOP or extended from other mini-GOP
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
*                                      (APPLICABLE RANGES: Lion2: 9,11; xCat3; AC5: 25,27)
* @param[in] enable                   - extended mode:
*                                      GT_TRUE - use extended MAC;
*                                      GT_FALSE - use local MAC;
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For now if application interested to implement GE and XG modes of ports 9
*       and 11 of every mini-GOP over extended MAC's, it can call this function
*       at init stage once for port 9 and once for port 11 and enough
*
*/
GT_STATUS cpssDxChPortExtendedModeEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortExtendedModeEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChPortExtendedModeEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortExtendedModeEnableGet function
* @endinternal
*
* @brief   Read which GE and XG MAC ports 9 and 11 of every mini-GOP will use -
*         from local mini-GOP or extended from other mini-GOP
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
*
* @param[out] enablePtr                - extended mode:
*                                      GT_TRUE - use extended MAC;
*                                      GT_FALSE - use local MAC;
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - enablePtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortExtendedModeEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      localPort;  /* number of port in GOP */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

    if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        if((localPort != 25) && (localPort != 27))
        {
            *enablePtr = GT_FALSE;
            return GT_OK;
        }

        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortExtendedModeCfgGet(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], *extendedMode)", devNum, 0, localPort, _10GBase_KR);
        rc = mvHwsPortExtendedModeCfgGet(devNum, 0, localPort, _10GBase_KR /*don't care for XCAT3*/,enablePtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }
    }
    else
    {
            GT_U32      portGroupId;/* core number */
            GT_U32      regAddr;    /* register address */

        if((localPort != 9) && (localPort != 11))
        {
                    *enablePtr = GT_FALSE;
            return GT_OK;
        }

        switch(PRV_CPSS_PP_MAC(devNum)->devType)
        {
            case CPSS_LION_2_THREE_MINI_GOPS_DEVICES_CASES_MAC:
                *enablePtr = GT_FALSE;
                return GT_OK;
            default:
                break;
        }

            portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->bufferMng.txdmaExtendedPortsConfig;
            rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                         ((9 == localPort) ? 0 : 1), 1, (GT_U32*)enablePtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal cpssDxChPortExtendedModeEnableGet function
* @endinternal
*
* @brief   Read which GE and XG MAC ports 9 and 11 of every mini-GOP will use -
*         from local mini-GOP or extended from other mini-GOP
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
*
* @param[out] enablePtr                - extended mode:
*                                      GT_TRUE - use extended MAC;
*                                      GT_FALSE - use local MAC;
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - enablePtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortExtendedModeEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortExtendedModeEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChPortExtendedModeEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortInterfaceSpeedSupportGet function
* @endinternal
*
* @brief   Check if given pair ifMode and speed supported by given port on
*         given device
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @param[out] supportedPtr             ? GT_TRUE ? (ifMode; speed) supported
*                                      GT_FALSE ? (ifMode; speed) not supported
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if one of input parameters wrong
* @retval GT_BAD_PTR               - if supportedPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortInterfaceSpeedSupportGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    OUT GT_BOOL                         *supportedPtr
)
{
    GT_STATUS rc;
    GT_U32  portMacNum;      /* MAC number */
    GT_BOOL ifIsSupported;
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;
    GT_BOOL doPpMacConfig = GT_TRUE;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(supportedPtr);

    if(((GT_U32)ifMode >= CPSS_PORT_INTERFACE_MODE_NA_E) || ((GT_U32)speed >= CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *supportedPtr = GT_FALSE;
    /* Get PHY MAC object pnt */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    /* run MACPHY callback  */
    if (portMacObjPtr != NULL && portMacObjPtr->macDrvMacPortInterfaceSpeedSupportGetFunc != NULL) {
        rc = portMacObjPtr->macDrvMacPortInterfaceSpeedSupportGetFunc(devNum,portNum,
                                  ifMode, speed, supportedPtr, CPSS_MACDRV_STAGE_PRE_E,
                                  &doPpMacConfig);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* check that this type of port supports given interface */
    /*
    //if(!supportedPortsModes[PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum,portMacNum)][ifMode])
    //{
    //    return GT_OK;
    //}
    */

    rc = prvCpssDxChPortSupportedModeCheck(devNum,portNum,ifMode,/*OUT*/&ifIsSupported);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (ifIsSupported == GT_FALSE)
    {
        return GT_OK;
    }


    if((!PRV_CPSS_SIP_5_CHECK_MAC(devNum)) &&
       (!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {/* check that interface supported by this device family */

        CPSS_PP_FAMILY_TYPE_ENT     devFamily = PRV_CPSS_PP_MAC(devNum)->devFamily;
        PRV_CPSS_DXCH_PORT_INTERFACE_MODE_SET_FUN ifFunsPtr = prvCpssDxChFillPortIfFunctionsSetFind(devFamily,ifMode);
        if (ifFunsPtr == NULL)
        {
            return GT_OK;
        }

    }

    if((PRV_CPSS_PP_MAC(devNum)->devFamily >= CPSS_PP_FAMILY_DXCH_LION2_E) ||
       (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        return prvCpssCommonPortInterfaceSpeedGet(devNum, portMacNum,
                                                                ifMode, speed,
                                                                supportedPtr);
    }
    else
    {
        if (PRV_CPSS_PORT_FE_E == PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum))
        {
            if(speed > CPSS_PORT_SPEED_100_E)
                return GT_OK;

            if((CPSS_PORT_INTERFACE_MODE_SGMII_E == ifMode) ||
               (CPSS_PORT_INTERFACE_MODE_1000BASE_X_E == ifMode))
            {/* portTypeOptions is not enough for FE ports */
                return GT_OK;
            }
        }
        else
        {
            if(CPSS_DXCH_PORT_SERDES_SPEED_NA_E == serdesFrequency[ifMode][speed])
                return GT_OK;
        }

        if(PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
        {
            switch(speed)
            {
                case CPSS_PORT_SPEED_20000_E:
                case CPSS_PORT_SPEED_40000_E:
                    return GT_OK;
                default:
                    break;
            }
        }

        *supportedPtr = GT_TRUE;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortInterfaceSpeedSupportGet function
* @endinternal
*
* @brief   Check if given pair ifMode and speed supported by given port on
*         given device
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @param[out] supportedPtr             ? GT_TRUE ? (ifMode; speed) supported
*                                      GT_FALSE ? (ifMode; speed) not supported
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if one of input parameters wrong
* @retval GT_BAD_PTR               - if supportedPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortInterfaceSpeedSupportGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    OUT GT_BOOL                         *supportedPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortInterfaceSpeedSupportGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ifMode, speed, supportedPtr));

    rc = internal_cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum, ifMode, speed, supportedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ifMode, speed, supportedPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortLion2InterfaceModeHwGet function
* @endinternal
*
* @brief   Gets Interface mode on a specified port from HW.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] ifModePtr                - interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - wrong media interface mode value received
*
* @note Because of specific of QSGMII interface in scenario when application configured
*       just one port of quadruplet before SW reset, after reset catchup will
*       return all four ports as configured to QSGMII - nothing to do with that,
*       because force_link_down, that CPSS uses as WA to cover fact that
*       all port of quadruplet configured at once, can't serve sign for ports not
*       configured by application before reset, because could be application
*       set it to force_link_down.
*
*/
GT_STATUS prvCpssDxChPortLion2InterfaceModeHwGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
)
{
    GT_U32 portGroupId; /*the port group Id - support multi-port-groups device */
    GT_U32 localPort;   /* number of port in local core */
    GT_STATUS   rc;     /* return code */
    MV_HWS_PORT_STANDARD    portMode; /* port mode in HWS format */
    GT_U32  portMacMap; /* number of MAC to which mapped given physical port */

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                 portNum,
                                                                 portMacMap);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portMacMap);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum, portMacMap);

    if((PRV_CPSS_GE_PORT_GE_ONLY_E == PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum,portMacMap))
       && (localPort%4 != 0))
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortInterfaceGet(devNum[%d], portGroup[%d], phyPortNum[%d], *portModePtr)", devNum, portGroupId, localPort);
        rc = mvHwsPortInterfaceGet(devNum, portGroupId, localPort&0xFFFFFFFC, &portMode);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }
        if (QSGMII == portMode)
        {
            goto hws2cpss;
        }
    }

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortInterfaceGet(devNum[%d], portGroup[%d], phyPortNum[%d], *portModePtr)", devNum, portGroupId, localPort);
    rc = mvHwsPortInterfaceGet(devNum, portGroupId, localPort, &portMode);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }

hws2cpss:
    switch(portMode)
    {
        case _100Base_FX:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_100BASE_FX_E;
            break;

        case SGMII:
        case SGMII2_5:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_SGMII_E;
            break;

        case _1000Base_X:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
            break;

        case _2500Base_X:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_2500BASE_X_E;
            break;

        case QSGMII:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_QSGMII_E;
            break;

        case _10GBase_KX4:
        case _20GBase_KX4:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_XGMII_E;
            break;

        case _10GBase_KX2:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_HX_E;
            break;

        case _10GBase_KR:
        case _100GBase_KR10:
        case _12_1GBase_KR:
        case _12_5GBase_KR:
        case _25GBase_KR:
        case _5GBaseR:
        case _26_7GBase_KR:
        case _20GBase_KR:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_KR_E;
            break;

        case _20GBase_KR2:
        case _25GBase_KR2:
        case _50GBase_KR2:
        case _40GBase_KR2:
        case _52_5GBase_KR2:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_KR2_E;
            break;

        case _40GBase_KR4:
        case _50GBase_KR4:
        case _100GBase_KR4:
        case _102GBase_KR4:
        case _107GBase_KR4:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_KR4_E;
            break;

        case _50GBase_CR:
        case _25GBase_CR:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_CR_E;
            break;

        case _25GBase_CR_S:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_CR_S_E;
            break;

        case _25GBase_KR_S:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_KR_S_E;
            break;

        case _100GBase_CR2:
        case _50GBase_CR2:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_CR2_E;
            break;

        case _40GBase_CR4:
        case _100GBase_CR4:
        case _200GBase_CR4:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_CR4_E;
            break;

        case HGL:
        case HGL16G:
        case XLHGL_KR4:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_HGL_E;
            break;

        case RXAUI:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_RXAUI_E;
            break;

        case _10GBase_SR_LR:
        case _100GBase_SR10:
        case _5_625GBaseR:
        case _25GBase_SR:
        case _50GBase_SR_LR:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_SR_LR_E;
CPSS_TBD_BOOKMARK_LION2
/* need to add support for CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E; */
            break;

        case _50GBase_SR2:
        case _20GBase_SR_LR2:
        case _100GBase_SR_LR2:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_SR_LR2_E;
            break;
        case _40GBase_SR_LR4:
        case _29_09GBase_SR4:
        case _100GBase_SR4:
        case _200GBase_SR_LR4:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_SR_LR4_E;
            break;

        case _100GBase_MLG:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E;
            break;

        case _12GBaseR:
        case _24GBase_KR2:
        case _48GBaseR4:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_XHGS_E;
            break;
        case _12GBase_SR:
        case _48GBase_SR4:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_XHGS_SR_E;
            break;
        case CHGL:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_CHGL_12_E;
            break;

        case INTLKN_12Lanes_6_25G:
        case INTLKN_12Lanes_10_3125G:
        case INTLKN_12Lanes_12_5G:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_ILKN12_E;
            break;

        case INTLKN_16Lanes_6_25G:
        case INTLKN_16Lanes_10_3125G:
        case INTLKN_16Lanes_12_5G:
        case INTLKN_16Lanes_3_125G:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_ILKN16_E;
            break;

        case INTLKN_24Lanes_6_25G:
        case INTLKN_24Lanes_3_125G:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_ILKN24_E;
            break;

        case INTLKN_4Lanes_3_125G:
        case INTLKN_4Lanes_6_25G:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_ILKN4_E;
            break;

        case INTLKN_8Lanes_3_125G:
        case INTLKN_8Lanes_6_25G:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_ILKN8_E;
            break;

        case _25GBase_KR_C:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_KR_C_E;
            break;

        case _50GBase_KR2_C:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_KR2_C_E;
            break;

        case _25GBase_CR_C:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_CR_C_E;
            break;

        case _50GBase_CR2_C:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_CR2_C_E;
            break;

        case _50GBase_KR:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_KR_E;
            break;
        case _100GBase_KR2:
        case _102GBase_KR2:
        case _106GBase_KR2:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_KR2_E;
            break;
        case _200GBase_KR4:
        case _212GBase_KR4:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_KR4_E;
            break;
        case _200GBase_KR8:
        case _400GBase_KR8:
        case _424GBase_KR8:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_KR8_E;
            break;
        case _200GBase_CR8:
        case _400GBase_CR8:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_CR8_E;
            break;
        case _200GBase_SR_LR8:
        case _400GBase_SR_LR8:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_SR_LR8_E;
            break;
        case _2_5G_SXGMII:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_USX_2_5G_SXGMII_E;
            break;
        case _5G_SXGMII:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_USX_5G_SXGMII_E;
            break;
        case _10G_SXGMII:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_USX_10G_SXGMII_E;
            break;
        case _5G_DXGMII:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_USX_5G_DXGMII_E;
            break;
        case _10G_DXGMII:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_USX_10G_DXGMII_E;
            break;
        case _20G_DXGMII:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_USX_20G_DXGMII_E;
            break;
        case _5G_QUSGMII:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_USX_QUSGMII_E;
            break;
        case _10G_QXGMII:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_USX_10G_QXGMII_E;
            break;
        case _20G_QXGMII:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_USX_20G_QXGMII_E;
            break;
        case _10G_OUSGMII:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E;
            break;
        case _20G_OXGMII:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E;
            break;

        default:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_NA_E;
            break;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortFecModeSet function
* @endinternal
*
* @brief   Configure Forward error correction (FEC) mode of port
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; AC5; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon; AC5P; AC5X; Harrier; Ironman
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - Forward error correction mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChPortFecModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_DXCH_PORT_FEC_MODE_ENT mode
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      localPort;  /* number of port in GOP */
    GT_U32      portGroupId;/* iterator of port groups */
    MV_HWS_PORT_FEC_MODE    fecMode; /* FEC state on port */
    GT_U32      portMacNum; /* MAC number */
    MV_HWS_PORT_STANDARD portMode;  /* HWS port interface mode */
    CPSS_PORT_INTERFACE_MODE_ENT    portIfMode;
    CPSS_PORT_SPEED_ENT             portSpeed;
    CPSS_PORTS_BMP_STC              portsBmp;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PM_NOT_APPLICABLE_DEVICE(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    PRV_CPSS_CONVERT_CPSS_FEC_MODE_VAL_TO_HWS_MAC(mode,fecMode);
    if (fecMode == FEC_NA)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,portMacNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portMacNum);

    if((CPSS_PORT_INTERFACE_MODE_NA_E == PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacNum))
       || (CPSS_PORT_SPEED_NA_E == PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portMacNum)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                    PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacNum),
                                    PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portMacNum),
                                    &portMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        if(GT_FALSE == hwsIsFecModeSupported(devNum, 0, portMacNum, portMode, fecMode))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        /* store current port configuration */
        portIfMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacNum);
        portSpeed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portMacNum);
        PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);

        if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portMacNum, NON_SUP_MODE, &curPortParams))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
        }
        if(curPortParams.portFecMode == fecMode)
        {
            return GT_OK;
        }

        rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_FALSE, portIfMode, portSpeed);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /* update HWS elements DB */
        rc = hwsPortModeParamsSetFec(devNum,0,portMacNum,portMode,fecMode);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, portIfMode, portSpeed);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        if(GT_FALSE == hwsIsFecModeSupported(devNum, portGroupId, localPort, portMode, fecMode))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroupId, localPort, portMode, &curPortParams))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
        }
        if((FEC_OFF == fecMode) && (FEC_OFF == curPortParams.fecSupportedModesBmp))
        {
            return GT_OK;
        }

        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortFecCofig(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], fecMode[%d])", devNum, portGroupId, localPort, portMode, fecMode);
        rc = mvHwsPortFecCofig(devNum, portGroupId, localPort, portMode, fecMode);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /* update HWS elements DB */
        rc = hwsPortModeParamsSetFec(devNum,portGroupId,localPort,portMode,fecMode);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortFecModeSet function
* @endinternal
*
* @brief   Configure Forward error correction (FEC) mode of port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - Forward error correction mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortFecModeSet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_DXCH_PORT_FEC_MODE_ENT mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortFecModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mode));

    rc = internal_cpssDxChPortFecModeSet(devNum, portNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortFecModeGet function
* @endinternal
*
* @brief   Read current Forward error correction (FEC) mode of port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] modePtr                  - current Forward error correction mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortFecModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_DXCH_PORT_FEC_MODE_ENT *modePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      portMacNum; /* MAC number */
    MV_HWS_PORT_STANDARD portMode;  /* port i/f mode and speed translated to
                                        BlackBox enum */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,
                                    PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum,portMacNum),
                                    PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum,portMacNum),
                                    &portMode);
    if(rc != GT_OK)
    {
        *modePtr = CPSS_DXCH_PORT_FEC_MODE_DISABLED_E;
        return GT_OK;
    }

    rc = prvCpssDxChPortFecModeGet(devNum,portNum,portMode,modePtr);
    return rc;
}

/**
* @internal cpssDxChPortFecModeGet function
* @endinternal
*
* @brief   Read current Forward error correction (FEC) mode of port
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] modePtr                  - current Forward error correction mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortFecModeGet
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_DXCH_PORT_FEC_MODE_ENT *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortFecModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, modePtr));

    rc = internal_cpssDxChPortFecModeGet(devNum, portNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortInterfaceModeHwGet function
* @endinternal
*
* @brief   Gets Interface mode on a specified port from HW.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] ifModePtr                - interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - wrong media interface mode value received
*/
GT_STATUS prvCpssDxChPortInterfaceModeHwGet
(
    IN  GT_U8                          devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
)
{
    GT_U32      portMacNum; /* MAC number */
    GT_STATUS   rc;         /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(ifModePtr);

    *ifModePtr = CPSS_PORT_INTERFACE_MODE_NA_E;
    if (!PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        if(CPSS_CPU_PORT_NUM_CNS == portMacNum)
        {
            return cpuPortIfModeGet(devNum,portMacNum,ifModePtr);
        }
    }
    else
    {
        CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, &portMapShadowPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E == portMapShadowPtr->portMap.mappingType)
        {/* return dummy ifMode to support legacy behavior */
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_MII_E;
            return GT_OK;
        }
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        return prvCpssDxChPortLion2InterfaceModeHwGet(devNum,portNum,ifModePtr);
    }

    rc = prvCpssDxChPortLion2InterfaceModeHwGet(devNum,portNum,ifModePtr);

    return rc;
}

GT_STATUS prvCpssDxChPortInterruptDisambiguation
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *extendedModePtr,
    OUT GT_BOOL                 *isSupportedPtr
)
{
    GT_STATUS   rc;
    GT_U32      localPort;

    if((PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)) ||
       (CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily))
    {
        *isSupportedPtr = GT_TRUE;
    }
    else
    {
        *isSupportedPtr = GT_FALSE;
        return GT_OK;
    }


    *extendedModePtr = GT_FALSE;

    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
    {
        /* in Lion2 the ports that are candidates for Extended mode are 9 and 11 */
        if((localPort != 9) && (localPort != 11))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        /* in xCat3 the ports that are candidates for Extended mode are 25 and 27 */
        if((localPort != 25) && (localPort != 27))
        {
            *isSupportedPtr = GT_FALSE;
            return GT_OK;
        }
    }

    rc = cpssDxChPortExtendedModeEnableGet(devNum, portNum,
                                           extendedModePtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortXcat3PortDelete function
* @endinternal
*
* @brief   Power down port and free all allocated for it resources.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] ifMode                   - interface mode according to which power down port
* @param[in] speed                    - port data  according to which power down port
* @param[in] qsgmiiFullDelete         - GT_TRUE - power down QSGMII interface
*                                      GT_FALSE - just free resources of this specific
*                                      member of QSGMII quadruplet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortXcat3PortDelete
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         qsgmiiFullDelete
)
{
    GT_STATUS               rc;         /* return code */
    CPSS_PORT_INTERFACE_MODE_ENT *portIfModePtr;/* interface configured on port now */
    CPSS_PORT_SPEED_ENT          *portSpeedPtr; /* speed configured on port now */

    PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);

    portIfModePtr = &(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portNum));


    /* if on port was configured interface - reset physical layer and free resources */
    if(*portIfModePtr != CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        portSpeedPtr = &(PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portNum));
        rc = prvCpssDxChPortBcat2PortReset(devNum, portNum, *portIfModePtr,
                                           *portSpeedPtr, qsgmiiFullDelete);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        CPSS_TBD_BOOKMARK_XCAT3

/*      should be checked later

        rc = prvCpssDxChPortXcat3ResourcesConfig(devNum, portNum, ifMode,
                                                 speed, GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }
*/

        /* reset port according to new interface */
        rc = prvCpssDxChPortBcat2PortReset(devNum, portNum, ifMode, speed,
                                           qsgmiiFullDelete);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChPortXcat3FcaBusWidthSet function
* @endinternal
*
* @brief   FCA bus width configuration.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] speed                    - port data speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported speed for given port
*/
GT_STATUS prvCpssDxChPortXcat3FcaBusWidthSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS rc; /* return code */
    GT_U32 regAddr; /* register address */
    GT_U32 regValue; /* register value */
    GT_U32 fieldOffset; /* register value */
    GT_U32 fieldLength; /* register value */
    PRV_CPSS_DXCH_PORT_FCA_REGS_ADDR_STC *fcaStcPtr;

    fcaStcPtr = (portNum == CPSS_CPU_PORT_NUM_CNS) ?
         &PRV_DXCH_REG_UNIT_GOP_CPU_FCA_MAC(devNum) :
         &PRV_DXCH_REG_UNIT_GOP_FCA_MAC(devNum, portNum);

    regAddr = fcaStcPtr->FCACtrl;

    fieldOffset = 4;
    fieldLength = 3;

    switch(speed)
    {
        case CPSS_PORT_SPEED_10_E:
        case CPSS_PORT_SPEED_100_E:
        case CPSS_PORT_SPEED_1000_E:
            regValue = 0; /* width 8bit */

            /*  On the stacking interfaces regardless of the port
                speed (1G or 10G) the bus width is ALWAYS 64 bits.
                Thus, you should set it to TypeB (0x1).*/
            if((portNum >= 24) && (portNum <= 27))
            {
                regValue = 1; /* width 64bit */
            }
            break;

        case CPSS_PORT_SPEED_10000_E:
        case CPSS_PORT_SPEED_11800_E:
        case CPSS_PORT_SPEED_2500_E:
        case CPSS_PORT_SPEED_5000_E:
        case CPSS_PORT_SPEED_20000_E:
        case CPSS_PORT_SPEED_22000_E:
            regValue = 1; /* width 64bit */
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc =  prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, regValue);
    return rc;
}


/**
* @internal prvCpssDxChPortXcat3ModeSpeedSet function
* @endinternal
*
* @brief   Configure Interface mode and speed on a specified port and execute
*         on port's serdeses power up sequence; or configure power down on port's
*         serdeses.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portsBmp                 - physical port number (or CPU port)
* @param[in] powerUp                  - serdes power:
*                                      GT_TRUE - up;
*                                      GT_FALSE - down;
* @param[in] ifMode                   - interface mode (related only for serdes power up [powerUp==GT_TRUE])
* @param[in] speed                    - port data  (related only for serdes power up [powerUp==GT_TRUE])
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortXcat3ModeSpeedSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORTS_BMP_STC              portsBmp,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS               rc;         /* return code */
    GT_PHYSICAL_PORT_NUM    portNum;    /* iterator */
    GT_BOOL                 supported;  /* is ifMode/speed supported on port */
    MV_HWS_PORT_STANDARD    portMode;
    GT_BOOL                 lbPort;
    MV_HWS_REF_CLOCK_SUP_VAL refClock;
    MV_HWS_REF_CLOCK_SOURCE  refClockSource;
    CPSS_PORT_INTERFACE_MODE_ENT *portIfModePtr;
    CPSS_PORT_SPEED_ENT          *portSpeedPtr;
    GT_BOOL                  fullConfig; /* because in QSGMII mode one serdes serves four ports
                                                    when one port of the quadruplete configured three
                                                    others configured too, so when application asks
                                                    to configure another port from quadruplete we don't need
                                                    to execute full configuretion of new port */
    PRV_CPSS_DXCH_PORT_STATE_STC      portStateStc;  /* current port state */
    GT_U32  firstInQuadruplet;  /* mac number of first port in quadruplet */
    GT_U32  regAddr;  /* register address */
    PRV_CPSS_DXCH_PP_REGS_ADDR_STC *regsAddrPtr;
    GT_BOOL                         isPortAlreadyConfigured;
    MV_HWS_PORT_INIT_INPUT_PARAMS   portInitInParam;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    GT_U8   sdVecSize;
    GT_U16  *sdVectorPtr;

    if(powerUp == GT_TRUE)
    {
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssDxChSerdesRefClockTranslateCpss2Hws(devNum, &refClock);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        lbPort = (CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E == ifMode) ?
                                                        GT_TRUE : GT_FALSE;
    }
    else
    {/* to prevent warnings */
        portMode = NON_SUP_MODE;
        refClock = MHz_156;
        lbPort = GT_FALSE;
    }

    for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portsBmp, portNum))
        {
            PRV_CPSS_DXCH_PHY_PORT_OR_CPU_PORT_CHECK_MAC(devNum, portNum);
        }
        else
        {
            continue;
        }

        if ((PRV_CPSS_GE_PORT_GE_ONLY_E == PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum, portNum))
                && (portNum < 24) && (CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E == ifMode)
                && (portNum != (portNum & 0xFFFFFFFC)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        portIfModePtr = &(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portNum));
        portSpeedPtr = &(PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portNum));

        rc = prvCpssDxChPortIsAlreadyConfigured(devNum, portNum, powerUp, *portIfModePtr, *portSpeedPtr, ifMode, speed, &isPortAlreadyConfigured);
        if (rc != GT_OK)
        {
            return rc;
        }
        if(isPortAlreadyConfigured)
        {
            continue;
        }

        rc = prvCpssDxChPortSerdesFreeCheck(devNum,portNum,ifMode,speed);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }


        if(powerUp == GT_FALSE)
        {
            /* we remove the portFs, so need to use current interface and speed
             * if configured, not the values from the caller */
            if((CPSS_PORT_INTERFACE_MODE_NA_E != *portIfModePtr) &&
                                        (CPSS_PORT_SPEED_NA_E != *portSpeedPtr))
            {
                ifMode = *portIfModePtr;
                speed =  *portSpeedPtr;
            }
            else
            {
                continue; /* port is already power down , go to next port */
            }
        }

        rc = prvCpssCommonPortInterfaceSpeedGet(devNum, portNum,
                                                              ifMode, speed,
                                                              &supported);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        if(!supported)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssDxChPortStateDisableAndGet(devNum,portNum,&portStateStc);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        fullConfig = GT_TRUE;
        if(CPSS_PORT_INTERFACE_MODE_QSGMII_E == ifMode)
        {
            rc = bcat2QsgmiiConfig(devNum, portNum, powerUp, &fullConfig);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }

        rc = prvCpssDxChPortXcat3PortDelete(devNum, portNum, ifMode, speed,
                                            fullConfig);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        if(!powerUp)
        {
            rc = prvCpssDxChPortStateRestore(devNum, portNum, &portStateStc);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }

        /* save new interface mode in DB */
        *portIfModePtr = CPSS_PORT_INTERFACE_MODE_NA_E;
        *portSpeedPtr = CPSS_PORT_SPEED_NA_E;

        if(!powerUp)
        {
            continue;
        }

        CPSS_TBD_BOOKMARK_XCAT3

        /* For setting RXAUI port interface first port in quadruplet also
        should be already configurated with RXAUI port interface */
        if(CPSS_PORT_INTERFACE_MODE_RXAUI_E == ifMode)
        {
            firstInQuadruplet = portNum & 0xFFFFFFFC;
            if(firstInQuadruplet != portNum)
            {
                CPSS_PORT_INTERFACE_MODE_ENT ifMode;
                rc = internal_cpssDxChPortInterfaceModeGet(devNum, firstInQuadruplet, &ifMode);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
                if(CPSS_PORT_INTERFACE_MODE_RXAUI_E != ifMode)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }
            }
        }

/*
 *       should be checked later
 *
 *       rc = prvCpssDxChBcat2PortResourcesConfig(devNum, portNum, ifMode, speed, GT_TRUE);
 *       if (rc != GT_OK)
 *       {
 *           return rc;
 *       }
 */
        /* Set FCA interface width */
        rc = prvCpssDxChPortXcat3FcaBusWidthSet(devNum, portNum, speed);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /* Set Count External FC En enable */
        if(PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum,portNum) == PRV_CPSS_GE_PORT_GE_ONLY_E)
        {
            regsAddrPtr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum);
            regAddr = regsAddrPtr->macRegs.perPortRegs[portNum].macRegsPerType[PRV_CPSS_PORT_GE_E].macCtrl4;
            rc =  prvCpssHwPpSetRegField(devNum, regAddr, 8, 1, 1);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }

        if(fullConfig)
        {
            if(CPSS_PORT_INTERFACE_MODE_QSGMII_E == ifMode)
            {/* provide to HWS first port in quadruplet and it will configure
                all other ports if needed */
                firstInQuadruplet = portNum & 0xFFFFFFFC;
            }
            else
            {
                firstInQuadruplet = portNum;
            }
            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E)
            {
                refClockSource = PRIMARY_LINE_SRC;
            }
            else
            {
                switch(speed)
                {
                    case CPSS_PORT_SPEED_10_E:
                    case CPSS_PORT_SPEED_100_E:
                    case CPSS_PORT_SPEED_1000_E:
                    case CPSS_PORT_SPEED_2500_E:
                        refClockSource = PRIMARY_LINE_SRC;
                        break;

                    case CPSS_PORT_SPEED_5000_E:
                    case CPSS_PORT_SPEED_10000_E:
                    case CPSS_PORT_SPEED_11800_E:
                    case CPSS_PORT_SPEED_20000_E:
                    case CPSS_PORT_SPEED_22000_E:

                        refClockSource = SECONDARY_LINE_SRC;
                        break;

                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }

            if(PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portRefClock.enableOverride == GT_TRUE)
            {
                switch(PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portNum].portRefClock.portRefClockSource)
                {
                    case CPSS_PORT_REF_CLOCK_SOURCE_PRIMARY_E:
                        refClockSource = PRIMARY_LINE_SRC;
                        break;
                    case CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E:
                        refClockSource = SECONDARY_LINE_SRC;
                        break;
                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }
            }

            rc = hwsPortModeParamsGetToBuffer(devNum, 0, firstInQuadruplet, portMode, &curPortParams);
            if (GT_OK != rc)
            {
                return rc;
            }
            sdVecSize   = curPortParams.numOfActLanes;
            sdVectorPtr = curPortParams.activeLanesList;

            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortFlavorInit(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], lbPort[%d], refClock[%d], refClockSource[%d]})", devNum, 0, firstInQuadruplet, portMode, lbPort, refClock, refClockSource);
            cpssOsMemSet(&portInitInParam, 0,sizeof(portInitInParam));
            portInitInParam.lbPort = lbPort;
            portInitInParam.refClock = refClock;
            portInitInParam.refClockSource = refClockSource;
            rc = mvHwsPortFlavorInit(devNum, 0, firstInQuadruplet, portMode, &portInitInParam);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            /* set the Polarity values on Serdeses if SW DB values initialized */
            rc = prvCpssDxChPortSerdesPolaritySet(devNum, 0, sdVectorPtr, sdVecSize);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortSerdesPolaritySet, portNum = %d\n", portNum);
            }
        }

        if((CPSS_PORT_SPEED_10_E == speed) || (CPSS_PORT_SPEED_100_E == speed))
        {/* HWS doesn't support 10/100M, so after 1G configured by HWS, fix speed here */
            PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portNum,&regAddr);
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 6, 1, 0);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = prvCpssHwPpSetRegField(devNum, regAddr, 5, 1, (CPSS_PORT_SPEED_10_E == speed) ? 0 : 1);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else if ((GT_FALSE == fullConfig) && (CPSS_PORT_SPEED_1000_E == speed))
        {
            /* 1G support is done by HWS only during initial creation through  */
            /* "mvHwsPortFlavorInit" (see usage above). However when switching */
            /* back to 1G from 10M/100M the following should be reconfigured.  */
            PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portNum,&regAddr);
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 6, 1, 1);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        prvCpssDxChPortTypeSet(devNum, portNum, ifMode, speed);

        /* update addresses of mac registers accordingly to used MAC GE/XLG/etc. */
        rc = prvCpssDxChHwXcat3RegAddrPortMacUpdate(devNum, portNum);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssDxChPortImplementWaSGMII2500(devNum, portNum, ifMode, speed);
        if (rc != GT_OK)
        {
             return rc;
        }

        /* save new interface mode in DB */
        *portIfModePtr = ifMode;
        *portSpeedPtr = speed;

        /*Creating Port LoopBack*/
        if(lbPort)
        {
            rc = prvCpssDxChPortSerdesLoopbackActivate (devNum, portNum, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }

        rc = prvCpssDxChPortStateRestore(devNum, portNum, &portStateStc);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

    } /* for(portNum = 0; */

    return GT_OK;
}


static GT_STATUS internal_cpssDxChPortRefClockSourceOverrideEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         overrideEnable,
    IN  CPSS_PORT_REF_CLOCK_SOURCE_ENT  refClockSource
)
{
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum, portMacNum);

    if(overrideEnable == GT_FALSE)
    {
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portRefClock.enableOverride = overrideEnable;
        /* invalidate entry */
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portRefClock.portRefClockSource = CPSS_PORT_REF_CLOCK_SOURCE_LAST_E;
    }
    else
    {
        if((refClockSource == CPSS_PORT_REF_CLOCK_SOURCE_PRIMARY_E) || (refClockSource == CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E))
        {
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portRefClock.enableOverride = overrideEnable;
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portRefClock.portRefClockSource = refClockSource;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortRefClockSourceOverrideEnableSet function
* @endinternal
*
* @brief   Enables/disables reference clock source override for port create.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] overrideEnable           - override
*                                      GT_TRUE - use preconfigured refClock source;
*                                      GT_FALSE - use default refClock source;
* @param[in] refClockSource           - reference clock source.
*                                      Not relevant when overrideEnable is false.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, refClockSource
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortRefClockSourceOverrideEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         overrideEnable,
    IN  CPSS_PORT_REF_CLOCK_SOURCE_ENT  refClockSource
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortRefClockSourceOverrideEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, overrideEnable, refClockSource));

    rc = internal_cpssDxChPortRefClockSourceOverrideEnableSet(devNum, portNum, overrideEnable, refClockSource);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, overrideEnable, refClockSource));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

static GT_STATUS internal_cpssDxChPortRefClockSourceOverrideEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *overrideEnablePtr,
    OUT CPSS_PORT_REF_CLOCK_SOURCE_ENT  *refClockSourcePtr
)
{
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(overrideEnablePtr);
    CPSS_NULL_PTR_CHECK_MAC(refClockSourcePtr);

    *overrideEnablePtr = PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portRefClock.enableOverride;
    *refClockSourcePtr = PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portRefClock.portRefClockSource;

    return GT_OK;
}

/**
* @internal cpssDxChPortRefClockSourceOverrideEnableGet function
* @endinternal
*
* @brief   Gets status of reference clock source override for port create.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] overrideEnablePtr        - (pointer to) override enable status
* @param[out] refClockSourcePtr        - (pointer to) reference clock source.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortRefClockSourceOverrideEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *overrideEnablePtr,
    OUT CPSS_PORT_REF_CLOCK_SOURCE_ENT  *refClockSourcePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortRefClockSourceOverrideEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, overrideEnablePtr, refClockSourcePtr));

    rc = internal_cpssDxChPortRefClockSourceOverrideEnableGet(devNum, portNum, overrideEnablePtr, refClockSourcePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, overrideEnablePtr, refClockSourcePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


#if 0
/**
* @internal prvCpssDxChPortCaelumTrafficFlushCheck function
* @endinternal
*
* @brief   Check all traffic to the port was flushed.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_TIMEOUT               - when not all traffic was flushed.
*/
GT_STATUS prvCpssDxChPortCaelumTrafficFlushCheck
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS rc;               /* return code */
    GT_U32 regAddr;             /* register address */
    GT_U32 regValue;            /* register value */
    GT_U32 regValue1;           /* register value */
    GT_U32 fieldOffset;         /* register offset value */
    GT_U32 fieldLen;            /* register length value */
    GT_U32 tryCounter;          /* check counter */

    /*
        1. Read Port<%n>DescCounter in the TXQ and wait for it to reach 0. This counter
           indicates how many descriptors are allocated for the port in the TXQ queues.
    */
    rc = prvCpssDxChPortCaelumRegAddrGet(devNum,portNum,
                                         PRV_CAELUM_REG_FIELDS_TXQ_Q_PORT_DESC_COUNTER_E,
                                         &regAddr, &fieldOffset, &fieldLen);
    if(GT_OK != rc)
    {
        return rc;
    }

    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLen, &regValue);
    if(GT_OK != rc)
    {
        return rc;
    }
    tryCounter = 100;
    while(regValue != 0)
    {
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLen, &regValue);
        if(GT_OK != rc)
        {
            return rc;
        }
        tryCounter--;
    }
    /* in case of error - return TIMEOUT value */
    if(tryCounter == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
    }

    /*
        2. Read TxDMA Port <%n> Credit Counter register in the TXQ and wait for it to reach
           the corresponding configured descriptor credits threshold in the
           TXDMA (Descriptor Credits SCDMA %p).
    */
    rc = prvCpssDxChPortCaelumRegAddrGet(devNum,portNum,
                                         PRV_CAELUM_REG_FIELDS_TXDMA_DESC_CREDITS_SCDMA_E,
                                         &regAddr, &fieldOffset, &fieldLen);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLen, &regValue);
    if(GT_OK != rc)
    {
        return rc;
    }

    rc = prvCpssDxChPortCaelumRegAddrGet(devNum,portNum,
                                         PRV_CAELUM_REG_FIELDS_TXQ_DQ_TXDMA_PORT_CREDIT_COUNTER_E,
                                         &regAddr, &fieldOffset, &fieldLen);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLen, &regValue1);
    if(GT_OK != rc)
    {
        return rc;
    }

    tryCounter = 100;
    while(regValue != regValue1)
    {
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLen, &regValue1);
        if(GT_OK != rc)
        {
            return rc;
        }
        tryCounter--;
    }
    /* in case of error - return TIMEOUT value */
    if(tryCounter == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
    }

    /*
        3. In the TXDMA regfile - read the TXFIFO credits counters status and wait for it
           to reach zero. There is a separate counter for headers (scdma_%p_header_txfifo_counter)
           and payloads (scdma_%p_payload_txfifo_counter). These counters count the amount of
           data that is inside the TXDMA/TXFIFO on its way to the port.
    */
    rc = prvCpssDxChPortCaelumRegAddrGet(devNum,portNum,
                                         PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_HEADER_TXFIFO_COUNTER_E,
                                         &regAddr, &fieldOffset, &fieldLen);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLen, &regValue);
    if(GT_OK != rc)
    {
        return rc;
    }
    tryCounter = 100;
    while(regValue != 0)
    {
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLen, &regValue);
        if(GT_OK != rc)
        {
            return rc;
        }
        tryCounter--;
    }
    /* in case of error - return TIMEOUT value */
    if(tryCounter == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
    }


    rc = prvCpssDxChPortCaelumRegAddrGet(devNum,portNum,
                                         PRV_CAELUM_REG_FIELDS_TXDMA_SCDMA_PAYLOAD_TXFIFO_COUNTER_E,
                                         &regAddr, &fieldOffset, &fieldLen);
    if(GT_OK != rc)
    {
        return rc;
    }
    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLen, &regValue);
    if(GT_OK != rc)
    {
        return rc;
    }
    tryCounter = 100;
    while(regValue != 0)
    {
        rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLen, &regValue);
        if(GT_OK != rc)
        {
            return rc;
        }
        tryCounter--;
    }
    /* in case of error - return TIMEOUT value */
    if(tryCounter == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}
#endif

/**
* @internal prvCpssDxChPortTxShaper29_09gConfig function
* @endinternal
*
* @brief   Set the TxQ Shaper configuration on specific port for port Speed 29.09G:
*         - Disable the High speed port
*         - Set porifile 3 value to 8
*         - Limit the Shaper burst size to max value 4KB and maxRate 25G
*         - Set the Token Bucket Baseline to 0x3FFFC0
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port MAC number (not CPU port)
* @param[in] powerUp                  - serdes power:
*                                      GT_TRUE - up;
*                                      GT_FALSE - down;
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortTxShaper29_09gConfig
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 powerUp
)
{
    GT_PORT_NUM highSpeedPortNum;
    GT_U32      highSpeedPortIdx;
    GT_U8       numOfTotalHighSpeedPorts=0;
    GT_U32      highSpeedPortConfig=0;
    GT_U16      bustSize;
    GT_U32      maxRate;
    GT_BOOL     isEnabled;
    GT_U32      txqPortNum; /* TXQ port number */
    GT_U8       maskValue;  /* requested mask value */
    GT_U32      numOfTxqUnits;
    GT_STATUS   rc;
    CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT shaperProfile;
    CPSS_DXCH_PA_UNIT_ENT unitIdx;
    GT_BOOL    isHighSpeedPort;

    PRV_CPSS_DXCH_PORT_CHECK_MAC(devNum, portNum);

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);

    numOfTxqUnits = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.numTxqDq;

    if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
    {
        numOfTotalHighSpeedPorts = 8;
    }
    else if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) &&
             (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E))
    {
        numOfTotalHighSpeedPorts = 6;
    }

    isHighSpeedPort = GT_FALSE;

    /* find the highSpeedPortNum */
    for (highSpeedPortIdx = 0; highSpeedPortIdx < numOfTotalHighSpeedPorts; highSpeedPortIdx++)
    {
        rc = prvCpssDxChPortTxQUnitDQHighSpeedPortGet(devNum, highSpeedPortIdx, /*OUT*/&isEnabled, &highSpeedPortNum);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChPortTxQUnitDQHighSpeedPortGet, portNum = %d\n", highSpeedPortIdx);
        }

        if (highSpeedPortNum == portNum)
        {
            highSpeedPortConfig = highSpeedPortIdx;
            isHighSpeedPort = GT_TRUE;
        }
    }

    if (powerUp == GT_TRUE)
    {
        if (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {

            shaperProfile = CPSS_PORT_TX_SCHEDULER_PROFILE_3_E;
            if (isHighSpeedPort != GT_FALSE)
            {
                /* Disable High speed port */
                rc = prvCpssDxChPortTxQUnitDQHighSpeedPortDisable(devNum, highSpeedPortConfig);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChPortTxQUnitDQHighSpeedPortDisable, highSpeedPortConfig = %d\n", highSpeedPortConfig);
                }
            }

            /* set the requested mask value to 5 or 8 clock-cycles according to the device core-clock */
            if (PRV_CPSS_PP_MAC(devNum)->coreClock == 365)
            {
                maskValue = 5;
            }
            else if ((PRV_CPSS_PP_MAC(devNum)->coreClock == 480) || (PRV_CPSS_PP_MAC(devNum)->coreClock == 525))
            {
                maskValue = 8;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(
                    GT_NOT_SUPPORTED,
                    "error in prvCpssDxChPortTxShaper29_09gConfig: PRV_CPSS_PP_MAC(devNum)->coreClock=%d\n",
                    PRV_CPSS_PP_MAC(devNum)->coreClock);
            }

            /* for mask index "customized"==#2 set the requested mask value */
            rc = prvCpssDxChTxRequestMaskSet(
                devNum, PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_IDX_CUSTOMIZED_PORT_CNS, maskValue);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in cpssDxChPortTxShaperEnableSet\n");
            }
            /* Set the Mask Selector #3 on port */
            rc = prvCpssDxChPortTxQUnitDQPortRequestMaskSelectorSet(
                devNum, txqPortNum, PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_IDX_CUSTOMIZED_PORT_CNS);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            shaperProfile = CPSS_PORT_TX_SCHEDULER_PROFILE_1_E;
            for (unitIdx = CPSS_DXCH_PA_UNIT_TXQ_0_E;
                  (unitIdx < (CPSS_DXCH_PA_UNIT_ENT)(CPSS_DXCH_PA_UNIT_TXQ_0_E + numOfTxqUnits));
                  unitIdx++)
            {
                 rc = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSet(devNum,unitIdx,GT_FALSE);
                 if (GT_OK != rc)
                 {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Error in cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSet, portNum = %d\n", portNum);
                }
            }
        }
        /* Enable the rate Shaping on port */
        rc = cpssDxChPortTxShaperEnableSet(devNum, portNum, GT_TRUE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in cpssDxChPortTxShaperEnableSet, portNum = %d\n", portNum);
        }

        /* Set the Token Bucket Baseline */
        rc = cpssDxChPortTxShaperBaselineSet(devNum, 0x3FFFC0);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in cpssDxChPortTxShaperBaselineSet\n");
        }

        bustSize = 1025;
        maxRate  = 25000000;
        /* according to the Token Bucket Baseline: Limit the Shaper burst size to 1025 and maxRate 25G */
        rc = cpssDxChPortTxShaperProfileSet(devNum, portNum, bustSize, &maxRate);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in cpssDxChPortTxShaperProfileSet, portNum = %d\n", portNum);
        }

        /* for port Shaping rate limiting: Adding 20bytes to the packet's length */
        rc = cpssDxChPortTxByteCountChangeValueSet(devNum, portNum, 20);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in cpssDxChPortIpgSet, portNum = %d\n", portNum);
        }

        /* Enable profile #2 Byte Count Change of the packet length */
        rc = cpssDxChPortProfileTxByteCountChangeEnableSet(devNum, shaperProfile,
                                                           CPSS_DXCH_PORT_TX_BC_CHANGE_ENABLE_SHAPER_ONLY_E, CPSS_ADJUST_OPERATION_ADD_E);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in cpssDxChPortProfileTxByteCountChangeEnableSet, portNum = %d\n", portNum);
        }

        /* Bind the port to scheduler profile #2 */
        rc = cpssDxChPortTxBindPortToSchedulerProfileSet(devNum, portNum, shaperProfile);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in cpssDxChPortTxBindPortToSchedulerProfileSet, portNum = %d\n", portNum);
        }
    }
    else
    { /* powerDown */
        if (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) )
        {

            shaperProfile = CPSS_PORT_TX_SCHEDULER_PROFILE_3_E;
            /* Enable High speed port */

            if (isHighSpeedPort != GT_FALSE)
            {
                /* when disabling the 2909 we return the high speed port to default which is disable
                   rc = prvCpssDxChPortTxQUnitDQHighSpeedPortEnable(devNum, highSpeedPortConfig, portNum);*/
                rc = prvCpssDxChPortTxQUnitDQHighSpeedPortDisable(devNum, highSpeedPortConfig);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChPortTxQUnitDQHighSpeedPortDisable, highSpeedPortConfig = %d\n", highSpeedPortConfig);
                }
            }

            /* Restore to default the Mask Selector #0 on port */
            rc = prvCpssDxChPortTxQUnitDQPortRequestMaskSelectorSet(
                devNum, txqPortNum, PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_IDX_REGULAR_PORT_CNS);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            shaperProfile = CPSS_PORT_TX_SCHEDULER_PROFILE_1_E;
            for (unitIdx = CPSS_DXCH_PA_UNIT_TXQ_0_E;
                  (unitIdx < (CPSS_DXCH_PA_UNIT_ENT)(CPSS_DXCH_PA_UNIT_TXQ_0_E + numOfTxqUnits));
                  unitIdx++)
            {
                 rc = cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSet(devNum,unitIdx,GT_TRUE);
                 if (GT_OK != rc)
                 {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Error in cpssDxChPortDynamicPizzaArbiterIfWorkConservingModeSet, portNum = %d\n", portNum);
                 }
            }
        }

        /* Disable the rate Shaping on port */
        rc = cpssDxChPortTxShaperEnableSet(devNum, portNum, GT_FALSE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in cpssDxChPortTxShaperEnableSet, portNum = %d\n", portNum);
        }

        maxRate = 17412;
        /* Restore the default value of Shaper configuration */
        rc = cpssDxChPortTxShaperProfileSet(devNum, portNum, 0x152, &maxRate);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in cpssDxChPortTxShaperProfileSet, portNum = %d\n", portNum);
        }

        /* Disable the profile Byte Count Change of the packet length */
        rc = cpssDxChPortProfileTxByteCountChangeEnableSet(devNum, shaperProfile,
                                                           CPSS_DXCH_PORT_TX_BC_CHANGE_DISABLE_ALL_E, CPSS_ADJUST_OPERATION_ADD_E);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in cpssDxChPortIpgSet, portNum = %d\n", portNum);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortCaelumPortDelete function
* @endinternal
*
* @brief   Power down port and free all allocated for it resources.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X.
*         Bobcat3; Aldrin2, Falcon, AC5P, AC5X
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] ifMode                   - interface mode according to which power down port
* @param[in] speed                    - port data  according to which power down port
* @param[in] qsgmiiFullDelete         - GT_TRUE - power down QSGMII interface
*                                      GT_FALSE - just free resources of this specific
*                                      member of QSGMII quadruplet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortCaelumPortDelete
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         qsgmiiFullDelete
)
{
    GT_STATUS               rc;         /* return code */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */
    CPSS_PORT_INTERFACE_MODE_ENT *portIfModePtr;/* interface configured on port now */
    CPSS_PORT_SPEED_ENT          *portSpeedPtr; /* speed configured on port now */

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                 portNum,
                                                                 portMacMap);

    portIfModePtr = &(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap));
    portSpeedPtr = &(PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap));

    /* if on port was configured interface - reset physical layer and free resources */
    if((*portIfModePtr != CPSS_PORT_INTERFACE_MODE_NA_E) && (*portIfModePtr != CPSS_PORT_INTERFACE_MODE_NA_HCD_E))
    {
        rc = prvCpssDxChPortBcat2PortReset(devNum, portNum, *portIfModePtr,
                                           *portSpeedPtr, qsgmiiFullDelete);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChPortCaelumPortDelete: error in prvCpssDxChPortBcat2PortReset, portNum = %d\n", portNum);
        }

        if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
        {
            rc = prvCpssDxChPortDpIronmanPortDown(devNum, portNum);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(
                    rc,"error in prvCpssDxChPortDpIronmanPortDown, portNum = %d\n",
                    portNum);
            }
        }
        else if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            rc = prvCpssDxChTxPortSpeedPizzaResourcesPortDown(devNum, portNum);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(
                    rc,"error in prvCpssDxChTxPortSpeedPizzaResourcesHawkPortDown, portNum = %d\n",
                    portNum);
            }
        }
        else if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            rc = prvCpssDxChTxPortSpeedPizzaResourcesSet(devNum, portNum, *portIfModePtr, *portSpeedPtr, GT_FALSE);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prevCpssDxChTxPortSpeedResourcesSet, portNum = %d\n", portNum);
            }
            rc = prvCpssDxChPortPacketBufferGpcChannelSpeedSet(
                devNum, portNum, CPSS_PORT_SPEED_NA_E);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(
                    rc,"error in prvCpssDxChPortPacketBufferGpcChannelSpeedSet, portNum = %d\n", portNum);
            }
        }
        else
        {
            /* Set PTP interface width */
            rc = prvCpssDxChPortBcat2PtpInterfaceWidthSelect(devNum, portNum, CPSS_PORT_SPEED_10_E);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChCaelumPortResourcesRelease: error in prvCpssDxChPortBcat2PtpInterfaceWidthSelect, portNum = %d\n", portNum);
            }

            rc = prvCpssDxChCaelumPortResourcesRelease(devNum, portNum);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChPortCaelumPortDelete: error in prvCpssDxChCaelumPortResourcesRelease, portNum = %d\n", portNum);
            }

            rc = prvCpssDxChPortPizzaArbiterIfDelete(devNum, portNum);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChCaelumPortResourcesRelease: error in prvCpssDxChPortPizzaArbiterIfDelete, portNum = %d\n", portNum);
            }


        if ((*portIfModePtr == CPSS_PORT_INTERFACE_MODE_SR_LR4_E) && (*portSpeedPtr == CPSS_PORT_SPEED_29090_E))
            {
                rc = prvCpssDxChPortTxShaper29_09gConfig(devNum, portNum, GT_FALSE);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChPortTxShaper29_09gConfig, portNum = %d\n", portNum);
                }
            }
            else /* prvCpssDxChPortTxShaper29_09gConfig treats High Speed ports */
            if (PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum) &&
                (*portSpeedPtr != CPSS_PORT_SPEED_NA_E))
            {
                /* Remove High Speed port */
                rc = prvCpssDxChPortHighSpeedPortSet(
                    devNum, PRV_CPSS_DXCH_PORT_HIGH_SPEED_OPERATION_REMOVE_E, 1, &portNum);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChPortHighSpeedPortSet, portNum = %d\n", portNum);
                }
            }
        }
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE)
    {
        if(!((*portIfModePtr == ifMode) && (*portSpeedPtr == speed)))
        {
            /* reset port according to new interface */
            rc = prvCpssDxChPortBcat2PortReset(devNum, portNum, ifMode, speed,
                                               qsgmiiFullDelete);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChCaelumPortResourcesRelease: error in prvCpssDxChPortBcat2PortReset, portNum = %d\n", portNum);
            }
        }
    }

    return GT_OK;
}

GT_STATUS prvCpssDxChPortRefClockUpdate
(
    IN  GT_U8                           devNum,
    MV_HWS_PORT_STANDARD                portMode,
    IN  GT_U16                          *serdesArrPtr,
    IN  GT_U8                           serdesSize,
    IN  MV_HWS_REF_CLOCK_SOURCE         refClockSource,
    OUT MV_HWS_REF_CLOCK_SUP_VAL        *refClockPtr
)
{
    GT_STATUS                       rc;
    MV_HWS_CPLL_OUTPUT_FREQUENCY    cpllOutFreq;
    GT_U32                          i;
    GT_U32                          dividerValue = 2;
    GT_U32                          regAddr;


    CPSS_NULL_PTR_CHECK_MAC(serdesArrPtr);

    if(serdesSize == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    CPSS_TBD_BOOKMARK_PHOENIX
    if(PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
        return GT_OK;
    }


    cpllOutFreq = prvCpssDxChPortCpllConfigArr[devNum][serdesArrPtr[0]].outFreq;

    if((PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum) == GT_TRUE) ||
       (PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum) == GT_TRUE)  ||
       (PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum) == GT_TRUE))
    {
        if((refClockSource == SECONDARY_LINE_SRC) && (prvCpssDxChPortCpllConfigArr[devNum][serdesArrPtr[0]].valid == GT_TRUE))
        {   /* CPLL as ref clock source */
            switch(cpllOutFreq)
            {
                case MV_HWS_156MHz_OUT:
                    *refClockPtr = MHz_156;
                    break;

                case MV_HWS_78MHz_OUT:
                    *refClockPtr = MHz_78;
                    break;

                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
    }

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if(prvCpssDxChPortCpllConfigArr[devNum][serdesArrPtr[0]].valid == GT_TRUE)
        {
            /**************************************************************************/
            /*  Dividers values are calculated according to CPLL OUT FREQ = 156.25Mhz  */
            /**************************************************************************/

            if(cpllOutFreq != MV_HWS_156MHz_OUT)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            if(refClockSource == PRIMARY_LINE_SRC)
            {   /* CPLL as ref clock source */
                *refClockPtr = MHz_156;
                dividerValue = 1;        /*div_by_1 (156.25 / 1 = 156.25) --> regValue = 1*/
            }

            for(i = 0; i < serdesSize; i++)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesArrPtr[i]].serdesExternalReg1;
                rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 11, 2, dividerValue);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }

        }
    }
    else if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
    {
        if(prvCpssDxChPortCpllConfigArr[devNum][serdesArrPtr[0]].valid == GT_TRUE)
        {

            if(refClockSource == PRIMARY_LINE_SRC)
            {
                /**************************************************************************/
                /*  Dividers values are calculated according to cpllOutFreq               */
                /**************************************************************************/
                if(cpllOutFreq == MV_HWS_312MHz_OUT)
                {   /* CPLL as ref clock source */
                    switch(portMode)
                    {
                        case _5GBaseR:
                        case _12_5GBase_KR:
                        case _25GBase_KR2:
                        case _50GBase_KR4:
                            *refClockPtr = MHz_78;
                            dividerValue = 3;       /*div_by_4 (312.5 / 4 = 78.125) --> regValue = 3*/
                            break;

                        case _107GBase_KR4:
                            *refClockPtr = MHz_312;
                            dividerValue = 1;       /*div_by_1 (312.5 / 1 = 312.5) --> regValue = 1*/
                            break;

                        default:
                            *refClockPtr = MHz_156;
                            dividerValue = 2;       /*div_by_2 (312.5 / 2 = 156.25) --> regValue = 2*/
                            break;
                    }
                }
                else if(cpllOutFreq == MV_HWS_164MHz_OUT && portMode == _52_5GBase_KR2)
                {
                    *refClockPtr = MHz_164;
                    dividerValue = 1;       /*div_by_1 (164.24 / 1 = 164.24) --> regValue = 1*/
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }
            }
            else /* SECONDARY */
            {
                switch(portMode)
                {
                    case _5GBaseR:
                    case _12_5GBase_KR:
                    case _25GBase_KR2:
                    case _50GBase_KR4:
                        *refClockPtr = MHz_78;
                        dividerValue = 2;       /*div_by_2 (156.25 / 2 = 78.125) --> regValue = 2*/
                        break;
                    default:
                        *refClockPtr = MHz_156;
                        dividerValue = 1;               /*div_by_1 (156.25 / 1 = 156.25) --> regValue = 1*/
                        break;
                }
            }

            for(i = 0; i < serdesSize; i++)
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesArrPtr[i]].serdesExternalReg1;
                rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 11, 2, dividerValue);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
        }
    }
    return GT_OK;
}


/**
* @internal prvCpssDxChIsCgUnitInUse function
* @endinternal
*
* @brief   Check if given pair portNum and portMode use CG MAC
*
* @note   APPLICABLE DEVICES:       Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port MAC number (not CPU port)
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - interface speed
*
* @param[out] isCgUnitInUsePtr         - GT_TRUE ? GC MAC in use
*                                      GT_FALSE ? GC MAC not in use
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if one of input parameters wrong
* @retval GT_BAD_PTR               - if supportedPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChIsCgUnitInUse
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    OUT GT_BOOL                         *isCgUnitInUsePtr
)
{
    GT_STATUS rc;
    GT_U32 portMacNum;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_STANDARD    portMode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_XCAT3_E | CPSS_AC5_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(isCgUnitInUsePtr);

    *isCgUnitInUsePtr = GT_FALSE;

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        return GT_OK;
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if((rc != GT_OK) || (portMode == NON_SUP_MODE))
    {
        return GT_OK;
    }


    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Only "real" CG unit port modes (like _100GBase_KR4, _100GBase_SR4, _107GBase_KR4) should configure its features */
    if((curPortParams.portMacType == CGMAC) && (curPortParams.portPcsType == CGPCS))
    {
        *isCgUnitInUsePtr = GT_TRUE;
    }

    return GT_OK;
}

/* PRV_CPSS_DXCH_ERRATA_ALDRIN_SDWRR_WA_E treatment */

/**
* @internal prvCpssDxChPortAldrinSdwrrWa function
* @endinternal
*
* @brief   Implements WA for not accurate packet mode SDWRR for slow ports
*
* @note   APPLICABLE DEVICES:      Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Falcon;  AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] powerUp                  - power up/down GT_TRUE - up; GT_FALSE - down;
* @param[in] speed                    - port speed enum element
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortAldrinSdwrrWa
(
    IN GT_U8                       devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_BOOL                    powerUp,
    IN  CPSS_PORT_SPEED_ENT        speed
)
{
    GT_STATUS rc;              /* return status */
    GT_U32  txqPortNum;        /* TXQ port number */
    GT_U32  shaperMaskIndex;   /* shaper Mask Index */
    GT_U32  speedPnMbPerSec;   /* speed in MB per second */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if ((PRV_CPSS_SIP_5_16_CHECK_MAC(devNum) == GT_FALSE) ||
        (PRV_CPSS_SIP_6_CHECK_MAC(devNum) != GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);

    rc = prvCpssDxChPortTxQUnitDQPortRequestMaskSelectorGet(
        devNum, txqPortNum, &shaperMaskIndex);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if ((powerUp != GT_FALSE)
        && (shaperMaskIndex != PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_IDX_REGULAR_PORT_CNS))
    {
        /* already configured another WA - bypass */
        return GT_OK;
    }

    if ((powerUp == GT_FALSE)
        && (shaperMaskIndex == PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_IDX_REGULAR_PORT_CNS))
    {
        /* the needed HW state obtained - bypass */
        return GT_OK;
    }

    /* slow is <= 20G */
    speedPnMbPerSec = prvCpssCommonPortSpeedEnumToMbPerSecConvert(speed);
    if ((speedPnMbPerSec <= 20000) && (powerUp != GT_FALSE))
    {
        shaperMaskIndex = PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_IDX_LOW_SPEED_PORT_CNS;
    }
    else
    {
        shaperMaskIndex = PRV_CPSS_DXCH_TXQDQ_PORT_REQUEST_MASK_IDX_REGULAR_PORT_CNS;
    }

    rc = prvCpssDxChPortTxQUnitDQPortRequestMaskSelectorSet(
            devNum, txqPortNum, shaperMaskIndex);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortCaelumModeSpeedSet function
* @endinternal
*
* @brief   Configure Interface mode and speed on a specified port and execute
*         on port's serdeses power up sequence; or configure power down on port's
*         serdeses.
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] devNum                   - physical device number
* @param[in] portsBmp                 - physical port number (or CPU port)
* @param[in] powerUp                  - serdes power:
*                                      GT_TRUE - up;
*                                      GT_FALSE - down;
* @param[in] ifMode                   - interface mode (related only for serdes power up [powerUp==GT_TRUE])
* @param[in] speed                    - port data  (related only for serdes power up [powerUp==GT_TRUE])
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChPortCaelumModeSpeedSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORTS_BMP_STC              portsBmp,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS               rc;         /* return code */
    GT_PHYSICAL_PORT_NUM    portNum;    /* iterator */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */
    GT_U32                  realMacNum; /* number of mac mapped to this physical port */
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;/* Get PHY MAC object pointer */
    GT_BOOL                 supported;  /* is ifMode/speed supported on port */
    MV_HWS_PORT_STANDARD    portMode;
    GT_BOOL                 lbPort;
    MV_HWS_REF_CLOCK_SUP_VAL refClock;
    MV_HWS_REF_CLOCK_SOURCE  refClockSource;
    CPSS_PORT_INTERFACE_MODE_ENT *portIfModePtr;
    CPSS_PORT_SPEED_ENT          *portSpeedPtr;
    GT_BOOL                 fullConfig; /* for QSGMII mode:
                                                    one serdes serves four ports, when one port of
                                                    the quadruplete configured three others configured too,
                                                    so when application asks to configure another port from
                                                    quadruplete we don't need to execute full configuretion of new port
                                           for MLG mode:
                                                    four Serdeses serves four MLG ports, when one port of the quadruplete
                                                    configured, three others ports share the same four Serdeses.
                                                    So when application asks to configure another port from quadruplete,
                                                    we don't need to execute full configuretion of new port */
    GT_U8                   configuredPortsBmp = 0; /*for MLG mode:
                                                    the first port in the quadruplete should be
                                                    configured first on powerUp and last on powerDown */
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    PRV_CPSS_DXCH_PORT_STATE_STC      portStateStc;  /* current port state */
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U8   sdVecSize;
    GT_U16  *sdVectorPtr;
    GT_BOOL isPortAlreadyConfigured;
    GT_BOOL isPortConnectedTo1690Phy;
    GT_BOOL isCgUnitInUse = GT_FALSE;
    GT_U32  regAddr;
    GT_U32  txFifoMin;
    MV_HWS_PORT_INIT_INPUT_PARAMS   portInitInParam;

    if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
    {
        /* For Aldrin: secondary means CPLL ref clock */
        refClockSource = SECONDARY_LINE_SRC;
        if(PRV_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_GET(aldrinDebugCpllInitFlag) == GT_FALSE)
        {
            refClockSource = PRIMARY_LINE_SRC;
        }
    }
    else
    {
        /* For BobK A0 and A1: PRIMARY means external ref clock */
        /* For BC3/Armstrong, Aldrin2 and above: PRIMARY means CPLL */
        refClockSource = PRIMARY_LINE_SRC;

        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E) &&
           (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E) &&
           (PRV_CPSS_PP_MAC(devNum)->revision > 0))
        {
            /* For 5G_BaseR in BobK it is needed to use CPLL as reference clock source */
            if((ifMode == CPSS_PORT_INTERFACE_MODE_KR_E) && (speed == CPSS_PORT_SPEED_5000_E))
            {
                refClockSource = SECONDARY_LINE_SRC;
            }
        }
    }

    if(powerUp == GT_TRUE)
    {
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChSerdesRefClockTranslateCpss2Hws(devNum, &refClock);
        if (rc != GT_OK)
        {
            return rc;
        }

        lbPort = (CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E == ifMode) ?
                                                        GT_TRUE : GT_FALSE;
    }
    else
    {/* to prevent warnings */
        portMode = NON_SUP_MODE;
        refClock = MHz_156;
        lbPort = GT_FALSE;
    }

    for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portsBmp, portNum))
        {
            PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                         portNum,
                                                                         portMacMap);
        }
        else
        {
            continue;
        }

        if ((PRV_CPSS_GE_PORT_GE_ONLY_E == PRV_CPSS_DXCH_PORT_TYPE_OPTIONS_MAC(devNum, portMacMap))
                && (portMacMap < 48) && (CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E == ifMode)
                && (portMacMap != (portMacMap & 0xFFFFFFFC)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        portIfModePtr = &(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap));
        portSpeedPtr = &(PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap));

        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, &portMapShadowPtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChPortCaelumModeSpeedSet: error in prvCpssDxChPortPhysicalPortMapShadowDBGet, portNum = %d\n", portNum);
        }

        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) &&
           portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
        {
            rc = prvCpssDxChIsCgUnitInUse(devNum, portNum, ifMode, speed, &isCgUnitInUse);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            if(isCgUnitInUse == GT_TRUE)
            {
                /* init CG port register database */
                rc = prvCpssDxChCgPortDbInvalidate(devNum, portMacMap, GT_FALSE);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
        }

        /*---------------------------------------------------------------------*
         *  remote port has no it own MAC ==> no interface can be defined ==>  *
         *  therefore IF could not be obtained                                 *
         *---------------------------------------------------------------------*/
        if (portMapShadowPtr->portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
        {
            rc = prvCpssDxChPortIsAlreadyConfigured(devNum, portNum, powerUp, *portIfModePtr, *portSpeedPtr, ifMode, speed, &isPortAlreadyConfigured);
            if (rc != GT_OK)
            {
                return rc;
            }
            if(isPortAlreadyConfigured)
            {
                continue;
            }
        }
#if 0 /* commented out segment of code correct only for Aldrin+88E1690 devices */
        /*-----------------------------------------------------------------------------------*
         * extended cascade and remote ports  : just speed under 10G (inclusive) are allowed *
         *-----------------------------------------------------------------------------------*/
        if (portMapShadowPtr->portMap.isExtendedCascadePort || portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
        {
            GT_U32 i;
            CPSS_PORT_SPEED_ENT allowedSpeedArr[] =
            {
                CPSS_PORT_SPEED_10_E,
                CPSS_PORT_SPEED_100_E,      /* 1 */
                CPSS_PORT_SPEED_1000_E,     /* 2 */
                CPSS_PORT_SPEED_2500_E,     /* 5 */
                CPSS_PORT_SPEED_5000_E,     /* 6 */
                CPSS_PORT_SPEED_10000_E,    /* 3 */
                CPSS_PORT_SPEED_NA_E
            };

            for (i = 0 ; allowedSpeedArr[i] != CPSS_PORT_SPEED_NA_E; i++)
            {
                if (allowedSpeedArr[i] == speed)
                {
                    break;
                }
            }
            if (allowedSpeedArr[i] == CPSS_PORT_SPEED_NA_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"prvCpssDxChPortCaelumModeSpeedSet: ETHERNET isExtendedCascade or REMOTE Port : allowed speeds under 10G, portNum = %d\n", portNum);
            }
        }
#endif
        if ((portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E) && (!PRV_CPSS_SIP_6_CHECK_MAC(devNum)))
        {
            /*-----------------------------------------------------------------------------------*
             *  remote has MAC of corresponding extended-cascade-port                            *
             *  when we set SPEED/IF table write according to MAC, we harm cascade port !!!!!!   *
             *-----------------------------------------------------------------------------------*
             * currently speed above 1G are PROHIBITED for REMOTE since absent the machanism that
             *  placed slices uniformly over dummy port space
             *-----------------------------------------------------------------------------------*
             *  portIfModePtr = &(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap));
             *  portSpeedPtr  = &(PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap));
             *-----------------------------------------------------------------------------------*
             * in Falcon new Pizza/Resource algorithm must NOT be called for remote ports !!!!!
             *------------------------------------------------------------------------------------*/

            portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacMap);

            /* run MACPHY callback */
            if (portMacObjPtr != NULL && portMacObjPtr->macDrvMacPortPowerDownSetFunc)
            {
                GT_BOOL doPpMacConfig = GT_TRUE;

                rc = portMacObjPtr->macDrvMacPortPowerDownSetFunc(devNum,portNum,
                                          !powerUp,
                                          CPSS_MACDRV_STAGE_PRE_E,
                                          &doPpMacConfig);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }

            if (powerUp == GT_TRUE)
            {
                rc = prvCpssDxChPortPizzaArbiterIfConfigure(devNum, portNum,  ((speed < CPSS_PORT_SPEED_1000_E) ? CPSS_PORT_SPEED_1000_E : speed));
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"REMOTE: error in prvCpssDxChPortPizzaArbiterIfConfigure, portNum = %d\n", portNum);
                }
            }
            else /* powerUp == GT_FALSE */
            {
                rc = prvCpssDxChPortPizzaArbiterIfDelete(devNum, portNum);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"REMOTE : error in prvCpssDxChPortPizzaArbiterIfDelete, portNum = %d\n", portNum);
                }
            }
            /* save new interface mode in DB */
            continue;
        }

        if(powerUp == GT_FALSE)
        {
            /* we remove the ports, so need to use current interface and speed
             * if configured, not the values from the caller */
            if((CPSS_PORT_INTERFACE_MODE_NA_E != *portIfModePtr) &&
                                        (CPSS_PORT_SPEED_NA_E != *portSpeedPtr))
            {
                ifMode = *portIfModePtr;
                speed =  *portSpeedPtr;
            }
            else
            {
                continue; /* port is already power down , go to next port */
            }
        }
        else
        {
            rc = prvCpssDxChPortSerdesFreeCheck(devNum,portNum,ifMode,speed);
            if(rc != GT_OK)
            {
                 return rc;
            }
            rc = prvCpssCommonPortInterfaceSpeedGet(devNum, portMacMap,
                                                                  ifMode, speed,
                                                                  &supported);
            if(rc != GT_OK)
            {
                return rc;
            }

            if(!supported)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
        }

        /****************************************************************************************
            PART 1 - port disable

            This configuration will trigger:
                  a. The MAC will issue an indication towards the EGF
                     to stop sending packets to it.
                  b. The MAC will issue ready to the TXFIFO and thus the
                     traffic on the way to the port will be flushed.

            PART 2 - Check all traffic to the port was flushed

            1. Read Port<%n>DescCounter in the TXQ and wait for it to reach 0.
               This counter indicates how many descriptors are allocated for the port in the TXQ queues.
            2. Read TxDMA Port <%n> Credit Counter register in the TXQ and wait for it to reach the corresponding configured descriptor credits threshold in the TXDMA (Descriptor Credits SCDMA %p).
            3. In the TXDMA regfile - read the TXFIFO credits counters status and wait for it to reach zero.
               There is a separate counter for headers (scdma_%p_header_txfifo_counter) and payloads (scdma_%p_payload_txfifo_counter).
               These counters count the amount of data that is inside the TXDMA/TXFIFO on its way to the port.

        *****************************************************************************************/

        rc = prvCpssDxChPortStateDisableAndGet(devNum,portNum,&portStateStc);
        if (rc != GT_OK)
        {
            return rc;
        }

        fullConfig = GT_TRUE;
        if(CPSS_PORT_IF_MODE_QSGMII_USX_QUSGMII_CHECK_MAC(ifMode))
        {
            rc = bcat2QsgmiiConfig(devNum, portNum, powerUp, &fullConfig);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in bcat2QsgmiiConfig, portNum = %d, powerUp = %d\n", portNum, powerUp);
            }
        }
        /* if one Port in the same GOP already configured to MLG mode, fullConfig=GT_FALSE */
        if(ifMode == CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E)
        {
            rc = checkMLGonePortConfigured(devNum, portNum, powerUp, &fullConfig, &configuredPortsBmp);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in checkMLGonePortConfigured, portNum = %d, powerUp = %d\n", portNum, powerUp);
            }

            if((portNum & 0xFFFFFFFC) == portNum) /* the current port is the first port (0) of the quad */
            {
                if((GT_FALSE == powerUp) && (0 != (configuredPortsBmp & (~0x1)))/* ports 1,2,3 of the quad are configured */)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"Cannot power-down MLG's first port since some of the other ports are not deleted (bitmap view): [1,2,3]=[%d,%d,%d]\n",
                                                    (configuredPortsBmp&2)?1:0 ,(configuredPortsBmp&4)?1:0, (configuredPortsBmp&8)?1:0);
                }
            }
            else /* the current ports is port 1,2 or 3 of the quad */
            {
                if((GT_TRUE == powerUp) && (0 == (configuredPortsBmp & 0x1))/* first port (0) of the quad is not configured*/)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"Cannot power-up MLG port since port %d is not configured\n", (portNum & 0xFFFFFFFC));
                }
            }
        }
        /****************************************************************************************
            PART 3 - Remove resources allocated to the port

        *****************************************************************************************/
        rc = prvCpssDxChPortCaelumPortDelete(devNum, portNum, ifMode, speed,
                                            fullConfig);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortCaelumPortDelete, portNum = %d, ifMode = %d, speed = %d\n", portNum, ifMode, speed);
        }

        /* For BC3, BobK, Aldrin and AC3X devices: Set the Shaping profile reservation for port mode 29.09G-SR_LR */
        if ((ifMode == CPSS_PORT_INTERFACE_MODE_SR_LR4_E) && (speed == CPSS_PORT_SPEED_29090_E))
        {
            rc = prvCpssDxChPortTxShaper29_09gConfig(devNum, portNum, powerUp);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChPortTxShaper29_09gConfig, portNum = %d\n", portNum);
            }
        }
        else /* prvCpssDxChPortTxShaper29_09gConfig treats High Speed ports */
        if ((powerUp) &&
             PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum) &&
            (ifMode != CPSS_PORT_INTERFACE_MODE_NA_E) &&
            (speed != CPSS_PORT_SPEED_NA_E))
        {
            GT_U32      speedIndex;
            GT_U32      speedValue;

            rc = prvCpssDxChCaelumPortResourcesSpeedIndexGet (devNum,
                                                                0,
                                                                speed,
                                                                &speedIndex,
                                                                &speedValue);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChCaelumPortResourcesSpeedIndexGet, portNum = %d\n", portNum);
            }

            if (speedValue >= 40000)
            {
                /* Add High Speed port */
                rc = prvCpssDxChPortHighSpeedPortSet(
                    devNum, PRV_CPSS_DXCH_PORT_HIGH_SPEED_OPERATION_ADD_E, 1, &portNum);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChPortHighSpeedPortSet, portNum = %d\n", portNum);
                }
            }
        }

        /* save new interface mode in DB after prvCpssDxChPortBcat2PortDelete */
        *portIfModePtr = CPSS_PORT_INTERFACE_MODE_NA_E;
        *portSpeedPtr = CPSS_PORT_SPEED_NA_E;

        if(!powerUp)
        {
            /*
                In order to avoid TXQ descriptors stuck it's not needed to restore
                EGF filtler state during "cpssDxChPortEnableSet". Therefore DB is
                updated before call, and restored after.
            */

            GT_BOOL egfFilterTempChangeStatus = GT_FALSE;

            if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                if (PRV_CPSS_PP_MAC(devNum)->portEgfForceStatusBitmapPtr == NULL)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "portEgfForceStatusBitmapPtr is NULL");
                }
                /* fetching Egf status from db */
                if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(PRV_CPSS_PP_MAC(devNum)->portEgfForceStatusBitmapPtr, portNum) == GT_FALSE)
                {
                    CPSS_PORTS_BMP_PORT_SET_MAC(PRV_CPSS_PP_MAC(devNum)->portEgfForceStatusBitmapPtr, portNum);
                    egfFilterTempChangeStatus = GT_TRUE;
                }
            }
            /* calling to prvCpssDxChPortStateRestore here will cause to BC3 port get stuck with TXQ buffers,
               so EGF filer state will be enabled during next link up event */
            rc = cpssDxChPortEnableSet(devNum, portNum, portStateStc.portEnableState);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortStateEnableSet, portNum = %d\n", portNum);
            }
            if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                /* restore prev value */
                if (egfFilterTempChangeStatus == GT_TRUE)
                {
                    CPSS_PORTS_BMP_PORT_CLEAR_MAC(PRV_CPSS_PP_MAC(devNum)->portEgfForceStatusBitmapPtr, portNum);
                }
            }
            if (GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_BOBCAT3_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E))
            {
                /* just detach MAC from Led */
                rc = prvCpssDxChLedErrataPortLedMacTypeSet(devNum,portNum,PRV_CPSS_PORT_NOT_EXISTS_E);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChLedErrataPortLedMacTypeSet() at power down, portNum = %d\n", portNum);
                }
            }

            if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
            {
                rc = prvCpssDxChPortWa10gXlgEnable(devNum,portNum,speed,GT_FALSE);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortWa10gXlgEnable, portNum = %d\n", portNum);
                }
            }

            if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
            {
                /* restore default after CG or GE power down */
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap].portType = PRV_CPSS_PORT_XG_E;

                if(isCgUnitInUse == GT_TRUE)
                {/* update port DB with the default port type value in order to prevent using CG MAC
                       when it is disabled  */
                    /* invalidate CG port register database */
                    rc = prvCpssDxChCgPortDbInvalidate(devNum, portMacMap, GT_TRUE);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                    }
                }
            }

            /* PRV_CPSS_DXCH_ERRATA_ALDRIN_SDWRR_WA_E treatment */
            if ((GT_FALSE !=
                    PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_ERRATA_ALDRIN_SDWRR_WA_E)) &&
                (portMapShadowPtr->portMap.txqNum != CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS))
            {
                rc = prvCpssDxChPortAldrinSdwrrWa(
                    devNum, portNum, powerUp, speed);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChPortAldrinSdwrrWa\n");
                }
            }
            continue;
        } /*!powerUp*/

        rc = prvCpssDxChPortPizzaArbiterIfConfigure(devNum, portNum,  ((speed < CPSS_PORT_SPEED_1000_E) ? CPSS_PORT_SPEED_1000_E : speed));
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortPizzaArbiterIfConfigure, portNum = %d\n", portNum);
        }

        if(!PRV_CPSS_PP_MAC(devNum)->isGmDevice)
        {
            rc = prvCpssDxChCaelumPortResourcesConfig(devNum, portNum, ifMode, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChCaelumPortResourcesConfig, portNum = %d\n", portNum);
            }
        }

        /* Set FCA interface width */
        rc = prvCpssDxChPortBcat2FcaBusWidthSet(devNum, portNum, ifMode, speed);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortBcat2FcaBusWidthSet, portNum = %d\n", portNum);
        }
        /* Set PTP interface width */
        rc = prvCpssDxChPortBcat2PtpInterfaceWidthSelect(devNum, portNum, speed);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortBcat2PtpInterfaceWidthSelect, portNum = %d\n", portNum);
        }

        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, &portMapShadowPtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortPhysicalPortMapShadowDBGet, portNum = %d\n", portNum);
        }

        if(portMapShadowPtr->portMap.trafficManagerEn)
        {
            /* Port speed calibration value for TM Flow Control */
            #if defined (INCLUDE_TM)
                        rc = prvCpssDxChTmGlueFlowControlPortSpeedSet(devNum,
                                                          portMapShadowPtr->portMap.tmPortIdx,
                                                                      speed);
            #else
                rc = GT_NOT_SUPPORTED;
            #endif /*#if defined (INCLUDE_TM)*/

            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChTmGlueFlowControlPortSpeedSet, portNum = %d\n", portNum);
            }
        }
        realMacNum = portMacMap;
        if(fullConfig)
        {

            if((CPSS_PORT_IF_MODE_QSGMII_USX_QUSGMII_CHECK_MAC(ifMode)) || (CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E == ifMode))
            {/* provide to HWS first port in quadruplet and it will configure
                all other ports if needed */
                portMacMap &= 0xFFFFFFFC;
            }

            if (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap].portRefClock.enableOverride == GT_TRUE)
            {
                switch(PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap].portRefClock.portRefClockSource)
                {
                    case CPSS_PORT_REF_CLOCK_SOURCE_PRIMARY_E:
                        refClockSource = PRIMARY_LINE_SRC;
                        break;
                    case CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E:
                        refClockSource = SECONDARY_LINE_SRC;
                        break;
                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }
            }

            if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) &&
                (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE))
            {
                if(refClockSource == SECONDARY_LINE_SRC) /* <-- EXTERNAL REF CLOCK IS USED */
                {
                    /* When BC3/Armstrong uses CPLL (which it's the default SW configuration) it is needed to  configure the following:
                        a. PECL_EN=0 /DFX/Units/DFX Server Registers/Device General Control 1 [0] = 0x0 (PECL Disable)
                        b. In addition, the software must configure it back to normal, once using on board ref clocks.
                    */
                    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl1;
                    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, regAddr, 0, 1, 1);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                    }
                }
            }

            rc = hwsPortModeParamsGetToBuffer(devNum, 0, portMacMap, portMode, &curPortParams);
            if (GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in hwsPortModeParamsGetToBuffer, portMacMap = %d\n", portMacMap);
            }
            sdVecSize   = curPortParams.numOfActLanes;
            sdVectorPtr = curPortParams.activeLanesList;

            /* CPLL ref clock update */
            rc = prvCpssDxChPortRefClockUpdate(devNum, portMode, sdVectorPtr, sdVecSize, refClockSource, &refClock);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortRefClockUpdate, portNum = %d\n", portNum);
            }

            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortFlavorInit(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], lbPort[%d], refClock[%d], refClockSource[%d]})", devNum, 0, portMacMap, portMode, lbPort, refClock, PRIMARY_LINE_SRC);
            cpssOsMemSet(&portInitInParam, 0,sizeof(portInitInParam));
            portInitInParam.lbPort = lbPort;
            portInitInParam.refClock = refClock;
            portInitInParam.refClockSource = refClockSource;

            rc = mvHwsPortFlavorInit(devNum, 0, portMacMap, portMode, &portInitInParam);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"mvHwsPortFlavorInit: error for portNum = %d\n", portNum);
            }
            CPSS_LOG_INFORMATION_MAC("Exit: mvHwsPortFlavorInit(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], lbPort[%d], refClock[%d], refClockSource[%d]})", devNum, 0, portMacMap, portMode, lbPort, refClock, PRIMARY_LINE_SRC);

            /* AC3X RXAUI ports needs to be set with custom serdes tx parameters. Those parameter
               need to override hws defaults */
            if ( PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E &&
                 ifMode == CPSS_PORT_INTERFACE_MODE_RXAUI_E)
            {
                CPSS_LOG_INFORMATION_MAC("performing AC3X serdes TX parameters tunning  - %s",
                                         "checking if port is connected to 1690 Phy");
                rc = prvCpssDxChAc3xIsPortConnectedTo1690Phy(devNum, portNum, &isPortConnectedTo1690Phy);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"calling prvCpssDxChAc3xIsPortConnectedTo1690Phy: %s %d\n",
                                                  "error for portNum =", portNum);
                }
                if (isPortConnectedTo1690Phy==GT_TRUE)
                {
                    CPSS_LOG_INFORMATION_MAC("performing AC3X serdes TX parameters tunning");
                    /* Serdes TX parameters API require CPSS ifMode set, so setting it
                       temporary, then setting it back to the NA ifMode and continue*/
                    *portIfModePtr = CPSS_PORT_INTERFACE_MODE_RXAUI_E;

                    rc = prvCpssDxChAc3xRxauiSerdesTxParametersSet(devNum, portNum, portMacMap, portMode);
                    if (rc != GT_OK)
                    {
                        *portIfModePtr = CPSS_PORT_INTERFACE_MODE_NA_E;
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling %s error for portNum = %d, portMacMap: = %d \n",
                                                      "prvCpssDxChAc3xRxauiSerdesTxParametersSet: "
                                                      , portNum, portMacMap);
                    }
                    *portIfModePtr = CPSS_PORT_INTERFACE_MODE_NA_E;
                }
            }

            /* set serdes Tx/Rx Tuning values if SW DB values initialized */
            rc = prvCpssDxChBobkPortSerdesTuning(devNum, 0, portNum, portMode);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChBobkPortSerdesTuning, portNum = %d\n", portNum);
            }

            /* set the Polarity values on Serdeses if SW DB values initialized */
            rc = prvCpssDxChPortSerdesPolaritySet(devNum, 0, sdVectorPtr, sdVecSize);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortSerdesPolaritySet, portNum = %d\n", portNum);
            }

            /* Enable the Tx signal, the signal was disabled during Serdes init */
            rc = mvHwsPortTxEnable(devNum, 0, portMacMap, portMode, GT_TRUE);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in mvHwsPortTxEnable, portNum = %d\n", portNum);
            }
            CPSS_LOG_INFORMATION_MAC("Exit: mvHwsPortTxEnable(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], enable[%d]})", devNum, 0, portMacMap, portMode, GT_TRUE);

            /* work around for AC3X link flapping issue on the RXAUI ports of the Aldrin, in which
               1690 Phy devices are connected to */
            if ( PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E &&
                 ifMode == CPSS_PORT_INTERFACE_MODE_RXAUI_E)
            {
                CPSS_LOG_INFORMATION_MAC("performing AC3X RXAUI port WA - checking if port is connected to 1690 Phy");
                rc = prvCpssDxChAc3xIsPortConnectedTo1690Phy(devNum, portNum, &isPortConnectedTo1690Phy);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"calling prvCpssDxChAc3xIsPortConnectedTo1690Phy: %s %d \n",
                                                  "error for portNum = ", portNum);
                }
                CPSS_LOG_INFORMATION_MAC("performing AC3X RXAUI port WA");
                if (isPortConnectedTo1690Phy==GT_TRUE)
                {
                    rc = prvCpssDxChAc3xRxauiPortWA(devNum, portNum);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"calling prvCpssDxChAc3xRxauiPortWA: error for portNum = %d\n", portNum);
                    }
                }
            }
        }
        prvCpssDxChPortTypeSet(devNum, portNum, ifMode, speed);

        if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            rc = prvCpssDxChPortWa10gXlgEnable(devNum,portNum,speed,GT_TRUE);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortWa10gXlgEnable, portNum = %d\n", portNum);
            }
        }


        /* update addresses of mac registers accordingly to used MAC GE/XLG/etc. */
        rc = prvCpssDxChHwRegAddrPortMacUpdate(devNum, portNum,ifMode);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChHwRegAddrPortMacUpdate, portNum = %d\n", portNum);
        }

        if((CPSS_PORT_SPEED_10_E == speed) || (CPSS_PORT_SPEED_100_E == speed))
        {/* HWS doesn't support 10/100M, so after 1G configured by HWS, fix speed here */
            rc = geMacUnitSpeedSet(devNum, portNum, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in geMacUnitSpeedSet, portNum = %d\n", portNum);
            }
        }

        /* RM-9725147 implementation */
        if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_GIGE_MAC_TX_FIFO_WA_E))
        {
            if((speed == CPSS_PORT_SPEED_1000_E) ||
               (speed == CPSS_PORT_SPEED_2500_E))
            {
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[realMacNum].gePortFIFOConfigReg1;
                if(speed == CPSS_PORT_SPEED_1000_E)
                {
                    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
                    {
                        case CPSS_PP_FAMILY_DXCH_BOBCAT3_E:
                            txFifoMin = 129;
                            break;
                        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
                            txFifoMin = 130;
                            break;
                        default:
                            txFifoMin = 65;
                            break;
                    }
                }
                else
                {
                    switch(PRV_CPSS_PP_MAC(devNum)->devFamily)
                    {
                        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
                            txFifoMin = 130;
                            break;
                        default:
                            txFifoMin = 129;
                            break;
                    }
                }
                rc = prvCpssHwPpSetRegField(devNum, regAddr, 6, 8, txFifoMin);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

                /* Watermarks configuration */
                regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[realMacNum].gePortFIFOConfigReg0;
                txFifoMin = (txFifoMin + 4) | ((txFifoMin + 2) << 8);
                rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 16, txFifoMin);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
        }

        /* Configures type of the port connected to the LED */
        rc = prvCpssDxChLedPortTypeConfig(devNum, portNum,powerUp);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChLedPortTypeConfig, portNum = %d\n", portNum);
        }

        rc = prvCpssDxChPortImplementWaSGMII2500(devNum, portNum, ifMode, speed);
        if (rc != GT_OK)
        {
             return rc;
        }

        /* save new interface mode in DB */
        *portIfModePtr = ifMode;
        *portSpeedPtr = speed;

        /*Creating Port LoopBack*/
        if(lbPort)
        {
            rc = prvCpssDxChPortSerdesLoopbackActivate (devNum, portNum, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortSerdesLoopbackActivate, portNum = %d\n", portNum);
            }
        }

        rc = prvCpssDxChPortStateRestore(devNum, portNum, &portStateStc);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortStateEnableSet, portNum = %d\n", portNum);
        }

        /* PRV_CPSS_DXCH_ERRATA_ALDRIN_SDWRR_WA_E treatment */
        if ((GT_FALSE !=
                PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_ERRATA_ALDRIN_SDWRR_WA_E)) &&
            (portMapShadowPtr->portMap.txqNum != CPSS_DXCH_PORT_MAPPING_INVALID_PORT_CNS))
        {
            rc = prvCpssDxChPortAldrinSdwrrWa(
                devNum, portNum, powerUp, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChPortAldrinSdwrrWa\n");
            }
        }

    } /* for(portNum = 0; */

    return GT_OK;
}


/**
* @internal internal_cpssDxChPortResourceTmBandwidthSet function
* @endinternal
*
* @brief   Define system bandwith through TM port. Following this configuration
*         TM port resources will be allocated.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
* @param[in] tmBandwidthMbps          - system TM bandwidth
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on no ports connected to TM
*
* @note Note:
*       - API should not be called under traffic
*       - zero bandwidth will release resources of TM for non TM ports
*
*/
GT_STATUS internal_cpssDxChPortResourceTmBandwidthSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  tmBandwidthMbps
)
{
    GT_STATUS                                   rc;                         /* return code */
    CPSS_DXCH_DETAILED_PORT_MAP_STC          *portMapShadowPtr;   /* pointer to mapping database */
    GT_PHYSICAL_PORT_NUM                        portNum;                    /* port number loop iterator */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E
                                            | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);



    /*-------------------------------------------------------------------------------------*
     *  find at least one port connected to TM                                             *
     *  if there is no ports mapped to TM, the determination of TM bandwidth is meaningless *
     *-------------------------------------------------------------------------------------*/
    for(portNum = 0; portNum < PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum); portNum++)
    {
        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/&portMapShadowPtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"cpssDxChPortResourceTmBandwidthSet: error in prvCpssDxChPortPhysicalPortMapShadowDBGet for port = %d\n", portNum);
        }
        if((portMapShadowPtr->valid == GT_TRUE) && (portMapShadowPtr->portMap.trafficManagerEn == GT_TRUE))
        {
            break;
        }
    }
    if(portNum == PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"cpssDxChPortResourceTmBandwidthSet: there are no ports connected to TM\n");
    }


    /* BobK family */
    if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChCaelumPortResourceStaticTmSet(devNum, tmBandwidthMbps);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"cpssDxChPortResourceTmBandwidthSet: error in prvCpssDxChCaelumPortResourceStaticTmSet()");
        }
    }
    else if(PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChCaelumPortResourceDynamicTmSet(devNum, tmBandwidthMbps);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"cpssDxChPortResourceTmBandwidthSet: error in prvCpssDxChCaelumPortResourceDynamicTmSet()");
        }
    }
    else if(PRV_CPSS_DXCH_BOBCAT2_B0_CHECK_MAC(devNum))
    {
        rc = prvCpssDxChBcat2B0PortResourceTmSet(devNum, tmBandwidthMbps);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"cpssDxChPortResourceTmBandwidthSet: error in prvCpssDxChCaelumPortResourceTmSet()");
        }
    }
    else if (PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum))
    {
        /*--------------------------------------------------------*
         * just configure TM for max resources(no specific BW)    *
         *--------------------------------------------------------*/
        rc = prvCpssDxChPort_Bcat2A0_TM_ResourcesConfig(devNum);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChPort_Bcat2A0_TM_ResourcesConfig(): error ()");
        }
     }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortDynamicPizzaArbiterIfTMBWSet(devNum,tmBandwidthMbps);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"cpssDxChPortResourceTmBandwidthSet: error in prvCpssDxChPortDynamicPizzaArbiterIfTMBWSet()");
    }


    return GT_OK;
}

/**
* @internal cpssDxChPortResourceTmBandwidthSet function
* @endinternal
*
* @brief   Define system bandwith through TM port. Following this configuration
*         TM port resources will be allocated.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.       .
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] tmBandwidthMbps          - system TM bandwidth
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on no ports connected to TM
*
* @note Note:
*       - API should not be called under traffic
*       - zero bandwidth will release resources of TM for non TM ports
*
*/
GT_STATUS cpssDxChPortResourceTmBandwidthSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  tmBandwidthMbps
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortResourceTmBandwidthSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tmBandwidthMbps));

    rc = internal_cpssDxChPortResourceTmBandwidthSet(devNum, tmBandwidthMbps);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tmBandwidthMbps));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortResourceTmBandwidthGet function
* @endinternal
*
* @brief   Get system bandwith through TM port. Following this configuration
*         TM port resources will be allocated.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
*
* @param[out] tmBandwidthMbpsPtr       - (pointer to) system TM bandwidth
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on no ports connected to TM
*/
GT_STATUS internal_cpssDxChPortResourceTmBandwidthGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *tmBandwidthMbpsPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E
                                            | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(tmBandwidthMbpsPtr);

    /* BobK family */
    if(PRV_CPSS_DXCH_CETUS_CHECK_MAC(devNum) || (PRV_CPSS_DXCH_CAELUM_CHECK_MAC(devNum)))
    {
        return prvCpssDxChCaelumPortResourceTmGet(devNum, tmBandwidthMbpsPtr);
    }

    /* Bobcat2 family */
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
        /*return prvCpssDxChBc2PortResourceTmSet(devNum, tmBandwidthMbps);*/
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortResourceTmBandwidthGet function
* @endinternal
*
* @brief   Get system bandwith through TM port. Following this configuration
*         TM port resources will be allocated.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number
*
* @param[out] tmBandwidthMbpsPtr       - (pointer to) system TM bandwidth
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on no ports connected to TM
*/
GT_STATUS cpssDxChPortResourceTmBandwidthGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *tmBandwidthMbpsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortResourceTmBandwidthGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tmBandwidthMbpsPtr));

    rc = internal_cpssDxChPortResourceTmBandwidthGet(devNum, tmBandwidthMbpsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tmBandwidthMbpsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortHwUnreset function
* @endinternal
*
* @brief   HW port unreset. This API unreset the HW port compenent Mac, PCS and SerDes.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2;
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently, API not supported for QSGMII ports.
*       Currently, API not supported for Serdes Loopback state restore.
*
*/
GT_STATUS prvCpssDxChPortHwUnreset
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum
)
{
    MV_HWS_REF_CLOCK_SUP_VAL    refClock;
    MV_HWS_REF_CLOCK_SOURCE     refClockSource;
    GT_U32      portMacMap;
    GT_STATUS   rc;
    GT_U8  sdVecSize;
    GT_U16 *sdVectorPtr;
    GT_U32      regAddr;
    GT_BOOL     isPortConnectedTo1690Phy;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;
    MV_HWS_PORT_STANDARD            portMode;
    MV_HWS_PORT_INIT_INPUT_PARAMS   portInitInParam;

    CPSS_LOG_INFORMATION_MAC("inside prvCpssDxChPortHwUnreset with portNum=%d. ", portNum);

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    /* get current ifMode and speed from CPSS db - they were not deleted in HwReset */
    speed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap);
    ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap);

    if (CPSS_PORT_IF_MODE_QSGMII_USX_QUSGMII_CHECK_MAC(ifMode))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "PortHwUnreset API not supported for QSGMII port interface mode" );
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "PortHwUnreset API - "
                                      " prvCpssCommonPortIfModeToHwsTranslate returned error=%d", rc );
    }

    if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        CPSS_LOG_INFORMATION_MAC("inside prvCpssDxChPortHwUnreset with portNum=%d. Device is sip5_15 ", portNum);

        if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portMacMap, portMode, &curPortParams))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "hwsPortModeParamsGet returned null " );
        }

        sdVecSize = curPortParams.numOfActLanes;
        sdVectorPtr = curPortParams.activeLanesList;


        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E))
        {
            /* For Aldrin: secondary means CPLL ref clock */
            refClockSource = SECONDARY_LINE_SRC;
            if(PRV_SHARED_PORT_DIR_PORT_CTRL_SRC_GLOBAL_VAR_GET(aldrinDebugCpllInitFlag) == GT_FALSE)
            {
                refClockSource = PRIMARY_LINE_SRC;
            }
        }
        else if (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))
        {
            /* For BC3: secondary means external ref clock */
            refClockSource = PRIMARY_LINE_SRC;
        }
        else /* BobK A0 and A1 */
        {
            refClockSource = PRIMARY_LINE_SRC;
        }

        rc = prvCpssDxChSerdesRefClockTranslateCpss2Hws(devNum, &refClock);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap].portRefClock.enableOverride == GT_TRUE)
        {
            switch(PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap].portRefClock.portRefClockSource)
            {
                case CPSS_PORT_REF_CLOCK_SOURCE_PRIMARY_E:
                    refClockSource = PRIMARY_LINE_SRC;
                    break;
                case CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E:
                    refClockSource = SECONDARY_LINE_SRC;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }

        rc = prvCpssDxChPortRefClockUpdate(devNum, portMode, sdVectorPtr, sdVecSize, refClockSource, &refClock);
        if (rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "FATAL handling clock #2 on portNum %d returned error code %d ",
                                                       portNum,
                                                       rc );
        }
        cpssOsMemSet(&portInitInParam, 0,sizeof(portInitInParam));
        portInitInParam.lbPort = GT_FALSE;
        portInitInParam.refClock = refClock;
        portInitInParam.refClockSource = refClockSource;
        portInitInParam.portSpeed = prvCpssCommonPortSpeedEnumToHwsSpeedConvert(speed);

        rc = mvHwsPortFlavorInit(devNum, 0, portMacMap, portMode, &portInitInParam);
        if (rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "FATAL handling mvHwsPortFlavorInit on portNum %d returned error code %d ",
                                                       portNum,
                                                       rc );
        }

        CPSS_LOG_INFORMATION_MAC("Exit: mvHwsPortFlavorInit(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], lbPort[%d], refClock[%d], refClockSource[%d]})", devNum, 0, portMacMap, portMode, 0/*lbPort*/, refClock, PRIMARY_LINE_SRC);

        /* AC3X RXAUI ports needs to be set with custom serdes tx parameters. Those parameter
           need to override hws defaults */
        if ( PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E &&
             ifMode == CPSS_PORT_INTERFACE_MODE_RXAUI_E)
        {
            CPSS_LOG_INFORMATION_MAC("performing AC3X serdes TX parameters tunning  - %s",
                                     "checking if port is connected to 1690 Phy");
            rc = prvCpssDxChAc3xIsPortConnectedTo1690Phy(devNum, portNum, &isPortConnectedTo1690Phy);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"calling prvCpssDxChAc3xIsPortConnectedTo1690Phy: %s %d\n",
                                              "error for portNum =", portNum);
            }
            if (isPortConnectedTo1690Phy==GT_TRUE)
            {
                CPSS_LOG_INFORMATION_MAC("performing AC3X serdes TX parameters tunning");
                /* Serdes TX parameters API require CPSS ifMode set, so setting it
                   temporary, then setting it back to the NA ifMode and continue*/
                rc = prvCpssDxChAc3xRxauiSerdesTxParametersSet(devNum, portNum, portMacMap, portMode);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"calling %s error for portNum = %d, portMacMap: = %d \n",
                                                    "prvCpssDxChAc3xRxauiSerdesTxParametersSet: "
                                                    , portNum, portMacMap);
                }
            }
        }

        /* set serdes Tx/Rx Tuning values if SW DB values initialized */
        rc = prvCpssDxChBobkPortSerdesTuning(devNum, 0, portNum, portMode);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChBobkPortSerdesTuning, portNum = %d\n", portNum);
        }

        /* set the Polarity values on Serdeses if SW DB values initialized */
        rc = prvCpssDxChPortSerdesPolaritySet(devNum, 0, sdVectorPtr, sdVecSize);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortSerdesPolaritySet, portNum = %d\n", portNum);
        }

        /* Enable the Tx signal, the signal was disabled during Serdes init */
        rc = mvHwsPortTxEnable(devNum, 0, portMacMap, portMode, GT_TRUE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in mvHwsPortTxEnable, portNum = %d\n", portNum);
        }
        CPSS_LOG_INFORMATION_MAC("Exit: mvHwsPortTxEnable(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], enable[%d]})", devNum, 0, portMacMap, portMode, GT_TRUE);

        /* work around for AC3X link flapping issue on the RXAUI ports of the Aldrin, in which
           1690 Phy devices are connected to */
        if ( PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E &&
             ifMode == CPSS_PORT_INTERFACE_MODE_RXAUI_E)
        {
            CPSS_LOG_INFORMATION_MAC("performing AC3X RXAUI port WA - checking if port is connected to 1690 Phy");
            rc = prvCpssDxChAc3xIsPortConnectedTo1690Phy(devNum, portNum, &isPortConnectedTo1690Phy);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"calling prvCpssDxChAc3xIsPortConnectedTo1690Phy: %s %d \n",
                                              "error for portNum = ", portNum);
            }
            CPSS_LOG_INFORMATION_MAC("performing AC3X RXAUI port WA");
            if (isPortConnectedTo1690Phy==GT_TRUE)
            {
                rc = prvCpssDxChAc3xRxauiPortWA(devNum, portNum);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"calling prvCpssDxChAc3xRxauiPortWA: error for portNum = %d\n", portNum);
                }
            }
        }

        if((CPSS_PORT_SPEED_10_E == speed) || (CPSS_PORT_SPEED_100_E == speed))
        {/* HWS doesn't support 10/100M, so after 1G configured by HWS, fix speed here */
            rc = geMacUnitSpeedSet(devNum, portNum, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in geMacUnitSpeedSet, portNum = %d\n", portNum);
            }
        }

        /* Configures type of the port connected to the LED */
        rc = prvCpssDxChLedPortTypeConfig(devNum, portNum,GT_TRUE/*powerUp*/);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChLedPortTypeConfig, portNum = %d\n", portNum);
        }

        rc = prvCpssDxChPortImplementWaSGMII2500(devNum, portNum, ifMode, speed);
        if (rc != GT_OK)
        {
             return rc;
        }

        /*Creating Port LoopBack*/
        if(0/*lbPort*/)
        {
            rc = prvCpssDxChPortSerdesLoopbackActivate (devNum, portNum, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortSerdesLoopbackActivate, portNum = %d\n", portNum);
            }
        }

    }
    else if ( PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum) )
    {
        CPSS_LOG_INFORMATION_MAC("inside prvCpssDxChPortHwUnreset with portNum=%d. Device is Xcat3 ", portNum);

        rc = prvCpssDxChSerdesRefClockTranslateCpss2Hws(devNum, &refClock);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        switch(speed)
        {
            case CPSS_PORT_SPEED_10_E:
            case CPSS_PORT_SPEED_100_E:
            case CPSS_PORT_SPEED_1000_E:
            case CPSS_PORT_SPEED_2500_E:
                refClockSource = PRIMARY_LINE_SRC;
                break;

            case CPSS_PORT_SPEED_5000_E:
            case CPSS_PORT_SPEED_10000_E:
            case CPSS_PORT_SPEED_11800_E:
            case CPSS_PORT_SPEED_20000_E:
            case CPSS_PORT_SPEED_22000_E:

                refClockSource = SECONDARY_LINE_SRC;
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if(PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap].portRefClock.enableOverride == GT_TRUE)
        {
            switch(PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap].portRefClock.portRefClockSource)
            {
                case CPSS_PORT_REF_CLOCK_SOURCE_PRIMARY_E:
                    refClockSource = PRIMARY_LINE_SRC;
                    break;
                case CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E:
                    refClockSource = SECONDARY_LINE_SRC;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortFlavorInit(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], "
                                 "lbPort[%d], refClock[%d], refClockSource[%d])",
                                 devNum, 0, portMacMap/*firstInQuadruplet*/, portMode, GT_FALSE/*lbPort*/,
                                 refClock, refClockSource);
        cpssOsMemSet(&portInitInParam, 0,sizeof(portInitInParam));
        portInitInParam.lbPort = GT_FALSE;
        portInitInParam.refClock = refClock;
        portInitInParam.refClockSource = refClockSource;

        rc = mvHwsPortFlavorInit(devNum, 0, portMacMap/*firstInQuadruplet*/, portMode, &portInitInParam);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        if((CPSS_PORT_SPEED_10_E == speed) || (CPSS_PORT_SPEED_100_E == speed))
        {/* HWS doesn't support 10/100M, so after 1G configured by HWS, fix speed here */
            PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacMap,&regAddr);
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 6, 1, 0);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = prvCpssHwPpSetRegField(devNum, regAddr, 5, 1, (CPSS_PORT_SPEED_10_E == speed) ? 0 : 1);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        else if ((GT_FALSE == GT_TRUE/*fullConfig*/) && (CPSS_PORT_SPEED_1000_E == speed))
        {
            /* 1G support is done by HWS only during initial creation through  */
            /* "mvHwsPortFlavorInit" (see usage above). However when switching */
            /* back to 1G from 10M/100M the following should be reconfigured.  */
            PRV_CPSS_DXCH_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacMap,&regAddr);
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 6, 1, 1);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        rc = prvCpssDxChPortImplementWaSGMII2500(devNum, portNum, ifMode, speed);
        if (rc != GT_OK)
        {
             return rc;
        }

        if(0/*lbPort*/)
        {
            rc = prvCpssDxChPortSerdesLoopbackActivate (devNum, portNum, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "PortHwUnreset not applicable for this device");
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortHwReset function
* @endinternal
*
* @brief   HW port reset. This API reset the HW port compenent Mac, PCS and SerDes.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2;
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] qsgmiiFullDelete         - qsgmii full config boolean
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently, API not supported for QSGMII ports.
*
*/
GT_STATUS prvCpssDxChPortHwReset
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    GT_BOOL                         qsgmiiFullDelete
)
{
    GT_STATUS   rc;
    GT_U32      portMacMap;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    CPSS_PORT_SPEED_ENT speed;

    CPSS_LOG_INFORMATION_MAC("inside prvCpssDxChPortHwReset with portNum=%d", portNum);

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap);
    speed = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap);

    if (CPSS_PORT_IF_MODE_QSGMII_USX_QUSGMII_CHECK_MAC(ifMode))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "PortHwReset API not supported for QSGMII port interface mode" );
    }

    if (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        CPSS_LOG_INFORMATION_MAC("inside prvCpssDxChPortHwReset with portNum=%d, device is Xcat3. ", portNum);

        rc = prvCpssDxChPortXcat3PortDelete(devNum, portNum, ifMode, speed,
                                qsgmiiFullDelete);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"calling prvCpssDxChPortXcat3PortDelete on DISABLE on port %d"
              " returned error code %d ",
              portNum, rc);
        }
    }
    else if (PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        CPSS_LOG_INFORMATION_MAC("inside prvCpssDxChPortHwReset with portNum=%d, device is SIP5_15. ", portNum);

        rc =  prvCpssDxChPortBcat2PortReset(devNum,portNum,ifMode,speed,qsgmiiFullDelete);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"calling prvCpssDxChPortBcat2PortReset on DISABLE on port %d"
                                      " returned error code %d ",
                                      portNum, rc);
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "PortHwReset not applicable for this device");
    }

    return rc;
}

/**
* @internal prvCpssDxChPortModeSpeedSetWrapper function
* @endinternal
*
* @brief   The Wrapper is using to set fec and other pre
*          configure before using portmodespeedset in some
*          devices that can use pre configuration
*
* @note   APPLICABLE DEVICES:     xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; AC5; Aldrin2.
* @note   NOT APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portsBmp                 - physical port number
* @param[in] powerUp                  - power up/down bool
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - interface speed
* @param[in] portMgrDbPtr             - PM DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - fír not supported actions
* @retval GT_BAD_STATE             - nan speed/mode
*
*/
GT_STATUS prvCpssDxChPortModeSpeedSetWrapper
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PORTS_BMP_STC                  portsBmp,
    IN  GT_BOOL                             powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT        ifMode,
    IN  CPSS_PORT_SPEED_ENT                 speed,
    IN  PRV_CPSS_PORT_MNG_PORT_SM_DB_STC    *portMgrDbPtr
)
{
    MV_HWS_PORT_FEC_MODE    hwsFecMode;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  portMacNum;
    MV_HWS_PORT_STANDARD    portMode;
    GT_STATUS               rc;
    CPSS_PORT_FEC_MODE_ENT  fecMode;
    CPSS_DXCH_PORT_FEC_MODE_ENT  dxFecMode;
    CPSS_PORT_INTERFACE_MODE_ENT    ifModeTmp = ifMode;
    CPSS_PORT_SPEED_ENT             speedTmp = speed;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PM_NOT_APPLICABLE_DEVICE(devNum, CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(portMgrDbPtr);

    /* Set CPSS_PORT_FEC_MODE_LAST_E in case of handling power down or AP port, otherwise from port database */
    fecMode = (!powerUp) ? (CPSS_PORT_FEC_MODE_LAST_E) : (portMgrDbPtr->fecAbility);
    dxFecMode = (CPSS_DXCH_PORT_FEC_MODE_ENT)fecMode;

    if (((PRV_CPSS_SIP_5_15_CHECK_MAC(devNum)) || (CPSS_PP_FAMILY_DXCH_AC5_E == PRV_CPSS_PP_MAC(devNum)->devFamily) ) && (fecMode != CPSS_PORT_FEC_MODE_LAST_E))
    {
        for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            if(prvCpssDxChPortRemotePortCheck(CAST_SW_DEVNUM(devNum),portNum))
                continue;

            if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portsBmp, portNum))
            {
                PRV_CPSS_CONVERT_CPSS_FEC_MODE_VAL_TO_HWS_MAC(dxFecMode,hwsFecMode);
                if (hwsFecMode == FEC_NA)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }

                PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(CAST_SW_DEVNUM(devNum), portNum, portMacNum);

                if ((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }

                if (ifMode > CPSS_PORT_INTERFACE_MODE_NA_E && speed > CPSS_PORT_SPEED_NA_E)
                {
                    ifModeTmp -= CPSS_PORT_INTERFACE_MODE_NA_E;
                    speedTmp -= CPSS_PORT_SPEED_NA_E;
                }

                rc = prvCpssCommonPortIfModeToHwsTranslate(CAST_SW_DEVNUM(devNum), ifModeTmp, speedTmp, &portMode);
                if (rc != GT_OK)
                {
                    return rc;
                }

                if (GT_FALSE == hwsIsFecModeSupported(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, hwsFecMode))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                }

                /* update HWS elements DB */
                rc = hwsPortModeParamsSetFec(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, hwsFecMode);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

                rc = prvCpssSetSpecialSpeedConfiguration(devNum, portNum, speed, powerUp);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"calling prvCpssSetSpecialSpeedConfiguration from wrapper failed=%d",rc);
                }
            }
        }
    }

    if ( PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) )
    {
        return prvCpssDxChPort_SIP6_10_ModeSpeedSet(CAST_SW_DEVNUM(devNum), portsBmp, powerUp, ifMode, speed, portMgrDbPtr);
    }
    else
    {
        return cpssDxChPortModeSpeedSet(CAST_SW_DEVNUM(devNum), &portsBmp, powerUp, ifMode, speed);
    }
}

/**
* @internal prvCpssDxChPortHwUnreset function
* @endinternal
*
* @brief   HW port unreset. This API unreset the HW port compenent Mac, PCS and SerDes.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2;
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently, API not supported for QSGMII ports.
*       Currently, API not supported for Serdes Loopback state restore.
*
*/
GT_STATUS prvCpssDxChPortHwUnresetWrapper
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum
)
{
    return prvCpssDxChPortHwUnreset(CAST_SW_DEVNUM(devNum), portNum);
}

/**
* @internal prvCpssDxChPortHwReset function
* @endinternal
*
* @brief   HW port reset. This API reset the HW port compenent Mac, PCS and SerDes.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2;
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] qsgmiiFullDelete         - qsgmii full config boolean
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently, API not supported for QSGMII ports.
*
*/
GT_STATUS prvCpssDxChPortHwResetWrapper
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    GT_BOOL                            qsgmiiFullDelete
)
{
    return prvCpssDxChPortHwReset(CAST_SW_DEVNUM(devNum), portNum, qsgmiiFullDelete);
}

/**
* @internal prvCpssDxChPortFecModeSetWrapper function
* @endinternal
*
* @brief   Wrapper function for CPSS function cpssDxChPortFecModeSet
*         in order to use in in Common code.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] mode                     - Forward error correction mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortFecModeSetWrapper
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_FEC_MODE_ENT  mode
)
{
    GT_STATUS rc;
    GT_U32 portMacNum;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(CAST_SW_DEVNUM(devNum), portNum, portMacNum);

    rc = cpssDxChPortFecModeSet(CAST_SW_DEVNUM(devNum), portNum,(CPSS_DXCH_PORT_FEC_MODE_ENT)mode);

    return rc;
}

/**
* @internal prvCpssDxChPortFecModeGetWrapper function
* @endinternal
*
* @brief   Read current Forward error correction (FEC) mode of port
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] mode                     - current Forward error correction mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortFecModeGetWrapper
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_PORT_FEC_MODE_ENT  *mode
)
{
    GT_STATUS rc;
    GT_U32 portMacNum;

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(CAST_SW_DEVNUM(devNum), portNum, portMacNum);

    rc = cpssDxChPortFecModeGet(CAST_SW_DEVNUM(devNum), portNum,(CPSS_DXCH_PORT_FEC_MODE_ENT*)mode);

    return rc;
}


/**
* @internal prvCpssDxChPortInterfaceModeGetWrapper function
* @endinternal
*
* @brief   convert input from GT_SW_DEV_NUM to U8
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
*/
GT_STATUS prvCpssDxChPortInterfaceModeGetWrapper
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
)
{
    return cpssDxChPortInterfaceModeGet(CAST_SW_DEVNUM(devNum), portNum, ifModePtr);
}

/**
* @internal prvCpssDxChFalconPortCutThroughSpeedSet function
* @endinternal
*
* @brief   Configure Cut Through port speed index for slow-to-fast Cut Through termination.
*          Note: (APPLICABLE DEVICES: Ironman;)
*                The application should re-enable cut through for 'unknown byte count' packets
*                for speeds from "fast" to "slow",use cpssDxChCutThroughPortUnknownByteCountEnableSet.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portsBmp                 - physical port number
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChFalconPortCutThroughSpeedSet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  CPSS_PORT_SPEED_ENT    speed
)
{
    GT_STATUS rc;
    GT_U32    speedIndex;

    /* speed index in range 0..31 */
    switch (speed)
    {
        case CPSS_PORT_SPEED_10_E:    speedIndex =  0; break;
        case CPSS_PORT_SPEED_100_E:   speedIndex =  1; break;
        case CPSS_PORT_SPEED_1000_E:  speedIndex =  2; break;
        case CPSS_PORT_SPEED_2500_E:  speedIndex =  3; break;
        case CPSS_PORT_SPEED_5000_E:  speedIndex =  4; break;
        case CPSS_PORT_SPEED_10000_E: speedIndex =  5; break;
        case CPSS_PORT_SPEED_11800_E: speedIndex =  5; break; /* same as 10000 assumed not be used together */
        case CPSS_PORT_SPEED_12000_E: speedIndex =  6; break;
        case CPSS_PORT_SPEED_12500_E: speedIndex =  7; break;
        case CPSS_PORT_SPEED_13600_E: speedIndex =  8; break;
        case CPSS_PORT_SPEED_15000_E: speedIndex =  9; break;
        case CPSS_PORT_SPEED_16000_E: speedIndex = 10; break;
        case CPSS_PORT_SPEED_20000_E: speedIndex = 11; break;
        case CPSS_PORT_SPEED_22000_E: speedIndex = 12; break;
        case CPSS_PORT_SPEED_23600_E: speedIndex = 13; break;
        case CPSS_PORT_SPEED_25000_E: speedIndex = 14; break;
        case CPSS_PORT_SPEED_26700_E: speedIndex = 15; break;
        case CPSS_PORT_SPEED_29090_E: speedIndex = 16; break;
        case CPSS_PORT_SPEED_40000_E: speedIndex = 17; break;
        case CPSS_PORT_SPEED_42000_E: speedIndex = 18; break;
        case CPSS_PORT_SPEED_47200_E: speedIndex = 19; break;
        case CPSS_PORT_SPEED_50000_E: speedIndex = 20; break;
        case CPSS_PORT_SPEED_53000_E: speedIndex = 21; break;
        case CPSS_PORT_SPEED_75000_E: speedIndex = 22; break;
        case CPSS_PORT_SPEED_100G_E:  speedIndex = 23; break;
        case CPSS_PORT_SPEED_102G_E:  speedIndex = 24; break;
        case CPSS_PORT_SPEED_106G_E:  speedIndex = 25; break;
        case CPSS_PORT_SPEED_107G_E:  speedIndex = 26; break;
        case CPSS_PORT_SPEED_140G_E:  speedIndex = 27; break;
        case CPSS_PORT_SPEED_200G_E:  speedIndex = 28; break;
        case CPSS_PORT_SPEED_212G_E:  speedIndex = 29; break;
        case CPSS_PORT_SPEED_400G_E:  speedIndex = 30; break;
        case CPSS_PORT_SPEED_424G_E:  speedIndex = 31; break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChWriteTableEntryField(
        devNum, CPSS_DXCH_SIP5_TABLE_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_E, portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP6_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_PORT_SPEED_E,
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        speedIndex);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChWriteTableEntryField(
        devNum, CPSS_DXCH_SIP5_20_TABLE_EGF_QAG_PORT_SOURCE_ATTRIBUTES_E, portNum,
        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
        SIP6_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_SPEED_E,
        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
        speedIndex);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /*for SIP >= 6.30, disable Egr cut through of fast ports(speed >= 5G)*/
    if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        if (speedIndex >= 4) /*CPSS_PORT_SPEED_5000_E*/
        {
            rc = cpssDxChCutThroughPortUnknownByteCountEnableSet(devNum, portNum, GT_FALSE);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChFalconPortQueueRateSet function
* @endinternal
*
* @brief   Configure Queue Rate based on port speed.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Lion2; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portsBmp                 - physical port number
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChFalconPortQueueRateSet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  CPSS_PORT_SPEED_ENT    speed
)
{
    GT_STATUS               rc;         /* return code */
    GT_U32                  qFctrE;
    GT_U32                  qFctrM;
    GT_U32                  qFctr;
    GT_U32 tileNum;
    GT_U32 i,lastQueue;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * anodePtr = NULL;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * preAnodePtr = NULL;/*preemptive partner node*/
    GT_U32 dummy = 0,localdpPortNum,dpNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT  mappingType;
    GT_BOOL isCascade;
    GT_U32 pNodeNum,aNodeListSize;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE * pNodePtr = NULL;

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum,portNum,&tileNum,&dpNum,&localdpPortNum,&mappingType);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ",portNum);
    }


    /*Threshold set only for local ports ,remote ports thresholds will be set via cascade port*/
    if(mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
    {
        rc = prvCpssFalconTxqUtilsIsCascadePort(devNum,portNum,&isCascade,NULL);

        if(rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsIsCascadePort  failed for portNum  %d  ",portNum);
        }

        if(isCascade == GT_TRUE)
        {
            rc    = prvCpssSip6TxqUtilsPnodeIndexGet(devNum, dpNum, localdpPortNum,&pNodeNum);
            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqUtilsPnodeIndexGet  failed for local port  %d  ",localdpPortNum);
            }

            PRV_TXQ_SIP_6_PNODE_INSTANCE_GET(pNodePtr,devNum,tileNum,pNodeNum);

            i = pNodePtr->aNodelist[0].queuesData.queueBase;
            aNodeListSize = pNodePtr->aNodeListSize;
            lastQueue = i+(pNodePtr->aNodelist[aNodeListSize-1].queuesData.pdqQueueLast -pNodePtr->aNodelist[0].queuesData.pdqQueueFirst);
        }
        else
        {

            rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum,portNum,&tileNum,&dummy);

            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet  failed for portNum  %d  ",portNum);
            }

            rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum,portNum,&anodePtr);

            if(rc!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ",portNum);
            }

            i = anodePtr->queuesData.queueBase;
            lastQueue = i+(anodePtr->queuesData.pdqQueueLast -anodePtr->queuesData.pdqQueueFirst);

        }


        /* speed index in range 0..31 */
        switch (speed)
        {
            case CPSS_PORT_SPEED_10_E:
                if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
                {
                    /*
                        TBD: values taken from 100M - need to check real config!!!!!!!!!

                    */
                    qFctrM = 1;
                    qFctrE = 0;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                break;
            case CPSS_PORT_SPEED_100_E:
                qFctrM = 1;
                qFctrE = 0;
                break;
            case CPSS_PORT_SPEED_1000_E:
                qFctrM = 8;
                qFctrE = 0;
                break;
            case CPSS_PORT_SPEED_2500_E:
                qFctrM = 1;
                qFctrE = 1;
                break;
            case CPSS_PORT_SPEED_5000_E:
                qFctrM = 2;
                qFctrE = 1;
                break;
            case CPSS_PORT_SPEED_10000_E:
                qFctrM = 5;
                qFctrE = 1;
                break;
            case CPSS_PORT_SPEED_12000_E:
            case CPSS_PORT_SPEED_12500_E:
            case CPSS_PORT_SPEED_13600_E:
                qFctrM = 6;
                qFctrE = 1;
                break;
            case CPSS_PORT_SPEED_11800_E:
                qFctrM = 5;
                qFctrE = 1;
                break;
            case CPSS_PORT_SPEED_15000_E:
            case CPSS_PORT_SPEED_16000_E:
                qFctrM = 7;
                qFctrE = 1;
                break;
            case CPSS_PORT_SPEED_20000_E:
                qFctrM = 9;
                qFctrE = 1;
                break;
            case CPSS_PORT_SPEED_22000_E:
                qFctrM = 10;
                qFctrE = 1;
                break;
            case CPSS_PORT_SPEED_23600_E:
                qFctrM = 11;
                qFctrE = 1;
                break;
            case CPSS_PORT_SPEED_25000_E:
                qFctrM = 12;
                qFctrE = 1;
                break;
            case CPSS_PORT_SPEED_29090_E:
                qFctrM = 14;
                qFctrE = 1;
                break;
            case CPSS_PORT_SPEED_40000_E:
            case CPSS_PORT_SPEED_42000_E:
            case CPSS_PORT_SPEED_47200_E:
            case CPSS_PORT_SPEED_50000_E:
                qFctrM = 1;
                qFctrE = 2;
                break;
            case CPSS_PORT_SPEED_75000_E:
                qFctrM = 2;
                qFctrE = 2;
                break;
            case CPSS_PORT_SPEED_53000_E:
            case CPSS_PORT_SPEED_100G_E:
            case CPSS_PORT_SPEED_107G_E:
            case CPSS_PORT_SPEED_102G_E:
                qFctrM = 3;
                qFctrE = 2;
                break;
            case CPSS_PORT_SPEED_140G_E:
                qFctrM = 4;
                qFctrE = 2;
                break;
            case CPSS_PORT_SPEED_106G_E:
            case CPSS_PORT_SPEED_200G_E:
            case CPSS_PORT_SPEED_212G_E:
                qFctrM = 6;
                qFctrE = 2;
                break;
            case CPSS_PORT_SPEED_400G_E:
            case CPSS_PORT_SPEED_424G_E:
                qFctrM = 12;
                qFctrE = 2;
                break;
            default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* QCN Factor is 6 bits wide with first 2 bits denoting QfctrE and
         * rest 4 bits for QfctrM.
         */
        qFctr = qFctrE | qFctrM << 2;

        rc =prvCpssSip6TxqSdqLocalPortQcnFactorSet(devNum,
                 tileNum,dpNum,i,lastQueue,qFctr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /*Handle preemptive channel*/
        if (TXQ_IS_PREEMPTIVE_DEVICE(devNum))
        {
            if( (NULL!=anodePtr) && (GT_TRUE==anodePtr->preemptionActivated))
            {
                rc = prvCpssDxChTxqSip6PreeptivePartnerAnodeGet(devNum, anodePtr,&preAnodePtr);
                if (rc != GT_OK)
                {
                    return rc;
                }

                CPSS_NULL_PTR_CHECK_MAC(preAnodePtr);

                i = preAnodePtr->queuesData.queueBase;
                lastQueue = i+(preAnodePtr->queuesData.pdqQueueLast -preAnodePtr->queuesData.pdqQueueFirst);

                rc =prvCpssSip6TxqSdqLocalPortQcnFactorSet(devNum,
                 tileNum,dpNum,i,lastQueue,qFctr);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
        }

    }
    return GT_OK;
}

/**
* @internal hawkUsxConfig function
* @endinternal
*
* @brief   Check if port needs configuration on MAC/PCS/Serdes levels,
*         force link down on ports of its quadruplet if application not
*         configured them yet.
*
* @note   APPLICABLE DEVICES:      AC5P, AC5X, Harrier, Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] fullConfigPtr            - call port delete/create or just configure pizza
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*
* @note In QSGMII can't configure only one port, because of internal physical
*       synchronization issues, but we don't want application get linkup interrupts
*       from ports it
*
*/
static GT_STATUS hawkUsxConfig
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  MV_HWS_PORT_STANDARD            portMode,
    IN  GT_BOOL                         powerUp,
    OUT GT_BOOL                         *fullConfigPtr
)
{
    GT_U32      i;                          /* iterator */
    GT_PHYSICAL_PORT_NUM    portMacNum;     /* MAC number mapped to given port */
    GT_U32      firstInGroup;               /* mac number of first port in group */
    GT_U32      numberOfPortInGroup;        /* number of ports in group (4 / 8) */

    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum,
                                                                 portMacNum);

    *fullConfigPtr = GT_TRUE;
    portMode = portMode;

    if(!isSupportUsxPort(devNum,portMacNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "The Device[%d] portNum[%d] not supports USX mode",
            devNum,portNum);
    }

    switch(ifMode)
    {
        case CPSS_PORT_INTERFACE_MODE_USX_5G_DXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_10G_DXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_20G_DXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_QSGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_QUSGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_10G_QXGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_20G_QXGMII_E:
            if(PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
            {
                firstInGroup = portMacNum & 0xFFFFFFFC;
            }
            else
            {
                firstInGroup = ((portMacNum - 2*(portMacNum / 26 + 1)) & 0xFFFFFFFC) + 2*(portMacNum / 26 + 1);
            }
            numberOfPortInGroup = 4;
            break;

        case CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E:
        case CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E:
            if(PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
            {
                firstInGroup = portMacNum & 0xFFFFFFF8;
            }
            else
            {
                firstInGroup = ((portMacNum - 2*(portMacNum / 26 + 1)) & 0xFFFFFFF8) + 2*(portMacNum / 26 + 1);
            }
            numberOfPortInGroup = 8;
            break;

        default:
            return GT_OK;
    }

    if(GT_TRUE == powerUp)
    {
        for(i = firstInGroup; i < firstInGroup+numberOfPortInGroup; i++)
        {
            if(CPSS_PORT_MULTIPLE_USX_PORT_MODE_CHECK_MAC(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, i)))
            {
                *fullConfigPtr = GT_FALSE;
                return GT_OK;
            }
        }
    }
    else
    {
        for(i = firstInGroup; i < firstInGroup+numberOfPortInGroup; i++)
        {
            if(i != portMacNum)
            {
                if(CPSS_PORT_INTERFACE_MODE_NA_E !=
                                            PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, i))
                {
                    *fullConfigPtr = GT_FALSE;
                    break;
                }
            }
        }
    }

    return GT_OK;
}


/**
* @internal pvrCpssDxChPortRemotePortConfigSet function
* @endinternal
*
* @brief  Remote port related configurations.
*
* @note   APPLICABLE DEVICES:      Falcon, AC5P, AC5X, Harrier, Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - physical port number
* @param[in] powerUp               - power up/down state
* @param[in] speed                 - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
*
*/
static GT_STATUS pvrCpssDxChPortRemotePortConfigSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         powerUp
)
{
    GT_STATUS rc;

    /* state that the TXQ resources of this portNum are 'removed' */
    rc = prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapSet(devNum, portNum,GT_FALSE);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapSet : disable, 'remote physical port' portNum = %d\n",
            portNum);
    }

    /*Stop enqueue*/
    rc =prvCpssDxChEgfPortLinkFilterForceLinkDown(devNum, portNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssFalconTxqUtilsRemotePortFlush(devNum, portNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(powerUp == GT_TRUE)
    {
        /* state that the TXQ resources of this portNum are 'valid'    */
        /* and therefore we allow the application to set 'EGF link up' */
        /* NOTE: remote port requires to have also valid speed on the 'cascade port' ,
                 before can have 'EGF link up' */
        rc = prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapSet(devNum, portNum,GT_TRUE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapSet : enable, 'remote physical port' portNum = %d\n",
                portNum);
        }

        rc = prvCpssDxChEgfPortLinkFilterRestoreApplicationIfAllowed(devNum, portNum);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChEgfPortLinkFilterRestoreApplicationIfAllowed failed for  portNum = %d\n",
                portNum);
        }
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChFalconPortModeSpeedSet function
* @endinternal
*
* @brief   Configure Interface mode and speed on a specified port and execute
*         on port's serdeses power up sequence; or configure power down on port's
*         serdeses.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portsBmp                 - physical port number
* @param[in] powerUp                  - serdes power:
*                                      GT_TRUE - up;
*                                      GT_FALSE - down;
* @param[in] ifMode                   - interface mode (related only for serdes power up [powerUp==GT_TRUE])
* @param[in] speed                    - port data  (related only for serdes power up [powerUp==GT_TRUE])
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Pay attention! Before configure CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E
*       MUST execute power down for port with this interface.
*
*/
static GT_STATUS prvCpssDxChPortFalconModeSpeedSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_PORTS_BMP_STC              portsBmp,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS               rc;         /* return code */
    GT_PHYSICAL_PORT_NUM    portNum;    /* iterator */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */
    GT_BOOL                 supported;  /* is ifMode/speed supported on port */
    MV_HWS_PORT_STANDARD    portMode;
    GT_BOOL                 lbPort;
    MV_HWS_REF_CLOCK_SUP_VAL refClock;
    MV_HWS_REF_CLOCK_SOURCE  refClockSource;
    CPSS_PORT_INTERFACE_MODE_ENT *portIfModePtr;
    CPSS_PORT_SPEED_ENT          *portSpeedPtr;
    GT_BOOL                 fullConfig;     /* for USX modes:
                                                    one serdes serves four ports, when one port of
                                                    the quadruplete configured three others configured too,
                                                    so when application asks to configure another port from
                                                    quadruplete we don't need to execute full configuretion of new port*/
    CPSS_DXCH_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    PRV_CPSS_DXCH_PORT_STATE_STC      portStateStc;  /* current port state */
    GT_U8   sdVecSize;
    GT_U16 *sdVectorPtr;
    GT_BOOL isPortAlreadyConfigured;
    GT_BOOL isPreemptionEnabled = GT_FALSE;

    MV_HWS_PORT_INIT_INPUT_PARAMS   portInitInParam;
    MV_HWS_PORT_INIT_PARAMS   curPortParams;  /* current port parameters */
    GT_BOOL isCascadePort; /* TXQ not support cascade port yet. skip TXQ configurations for cascade ports*/
    GT_U32  minCtByteCount;  /* cut through minimal byte count per port */
    GT_U32  speedInMbPerSec; /* speed In MB Per Second */
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    if(powerUp == GT_TRUE)
    {
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum, ifMode, speed, &portMode);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssDxChSerdesRefClockTranslateCpss2Hws(devNum, &refClock);
        if (rc != GT_OK)
        {
            return rc;
        }

        lbPort = (CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E == ifMode) ?
                                                        GT_TRUE : GT_FALSE;
    }
    else
    {/* to prevent warnings */
        portMode = NON_SUP_MODE;
        refClock = MHz_156;
        lbPort = GT_FALSE;
    }
    for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portsBmp, portNum))
        {
            PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                         portNum,
                                                                         portMacMap);
        }
        else
        {
            continue;
        }

        portIfModePtr = &(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap));
        portSpeedPtr = &(PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap));

        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, &portMapShadowPtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChPortPhysicalPortMapShadowDBGet, portNum = %d\n", portNum);
        }

        rc = prvCpssDxChPortIsAlreadyConfigured(devNum, portNum, powerUp, *portIfModePtr, *portSpeedPtr, ifMode, speed, &isPortAlreadyConfigured);
        if (rc != GT_OK)
        {
            return rc;
        }
        if(isPortAlreadyConfigured)
        {
            continue;
        }

        /* remote ports do not have MAC,only flush txQ*/
        if (prvCpssDxChPortRemotePortCheck(devNum, portNum) == GT_TRUE)
        {
            if(GT_TRUE==powerUp)
            {
                if((speed!=CPSS_PORT_SPEED_REMOTE_E)||(ifMode!=CPSS_PORT_INTERFACE_MODE_REMOTE_E))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                        "port %d speed/mode  can only be set to CPSS_PORT_SPEED_REMOTE_E/CPSS_PORT_INTERFACE_MODE_REMOTE_E\n",
                         portNum);
                }
            }

            rc = pvrCpssDxChPortRemotePortConfigSet(devNum, portNum, powerUp);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"pvrCpssDxChPortRemotePortConfigSet : failed for portNum = %d\n", portNum);
            }

            continue;
        }

        rc = prvCpssFalconTxqUtilsIsCascadePort(devNum, portNum,&isCascadePort,NULL);
        if(rc != GT_OK)
        {
             return rc;
        }

        rc = prvCpssDxChPortSerdesFreeCheck(devNum, portNum, ifMode, speed);
        if(rc != GT_OK)
        {
             return rc;
        }

        if(powerUp == GT_FALSE)
        {
            /* we remove the ports, so need to use current interface and speed
             * if configured, not the values from the caller */
            if((CPSS_PORT_INTERFACE_MODE_NA_E != *portIfModePtr) &&
                                        (CPSS_PORT_SPEED_NA_E != *portSpeedPtr))
            {
                ifMode = *portIfModePtr;
                speed =  *portSpeedPtr;
            }
            else
            {
                continue; /* port is already power down , go to next port */
            }
        }

        rc = prvCpssCommonPortInterfaceSpeedGet(devNum, portMacMap,
                                                              ifMode, speed,
                                                              &supported);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(!supported)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        /* check if we need to protect the port create of this port with interaction with OTHER ports in the DP[] ! */
        if (powerUp == GT_TRUE)
        {
            rc = prvFalconPortCreateWa_start(devNum,portNum);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        /* Invalidate LED port position */
        rc = prvCpssDxChFalconLedStreamPortPositionSet(devNum, portNum, 0x3F);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChFalconLedStreamPortPositionSet, portNum = %d\n", portNum);
        }

        /****************************************************************************************
            PART 1 - port disable

            This configuration will trigger:
                  a. The MAC will issue an indication towards the EGF
                     to stop sending packets to it.
                  b. The MAC will issue ready to the TXFIFO and thus the
                     traffic on the way to the port will be flushed.

            PART 2 - Check all traffic to the port was flushed

            1. Read Port<%n>DescCounter in the TXQ and wait for it to reach 0.
               This counter indicates how many descriptors are allocated for the port in the TXQ queues.
            2. Read TxDMA Port <%n> Credit Counter register in the TXQ and wait for it to reach the corresponding configured descriptor credits threshold in the TXDMA (Descriptor Credits SCDMA %p).
            3. In the TXDMA regfile - read the TXFIFO credits counters status and wait for it to reach zero.
               There is a separate counter for headers (scdma_%p_header_txfifo_counter) and payloads (scdma_%p_payload_txfifo_counter).
               These counters count the amount of data that is inside the TXDMA/TXFIFO on its way to the port.

        *****************************************************************************************/

        if(powerUp == GT_FALSE)
        {
            /* state that we are doing 'port disable' from 'port delete' context , and we may need the 'port delete WA' */
            prvFalconPortDeleteWa_enable(devNum);
        }

        rc = prvCpssDxChPortStateDisableAndGet(devNum, portNum, &portStateStc);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(powerUp == GT_FALSE)
        {
            /* reset the flag (before checking rc !) */
            rc = prvFalconPortDeleteWa_disable(devNum, portNum, portStateStc.portEnableState);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }

        fullConfig = GT_TRUE;
        if(isSupportUsxPort(devNum,portMacMap))
        {
            rc = hawkUsxConfig(devNum, portNum, ifMode, portMode, powerUp, &fullConfig);
        }

        /****************************************************************************************
            PART 3 - Remove resources allocated to the port

        *****************************************************************************************/

        /*
            In order to avoid ambiguity with MIB counters need
            to reset it before port power down.
        */
        if ((powerUp == GT_FALSE) && PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            CPSS_PORT_MAC_MTI_COUNTER_SET_STC  mtiMibStc;

            rc = cpssDxChPortMacCountersOnMtiPortGet(devNum, portNum, &mtiMibStc);
            if (GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"cpssDxChPortMacCountersOnMtiPortGet : MIB read for portNum = %d\n", portNum);
            }
        }

        /* state that the TX resources of this portNum are 'removed' */
        rc = prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapSet(devNum, portNum,GT_FALSE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapSet : disable, portNum = %d\n",
                portNum);
        }

        rc = prvCpssDxChPortCaelumPortDelete(devNum, portNum, ifMode, speed,
                                            fullConfig);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortCaelumPortDelete, portNum = %d, ifMode = %d, speed = %d\n", portNum, ifMode, speed);
        }

        /* save new interface mode in DB after prvCpssDxChPortBcat2PortDelete */
        *portIfModePtr = CPSS_PORT_INTERFACE_MODE_NA_E;
        *portSpeedPtr = CPSS_PORT_SPEED_NA_E;

        if(!powerUp)
        {
            /* IMPORTANT : UNLIKE sip5_20 in function prvCpssDxChPortCaelumModeSpeedSet(...))

                we NOT need to manipulate the 'DB' of portEgfForceStatusBitmapPtr ,
                because we are protected by next 2 functions :
                prvCpssDxChEgfPortLinkFilterRestoreApplicationIfAllowed(...) and
                prvCpssDxChEgfPortLinkFilterForceLinkDown().
            */
            rc = cpssDxChPortEnableSet(devNum, portNum, portStateStc.portEnableState);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortStateEnableSet, portNum = %d\n", portNum);
            }

            /* set to default */
            prvCpssDxChPortTypeSet(devNum, portNum, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E);

            continue;
        }

        if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->mngInterfaceType != CPSS_CHANNEL_PEX_FALCON_Z_E)
        {
            /*Set txQ thresholds for credit management  and BP*/
            rc = prvCpssDxChTxqSetPortSdqThresholds(devNum, portNum,  speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChTxqSetPortSdqThresholds, portNum = %d\n", portNum);
            }

            /*Set txQ thresholds for long queue management*/
            rc = prvCpssDxChTxqBindPortQueuesToPdsProfile(devNum, portNum,  speed,NULL);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChTxqBindPortQueuesToPdsProfile, portNum = %d\n", portNum);
            }
        }

        if (!PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            /* Falcon:
               - Configuration of DP units should be done before Chiplet/GOP.
               Hawk and above:
               - Configuration of PCA units in Hawk should be done after EPI/GOP.
                 Therefore PCA and DP logic (prvCpssDxChTxPortSpeedPizzaResourcesHawkPortConfigure) is called below.
            */
            rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
            {
                rc = prvCpssDxChTxPortSpeedPizzaResourcesSet(devNum, portNum, ifMode, speed, GT_TRUE);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prevCpssDxChTxPortSpeedResourcesSet, portNum = %d\n", portNum);
                }
            }

            if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->mngInterfaceType != CPSS_CHANNEL_PEX_FALCON_Z_E)
            {
                /* call to set PB after 'TXD' done , because 'TXD' reset the credits of PB */
                 /* Here is only Power On case */
                 rc = prvCpssDxChPortPacketBufferGpcChannelSpeedSet(
                    devNum, portNum, speed);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(
                        rc,"error in prvCpssDxChPortPacketBufferGpcChannelSpeedSet, portNum = %d\n", portNum);
                }
            }
            speedInMbPerSec = prvCpssCommonPortSpeedEnumToMbPerSecConvert(speed);

            if ((speedInMbPerSec <= 10000) && (speedInMbPerSec > 0))
            {
                /* ports with known slow speed */
                minCtByteCount = 0x101;
            }
            else
            {
                /* ports with unknown or fast speed */
                minCtByteCount = 0x100;
            }

            rc = prvCpssDxChCutThroughPortMinCtByteCountSet(
                devNum, portMacMap, minCtByteCount);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(
                    rc,"error in prvCpssDxChCutThroughPortMinCtByteCountSet, portNum %d minCtByteCount %d\n",
                    portNum, minCtByteCount);
            }
            rc = prvCpssDxChFalconPortCutThroughSpeedSet(
                devNum, portNum, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(
                    rc,"error in prvCpssDxChFalconPortCutThroughSpeedSet, portNum %d speed %d\n",
                    portNum, speed);
            }
        }

        rc = prvCpssDxChFalconPortQueueRateSet(
            devNum, portNum, speed);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(
                rc,"error in prvCpssDxChFalconPortQueueRateSet, portNum %d speed %d\n",
                portNum, speed);
        }

        rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, &portMapShadowPtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortPhysicalPortMapShadowDBGet, portNum = %d\n", portNum);
        }

        refClockSource = PRIMARY_LINE_SRC;

        if (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap].portRefClock.enableOverride == GT_TRUE)
        {
            switch(PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap].portRefClock.portRefClockSource)
            {
                case CPSS_PORT_REF_CLOCK_SOURCE_PRIMARY_E:
                    refClockSource = PRIMARY_LINE_SRC;
                    break;
                case CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E:
                    refClockSource = SECONDARY_LINE_SRC;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }

        rc = hwsPortModeParamsGetToBuffer(devNum, 0, portMacMap, portMode, &curPortParams);
        if (GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in hwsPortModeParamsGetToBuffer, portMacMap = %d\n", portMacMap);
        }

        sdVecSize   = curPortParams.numOfActLanes;
        sdVectorPtr = curPortParams.activeLanesList;

        /* CPLL ref clock update */
        rc = prvCpssDxChPortRefClockUpdate(devNum, portMode, sdVectorPtr, sdVecSize, refClockSource, &refClock);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortRefClockUpdate, portNum = %d\n", portNum);
        }

        rc = prvCpssDxChTxqSip6_10PreemptionEnableGet(devNum, portNum, 0, &isPreemptionEnabled);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChTxqSip6_10PreemptionEnableGet: error for portNum = %d\n", portNum);
        }

        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortFlavorInit(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], lbPort[%d], refClock[%d], refClockSource[%d]})", devNum, 0, portMacMap, portMode, lbPort, refClock, PRIMARY_LINE_SRC);
        cpssOsMemSet(&portInitInParam, 0,sizeof(portInitInParam));
        portInitInParam.lbPort = lbPort;
        portInitInParam.refClock = refClock;
        portInitInParam.refClockSource = refClockSource;
        portInitInParam.isPreemptionEnabled = isPreemptionEnabled;
        /* HWS Elements DB update */
        if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portMacMap, portMode, &curPortParams))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
        }

        portInitInParam.portFecMode = curPortParams.portFecMode;
        portInitInParam.portSpeed = prvCpssCommonPortSpeedEnumToHwsSpeedConvert(speed);

        rc = mvHwsPortFlavorInit(devNum, 0, portMacMap, portMode, &portInitInParam);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"mvHwsPortFlavorInit: error for portNum = %d\n", portNum);
        }
        CPSS_LOG_INFORMATION_MAC("Exit: mvHwsPortFlavorInit(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], lbPort[%d], refClock[%d], refClockSource[%d]})", devNum, 0, portMacMap, portMode, lbPort, refClock, PRIMARY_LINE_SRC);

        if(fullConfig)
        {
            rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
            if (rc != GT_OK)
            {
                return rc;
            }
            /* set serdes Tx/Rx Tuning values if SW DB values initialized */
            if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
            {
                CPSS_TBD_BOOKMARK_AC5P /* need to add comPhy support for HAWK */
                rc = prvCpssDxChBobkPortSerdesTuning(devNum, 0, portNum, portMode);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChBobkPortSerdesTuning, portNum = %d\n", portNum);
                }
                /* set the Polarity values on Serdeses if SW DB values initialized */
                rc = prvCpssDxChPortSerdesPolaritySet(devNum, 0, sdVectorPtr, sdVecSize);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortSerdesPolaritySet, portNum = %d\n", portNum);
                }
            CPSS_TBD_BOOKMARK_AC5P
    #ifndef ASIC_SIMULATION
                /* Enable the Tx signal, the signal was disabled during Serdes init */
                rc = mvHwsPortTxEnable(devNum, 0, portMacMap, portMode, GT_TRUE);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in mvHwsPortTxEnable, portNum = %d\n", portNum);
                }
    #endif
                CPSS_LOG_INFORMATION_MAC("Exit: mvHwsPortTxEnable(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], enable[%d]})", devNum, 0, portMacMap, portMode, GT_TRUE);
            }
        }

        if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
        {
            /* Ironman
               Configuration of DP, PB, PCA and MIF units. */
            rc = prvCpssDxChPortDpIronmanPortConfigure(
                devNum, portNum, ifMode, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(
                    rc,
                    "error in prvCpssDxChPortDpIronmanPortConfigure, portNum = %d\n",
                    portNum);
            }
        }
        else if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            /* Hawk and above.
               Configuration of DP, PB and PCA units. */
            rc = prvCpssDxChTxPortSpeedPizzaResourcesPortConfigure(
                devNum, portNum, ifMode, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(
                    rc,
                    "error in prvCpssDxChTxPortSpeedPizzaResourcesHawkPortConfigure, portNum = %d\n",
                    portNum);
            }
        }

        prvCpssDxChPortTypeSet(devNum, portNum, ifMode, speed);

        CPSS_TBD_BOOKMARK_AC5P
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_FALSE)
        {
            /* Configures type of the port connected to the LED */
            rc = prvCpssDxChLedPortTypeConfig(devNum, portNum,powerUp);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChLedPortTypeConfig, portNum = %d\n", portNum);
            }
        }

        /* save new interface mode in DB */
        *portIfModePtr = ifMode;
        *portSpeedPtr = speed;

        /* state that the TX resources of this portNum are 'valid' */
        /* and therefore we allow the application to set 'EGF link up' */
        rc = prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapSet(devNum, portNum,GT_TRUE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapSet : Enable, portNum = %d\n",
                portNum);
        }

        /*Creating Port LoopBack*/
        if(lbPort)
        {
            rc = prvCpssDxChPortSerdesLoopbackActivate (devNum, portNum, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortSerdesLoopbackActivate, portNum = %d\n", portNum);
            }
        }
        rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
        {
            rc = prvCpssDxChPortStateRestore(devNum, portNum, &portStateStc);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortStateEnableSet, portNum = %d\n", portNum);
            }
        }

    } /* for(portNum = 0; */

    return GT_OK;
}

/**
* @internal prvCpssDxChPort_SIP6_10_ModeSpeedSet function
* @endinternal
*
* @brief   Configure Interface mode and speed on a specified port and execute
*         on port's serdeses power up sequence; or configure power down on port's
*         serdeses.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                   - physical device number
* @param[in] portsBmp                 - physical port number
* @param[in] powerUp                  - serdes power:
*                                      GT_TRUE - up;
*                                      GT_FALSE - down;
* @param[in] ifMode                   - interface mode (related only for serdes power up [powerUp==GT_TRUE])
* @param[in] speed                    - port data  (related only for serdes power up [powerUp==GT_TRUE])
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Pay attention! Before configure CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E
*       MUST execute power down for port with this interface.
*
*/
static GT_STATUS prvCpssDxChPort_SIP6_10_ModeSpeedSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORTS_BMP_STC                  portsBmp,
    IN  GT_BOOL                             powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT        ifMode,
    IN  CPSS_PORT_SPEED_ENT                 speed,
    IN  PRV_CPSS_PORT_MNG_PORT_SM_DB_STC    *portMgrDbPtr
)
{
    GT_STATUS               rc;         /* return code */
    GT_PHYSICAL_PORT_NUM    portNum;    /* iterator */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */
    GT_BOOL                 supported;  /* is ifMode/speed supported on port */
    MV_HWS_PORT_STANDARD    portMode;
    GT_BOOL                 isRemotePort = GT_FALSE;

    CPSS_TBD_BOOKMARK_PHOENIX   /* need to be approved for AC5x/AC5p */
    MV_HWS_REF_CLOCK_SUP_VAL refClock = MHz_25;
    MV_HWS_REF_CLOCK_SOURCE  refClockSource = PRIMARY_LINE_SRC;  /* obsolete for the new COMPHY SD, will be controlled by global API for all Serdeses*/
    CPSS_PORT_INTERFACE_MODE_ENT *portIfModePtr;
    CPSS_PORT_SPEED_ENT          *portSpeedPtr;
    GT_BOOL                 fullConfig;     /* for USX modes:
                                                    one serdes serves four ports, when one port of
                                                    the quadruplete configured three others configured too,
                                                    so when application asks to configure another port from
                                                    quadruplete we don't need to execute full configuretion of new port*/
    PRV_CPSS_DXCH_PORT_STATE_STC      portStateStc;  /* current port state */
    GT_BOOL isPortAlreadyConfigured = GT_FALSE;
    GT_BOOL isPreemptionEnabled = GT_FALSE;

    MV_HWS_PORT_INIT_INPUT_PARAMS   portInitInParam;
    MV_HWS_PORT_INIT_PARAMS   curPortParams;  /* current port parameters */
    GT_BOOL isCascadePort; /* TXQ not support cascade port yet. skip TXQ configurations for cascade ports*/
    GT_BOOL apEnabled = GT_FALSE;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    if (ifMode > CPSS_PORT_INTERFACE_MODE_NA_E && speed > CPSS_PORT_SPEED_NA_E)
    {
        /* check if AP is enabled */
        rc = cpssDxChPortApEnableGet(devNum, 0, &apEnabled);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "cpssDxChPortModeSpeedSet - try to set AP port but can not get FW status");
        }
        ifMode -= CPSS_PORT_INTERFACE_MODE_NA_E;
        speed -= CPSS_PORT_SPEED_NA_E;
    }

    if(powerUp == GT_TRUE)
    {
        rc = prvCpssCommonPortIfModeToHwsTranslate(devNum, ifMode, speed, &portMode);
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChSerdesRefClockTranslateCpss2Hws(devNum, &refClock);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    else
    {/* to prevent warnings */
        portMode = NON_SUP_MODE;
        refClock = MHz_156;
    }

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
    {
        if(refClock == MHz_156)
        {
            refClockSource = SECONDARY_LINE_SRC;
        }
        else if(refClock == MHz_25)
        {
            refClockSource = PRIMARY_LINE_SRC;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "cpssDxChPortModeSpeedSet - wrong SerDes reference clock frequency");
        }
    }

    for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portsBmp, portNum))
        {
            PRV_CPSS_DXCH_MAC_IN_PHY_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,
                                                                         portNum,
                                                                         portMacMap);
        }
        else
        {
            continue;
        }

        /*check that port does not serve as additional preeemption  channel*/
        rc = prvCpssTxqPreemptionUtilsPortConfigurationAllowedGet(devNum,portNum);
        if(rc!=GT_OK)
        {
            return rc;
        }


        portIfModePtr = &(PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacMap));
        portSpeedPtr = &(PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacMap));

        isRemotePort = prvCpssDxChPortRemotePortCheck(devNum, portNum);

        if(powerUp == GT_FALSE)
        {
            if ((*portSpeedPtr == CPSS_PORT_SPEED_NA_E) || (*portIfModePtr == CPSS_PORT_INTERFACE_MODE_NA_E))
            {
                /* port already deleted */
                return GT_OK;
            }
        }
        else
        {
            if (GT_FALSE == isRemotePort)
            {
                if (((*portSpeedPtr != CPSS_PORT_SPEED_NA_E) && (*portSpeedPtr != CPSS_PORT_SPEED_NA_HCD_E)) ||
                    ((*portIfModePtr != CPSS_PORT_INTERFACE_MODE_NA_E)&& (*portIfModePtr != CPSS_PORT_INTERFACE_MODE_NA_HCD_E)))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST,"port already configured - error - delete and create again: portNum = %d\n", portNum);
                }
            }

            rc = prvCpssDxChPortIsAlreadyConfigured(devNum, portNum, powerUp, *portIfModePtr, *portSpeedPtr, ifMode, speed, &isPortAlreadyConfigured);
            if (rc != GT_OK)
            {
                return rc;
            }
            if(isPortAlreadyConfigured)
            {
                continue;
            }
        }

        /* remote ports do not have MAC,only flush txQ*/
        if (GT_TRUE == isRemotePort)
        {
            if(GT_TRUE==powerUp)
            {
                if((speed!=CPSS_PORT_SPEED_REMOTE_E)||(ifMode!=CPSS_PORT_INTERFACE_MODE_REMOTE_E))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                        "port %d speed/mode  can only be set to CPSS_PORT_SPEED_REMOTE_E/CPSS_PORT_INTERFACE_MODE_REMOTE_E\n",
                         portNum);
                }
            }

            rc = pvrCpssDxChPortRemotePortConfigSet(devNum, portNum, powerUp);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"pvrCpssDxChPortRemotePortConfigSet : failed for portNum = %d\n", portNum);
            }

            continue;
        }

        rc = hwsPortModeParamsSetMode(devNum,0, portMacMap, powerUp ? portMode : NON_SUP_MODE);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "hwsPortModeParamsSetMode - failed ");
        }

        if(powerUp == GT_TRUE)
        {
            rc = prvCpssDxChPortSerdesFreeCheck(devNum, portNum, ifMode, speed);
            if(rc != GT_OK)
            {
                 return rc;
            }
        }

        rc = prvCpssFalconTxqUtilsIsCascadePort(devNum, portNum,&isCascadePort,NULL);
        if(rc != GT_OK)
        {
             return rc;
        }

        if(powerUp == GT_FALSE)
        {
            /* we remove the ports, so need to use current interface and speed
             * if configured, not the values from the caller */
            if((CPSS_PORT_INTERFACE_MODE_NA_E != *portIfModePtr) &&
                                        (CPSS_PORT_SPEED_NA_E != *portSpeedPtr))
            {
                ifMode = *portIfModePtr;
                speed =  *portSpeedPtr;
            }
            else
            {
                continue; /* port is already power down , go to next port */
            }
        }

        rc = prvCpssCommonPortInterfaceSpeedGet(devNum, portMacMap,
                                                              ifMode, speed,
                                                              &supported);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(!supported)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        /* Invalidate LED port position */
        rc = prvCpssDxChFalconLedStreamPortPositionSet(devNum, portNum, 0x3F);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChFalconLedStreamPortPositionSet, portNum = %d\n", portNum);
        }

        /****************************************************************************************
            PART 1 - port disable

            This configuration will trigger:
                  a. The MAC will issue an indication towards the EGF
                     to stop sending packets to it.
                  b. The MAC will issue ready to the TXFIFO and thus the
                     traffic on the way to the port will be flushed.

            PART 2 - Check all traffic to the port was flushed

            1. Read Port<%n>DescCounter in the TXQ and wait for it to reach 0.
               This counter indicates how many descriptors are allocated for the port in the TXQ queues.
            2. Read TxDMA Port <%n> Credit Counter register in the TXQ and wait for it to reach the corresponding configured descriptor credits threshold in the TXDMA (Descriptor Credits SCDMA %p).
            3. In the TXDMA regfile - read the TXFIFO credits counters status and wait for it to reach zero.
               There is a separate counter for headers (scdma_%p_header_txfifo_counter) and payloads (scdma_%p_payload_txfifo_counter).
               These counters count the amount of data that is inside the TXDMA/TXFIFO on its way to the port.

        *****************************************************************************************/

        rc = prvCpssDxChPortStateDisableAndGet(devNum, portNum, &portStateStc);
        if (rc != GT_OK)
        {
            return rc;
        }

        fullConfig = GT_TRUE;
        if(isSupportUsxPort(devNum,portMacMap))
        {
            rc = hawkUsxConfig(devNum, portNum, ifMode, portMode, powerUp, &fullConfig);
            if (GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }

        /****************************************************************************************
            PART 3 - Remove resources allocated to the port

        *****************************************************************************************/

        /*
            In order to avoid ambiguity with MIB counters need
            to reset it before port power down.
        */
        if (powerUp == GT_FALSE)
        {
            CPSS_PORT_MAC_MTI_COUNTER_SET_STC  mtiMibStc;

            rc = cpssDxChPortMacCountersOnMtiPortGet(devNum, portNum, &mtiMibStc);
            if (GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"cpssDxChPortMacCountersOnMtiPortGet : MIB read for portNum = %d\n", portNum);
            }
        }

        /* state that the TX resources of this portNum are 'removed' */
        rc = prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapSet(devNum, portNum,GT_FALSE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapSet : disable, portNum = %d\n",
                portNum);
        }

        rc = prvCpssDxChPortCaelumPortDelete(devNum, portNum, ifMode, speed,
                                            fullConfig);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortCaelumPortDelete, portNum = %d, ifMode = %d, speed = %d\n", portNum, ifMode, speed);
        }

        /* save new interface mode in DB after prvCpssDxChPortBcat2PortDelete */
        *portIfModePtr = CPSS_PORT_INTERFACE_MODE_NA_E;
        *portSpeedPtr = CPSS_PORT_SPEED_NA_E;

        if(!powerUp)
        {
            /* IMPORTANT : UNLIKE sip5_20 in function prvCpssDxChPortCaelumModeSpeedSet(...))

                we NOT need to manipulate the 'DB' of portEgfForceStatusBitmapPtr ,
                because we are protected by next 2 functions :
                prvCpssDxChEgfPortLinkFilterRestoreApplicationIfAllowed(...) and
                prvCpssDxChEgfPortLinkFilterForceLinkDown().
            */
            rc = cpssDxChPortEnableSet(devNum, portNum, portStateStc.portEnableState);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortStateEnableSet, portNum = %d\n", portNum);
            }

            /* set to default */
            prvCpssDxChPortTypeSet(devNum, portNum, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E);

            continue;
        }

        /*Set txQ thresholds for credit management  and BP*/
        rc = prvCpssDxChTxqSetPortSdqThresholds(devNum, portNum,  speed);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChTxqSetPortSdqThresholds, portNum = %d\n", portNum);
        }

        /*Set txQ thresholds for long queue management*/
        rc = prvCpssDxChTxqBindPortQueuesToPdsProfile(devNum, portNum,  speed,NULL);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChTxqBindPortQueuesToPdsProfile, portNum = %d\n", portNum);
        }

        rc = prvCpssDxChFalconPortQueueRateSet(
            devNum, portNum, speed);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(
                rc,"error in prvCpssDxChFalconPortQueueRateSet, portNum %d speed %d\n",
                portNum, speed);
        }

        rc = prvCpssDxChTxqSip6_10PreemptionEnableGet(devNum, portNum, 0, &isPreemptionEnabled);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChTxqSip6_10PreemptionEnableGet: error for portNum = %d\n", portNum);
        }

        cpssOsMemSet(&portInitInParam, 0,sizeof(portInitInParam));
        portInitInParam.lbPort = GT_FALSE;
        portInitInParam.refClock = refClock;
        portInitInParam.refClockSource = refClockSource;
        portInitInParam.isPreemptionEnabled = isPreemptionEnabled;
        portInitInParam.apEnabled = apEnabled;
        if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portMacMap, portMode, &curPortParams))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
        }

        portInitInParam.portFecMode = curPortParams.portFecMode;
        portInitInParam.portSpeed = prvCpssCommonPortSpeedEnumToHwsSpeedConvert(speed);

        if (NULL == portMgrDbPtr)
        {
            portInitInParam.autoNegEnabled = GT_FALSE;
        }
        else
        {
            portInitInParam.autoNegEnabled = (portMgrDbPtr->autoNegotiation.inbandEnable && !portMgrDbPtr->autoNegotiation.byPassEnable);
        }

        rc = mvHwsPortFlavorInit(devNum, 0, portMacMap, portMode, &portInitInParam);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"mvHwsPortFlavorInit: error for portNum = %d\n", portNum);
        }

        if(fullConfig)
        {
            /* set serdes Tx/Rx Tuning values if SW DB values initialized */
            rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
            {
                /* set the Polarity values on Serdeses if SW DB values initialized */
                if ( curPortParams.numOfActLanes >= MV_HWS_MAX_LANES_NUM_PER_PORT )
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"error before prvCpssDxChPortSerdesPolaritySet - wrong array size, portNum = %d\n", portNum);
                }
                rc = prvCpssDxChPortSerdesPolaritySet(devNum, 0, &curPortParams.activeLanesList[0], curPortParams.numOfActLanes);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortSerdesPolaritySet, portNum = %d\n", portNum);
                }
            }
        }

        if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
        {
            /* Ironman
               Configuration of DP, PB, PCA and MIF units. */
            rc = prvCpssDxChPortDpIronmanPortConfigure(
                devNum, portNum, ifMode, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(
                    rc,
                    "error in prvCpssDxChPortDpIronmanPortConfigure, portNum = %d\n",
                    portNum);
            }
        }
        else if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            /* Hawk and above.
               Configuration of DP, PB and PCA units. */
            rc = prvCpssDxChTxPortSpeedPizzaResourcesPortConfigure(
                devNum, portNum, ifMode, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(
                    rc,
                    "error in prvCpssDxChTxPortSpeedPizzaResourcesHawkPortConfigure, portNum = %d\n",
                    portNum);
            }
        }

        prvCpssDxChPortTypeSet(devNum, portNum, ifMode, speed);

        /* Configures type of the port connected to the LED */
        rc = prvCpssDxChLedPortTypeConfig(devNum, portNum,powerUp);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChLedPortTypeConfig, portNum = %d\n", portNum);
        }

        /* save new interface mode in DB */
        *portIfModePtr = ifMode;
        *portSpeedPtr = speed;

        /* state that the TX resources of this portNum are 'valid' */
        /* and therefore we allow the application to set 'EGF link up' */
        rc = prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapSet(devNum, portNum,GT_TRUE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChSip6QueueBaseIndexToValidTxCreditsMapSet : Enable, portNum = %d\n",
                portNum);
        }

        rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
        {
            rc = prvCpssDxChPortStateRestore(devNum, portNum, &portStateStc);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssDxChPortStateEnableSet, portNum = %d\n", portNum);
            }
        }

    } /* for(portNum = 0; */

    return GT_OK;
}

GT_STATUS prvCpssDxChDebugRavenCounterGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    ravenIndex
)
{
    GT_STATUS               rc;         /* return code */
    GT_PHYSICAL_PORT_NUM    portNum;    /* iterator */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */
    GT_BOOL                 nonZeroCounterFound = GT_FALSE;
    GT_U32                  dropCounterValueArr[10];
    GT_U32 i;

    for(portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum += 8)
    {
        rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(devNum, portNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &portMacMap);
        if(rc != GT_OK)
        {
            continue;
        }

        if((portMacMap / 16) != ravenIndex)
        {
            continue;
        }

        rc = mvHwsMpfSauStatusGet(devNum,portMacMap, 10, dropCounterValueArr);
        if(rc != GT_OK)
        {
            return rc;
        }

        for(i = 0; i < 10; i++)
        {
            if(dropCounterValueArr[i] != 0)
            {
                if(i < 8)
                {
                    cpssOsPrintf("Raven Index[%d], MPF channel = %d, DropCounter[50G/100G] = 0x%8.8x\n", ravenIndex, (portMacMap + i), dropCounterValueArr[i]);
                    nonZeroCounterFound = GT_TRUE;
                }
                else if(i == 8)
                {
                    cpssOsPrintf("Raven Index[%d], MPF channel = %d, DropCounter[200G/400G on index 0] = 0x%8.8x\n", ravenIndex, (portMacMap + i), dropCounterValueArr[i]);
                    nonZeroCounterFound = GT_TRUE;
                }
                else
                {
                    cpssOsPrintf("Raven Index[%d], MPF channel = %d, DropCounter[200G on index 4] = 0x%8.8x\n", ravenIndex, (portMacMap + i), dropCounterValueArr[i]);
                    nonZeroCounterFound = GT_TRUE;
                }
            }
        }
    }

    if(nonZeroCounterFound == GT_FALSE)
    {
        cpssOsPrintf("Raven Index[%d] - all drop counnters are zero\n", ravenIndex);
    }

    return GT_OK;
}
