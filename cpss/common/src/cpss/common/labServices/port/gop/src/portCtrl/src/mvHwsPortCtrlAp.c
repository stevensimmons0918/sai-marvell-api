/*******************************************************************************
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
* @file mvHwsPortCtrlAp.c
*
* @brief Port Control AP Detection State Machine
*
* @version   1
********************************************************************************
*/
#include <mvHwsPortCtrlInc.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <mvHwsPortCtrlDoorbellEvents.h>

#ifndef BOBK_DEV_SUPPORT
#include <mvHwsPortCtrlCommonEng.h>
#endif

#include <mvHwsPortCtrlAp.h>
#if defined (ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT)
#include <cpss/common/labServices/port/gop/port/serdes/avago/mv_hws_avago_if.h>
#endif
#if defined(AC5_DEV_SUPPORT)
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvComphyIf.h>
#endif
extern GT_STATUS mvHwsGetPortParameters(GT_U32                  portNum,
                                        MV_HWA_AP_PORT_MODE     apPortMode,
                                        MV_HWS_PORT_INIT_PARAMS *portParams);

GT_U32 mvHwsApDetectExecutionEnabled = 0;
GT_BOOL mvHwsApPortEnableCtrl = GT_TRUE;

#ifdef ALDRIN_DEV_SUPPORT
GT_U8 refClockSourceDb[ALDRIN_PORTS_NUM];
#endif

#if defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
GT_U8 refClockSourceDb[MV_HWS_MAX_PORT_NUM];
#endif

#ifdef PIPE_DEV_SUPPORT
GT_U8 refClockSourceDb[PIPE_PORTS_NUM];
#endif

#ifdef AC5_DEV_SUPPORT
GT_U8 refClockSourceDb[AC5_PORTS_NUM];
#endif

#if (defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
GT_U8 nextAdaptCtlePort = 0;
#define MAX_ADAPTIVE_CTLE_PORTS_IN_CYCLE 10
#endif

static MV_HWS_PORT_CTRL_COMMON *infoPtr;

#if defined (PIPE_DEV_SUPPORT) && !defined(MICRO_INIT)
#include <cpss/common/labServices/port/gop/port/silicon/pipe/mvHwsPipePortIf.h>

#define LKB_PORT_NONE   0xFF
unsigned char mvLkbDebugPort = LKB_PORT_NONE;
#define LKB_PORT_DEBUG(port, str, ...)    \
    if (mvLkbDebugPort == port) {         \
        osPrintf("" str, ##__VA_ARGS__);  \
    }


/* Link binding machine states */
typedef enum
{
    MV_HWS_LKB_NA_E,            /* port is not LKB enabed */
    MV_HWS_LKB_REGISTERED_E,    /* AP port is registered as LKB, but still not active (FW does not handle its state) */

    MV_HWS_LKB_NO_FAULT_E,      /* port is in regular state (maty be up/down) */
    MV_HWS_LKB_FAULT_E,         /* port is in fault state*/
    MV_HWS_LKB_AWAKE_E,         /* after unset from fault, waiting for it to link-up */

    MV_HWS_LKB_LAST_E
} MV_HWS_LKB_STATE_ENT;


/* Link binding requests */
typedef enum
{
    MV_HWS_LKB_REQUEST_NONE_E,
    MV_HWS_LKB_REQUEST_AP_REGISTER_E,
    MV_HWS_LKB_REQUEST_AP_DEACTIVATE_E,
    MV_HWS_LKB_REQUEST_ADD_PORT_E,
    MV_HWS_LKB_REQUEST_REMOVE_PORT_E,
} MV_HWS_LKB_REQUEST_ENT;

typedef struct {
    GT_U16                  pair;
    GT_U16                  isAP;
    MV_HWS_LKB_STATE_ENT    state;
    MV_HWS_PORT_STANDARD    portMode;
    GT_U32                  awakeTime;

    MV_HWS_LKB_REQUEST_ENT  request;
} MV_HWS_LKB_STC;

#define LKB_AWAKE_PERIOD_MS         5 /* ms */

#define LKB_CHECK_PORT_MAC(port)        \
    if (port > HWS_PIPE_PORTS_NUM_CNS)  \
        return GT_OUT_OF_RANGE;


MV_HWS_LKB_STC  linkBindingPorts[HWS_PIPE_PORTS_NUM_CNS];

char* LKB_STRINGS[MV_HWS_LKB_LAST_E] =
{
    "N/A",
    "REGISTERED",
    "NO-FAULT",
    "FAULT",
    "AWAKE"
};


/**
* @internal mvLkbInitDB function
* @endinternal
*
*/
void mvLkbInitDB()
{
    int i;
    MV_HWS_LKB_STC  lkb = {0, 0, MV_HWS_LKB_NA_E, NON_SUP_MODE, 0, MV_HWS_LKB_REQUEST_NONE_E};

    for (i=0; i<HWS_PIPE_PORTS_NUM_CNS; i++)
        linkBindingPorts[i] = lkb;
}


/**
* @internal mvLkbPrintAll function
* @endinternal
*
*/
void mvLkbPrintAll(void)
{
    unsigned int i;
    GT_BOOL found = GT_FALSE;

    for (i=0; i<sizeof(linkBindingPorts)/sizeof(MV_HWS_LKB_STC); i++)
        if (linkBindingPorts[i].state != MV_HWS_LKB_NA_E) {
            printf("Port %d:[%d] %s portMode %d\n",i, linkBindingPorts[i].pair, LKB_STRINGS[linkBindingPorts[i].state], linkBindingPorts[i].portMode);
            found = GT_TRUE;
        }

    if (found == GT_FALSE)
        printf("No LKB ports\n");
}


static GT_STATUS prvSendFaultDoorbell(
    GT_U32  phyPortNum
)
{
    switch (doorbellIntrAddEvent(DOORBELL_EVENT_REMOTE_FAULT_TRANSMISSION_CHANGE(phyPortNum))) {
    case -1:
        LKB_PRINT("doorbellIntrAddEvent failed\n");
        return GT_FAIL;

    case 0:
        LKB_PRINT("doorbellIntrAddEvent - masked\n");
        return GT_FAIL;

    default:
        return GT_OK;
    }
}

static GT_STATUS prvSetFaultAndSendDoorbell(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 send
)
{
    GT_STATUS   rc;

    rc = hwsPortFaultSendSet(devNum, portGroup, phyPortNum, portMode, send);
    if (GT_OK != rc) {
        LKB_PRINT("hwsPortFaultSendSet return: rc=%d\n", rc);
        return GT_FAIL;
    }

    return prvSendFaultDoorbell(phyPortNum);
}


/**
* @internal mvLkbReqApPortRegister function
* @endinternal
*
* @brief  send req to register AP port with LKB
*
* @param[in] port                     - physical port number
* @param[in] pair                     - physical pair port number.
*                                       when the link on port is down -
*                                       the remote fault enabled on the pair port.
*
* @retval GT_OK                       - on success
* @retval GT_FAIL                     - on error
*/
GT_STATUS mvLkbReqApPortRegister(unsigned char port, unsigned char pair)
{
    LKB_CHECK_PORT_MAC(port);

    LKB_PRINT("ApPortRegister %d\n", port);
    if (linkBindingPorts[port].state != MV_HWS_LKB_NA_E) {
        LKB_PRINT("AP port %u already initialized\n", port);
        return GT_FAIL;
    }

    linkBindingPorts[port].request = MV_HWS_LKB_REQUEST_AP_REGISTER_E;
    linkBindingPorts[port].pair = pair;
    linkBindingPorts[port].isAP = GT_TRUE;

    return GT_OK;
}


/**
* @internal mvLkbReqApPortDeactivate function
* @endinternal
*
* @brief  send req to move an AP port to 'register' (inactive) state
*
* @param[in] port                     - physical port number
*
* @retval GT_OK                       - on success
* @retval GT_FAIL                     - on error
*/
GT_STATUS mvLkbReqApPortDeactivate(unsigned char port)
{
    GT_STATUS               rc = GT_OK;

    LKB_CHECK_PORT_MAC(port);
    LKB_PRINT("ApPortDeactivate %d\n", port);

    if (!linkBindingPorts[port].isAP)
        return GT_FALSE;

    if (linkBindingPorts[port].state > MV_HWS_LKB_REGISTERED_E)
        linkBindingPorts[port].request = MV_HWS_LKB_REQUEST_AP_DEACTIVATE_E;

    return rc;
}

/* Converts HWS port mode to an arbitrary AP-mode. Is used to to update AP-FW DB with current working mode */
static MV_HWA_AP_PORT_MODE prvHwsPortModeToAP(MV_HWS_PORT_STANDARD hwsPortMode)
{
    switch (hwsPortMode)
    {
    case _100GBase_KR10:
    case _100GBase_SR10:
        return Port_100GBASE_CR10;

    case _100GBase_CR4:
        return Port_100GBASE_CR4;

    case _100GBase_KR4:
    case _100GBase_SR4:
        return Port_100GBASE_KR4;

    default:
        return MODE_NOT_SUPPORTED;
    }
}


/**
* @internal mvLkbReqPortSet function
* @endinternal
*
* @brief sets link binding on port
*
* @param[in] port                     - physical port number
* @param[in] portMode                 - HWS port mode
* @param[in] pair                     - physical pair port number.
*                                       when the link on port is down -
*                                       the remote fault enabled on the pair port.
* @param[in] enable                   -
* @param[in] isAP                     -
*
* @retval GT_OK                       - on success
* @retval GT_FAIL                     - on error
*/
GT_STATUS mvLkbReqPortSet(unsigned char port, MV_HWS_PORT_STANDARD portMode, unsigned char pair, GT_BOOL enable, GT_BOOL isAP)
{
    LKB_PRINT("mvLkbReqPortSet %d:[%d], portMode %d, enable %d isAP %d\n", port, pair, portMode, enable, isAP);
    LKB_CHECK_PORT_MAC(port);

    /* Each AP port call this func, but it may not be LKB enabled */
    if (isAP && (linkBindingPorts[port].state == MV_HWS_LKB_NA_E /* AP port isn't registered */) )
        return GT_OK;

    if (enable == GT_TRUE)
    {
        if (linkBindingPorts[port].state > MV_HWS_LKB_REGISTERED_E) {
            LKB_PRINT("Port %u already initialized\n", port);
            return GT_FAIL;
        }

        linkBindingPorts[port].portMode = portMode;
        linkBindingPorts[port].request =  MV_HWS_LKB_REQUEST_ADD_PORT_E;
    } else {
        if (linkBindingPorts[port].state == MV_HWS_LKB_NA_E)
            return GT_OK;
        linkBindingPorts[port].request = MV_HWS_LKB_REQUEST_REMOVE_PORT_E;
    }

    if (!isAP) /* for AP, pair was already set */
        linkBindingPorts[port].pair = pair;
    linkBindingPorts[port].isAP =  isAP;
    return GT_OK;
}

/**
* @internal linkBindingPortSet function
* @endinternal
*
*/
static GT_STATUS linkBindingPortSet(unsigned char port, unsigned char pair, GT_BOOL enable, GT_BOOL isAP)
{
    MV_HWA_AP_PORT_MODE     apPortMode;
    GT_STATUS               rc = GT_OK;

    LKB_PRINT("linkBindingPortSet\n");

    if (enable == GT_TRUE)
    {
        if (linkBindingPorts[port].state > MV_HWS_LKB_REGISTERED_E) {
            LKB_PRINT("Port %u already initialized\n", port);
            return GT_FAIL;
        }

        apPortMode = prvHwsPortModeToAP(linkBindingPorts[port].portMode);
        if (!isAP && (apPortMode != MODE_NOT_SUPPORTED) ) {
            /* for non-AP port, if it's a mode that uses a non-XLG mac, update HWS DB, so that mvHwsPortLinkStatusGet will return valid value */
            rc = mvHwsApSetPortParameters(port, apPortMode);
            if (GT_OK != rc) {
                LKB_PRINT("%s: mvHwsApSetPortParameters return: rc=%d\n", __LINE__, rc);
                return GT_FAIL;
            }
        }

        if (linkBindingPorts[pair].state > MV_HWS_LKB_REGISTERED_E)
        {
            linkBindingPorts[port].state = MV_HWS_LKB_NO_FAULT_E;

            /* Fot non-AP, if pair already joined, remove remote fault */
            if (!isAP) {
                rc = hwsPortFaultSendSet(0, 0, port, linkBindingPorts[port].portMode, GT_FALSE);
                if (GT_OK != rc) {
                    LKB_PRINT("%s: hwsPortFaultSendSet return: rc=%d\n", __LINE__, rc);
                    return GT_FAIL;
                }
            }
        } else {
            linkBindingPorts[port].state = MV_HWS_LKB_FAULT_E;

            /* Fot AP, if pair hasn't joined, set remote fault */
            if (isAP) {
                rc = hwsPortFaultSendSet(0, 0, port, linkBindingPorts[port].portMode, GT_TRUE);
                if (GT_OK != rc) {
                    LKB_PRINT("%s: hwsPortFaultSendSet return: rc=%d\n", __LINE__, rc);
                    return GT_FAIL;
                }
            }

            prvSendFaultDoorbell(port); /* For non-AP, fault was already set by PM */
        }

        linkBindingPorts[port].pair = pair;

        LKB_PRINT("LKB %s port %d:[%d] state %s\n", isAP ? "AP" : "", port, pair,
                  linkBindingPorts[port].state == MV_HWS_LKB_FAULT_E ? "fault" : "no-fault");

    } else /* remove LKB port */
    {
        if (linkBindingPorts[port].state ==  MV_HWS_LKB_FAULT_E)
            prvSetFaultAndSendDoorbell(0, 0, port, linkBindingPorts[port].portMode, GT_FALSE);

        linkBindingPorts[port].portMode = NON_SUP_MODE;
        linkBindingPorts[port].state = MV_HWS_LKB_NA_E;
        LKB_PRINT("LKB port %d deleted\n", port);
    }

    return rc;
}
#endif


/**
* @internal mvHwsApDetectStateSet function
* @endinternal
*
* @brief   AP Detection process enable / disable set
*/
void mvHwsApDetectStateSet(GT_U32 state)
{
    mvHwsApDetectExecutionEnabled = state;
}

/**
* @internal mvHwsApPortEnableCtrlSet function
* @endinternal
*
* @brief   Set the entity responsible to preform port enable after training � Host or Service CPU (default value � service CPU)
*/
void mvHwsApPortEnableCtrlSet(GT_BOOL portEnableCtrl)
{
    mvHwsApPortEnableCtrl = portEnableCtrl;
}


/**
* @internal mvHwsApPortEnableCtrlGet function
* @endinternal
*
* @brief   Get the entity responsible to preform port enable after training � Host or Service CPU (default value � service CPU
*/
void mvHwsApPortEnableCtrlGet(GT_BOOL *portEnableCtrl)
{
    *portEnableCtrl = mvHwsApPortEnableCtrl;
}

/**
* @internal mvHwsApDetectRoutine function
* @endinternal
*
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void mvHwsApDetectRoutine(void* pvParameters)
{
    mvPortCtrlSyncLock();
    mvPortCtrlSyncUnlock();

    infoPtr = &(mvHwsPortCtrlApPortDetect.info);

    /* enable traffic after link-up, only for  bobk */
#ifdef BOBK_DEV_SUPPORT
    mvHwsApPortEnableCtrl = GT_TRUE;
#endif
    for( ;; )
    {
        /*
        ** State Machine Transitions Table
        ** +================+=============+=============+=============+
        ** + Event \ State  +  Active O1  +   Msg O2    +  Delay O3   +
        ** +================+=============+=============+=============+
        ** + Active         +     O1      +   Invalid   +    ==> O1   +
        ** +================+=============+=============+=============+
        ** + Msg            +   ==> O2    +     O2      +   Invalid   +
        ** +================+=============+=============+=============+
        ** + Delay          +   Invalid   +   ==> O3    +   Invalid   +
        ** +================+=============+=============+=============+
        */
        if (mvHwsPortCtrlApPortDetect.funcTbl[infoPtr->event][infoPtr->state] != NULL)
        {
            mvHwsPortCtrlApPortDetect.funcTbl[infoPtr->event][infoPtr->state]();
        }
        else
        {
            mvPcPrintf("Error, Port Detect, Func table: state[%d] event[%d] is NULL\n",
                       infoPtr->state, infoPtr->event);
        }
    }
}



#if (defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
#ifdef RUN_ADAPTIVE_CTLE
/**
* @internal mvPortCtrlNonApPortDetectionActiveExec function
* @endinternal
*
* @brief   non AP Active state execution
*         - Exract port state
*         - Execute state functionality
*/
static void mvPortCtrlNonApPortDetectionActiveExec
(
    GT_U8 portIndex,
    GT_BOOL *adaptCtleExec
)
{
    MV_HWS_AP_SM_INFO *apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    switch (apSm->state)
    {
    case AP_PORT_SM_IDLE_STATE:
        return;

    case NON_AP_PORT_SM_ADAPTIVE_CTLE_STATE:
        mvApPortRunAdaptiveCtle(portIndex,adaptCtleExec);
        break;

    default:
        mvPcPrintf("Error, non AP Detect, PortIndex-%d Invalid state %d!!!\n", portIndex, apSm->state);
        break;
    }

}
#endif
#endif
/**
* @internal mvPortCtrlApPortDetectionActiveExec function
* @endinternal
*
* @brief   AP Detect Active state execution
*         - Exract port state
*         - Execute state functionality
*/
static void mvPortCtrlApPortDetectionActiveExec(GT_U8 portIndex)
{
    MV_HWS_AP_SM_INFO *apSm = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);

    switch (apSm->state)
    {
    case AP_PORT_SM_IDLE_STATE:
        return;

    case AP_PORT_SM_INIT_STATE:
        mvApPortInit(portIndex);
        break;

    case AP_PORT_SM_TX_DISABLE_STATE:
        mvApPortTxDisable(portIndex);
        break;

    case AP_PORT_SM_RESOLUTION_STATE:
        mvApPortResolution(portIndex);
        break;

    case AP_PORT_SM_ACTIVE_STATE:
        mvApPortLinkUp(portIndex);
        break;

    case AP_PORT_SM_DELETE_STATE:
    case AP_PORT_SM_DISABLE_STATE:
        mvApPortDeleteValidate(portIndex);
        break;

    default:
        mvPcPrintf("Error, AP Detect, PortIndex-%d Invalid state %d!!!\n", portIndex, apSm->state);
        break;
    }
}

/**
* @internal mvPortCtrlApDetectActive function
* @endinternal
*
* @brief   AP Detect Active state execution
*         - Scan all ports and call port execution function
*/
void mvPortCtrlApDetectActive(void)
{
    GT_U8 portIndex;
    GT_U8 maxApPortNum = 0;

#if (defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
#ifdef RUN_ADAPTIVE_CTLE
    GT_U8 bitMapIndex = 0;
    GT_BOOL portType = GT_FALSE;
    GT_BOOL adaptCtleExec = GT_FALSE;
    GT_U8 ii = 0;
#endif /*RUN_ADAPTIVE_CTLE*/
#endif

#ifdef BC2_DEV_SUPPORT
    if(BobcatA0 == hwsDeviceSpecInfo[0].devType)
    {
        maxApPortNum = MV_PORT_CTRL_BC2_MAX_AP_PORT_NUM;
    }
    else if(Alleycat3A0 == hwsDeviceSpecInfo[0].devType)
    {
        maxApPortNum = MV_PORT_CTRL_AC3_MAX_AP_PORT_NUM;
    }
    else
    {
        maxApPortNum = 0;
    }
#else
    maxApPortNum = MV_PORT_CTRL_MAX_AP_PORT_NUM;
#endif
    infoPtr->state = AP_PORT_DETECT_ACTIVE_PORT_STATE;
    for (portIndex = 0; portIndex < maxApPortNum; portIndex++)
    {

#if (defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
#ifdef RUN_ADAPTIVE_CTLE
        bitMapIndex = portIndex/HWS_MAX_ADAPT_CTLE_DB_SIZE;
        portType = ((hwsDeviceSpecInfo[0].adaptiveCtleDb.bitMapadaptCtleMode[bitMapIndex] & (0x1 << (portIndex%HWS_MAX_ADAPT_CTLE_DB_SIZE))) > 0 ? GT_TRUE : GT_FALSE);
        if(GT_TRUE == portType) /* non ap port, the port state that is invalid in AP SM*/
            continue;
#endif /*RUN_ADAPTIVE_CTLE*/
#endif
        mvPortCtrlApPortDetectionActiveExec(portIndex);
    }

#if (defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
#ifdef RUN_ADAPTIVE_CTLE
    /* adaptive CTLE loop for Ap ports and Non AP ports.
       ii : number of active ports. We don't want to run adaptive CTLE more than MAX_ADAPTIVE_CTLE_PORTS times in one AP cycle*/
    ii = 0;
    for (portIndex = nextAdaptCtlePort; portIndex < maxApPortNum; portIndex++)
    {
        bitMapIndex = portIndex/HWS_MAX_ADAPT_CTLE_DB_SIZE;
        portType = ((hwsDeviceSpecInfo[0].adaptiveCtleDb.bitMapadaptCtleMode[bitMapIndex] & (0x1 << (portIndex%HWS_MAX_ADAPT_CTLE_DB_SIZE))) > 0 ? GT_TRUE : GT_FALSE);
        /* non AP port*/
        if(GT_TRUE == portType)
        {
            mvPortCtrlNonApPortDetectionActiveExec(portIndex,&adaptCtleExec);
        }
        else /* AP port*/
        {
            mvPortCtrlApPortAdaptiveCtleBaseTemp(portIndex,&adaptCtleExec);
        }

        if (adaptCtleExec == GT_TRUE)
        {
            ii++;
            adaptCtleExec = GT_FALSE;
        }
        if (ii == MAX_ADAPTIVE_CTLE_PORTS_IN_CYCLE)
        {
            break;
        }
    }
    nextAdaptCtlePort = ((portIndex == maxApPortNum)? 0 : portIndex);
#endif /*RUN_ADAPTIVE_CTLE*/
#endif

#if defined (PIPE_DEV_SUPPORT) && ! defined(MICRO_INIT)
    /* LinkBindingFw */
    {
        GT_STATUS                   rc;
        GT_BOOL                     portUp, pairUp;
        unsigned char               port, pair;

        /* go over all link binding ports */
        for (port = 0; port < HWS_PIPE_PORTS_NUM_CNS; port++)
        {
            pair = linkBindingPorts[port].pair;

            /* First handle new requests */
            switch (linkBindingPorts[port].request) {
            case MV_HWS_LKB_REQUEST_NONE_E:
                continue;

            case MV_HWS_LKB_REQUEST_AP_REGISTER_E:
                if (linkBindingPorts[port].state == MV_HWS_LKB_NA_E) {
                    LKB_PRINT("AP port %u:[%u] registered\n", port, pair);
                    linkBindingPorts[port].state = MV_HWS_LKB_REGISTERED_E;
                }
                continue;

            case MV_HWS_LKB_REQUEST_AP_DEACTIVATE_E:
                if (linkBindingPorts[port].state > MV_HWS_LKB_REGISTERED_E)
                {
                    if (linkBindingPorts[port].state == MV_HWS_LKB_FAULT_E)
                        prvSetFaultAndSendDoorbell(0, 0, port, linkBindingPorts[port].portMode, GT_FALSE);

                    linkBindingPorts[port].portMode = NON_SUP_MODE;
                    linkBindingPorts[port].state = MV_HWS_LKB_REGISTERED_E;
                    continue;
                } else
                    break; /* no new req */

            case MV_HWS_LKB_REQUEST_ADD_PORT_E:
                if (linkBindingPorts[port].state <= MV_HWS_LKB_REGISTERED_E) {
                    linkBindingPortSet(port, pair, GT_TRUE, linkBindingPorts[port].isAP);
                    continue;
                } else
                    break;

            case MV_HWS_LKB_REQUEST_REMOVE_PORT_E:
                if (linkBindingPorts[port].state != MV_HWS_LKB_NA_E)
                    linkBindingPortSet(port, 0 /* dummy*/ , GT_FALSE, linkBindingPorts[port].isAP);

                continue; /*no new req - proceed to handeling port in SM */

            default:
                break;
            }

            /* if port is down - no need to read it's state and run SM (in case cable is disconnected -it will cause an error) */
            if (linkBindingPorts[port].state <= MV_HWS_LKB_REGISTERED_E)
                continue;

            /* now handle changing in port state */
            if (GT_OK != mvHwsPortLinkStatusGet(0, 0, port, linkBindingPorts[port].portMode, &portUp)) {
                 LKB_PRINT("mvHwsPortLinkStatusGet failed, port %d\n", port);
                 continue;
            }

            if (linkBindingPorts[pair].state > MV_HWS_LKB_REGISTERED_E) {
                if (GT_OK != mvHwsPortLinkStatusGet(0, 0, pair, linkBindingPorts[pair].portMode, &pairUp)) {
                    LKB_PRINT("mvHwsPortLinkStatusGet failed, pair %d\n", pair);
                    continue;
                }
            } else
                pairUp = GT_FALSE;

            LKB_PORT_DEBUG(port, "port %d: up/down %d, pair %d up/down: %d\n", port, portUp, pair, pairUp);

            switch (linkBindingPorts[port].state) {
            case MV_HWS_LKB_NO_FAULT_E:
                if ( (pairUp == GT_FALSE) &&
                     ( (linkBindingPorts[pair].state != MV_HWS_LKB_FAULT_E) && /* only one port in a pair can be 'fault' */
                       (linkBindingPorts[pair].state != MV_HWS_LKB_AWAKE_E)) )
                {
                    rc = prvSetFaultAndSendDoorbell(0, 0, port, linkBindingPorts[port].portMode, GT_TRUE);
                    if (rc != GT_OK)
                        break;

                    linkBindingPorts[port].state = MV_HWS_LKB_FAULT_E;
                    LKB_PRINT("port %d changed to fault\n", port);
                }
                break;

            case MV_HWS_LKB_FAULT_E:
                if (pairUp == GT_TRUE) {
                    rc = prvSetFaultAndSendDoorbell(0, 0, port, linkBindingPorts[port].portMode, GT_FALSE);
                    if (rc != GT_OK)
                        break;

                    linkBindingPorts[port].awakeTime = xTaskGetTickCount();
                    linkBindingPorts[port].state = MV_HWS_LKB_AWAKE_E;
                    LKB_PRINT("port %d changed to awake\n", port);
                }
                break;

            case MV_HWS_LKB_AWAKE_E:
                if (portUp == GT_TRUE) {
                    linkBindingPorts[port].state = MV_HWS_LKB_NO_FAULT_E;
                    LKB_PRINT("port %d changed to no-fault\n", port);
                } else if ( ((xTaskGetTickCount() - linkBindingPorts[port].awakeTime) / portTICK_RATE_MS)
                     > LKB_AWAKE_PERIOD_MS)
                {
                    /* Port didn't come up */
                    if (linkBindingPorts[pair].state <= MV_HWS_LKB_REGISTERED_E) {
                        LKB_PRINT("LKB error: pair of awaiking port is not LKB enabled\n");
                        break;
                    }

                    LKB_PRINT("port %d changed to no-fault\n", port);
                    linkBindingPorts[port].state = MV_HWS_LKB_NO_FAULT_E;
                };
                break;

            default:
                break;
            }

            if (mvLkbDebugPort == port)
                mvLkbDebugPort = LKB_PORT_NONE;

        } /* ports loop  */
    } /* LinkBindingFw */
#endif

    infoPtr->event = AP_PORT_DETECT_DELAY_EVENT;
}

/**
* @internal mvPortCtrlApPortMsgReply function
* @endinternal
*
* @brief   AP Port mng info message reply
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvPortCtrlApPortMsgReply(MV_HWS_IPC_REPLY_MSG_STRUCT *replyMsg, GT_U8 queueId)
{
    /* Build and Send IPC reply */
    CHECK_STATUS(mvHwsIpcReplyMsg(queueId, replyMsg));

    return GT_OK;
}

/**
* @internal mvPortCtrlApPortDetectionMsgExec function
* @endinternal
*
* @brief   AP Detect message state execution
*         - Extract Message type, Message params, and Port index
*         - Handle AP Enable message
*         AP State == IDLE
*         Extract and update message parameters
*         Execute port initial configuration
*         Trigger state change
*         AP State != IDLE
*         Send reply error message to calling applicaion
*         - Handle AP Disable message
*         AP State != IDLE
*         Execute port delete
*         AP State == IDLE
*         Print error message
*/
static void mvPortCtrlApPortDetectionMsgExec(MV_HWS_IPC_CTRL_MSG_STRUCT *msg)
{
    GT_U8 msgType;
    GT_U8 portIndex;
    MV_HWS_IPC_PORT_AP_DATA_STRUCT *msgParams;
    MV_HWS_AP_SM_INFO *apSm;
    MV_HWS_AP_SM_STATS *apStats;
    MV_HWS_AP_DETECT_ITEROP *apIntrop;
    MV_HWS_IPC_PORT_AP_INTROP_STRUCT  *apIntropSetMsg;
    MV_HWS_IPC_PORT_ENABLE_CTRL_DATA_STRUCT  *portEnableCtrlSetMsg;
    GT_U8 portNumBase = 0;
    MV_HWS_PORT_CTRL_PORT_SM       *portSm;
    MV_HWS_IPC_PORT_OPERATION_STRUCT  *portOperationMsg;
#if defined (ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)
    GT_U8 maxLanes, serdesIndex;
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
#endif

#if (defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
    MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_STC *portRxCfgOverrideMsg;
    GT_U8   i = 0;
#endif

#if defined (PIPE_DEV_SUPPORT) && ! defined(MICRO_INIT)
    MV_HWS_IPC_LINK_BINDING_STRUCT* portLKB;
#endif
/* Adaptive CTLE*/
#if (defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
#ifdef RUN_ADAPTIVE_CTLE
    MV_HWS_IPC_PORT_ADAPTIVE_CTLE_ENABLE_STC *adptCtleMsg;
    MV_HWS_IPC_NON_AP_PORT_IS_LINK_UP_STC *isLinkUpMsg;
    GT_U8 bitMapIndex;
#endif/*RUN_ADAPTIVE_CTLE*/
#endif

#if defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)  || defined(ALDRIN_DEV_SUPPORT)
    MV_HWS_AP_SM_TIMER *apTimer;
#endif

    MV_HWS_IPC_REPLY_MSG_STRUCT replyMsg;

    msgType = msg->ctrlMsgType;
    msgParams = (MV_HWS_IPC_PORT_AP_DATA_STRUCT*)&(msg->msgData);

    replyMsg.replyTo    = msgType;
    replyMsg.returnCode = GT_OK;

#ifdef BC2_DEV_SUPPORT
    if(BobcatA0 == hwsDeviceSpecInfo[0].devType)
    {
        portNumBase = MV_PORT_CTRL_BC2_AP_PORT_NUM_BASE;
    }
    else if(Alleycat3A0 == hwsDeviceSpecInfo[0].devType)
    {
        portNumBase = MV_PORT_CTRL_AC3_AP_PORT_NUM_BASE;
    }
    else
    {
        portNumBase = 0;
    }
#else
    portNumBase = MV_PORT_CTRL_AP_PORT_NUM_BASE;
#endif
    /* Validate input port number */
    if ((msgParams->phyPortNum < portNumBase) &&
        (msgType != MV_HWS_IPC_PORT_AP_INTROP_GET_MSG) &&
        (msgType != MV_HWS_IPC_PORT_AP_INTROP_SET_MSG) &&
        (msgType != MV_HWS_IPC_PORT_AP_DEBUG_GET_MSG))
    {
        AP_PRINT_MAC(("mvPortCtrlApPortDetectionMsgExec GT_BAD_PARAM msgParams->phyPortNum %d portNumBase %d\n",msgParams->phyPortNum, portNumBase));
        replyMsg.returnCode = GT_BAD_PARAM;
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        return;
    }

    portIndex = MV_HWS_PORT_CTRL_AP_LOGICAL_PORT_GET(msgParams->phyPortNum, portNumBase);
    apSm     = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].info);
    apStats  = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].stats);
    portSm    = &(mvHwsPortCtrlApPortMng.apPortSm[portIndex]);

    switch (msgType)
    {
    case MV_HWS_IPC_PORT_AP_ENABLE_MSG:
#if defined (ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)
        refClockSourceDb[msgParams->phyPortNum] = msgParams->refClockSrcParams;
#endif
        if (apSm->state == AP_PORT_SM_IDLE_STATE)
        {
#ifdef BOBK_DEV_SUPPORT
            MV_HWS_PORT_INIT_PARAMS portParams;
#endif

            /*apSm->queueId = msg->msgQueueId;*/
            AP_CTRL_QUEUEID_SET(apSm->ifNum, msg->msgQueueId)
            apSm->portNum = msgParams->phyPortNum;
            AP_CTRL_LANE_SET(apSm->ifNum, msgParams->laneNum);
#ifdef BOBK_DEV_SUPPORT
            mvHwsGetPortParameters(msgParams->macNum, Port_10GBase_R, &portParams);
            AP_CTRL_PCS_SET(apSm->ifNum, msgParams->pcsNum + (portParams.firstLaneNum - msgParams->laneNum));
#else
            AP_CTRL_PCS_SET(apSm->ifNum, msgParams->pcsNum);
#endif
            AP_CTRL_MAC_SET(apSm->ifNum, msgParams->macNum);
            apSm->capability = 0;
            AP_CTRL_ADV_ALL_SET(apSm->capability, msgParams->advMode);
            apSm->options = 0;
            AP_CTRL_LB_EN_SET(apSm->options, AP_CTRL_LB_EN_GET(msgParams->options));
            AP_CTRL_FC_PAUSE_SET(apSm->options, AP_CTRL_FC_PAUSE_GET(msgParams->options));
            AP_CTRL_FC_ASM_SET(apSm->options, AP_CTRL_FC_ASM_GET(msgParams->options));
            AP_CTRL_FEC_ABIL_SET(apSm->options, AP_CTRL_FEC_ABIL_GET(msgParams->options));
            AP_CTRL_FEC_REQ_SET(apSm->options, AP_CTRL_FEC_REQ_GET(msgParams->options));
            AP_CTRL_FEC_ABIL_CONSORTIUM_SET(apSm->options, AP_CTRL_FEC_ABIL_CONSORTIUM_GET(msgParams->options));
            AP_CTRL_FEC_REQ_CONSORTIUM_SET(apSm->options, AP_CTRL_FEC_REQ_CONSORTIUM_GET(msgParams->options));
            AP_CTRL_FEC_ADVANCED_REQ_SET(apSm->options, AP_CTRL_FEC_ADVANCED_REQ_GET(msgParams->options));
#if defined(BC3_DEV_SUPPORT) || defined(BOBK_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
            AP_CTRL_CTLE_BIAS_VAL_SET(apSm->options, AP_CTRL_CTLE_BIAS_VAL_GET(msgParams->options));
#endif
#if defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT)
            AP_CTRL_20G_R1_SET(apSm->options, AP_CTRL_20G_R1_GET(msgParams->options));
#endif
            AP_PRINT_MAC(("detect port:%d idx:%d capability:0x%x advMode:0x%x options:0x%x\n",msgParams->phyPortNum, portIndex, apSm->capability, msgParams->advMode, apSm->options));
            apSm->polarityVector = msgParams->polarityVector;

            /* Initial configuration */
            mvHwsInitialConfiguration(portIndex);

#if defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT)
            apTimer = &(mvHwsPortCtrlApPortDetect.apPortSm[portIndex].timer);
           /* set timer  when initializing port, each port will start AP in a different time*/
            mvPortCtrlThresholdSet(
            ((portIndex *10) + 10/*minimum delay*/), &(apTimer->abilityThreshold));
#endif

            apSm->status = AP_PORT_START_EXECUTE;
            apSm->state = AP_PORT_SM_INIT_STATE;
            mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        }
        else
        {
            mvPcPrintf("Error, AP Detect, Port-%d is under execution[0x%x], AP Enable is not valid!!!\n", msgParams->phyPortNum, apSm->state);
            replyMsg.returnCode = GT_CREATE_ERROR;
            mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        }
        break;

    case MV_HWS_IPC_PORT_AP_DISABLE_MSG:
        if (apSm->state != AP_PORT_SM_IDLE_STATE)
        {
#if defined (ALDRIN_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(AC5_DEV_SUPPORT)
            maxLanes = mvPortCtrlApPortGetMaxLanes(apSm->capability);
            if (maxLanes > 0)
            {
                curPortParams.numOfActLanes = maxLanes;
                curPortParams.portStandard = _10GBase_KR; /* parameter is ignored */
                curPortParams.firstLaneNum = apSm->portNum;
#ifdef AC5_DEV_SUPPORT
                mvHwsGetPortParameters(apSm->portNum, Port_10GBase_R , &curPortParams);
#else
                hwsPortParamsGetLanes(0, 0, apSm->portNum, &curPortParams);
#endif
                for (serdesIndex = 0; serdesIndex < maxLanes; serdesIndex++)
                {
#ifdef AC5_DEV_SUPPORT
                    mvHwsComphySerdesManualDBClear(0, apSm->portNum, curPortParams.activeLanesList[serdesIndex]);
#else
                    mvHwsAvagoSerdesManualDBClear(0, apSm->portNum, curPortParams.activeLanesList[serdesIndex]);
#endif
                }
            }
#endif
            mvApPortDeleteMsg(portIndex);
            mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        }
        else
        {
            mvPcPrintf("Error, AP Detect, Port-%d is NOT under execution, AP Disable is not valid!!!\n", msgParams->phyPortNum);
            replyMsg.returnCode = GT_FAIL;
            mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        }
        break;

    case MV_HWS_IPC_PORT_AP_CFG_GET_MSG:
        replyMsg.readData.portApCfgGet.ifNum      = apSm->ifNum;
        replyMsg.readData.portApCfgGet.capability = apSm->capability;
        replyMsg.readData.portApCfgGet.options    = apSm->options;
        AP_PRINT_MAC(("detect MV_HWS_IPC_PORT_AP_CFG_GET_MSG port:%d idx:%d capability:0x%x options:0x%x\n",msgParams->phyPortNum, portIndex, apSm->capability, apSm->options));
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_AP_STATUS_MSG:
          /* return AP disabled only after both state machines reached idle state and application asked to disable AP on port */
        replyMsg.readData.portApStatusGet.state       = ((PORT_SM_IDLE_STATE == portSm->state) && (AP_PORT_SM_IDLE_STATE == apSm->state))?
                                                         0 : (AP_PORT_SM_IDLE_STATE == apSm->state)? 1: apSm->state;
        replyMsg.readData.portApStatusGet.status      = apSm->status;
        replyMsg.readData.portApStatusGet.laneNum     = AP_CTRL_LANE_GET(apSm->ifNum);
        replyMsg.readData.portApStatusGet.ARMSmStatus = apSm->ARMSmStatus;
        if ((AP_ST_HCD_TYPE_GET(apSm->hcdStatus) == Port_25GBASE_KR) && (AP_CTRL_20G_R1_GET(apSm->options))) {
            AP_ST_HCD_TYPE_SET(apSm->hcdStatus, Port_20GBASE_KR);
        }
        replyMsg.readData.portApStatusGet.hcdStatus   = apSm->hcdStatus;
        AP_PRINT_MAC(("detect MV_HWS_IPC_PORT_AP_STATUS_MSG port:%d idx:%d capability:0x%x options:0x%x hcdStatus:0x%x\n",msgParams->phyPortNum, portIndex, apSm->capability, apSm->options,apSm->hcdStatus));
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_AP_STATS_MSG:
        replyMsg.readData.portApStatsGet.txDisCnt          = apStats->txDisCnt;
        replyMsg.readData.portApStatsGet.abilityCnt        = apStats->abilityCnt;
        replyMsg.readData.portApStatsGet.abilitySuccessCnt = apStats->abilitySuccessCnt;
        replyMsg.readData.portApStatsGet.linkFailCnt       = apStats->linkFailCnt;
        replyMsg.readData.portApStatsGet.linkSuccessCnt    = apStats->linkSuccessCnt;
        replyMsg.readData.portApStatsGet.hcdResoultionTime = apStats->hcdResoultionTime;
        replyMsg.readData.portApStatsGet.linkUpTime        = apStats->linkUpTime;
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_AP_STATS_RESET_MSG:
        mvPortCtrlDbgStatsReset(msgParams->phyPortNum);
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_AP_INTROP_GET_MSG:
        apIntrop = &(mvHwsPortCtrlApPortDetect.introp);
        replyMsg.readData.portApIntropGet.txDisDuration          = apIntrop->txDisDuration;
        replyMsg.readData.portApIntropGet.abilityDuration        = apIntrop->abilityDuration;
        replyMsg.readData.portApIntropGet.abilityMaxInterval     = apIntrop->abilityMaxInterval;
        replyMsg.readData.portApIntropGet.abilityFailMaxInterval = apIntrop->abilityFailMaxInterval;
        replyMsg.readData.portApIntropGet.apLinkDuration         = apIntrop->apLinkDuration;
        replyMsg.readData.portApIntropGet.apLinkMaxInterval      = apIntrop->apLinkMaxInterval;
        replyMsg.readData.portApIntropGet.pdLinkDuration         = apIntrop->pdLinkDuration;
        replyMsg.readData.portApIntropGet.pdLinkMaxInterval      = apIntrop->pdLinkMaxInterval;
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_AP_INTROP_SET_MSG:
        apIntropSetMsg  = (MV_HWS_IPC_PORT_AP_INTROP_STRUCT*)&(msg->msgData);
        mvPortCtrlDbgIntropCfg(apIntropSetMsg);
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_AP_DEBUG_GET_MSG:
        mvPortCtrlLogInfoGet(&replyMsg.readData.logGet.fwBaseAddr,
                &replyMsg.readData.logGet.logBaseAddr,
                &replyMsg.readData.logGet.logPointerAddr,
                &replyMsg.readData.logGet.logCountAddr,
                &replyMsg.readData.logGet.logResetAddr);
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_ENABLE_CTRL_SET_MSG:
        portEnableCtrlSetMsg = (MV_HWS_IPC_PORT_ENABLE_CTRL_DATA_STRUCT*)&(msg->msgData);
        mvHwsApPortEnableCtrlSet((GT_BOOL)portEnableCtrlSetMsg->portEnableCtrl);
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

   case MV_HWS_IPC_PORT_ENABLE_CTRL_GET_MSG:
        mvHwsApPortEnableCtrlGet((GT_BOOL*)&replyMsg.readData.portEnableCtrlGet.portEnableCtrl);
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

#if (defined (BOBK_DEV_SUPPORT) || defined(BC3_DEV_SUPPORT) || defined(ALDRIN_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
   case MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_MSG:
        portRxCfgOverrideMsg = (MV_HWS_IPC_PORT_SERDES_RX_CONFIG_OVERRIDE_STC*)&(msg->msgData);
        if (portRxCfgOverrideMsg->rxCfg.serdesSpeed == _10_3125G) {
            i = MV_HWS_MAN_TUNE_CTLE_CONFIG_10G_INDEX_CNS;
        }
        else
        {
            i = MV_HWS_MAN_TUNE_CTLE_CONFIG_25G_INDEX_CNS;
        }
        portSm->apPortRxConfigOverride[i].bandLoopWidth = (GT_U8)(portRxCfgOverrideMsg->rxCfg.ctleParams.bandWidth & 0xF);
        portSm->apPortRxConfigOverride[i].dcGain = (GT_U8)(portRxCfgOverrideMsg->rxCfg.ctleParams.dcGain & 0xFF);
        portSm->apPortRxConfigOverride[i].lfHf = (GT_U8)((portRxCfgOverrideMsg->rxCfg.ctleParams.lowFrequency & 0xF) |
                ((portRxCfgOverrideMsg->rxCfg.ctleParams.highFrequency& 0xF)<< MV_HWS_MAN_TUNE_CTLE_CONFIG_OVERRIDE_HF_SHIFT));
        portSm->apPortRxConfigOverride[i].squelch = (GT_U16)(portRxCfgOverrideMsg->rxCfg.ctleParams.squelch & 0xFFFF);
        portSm->apPortRxConfigOverride[i].fieldOverrideBmp = (GT_U16)(portRxCfgOverrideMsg->rxCfg.fieldOverrideBmp & 0xFFFF);
        portSm->apPortRxConfigOverride[i].etlMinDelay = (GT_U8)(portRxCfgOverrideMsg->rxCfg.etlParams.etlMinDelay & 0xFF);
        portSm->apPortRxConfigOverride[i].etlMaxDelay = (GT_U8)((portRxCfgOverrideMsg->rxCfg.etlParams.etlMaxDelay & 0x7F) |
            ((portRxCfgOverrideMsg->rxCfg.etlParams.etlEnableOverride & 0x1) << MV_HWS_MAN_TUNE_ETL_CONFIG_OVERRIDE_ENABLE_SHIFT));
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;
#endif

#if defined (PIPE_DEV_SUPPORT) && ! defined(MICRO_INIT)
    case MV_HWS_IPC_PORT_LINK_BINDING_SET_MSG:
        portLKB = (MV_HWS_IPC_LINK_BINDING_STRUCT*)&(msg->msgData);
        mvLkbReqPortSet(portLKB->phyPortNum,
                        portLKB->portMode,
                        portLKB->phyPairNum,
                        (GT_BOOL)(portLKB->enable),
                        GT_FALSE);
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    case MV_HWS_IPC_PORT_LINK_BINDING_REGISTER_AP_MSG:
        portLKB = (MV_HWS_IPC_LINK_BINDING_STRUCT*)&(msg->msgData);
        mvLkbReqApPortRegister(portLKB->phyPortNum,
                                  portLKB->phyPairNum);
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;
#endif
    case MV_HWS_IPC_PORT_OPERATION_MSG:
        portOperationMsg = (MV_HWS_IPC_PORT_OPERATION_STRUCT*)&(msg->msgData);
        AP_PRINT_MAC2(("MV_HWS_IPC_PORT_OPERATION_MSG port:%d  \n", portIndex));
        if (portOperationMsg->portOperation == MV_HWS_IPC_PORT_OPERATION_DUMP)
        {
            mvPortCtrlApPortdump(apSm->portNum);
            printf("AP port %d dump; apSm->status %d apSm->state %d portSm->status %d portSm->state %d\n", portIndex,
                   apSm->status, apSm->state, portSm->status, portSm->state);
            mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(portIndex, 0, apSm->status, apSm->state));
            mvPortCtrlLogAdd(AN_PORT_MNG_LOG_DEBUG(portIndex, 1, portSm->status, portSm->state));

        }

        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

#if (defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))
#ifdef RUN_ADAPTIVE_CTLE
    case MV_HWS_IPC_PORT_ADAPTIVE_CTLE_ENABLE_MSG:
        /* enable/disable adapive CTLE per port*/
        adptCtleMsg = (MV_HWS_IPC_PORT_ADAPTIVE_CTLE_ENABLE_STC*)&(msg->msgData);
        i /*bitMapIndex*/ =  (adptCtleMsg->phyPortNum)/HWS_MAX_ADAPT_CTLE_DB_SIZE;
        if ((adptCtleMsg->enable) == 1)
        {
            hwsDeviceSpecInfo[0].adaptiveCtleDb.bitMapEnableCtleCalibrationBasedTemperture[i] |= (0x1 << ((adptCtleMsg->phyPortNum)%HWS_MAX_ADAPT_CTLE_DB_SIZE));
        }
        else
        {
            hwsDeviceSpecInfo[0].adaptiveCtleDb.bitMapEnableCtleCalibrationBasedTemperture[i] &= ~(0x1 << ((adptCtleMsg->phyPortNum)% HWS_MAX_ADAPT_CTLE_DB_SIZE));
        }
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;

    /*this case is for non ap port, running adaptive ctle in CM3*/
    case MV_HWS_IPC_NON_AP_PORT_IS_LINK_UP_MSG:
        isLinkUpMsg = (MV_HWS_IPC_NON_AP_PORT_IS_LINK_UP_STC *)&(msg->msgData);
        bitMapIndex = (isLinkUpMsg->phyPortNum)/HWS_MAX_ADAPT_CTLE_DB_SIZE;

        if ((isLinkUpMsg->isLinkUp) == 1)
        {
            if (apSm->state == AP_PORT_SM_IDLE_STATE)
            {
                if (GT_FALSE ==((hwsDeviceSpecInfo[0].adaptiveCtleDb.bitMapadaptCtleMode[bitMapIndex] & (0x1 << ((isLinkUpMsg->phyPortNum)%HWS_MAX_ADAPT_CTLE_DB_SIZE))) > 0 ? GT_TRUE : GT_FALSE))
                {
                    /*mark port as non ap port*/
                    hwsDeviceSpecInfo[0].adaptiveCtleDb.bitMapadaptCtleMode[bitMapIndex] |= (0x1 << ((isLinkUpMsg->phyPortNum)%HWS_MAX_ADAPT_CTLE_DB_SIZE));
                }

                /* get adaptive CTLE Params: train Lf, trainDelay, currDelay, serdes list, num of serdeses*/
                for (i = 0 ; i < isLinkUpMsg->numOfSerdeses; i++ )
                {
                    hwsDeviceSpecInfo[0].adaptiveCtleDb.adaptCtleParams[isLinkUpMsg->serdesList[i]].trainLf = (isLinkUpMsg->trainLf[i]);
                    hwsDeviceSpecInfo[0].adaptiveCtleDb.adaptCtleParams[isLinkUpMsg->serdesList[i]].enhTrainDelay =  (isLinkUpMsg->enhTrainDelay[i]);
                    hwsDeviceSpecInfo[0].adaptiveCtleDb.adaptCtleParams[isLinkUpMsg->serdesList[i]].currSerdesDelay = (isLinkUpMsg->enhTrainDelay[i]);
                    hwsDeviceSpecInfo[0].adaptiveCtleDb.portsSerdes[isLinkUpMsg->phyPortNum].serdesList[i] = isLinkUpMsg->serdesList[i];
                }
                hwsDeviceSpecInfo[0].adaptiveCtleDb.portsSerdes[isLinkUpMsg->phyPortNum].numOfSerdeses =  isLinkUpMsg->numOfSerdeses;
                apSm->status = NON_AP_ADAPTIVE_CTLE_INIT;
                apSm->state = NON_AP_PORT_SM_ADAPTIVE_CTLE_STATE;
            }
            else
            {
                mvPcPrintf("Error, AP Detect, Port-%d is under execution[0x%x], AP Enable is not valid!!!\n", msgParams->phyPortNum, apSm->state);
                replyMsg.returnCode = GT_FAIL;
            }
        }
        else /* Port is link down, remove from non-AP ports list.*/
        {
            if (apSm->state != AP_PORT_SM_IDLE_STATE)
            {
                if (GT_TRUE ==((hwsDeviceSpecInfo[0].adaptiveCtleDb.bitMapadaptCtleMode[bitMapIndex] & (0x1 << ((isLinkUpMsg->phyPortNum)%HWS_MAX_ADAPT_CTLE_DB_SIZE))) > 0 ? GT_TRUE : GT_FALSE))
                {
                    hwsDeviceSpecInfo[0].adaptiveCtleDb.bitMapadaptCtleMode[bitMapIndex] &= ~(0x1 << ((isLinkUpMsg->phyPortNum)%HWS_MAX_ADAPT_CTLE_DB_SIZE));
                }
                apSm->status = PORT_SM_NOT_RUNNING;
                apSm->state = AP_PORT_SM_IDLE_STATE;
            }
            else
            {
                mvPcPrintf("Error, AP Detect, Port-%d is under execution[0x%x], AP Enable is not valid!!!\n", msgParams->phyPortNum, apSm->state);
                replyMsg.returnCode = GT_FAIL;
            }
        }
        mvPortCtrlApPortMsgReply(&replyMsg, msg->msgQueueId);
        break;
#endif /*RUN_ADAPTIVE_CTLE*/
#endif /*(defined(BC3_DEV_SUPPORT) || defined(PIPE_DEV_SUPPORT) || defined(ALDRIN2_DEV_SUPPORT))*/
    }

}

/**
* @internal mvPortCtrlApDetectMsg function
* @endinternal
*
* @brief   AP Detect message state execution
*         - Read message from AP detection message queue
*         Message received
*         Execute message to completion
*         Message not received
*         Trigger state change
*/
void mvPortCtrlApDetectMsg(void)
{
    GT_STATUS rcode;

    MV_HWS_IPC_CTRL_MSG_STRUCT recvMsg;

    osMemSet(&recvMsg, 0, sizeof(recvMsg));
    infoPtr->state = AP_PORT_DETECT_MSG_PROCESS_STATE;

    do
    {
        rcode = mvPortCtrlProcessMsgRecv(M5_AP_PORT_DET, MV_PROCESS_MSG_RX_DELAY, &recvMsg);
        if (rcode == GT_OK)
        {
            mvPortCtrlApPortDetectionMsgExec(&recvMsg);
        }
    } while (rcode == GT_OK);

    infoPtr->event = AP_PORT_DETECT_ACTIVE_PORT_EVENT;
}

/**
* @internal mvPortCtrlApDetectDelay function
* @endinternal
*
* @brief   AP Delay state execution
*/
void mvPortCtrlApDetectDelay(void)
{
    infoPtr->state = AP_PORT_DETECT_DELAY_STATE;
    do
    {
        /* When Avago GUI is enabled, AP periodic execution is placed on hold
        ** AP process will not execute any activity
        */
        mvPortCtrlProcessDelay(mvHwsPortCtrlApPortDetect.info.delayDuration);
    } while (mvHwsApDetectExecutionEnabled == MV_PORT_CTRL_AVAGO_GUI_ENABLED);

    infoPtr->event = AP_PORT_DETECT_MSG_PROCESS_EVENT;
}


