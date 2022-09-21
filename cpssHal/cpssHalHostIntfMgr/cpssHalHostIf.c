/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalHostIf.c
*
* @brief Include  cpssHalHostIf for PCIe SDMA based control packet rx tx event handling
* control packet rate limiting and QOS profiling.Mapping application specific control packet
* id to PP CPU codes.
*
* @reference
*   1.userEventHandler.c --- appDemoEventRequestDrvnModeInit()
*   2.rxTxProcess.c --- rxTxEventRequestDrvnModeInit()
*   3.[FPAv1.2] fpaHalCommon.c --- fpaHalPktReceive,fpaHalPktTransmit,fpaHalShowRxPktReceive,fpaHalDumpSdmaRxCounters
*   4.[FPAv1.2] fpaMainLinux.c --- fpa_asic_listen, fpa_tap_listen
*   5.[CiscoHal] mrvl_ac3_cpu_rx_tx.c mrvl_ac3_test_tx_to_rx.c
* @version   1
********************************************************************************
*/

#include "cpssHalHostIf.h"
#include "xpsCommon.h"
#include <sys/select.h>
#include "cpssHalUtil.h"
#include "cpssHalCopp.h"
#include <gtOs/gtOsTimer.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


fd_set fds;
int gHostIfFds[1024] = {-1};
char gHostIfNames[1024][32];
uint64_t gNumHostIfFds = 0;
uint32_t eventHandlerTid; /* The task Tid */
extern bool gResetInProgress;

/* Local DB to convert HW device number to SW device number. */
/* Can be used also for the SW to HW direction for increased effectivity. */
GT_HW_DEV_NUM hwDevDb[XPS_NUM_OF_ASICS_IN_B2B];

/* Handle for handling Rx events through bind, wait and recieve
 * phase of HW events*/
static GT_UINTPTR cpssHalHostIfRxEventHandle = 0;

/*Enable flag for printing CPU received packet*/
static int cpuRxPacketShow = 0, cpuTxPacketShow = 0;

/*Number of packets received by cpu*/
static uint64_t cpuRxPacketCount = 0;

/*Number of packets sent by cpu*/
static uint64_t cpuTxPacketCount = 0;

static uint32_t rxPktEvnTid = 0;

static GT_UINTPTR rxEventHandlerHndl = 0;
/*Initialization of Rx event cause reception */
static CPSS_UNI_EV_CAUSE_ENT cpssHalHostIfRxEventsCauseArr[] =
{
    CPSS_PP_RX_BUFFER_QUEUE0_E,
    CPSS_PP_RX_BUFFER_QUEUE1_E,
    CPSS_PP_RX_BUFFER_QUEUE2_E,
    CPSS_PP_RX_BUFFER_QUEUE3_E,
    CPSS_PP_RX_BUFFER_QUEUE4_E,
    CPSS_PP_RX_BUFFER_QUEUE5_E,
    CPSS_PP_RX_BUFFER_QUEUE6_E,
    CPSS_PP_RX_BUFFER_QUEUE7_E,
    CPSS_PP_RX_ERR_QUEUE0_E,
    CPSS_PP_RX_ERR_QUEUE1_E,
    CPSS_PP_RX_ERR_QUEUE2_E,
    CPSS_PP_RX_ERR_QUEUE3_E,
    CPSS_PP_RX_ERR_QUEUE4_E,
    CPSS_PP_RX_ERR_QUEUE5_E,
    CPSS_PP_RX_ERR_QUEUE6_E,
    CPSS_PP_RX_ERR_QUEUE7_E
};

#define XPS_HOSTIF_RX_EVENTS_COUNT (sizeof(cpssHalHostIfRxEventsCauseArr)/sizeof(CPSS_UNI_EV_CAUSE_ENT))


#if 0 // UNIT_TEST
static void debug_send_packet();
#endif // UNIT_TEST

#if 0 // UNIT_TEST
static GT_STATUS cpuRxSet()
{

    CPSS_MAC_ENTRY_EXT_STC          macEntry;
    GT_STATUS rc;

    /* set Mac for traffic mirrored to CPU*/
    osMemSet(&macEntry, 0, sizeof(CPSS_MAC_ENTRY_EXT_STC));

    macEntry.daCommand = CPSS_MAC_TABLE_MIRROR_TO_CPU_E;

    macEntry.isStatic = GT_TRUE;

    macEntry.key.key.macVlan.macAddr.arEther[0] = 0;

    macEntry.key.key.macVlan.macAddr.arEther[1] = 0;

    macEntry.key.key.macVlan.macAddr.arEther[2] = 0;

    macEntry.key.key.macVlan.macAddr.arEther[3] = 0;

    macEntry.key.key.macVlan.macAddr.arEther[4] = 0;

    macEntry.key.key.macVlan.macAddr.arEther[5] = 0x77;

    macEntry.key.key.macVlan.vlanId = 1;

    macEntry.key.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;

    macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;

    macEntry.dstInterface.devPort.hwDevNum = 1;

    macEntry.dstInterface.devPort.portNum = 52;

    rc = cpssDxChBrgFdbMacEntrySet(0, &macEntry);
    rc = cpssDxChBrgFdbMacEntrySet(1, &macEntry);

    return rc;
}
#endif // UNIT_TEST

/**
 * \brief Handle rx queues based on PP events
 *
 * \param [in] devNum
 * \param [in] eventValue
 * \param [in] eventAdditionalData
 * \return XP_STATUS
*/
static GT_STATUS cpssHalHostIfRxQueueHandle
(
    uint32_t devNum,
    uint32_t eventValue,
    uint32_t eventAdditionalData,
    XPS_HOSTIF_IN_PACKET_BUFFER_STC *packetPointer
);


/**
 * \brief  process the event receive from CPU rx queue
 *
 * \param [in] eventType
 * \param [in] devNum
 * \param [in] queueIndex
 *
 * \return XP_STATUS
*/
static GT_STATUS cpssHalHostIfRxQueuePacketGetOperation
(
    XPS_HOSTIF_RX_EV_HANDLER_TYPE_ENT eventType,
    uint32_t devNum,
    uint32_t queueIndex,
    XPS_HOSTIF_IN_PACKET_BUFFER_STC *pktPtr
);


XP_STATUS cpssHalHostIfNetDevInit
(
    GT_U8 devId,
    RX_EV_PACKET_RECEIVE_CB_FUN    rxHandlerCb
)
{
    GT_STATUS rc;
    /*Initialize the netdevice utility*/
    rc = cpssHalUtilOsNetworkStackInit(devId, rxHandlerCb);
    if (rc != GT_OK)
    {
        return XP_ERR_INVALID_VALUE;
    }
    FD_ZERO(&fds);
    return XP_NO_ERR;
}

XP_STATUS cpssHalHostIfNetDevDeInit
(
    GT_U8 devId,
    RX_EV_PACKET_RECEIVE_CB_FUN    rxHandlerCb
)
{
    GT_STATUS rc;
    rc = cpssHalUtilOsNetworkStackDeInit(devId, rxHandlerCb);
    if (rc != GT_OK)
    {
        return XP_ERR_INVALID_VALUE;
    }
    return XP_NO_ERR;
}

XP_STATUS cpssHalHostIfNetDevCreate
(
    GT_U8         devId,
    GT_U8         *netDevName,
    GT_32         *fd
)
{
    GT_STATUS rc;
    rc = cpssHalUtilOsNetworkStackConnect(devId, netDevName, fd);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed open net Device %s fd %d\n", netDevName, fd);
        return XP_ERR_DEVICE_OPEN;
    }
    FD_SET((int)*fd, &fds);
    gHostIfFds[gNumHostIfFds] = (int)*fd;
    strncpy(gHostIfNames[gNumHostIfFds], (char *)netDevName,
            sizeof(gHostIfNames[gNumHostIfFds]));
    gNumHostIfFds++;
    return XP_NO_ERR;
}

XP_STATUS cpssHalHostIfNetDevDestroy
(
    GT_U8          devId,
    GT_32         fd
)
{
    GT_STATUS rc;
    rc = cpssHalUtilOsNetworkStackDisconnect(devId, fd);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed close net Device fd %d\n", fd);
        return XP_ERR_DEVICE_CLOSE;
    }
    FD_CLR((int)fd, &fds);
    return XP_NO_ERR;
}

XP_STATUS cpssHalHostIfNetDevWrite
(
    int32_t tnFd,
    GT_U8   *pktBuf,
    GT_U32  pktLen
)
{
    GT_STATUS rc;
    rc = cpssHalUtilOsNetworkStackDeviceWrite(tnFd, pktBuf, pktLen);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_WARNING,
              "Failed to write to net Device\n");
        return XP_ERR_REG_WRITE;
    }
    return XP_NO_ERR;
}

XP_STATUS cpssHalHostIfNetDevRead
(
    int32_t tnFd,
    GT_U8   *pktBuf,
    INOUT GT_U32  *pktLen
)
{
    GT_STATUS rc;
    rc = cpssHalUtilOsNetworkStackDeviceRead(tnFd, pktBuf, pktLen);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to read from net Device\n");
        return XP_ERR_REG_READ;
    }
    return XP_NO_ERR;
}

XP_STATUS cpssHalHostIfNetDevConfig
(
    const char *config,
    char* tnName
)
{
    GT_STATUS rc;
    rc = cpssHalUtilOsNetworkStackIfconfig(config, tnName);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to configure net Device\n");
        return XP_ERR_REG_READ;
    }
    return XP_NO_ERR;
}

XP_STATUS cpssHalHostIfNetDevGetMacAddr
(
    int32_t fd,
    uint8_t macAddr_t[]
)
{
    GT_STATUS rc;
    GT_ETHERADDR etherPtr;
    uint8_t i = 0;
    rc = cpssHalUtilOsNetworkStackMacGet(fd, &etherPtr);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get macaddr of net Device\n");
        return XP_ERR_REG_READ;
    }
    for (i =0; i < CPSS_HOSTIF_MAC_ADDR_SIZE; i++)
    {
        macAddr_t[i] = etherPtr.arEther[i];
    }
    return XP_NO_ERR;

}

XP_STATUS cpssHalHostIfSetPktCmdFdbEntry
(
    uint32_t  devId,
    uint16_t vlanId,
    uint8_t macAddr[],
    xpPktCmd_e cmd

)
{

    uint32_t index;
    xpsFdbEntry_t fdbEntry= {0};
    xpsFdbEntry_t         fdbEntryGet;
    XP_STATUS retVal=XP_NO_ERR;
    memset(&fdbEntryGet, 0, sizeof(fdbEntryGet));

    fdbEntry.vlanId = vlanId;
    fdbEntry.pktCmd = cmd;
    fdbEntry.isControl = GT_TRUE;
    fdbEntry.isStatic = GT_TRUE;
    memcpy(fdbEntry.macAddr, macAddr, sizeof(macAddr_t));

    retVal = xpsFdbFindEntry(devId, &fdbEntry, &index);
    if (retVal == XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
              "updating mac2me entry for vlan %d.\n", vlanId);
    }

    if (cmd == XP_PKTCMD_FWD)
    {
        /*for FWD action, we remove the control fdb rule,
        to save an entry in fdb table.
        if entry not found, it can be fresh entry with fwd action,
        which is also SAI default behaviour for ARP reply*/
        retVal = xpsFdbRemoveEntry(devId, &fdbEntry);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
                  "Failed to delete Fdb Entry for TRAP of ARP Reply\n");
        }
        return XP_NO_ERR;
    }

    memcpy(fdbEntryGet.macAddr, macAddr, sizeof(macAddr_t));
    fdbEntryGet.vlanId = vlanId;

    /*Update an local entry.*/
    retVal = (XP_STATUS) xpsFdbGetEntry(devId, &fdbEntryGet);
    if (retVal == XP_NO_ERR)
    {
        if (fdbEntryGet.isStatic)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "StaticMac" FMT_MAC "already present \n", PRI_MAC(macAddr));
            return XP_ERR_INVALID_VALUE;
        }

        retVal = xpsFdbRemoveEntry(devId, &fdbEntryGet);
        if (retVal != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
                  "Failed to delete Fdb Entry for TRAP \n");
        }
    }

    retVal = xpsFdbWriteEntry(devId, index, &fdbEntry);
    if (retVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to add Fdb Entry for TRAP \n");
        return retVal;
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
          "mac2me" FMT_MAC "entry for vlan %d success\n", PRI_MAC(macAddr), vlanId);
    return retVal;
}

XP_STATUS cpssHalHostIfRxTxInit
(
    xpsDevice_t devId
)
{
    XP_STATUS result = XP_NO_ERR;
    GT_U8 devNum = (GT_U8)devId;
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC entryInfo;
    uint32_t j =0 ;

    result = cpssHalHostIfEventRequest(true, devId);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalHostIfEventRequest failed");
        return result;
    }

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        rc = cpssDxChCfgHwDevNumGet(devNum, &hwDevDb[devNum]);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get HW dev for asic %d, "
                  "cpssDxChCfgHwDevNumGet rc %d\n",
                  devNum, rc);
            return XP_ERR_INIT;
        }
        for (j = 0; j < XPS_HOSTIF_RX_EVENTS_COUNT; ++j)
        {
            rc = cpssEventDeviceMaskSet(devNum,
                                        cpssHalHostIfRxEventsCauseArr[j],
                                        CPSS_EVENT_UNMASK_E);
            if (GT_OK == rc)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
                      "Mask set for event cause [%d]", cpssHalHostIfRxEventsCauseArr[j]);
            }
            else
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Mask set for event cause [%d] failed with error[%X]",
                      cpssHalHostIfRxEventsCauseArr[j], rc);
                return XP_ERR_NOT_INITED;
            }
        }

        /* In B2B, all traffic is sent to CPU SDMA connected to the first device */
        rc = cpssDxChNetIfCpuCodeDesignatedDeviceTableSet(devNum,
                                                          XPS_DESIGNATED_DEV_NUM_INDEX, hwDevDb[0]);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChNetIfCpuCodeDesignatedDeviceTableSet rc %d\n", rc);
            return XP_ERR_INIT;
        }

        cpssOsMemSet(&entryInfo, 0, sizeof(CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC));
        entryInfo.designatedDevNumIndex = XPS_DESIGNATED_DEV_NUM_INDEX;
        entryInfo.cpuRateLimitMode = CPSS_NET_CPU_CODE_RATE_LIMIT_AGGREGATE_E;

        /*Set Statistical rate limit index 0 to forward and 1 to drop all the packets sent to CPU */
        rc = cpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet(devId,
                                                               XPS_STAT_RATE_INDEX_FWD_ALL, 0xFFFFFFFF);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet rc %d\n", rc);
            return XP_ERR_INIT;
        }

        rc = cpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet(devId,
                                                               XPS_STAT_RATE_INDEX_DROP_ALL, 0);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChNetIfCpuCodeStatisticalRateLimitsTableSet rc %d\n", rc);
            return XP_ERR_INIT;
        }

        /* Initialize the CPU-CODE table */
        if ((rc = cpssDxChNetIfCpuCodeTableSet(devNum,
                                               CPSS_NET_ALL_CPU_OPCODES_E,
                                               &entryInfo)) != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChNetIfCpuCodeTableSet: ret[%d] asic[%d]", rc, devNum);
            return XP_ERR_INIT;
        }
    }
    rxEventHandlerLibInit();
    rxEventHandlerInitHandler(
        RX_EV_HANDLER_DEVNUM_ALL,
        RX_EV_HANDLER_QUEUE_ALL,
        RX_EV_HANDLER_TYPE_RX_E,
        &rxEventHandlerHndl);

    result = cpssHalHostIfDmaRxThread(CPSS_HOSTIF_DEFAULT_TIMEOUT_VAL);
    if (result != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, \
              "cpssHalHostIfDmaRxThread thread creation failed");
    }
    /* Sets a FDB entry to trap packet to CPU */
#if 0 // UNIT_TEST
    rc = cpuRxSet();
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, \
              "Debug Rx CPU trap entry set failed");
    }
#endif // UNIT_TEST

    return result;

}


static unsigned __TASKCONV cpssHalHostIfRxThread
(
    GT_VOID * param
)
{
    XP_STATUS status = XP_NO_ERR;
    XPS_HOSTIF_IN_PACKET_BUFFER_STC packetPointer;
    packetPointer.hostPktDataPtr = NULL;
    uint32_t timeout = CPSS_HOSTIF_DEFAULT_TIMEOUT_VAL;
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "DmaRxThread Created with timeOut %d \n", timeout);
    status = cpssHalHostIfPPtoCPUPacketReceive(timeout, &packetPointer);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Cpss HostIf Rx Path failed");
    }
    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT, "Exiting DmaRxThread");
    rxPktEvnTid = 0;
    pthread_exit(NULL);

    return status;
}

XP_STATUS cpssHalHostIfDmaRxThread
(
    uint32_t timeout
)
{
    XP_STATUS status = XP_NO_ERR;
    GT_STATUS rc     = GT_OK;
    rc = cpssOsTaskCreate("xpsDmaRx",
                          EV_HANDLER_MAX_PRIO,
                          _32KB,
                          cpssHalHostIfRxThread,
                          &timeout,
                          &eventHandlerTid);
    if (rc != GT_OK)
    {
        return XP_ERR_THREAD_CREATION;
    }
    rxPktEvnTid = eventHandlerTid;
    return status;
}

/**
 * \brief Initialize the CPU Rx cause event handling mechanism
 *
 * \param [in] isPollingEnabled
 * \param [in] deviceNumber
 *
 * \return XP_STATUS
*/
XP_STATUS cpssHalHostIfEventRequest
(
    bool      isPollingEnabled,
    GT_U8     deviceNumber
)
{
    XP_STATUS status = XP_NO_ERR;
    GT_STATUS rc     = GT_OK;
    uint32_t rxEventArrLen = XPS_HOSTIF_RX_EVENTS_COUNT;

    if (isPollingEnabled == true)
    {
        /* register callback handling RX events */
        rc = cpssEventBind(cpssHalHostIfRxEventsCauseArr, rxEventArrLen,
                           (GT_UINTPTR *)&cpssHalHostIfRxEventHandle);
        switch (rc)
        {
            case GT_FAIL:
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "General failure\n");
                break;
            case GT_BAD_PTR:
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "One of the parameters is NULL pointer\n");
                break;
            case GT_OUT_OF_CPU_MEM:
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to allocate CPU memory\n");
                break;
            case GT_FULL:
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "When trying to set the 'tx buffer queue unify event' \
                        (CPSS_PP_TX_BUFFER_QUEUE_E) with other events in the same handler\n");
                break;
            case GT_ALREADY_EXIST:
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "One of the unified events already bound to another \
                        handler (not applicable to CPSS_PP_TX_BUFFER_QUEUE_E)\n");
                break;
        }

        if (GT_OK != rc)
        {
            return XP_ERR_NOT_INITED;
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
                  "Events Bound Successfully with handle %d \n", cpssHalHostIfRxEventHandle);
        }
    }
    else
    {
        //Implement ISR based HW Rx event handling
    }
    return status;
}

XP_STATUS cpssHalHostIfPPtoCPUPacketReceive
(
    uint32_t timeout,
    XPS_HOSTIF_IN_PACKET_BUFFER_STC *packetPointer
)
{
    XP_STATUS status = XP_NO_ERR;
    GT_STATUS       rc;                  /* return code         */
    GT_U32          evBitmapArr[CPSS_UNI_EV_BITMAP_SIZE_CNS] = {0}; /* event bitmap array */
    GT_U8
    devNum;                                     /* device number       */
    GT_U32          evExtData;
    GT_32          queue;
    GT_U32_PTR      wait_ptr = NULL;

    /* array of flags per queue to verify that we
     * get GT_NO_MORE before doing 'cpssEventRecv' again */
    GT_BOOL queueReceiveEnable[CPSS_TC_RANGE_CNS] =
    {GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE};

    /* if timeout = 0 mode is Blocking; wait forever
       else mode is nonBlocking; wait timeout in milliseconds */
    wait_ptr = (timeout == 0) ? NULL : &timeout;
    while (1)
    {
        if (gResetInProgress)
        {
            return XP_NO_ERR;
        }

        rc = cpssEventSelect(cpssHalHostIfRxEventHandle, wait_ptr, evBitmapArr,
                             CPSS_UNI_EV_BITMAP_SIZE_CNS);
        if (rc != GT_OK)
        {
            /* no packet, normal return in polling mode */
            if ((rc == GT_EMPTY || rc == GT_TIMEOUT) && (timeout != 0))
            {
                /* non blocking mode */
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
                      " cpssEventSelect returned with status %d \n", rc);
                return XP_ERR_DMA0_TIMEOUT;
            }
            else if (rc == GT_ABORTED)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
                      " Destroy in progress Exit..%d \n", rc);
                return XP_NO_ERR;
            }

            else
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " cpssEventSelect returned with status %d \n", rc);
                return XP_ERR_NOT_FOUND;
            }
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                  " cpssEventSelect success with status %d", rc);
        }
        /* Check if new event is RX buffer event for all queues in RR order
         * start from CPSS_PP_RX_BUFFER_QUEUE0_E.
         * Verify to read all 8 queues for each selest API on same iteration
         * In SAI. 0 is lowest and 7 is highest priority.
         */
        bool keep_scanning;
        while (1)
        {
            for (queue = (CPSS_TC_RANGE_CNS-1); queue >= 0; queue--)
            {
                rc=cpssEventRecv(cpssHalHostIfRxEventHandle, CPSS_PP_RX_BUFFER_QUEUE0_E+queue,
                                 &evExtData, &devNum);
                if (rc == GT_OK)
                {
                    /*cpssEventRecv can be done again on this queue*/
                    queueReceiveEnable[queue] = GT_TRUE;
                }
                else if ((rc != GT_OK) && (rc != GT_NO_MORE))
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "\n cpssEventRecv(RX_BUFFER) failed with status %d on device %d queue %d\n"
                          , rc, devNum, queue);
                }
                else
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                          "cpssEventRecv(RX_BUFFER) success with status %d on device %d queue %d"
                          , rc, devNum, queue);
                }

                rc=cpssEventRecv(cpssHalHostIfRxEventHandle, CPSS_PP_RX_ERR_QUEUE0_E+queue,
                                 &evExtData, &devNum);
                if (rc == GT_OK)
                {
                    /*cpssEventRecv can be done again on this queue*/
                    queueReceiveEnable[queue] = GT_TRUE;
                }
                else if ((rc != GT_OK) && (rc != GT_NO_MORE))
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "\n cpssEventRecv(RX_ERR) failed with status %d on device %d queue %d\n"
                          , rc, devNum, queue);
                }
                else
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                          "cpssEventRecv(RX_ERR) success with status %d on device %d queue %d"
                          , rc, devNum, queue);
                }
            }
            keep_scanning = FALSE;
            for (queue = (CPSS_TC_RANGE_CNS-1); queue >= 0; queue--)
            {
                if (queueReceiveEnable[queue] == GT_TRUE)
                {
                    keep_scanning = TRUE;
                    uint32_t max_count = 50;
                    queueReceiveEnable[queue] = GT_FALSE;
                    while (max_count)
                    {
                        max_count --;
                        /* Treat packet Rx */
                        rc = cpssHalHostIfRxQueueHandle(devNum, (CPSS_PP_RX_BUFFER_QUEUE0_E+queue),
                                                        evExtData, packetPointer);
                        if (rc == GT_OK)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                                  " cpssHalHostIfRxQueueHandle returned success with status %d on queue %d\n", rc,
                                  queue);
                        }
                        /* no packet no current queue, continue to next queue */
                        if (rc == GT_NO_MORE)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                                  " cpssHalHostIfRxQueueHandle returned with status %d on queue %d\n", rc, queue);
                            /* can do 'cpssEventRecv' again on this queue */
                            break;
                        }
                        else if (rc != GT_OK)
                        {
                            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                  " cpssHalHostIfRxQueueHandle returned with status %d on queue %d\n", rc, queue);
                        }
                    }
                }
            }
            if (keep_scanning == FALSE)
            {
                break;
            }
        }
    }
    return status;
}

static GT_STATUS cpssHalHostIfRxQueueHandle
(
    uint32_t devNum,
    uint32_t eventValue,
    uint32_t eventAdditionalData,
    XPS_HOSTIF_IN_PACKET_BUFFER_STC *packetPointer
)
{
    XP_STATUS status = XP_NO_ERR;
    uint8_t     queue = 0;

    /* Event handler */
    switch (eventValue)
    {
        case CPSS_PP_RX_BUFFER_QUEUE0_E:
        case CPSS_PP_RX_BUFFER_QUEUE1_E:
        case CPSS_PP_RX_BUFFER_QUEUE2_E:
        case CPSS_PP_RX_BUFFER_QUEUE3_E:
        case CPSS_PP_RX_BUFFER_QUEUE4_E:
        case CPSS_PP_RX_BUFFER_QUEUE5_E:
        case CPSS_PP_RX_BUFFER_QUEUE6_E:
        case CPSS_PP_RX_BUFFER_QUEUE7_E:
            queue = (uint8_t)(eventValue - CPSS_PP_RX_BUFFER_QUEUE0_E);
            status = cpssHalHostIfRxQueuePacketGetOperation(XPS_HOSTIF_HANDLER_TYPE_RX_E,
                                                            devNum, queue, packetPointer);
            break;

        case CPSS_PP_RX_ERR_QUEUE0_E:
        case CPSS_PP_RX_ERR_QUEUE1_E:
        case CPSS_PP_RX_ERR_QUEUE2_E:
        case CPSS_PP_RX_ERR_QUEUE3_E:
        case CPSS_PP_RX_ERR_QUEUE4_E:
        case CPSS_PP_RX_ERR_QUEUE5_E:
        case CPSS_PP_RX_ERR_QUEUE6_E:
        case CPSS_PP_RX_ERR_QUEUE7_E:
            queue = (GT_U8)(eventValue - CPSS_PP_RX_ERR_QUEUE0_E);
            /* Handle Rx SDMA resource error */
            status = cpssHalHostIfRxQueuePacketGetOperation(
                         XPS_HOSTIF_HANDLER_TYPE_RX_ERR_E, devNum, queue, packetPointer);
            break;

        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "\n cpssHalHostIfRxQueueHandle returned due to invalid eventValue %d\n",
                  eventValue);
            break;
    }
    return status;
}


static GT_STATUS cpssHalHostIfRxQueuePacketGetOperation
(
    XPS_HOSTIF_RX_EV_HANDLER_TYPE_ENT eventType,
    uint32_t devNum,
    uint32_t queueIndex,
    XPS_HOSTIF_IN_PACKET_BUFFER_STC *pktPtr
)
{
    XP_STATUS status = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    uint32_t  numOfBuff = NUMBER_OF_RX_BUFFER;
    uint8_t*  packetBuffs[NUMBER_OF_RX_BUFFER]; //Pointer to received packet buffer
    uint32_t  buffLenArr[NUMBER_OF_RX_BUFFER]; //List of buffer lengths for packetBuffs Array
    CPSS_DXCH_NET_RX_PARAMS_STC         rxParams;
    if (eventType == XPS_HOSTIF_HANDLER_TYPE_RX_E)
    {
        rc = cpssDxChNetIfSdmaRxPacketGet(devNum, queueIndex, &numOfBuff,
                                          packetBuffs, buffLenArr, &rxParams);
        switch (rc)
        {
            case GT_FAIL:
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to get packet\n");
                break;
            case GT_BAD_PTR:
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "One of the parameters is NULL pointer\n");
                break;
            case GT_BAD_PARAM:
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "wrong devNum,portNum,queueIdx\n");
                break;
            case GT_NOT_INITIALIZED:
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "the library was not initialized\n");
                break;
            case GT_DSA_PARSING_ERROR:
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "DSA tag parsing error\n");
                break;
            case GT_NO_MORE:
                //LOGFN(xpLogModXps,XP_SUBMOD_MAIN,XP_LOG_DEBUG,"no more packets on the device/queue\n");
                break;
            case GT_BAD_SIZE:
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "GT_BAD_SIZE\n");
                break;
        }

        if (GT_OK != rc)
        {
            if (rc != GT_NO_MORE)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, \
                      "cpssDxChNetIfSdmaRxPacketGet failed with rc %d \n", rc);
            }
            return rc;
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Received packets from \
                    PP destined to the CPU port Successfully with rc %d \n",
                  rc);
            /* Remove 4 byte pattern (55 55 55 55) only from last buffer. */
            if (numOfBuff && (buffLenArr[numOfBuff-1] > MAC_FCS_LENGTH_CNS))
            {
                buffLenArr[numOfBuff-1] -= MAC_FCS_LENGTH_CNS;
            }

            if (cpuRxPacketShow != 0)
            {
                status = cpssHalHostIfRxPacketShow(devNum, queueIndex, numOfBuff,
                                                   packetBuffs, buffLenArr, &rxParams);
                if (status != XP_NO_ERR)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssHalHostIfRxPacketShow failed %d\n", status);
                }
                if (cpuRxPacketShow > 0)
                {
                    cpuRxPacketShow--;
                }
            }
            cpuRxPacketCount++;
            /*Packet receive initiated by xpsShell packetDrv receive command*/
            if (pktPtr->hostPktDataPtr != NULL)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Received packets  \
                        to be copied to user allocated backet buffer \n");

                pktPtr->reason = rxParams.dsaParam.dsaInfo.toCpu.cpuCode;
                pktPtr->ingressPortNum = rxParams.dsaParam.dsaInfo.toCpu.interface.portNum;
                pktPtr->vlanId = rxParams.dsaParam.commonParams.vid;
                memcpy(pktPtr->hostPktDataPtr, packetBuffs[0], buffLenArr[0]); /* packet data */
                pktPtr->hostPktDataSize = buffLenArr[0];
            }
            else
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, "Received packets  \
                        to be handled by callback function \n");
                /* send the buffer to the application netDev fd- appCallback */
                rxEventHandlerDoCallbacks(rxEventHandlerHndl,
                                          RX_EV_HANDLER_TYPE_RX_E,
                                          devNum,
                                          queueIndex,
                                          numOfBuff,
                                          packetBuffs,
                                          buffLenArr,
                                          &rxParams);

            }
        }


        /*Finally free the buffers allocated for the packet DMA*/
        rc = cpssDxChNetIfRxBufFree(devNum, queueIndex, packetBuffs, numOfBuff);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, \
                  "cpssHalHostIfRxQueuePacketGetOperation::failed to free list of buffers with rc %d \n",
                  rc);
            return rc;
        }
        else
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG, \
                  "cpssHalHostIfRxQueuePacketGetOperation::sucessfully freed list of buffers with rc %d \n",
                  rc);
        }
    }
    else if (eventType == XPS_HOSTIF_HANDLER_TYPE_RX_ERR_E)
    {
        //TO DO
    }
    else
    {
        //TO DO
    }
    return rc;
}


XP_STATUS cpssHalHostIfRxPacketShow
(
    uint8_t                         devNum,
    uint8_t                         queueIdx,
    uint32_t                        numOfBuff,
    uint8_t                         **packetBuffs,
    uint32_t                        *buffLenArr,
    CPSS_DXCH_NET_RX_PARAMS_STC     *rxParamsPtr
)
{
    uint32_t i;

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "(traffic)RX dev=%d queue=%d vid=%d \n",
          devNum, queueIdx,
          rxParamsPtr->dsaParam.commonParams.vid);
    cpssOsPrintf("(traffic)Rx Packet To CPU numBuf : %d \n", numOfBuff);

    if (rxParamsPtr->dsaParam.dsaType == CPSS_DXCH_NET_DSA_CMD_TO_CPU_E)
    {
        CPSS_DXCH_NET_DSA_TO_CPU_STC *toCpu = &(rxParamsPtr->dsaParam.dsaInfo.toCpu);

        cpssOsPrintf("hwDev=%d tagged=%d", toCpu->hwDevNum, toCpu->isTagged);
        if (toCpu->srcIsTrunk == GT_TRUE)
        {
            cpssOsPrintf(" srcTrunk=%d", toCpu->interface.srcTrunkId);
        }
        else
        {
            cpssOsPrintf(" portNum=%d ePort=%d queue=%d size=%d",
                         toCpu->interface.portNum,
                         toCpu->interface.ePort, queueIdx, *buffLenArr);
        }
        cpssOsPrintf(" cpuCode=%d \n", toCpu->cpuCode);
    }
    /* TODO: other DSA type */
    cpssOsPrintf("\n");
    uint32_t offset = 0;

    for (; numOfBuff; numOfBuff--, packetBuffs++, buffLenArr++)
    {
        cpssOsPrintf("(traffic)RX bufLenArr : %d \n", *buffLenArr);
        for (i = 0; i < *buffLenArr; i++)
        {
            if ((offset & 0x0F) == 0)
            {
                cpssOsPrintf("0x%4.4x :", offset);
            }
            cpssOsPrintf(" %2.2x", (*packetBuffs)[i]);

            if ((offset & 0x0F) == 0x0F)
            {
                cpssOsPrintf("\n");
            }

            offset++;

        }
    }
    if (offset != 0)
    {
        cpssOsPrintf("\n");
    }
    return XP_NO_ERR;
}

XP_STATUS cpssHalHostIfTxPacketShow
(
    uint8_t                         devNum,
    uint8_t                         queueIdx,
    uint32_t                        numOfBuff,
    uint8_t                         **packetBuffs,
    uint32_t                        *buffLenArr,
    CPSS_DXCH_NET_TX_PARAMS_STC     *txParamsPtr
)
{
    uint32_t bytesShown = 0;
    uint32_t i;

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
          "(traffic)TX dev=%d queue=%d vid=%d \n", devNum, queueIdx,
          txParamsPtr->dsaParam.commonParams.vid);
    cpssOsPrintf("(traffic)Tx Packet From CPU\n");

    if (txParamsPtr->dsaParam.dsaType == CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E)
    {
        CPSS_DXCH_NET_DSA_FROM_CPU_STC *fromCpu = &
                                                  (txParamsPtr->dsaParam.dsaInfo.fromCpu);
        if (fromCpu->dstInterface.type == CPSS_INTERFACE_PORT_E)
        {
            cpssOsPrintf("dev=%d port=%d", fromCpu->dstInterface.devPort.hwDevNum,
                         fromCpu->dstInterface.devPort.portNum);
        }
        else if (fromCpu->dstInterface.type == CPSS_INTERFACE_TRUNK_E)
        {
            cpssOsPrintf("trunk=%u", fromCpu->dstInterface.trunkId);
        }
        else if (fromCpu->dstInterface.type == CPSS_INTERFACE_VIDX_E)
        {
            cpssOsPrintf("vidx=%u", fromCpu->dstInterface.vidx);
        }
        else if (fromCpu->dstInterface.type == CPSS_INTERFACE_VID_E)
        {
            cpssOsPrintf("vlanid=%u", fromCpu->dstInterface.vlanId);
        }
        else if (fromCpu->dstInterface.type == CPSS_INTERFACE_DEVICE_E)
        {
            cpssOsPrintf("hwdev=%d", fromCpu->dstInterface.hwDevNum);
        }
        else if (fromCpu->dstInterface.type == CPSS_INTERFACE_FABRIC_VIDX_E)
        {
            cpssOsPrintf("fabricvidx=%u", fromCpu->dstInterface.fabricVidx);
        }
        else if (fromCpu->dstInterface.type == CPSS_INTERFACE_INDEX_E)
        {
            cpssOsPrintf("index=%u", fromCpu->dstInterface.index);
        }
        cpssOsPrintf(" srcHwDev=%d", fromCpu->srcHwDev);
    }
    cpssOsPrintf(" size=%u\n", *buffLenArr);

    for (; numOfBuff; numOfBuff--, packetBuffs++, buffLenArr++)
    {
        cpssOsPrintf("(traffic)TX");
        for (i = 0; i < *buffLenArr; i++, bytesShown++)
        {
            if ((bytesShown % PKTDUMP_BYTES_PER_LINE) == 0 && bytesShown != 0)
            {
                cpssOsPrintf("\n(traffic)TX");
            }
            cpssOsPrintf(" %02x", (*packetBuffs)[i]);
        }
    }
    if (bytesShown != 0)
    {
        cpssOsPrintf("\n");
    }
    return XP_NO_ERR;
}

XP_STATUS cpssHalHostIfCPUtoPPPacketForward
(
    uint32_t devId,
    XPS_HOSTIF_OUT_PACKET_BUFFER_STC *pktPtr
)
{
    CPSS_DXCH_NET_TX_PARAMS_STC  txParams;
    uint8_t          *buffList[NUMBER_OF_TX_BUFFER];
    uint32_t         buffLenList[NUMBER_OF_TX_BUFFER];
    GT_STATUS        status;
    uint32_t         devNum = devId;
    uint8_t   *static_buffer = (uint8_t *)0;
    uint8_t          *buffer;
    uint16_t         vid=0;
    GT_BOOL          isTagged = GT_FALSE;
    static uint32_t  cookie = 0;
    uint32_t         devPort = 0;
    int              packet_size = 0;
    CPSS_PP_FAMILY_TYPE_ENT    devType = cpssHalDevPPFamilyGet(devId);

    if ((pktPtr == NULL) || (pktPtr->pktDataPtr == NULL))
    {
        return XP_ERR_NULL_POINTER;
    }

    devPort = pktPtr->outPortNum;
    memset(&txParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));

    if (static_buffer == (uint8_t *)0)
    {
        static_buffer = (uint8_t *)osCacheDmaMalloc(MAX_TX_SDMA_BUFFER_SIZE);
        if (NULL == static_buffer)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " DMA allocate memory failed, tx failure");
            return XP_ERR_NULL_POINTER;
        }
    }

    buffer = (static_buffer + DMA_BUFFER_OFFSET);

    memcpy(buffer, pktPtr->pktDataPtr, pktPtr->pktDataSize);
    if (pktPtr->pktDataSize < MIN_DMA_BUFF_PKT_LEN)
    {
        packet_size = MIN_DMA_BUFF_PKT_LEN;
        memset(&buffer[pktPtr->pktDataSize], 0,
               (MIN_DMA_BUFF_PKT_LEN - pktPtr->pktDataSize));
    }
    else
    {
        packet_size = pktPtr->pktDataSize;
    }

    /* Handling for fetching Vlan Id from Ethernet header*/
    if ((pktPtr->pktDataPtr[12] == 0x81) && (pktPtr->pktDataPtr[13] == 0x00))
    {
        vid = (pktPtr->pktDataPtr[14] << 8) + (pktPtr->pktDataPtr[15]);
        vid &= 0xfff;
        isTagged = GT_TRUE;
    }

    txParams.packetIsTagged = isTagged;
    cookie++;
    txParams.cookie = &cookie;
    txParams.sdmaInfo.recalcCrc = GT_TRUE;
    txParams.sdmaInfo.txQueue = FROM_CPU_QUEUE_NUM;
    txParams.sdmaInfo.evReqHndl = 0;
    txParams.sdmaInfo.invokeTxBufferQueueEvent = GT_FALSE;
    if (devType == CPSS_PP_FAMILY_DXCH_FALCON_E ||
        devType == CPSS_PP_FAMILY_DXCH_AC5X_E ||
        devType == CPSS_PP_FAMILY_DXCH_ALDRIN2_E ||
        devType == CPSS_PP_FAMILY_TIGER_E)
    {
        txParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
    }
    else
    {
        txParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_2_WORD_TYPE_ENT;
    }
    txParams.dsaParam.commonParams.dropOnSource = GT_FALSE;
    txParams.dsaParam.commonParams.packetIsLooped = GT_FALSE;
    txParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FORWARD_E;
    //the source device number is configured to the other than that of the PP
    txParams.dsaParam.dsaInfo.forward.srcHwDev = CPSS_HOSTIF_IMAGINARY_SOURCE_DEV;
    //Traffic class - Low Priority
    txParams.dsaParam.dsaInfo.forward.qosProfileIndex = TC_PRIORITY_7;
    txParams.dsaParam.dsaInfo.forward.egrFilterRegistered = GT_FALSE;
    txParams.dsaParam.dsaInfo.forward.source.portNum = devPort;
    txParams.dsaParam.dsaInfo.forward.srcIsTagged = GT_TRUE;
    txParams.dsaParam.commonParams.cfiBit = 0;
    txParams.dsaParam.commonParams.vid  = 1;
    txParams.dsaParam.commonParams.vpt = 0;
    txParams.packetIsTagged = GT_TRUE;
    txParams.dsaParam.dsaInfo.forward.dstInterface.type = CPSS_INTERFACE_PORT_E;
    txParams.dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum = devNum;
    txParams.dsaParam.dsaInfo.forward.dstInterface.devPort.portNum = devPort;

    buffList[0] = buffer;
    buffLenList[0] = packet_size + MAC_FCS_LENGTH_CNS;

    if (cpuTxPacketShow != 0)
    {
        cpssHalHostIfTxPacketShow(devNum, txParams.sdmaInfo.txQueue, 1, buffList,
                                  buffLenList, &txParams);
        if (cpuTxPacketShow > 0)
        {
            cpuTxPacketShow--;
        }
    }

    /* Send single packet from CPU with synchronize access */
    status = cpssDxChNetIfSdmaSyncTxPacketSend(devNum, &txParams, buffList,
                                               buffLenList, NUMBER_OF_TX_BUFFER);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "%s:%d: \
                cpssDxChNetIfSdmaSyncTxPacketSend returned with status %d on packet size %d \n",
              __func__, __LINE__, status, pktPtr->pktDataSize);

        status = osCacheDmaFree(static_buffer);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " free DMAmemory failed with status %d", status);
        }

        return XP_ERR_INVALID_VALUE;
    }

    /* Handle free DMAmemory pointer in a better way */
    status = osCacheDmaFree(static_buffer);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " free DMAmemory failed with status %d", status);
    }

    return XP_NO_ERR;
}


XP_STATUS cpssHalHostIfCPUtoPPPacketTransmit
(
    uint32_t devId,
    XPS_HOSTIF_OUT_PACKET_BUFFER_STC *pktPtr
)
{
    CPSS_DXCH_NET_TX_PARAMS_STC  txParams;
    uint8_t          *buffList[NUMBER_OF_TX_BUFFER];
    uint32_t         buffLenList[NUMBER_OF_TX_BUFFER];
    GT_STATUS        status;
    uint32_t         devNum = devId;
    static uint8_t   *static_buffer=(uint8_t *)0;
    uint8_t          *buffer;
    uint16_t         vid=0;
    GT_BOOL          isTagged = GT_FALSE;
    static uint32_t  cookie = 0;
    uint32_t         devPort = 0;
    int              packet_size = 0;
    static int       allocWorkAroundOnce = 1;
    CPSS_PP_FAMILY_TYPE_ENT    devType = cpssHalDevPPFamilyGet(devId);

    if ((pktPtr == NULL) || (pktPtr->pktDataPtr == NULL))
    {
        return XP_ERR_NULL_POINTER;
    }
    devPort = pktPtr->outPortNum;
    memset(&txParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));


    if (static_buffer == (uint8_t *)0)
    {
        static_buffer = (uint8_t *)osCacheDmaMalloc(MAX_TX_SDMA_BUFFER_SIZE);
        if (devType == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {

            // FALCON BRINGUP WORKAROUND
            if (allocWorkAroundOnce && static_buffer)
            {
                osCacheDmaFree(static_buffer);
                static_buffer = (uint8_t *)osCacheDmaMalloc(MAX_TX_SDMA_BUFFER_SIZE);
                allocWorkAroundOnce = 0;
            }
        }

        if (! static_buffer)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " DMA Allocation failed size %x", MAX_TX_SDMA_BUFFER_SIZE);
            return XP_ERR_NULL_POINTER;
        }
    }

    buffer = (static_buffer + DMA_BUFFER_OFFSET);

    memcpy(buffer, pktPtr->pktDataPtr, pktPtr->pktDataSize);
    if (pktPtr->pktDataSize < MIN_DMA_BUFF_PKT_LEN)
    {
        packet_size = MIN_DMA_BUFF_PKT_LEN;
        memset(&buffer[pktPtr->pktDataSize], 0,
               (MIN_DMA_BUFF_PKT_LEN - pktPtr->pktDataSize));
    }
    else
    {
        packet_size = pktPtr->pktDataSize;
    }

    /* Handling for fetching Vlan Id from Ethernet header*/
    if ((pktPtr->pktDataPtr[12] == 0x81) && (pktPtr->pktDataPtr[13] == 0x00))
    {
        vid = (pktPtr->pktDataPtr[14] << 8) + (pktPtr->pktDataPtr[15]);
        vid &= 0xfff;
        isTagged = GT_TRUE;
    }

    txParams.packetIsTagged = isTagged;
    cookie++;
    txParams.cookie = &cookie;

    txParams.sdmaInfo.recalcCrc = GT_TRUE;
    txParams.sdmaInfo.txQueue = FROM_CPU_QUEUE_NUM;
    txParams.sdmaInfo.evReqHndl = 0;
    txParams.sdmaInfo.invokeTxBufferQueueEvent = GT_FALSE;
    if (devType == CPSS_PP_FAMILY_DXCH_FALCON_E ||
        devType == CPSS_PP_FAMILY_DXCH_AC5X_E ||
        devType == CPSS_PP_FAMILY_DXCH_ALDRIN2_E ||
        devType == CPSS_PP_FAMILY_TIGER_E)
    {
        txParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
    }
    else
    {
        txParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_2_WORD_TYPE_ENT;
    }
    txParams.dsaParam.commonParams.vpt = pktPtr->txData.tagVpt;
    txParams.dsaParam.commonParams.cfiBit = 0;
    txParams.dsaParam.commonParams.vid =
        vid; //pktPtr->txData.tagVlan can also provide vlan info
    txParams.dsaParam.commonParams.dropOnSource = GT_FALSE;
    txParams.dsaParam.commonParams.packetIsLooped = GT_FALSE;

    txParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;

    txParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = CPSS_INTERFACE_PORT_E;

    txParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = devNum;
    txParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = devPort;

    txParams.dsaParam.dsaInfo.fromCpu.tc = pktPtr->txData.egressQueueNum;
    txParams.dsaParam.dsaInfo.fromCpu.dp = CPSS_DP_GREEN_E;
    txParams.dsaParam.dsaInfo.fromCpu.egrFilterEn = GT_FALSE;
    txParams.dsaParam.dsaInfo.fromCpu.cascadeControl = GT_FALSE;
    txParams.dsaParam.dsaInfo.fromCpu.egrFilterRegistered = GT_FALSE;
    txParams.dsaParam.dsaInfo.fromCpu.srcId = 0;
    txParams.dsaParam.dsaInfo.fromCpu.srcHwDev = devNum;
    txParams.dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.dstIsTagged = isTagged;
    txParams.dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.mailBoxToNeighborCPU =
        GT_FALSE;

    txParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = GT_TRUE;
    txParams.dsaParam.dsaInfo.fromCpu.dstEport = devPort;
    txParams.dsaParam.dsaInfo.fromCpu.tag0TpidIndex = 0;

    buffList[0] = buffer;
    buffLenList[0] = packet_size + MAC_FCS_LENGTH_CNS;

    if (cpuTxPacketShow != 0)
    {
        cpssHalHostIfTxPacketShow(devNum, txParams.sdmaInfo.txQueue, 1, buffList,
                                  buffLenList, &txParams);
        if (cpuTxPacketShow > 0)
        {
            cpuTxPacketShow--;
        }
    }

    /* Send single packet from CPU with synchronize access */
    status = cpssDxChNetIfSdmaSyncTxPacketSend(devNum, &txParams, buffList,
                                               buffLenList, NUMBER_OF_TX_BUFFER);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "%s:%d: \
                cpssDxChNetIfSdmaSyncTxPacketSend returned with status %d on packet size %d on port %d\n",
              __func__, __LINE__, status, pktPtr->pktDataSize, devPort);

        // TODO_RPR Need to check in M0, the impact of Free
#if 0
        status = osCacheDmaFree(static_buffer);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " free DMAmemory failed with status %d", status);
        }
#endif
        return XP_ERR_INVALID_VALUE;
    }

    cpuTxPacketCount++;

    /* Reuse the allocated ptr to optimize the CPU usage.
       Enabling free may have impact on CPU utilization.*/
#if 0
    /* Handle free DMAmemory pointer in a better way */
    status = osCacheDmaFree(static_buffer);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " free DMAmemory failed with status %d", status);
    }
#endif

    return XP_NO_ERR;
}

XP_STATUS cpssHalHostIfCPUtoPPPacketTransmitWithDSA
(
    uint32_t devId,
    XPS_HOSTIF_OUT_PACKET_BUFFER_STC *pktPtr,
    xpDsaHeader *xpDsaHeader_stc
)
{
    CPSS_DXCH_NET_TX_PARAMS_STC  txParams;
    uint8_t          *buffList[NUMBER_OF_TX_BUFFER];
    uint32_t         buffLenList[NUMBER_OF_TX_BUFFER];
    GT_STATUS        status;
    uint16_t         vid=0;
    uint8_t          devNum = 0;
    uint8_t          *tx_sdma_buffer=(uint8_t *)0;
    uint8_t          *buffer;
    static uint32_t  cookie = 0;
    uint32_t         devPort = 0;
    int              packet_size = 0;
    GT_BOOL isTagged = GT_FALSE;
    GT_BOOL isTrgPhyPortValid = GT_TRUE;
    CPSS_PP_FAMILY_TYPE_ENT    devType = cpssHalDevPPFamilyGet(devId);

    if ((pktPtr == NULL) || (pktPtr->pktDataPtr == NULL))
    {
        return XP_ERR_NULL_POINTER;
    }
    devNum = xpsGlobalIdToDevId(devId, pktPtr->outPortNum);
    devPort = xpsGlobalPortToPortnum(devId, pktPtr->outPortNum);
    if ((devNum == 0xff) || (devPort == 0xffff))
    {
        return XP_ERR_INVALID_ARG;
    }

    if (devPort == 0)
    {
        isTrgPhyPortValid = GT_FALSE;
    }
    memset(&txParams, 0, sizeof(CPSS_DXCH_NET_TX_PARAMS_STC));

    if (tx_sdma_buffer == (uint8_t *)0)
    {
        tx_sdma_buffer = (uint8_t *)osCacheDmaMalloc(MAX_TX_SDMA_BUFFER_SIZE);
        if (tx_sdma_buffer == NULL)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " DMA allocate memory failed, tx failure");
            return XP_ERR_NULL_POINTER;
        }
    }

    buffer = (tx_sdma_buffer + DMA_BUFFER_OFFSET);

    memcpy(buffer, pktPtr->pktDataPtr, pktPtr->pktDataSize);
    if (pktPtr->pktDataSize < MIN_DMA_BUFF_PKT_LEN)
    {
        packet_size = MIN_DMA_BUFF_PKT_LEN;
        memset(&buffer[pktPtr->pktDataSize], 0,
               (MIN_DMA_BUFF_PKT_LEN - pktPtr->pktDataSize));
    }
    else
    {
        packet_size = pktPtr->pktDataSize;
    }

    if (xpDsaHeader_stc->vid != 0)
    {
        isTagged = GT_TRUE;
    }

    /* Handling for fetching Vlan Id from Ethernet header*/
    if ((pktPtr->pktDataPtr[12] == 0x81) && (pktPtr->pktDataPtr[13] == 0x00))
    {
        vid = (pktPtr->pktDataPtr[14] << 8) + (pktPtr->pktDataPtr[15]);
        vid &= 0xfff;
        isTagged = GT_TRUE;
    }
    txParams.packetIsTagged = isTagged;
    cookie++;
    txParams.cookie = &cookie;

    txParams.sdmaInfo.recalcCrc = (GT_BOOL)xpDsaHeader_stc->recalcCrc;
    txParams.sdmaInfo.txQueue = pktPtr->txData.egressQueueNum;
    txParams.sdmaInfo.evReqHndl = 0;
    txParams.sdmaInfo.invokeTxBufferQueueEvent = GT_FALSE;

    if (devType == CPSS_PP_FAMILY_DXCH_FALCON_E ||
        devType == CPSS_PP_FAMILY_DXCH_AC5X_E)
    {
        txParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
    }
    else
    {
        txParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_2_WORD_TYPE_ENT;
    }
    txParams.dsaParam.commonParams.vpt = xpDsaHeader_stc->vpt;
    txParams.dsaParam.commonParams.cfiBit = xpDsaHeader_stc->cfiBit;
    txParams.dsaParam.commonParams.vid = vid;
    txParams.dsaParam.commonParams.dropOnSource = (GT_BOOL)
                                                  xpDsaHeader_stc->dropOnSouce;
    txParams.dsaParam.commonParams.packetIsLooped = (GT_BOOL)
                                                    xpDsaHeader_stc->packetIsLooped;

    txParams.dsaParam.dsaType = CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E;

    txParams.dsaParam.dsaInfo.fromCpu.dstInterface.type =
        xpDsaHeader_stc->dstInterfaceType;

    txParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = devNum;
    txParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = devPort;

    txParams.dsaParam.dsaInfo.fromCpu.tc = xpDsaHeader_stc->tc;
    txParams.dsaParam.dsaInfo.fromCpu.dp = xpDsaHeader_stc->dp;
    txParams.dsaParam.dsaInfo.fromCpu.egrFilterEn = (GT_BOOL)
                                                    xpDsaHeader_stc->egrFilterEn;
    txParams.dsaParam.dsaInfo.fromCpu.cascadeControl = (GT_BOOL)
                                                       xpDsaHeader_stc->cascadeControl;
    txParams.dsaParam.dsaInfo.fromCpu.egrFilterRegistered =
        (GT_BOOL)xpDsaHeader_stc->egrFilterRegistered;
    txParams.dsaParam.dsaInfo.fromCpu.srcId = xpDsaHeader_stc->srcId;
    txParams.dsaParam.dsaInfo.fromCpu.srcHwDev = devId;
    txParams.dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.dstIsTagged = isTagged;
    txParams.dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.mailBoxToNeighborCPU =
        (GT_BOOL)xpDsaHeader_stc->mailBoxToNeighborCPU;

    txParams.dsaParam.dsaInfo.fromCpu.isTrgPhyPortValid = isTrgPhyPortValid;
    txParams.dsaParam.dsaInfo.fromCpu.dstEport = devPort;
    txParams.dsaParam.dsaInfo.fromCpu.tag0TpidIndex = (GT_BOOL)
                                                      xpDsaHeader_stc->tag0TpidIndex;

    buffList[0] = buffer;
    buffLenList[0] = packet_size + MAC_FCS_LENGTH_CNS;

    if (cpuTxPacketShow != 0)
    {
        cpssHalHostIfTxPacketShow(devNum, txParams.sdmaInfo.txQueue, 1, buffList,
                                  buffLenList, &txParams);
        if (cpuTxPacketShow > 0)
        {
            cpuTxPacketShow--;
        }
    }

    /* Send single packet from CPU with synchronize access */
    status = cpssDxChNetIfSdmaSyncTxPacketSend(devNum, &txParams, buffList,
                                               buffLenList, NUMBER_OF_TX_BUFFER);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "%s:%d: \
                cpssDxChNetIfSdmaSyncTxPacketSend returned with status %d on packet size %d \n",
              __func__, __LINE__, status, pktPtr->pktDataSize);
        return XP_ERR_INVALID_VALUE;
    }

    cpuTxPacketCount++;

    /* Handle free DMAmemory pointer in a better way */
    status = osCacheDmaFree(tx_sdma_buffer);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " free DMAmemory failed with status %d", status);
    }

    return XP_NO_ERR;
}


XP_STATUS cpssHalHostIfDisplayCpuCodeTable
(
    uint8_t deviceNumber
)
{
    GT_STATUS rc = GT_OK;
    uint32_t index;
    uint32_t hwData; /* data in the HW format */

    cpssOsPrintf("\n CPU Code Entries in HW\n\n");
    cpssOsPrintf("Index\t TC \tDP Truncated StatRateLimit TrgDevIndex RateLimiter RateLimitMode  \n");
    cpssOsPrintf("-----\t----\t-- --------- ------------- ----------- ----------- -------------  \n");

    for (index = 0; index<256; index++)
    {
        /* Read from table */
        rc = prvCpssDxChReadTableEntry(deviceNumber,
                                       CPSS_DXCH_TABLE_CPU_CODE_E,
                                       index,/* DSA CPU code */
                                       &hwData);
        if (GT_OK!=rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "[%s:%d] - prvCpssDxChReadeTableEntry: "
                  "asic_num[%d], cpuCode EntryIndex[%d], rc[%d]",
                  __func__, __LINE__, deviceNumber, index, rc);
            return XP_ERR_INVALID_VALUE;
        }
        cpssOsPrintf("%d\t", index);
        cpssOsPrintf("%d\t", (GT_U8)U32_GET_FIELD_MAC(hwData, 0, 3)); /* tc */
        cpssOsPrintf("%d\t", (GT_U8)U32_GET_FIELD_MAC(hwData, 4, 1)); /* dp */
        cpssOsPrintf("%d\t", (GT_U8)U32_GET_FIELD_MAC(hwData, 4, 1)); /* truncated */
        cpssOsPrintf("%d\t\t", (GT_U8)U32_GET_FIELD_MAC(hwData, 6,
                                                        5)); /* cpu Code Stat Rate Limit Index */
        cpssOsPrintf("%d\t", (GT_U8)U32_GET_FIELD_MAC(hwData, 11,
                                                      3)); /* designated Dev Num Index */
        cpssOsPrintf("%d\t\t", (GT_U8)U32_GET_FIELD_MAC(hwData, 14,
                                                        5)); /* cpu Code Rate Limiter Index */
        cpssOsPrintf("%d\t", (GT_U8)U32_GET_FIELD_MAC(hwData, 19,
                                                      1)); /* cpu Code Rate Limiter Mode */
        cpssOsPrintf("\n");
    }
    return XP_NO_ERR;
}

XP_STATUS cpssHalHostIfDisplayRateLimiterTable
(
    uint8_t deviceNumber
)
{
    GT_STATUS rc = GT_OK;
    uint32_t    index;
    uint32_t    windowSize;
    uint32_t    pktLimit;
    uint32_t  windowResolution;

    cpssOsPrintf("\n CPU Code Rate Limit Table \n");
    cpssOsPrintf("Index  window size  Paket Limit  \n");
    cpssOsPrintf("-----  -----------  -----------  \n");

    for (index = 1; index<32; index++)
    {
        rc = cpssDxChNetIfCpuCodeRateLimiterTableGet(deviceNumber,
                                                     index,
                                                     &windowSize,
                                                     &pktLimit);
        if (GT_OK != rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "[%s:%d] - cpssDxChNetIfCpuCodeRateLimiterTableGet: "
                  "asic_num[%d], EntryIndex[%d], rc[%d]",
                  __func__, __LINE__, deviceNumber, index, rc);
            return XP_ERR_INVALID_VALUE;
        }

        cpssOsPrintf("%d\t  ", index);
        cpssOsPrintf("%d\t\t ", windowSize);
        cpssOsPrintf("%d\t",  pktLimit);

        cpssOsPrintf("\n");
    }

    rc = cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet(deviceNumber,
                                                            &windowResolution);
    if (GT_OK!=rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "[%s:%d] - cpssDxChNetIfCpuCodeRateLimiterWindowResolutionGet: "
              "asic_num[%d], rc[%d]",
              __func__, __LINE__, deviceNumber, rc);
        return XP_ERR_INVALID_VALUE;
    }

    cpssOsPrintf("TO CPU Rate Limiter Window Resolution = %d \n",
                 windowResolution);

    return XP_NO_ERR;
}

XP_STATUS cpssHalHostIfSdmaRxCountersDisplay
(
    uint8_t deviceNumber
)
{
    XP_STATUS status = XP_NO_ERR;
    GT_STATUS rc = GT_OK;
    GT_U8                               queueIdx;
    CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC  rxCounters;
    CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC rxErrCount;
    cpssOsPrintf("Hardware Counters\n");
    for (queueIdx = 0; queueIdx < CPSS_TC_RANGE_CNS; queueIdx++)
    {
        rc = cpssDxChNetIfSdmaRxCountersGet(deviceNumber, queueIdx, &rxCounters);
        if (GT_OK!=rc)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "[%s:%d] - cpssDxChNetIfSdmaRxCountersGet: "
                  "asic_num[%d], queueIdx[%d], rc[%d]",
                  __func__, __LINE__, deviceNumber, queueIdx, rc);
            status = XP_ERR_INVALID_VALUE;
        }
        else
        {
            cpssOsPrintf("Queue[%d]: Rx packets[%d], Rx Bytes[%d]\n",
                         queueIdx, rxCounters.rxInPkts, rxCounters.rxInOctets);
        }

    }

    rc = cpssDxChNetIfSdmaRxErrorCountGet(deviceNumber, &rxErrCount);
    if (GT_OK!=rc)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "[%s:%d] - cpssDxChNetIfSdmaRxErrorCountGet: "
              "asic_num[%d], rc[%d]",
              __func__, __LINE__, deviceNumber, rc);
        status = XP_ERR_INVALID_VALUE;
    }
    else
    {
        for (queueIdx=0; queueIdx< CPSS_TC_RANGE_CNS; queueIdx++)
        {
            cpssOsPrintf("Queue[%d]: Rx Error packets[%d]\n",
                         queueIdx, rxErrCount.counterArray[queueIdx]);
        }
    }

    cpssOsPrintf("\n");
    cpssOsPrintf("Software Counters\n");
    cpssOsPrintf("Rx Counters = %" PRIu64 ", Tx Counters = %" PRIu64 "\n",
                 cpuRxPacketCount, cpuTxPacketCount);
    return status;
}

XP_STATUS cpssHalHostIfCPURxPacketDumpEnable(GT_BOOL enable)
{
    /* allow to 'see' the packets that CPU receives (with the DSA) */
    dxChNetIfRxPacketParse_DebugDumpEnable(enable);
    return XP_NO_ERR;
}

XP_STATUS cpssHalHostIfCPURxPacketPrintEnable(int printPktCnt)
{
    cpuRxPacketShow = printPktCnt;
    return XP_NO_ERR;
}

XP_STATUS cpssHalHostIfCPUTxPacketPrintEnable(int printPktCnt)
{
    cpuTxPacketShow = printPktCnt;
    return XP_NO_ERR;
}

#if 0 // UNIT_TEST

void cpssHalDebugNetdevLib(int32_t *netFd)
{
    char netdev_name[]="eth10";
    int32_t fd = -1;
    GT_U8 buf[64] =
        "000000000077000000000011ffff000102030405060708090a0b0c0d0e0f1011";
    GT_U8 readBuf[64] = "0";
    GT_U32 pktlen = 64;
    XP_STATUS status = XP_NO_ERR;
    status = cpssHalHostIfNetDevCreate(0, (GT_U8*)netdev_name, &fd);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalHostIfNetDevCreate failed %d\n", status);
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalHostIfNetDevCreate success fd %d\n", fd);

    }
    status = cpssHalHostIfNetDevWrite(fd, buf, 64);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalHostIfNetDevWrite failed %d\n", status);
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "cpssHalHostIfNetDevWrite success fd %d\n", fd);
    }
    status = cpssHalHostIfNetDevRead(fd, readBuf, &pktlen);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalHostIfNetDevRead failed %d\n", status);
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "cpssHalHostIfNetDevRead success fd %d %s\n", fd, readBuf);
    }

    status = cpssHalHostIfNetDevWrite(fd, buf, 64);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalHostIfNetDevWrite failed %d\n", status);
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "cpssHalHostIfNetDevWrite success fd %d\n", fd);
    }
    status = cpssHalHostIfNetDevRead(fd, readBuf, &pktlen);
    if (status != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalHostIfNetDevRead failed %d\n", status);
    }
    else
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_TRACE,
              "cpssHalHostIfNetDevRead success fd %d %s\n", fd, readBuf);
    }

    print_packet(readBuf, 64);
    *netFd = fd;
    return;
}


/*Debug code to send packet*/
static void debug_send_packet()
{
    uint8_t       *buffer;
    uint32_t      buffer_size = 64;
    unsigned char*      packetBuffs[1];
    uint32_t      buffLen[1];
    XP_STATUS    status;
    XPS_HOSTIF_OUT_PACKET_BUFFER_STC pkt = {0};

    /* init configuration */

    buffer = cpssOsMalloc(buffer_size*sizeof(uint8_t));
    if (buffer == NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssOsCacheDmaMalloc: failed with bad pointer\n");
        return;
    }

    buffer[0] = 0;
    buffer[1] = 0;
    buffer[2] = 0;
    buffer[3] = 0x00;
    buffer[4] = 0x00;
    buffer[5] = 0x77;
    buffer[6] = 0;
    buffer[7] = 0;
    buffer[8] = 0;
    buffer[9] = 0;
    buffer[10]= 0;
    buffer[11]= 0x11;
    buffer[12]= 0x81;
    buffer[13]= 0;
    buffer[14]= 0;
    buffer[15]= 1;
    buffer[16]= 0x66;
    buffer[17]= 0x66;

    *packetBuffs = (unsigned char*)buffer;
    *buffLen = (uint32_t)buffer_size;

    print_packet(buffer, 64);
    /* print the transmit packet */
    pkt.pktDataPtr = buffer;
    pkt.pktDataSize = buffer_size;
    pkt.outPortNum = 49;
    pkt.txData.egressQueueNum = 0;
    pkt.txData.tagVpt = 0;
    pkt.txData.tagVlan = 0;
    int i =0;
    for (i = 0; i <1; i++)
    {
        status = cpssHalHostIfCPUtoPPPacketTransmit(0, &pkt);
        if (status != XP_NO_ERR)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, " Tx test failed \n");
            cpssOsFree(buffer);
            return ;
        }
    }
    cpssOsFree(buffer);
    return ;
}
#endif

XP_STATUS cpssHalHostIfDmaRxThreadDeInit(uint32_t devId)
{
    XP_STATUS status = XP_NO_ERR;
    GT_STATUS rc     = GT_OK;
    uint8_t cpssDevNum;
    uint32_t j = 0;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        for (j=0; j < CPSS_MAX_RX_QUEUE_CNS; j++)
        {
            rc = cpssDxChNetIfSdmaRxQueueEnable(cpssDevNum, j, GT_FALSE);
            if (GT_OK != rc)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "SdmaRxQueue Failed [%d] , Queue :%d \n", rc, j);
                return XP_ERR_NOT_INITED;
            }
        }
    }



    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        for (j = 0; j < XPS_HOSTIF_RX_EVENTS_COUNT; ++j)
        {
            rc = cpssEventDeviceMaskSet(cpssDevNum,
                                        cpssHalHostIfRxEventsCauseArr[j],
                                        CPSS_EVENT_MASK_E);
            if (GT_OK != rc)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Mask set for event cause [%d] \n", cpssHalHostIfRxEventsCauseArr[j]);
                return XP_ERR_INIT;
            }
        }
    }

    /* Post this clean-up any call to cpssEvn from Rx thread will fail with GT_ABORT,
       for which thread exit will happen. */
    cpssEventUnBind(cpssHalHostIfRxEventsCauseArr, XPS_HOSTIF_RX_EVENTS_COUNT);
    cpssEventDestroy(cpssHalHostIfRxEventHandle);

    return status;
}

