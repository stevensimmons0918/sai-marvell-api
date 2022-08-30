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
* @file appDemoDxChEventHandle.c
*
* @brief this library contains the implementation of the event handling functions
* for the DXCH device , and the redirection to the GalTis Agent for
* extra actions .
*
*
* ---> this file is compiled only under CHX_FAMILY <---
*
*
* @version   16
********************************************************************************
*/

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfMii.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <appDemo/userExit/userEventHandler.h>
#include <appDemo/userExit/dxCh/appDemoDxChEventHandle.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwInit.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/****************************************************************************
* Debug                                                                     *
****************************************************************************/

#define BUFF_LEN    20

#define MAX_REGISTER    5
static GT_U32                   numRegistrations=0;
static RX_PACKET_RECEIVE_CB_FUN rxPktReceiveCbArray[MAX_REGISTER];
extern GT_UINTPTR rxEventHanderHnd;

/* workaround */
static GT_STATUS prvAppDemoDxChNetRxPacketRegisteredCb
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
* @internal appDemoDxChNetRxPacketCbRegister function
* @endinternal
*
* @brief   register a CB function to be called on every RX packet to CPU
*         NOTE: function is implemented in the 'AppDemo' (application)
* @param[in] rxPktReceiveCbFun        - CB function to be called on every RX packet to CPU
*/
GT_STATUS appDemoDxChNetRxPacketCbRegister
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
            prvAppDemoDxChNetRxPacketRegisteredCb,
            0);

    return GT_OK;
}


/**
* @internal appDemoDxChNetRxPktHandle function
* @endinternal
*
* @brief   application routine to receive frames . -- DXCH function
*
* @param[in] devNum                   - Device number.
* @param[in] queueIdx                 - The queue from which this packet was received.
*                                       None.
*/
GT_STATUS appDemoDxChNetRxPktHandle
(
    IN GT_UINTPTR                           evHandler,
    IN RX_EV_HANDLER_EV_TYPE_ENT            evType,
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx
)
{
    GT_STATUS                           rc;
    GT_U32                              numOfBuff = BUFF_LEN;
    GT_U8                               *packetBuffs[BUFF_LEN];
    GT_U32                              buffLenArr[BUFF_LEN];
    CPSS_DXCH_NET_RX_PARAMS_STC         rxParams;

    /* get the packet from the device */
    if(appDemoPpConfigList[devNum].cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
    {
        rc = cpssDxChNetIfSdmaRxPacketGet(devNum, queueIdx,&numOfBuff,
                            packetBuffs,buffLenArr,&rxParams);
    }
    else if(appDemoPpConfigList[devNum].cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E)
    {
        rc = cpssDxChNetIfMiiRxPacketGet(devNum,queueIdx,&numOfBuff,packetBuffs,buffLenArr,&rxParams);
    }
    else
    {
        rc = GT_FAIL;
    }

    if (rc != GT_OK)  /* GT_NO_MORE is good and not an error !!! */
    {
        /* no need to free buffers because not got any */
        return rc;
    }

    if(evHandler)
    {
        rxEventHandlerDoCallbacks(evHandler, evType,
            devNum,queueIdx,numOfBuff,
            packetBuffs,buffLenArr,&rxParams);
    }

    if(appDemoPpConfigList[devNum].cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
    {
        /* now you need to free the buffers */
        if(appDemoPpConfigList[devNum].allocMethod != CPSS_RX_BUFF_NO_ALLOC_E)
        {
            rc = cpssDxChNetIfRxBufFree(devNum,queueIdx,packetBuffs,numOfBuff);
        }
        else
        {

            /* move the first buffer pointer to the original place*/
            packetBuffs[0] = (GT_U8*)((((GT_UINTPTR)(packetBuffs[0])) >> APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF)
                                      << APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF);

            /* fix the last buffer's size*/
            buffLenArr[numOfBuff-1] = APP_DEMO_RX_BUFF_SIZE_DEF;

            rc = cpssDxChNetIfRxBufFreeWithSize(devNum,queueIdx,packetBuffs,buffLenArr,numOfBuff);
        }

    }
    else /* CPSS_NET_CPU_PORT_MODE_MII_E */
    {
        rc = cpssDxChNetIfMiiRxBufFree(devNum,queueIdx,packetBuffs,numOfBuff);
    }

    return rc;
}

extern void dxChNetIfRxPacketParse_DebugDumpEnable
(
    IN GT_BOOL  enableDumpRxPacket
);
/**
* @internal appDemoDxChNetRxPktHandle_allQueues function
* @endinternal
*
* @brief   allow call from terminal to treat Rx Packets to the CPU. -- DXCH function

*
* @param[in] devNum                   - Device number.
*
*/
GT_STATUS appDemoDxChNetRxPktHandle_allQueues
(
    IN GT_U8                                devNum
)
{
    GT_STATUS   rc;
    GT_U8    queueIdx;
    GT_UINTPTR  evHandler = 0;
    GT_U32      numOfNetIfs = 1;
    /* allow to 'see' the packets that we get (with the DSA) */
    dxChNetIfRxPacketParse_DebugDumpEnable(GT_TRUE);

    rc = prvCpssDxChNetIfMultiNetIfNumberGet(devNum,&numOfNetIfs);
    if (rc != GT_OK)
    {
        numOfNetIfs = 1;
    }


    for(queueIdx = 0 ;queueIdx < (8*numOfNetIfs) ; queueIdx++)
    {
        appDemoDxChNetRxPktHandle(evHandler,0,devNum,queueIdx);
    }

    return GT_OK;
}

/**
* @internal appDemoDxChNetRxPktGet function
* @endinternal
*
* @brief   application routine to get the packets from the queue and
*         put it in msgQ. -- DXCH function
* @param[in] devNum                   - Device number.
* @param[in] queueIdx                 - The queue from which this packet was received.
* @param[in] msgQId                   - Message queue Id.
*                                       None.
*/
GT_STATUS appDemoDxChNetRxPktGet
(
    IN GT_U8            devNum,
    IN GT_U8            queueIdx,
    IN CPSS_OS_MSGQ_ID  msgQId
)
{
    GT_STATUS                 rc;
    APP_DEMO_RX_PACKET_PARAMS rxParams;
    rxParams.numOfBuff = BUFF_LEN;
    cpssOsMutexLock(rxMutex);
    /* get the packet from the device */
    if(appDemoPpConfigList[devNum].cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
    {
        rc = cpssDxChNetIfSdmaRxPacketGet(devNum, queueIdx,&(rxParams.numOfBuff),
                            rxParams.packetBuffs,rxParams.buffLenArr,
                                          &(rxParams.dxChNetRxParams));
    }
    else if(appDemoPpConfigList[devNum].cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E)
    {
        rc = cpssDxChNetIfMiiRxPacketGet(devNum, queueIdx,&(rxParams.numOfBuff),
                            rxParams.packetBuffs,rxParams.buffLenArr,
                                          &(rxParams.dxChNetRxParams));
    }
    else
    {
        rc = GT_FAIL;
    }
    cpssOsMutexUnlock(rxMutex);

    if (rc != GT_OK)
    {
        /* no need to free buffers because not got any */
        return rc;
    }

    rxParams.devNum = devNum;
    rxParams.queue = queueIdx;
    /* put in msgQ */
    rc = cpssOsMsgQSend(msgQId,&rxParams,
                        sizeof(APP_DEMO_RX_PACKET_PARAMS),CPSS_OS_MSGQ_WAIT_FOREVER);
    return rc;
}

/**
* @internal appDemoDxChNetRxPktTreat function
* @endinternal
*
* @brief   application routine to treat the packets . -- DXCH function
*
* @param[in] rxParamsPtr              - (pointer to) rx paacket params
*                                       None.
*/
GT_STATUS appDemoDxChNetRxPktTreat
(
    IN APP_DEMO_RX_PACKET_PARAMS  *rxParamsPtr
)
{
    GT_STATUS   rc;
    GT_U8       devNum ;
    if (rxParamsPtr == NULL)
    {
        /* debug */
        return GT_BAD_PTR;
    }

    devNum = rxParamsPtr->devNum;

    rxEventHandlerDoCallbacks(rxEventHanderHnd, RX_EV_HANDLER_TYPE_RX_E,
            rxParamsPtr->devNum,rxParamsPtr->queue,
            rxParamsPtr->numOfBuff,rxParamsPtr->packetBuffs,
            rxParamsPtr->buffLenArr,&(rxParamsPtr->dxChNetRxParams));

    cpssOsMutexLock(rxMutex);
    if(appDemoPpConfigList[devNum].cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
    {
        /* now you need to free the buffers */
        if(appDemoPpConfigList[devNum].allocMethod != CPSS_RX_BUFF_NO_ALLOC_E)
        {
            rc = cpssDxChNetIfRxBufFree(rxParamsPtr->devNum,rxParamsPtr->queue,rxParamsPtr->packetBuffs,
                                        rxParamsPtr->numOfBuff);
        }
        else
        {

            /* move the first buffer pointer to the original place*/
            rxParamsPtr->packetBuffs[0] = (GT_U8*)((((GT_UINTPTR)(rxParamsPtr->packetBuffs[0])) >> APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF)
                                      << APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF);
            /* fix the last buffer's size*/
            rxParamsPtr->buffLenArr[rxParamsPtr->numOfBuff-1] = APP_DEMO_RX_BUFF_SIZE_DEF;

            rc = cpssDxChNetIfRxBufFreeWithSize(rxParamsPtr->devNum,rxParamsPtr->queue,rxParamsPtr->packetBuffs,
                                                rxParamsPtr->buffLenArr,rxParamsPtr->numOfBuff);
        }
    }
    else if (appDemoPpConfigList[devNum].cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E)
    {
        rc = cpssDxChNetIfMiiRxBufFree(rxParamsPtr->devNum,rxParamsPtr->queue
                                    ,rxParamsPtr->packetBuffs,rxParamsPtr->numOfBuff);
    }
    else
    {
        rc = GT_FAIL;
    }
    cpssOsMutexUnlock(rxMutex);

    return rc;
}
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/cpssDxChBrgFdbManager.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_utils.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_db.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_debug.h>

#ifdef  INCLUDE_UTF
extern GT_STATUS cpssDxChFdbDump(IN GT_U8 devNum);
#endif  /*INCLUDE_UTF*/
GT_STATUS fdb_manager_db_check(IN GT_U32    fdbManagerId)
{
    GT_STATUS   rc;
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_RESULT_ENT          testResultArray[CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS___LAST___E];

    GT_U32                                                  errorNum, errorNumItr;
    static CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_STC          dbCheckParam;

    /* set to enable all the checkers */
    CPSS_DXCH_BRG_FDB_MANAGER_DB_CHECKS_DO_ALL_MAC(dbCheckParam);

    errorNum = 0;
    rc = cpssDxChBrgFdbManagerDatabaseCheck(fdbManagerId, &dbCheckParam, testResultArray, &errorNum);
    if (rc != GT_OK)
    {
        cpssOsPrintf("cpssDxChBrgFdbManagerDatabaseCheck failed rc[%d] \n",rc);
        return rc;
    }

    if(errorNum == 0)
    {
        cpssOsPrintf("no errors \n");
        return GT_OK;
    }

    cpssOsPrintf("got [%d] ERRORs: \n ",errorNum);
    /*For each error ' print 'type'*/
    for(errorNumItr = 0; errorNumItr < errorNum; errorNumItr++)
    {
        cpssOsPrintf("ERROR[%d] , testResultArray[%d] \n " ,
            errorNumItr ,
            testResultArray[errorNumItr]);
    }

    return GT_OK;
}

GT_STATUS fdb_manager_db_print(IN GT_U32    fdbManagerId)
{
#ifdef  INCLUDE_UTF
    (void)cpssDxChFdbDump(0/*devNum*/);
#endif /*INCLUDE_UTF*/

    prvCpssDxChFdbManagerDebugPrintValidEntries_mac   (fdbManagerId);
    prvCpssDxChFdbManagerDebugPrintValidEntries_ipv4Uc(fdbManagerId);
    prvCpssDxChFdbManagerDebugPrintValidEntries_ipv6Uc(fdbManagerId);

    prvCpssDxChFdbManagerDebugPrintCounters  (fdbManagerId);
    prvCpssDxChFdbManagerDebugPrintStatistics(fdbManagerId);



    return GT_OK;
}

GT_STATUS fdb_manager_addipv4_uc_incremental(
    IN GT_U32       fdbManagerId ,
    IN GT_U8        devNum,
    IN GT_U32       portNum,
    IN GT_U32       base_ipv4Addr,
    IN GT_U32       vrfId,
    IN GT_U32       numOfEntries
)
{
    GT_STATUS   rc;
    GT_U32      ii;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC              entry;
    CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_ADD_PARAMS_STC   params;
    CPSS_DXCH_BRG_FDB_MANAGER_IPV4_UC_ROUTING_INFO_STC  *fdbEntryIpv4UcFormatPtr;
    GT_U32      ipv4Addr;

    cpssOsMemSet(&params,0,sizeof(params));

    entry.fdbEntryType = CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_TYPE_IPV4_UC_E;
    fdbEntryIpv4UcFormatPtr = &entry.format.fdbEntryIpv4UcFormat;
    fdbEntryIpv4UcFormatPtr->vrfId = vrfId;
    fdbEntryIpv4UcFormatPtr->age = GT_TRUE;
    fdbEntryIpv4UcFormatPtr->ucRouteType = CPSS_DXCH_BRG_FDB_MANAGER_UC_ROUTING_TYPE_NEXT_HOP_INFO_FULL_IN_FDB_ARP_PTR_E;
    cpssOsMemSet(&fdbEntryIpv4UcFormatPtr->ucRouteInfo.fullFdbInfo,0,sizeof(fdbEntryIpv4UcFormatPtr->ucRouteInfo.fullFdbInfo));

    fdbEntryIpv4UcFormatPtr->ucRouteInfo.fullFdbInfo.dstInterface.type = CPSS_INTERFACE_PORT_E;
    fdbEntryIpv4UcFormatPtr->ucRouteInfo.fullFdbInfo.dstInterface.interfaceInfo.devPort.portNum = portNum;
    fdbEntryIpv4UcFormatPtr->ucRouteInfo.fullFdbInfo.dstInterface.interfaceInfo.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);

    for(ii = 0 ; ii < numOfEntries ; ii++)
    {
        ipv4Addr = base_ipv4Addr + ii;

        fdbEntryIpv4UcFormatPtr->ipv4Addr.arIP[0] = (GT_U8)(ipv4Addr >> 24);
        fdbEntryIpv4UcFormatPtr->ipv4Addr.arIP[1] = (GT_U8)(ipv4Addr >> 16);
        fdbEntryIpv4UcFormatPtr->ipv4Addr.arIP[2] = (GT_U8)(ipv4Addr >>  8);
        fdbEntryIpv4UcFormatPtr->ipv4Addr.arIP[3] = (GT_U8)(ipv4Addr >>  0);

        rc = cpssDxChBrgFdbManagerEntryAdd(fdbManagerId,&entry,&params);
        if(rc != GT_OK)
        {
            cpssOsPrintf("failed rc[%d] on ip[0x%8.8x]\n",
                rc,fdbEntryIpv4UcFormatPtr->ipv4Addr.u32Ip);
        }
    }

    return GT_OK;
}

/**
* @internal appDemoDxChPortFecLaneIndexGet function
* @endinternal
*
* @brief    Gets FEC lane index per physical port
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   Lion2; Bobcat2; Caelum;
*                                   AC5P;Aldrin; AC3X; xCat3;
*                                   AC5; Bobcat3; Aldrin2; AC5X; Harrier; Ironman.
*
* @param[in]    devNum              - physical device number
* @param[in]    portNum             - physical port number
* @param[out]   fecLaneIndexPtr     - (pointer to) first FEC virtual lane global index per physical port
* @param[out]   fecLanesNumbersPtr  - (pointer to) total FEC virtual lanes per physical port
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on wrong port number or device
* @retval GT_HW_ERROR               - on hardware error
* @retval GT_NOT_SUPPORTED          - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
* @retval GT_BAD_PTR                - on NULL pointer.
*
*/
GT_STATUS appDemoDxChPortFecLaneIndexGet
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    OUT GT_U32                              *fecLaneIndexPtr,
    OUT GT_U32                              *fecLanesNumbersPtr
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U32                          portMacNum;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;
    MV_HWS_PORT_STANDARD            portMode;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    GT_U32                          fecLanesNumbers;

    /* check arguments */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(fecLaneIndexPtr);

    if(portNum >= CPSS_MAX_PORTS_NUM_CNS)
    {
        return GT_BAD_PARAM;
    }
    if ( CPSS_PP_FAMILY_DXCH_FALCON_E != PRV_CPSS_PP_MAC(devNum)->devFamily )
    {
        return GT_NOT_SUPPORTED;
    }
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    ifMode = PRV_CPSS_DXCH_PORT_IFMODE_MAC(devNum, portMacNum);
    speed  = PRV_CPSS_DXCH_PORT_SPEED_MAC(devNum, portMacNum);

    /* extract num of active lanes */
    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,ifMode, speed, &portMode);
    if ( GT_OK != rc )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    rc = hwsPortModeParamsGetToBuffer(devNum, 0, portMacNum, portMode, &curPortParams);
    if (GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    switch (curPortParams.numOfActLanes)
    {
        case 1:
            fecLanesNumbers = 2;
            break;
        case 2:
        case 4:
            fecLanesNumbers = 8;
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    *fecLaneIndexPtr = (portMacNum / curPortParams.numOfActLanes) * fecLanesNumbers;
    *fecLanesNumbersPtr = fecLanesNumbers;

    return rc;
}

/**
* @internal prvFecEventPortCounterReadAndClear function
* @endinternal
*
* @brief  Get the FEC counters per physical port and clear them after read.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] portNum                - physical port number
* @param[in] counterType            - FEC counter type
*
* @param[out] countersArrayPtr      - (pointer to) the counters per physical port
*
* @retval GT_OK                     - on success.
* @retval GT_BAD_PARAM              - on bad devNum or counter type.
*
* @note none
*
*/
static GT_STATUS prvFecEventPortCounterReadAndClear
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  APP_DEMO_FEC_COUNTER_TYPES_ENT  counterType,
    OUT GT_U32                          *countersArrayPtr
)
{
    GT_STATUS                       rc;
    GT_U32                          fecVirtualLaneIndex;
    GT_U32                          fecVirtualLanesNumbers;
    GT_U32                          *fecEventCountersPtr;
    GT_U32                          ii;

    /* DB counters not allocated yet, all counters are zero */
    if (uniFecEventCounters[devNum] == NULL)
    {
        return GT_OK;
    }

    switch (counterType)
    {
        case APP_DEMO_FEC_CE_COUNTER_TYPE_E:
            fecEventCountersPtr = uniFecEventCounters[devNum]->fecCeEventCounters;
            break;
        case APP_DEMO_FEC_NCE_COUNTER_TYPE_E:
            fecEventCountersPtr = uniFecEventCounters[devNum]->fecNceEventCounters;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = appDemoDxChPortFecLaneIndexGet(devNum, portNum, &fecVirtualLaneIndex, &fecVirtualLanesNumbers);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* Start from FEC lane index in DB */
    fecEventCountersPtr = &fecEventCountersPtr[fecVirtualLaneIndex / 2];

    /* Iterate lanes and read FEC counters from DB */
    for (ii = 0; ii < fecVirtualLanesNumbers / 2; ii++)
    {
        if (countersArrayPtr)
        {
            countersArrayPtr[ii] = fecEventCountersPtr[ii];
        }
        /* Clear FEC counters in DB */
        fecEventCountersPtr[ii] = 0;
    }

    return GT_OK;
}

/**
* @internal appDemoDxChFecEventPortCounterGet function
* @endinternal
*
* @brief  Get the FEC counters per physical port.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] portNum                - physical port number
* @param[in] counterType            - FEC counter type
*
* @param[out] countersArray         - array of the counters per physical port
*                                     Every element of array represents counter per one virtual lane for the physical port.
*                                     1 lane    - for ports 10G-R1 and 25G-R1
*                                     4 lanes   - for ports 40G-R4, 50G-R2 and 40G-R2
*
* @retval GT_OK                     - on success.
* @retval GT_BAD_PARAM              - on bad devNum or counter type.
*
* @note none
*
*/
GT_STATUS appDemoDxChFecEventPortCounterGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  APP_DEMO_FEC_COUNTER_TYPES_ENT  counterType,
    OUT GT_U32                          countersArray[MAX_VIRTUAL_LANES_CNS]
)
{
    GT_U32                          *countersArrayPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    countersArrayPtr = &countersArray[0];
    osMemSet(countersArrayPtr, 0, MAX_VIRTUAL_LANES_CNS * sizeof(GT_U32));

    return prvFecEventPortCounterReadAndClear(devNum, portNum, counterType, countersArrayPtr);
}

/**
* @internal appDemoDxChFecEventPortCounterClear function
* @endinternal
*
* @brief  Clear the FEC counters per physical port.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; xCat3; AC5; Lion2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] portNum                - physical port number
*
* @retval GT_OK                     - on success.
* @retval GT_BAD_PARAM              - on wrong port number or device.
* @retval GT_NOT_SUPPORTED          - the counter type is not supported.
*
* @note none
*
*/
GT_STATUS appDemoDxChFecEventPortCounterClear
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum
)
{
    GT_STATUS                       rc;
    APP_DEMO_FEC_COUNTER_TYPES_ENT  counterType;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    for (counterType = APP_DEMO_FEC_CE_COUNTER_TYPE_E; counterType < APP_DEMO_FEC_LAST_COUNTER_TYPE_E; counterType++)
    {
        rc = prvFecEventPortCounterReadAndClear(devNum, portNum, counterType, NULL);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

    }

    return GT_OK;
}

GT_STATUS debug_appDemoDxChFecEventPortCounterGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  APP_DEMO_FEC_COUNTER_TYPES_ENT  counterType,
    IN  GT_BOOL                         printHeader
)
{
    GT_STATUS                       rc;
    GT_U32                          countersArray[MAX_VIRTUAL_LANES_CNS] = {0,0,0,0};

    rc = appDemoDxChFecEventPortCounterGet(devNum, portNum, counterType, countersArray);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (printHeader == GT_TRUE)
    {
        osPrintf("FEC %s Events per Port    lane0   lane1   lane2   lane3\n", (counterType == APP_DEMO_FEC_CE_COUNTER_TYPE_E) ? "CE" : "NCE");
    }
        osPrintf("                    %3d      %d      %d      %d      %d  \n",
             portNum, countersArray[0], countersArray[1], countersArray[2], countersArray[3]);

    return GT_OK;
}
