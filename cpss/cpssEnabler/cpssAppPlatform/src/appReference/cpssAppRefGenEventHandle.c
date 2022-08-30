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
* @file cpssAppRefGenEventHandle.c
*
* @brief this file contains the implementation of generic event handling functions
*
* @version   0
********************************************************************************
*/
#include <profiles/cpssAppPlatformProfile.h>
#include <cpssAppPlatformSysConfig.h>
#include <cpssAppPlatformRunTimeConfig.h>
#include <cpssAppPlatformPortInit.h>

#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrity.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfMii.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortAp.h>
#include <cpss/dxCh/dxChxGen/vnt/cpssDxChVnt.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>

#include <extUtils/rxEventHandler/rxEventHandler.h>
#include <extUtils/auEventHandler/auEventHandler.h>

#include <appReference/cpssAppRefUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define BUFF_LEN 20

static GT_CHAR *uniEvName[CPSS_UNI_EVENT_COUNT_E] = {UNI_EV_NAME};

GT_UINTPTR rxEventHandle = 0;
GT_UINTPTR auEventHandle = 0;

#define CPSS_APP_REF_RX_BUFF_ALLIGN_BITS_DEF  7
#define CPSS_APP_REF_RX_BUFF_ALLIGN_DEF      (1 << CPSS_APP_REF_RX_BUFF_ALLIGN_BITS_DEF)
#define CPSS_APP_REF_RX_BUFF_SIZE_DEF        800
/* Notification callback for catching events */
EVENT_NOTIFY_FUNC *notifyEventArrivedFunc = NULL;

#ifdef CHX_FAMILY

extern GT_STATUS appRefDxChDiagDataIntegrityCountersCbGet
(
    DXCH_DATA_INTEGRITY_EVENT_COUNTER_INCREMENT_FUNC **dataIntegrityEventCounterBlockGetCB
);
#endif

#ifndef ASIC_SIMULATION
extern CPSS_OS_SIG_SEM    serdesTuneSmId[CPSS_APP_PLATFORM_MAX_PP_CNS];
extern CPSS_PORTS_BMP_STC todoTuneBmp[CPSS_APP_PLATFORM_MAX_PP_CNS]; /* bitmap of ports to run tuning*/

extern GT_BOOL trxTraining;
extern GT_BOOL rxTraining;

extern GT_STATUS cpssAppRefSerdesTrainingTaskCreate
(
    GT_U8  devNum
);
#endif

/* flag to enable the printings of 'link change' */
#ifdef ASIC_SIMULATION
static GT_U32   printLinkChangeEnabled = 1;
#else  /*NOT_ASIC_SIMULATION*/
static GT_U32   printLinkChangeEnabled = 0;
#endif /*ASIC_SIMULATION*/

static GT_U32   printEeeInterruptInfo = 0;/* WM not supports this interrupt */

/**
* @internal appRefPrintPortEeeInterruptInfoSet function
* @endinternal
*
* @brief   function to allow set the flag of : printEeeInterruptInfo
*
* @param[in] enable                   - enable/disable the printings of ports 'EEE interrupts info'
*
* @retval GT_OK                    - on success
*/
GT_STATUS appRefPrintPortEeeInterruptInfoSet
(
    IN GT_U32   enable
)
{
    printEeeInterruptInfo = enable;
    return GT_OK;
};

#define MAX_REGISTER    5
static GT_U32                   numRegistrations=0;
static RX_PACKET_RECEIVE_CB_FUN rxPktReceiveCbArray[MAX_REGISTER];
extern GT_UINTPTR rxEventHanderHnd;

/* workaround */
static GT_STATUS prvAppRefDxChNetRxPacketRegisteredCb
(
  IN  GT_UINTPTR                cookie,
  IN  RX_EV_HANDLER_EV_TYPE_ENT evType,
  IN  GT_U8                     devNum,
  IN  GT_U8                     queueIdx,
  IN  GT_U32                    numOfBuff,
  IN  GT_U8                     *packetBuffs[],
  IN  GT_U32                    buffLen[],
  IN  GT_VOID                   *rxParamsPtr
)
{
    GT_U32  ii;
    evType = evType;
    cookie = cookie;

    for(ii = 0 ; ii < numRegistrations ; ii++)
    {
        (rxPktReceiveCbArray[ii])(devNum,queueIdx,numOfBuff,
                                packetBuffs,buffLen,rxParamsPtr);
    }
    return GT_OK;
}

/**
* @internal appRefDxChNetRxPacketCbRegister function
* @endinternal
*
* @brief   register a CB function to be called on every RX packet to CPU
*         NOTE: function is implemented in the 'AppDemo' (application)
* @param[in] rxPktReceiveCbFun        - CB function to be called on every RX packet to CPU
*/
GT_STATUS appRefDxChNetRxPacketCbRegister
(
    IN  RX_PACKET_RECEIVE_CB_FUN  rxPktReceiveCbFun
)
{
    GT_U32  ii;

    for(ii = 0 ; ii < numRegistrations ; ii++)
    {
        if(rxPktReceiveCbArray[ii] == rxPktReceiveCbFun)
        {
            /* Already registered */
            return GT_OK;
        }
    }

    if(numRegistrations >= MAX_REGISTER)
    {
        return GT_FULL;
    }

    rxPktReceiveCbArray[numRegistrations] = rxPktReceiveCbFun;

    numRegistrations++;

    /* will be added once */
    rxEventHandlerAddCallback(
            RX_EV_HANDLER_DEVNUM_ALL,
            RX_EV_HANDLER_QUEUE_ALL,
            RX_EV_HANDLER_TYPE_ANY_E,
            prvAppRefDxChNetRxPacketRegisteredCb,
            0);

    return GT_OK;
}

/**
* @internal cpssAppRefDxChLinkChangeWAs function
* @endinternal
*
* @brief   implement WAs for link change event for DXCH ports
*
* @param[in] devNum                - the device number.
* @param[in] physicalPortNum       - physical port number
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS cpssAppRefDxChLinkChangeWAs
(
    GT_U8  devNum,
    GT_U32 portNum
)
{
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_IMPLEMENT_WA_ENT waArr[1];
    GT_U32 additionalInfoBmpArr[1];

    /* work arround appearing wrong MIB counters after port link down */
    waArr[0] = CPSS_DXCH_IMPLEMENT_WA_WRONG_MIB_COUNTERS_LINK_DOWN_E;
    additionalInfoBmpArr[0] = portNum;
    rc = cpssDxChHwPpImplementWaInit(devNum,1,&waArr[0], &additionalInfoBmpArr[0]);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChHwPpImplementWaInit);

    /* WA for fixing CRC errors when Auto-Neg is disabled on 10M/100M port speed */
    waArr[0] = CPSS_DXCH_IMPLEMENT_WA_100BASEX_AN_DISABLE_E;
    additionalInfoBmpArr[0] = portNum;
    rc = cpssDxChHwPpImplementWaInit(devNum, 1, &waArr[0], &additionalInfoBmpArr[0]);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChHwPpImplementWaInit);

    return rc;
}

/**
* @internal cpssAppRefDxChLinkChangeTreat function
* @endinternal
*
* @brief   handle link chnage event for DXCH non-AP ports
*
* @param[in] devNum                - the device number.
* @param[in] physicalPortNum       - physical port number
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS cpssAppRefDxChLinkChangeTreat
(
    GT_U8  devNum,
    GT_U32 phyPortNum
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL linkUp;
    GT_BOOL isPortInUnidirectionalMode = GT_FALSE;
    CPSS_PORT_MAC_TYPE_ENT   portMacType;
    GT_U32 regAddr;
    CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
    GT_U32      timeout;        /* resources free timeout counter */
    GT_U16      portTxqDescNum; /* number of not treated TXQ descriptors */
    GT_BOOL     portShaperEnable; /* current state of port shaper */
    GT_BOOL     portTcShaperEnable[CPSS_TC_RANGE_CNS]; /* current state of port TC shapers */
    GT_U8       tc; /* traffic class */
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusState;

    rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, phyPortNum, &portMap);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortPhysicalPortDetailedMapGet);

    rc = cpssDxChPortMacTypeGet(devNum, phyPortNum, &portMacType);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortMacTypeGet);

    /* 1. we got valid physical port for our MAC number */
    rc = cpssDxChPortLinkStatusGet(devNum, phyPortNum, &linkUp);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortLinkStatusGet);

    if(linkUp == GT_FALSE)
    {
        rc = cpssDxChVntOamPortUnidirectionalEnableGet(devNum, phyPortNum, &isPortInUnidirectionalMode);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChVntOamPortUnidirectionalEnableGet);
        if(isPortInUnidirectionalMode == GT_TRUE)
        {
            /* !!! do not modify the filter !!! keep it as 'link up' */
            return GT_OK;
        }
    }

    /* 2. set CG Flush configuration */
    if((portMacType == CPSS_PORT_MAC_TYPE_CG_E) && (linkUp == GT_FALSE))
    {
        regAddr = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portMap.portMap.macNum).CGPORTMACCommandConfig;

        if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
        {
             rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 22, 1, 1);
             CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssDrvHwPpSetRegField);
        }
    }

    /* 3. need to set the EGF link status filter according to new state of the port */
    portLinkStatusState =  linkUp ?
            CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E :
            CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;

    if(linkUp == GT_TRUE)
    {
        GT_BOOL     portEnabled;
        rc = cpssDxChPortEnableGet(devNum,phyPortNum,&portEnabled);
        /* if the caller set the MAC to be disabled ...
           we need to assume that the EGF filter should treat as 'link down' */
        if(rc == GT_OK && portEnabled == GT_FALSE)
        {
            portLinkStatusState = CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;
        }
    }

    rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum, phyPortNum, portLinkStatusState);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgEgrFltPortLinkEnableSet);

    if(linkUp == GT_FALSE)
    {
        /****************************************/
        /* 4. Disable any shapers on given port */
        /****************************************/
        rc = cpssDxChPortTxShaperEnableGet(devNum, phyPortNum, &portShaperEnable);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxShaperEnableGet);
        for(tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
        {
            rc = cpssDxChPortTxQShaperEnableGet(devNum, phyPortNum, tc, &portTcShaperEnable[tc]);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxQShaperEnableGet);

        }
        rc = cpssDxChTxPortShapersDisable(devNum, phyPortNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTxPortShapersDisable);

        /*******************************************************/
        /* 5. Polling the TXQ port counter until it reach zero */
        /*******************************************************/
        for(timeout = 100; timeout > 0; timeout--)
        {
            rc = cpssDxChPortTxDescNumberGet(devNum, phyPortNum, &portTxqDescNum);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxDescNumberGet);

            if(0 == portTxqDescNum)
                break;
            else
                cpssOsTimerWkAfter(10);
        }

        if(0 == timeout)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortTxDescNumberGet: TXQ descriptor counter read timeout error for port=%d,portTxqDescNum=0x%x \n", phyPortNum, portTxqDescNum);
            return GT_TIMEOUT;
        }

        /*********************************************/
        /* 6. wait 1us for the TXQ-MAC path to drain */
        /*********************************************/
        cpssOsTimerWkAfter(1);

        /*************************************/
        /* 7. Re-enable all disabled shapers */
        /*************************************/
        rc = cpssDxChPortTxShaperEnableSet(devNum, phyPortNum, portShaperEnable);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxShaperEnableSet);
        for(tc = 0; tc < CPSS_TC_RANGE_CNS; tc++)
        {
            rc = cpssDxChPortTxQShaperEnableSet(devNum, phyPortNum, tc, portTcShaperEnable[tc]);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxQShaperEnableSet);
        }

        /***********************************/
        /* 8. unset CG Flush configuration */
        /***********************************/
        if(portMacType == CPSS_PORT_MAC_TYPE_CG_E)
        {
            regAddr = PRV_DXCH_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portMap.portMap.macNum).CGPORTMACCommandConfig;

            if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                 rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, 22, 1, 0);
                 CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssDrvHwPpSetRegField);
            }
        }
    }

    return rc;
}

/*******************************************************************************
* cpssAppRefDataIntegrityScan
*
* DESCRIPTION:
*       This routine treats Data Integrity events.
*
* INPUTS:
*       devNum - device number
*       evExtData - event external data
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
*******************************************************************************/
#define CPSS_APP_REF_DATA_INTEGRITY_EVENTS_NUM_CNS 20

extern DXCH_DATA_INTEGRITY_EVENT_COUNTER_INCREMENT_FUNC *dxChDataIntegrityEventIncrementFunc;

GT_STATUS cpssAppRefDataIntegrityScan
(
    GT_U8   devNum,
    GT_U32  evExtData
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    i = 0;
    GT_BOOL   isScanFinished = GT_FALSE; /* events scan finish status */
    GT_U32    eventsNum = CPSS_APP_REF_DATA_INTEGRITY_EVENTS_NUM_CNS; /* initial number of ECC/parity events */
    CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC eventsArr[CPSS_APP_REF_DATA_INTEGRITY_EVENTS_NUM_CNS];

    while(isScanFinished != GT_TRUE)
    {
        rc = cpssDxChDiagDataIntegrityEventsGet(devNum, evExtData, &eventsNum, eventsArr, &isScanFinished);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChDiagDataIntegrityEventsGet);

        if(dxChDataIntegrityEventIncrementFunc != NULL)
        {
            for(i = 0; i < eventsNum; i++)
            {
                /* counting the event */
                rc = dxChDataIntegrityEventIncrementFunc(devNum, &eventsArr[i]);
                if(rc != GT_OK)
                {
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("appDemoDxChLion2DataIntegrityScan: failed to increment event counter for memType %d\r\n", eventsArr[i].location.ramEntryInfo.memType);
                    return rc;
                }
            }
        }
    }

    return rc;
}

GT_STATUS cpssAppRefGenEventHandle
(
    GT_U8  devNum,
    GT_U32 uniEv,
    GT_U32 evExtData
)
{
    GT_STATUS rc = GT_OK;

    if (notifyEventArrivedFunc != NULL)
    {
        notifyEventArrivedFunc(devNum, uniEv, evExtData);
    }

    switch (uniEv)
    {
        case CPSS_PP_BM_MISC_E: if( evExtData > 2 )
                                {
                                    break;
                                }
             GT_ATTR_FALLTHROUGH;
        case CPSS_PP_BM_PORT_RX_BUFFERS_CNT_UNDERRUN_E:
             GT_ATTR_FALLTHROUGH;
        case CPSS_PP_BM_PORT_RX_BUFFERS_CNT_OVERRUN_E:
             GT_ATTR_FALLTHROUGH;
        case CPSS_PP_BM_WRONG_SRC_PORT_E:
             GT_ATTR_FALLTHROUGH;
        case CPSS_PP_CRITICAL_HW_ERROR_E:
             GT_ATTR_FALLTHROUGH;
        case CPSS_PP_PORT_PCS_PPM_FIFO_UNDERRUN_E:
             GT_ATTR_FALLTHROUGH;
        case CPSS_PP_PORT_PCS_PPM_FIFO_OVERRUN_E:
            CPSS_APP_PLATFORM_LOG_ERR_MAC("FATAL Error - %s\n", uniEvName[uniEv]);
            break;
        case CPSS_PP_PORT_EEE_E:
            if (printEeeInterruptInfo)
            {
                GT_PHYSICAL_PORT_NUM    portNum;
                GT_U32                  subEvent = (evExtData & 0xFF);

                portNum = evExtData>>8;
                cpssOsPrintf("EEE Interrupt: devNum[%ld] port[%ld] subEvent [%ld] \n",
                                                (GT_U32)devNum, portNum , subEvent);
            }
            CPSS_APP_PLATFORM_LOG_INFO_MAC("Event Recevied - %s\n", uniEvName[uniEv]);
            break;
        case CPSS_PP_DATA_INTEGRITY_ERROR_E:
            {
                rc = cpssAppRefDataIntegrityScan(devNum, evExtData);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppRefDataIntegrityScan);
            }
            break;
            /* Doorbell interrupts - end*/
        default:
            /* do nothing */
            break;
    }

    return rc;
}

/**
* @internal appRefSip6_linkChange function
* @endinternal
*
* @brief   function to handle link change in sip6 .
*
* @param[in] devNum                   - the device number.
* @param[in] phyPortNum               - the physical port number that generated the event of
*                                      CPSS_PP_PORT_LINK_STATUS_CHANGED_E
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS   appRefSip6_linkChange(
    IN GT_U8 devNum,
    IN GT_U32 phyPortNum
)
{
    GT_STATUS rc;
    GT_BOOL linkUp;
    GT_BOOL portMngEnable = GT_FALSE;
    CPSS_DXCH_DETAILED_PORT_MAP_STC portMap;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusState;
    GT_BOOL     portEnabled;

    rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, phyPortNum, &portMap);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortPhysicalPortDetailedMapGet);

    /* 1. we got valid physical port for our MAC number */
    rc = cpssDxChPortLinkStatusGet(devNum, phyPortNum, &linkUp);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortLinkStatusGet);

    portLinkStatusState =  linkUp ?
            CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E :
            CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;

    rc = cpssDxChPortEnableGet(devNum,phyPortNum,&portEnabled);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortEnableGet);

    /* 3. need to set the EGF link status filter according to new state of the
       port
        Note: (unlike sip 5.20)
            cpssDxChBrgEgrFltPortLinkEnableSet implementation checks port link enable status and allows
            FORCE_LINK_UP_E only if port TX enabled and there is TXQ resources allocated.
    */
    rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum, phyPortNum,
        portLinkStatusState);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgEgrFltPortLinkEnableSet);

    rc = cpssDxChPortManagerEnableGet(devNum, &portMngEnable);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortManagerEnableGet);

    if (!portMngEnable && linkUp == GT_FALSE)
    {
        /*Disable port,this will cause "open drain"*/
        rc = cpssDxChPortEnableSet(devNum, phyPortNum,GT_FALSE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortEnableSet);

        /*Restore previus port state*/
        if(portEnabled==GT_TRUE)
        {
            rc = cpssDxChPortEnableSet(devNum, phyPortNum,GT_TRUE);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortEnableSet);
        }

    }

    return GT_OK;
}

GT_STATUS cpssAppRefLinkChangeEvHandle
(
    GT_U8  devNum,
    GT_U32 uniEv,
    GT_U32 evExtData
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL   apEnable = GT_FALSE;
    GT_PHYSICAL_PORT_NUM         portNum;
    CPSS_DXCH_PORT_AP_PARAMS_STC apParams;
    GT_U32    physicalPortNum;

    if (notifyEventArrivedFunc != NULL)
    {
        notifyEventArrivedFunc(devNum, uniEv, evExtData);
    }

    switch (uniEv)
    {
        case CPSS_PP_PORT_LINK_STATUS_CHANGED_E:
            portNum = (GT_U32)evExtData;
            CPSS_APP_PLATFORM_LOG_DBG_MAC("Event - %s, PortNum - %d\n", uniEvName[uniEv], portNum);

            /* For SIP6 devices all port functionally is completely new, so no need to implement legacy WAs */
            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
            {
                rc = appRefSip6_linkChange(devNum, portNum);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, appRefSip6_linkChange);

                return GT_OK;
            }

            rc = cpssDxChPortApPortConfigGet(devNum, portNum, &apEnable, &apParams);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortApPortConfigGet);

            if (cpssAppPlatformIsPortMgrPort(devNum) && apEnable == GT_FALSE)
            {
                /* Do nothing for non-AP ports in port Manager mode.
                 * Event is already notified to port manager by cpss App platform
                cpssAppPlatformPmPortLinkStatusChangeSignal(devNum, portNum);*/
                break;
            }

            /* for XLG Unidirectional WA ports - enables/disables port */
            /* for regular ports does nothing                          */
            rc = cpssDxChPortEnableWaWithLinkStatusSet(devNum, portNum);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortEnableWaWithLinkStatusSet);

            rc = cpssAppRefDxChLinkChangeWAs(devNum, portNum);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppRefDxChLinkChangeWAs);

            if (apEnable == GT_FALSE)
            {
                rc = cpssAppRefDxChLinkChangeTreat(devNum, portNum);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppRefDxChLinkChangeTreat);
            }
            /* ToDo add ApLinkChangeTreat for Aldrin */

            break;

#ifndef ASIC_SIMULATION

        case CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E:
            portNum = (GT_U32)evExtData;
            CPSS_APP_PLATFORM_LOG_DBG_MAC("DevNum - %d, Port Event - %s, Port - %d\n", devNum, uniEvName[uniEv], portNum);

            if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
            {
                /* this event is not handled for SIP6 devices */
                return GT_OK;
            }

            if (cpssAppPlatformIsPortMgrPort(devNum))
            {
                /* Do nothing in port Manager mode.
                 * Event is already notified to port manager by cpss App platform */
                break;
            }
            else /* legacy Mode */
            {
                if(trxTraining || rxTraining)
                {
                    CPSS_PORTS_BMP_PORT_SET_MAC(&todoTuneBmp[devNum], portNum);
                    rc = cpssOsSigSemSignal(serdesTuneSmId[devNum]);
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsSigSemSignal);
                }
            }
            break;
#endif /* ASIC_SIMULATION */

        case CPSS_PP_PORT_SYNC_STATUS_CHANGED_E:
        case CPSS_PP_PORT_PCS_GB_LOCK_SYNC_CHANGE_E:
        case CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E:
            portNum = (GT_U32)evExtData;
            CPSS_APP_PLATFORM_LOG_DBG_MAC("DevNum - %d, Port Event - %s, Port - %d\n", devNum, uniEvName[uniEv], portNum);

            if (cpssAppPlatformIsPortMgrPort(devNum))
            {
                /* Do nothing in port Manager mode.
                 * Event is already notified to port manager by cpss App platform */
            }
            break;

        case CPSS_PP_PORT_AN_COMPLETED_E:
            CPSS_APP_PLATFORM_LOG_DBG_MAC("Event Recevied - %s\n", uniEvName[uniEv]);
            break;

        case CPSS_SRVCPU_PORT_AP_DISABLE_E:
            portNum = (GT_U16)evExtData;

            rc = cpssDxChPortPhysicalPortMapReverseMappingGet(devNum, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,portNum, &physicalPortNum);
            if(rc != GT_OK)
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E - cpssDxChPortPhysicalPortMapReverseMappingGet:rc=%d,portNum=%d\n",rc, portNum);
                return rc;
            }

            /*osPrintf("CPSS_SRVCPU_PORT_AP_DISABLE mac %d portNum=%d \n",portNum, physicalPortNum);*/
            if (cpssAppPlatformIsPortMgrPort(devNum))
            {
                cpssAppPlatformPmPortStatusChangeSignal(devNum, physicalPortNum, CPSS_SRVCPU_PORT_AP_DISABLE_E);
                /* pizza allocation done inside port manager, so no need to continue */
                break;
            }
            /*TODO add support for non PM port*/
            break;
        case CPSS_PP_PORT_AN_HCD_FOUND_E:

            portNum = (GT_U16)evExtData;

            rc = cpssDxChPortPhysicalPortMapReverseMappingGet(devNum, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,portNum, &physicalPortNum);
            if(rc != GT_OK)
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E - cpssDxChPortPhysicalPortMapReverseMappingGet:rc=%d,portNum=%d\n",rc, portNum);
                return rc;
            }

            /*osPrintf("CPSS_SRVCPU_PORT_AP_DISABLE mac %d portNum=%d \n",portNum, physicalPortNum);*/
            if (cpssAppPlatformIsPortMgrPort(devNum))
            {
                cpssAppPlatformPmPortStatusChangeSignal(devNum, physicalPortNum, CPSS_PP_PORT_AN_HCD_FOUND_E);
                /* pizza allocation done inside port manager, so no need to continue */
                break;
            }
            break;
        case CPSS_PP_PORT_AN_RESTART_E:

            portNum = (GT_U16)evExtData;

            rc = cpssDxChPortPhysicalPortMapReverseMappingGet(devNum, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,portNum, &physicalPortNum);
            if(rc != GT_OK)
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("CPSS_PP_PORT_AN_RESTART_E - cpssDxChPortPhysicalPortMapReverseMappingGet:rc=%d,portNum=%d\n",rc, portNum);
                return rc;
            }

            if (cpssAppPlatformIsPortMgrPort(devNum))
            {
                cpssAppPlatformPmPortStatusChangeSignal(devNum, physicalPortNum, CPSS_PP_PORT_AN_RESTART_E);
                break;
            }
            /*TODO add support for non PM port*/
            break;
        case CPSS_PP_PORT_AN_PARALLEL_DETECT_E:

            portNum = (GT_U16)evExtData;

            rc = cpssDxChPortPhysicalPortMapReverseMappingGet(devNum, CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,portNum, &physicalPortNum);
            if(rc != GT_OK)
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("CPSS_PP_PORT_AN_PARALLEL_DETECT_E - cpssDxChPortPhysicalPortMapReverseMappingGet:rc=%d,portNum=%d\n",rc, portNum);
                return rc;
            }

            if (cpssAppPlatformIsPortMgrPort(devNum))
            {
                cpssAppPlatformPmPortStatusChangeSignal(devNum, physicalPortNum, CPSS_PP_PORT_AN_PARALLEL_DETECT_E);
                break;
            }
            /*TODO add support for non PM port*/
            break;
        default:
            /* do nothing */
            break;
    }

    return rc;
}

GT_BOOL isRxSdmaEvent
(
    IN  GT_U8                     devNum,
    IN  GT_U32                    uniEv,
    OUT RX_EV_HANDLER_EV_TYPE_ENT *rxOperType
)
{
    (void)devNum;

        /* range 0..7 */
    if(uniEv >= CPSS_PP_RX_BUFFER_QUEUE0_E && uniEv <= CPSS_PP_RX_BUFFER_QUEUE7_E)
    {
        *rxOperType = RX_EV_HANDLER_TYPE_RX_E;
    }
    else
    if(uniEv >= CPSS_PP_RX_ERR_QUEUE0_E && uniEv <= CPSS_PP_RX_ERR_QUEUE7_E)
    {
        *rxOperType = RX_EV_HANDLER_TYPE_RX_ERR_E;
    }
    else   /* range 8..15 */
    if(uniEv >= CPSS_PP_RX_BUFFER_QUEUE8_E && uniEv <= CPSS_PP_RX_BUFFER_QUEUE15_E)
    {
        *rxOperType = RX_EV_HANDLER_TYPE_RX_E;
    }
    else
    if(uniEv >= CPSS_PP_RX_ERR_QUEUE8_E && uniEv <= CPSS_PP_RX_ERR_QUEUE15_E)
    {
        *rxOperType = RX_EV_HANDLER_TYPE_RX_ERR_E;
    }
    else   /* range 16..23 */
    if(uniEv >= CPSS_PP_RX_BUFFER_QUEUE16_E && uniEv <= CPSS_PP_RX_BUFFER_QUEUE23_E)
    {
        *rxOperType = RX_EV_HANDLER_TYPE_RX_E;
    }
    else
    if(uniEv >= CPSS_PP_RX_ERR_QUEUE16_E && uniEv <= CPSS_PP_RX_ERR_QUEUE23_E)
    {
        *rxOperType = RX_EV_HANDLER_TYPE_RX_ERR_E;
    }
    else   /* range 24..31 */
    if(uniEv >= CPSS_PP_RX_BUFFER_QUEUE24_E && uniEv <= CPSS_PP_RX_BUFFER_QUEUE31_E)
    {
        *rxOperType = RX_EV_HANDLER_TYPE_RX_E;
    }
    else
    if(uniEv >= CPSS_PP_RX_ERR_QUEUE24_E && uniEv <= CPSS_PP_RX_ERR_QUEUE31_E)
    {
        *rxOperType = RX_EV_HANDLER_TYPE_RX_ERR_E;
    }
    else   /* range 32..127 */
    if(uniEv >= CPSS_PP_RX_BUFFER_QUEUE32_E && uniEv <= CPSS_PP_RX_BUFFER_QUEUE127_E)
    {
        *rxOperType = RX_EV_HANDLER_TYPE_RX_E;
    }
    else
    if(uniEv >= CPSS_PP_RX_ERR_QUEUE32_E && uniEv <= CPSS_PP_RX_ERR_QUEUE127_E)
    {
        *rxOperType = RX_EV_HANDLER_TYPE_RX_ERR_E;
    }
    else
    {
        return GT_FALSE;
    }

    return GT_TRUE;
}

#define APP_REF_RX_BUFF_ALLIGN_BITS_DEF  7
#define APP_REF_RX_BUFF_ALLIGN_DEF      (1 << APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF)
#define APP_REF_RX_BUFF_SIZE_DEF        800

GT_STATUS cpssAppRefRxEventHandle
(
    GT_U8  devNum,
    GT_U32 uniEv,
    GT_U32 evExtData
)
{
    GT_STATUS                   rc;
    GT_U32                      numOfBuff = BUFF_LEN;
    GT_U8                       *packetBuffs[BUFF_LEN];
    GT_U32                      buffLenArr[BUFF_LEN];
    CPSS_DXCH_NET_RX_PARAMS_STC rxParams;
    GT_U8                       queueIdx = (GT_U8)evExtData;
    RX_EV_HANDLER_EV_TYPE_ENT   evType = RX_EV_HANDLER_TYPE_RX_E;

    if (notifyEventArrivedFunc != NULL)
    {
        notifyEventArrivedFunc(devNum, uniEv, evExtData);
    }

    if(!isRxSdmaEvent(devNum, uniEv, &evType))
    {
        return GT_FAIL;
    }

    do
    {
        /* get the packet from the device */
        if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
        {
            rc = cpssDxChNetIfSdmaRxPacketGet(devNum, queueIdx, &numOfBuff,
                                              packetBuffs, buffLenArr, &rxParams);
        }
        else if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E)
        {
            rc = cpssDxChNetIfMiiRxPacketGet(devNum,queueIdx,&numOfBuff,packetBuffs,buffLenArr,&rxParams);
        }
        else
        {
            rc = GT_FAIL;
        }

        if (rc != GT_OK)
        {
            /* GT_NO_MORE is good and not an error !!! */
            if(rc == GT_NO_MORE)
                rc = GT_OK;
            /* no need to free buffers because not got any */
            return rc;
        }

        if(rxEventHandle)
        {
            rxEventHandlerDoCallbacks(rxEventHandle, evType,
                devNum,queueIdx,numOfBuff,
                packetBuffs,buffLenArr,&rxParams);
        }

        /* Free the buffers */
        if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
        {
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.allocMethod[queueIdx] != CPSS_RX_BUFF_NO_ALLOC_E)
            {
                rc = cpssDxChNetIfRxBufFree(devNum,queueIdx,packetBuffs,numOfBuff);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChNetIfRxBufFree);
            }
            else
            {

                /* move the first buffer pointer to the original place*/
                packetBuffs[0] = (GT_U8*)((((GT_UINTPTR)(packetBuffs[0])) >> APP_REF_RX_BUFF_ALLIGN_BITS_DEF)
                                          << APP_REF_RX_BUFF_ALLIGN_BITS_DEF);

                /* fix the last buffer's size*/
                buffLenArr[numOfBuff-1] = APP_REF_RX_BUFF_SIZE_DEF;

                rc = cpssDxChNetIfRxBufFreeWithSize(devNum,queueIdx,packetBuffs,buffLenArr,numOfBuff);
            }
        }
        else /* CPSS_NET_CPU_PORT_MODE_MII_E */
        {
            rc = cpssDxChNetIfMiiRxBufFree(devNum,queueIdx,packetBuffs,numOfBuff);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChNetIfMiiRxBufFree);
        }
    }while(rc == GT_OK);

    return rc;
}

/**
* @internal cpssAppRefEventFuncInit function
* @endinternal
*
* @brief   init rx, au, data Integrity CBs used by UT.
*          create serdes training task.
*
* @param[in] devNum    - device number.
*
* @retval GT_OK        - on success,
* @retval GT_FAIL      - otherwise.
*
*/
GT_STATUS cpssAppRefEventFuncInit
(
    GT_U8  devNum
)
{
    GT_STATUS rc = GT_OK;

#ifdef CHX_FAMILY
    /* Get event increment CB routine. */
    appRefDxChDiagDataIntegrityCountersCbGet(&dxChDataIntegrityEventIncrementFunc);

    if(dxChDataIntegrityEventIncrementFunc != NULL)
    {
        /* trigger to allocate the needed memory for the device (during initialization)*/
        /* don't care about the return value */
        (GT_VOID)dxChDataIntegrityEventIncrementFunc(devNum, NULL);
    }
#endif
    if(rxEventHandle == 0)
    {
        rc = rxEventHandlerInitHandler(RX_EV_HANDLER_DEVNUM_ALL,
                                  RX_EV_HANDLER_QUEUE_ALL,
                                  RX_EV_HANDLER_TYPE_ANY_E,
                                  &rxEventHandle);
        if (rc != GT_OK)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("rxEventHandlerInitHandler failed rc : %d\n", rc);
            rxEventHandle = 0;
        }
    }

    if(auEventHandle == 0)
    {
        rc = auEventHandlerInitHandler(AU_EV_HANDLER_DEVNUM_ALL,
                                       AU_EV_HANDLER_TYPE_ANY_E,
                                       &auEventHandle);
        if (rc != GT_OK)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("auEventHandlerInitHandler failed rc : %d\n", rc);
            auEventHandle = 0;
        }
    }

#ifndef ASIC_SIMULATION
    rc = cpssAppRefSerdesTrainingTaskCreate(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppRefSerdesTrainingTaskCreate);
#endif

    return rc;
}

/**
* @internal appRefPrintLinkChangeFlagSet function
* @endinternal
*
* @brief   function to allow set the flag of : printLinkChangeEnabled
*
* @param[in] enable                   - enable/disable the printing of 'link change'
*
* @retval GT_OK                    - on success
*/
GT_STATUS appRefPrintLinkChangeFlagSet
(
    IN GT_U32   enable
)
{
    printLinkChangeEnabled = enable;
    return GT_OK;
};

/**
* @internal appRefPrintLinkChangeFlagGet function
* @endinternal
*
* @brief   function to get the flag of : printLinkChangeEnabled
*
* @retval printLinkChangeEnabled - do we print link status change info
*/
GT_BOOL appRefPrintLinkChangeFlagGet(void)
{
    return printLinkChangeEnabled;
}

