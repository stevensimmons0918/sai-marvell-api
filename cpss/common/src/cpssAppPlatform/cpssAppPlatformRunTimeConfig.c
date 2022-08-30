/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
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
* @file cpssAppPlatfomRunTimeConfig.c
*
* @brief CPSS Application Platform - run time configuration
*
* @version   1
********************************************************************************
*/

#include <profiles/cpssAppPlatformProfile.h>
#include <ezbringup/cpssAppPlatformEzBringupTools.h>
#include <ezbringup/cpssAppPlatformMpdTool.h>
#include <ezbringup/cpssAppPlatformMpdAndPp.h>
#include <cpssAppPlatformSysConfig.h>
#include <cpssAppPlatformRunTimeConfig.h>
#include <cpssAppPlatformPortInit.h>
#include <cpssAppPlatformPpConfig.h>

#include <gtOs/gtOsTask.h>
#include <gtOs/gtOsTimer.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsSem.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmiPreInit.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortAp.h>

#include <extUtils/common/cpssEnablerUtils.h>
#include <cpssAppUtilsEvents.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

typedef struct{

    /** The process unified event handle (got from CpssEventBind). */
    GT_UINTPTR evHndl;

    /* event handler number */
    GT_U32 index;

    /* event handler task Id */
    GT_TASK taskId;

    /* callback function */
    CPSS_APP_PLATFORM_USER_EVENT_CB_FUNC callbackFuncPtr;

    /* List of Events handled */
    CPSS_UNI_EV_CAUSE_ENT *eventListPtr;

    /* Number of events in eventListPtr*/
    GT_U8 numOfEvents;

} EVENT_HANDLER_PARAM_STC;

/* TBD - remove MAX limit. change to dynamic allocation */
#define CPSS_APP_PLATFORM_EVENT_HANDLER_MAX_CNS 10

static EVENT_HANDLER_PARAM_STC eventHandlerParams[CPSS_APP_PLATFORM_EVENT_HANDLER_MAX_CNS];
static GT_U8 eventHandlerNum = 0;

/* default HWS squelch for some of the 10G port modes on Aldrin2 DB boards can be problematic,
   so there is a need to set a custom value */
GT_BOOL port_serdes_squelch_WA_enable = GT_TRUE;

static GT_U32 taskCreated = 0;
static GT_U32 eventHandlerReset = 0;
#define FREE_TASK_CNS   0xFFFFFFFF

static GT_CHAR *uniEvName[CPSS_UNI_EVENT_COUNT_E] = {UNI_EV_NAME};

GT_BOOL eventHandlerInitDone = GT_FALSE;

GT_U32  disableAutoPortMgr;
GT_BOOL portMgr = GT_FALSE;


GT_CHAR * CPSS_FEC_2_STR
(
    CPSS_PORT_FEC_MODE_ENT fecMode
)
{
    typedef struct
    {
        CPSS_PORT_FEC_MODE_ENT fecModeEnm;
        GT_CHAR            *fecStr;
    }APPDEMO_FEC_TO_STR_STC;

    static APPDEMO_FEC_TO_STR_STC prv_fec2str[] =
    {
         {  CPSS_PORT_FEC_MODE_ENABLED_E,              "FC    "}
        ,{  CPSS_PORT_FEC_MODE_DISABLED_E,             "NONE  "}
        ,{  CPSS_PORT_RS_FEC_MODE_ENABLED_E,           "RS    "}
        ,{  CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E,   "RS544 "}
    };

    GT_U8 i;
    for (i = 0 ; prv_fec2str[i].fecModeEnm != CPSS_PORT_FEC_MODE_LAST_E; i++)
    {
        if (prv_fec2str[i].fecModeEnm == fecMode)
        {
            return prv_fec2str[i].fecStr;
        }
    }
    return "-----";
}

GT_CHAR * CPSS_SPEED_2_STR
(
    CPSS_PORT_SPEED_ENT speed
)
{
    typedef struct
    {
        CPSS_PORT_SPEED_ENT speedEnm;
        GT_CHAR            *speedStr;
    }APPDEMO_SPEED_TO_STR_STC;

    static APPDEMO_SPEED_TO_STR_STC prv_speed2str[] =
    {
         {  CPSS_PORT_SPEED_10_E,     "10M  "}
        ,{  CPSS_PORT_SPEED_100_E,    "100M "}
        ,{  CPSS_PORT_SPEED_1000_E,   "1G   "}
        ,{  CPSS_PORT_SPEED_2500_E,   "2.5G "}
        ,{  CPSS_PORT_SPEED_5000_E,   "5G   "}
        ,{  CPSS_PORT_SPEED_10000_E,  "10G  "}
        ,{  CPSS_PORT_SPEED_11800_E,  "11.8G"}
        ,{  CPSS_PORT_SPEED_12000_E,  "12G  "}
        ,{  CPSS_PORT_SPEED_12500_E,  "12.5G"}
        ,{  CPSS_PORT_SPEED_13600_E,  "13.6G"}
        ,{  CPSS_PORT_SPEED_15000_E,  "15G  "}
        ,{  CPSS_PORT_SPEED_16000_E,  "16G  "}
        ,{  CPSS_PORT_SPEED_20000_E,  "20G  "}
        ,{  CPSS_PORT_SPEED_22000_E,  "22G  "}
        ,{  CPSS_PORT_SPEED_23600_E,  "23.6G"}
        ,{  CPSS_PORT_SPEED_25000_E,  "25G  "}
        ,{  CPSS_PORT_SPEED_29090_E,  "29.1G"}
        ,{  CPSS_PORT_SPEED_40000_E,  "40G  "}
        ,{  CPSS_PORT_SPEED_47200_E,  "47.2G"}
        ,{  CPSS_PORT_SPEED_50000_E,  "50G  "}
        ,{  CPSS_PORT_SPEED_200G_E,   "200G "}
        ,{  CPSS_PORT_SPEED_75000_E,  "75G  "}
        ,{  CPSS_PORT_SPEED_100G_E,   "100G "}
        ,{  CPSS_PORT_SPEED_107G_E,   "107G "}
        ,{  CPSS_PORT_SPEED_140G_E,   "140G "}
        ,{  CPSS_PORT_SPEED_102G_E,   "102G "}
        ,{  CPSS_PORT_SPEED_52500_E,  "52.5G"}
        ,{  CPSS_PORT_SPEED_26700_E,  "26.7G"}
        ,{  CPSS_PORT_SPEED_400G_E,   "400G "}
        ,{  CPSS_PORT_SPEED_NA_E,     "NA   "}
    };

    GT_U8 i;
    for (i = 0 ; prv_speed2str[i].speedEnm != CPSS_PORT_SPEED_NA_E; i++)
    {
        if (prv_speed2str[i].speedEnm == speed)
        {
            return prv_speed2str[i].speedStr;
        }
    }
    return "-----";
}

GT_CHAR * CPSS_IF_2_STR
(
    CPSS_PORT_INTERFACE_MODE_ENT ifEnm
)
{
    typedef struct
    {
        CPSS_PORT_INTERFACE_MODE_ENT ifEnm;
        GT_CHAR                     *ifStr;
    }APPDEMO_IF_TO_STR_STC;

    APPDEMO_IF_TO_STR_STC prv_prvif2str[] =
    {
         {  CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E, "REDUCED_10BIT"  }
        ,{  CPSS_PORT_INTERFACE_MODE_REDUCED_GMII_E,  "REDUCED_GMII"   }
        ,{  CPSS_PORT_INTERFACE_MODE_MII_PHY_E,       "MII_PHY     "   }
        ,{  CPSS_PORT_INTERFACE_MODE_MII_E,           "MII         "   }
        ,{  CPSS_PORT_INTERFACE_MODE_GMII_E,          "GMII        "   }
        ,{  CPSS_PORT_INTERFACE_MODE_SGMII_E,         "SGMII       "   }
        ,{  CPSS_PORT_INTERFACE_MODE_XGMII_E,         "XGMII       "   }
        ,{  CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E,   "LOCAL_XGMII "   }
        ,{  CPSS_PORT_INTERFACE_MODE_MGMII_E,         "MGMII       "   }
        ,{  CPSS_PORT_INTERFACE_MODE_QSGMII_E,        "QSGMII      "   }
        ,{  CPSS_PORT_INTERFACE_MODE_QX_E,            "QX          "   }
        ,{  CPSS_PORT_INTERFACE_MODE_HX_E,            "HX          "   }
        ,{  CPSS_PORT_INTERFACE_MODE_RXAUI_E,         "RXAUI       "   }
        ,{  CPSS_PORT_INTERFACE_MODE_100BASE_FX_E,    "100BASE_FX  "   }
        ,{  CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,    "1000BASE_X  "   }
        ,{  CPSS_PORT_INTERFACE_MODE_XLG_E,           "XLG         "   }
        ,{  CPSS_PORT_INTERFACE_MODE_HGL_E,           "HGL         "   }
        ,{  CPSS_PORT_INTERFACE_MODE_CHGL_12_E,       "CHGL_12     "   }
        ,{  CPSS_PORT_INTERFACE_MODE_KR_E,            "KR          "   }
        ,{  CPSS_PORT_INTERFACE_MODE_KR2_E,           "KR2         "   }
        ,{  CPSS_PORT_INTERFACE_MODE_KR4_E,           "KR4         "   }
        ,{  CPSS_PORT_INTERFACE_MODE_KR8_E,           "KR8         "   }
        ,{  CPSS_PORT_INTERFACE_MODE_SR_LR_E,         "SR_LR       "   }
        ,{  CPSS_PORT_INTERFACE_MODE_SR_LR2_E,        "SL_LR2      "   }
        ,{  CPSS_PORT_INTERFACE_MODE_SR_LR4_E,        "SL_LR4      "   }
        ,{  CPSS_PORT_INTERFACE_MODE_SR_LR8_E,        "SL_LR8      "   }
        ,{  CPSS_PORT_INTERFACE_MODE_ILKN4_E,         "ILKN4       "   }
        ,{  CPSS_PORT_INTERFACE_MODE_ILKN8_E,         "ILKN8       "   }
        ,{  CPSS_PORT_INTERFACE_MODE_ILKN12_E,        "ILKN12      "   }
        ,{  CPSS_PORT_INTERFACE_MODE_ILKN16_E,        "ILKN16      "   }
        ,{  CPSS_PORT_INTERFACE_MODE_ILKN24_E,        "ILKN24      "   }
        ,{  CPSS_PORT_INTERFACE_MODE_XHGS_E,          "XHGS        "   }
        ,{  CPSS_PORT_INTERFACE_MODE_XHGS_SR_E ,      "XHGS-SR     "   }
        ,{  CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E,     "MLG"  }
        ,{  CPSS_PORT_INTERFACE_MODE_KR_C_E,          "KR_C        "   }
        ,{  CPSS_PORT_INTERFACE_MODE_CR_C_E,          "CR_C        "   }
        ,{  CPSS_PORT_INTERFACE_MODE_KR2_C_E,         "KR2_C       "   }
        ,{  CPSS_PORT_INTERFACE_MODE_CR2_C_E,         "CR2_C       "   }
        ,{  CPSS_PORT_INTERFACE_MODE_CR_E,            "CR          "   }
        ,{  CPSS_PORT_INTERFACE_MODE_CR_E,            "CR2         "   }
        ,{  CPSS_PORT_INTERFACE_MODE_CR4_E,           "CR4         "   }
        ,{  CPSS_PORT_INTERFACE_MODE_CR8_E,           "CR8         "   }
        ,{  CPSS_PORT_INTERFACE_MODE_KR_S_E,          "KR_S        "   }
        ,{  CPSS_PORT_INTERFACE_MODE_CR_S_E,          "CR_S        "   }
        ,{  CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E,   "USX_OUSGMII "   }

        ,{  CPSS_PORT_INTERFACE_MODE_NA_E,            "NA          "   }
    };

    GT_U8 i;
    for (i = 0 ; prv_prvif2str[i].ifEnm != CPSS_PORT_INTERFACE_MODE_NA_E; i++)
    {
        if (prv_prvif2str[i].ifEnm == ifEnm)
        {
            return prv_prvif2str[i].ifStr;
        }
    }
    return "------------";
}

GT_CHAR * CPSS_MAPPING_2_STR
(
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mapEnm
)
{
    typedef struct
    {
        CPSS_DXCH_PORT_MAPPING_TYPE_ENT mapEnm;
        GT_CHAR                        *mapStr;
    }APPDEMO_MAPPING_TO_STR_STC;


    static APPDEMO_MAPPING_TO_STR_STC prv_mappingTypeStr[] =
    {
             { CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,           "ETHERNET" }
            ,{ CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,               "CPU-SDMA" }
            ,{ CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,           "ILKN-CHL" }
            ,{ CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,   "REMOTE-P" }
            ,{ CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E,                "--------" }
    };
    GT_U8 i;
    for (i = 0 ; prv_mappingTypeStr[i].mapEnm != CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E; i++)
    {
        if (prv_mappingTypeStr[i].mapEnm == mapEnm)
        {
            return prv_mappingTypeStr[i].mapStr;
        }
    }
    return "--------";
}

/**
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
* @internal cpssAppPlatfromPpEvTreat function
* @endinternal
*
* @brief   This routine handles PP events.
*
* @param[in] callbackFuncPtr - event callback function given by runTime profile.
* @param[in] devNum          - the device number.
* @param[in] uniEv           - Unified event number
* @param[in] evExtData       - Unified event additional information
*
* @retval GT_OK          - on success,
* @retval GT_FAIL        - otherwise.
*/
static GT_STATUS cpssAppPlatfromPpEvTreat
(
    CPSS_APP_PLATFORM_USER_EVENT_CB_FUNC callbackFuncPtr,
    GT_U8  devNum,
    GT_U32 uniEv,
    GT_U32 evExtData
)
{
    GT_STATUS            rc = GT_OK;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_PORT_SPEED_ENT apSpeed;
    GT_BOOL     hcdFound;
    GT_U32    physicalPortNum;
    CPSS_DXCH_PORT_AP_STATUS_STC apStatusDx;
    CPSS_PORT_INTERFACE_MODE_ENT apIfMode;

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        /* device was removed */
        return GT_OK;
    }
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    switch (uniEv)
    {
        case CPSS_PP_PORT_LINK_STATUS_CHANGED_E:
            portNum = (GT_U32)evExtData;
            if (cpssAppPlatformIsPortMgrPort(devNum))
            {
                rc = cpssAppPlatformPmPortLinkStatusChangeSignal(devNum, portNum);
                if (rc != GT_OK)
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssAppPlatformPmPortLinkStatusChangeSignal rc=%d\n", rc);
            }
            break;

        case CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E:
             if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
             {
                 /* this event is not handled for SIP6 devices */
                 break;
             }
             GT_ATTR_FALLTHROUGH;
        case CPSS_PP_PORT_SYNC_STATUS_CHANGED_E:
             GT_ATTR_FALLTHROUGH;
        case CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E:
             GT_ATTR_FALLTHROUGH;
        case CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E:
             portNum = (GT_U32)evExtData;
             if (cpssAppPlatformIsPortMgrPort(devNum))
             {
                 rc = cpssAppPlatformPmPortStatusChangeSignal(devNum, portNum, uniEv);
                 if (rc != GT_OK)
                     CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssAppPlatformPmPortStatusChangeSignal rc=%d\n", rc);
             }
             break;
        case CPSS_SRVCPU_PORT_802_3_AP_E:
             portNum = (GT_U32)evExtData;

             rc = cpssDxChPortPhysicalPortMapReverseMappingGet(devNum, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,
                     portNum, &physicalPortNum);
             CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortPhysicalPortMapReverseMappingGet);

             if (cpssAppPlatformIsPortMgrPort(devNum))
             {
                 cpssAppPlatformPmPortStatusChangeSignal(devNum, physicalPortNum, CPSS_SRVCPU_PORT_802_3_AP_E);
                 /* pizza allocation done inside port manager, so no need to continue */
                 break;
             }

             /* query resolution results */
             rc = cpssDxChPortApPortStatusGet(devNum, physicalPortNum, &apStatusDx);
             CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortApPortStatusGet);

             hcdFound = apStatusDx.hcdFound;
             apSpeed = apStatusDx.portMode.speed;
             apIfMode = apStatusDx.portMode.ifMode;

             /* resolution found - allocate pizza resources*/
             if(hcdFound)
             {
                 CPSS_PORT_SPEED_ENT speed;
                 CPSS_PORTS_BMP_STC portsBmp;
                 CPSS_PORT_INTERFACE_MODE_ENT ifMode;
                 PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
                 CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,physicalPortNum);

                 /* Check current spped */
                 rc = cpssDxChPortSpeedGet(devNum, physicalPortNum, &speed);
                 CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortSpeedGet);

                 /*if speed <= CPSS_PORT_SPEED_1000_E we get the value from HW; at this stage speed is not always update correctly;
                   assume pizza allocation cannot be less than 1G*/
                 if(speed <= CPSS_PORT_SPEED_1000_E)
                 {
                     speed = CPSS_PORT_SPEED_1000_E;
                 }

                 /*if pizza already configured, Release it if not the same speed */
                 if((speed != CPSS_PORT_SPEED_NA_HCD_E) && ((speed != apStatusDx.portMode.speed) && (CPSS_PORT_SPEED_20000_E != speed)))
                 {
                     /* Release pizza resources */
                     rc = cpssDxChPortInterfaceModeGet(devNum, physicalPortNum, &ifMode);
                     if(rc != GT_OK)
                     {
                         CPSS_APP_PLATFORM_LOG_ERR_MAC("CPSS_SRVCPU_PORT_802_3_AP_E - cpssDxChPortInterfaceModeGet:rc=%d,portNum=%d\n",rc, physicalPortNum);
                         return rc;
                     }

                     rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_FALSE, ifMode + CPSS_PORT_INTERFACE_MODE_NA_E, speed + CPSS_PORT_SPEED_NA_E);
                     if(rc != GT_OK)
                     {
                         CPSS_APP_PLATFORM_LOG_ERR_MAC("CPSS_SRVCPU_PORT_802_3_AP_E - cpssDxChPortModeSpeedSet:rc=%d,portNum=%d\n", rc, physicalPortNum);
                         return rc;
                     }
                 }
                 /* allocate pizza resources only if not configured earlier or speed changed */
                 if(speed == CPSS_PORT_SPEED_NA_HCD_E)
                 {
                     rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, apIfMode + CPSS_PORT_INTERFACE_MODE_NA_E, apSpeed + CPSS_PORT_SPEED_NA_E);
                     if(rc != GT_OK)
                     {
                         CPSS_APP_PLATFORM_LOG_ERR_MAC("CPSS_SRVCPU_PORT_802_3_AP_E - cpssDxChPortModeSpeedSet:rc=%d,portNum=%d\n", rc, physicalPortNum);
                         return rc;
                     }
                 }
             }
             else
             {
                 CPSS_APP_PLATFORM_LOG_ERR_MAC("CPSS_SRVCPU_PORT_802_3_AP_E - portNum=%d, no resolution\n",physicalPortNum);
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
             }

             break;

        default:
            break;
    }

    if(callbackFuncPtr != NULL)
    {
        rc = callbackFuncPtr(devNum, uniEv, evExtData);
    }

    return rc;
}

/*
* @internal cpssAppPlatformEventHandler function
* @endinternal
*
* @brief   This function is the event handler for CPSS Event-Request-Driven mode
*          (polling mode).
*
* @param[in] param               - pointer to event handler parameters.
*
* @retval GT_OK                  - on success.
* @retval GT_FAIL                - otherwise.
*/
static unsigned __TASKCONV cpssAppPlatformEventHandler
(
    GT_VOID_PTR param
)
{
    GT_STATUS           rc;                                         /* return code         */
    GT_U32              i;                                          /* iterator            */
    GT_UINTPTR          evHndl;                                     /* event handler       */
    GT_U32              evBitmapArr[CPSS_UNI_EV_BITMAP_SIZE_CNS];   /* event bitmap array  */
    GT_U32              evBitmap;                                   /* event bitmap 32 bit */
    GT_U32              evExtData;                                  /* event extended data */
    GT_U8               devNum;                                     /* device number       */
    GT_U32              uniEv;                                      /* unified event cause */
    GT_U32              evCauseIdx;                                 /* event index         */
    EVENT_HANDLER_PARAM_STC      *hndlrParamPtr;                             /* bind event array    */

    hndlrParamPtr = (EVENT_HANDLER_PARAM_STC*)param;
    evHndl        = hndlrParamPtr->evHndl;

    CPSS_APP_PLATFORM_LOG_DBG_MAC("cpssAppPlatformEventHandler[%d]: created \n", hndlrParamPtr->index);

    /* indicate that task start running */
    taskCreated = 1;
    cpssOsTimerWkAfter(1);

    while (1)
    {
        rc = cpssEventSelect(evHndl, NULL, evBitmapArr, (GT_U32)CPSS_UNI_EV_BITMAP_SIZE_CNS);
        if(eventHandlerReset)
        {
            break;
        }
        if(rc != GT_OK)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssEventSelect rc=%d\n", rc);
            continue;
        }
        for (evCauseIdx = 0; evCauseIdx < CPSS_UNI_EV_BITMAP_SIZE_CNS; evCauseIdx++)
        {
            if (evBitmapArr[evCauseIdx] == 0)
            {
                continue;
            }

            evBitmap = evBitmapArr[evCauseIdx];

            for (i = 0; evBitmap; evBitmap >>= 1, i++)
            {
                if ((evBitmap & 1) == 0)
                {
                    continue;
                }

                uniEv = (evCauseIdx << 5) + i;

                if (cpssEventRecv(evHndl, uniEv, &evExtData, &devNum) == GT_OK)
                {
                    CPSS_APP_PLATFORM_LOG_DBG_MAC("cpssEventRecv: %d <dev %d, %s, extData %d>\n",
                             hndlrParamPtr->index, devNum, uniEvName[uniEv], evExtData);

                    if (uniEv <= CPSS_SRVCPU_MAX_E)
                    {
                        rc = cpssAppPlatfromPpEvTreat(hndlrParamPtr->callbackFuncPtr, devNum, uniEv, evExtData);
                        if (rc != GT_OK)
                           CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssAppPlatfromPpEvTreat rc=%d portNum=%d\n", rc, evExtData);
                    }

                    rc= cpssAppUtilsGenEventCounterIncrement(devNum, uniEv, evExtData);
                    if(rc != GT_OK)
                    {
                        CPSS_APP_PLATFORM_LOG_ERR_MAC("prvAppPlatformGenEventCounterIncrement rc=%d\n", rc);
                    }

                }
            }
        }
    }

    CPSS_APP_PLATFORM_LOG_DBG_MAC("cpssAppPlatformEventHandler[%d]: thread exits\n",hndlrParamPtr->index);

    hndlrParamPtr->index = FREE_TASK_CNS;

    return 0;
}

/*
* @internal prvEventMaskSet function
* @endinternal
*
* @brief   Mask/Unmask user events for given device.
*
* @param [in] devNum       - CPSS Device Number.
* @param [in] operation    - Mask / Unmask option.
*
* @retval GT_OK            - on success.
* @retval GT_FAIL          - otherwise.
*/
GT_STATUS prvEventMaskSet
(
    IN GT_U8 devNum,
    IN CPSS_EVENT_MASK_SET_ENT operation
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    i  = 0;
    GT_U32    j  = 0;

    for (i = 0; i < eventHandlerNum; i++)
    {
        for (j = 0; j < eventHandlerParams[i].numOfEvents; j++)
        {
            if (eventHandlerParams[i].eventListPtr[j] > CPSS_UNI_EVENT_COUNT_E) /* last event */
            {
                continue;
            }

            rc = cpssEventDeviceMaskSet(devNum, eventHandlerParams[i].eventListPtr[j], operation);
            switch(rc)
            {
                case GT_NOT_INITIALIZED:
                    /* assume there are no FA/XBAR devices in the system ,
                       because there was initialization of the 'phase 1' for any FA/XBAR */

                    /* fall through */
                case GT_BAD_PARAM:
                    /* assume that this PP/FA/XBAR device not exists , so no DB of PP/FA/XBAR devices needed .... */

                    /* fall through */
                case GT_NOT_FOUND:
                    /* this event not relevant to this device */
                    rc = GT_OK;
                    break;

                case GT_OK:
                    break;

                default:
                    /* other real error */
                    break;
            }

            if (rc != GT_OK)
            {
                /* Mask/unMask fails. Continue to other events in the list */
                CPSS_APP_PLATFORM_LOG_ERR_MAC("%s failed for Event: %s\n",
                             (operation == CPSS_EVENT_MASK_E) ? "Masking":"UnMasking",
                             uniEvName[eventHandlerParams[i].eventListPtr[j]]);
                continue;
            }
        }
    }

    return rc;
}

/*
* @internal cpssAppPlatformUserEventHandlerInit function
* @endinternal
*
* @brief   create user event handlers.
*
* @param [in] eventHandlePtr   - Event Handle Ptr for RX,AUQ,Link change events.
*
* @retval GT_OK                - on success.
* @retval GT_FAIL              - otherwise.
*/
GT_STATUS cpssAppPlatformUserEventHandlerInit
(
    IN CPSS_APP_PLATFORM_EVENT_HANDLE_STC *eventHandlePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_CHAR   taskName[30];

    if(eventHandlerNum >= CPSS_APP_PLATFORM_EVENT_HANDLER_MAX_CNS)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("Number of User Event handlers exceeds supported limit: %d\n", CPSS_APP_PLATFORM_EVENT_HANDLER_MAX_CNS);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if((eventHandlePtr->numOfEvents == 0) ||
       (eventHandlePtr->eventListPtr[0] >= CPSS_UNI_EVENT_COUNT_E)) /* last event */
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("eventListPtr is Empty");
        return GT_OK;
    }

    /* call CPSS to bind the events under single handler */
    rc = cpssEventBind(eventHandlePtr->eventListPtr,
                       eventHandlePtr->numOfEvents,
                       &eventHandlerParams[eventHandlerNum].evHndl);
    if (rc == GT_ALREADY_EXIST)
        CPSS_APP_PLATFORM_LOG_ERR_MAC("One of the events is already bound to another handler\n");
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssEventBind);

    eventHandlerParams[eventHandlerNum].index = eventHandlerNum;
    eventHandlerParams[eventHandlerNum].callbackFuncPtr = eventHandlePtr->callbackFuncPtr;
    eventHandlerParams[eventHandlerNum].eventListPtr = eventHandlePtr->eventListPtr;
    eventHandlerParams[eventHandlerNum].numOfEvents = eventHandlePtr->numOfEvents;

    /* spawn all the event handler processes */
    cpssOsSprintf(taskName, "evHndl_%d", eventHandlerNum);

    taskCreated = 0;
    rc = cpssOsTaskCreate(taskName,
                      eventHandlePtr->taskPriority,
                      _32KB,
                      cpssAppPlatformEventHandler,
                      &eventHandlerParams[eventHandlerNum],
                      &eventHandlerParams[eventHandlerNum].taskId);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsTaskCreate);

    while(taskCreated == 0)
    {
        /* wait for indication that task created */
        cpssOsTimerWkAfter(1);
    }

    eventHandlerNum++;
    return rc;
}

/**
* @internal cpssAppPlatformEventHandlerReset function
* @endinternal
*
* @brief   This routine deletes event handlers when last PP
*          device is removed. It also deletes event counters.
*
* @param[in] devNum           - device number
*
* @retval GT_OK               - on success,
* @retval GT_FAIL             - otherwise.
*/
GT_STATUS cpssAppPlatformEventHandlerReset
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc     = GT_OK;
    GT_U32    i      = 0;
    GT_BOOL   isLast = GT_FALSE;

    /* mask events of the devices */
    rc = prvEventMaskSet(devNum, CPSS_EVENT_MASK_E);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvEventMaskSet);

    /* delete event handlers when last device is removed */
    PRV_IS_LAST_DEVICE_MAC(devNum, isLast);
    if(isLast == GT_TRUE)
    {
        eventHandlerReset = 1;

        /* changing the mask interrupts registers may invoke 'last minute' interrupts */
        /* let the task handle it before we continue */
        cpssOsTimerWkAfter(100);

        CPSS_APP_PLATFORM_LOG_INFO_MAC("cpssAppPlatformEventHandlerReset: destroy task handlers \n");

        for (i = 0; i < eventHandlerNum; i++)
        {
            /* call CPSS to destroy the events */
            rc = cpssEventDestroy(eventHandlerParams[i].evHndl);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssEventDestroy);

            while(eventHandlerParams[i].index != FREE_TASK_CNS)
            {
                /* this is indication that the thread is exited properly ... no need to kill it */
                cpssOsTimerWkAfter(1);
                CPSS_APP_PLATFORM_LOG_INFO_MAC("*");
            }

            eventHandlerParams[i].index = 0;
            eventHandlerParams[i].evHndl= 0;
            eventHandlerParams[i].taskId= 0;
            eventHandlerParams[i].callbackFuncPtr = NULL;
            eventHandlerParams[i].eventListPtr = NULL;
            eventHandlerParams[i].numOfEvents = 0;
        }

        /* state that reset of appDemo events finished */
        eventHandlerNum = 0;
        eventHandlerReset = 0;
        eventHandlerInitDone = GT_FALSE;
    }

    cpssAppUtilsUniEventsFree(devNum);

    return GT_OK;
}

/*
* @internal cpssAppPlatformUserEventsInit function
* @endinternal
*
* @brief   Initialize user event handlers.
*
* @param[in] inputProfileListPtr - Input profile list containing run time profile.
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if input profile is not run time profile.
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformUserEventsInit
(
    IN CPSS_APP_PLATFORM_PROFILE_STC  *inputProfileListPtr
)
{
    GT_STATUS rc  = GT_OK;
    GT_U8     dev = 0;
    CPSS_APP_PLATFORM_PROFILE_STC *profileListPtr = inputProfileListPtr;
    CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC *runTimeProfilePtr = NULL;
    CPSS_APP_PLATFORM_EVENT_HANDLE_STC    *eventHandlePtr = NULL;

    if(profileListPtr == NULL)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("Input Profile list is NULL\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Iterate through all run-time profiles and Create Event Handlers */
    PRV_CPSS_APP_START_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_RUNTIME_E)
    {
        if(profileListPtr->profileValue.runTimeInfoPtr == NULL)
        {
            /* RunTime profile is Empty. proceed to next profile */
            continue;
        }

        runTimeProfilePtr = profileListPtr->profileValue.runTimeInfoPtr;
        PRV_CPSS_APP_START_LOOP_RUNTIME_PROFILE_MAC(runTimeProfilePtr, CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E)
        {
            eventHandlePtr = runTimeProfilePtr->runtimeInfoValue.eventHandlePtr;
            if(eventHandlePtr == NULL)
            {
                /* event handle is NULL. proceed to next profile */
                continue;
            }
            rc = cpssAppPlatformUserEventHandlerInit(eventHandlePtr);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformUserEventHandlerInit);

            eventHandlerInitDone = GT_TRUE;
        }
        PRV_CPSS_APP_END_LOOP_RUNTIME_PROFILE_MAC(runTimeProfilePtr, CPSS_APP_PLATFORM_RUNTIME_PARAM_EVENT_HANDLE_E)
    }
    PRV_CPSS_APP_END_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_RUNTIME_E)

    if(eventHandlerInitDone)
    {
        /* unmask user events for all devices */
        for (dev = 0; dev < CPSS_APP_PLATFORM_MAX_PP_CNS; dev++)
        {
            if(!PRV_CPSS_IS_DEV_EXISTS_MAC(dev))
                continue;

            rc = prvEventMaskSet(dev, CPSS_EVENT_UNMASK_E);


            if (GT_OK != rc)
               CPSS_APP_PLATFORM_LOG_ERR_MAC("prvEventMaskSet rc=%d\n", rc);
        }
        CPSS_APP_PLATFORM_LOG_INFO_MAC("Event Handler Init Done ...\n");
    }

    return GT_OK;
}

/*
* @internal cpssAppPlatformPortManagerInit function
* @endinternal
*
* @brief   initialize port manager.
*
* @param[in] inputProfileListPtr - Input profile list containing run time profile.
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if input profile is not run time profile.
* @retval GT_FAIL                - otherwise.
*
*/
GT_STATUS cpssAppPlatformPortManagerInit
(
    IN CPSS_APP_PLATFORM_PROFILE_STC *inputProfileListPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_APP_PLATFORM_PROFILE_STC     *profileListPtr = inputProfileListPtr;
    CPSS_APP_PLATFORM_RUNTIME_PROFILE_STC     *runTimeProfilePtr = NULL;
    CPSS_APP_PLATFORM_PORT_MANAGER_HANDLE_STC *portManagerHandlePtr = NULL;

    if(profileListPtr == NULL)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("Input Profile list is NULL\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_APP_START_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_RUNTIME_E)
    {
        if(profileListPtr->profileValue.runTimeInfoPtr == NULL)
        {
            /* RunTime profile is Empty. proceed to next profile */
            continue;
        }

        runTimeProfilePtr = profileListPtr->profileValue.runTimeInfoPtr;
        PRV_CPSS_APP_START_LOOP_RUNTIME_PROFILE_MAC(runTimeProfilePtr, CPSS_APP_PLATFORM_RUNTIME_PARAM_PORT_MANAGER_HANDLE_E)
        {
            portManagerHandlePtr = runTimeProfilePtr->runtimeInfoValue.portManagerHandlePtr;
            if(portManagerHandlePtr == NULL)
            {
                /* port manager handle is NULL. proceed to next profile */
                continue;
            }

            /* create Port Manager task to initialize ports based on user events */
            rc = cpssAppPlatformPmTaskCreate(portManagerHandlePtr->taskPriority);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPmTaskCreate);

            portMgr = GT_TRUE;
            appPlatformDbEntryAdd("portMgr",1);
            break;
        }
        PRV_CPSS_APP_END_LOOP_RUNTIME_PROFILE_MAC(runTimeProfilePtr, CPSS_APP_PLATFORM_RUNTIME_PARAM_PORT_MANAGER_HANDLE_E)
        if(portMgr == GT_TRUE)
        {
            CPSS_APP_PLATFORM_LOG_INFO_MAC("Port manager Init Done ...\n");
            break;
        }
    }
    PRV_CPSS_APP_END_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_RUNTIME_E)

    return rc;
}

#define SIP6_ROW_DELEMITER \
                     "\n|----+------+-------+--------------+------------+----+-----+-----+-----+----|"

#define SIP6_15_ROW_DELEMITER \
                     "\n|----+------+-------+--------------+------------+----+-----+----|"

/*
* @internal cpssAppPlatformTrafficEnable function
* @endinternal
*
* @brief   Enable traffic for given ports.
*
* @param [in] *trafficEnablePtr - traffic enable handle ptr
*
* @retval GT_OK                - on success.
* @retval GT_FAIL              - otherwise.
*/

GT_STATUS cpssAppPlatformTrafficEnable
(
    IN CPSS_APP_PLATFORM_TRAFFIC_ENABLE_STC *trafficEnablePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    i;
    GT_U32    portIdx;
    GT_U32    maxPortIdx = 0;
    GT_U32    pipeId, tileId, dpIndex;
    GT_U8     devNum;
    GT_U32    portNum;
    GT_U32    mcFifo_0_1 = 0,
              mcFifo_2_3 = 0,
              mcFifo_0_1_2_3 = 0;

    static PortInitInternal_STC        portList[CPSS_MAX_PORTS_NUM_CNS];

    CPSS_APP_PLATFORM_PORT_CONFIG_STC *portTypeList = NULL;
    CPSS_APP_PLATFORM_PORT_CONFIG_STC *portInitPtr  = NULL;
    CPSS_PORTS_BMP_STC                 portsBmp;
    CPSS_DXCH_DETAILED_PORT_MAP_STC    portMap;
    CPSS_SYSTEM_RECOVERY_INFO_STC      system_recovery; /* holds system recovery information */
    GT_U32  doJustPrintPortMapping = 0;

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssSystemRecoveryStateGet);

    if(trafficEnablePtr->portTypeListPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    else
    {
        portTypeList = trafficEnablePtr->portTypeListPtr;
    }

    devNum = trafficEnablePtr->devNum;

    for (portIdx = 0 ; portIdx < sizeof(portList)/sizeof(portList[0]); portIdx++)
    {
        portList[portIdx].portNum       = APP_INV_PORT_CNS;
        portList[portIdx].speed         = CPSS_PORT_SPEED_NA_E;
        portList[portIdx].interfaceMode = CPSS_PORT_INTERFACE_MODE_NA_E;
        portList[portIdx].fecMode       = CPSS_PORT_FEC_MODE_DISABLED_E;
    }

    maxPortIdx = 0;
    portInitPtr = portTypeList;
    for (i = 0 ; portInitPtr->entryType != CPSS_APP_PLATFORM_PORT_LIST_TYPE_EMPTY_E; i++, portInitPtr++)
    {
        if (maxPortIdx >= CPSS_MAX_PORTS_NUM_CNS)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("port list has more ports than allowed - %d\n", maxPortIdx);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        switch (portInitPtr->entryType)
        {
            case CPSS_APP_PLATFORM_PORT_LIST_TYPE_INTERVAL_E:
                for (portNum = portInitPtr->portList[0] ; portNum <= portInitPtr->portList[1]; portNum += portInitPtr->portList[2])
                {
                    CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);
                    portList[maxPortIdx].portNum       = portNum;
                    portList[maxPortIdx].speed         = portInitPtr->speed;
                    portList[maxPortIdx].interfaceMode = portInitPtr->ifMode;
                    portList[maxPortIdx].fecMode       = portInitPtr->fecMode;
                    maxPortIdx++;
                }
            break;
            case CPSS_APP_PLATFORM_PORT_LIST_TYPE_LIST_E:
                for (portIdx = 0 ; portInitPtr->portList[portIdx] != APP_INV_PORT_CNS; portIdx++)
                {
                    portNum = portInitPtr->portList[portIdx];
                    CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);
                    portList[maxPortIdx].portNum       = portNum;
                    portList[maxPortIdx].speed         = portInitPtr->speed;
                    portList[maxPortIdx].interfaceMode = portInitPtr->ifMode;
                    portList[maxPortIdx].fecMode       = portInitPtr->fecMode;
                    maxPortIdx++;
                }
            break;
            default:
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                }
        }
    }

    if(portMgr == GT_TRUE)
    {
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("Configuring ports in Port Manager mode\n");
    }
    else
    {
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("Configuring ports in Legacy mode\n");
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes)
        {
            CPSS_APP_PLATFORM_LOG_PRINT_MAC (SIP6_ROW_DELEMITER);
            CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n| #  | Port | Speed | Fec  | MAC IF MODE  | map  Type  | mac| txq | pipe| tile| dp |");
            CPSS_APP_PLATFORM_LOG_PRINT_MAC(SIP6_ROW_DELEMITER);
        }
        else
        {
            CPSS_APP_PLATFORM_LOG_PRINT_MAC (SIP6_15_ROW_DELEMITER);
            CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n| #  | Port | Speed | MAC IF MODE  | map  Type  | mac| txq | dp |");
            CPSS_APP_PLATFORM_LOG_PRINT_MAC (SIP6_15_ROW_DELEMITER);
        }
    }
    else
    {
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n+----+------+-------+--------------+-----------------+------------------------------+");
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n| #  | Port | Speed |    IF        |   mapping Type  | rxdma txdma mac txq ilkn  tm |");
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n+----+------+-------+--------------+-----------------+------------------------------+");
    }

    for (portIdx = 0 ; portIdx < maxPortIdx; portIdx++)
    {
        portNum = portList[portIdx].portNum;

        if ( portList[portIdx].speed == CPSS_PORT_SPEED_NA_E || portList[portIdx].interfaceMode == CPSS_PORT_INTERFACE_MODE_NA_E )
        {
            doJustPrintPortMapping = 1; /* do print mapping only in case mode/speed specified as NA in profile */
        }

        rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, portNum, &portMap);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortPhysicalPortDetailedMapGet);

        if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n| %2d | %4d | %s | %s | %-15s | %5d %5d %3d %3d %4d %3d |",
                    portIdx, portNum,
                    doJustPrintPortMapping?"--NA-":CPSS_SPEED_2_STR(portList[portIdx].speed),
                    doJustPrintPortMapping?"  ---NA---  ":CPSS_IF_2_STR(portList[portIdx].interfaceMode),
                    CPSS_MAPPING_2_STR(portMap.portMap.mappingType),
                    portMap.portMap.rxDmaNum, portMap.portMap.txDmaNum,
                    portMap.portMap.macNum, portMap.portMap.txqNum,
                    portMap.portMap.ilknChannel, portMap.portMap.tmPortIdx);
        }
        else
        {
            if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes)
            {
                /* global pipeId (not relative to the tileId) */
                rc = prvCpssDxChHwPpGopGlobalMacPortNumToLocalMacPortInPipeConvert(devNum,
                        portMap.portMap.macNum/*global MAC port*/,
                        &pipeId, NULL);/*local MAC*/

                rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,
                        portMap.portMap.rxDmaNum/*global DMA port*/,
                        &dpIndex, NULL);/*local DMA*/

                tileId = pipeId / PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;

                CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n| %2d | %4d | %s | %s | %s | %-10s |%3d |%4d | %3d | %3d | %2d |",
                        portIdx, portNum,
                        doJustPrintPortMapping?"--NA-":CPSS_SPEED_2_STR(portList[portIdx].speed),
                        doJustPrintPortMapping?" -NA-  ":CPSS_FEC_2_STR(portList[portIdx].fecMode),
                        doJustPrintPortMapping?"  ---NA---  ":CPSS_IF_2_STR(portList[portIdx].interfaceMode),
                        CPSS_MAPPING_2_STR(portMap.portMap.mappingType),
                        portMap.portMap.macNum,
                        portMap.portMap.txqNum,
                        pipeId,
                        tileId,
                        dpIndex);
            }
            else
            {
                rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,
                    portMap.portMap.rxDmaNum/*global DMA port*/,
                    &dpIndex, NULL);/*local DMA*/
                CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n| %2d | %4d | %s | %s | %-10s |%3d |%4d | %2d |",
                                                portIdx, portNum
                                                ,doJustPrintPortMapping?"--NA-":CPSS_SPEED_2_STR(portList[portIdx].speed)
                                                ,doJustPrintPortMapping?"  ---NA---  ":CPSS_IF_2_STR(portList[portIdx].interfaceMode)
                                                ,CPSS_MAPPING_2_STR(portMap.portMap.mappingType)
                                                ,portMap.portMap.macNum
                                                ,portMap.portMap.txqNum
                                                ,dpIndex);
            }
        }

        if(doJustPrintPortMapping)
        {
            doJustPrintPortMapping = 0;
            continue;
        }

        PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp,portNum);

        if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
        {
            if(portMgr == GT_TRUE)
            {
                rc = cpssAppPlatformPortInitSeqStart(devNum, portNum, portList[portIdx].interfaceMode, portList[portIdx].speed, portList[portIdx].fecMode, GT_TRUE);
                if(GT_OK != rc)
                {
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssAppPlatformPortInitSeqStart(portNum=%d, ifMode=%s, speed=%s) :rc=%d\n",
                            portNum, CPSS_IF_2_STR(portList[portIdx].interfaceMode), CPSS_SPEED_2_STR(portList[portIdx].speed), rc);
                }
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPortInitSeqStart);

            }
            else /* configure ports in legacy mode */
            {
                rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE,
                        portList[portIdx].interfaceMode, portList[portIdx].speed);
                if(rc != GT_OK)
                {
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortModeSpeedSet(portNum=%d, ifMode=%s, speed=%s) :rc=%d\n",
                            portNum, CPSS_IF_2_STR(portList[portIdx].interfaceMode), CPSS_SPEED_2_STR(portList[portIdx].speed), rc);
                }
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortModeSpeedSet);

#if 0
                /* MC FIFO mapping for SIP devices */
                rc = cpssDxChPortTxMcFifoSet(devNum, portNum, portlist_aldrin2[portIdx].mcFifo);
#endif
                /* Enable/Disable ports based on profile */
                rc = cpssDxChPortEnableSet(devNum, portList[portIdx].portNum, GT_TRUE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortEnableSet);
            }

            /* WA for Aldrin2 DB board */
            if(!PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                if(port_serdes_squelch_WA_enable == GT_TRUE)
                {
                    if ((portList[portIdx].speed == CPSS_PORT_SPEED_10000_E) &&
                            (!prvCpssDxChPortRemotePortCheck(devNum, portNum)))
                    {
                        /* value 2 is the normalized value of signal_ok level (squelch) of value 100mV, to Avago API units */
                        rc = cpssDxChPortSerdesSquelchSet(devNum, portNum, 2);
                        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortSerdesSquelchSet);
                    }
                }
            }

            if (PRV_CPSS_DXCH_ALDRIN2_CHECK_MAC(devNum))
            {
                if (portList[portIdx].speed <= CPSS_PORT_SPEED_10000_E ||
                        portList[portIdx].speed == CPSS_PORT_SPEED_2500_E ||
                        portList[portIdx].speed == CPSS_PORT_SPEED_5000_E)
                {
                    rc = cpssDxChPortTxMcFifoSet(devNum, portNum, mcFifo_0_1%2);
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxMcFifoSet);
                    mcFifo_0_1++;
                }
                else
                {
                    rc = cpssDxChPortTxMcFifoSet(devNum, portNum, ((mcFifo_2_3%2) + 2));
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxMcFifoSet);
                    mcFifo_2_3++;
                }
            }
            else if ( PRV_CPSS_DXCH_ALDRIN_CHECK_MAC(devNum))
            {
                rc = cpssDxChPortTxMcFifoSet(devNum, portNum, mcFifo_0_1_2_3%4);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxMcFifoSet);
                mcFifo_0_1_2_3++;
            }
            else if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                /* Sip6 have FIFO 0-1 for all port speeds.
                 */
                rc = cpssDxChPortTxMcFifoSet(devNum, portNum, mcFifo_0_1%2);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxMcFifoSet);
                mcFifo_0_1++;
            }
        }
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        GT_U32    numOfNetIfs,mgUnitId;
        GT_U8     sdmaQueueIndex = 0;

        if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes)
        {
            CPSS_APP_PLATFORM_LOG_PRINT_MAC(SIP6_ROW_DELEMITER);
        }
        else
        {
            CPSS_APP_PLATFORM_LOG_PRINT_MAC(SIP6_15_ROW_DELEMITER);
        }
        rc = prvCpssDxChNetIfMultiNetIfNumberGet(devNum, &numOfNetIfs);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssDxChNetIfMultiNetIfNumberGet);

        /***************************/
        /* list the SDMA CPU ports */
        /***************************/
        for(/*portIdx*/;portIdx < (maxPortIdx+numOfNetIfs); portIdx++ , sdmaQueueIndex+=8)
        {
            rc = cpssDxChNetIfSdmaQueueToPhysicalPortGet(devNum,sdmaQueueIndex, &portNum);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChNetIfSdmaQueueToPhysicalPortGet);

            rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, portNum,/*OUT*/&portMap);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortPhysicalPortDetailedMapGet);

            rc = prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert(devNum,
                         portMap.portMap.rxDmaNum/*global DMA port*/, &dpIndex, NULL);/*local DMA*/
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssDxChHwPpDmaGlobalNumToLocalNumInDpConvert);

            PRV_CPSS_DXCH_NETIF_CHECK_NETIF_NUM_AND_CONVERT_TO_MG_UNIT_ID_MAC(devNum,(sdmaQueueIndex>>3),mgUnitId);

            if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes)
            {
                pipeId = dpIndex / 4;
                tileId = pipeId  / PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipesPerTile;

                CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n| %2d | %4d | CPU-SDMA queue [%3.1d..%3.1d]  MG[%2.1d]",portIdx,
                                                      portNum,
                                                      sdmaQueueIndex,
                                                      sdmaQueueIndex+7,
                                                      mgUnitId);

                CPSS_APP_PLATFORM_LOG_PRINT_MAC(" |%3d |%4d | %3d | %3d | %2d |"
                                                ,portMap.portMap.rxDmaNum
                                                ,portMap.portMap.txqNum
                                                ,pipeId
                                                ,tileId
                                                ,dpIndex);
            }
            else
            {
                CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n| %2d | %4d | CPU-SDMA queue [%3.1d..%3.1d]  MG[%2.1d]",portIdx,
                                                      portNum,
                                                      sdmaQueueIndex,
                                                      sdmaQueueIndex+7,
                                                      mgUnitId);

                CPSS_APP_PLATFORM_LOG_PRINT_MAC(" |%3d |%4d | %2d |"
                                                ,portMap.portMap.rxDmaNum
                                                ,portMap.portMap.txqNum
                                                ,dpIndex);
            }
        }

        if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfPipes)
        {
            CPSS_APP_PLATFORM_LOG_PRINT_MAC(SIP6_ROW_DELEMITER);
        }
        else
        {
            CPSS_APP_PLATFORM_LOG_PRINT_MAC(SIP6_15_ROW_DELEMITER);
        }
    }
    else
    {
        CPSS_APP_PLATFORM_LOG_PRINT_MAC("\n+----+------+-------+--------------+-----------------+------------------------------+------+");
    }

    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\nTraffic Enable Done...\n");

    if(GT_TRUE == ezbIsXmlLoaded())
    {
        /* check if we need to init the MPD to support phys */
        rc = cpssAppPlatformEzbMpdPpPortInit(trafficEnablePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformEzbMpdPpPortInit);

        rc = cpssAppPlatformEzbPortSpeedSet(trafficEnablePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformEzbPortSpeedSet);
    }
    else
    {
        if (trafficEnablePtr->phyMapListPtr == NULL)
        {
            CPSS_APP_PLATFORM_LOG_INFO_MAC("\nPhy Init - Skipping: External PHY Config not present\n");
            return rc;
        }

        CPSS_APP_PLATFORM_LOG_INFO_MAC("\nExternal Phy Init\n");
        for (portIdx = 0; portIdx < trafficEnablePtr->phyMapListPtrSize; portIdx++)
        {
            switch (trafficEnablePtr->phyMapListPtr[portIdx].phyInfo.phyItfType)
            {
            case CPSS_APP_PLATFORM_PHY_ITF_TYPE_SMI_E:
                rc = cpssDxChPhyPortSmiInterfaceSet(devNum, trafficEnablePtr->phyMapListPtr[portIdx].portNum,
                        trafficEnablePtr->phyMapListPtr[portIdx].phyInfo.phyItf.smiItf);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiInterfaceSet);
                break;
            case CPSS_APP_PLATFORM_PHY_ITF_TYPE_XSMI_E:
                rc = cpssDxChPhyPortXSmiInterfaceSet(devNum, trafficEnablePtr->phyMapListPtr[portIdx].portNum,
                        trafficEnablePtr->phyMapListPtr[portIdx].phyInfo.phyItf.xSmiItf);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiInterfaceSet);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

            }
            rc = cpssDxChPhyPortAddrSet(devNum, trafficEnablePtr->phyMapListPtr[portIdx].portNum, trafficEnablePtr->phyMapListPtr[portIdx].phyInfo.phyAddr);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortAddrSet);

            rc = cpssAppPlatformExtPhyConfig(devNum, trafficEnablePtr->phyMapListPtr[portIdx].portNum,
                    &(trafficEnablePtr->phyMapListPtr[portIdx].phyConfigData));
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformExtPhyConfig);
        }
    }
    CPSS_APP_PLATFORM_LOG_PRINT_MAC("\nExternal PHY Config End...\n");

    return rc;
}

CPSS_APP_PLATFORM_USER_EVENT_CB_FUNC cpssAppPlatformGenEventHandlePtr = NULL;

static  GT_TASK appRefTestsEventHandlerTid = 0;
/**
* @internal appRefEventsToTestsHandlerBind function
* @endinternal
*
* @brief   This routine creates new event handler for the unified event list.
*
* @param[in] cpssUniEventArr[]        - The CPSS unified event list.
* @param[in] arrLength                - The unified event list length.
* @param[in] operation                - type of  mask/unmask to do on the events
*                                       GT_OK on success, or
*                                       GT_FAIL if failed.
*/
GT_STATUS appRefEventsToTestsHandlerBind
(
    IN  CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    IN  GT_U32                      arrLength,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
)
{
    static EVENT_HANDLER_PARAM_STC eventHandlerParam;  /* parameters for task */
    GT_STATUS  rc = GT_OK;            /* return code */
    char    name[30] = "eventGenerationTask"; /* task name */
    GT_U32 i;

    if (operation == CPSS_EVENT_UNMASK_E && appRefTestsEventHandlerTid == 0)
    {
        /* call CPSS to bind the events under single handler */
        rc = cpssEventBind(cpssUniEventArr, arrLength, &eventHandlerParam.evHndl);
        if (rc == GT_ALREADY_EXIST)
            CPSS_APP_PLATFORM_LOG_ERR_MAC("One of the events is already bound to another handler\n");
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssEventBind);

        /* Create event handler task */
        eventHandlerParam.callbackFuncPtr = NULL;
        eventHandlerParam.index = 25;
        eventHandlerParam.numOfEvents = arrLength;
        eventHandlerParam.eventListPtr = cpssUniEventArr;
        /* spawn all the event handler processes */
        cpssOsSprintf(name, "evHndl_test_%d", eventHandlerParam.index);

        taskCreated = 0;
        rc = cpssOsTaskCreate(name,
                      200,
                      _32KB,
                      cpssAppPlatformEventHandler,
                      &eventHandlerParam,
                      &appRefTestsEventHandlerTid);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsTaskCreate);

        eventHandlerParam.taskId = appRefTestsEventHandlerTid;

        while(taskCreated == 0)
        {
            /* wait for indication that task created */
            cpssOsTimerWkAfter(1);
        }
    }

    for(i=0;i<arrLength;i++)
    {
        if (cpssUniEventArr[i] > CPSS_UNI_EVENT_COUNT_E) /* last event */
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        if(cpssUniEventArr[i] == CPSS_PP_PEX_HIT_DEFAULT_WIN_ERR_E)
        {
            continue;
        }

        /* call the CPSS to enable those interrupts in the HW of the device */
        rc = cpssEventDeviceMaskSet(0, cpssUniEventArr[i], operation);
        switch(rc)
        {
            case GT_NOT_INITIALIZED:
                /* assume there are no FA/XBAR devices in the system ,
                    because there was initialization of the 'phase 1' for any FA/XBAR */

                /* fall through */
            case GT_BAD_PARAM:
                /* assume that this PP/FA/XBAR device not exists , so no DB of PP/FA/XBAR devices needed .... */

                /* fall through */
            case GT_NOT_FOUND:
                /* this event not relevant to this device */
                rc = GT_OK;
                break;

            case GT_OK:
                break;

            default:
                /* other real error */
                break;
        }
    }

    return rc;
}

/**
* @internal appRefEventsToTestsHandlerUnbind function
* @endinternal
*
* @brief   This routine unbind the unified event list and remove
*          the event handler made in appRefEventsToTestsHandlerBind .
*
* @param[in] cpssUniEventArr[]        - The CPSS unified event list.
* @param[in] arrLength                - The unified event list length.
*/
GT_STATUS appRefEventsToTestsHandlerUnbind
(
    IN  CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    IN  GT_U32                      arrLength
)
{
    GT_STATUS  rc = GT_OK;            /* return code */

    if (appRefTestsEventHandlerTid != 0)
    {
        rc  = cpssEventUnBind(cpssUniEventArr, arrLength);
        if(rc != GT_OK)
        {
            cpssOsPrintf("cpssEventUnBind failed rc[%d] \n",rc);
            return rc ;
        }

         rc = osTaskDelete(appRefTestsEventHandlerTid);
        if (rc != GT_OK)
        {
            cpssOsPrintf("osTaskDelete : osTaskDelete: failed on [i=%d] \n",appRefTestsEventHandlerTid);
            return rc;
        }

        appRefTestsEventHandlerTid = 0;
    }
    return GT_OK;
}

