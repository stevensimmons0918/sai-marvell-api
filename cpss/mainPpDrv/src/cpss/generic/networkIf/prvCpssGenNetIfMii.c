/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssGenNetIfMii.c
*
* DESCRIPTION:
*       Implement CPSS generic MII/CPU Ethernet port network interface
*       management APIs
*
* FILE REVISION NUMBER:
*       $Revision: 21 $
*******************************************************************************/

/*******************************************************************************
* External usage environment parameters
*******************************************************************************/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfMii.h>
#include <cpss/generic/networkIf/private/prvCpssGenNetworkIfMii.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvPpIntGenMiiEmulateInterrupts.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#define HWINIT_GLOVAR(_var) \
    PRV_SHARED_GLOBAL_VAR_GET(commonMod.genericHwInitDir._var)

/****************************************************************************
 * private data                                                             *
 ****************************************************************************/
#ifdef SHARED_MEMORY
static GT_BOOL multiProcessAppDemo = GT_FALSE;
#endif

extern GT_STATUS extDrvEthPortRxInitWithoutInterruptConnection
(
    IN GT_U32           rxBufPoolSize,
    IN GT_U8_PTR        rxBufInfo_PTR,
    IN GT_U32           rxQNum,
    IN GT_U32           rxQbufPercentageArr[],
    IN GT_U32           rxBufSize,
    INOUT GT_U32        *numOfRxBufsPtr,
    IN GT_U32           headerOffset
);

/*******************************************************************************
* prvCpssGenNetIfMiiTxCtrlGet
*
* DESCRIPTION:
*       Return the ethernet port transmit control data
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_NETIF_MII_TX_CTRL_STC *prvCpssGenNetIfMiiTxCtrlGet(GT_VOID)
{
    if (HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssGenNetIfMiiCtrl.txValid))
        return &(HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssGenNetIfMiiTxCtrl));
    else
        return NULL;
}

/*******************************************************************************
* prvCpssGenNetIfMiiRxQCtrlGet
*
* DESCRIPTION:
*       Return the ethernet port recieve queue control data
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_NETIF_MII_RX_Q_CTRL_STC *prvCpssGenNetIfMiiRxQCtrlGet(GT_VOID)
{
    if (HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssGenNetIfMiiCtrl.rxValid))
        return &(HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssGenNetIfMiiRxQCtrl));
    else
        return NULL;
}

/**
* @internal prvCpssGenNetIfMiiDevNumGet function
* @endinternal
*
* @brief   Return the ethernet port device number
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
*                                       None.
*/
GT_U32 prvCpssGenNetIfMiiDevNumGet(GT_VOID)
{
    return HWINIT_GLOVAR(sysGenGlobalInfo.prvMiiDevNum);
}

/**
* @internal prvCpssGenNetIfMiiTxModeSet function
* @endinternal
*
* @brief   Set the ethernet port tx mode
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] txMode                   - tx mode
*                                       GT_OK if successful, or
*                                       GT_FAIL otherwise
*                                       GT_NOT_SUPPORTED
*/
GT_STATUS prvCpssGenNetIfMiiTxModeSet
(
    IN PRV_CPSS_GEN_NETIF_MII_TX_MODE_ENT txMode
)
{
    GT_STATUS rc;

    if (HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssGenNetIfMiiTxCtrl.txMode) == txMode)
        return GT_OK;

    rc = cpssExtDrvEthPortTxModeSet(txMode);
    if (rc != GT_OK)
    {
        return rc;
    }
    HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssGenNetIfMiiTxCtrl.txMode) = txMode;
    return GT_OK;
}


/**
* @internal prvCpssGenNetIfMiiRxPacketGet function
* @endinternal
*
* @brief   This function returns packets from PP destined to the CPU port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] queueIdx                 - The queue from which this packet was received.
* @param[in,out] numOfBuffPtr             Num of buffs in packetBuffsArrPtr.
* @param[in,out] numOfBuffPtr             - Num of used buffs in packetBuffsArrPtr.
*
* @param[out] packetBuffsArrPtr[]      - (pointer to)The received packet buffers list.
* @param[out] buffLenArr[]             - List of buffer lengths for packetBuffsArrPtr.
*
* @retval GT_NO_MORE               - no more packets on the device/queue
* @retval GT_OK                    - packet got with no error.
* @retval GT_FAIL                  - failed to get the packet
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong devNum,portNum,queueIdx.
* @retval GT_NOT_INITIALIZED       - the library was not initialized
*/
GT_STATUS prvCpssGenNetIfMiiRxPacketGet
(
    IN GT_U8                                queueIdx,
    INOUT GT_U32                            *numOfBuffPtr,
    OUT GT_U8                               *packetBuffsArrPtr[],
    OUT GT_U32                              buffLenArr[]
)
{
    GT_STATUS               rc;             /* Return Code                  */
    GT_U32                  i;              /* Iterator                     */
    PRV_CPSS_NETIF_MII_RX_Q_CTRL_STC *qCtrlPtr;      /* rx Q ctrl struct ptr         */
    PRV_CPSS_GEN_NETIF_RX_Q_MSG_STC  qMsg;           /* message from Queue           */
    PRV_CPSS_GEN_NETIF_SEG_STC   *currSegPtr;    /* segment ptr                  */
    PRV_CPSS_GEN_NETIF_SEG_STC   *tmpSegPtr;     /* temp segment ptr             */
    GT_U32 numMessages;
    GT_U32 messageSize = PRV_CPSS_NETIF_MII_RX_Q_MSG_LEN_CNS;

    qCtrlPtr = prvCpssGenNetIfMiiRxQCtrlGet();

    if (NULL == qCtrlPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(((GT_STATUS)GT_ERROR), LOG_ERROR_NO_MSG);
    }
    if (queueIdx >= PRV_CPSS_GEN_NETIF_MAX_RX_QUEUES_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* get waiting message from Queue */
    rc = cpssOsMsgQNumMsgs(qCtrlPtr->intQueId[queueIdx],&numMessages);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (numMessages == 0)
    {
        return /* it's not error for log */ GT_NO_MORE;
    }

    /* get a message from queue */
    rc = cpssOsMsgQRecv(qCtrlPtr->intQueId[queueIdx],(GT_PTR)&qMsg,
                       &messageSize,CPSS_OS_MSGQ_WAIT_FOREVER);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (messageSize != PRV_CPSS_NETIF_MII_RX_Q_MSG_LEN_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* get data from queue message and validate the first segment pointer */
    if((currSegPtr = (PRV_CPSS_GEN_NETIF_SEG_STC*)(qMsg.seg)) == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* copy all buffer pointers to buffer list */
    for(i = 0; currSegPtr != NULL; i++)
    {
        packetBuffsArrPtr[i]  = currSegPtr->seg_PTR;
        buffLenArr[i]   = currSegPtr->len;
        tmpSegPtr       = currSegPtr;
        currSegPtr      = currSegPtr->nextSeg_PTR;
        if((rc = prvCpssGenNetIfMiiPoolFreeBuf(PRV_CPSS_GEN_NETIF_MII_RX_SEG_POOL_E, tmpSegPtr)) != GT_OK)
            return rc;
    }

    /* update the actual number of buffers in packet */
    *numOfBuffPtr = i;

    return GT_OK;
}

/**
* @internal ethPrtInitRx function
* @endinternal
*
* @brief   This function initializes the Core Rx module, by allocating the list of
*         Rx buffers.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] rxQbufPercentageArr[CPSS_MAX_RX_QUEUE_CNS] - the buffer percentage dispersal for all queues
* @param[in] rxBufSize                - Size of a single Rx buffer.
* @param[in] headerOffset             - The application required header offset to be
*                                      kept before
* @param[in] rxBufBlockPtr            - (Pointer to) a block of memory to be used for
*                                      allocating Rx packet data buffers.
* @param[in] rxBufBlockSize           - The raw size in byte of rxDataBufBlock.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS ethPrtInitRx
(
    IN GT_U32    rxQbufPercentageArr[CPSS_MAX_RX_QUEUE_CNS],
    IN GT_U32    rxBufSize,
    IN GT_U32    headerOffset,
    IN GT_U8    *rxBufBlockPtr,
    IN GT_U32    rxBufBlockSize
)
{
    GT_U16              rxQueue;                /* Rx queue number          */
    char                semName[30];            /* semaphore name           */
    GT_U32              numOfRxBufs;            /* number of Rx buffers     */
    GT_STATUS           status;                 /* routine status           */
    GT_U32              segPoolSize;            /*  */
    GT_U8             * segPoolBlock_PTR;       /*  */
    CPSS_OS_MSGQ_ID     msgqId;

    /* validate the Rx buffer size */
    if (0 == rxBufSize)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    /* save space before user header for tagged packets indication */
    rxBufSize    += PRV_CPSS_NETIF_MII_TAGGED_PCKT_FLAG_LEN_CNS;
    headerOffset += PRV_CPSS_NETIF_MII_TAGGED_PCKT_FLAG_LEN_CNS;

    /* update the number of buffers in the pool */
    numOfRxBufs = rxBufBlockSize / rxBufSize;

    /* validate that we did not run out of buffers */
    if (0 == numOfRxBufs)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }


#ifdef SHARED_MEMORY
    if(multiProcessAppDemo == GT_FALSE)
    {
        /* init the external driver Rx */
        status = cpssExtDrvEthPortRxInit(rxBufBlockSize, rxBufBlockPtr, CPSS_MAX_RX_QUEUE_CNS,
         rxQbufPercentageArr, rxBufSize, &numOfRxBufs,
        headerOffset);
        /* temporary solution, similar to previous version */
        if ((GT_OK != status) && (GT_NOT_SUPPORTED != status))
        {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        /* init the external driver Rx */
        status = extDrvEthPortRxInitWithoutInterruptConnection(rxBufBlockSize, rxBufBlockPtr,
                 CPSS_MAX_RX_QUEUE_CNS, rxQbufPercentageArr, rxBufSize, &numOfRxBufs,
                 headerOffset);
        /* temporary solution, similar to previous version */
        if ((GT_OK != status) && (GT_NOT_SUPPORTED != status))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

    }
#else
    /* init the external driver Rx */
    status = cpssExtDrvEthPortRxInit(rxBufBlockSize, rxBufBlockPtr, CPSS_MAX_RX_QUEUE_CNS,
                                 rxQbufPercentageArr, rxBufSize, &numOfRxBufs,
                                 headerOffset);
    /* temporary solution, similar to previous version */
    if ((GT_OK != status) && (GT_NOT_SUPPORTED != status))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
#endif

    /* create segment pool that is used by the Rx ISR and Handler */
    segPoolSize         = numOfRxBufs * sizeof(PRV_CPSS_GEN_NETIF_SEG_STC);
    segPoolBlock_PTR    = cpssOsMalloc(segPoolSize);

    if (NULL == segPoolBlock_PTR)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    status = prvCpssGenNetIfMiiCreatePool(PRV_CPSS_GEN_NETIF_MII_RX_SEG_POOL_E, sizeof(PRV_CPSS_GEN_NETIF_SEG_STC),
                                     &numOfRxBufs, segPoolBlock_PTR, segPoolSize,
                                     NULL);
    if (GT_OK != status)
    {
        return status;
    }

    /* create the queues for the Rx packets (one per int queue) */
    for (rxQueue = 0; rxQueue < PRV_CPSS_GEN_NETIF_MAX_RX_QUEUES_NUM_CNS; rxQueue++)
    {
        cpssOsSprintf(semName,"rxQ-%d", rxQueue);

        status = cpssOsMsgQCreate(semName,numOfRxBufs,PRV_CPSS_NETIF_MII_RX_Q_MSG_LEN_CNS,&msgqId);
        if (status != GT_OK)
        {
            return status;
        }
        HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssGenNetIfMiiRxQCtrl.intQueId[rxQueue]) = msgqId;
    }

    /* set the header size */
    HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssGenNetIfMiiRxQCtrl.headerOffset) = headerOffset;
    HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssGenNetIfMiiRxQCtrl.auQueId)      = NULL;
    HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssGenNetIfMiiRxQCtrl.numOfRxBufs)  = numOfRxBufs;

    /* bind External Driver routine */
    HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssGenNetIfMiiRxQCtrl.packetFreeFunc_PTR) = cpssExtDrvEthRxPacketFree;

    HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssGenNetIfMiiCtrl.rxValid) = GT_TRUE;

    return GT_OK;
}

/**
* @internal ethPrtInitTx function
* @endinternal
*
* @brief   This function initializes the Core Tx module, by creating the
*         tx Header pool and queue.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] numOfTxDesc              - number of TX descriptors
* @param[in] txHdrBufBlockPtr         - ptr to the Tx header buffer block.
* @param[in] txHdrBufBlockSize        Tx header buffer block size.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS ethPrtInitTx
(
    IN GT_U8     devNum,
    IN GT_U32       numOfTxDesc,
    IN GT_U8        *txHdrBufBlockPtr,
    IN GT_U32       txHdrBufBlockSize
)
{
    GT_STATUS       status;
    GT_U32          numOfTxHdrBufs;

    /* tx buffers are taken from user allocated Tx decs Block. User MUST */
    /* allocate this area and set the tx desc block size accordingly.    */
    if (0 == txHdrBufBlockSize || NULL == txHdrBufBlockPtr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

    numOfTxHdrBufs = txHdrBufBlockSize / CPSS_GEN_NETIF_MII_TX_INTERNAL_BUFF_SIZE_CNS;

    /* create pool with caller`s allocated memory */
    status = prvCpssGenNetIfMiiCreatePool(PRV_CPSS_GEN_NETIF_MII_TX_POOL_E,
                                         CPSS_GEN_NETIF_MII_TX_INTERNAL_BUFF_SIZE_CNS,
                                         &numOfTxHdrBufs,
                                         (GT_U8*)txHdrBufBlockPtr,
                                         txHdrBufBlockSize,
                                         NULL);
    if (GT_OK != status)
    {
        return status;
    }

    /* init the external driver Tx */
    if(!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        status = cpssExtDrvEthPortTxInit(numOfTxDesc);
        if (GT_OK != status)
        {
            return status;
        }
    }

    /* init the external driver Tx mode to asynch */
    HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssGenNetIfMiiTxCtrl.txMode) = PRV_CPSS_GEN_NETIF_MII_TX_MODE_ASYNCH_E;

    if(!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum))
    {
        status = cpssExtDrvEthPortTxModeSet(CPSS_EXTDRV_ETH_TX_MODE_ASYNC_E);

        /* temporary solution, similar to previous version */
        if ((GT_OK != status) && (GT_NOT_SUPPORTED != status))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    HWINIT_GLOVAR(sysGenGlobalInfo.prvCpssGenNetIfMiiCtrl.txValid) = GT_TRUE;

    return GT_OK;
}

/**
* @internal prvCpssGenNetIfMiiInitNetIfDev function
* @endinternal
*
* @brief   Initialize the network interface structures, Rx buffers and Tx header
*         buffers (For a single device).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] numOfTxDesc              - number of requested TX descriptors.
* @param[in] txInternalBufBlockPtr    - (Pointer to) a block of host memory to be used
*                                      for internal TX buffers.
* @param[in] txInternalBufBlockSize   - The raw size in bytes of txInternalBufBlock memory.
* @param[in] bufferPercentageArr[CPSS_MAX_RX_QUEUE_CNS] - A table (entry per queue) that describes the buffer
*                                      dispersal among all Rx queues. (values 0..100)
* @param[in] rxBufSize                - The size of the Rx data buffer.
* @param[in] headerOffset             - packet header offset size
* @param[in] rxBufBlockPtr            - (Pointer to) a block of memory to be used for
*                                      allocating Rx packet data buffers.
* @param[in] rxBufBlockSize           - The raw size in byte of rxDataBufBlock.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssGenNetIfMiiInitNetIfDev
(
    IN GT_U8     devNum,
    IN GT_U32    numOfTxDesc,
    IN GT_U8    *txInternalBufBlockPtr,
    IN GT_U32    txInternalBufBlockSize,
    IN GT_U32    bufferPercentageArr[CPSS_MAX_RX_QUEUE_CNS],
    IN GT_U32    rxBufSize,
    IN GT_U32    headerOffset,
    IN GT_U8    *rxBufBlockPtr,
    IN GT_U32    rxBufBlockSize
)
{
    GT_STATUS           retVal;

    /* currently we support only one CPU MII port per system */
    if (HWINIT_GLOVAR(sysGenGlobalInfo.prvMiiDevNum) != PRV_CPSS_GEN_NETIF_MII_DEV_NOT_SET)
    {
        if (HWINIT_GLOVAR(sysGenGlobalInfo.prvMiiDevNum) == devNum)
        {
            return GT_OK;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    /* Init RX */
    retVal = ethPrtInitRx(bufferPercentageArr,rxBufSize,headerOffset,rxBufBlockPtr,rxBufBlockSize);

    if (GT_OK != retVal)
    {
        return retVal;
    }

    /* Init TX */
    retVal = ethPrtInitTx(devNum, numOfTxDesc,txInternalBufBlockPtr,txInternalBufBlockSize);

    if (GT_OK != retVal)
    {
        return retVal;
    }

    /* bind the RX callback routine */
    retVal = cpssExtDrvEthInputHookAdd(interruptMiiRxSR);
    if (GT_OK != retVal)
    {
        return retVal;
    }

    retVal = cpssExtDrvEthRawSocketRxHookAdd(rawSocketRx);
    if (GT_OK != retVal)
    {
        return retVal;
    }

    /* bind the TX complete callback routine */
    retVal = cpssExtDrvEthTxCompleteHookAdd(interruptMiiTxEndSR);
    if (GT_OK != retVal)
    {
        return retVal;
    }

    /* enable the ethernet port */
    retVal = cpssExtDrvEthPortEnable();
    if (GT_OK != retVal)
    {
        return retVal;
    }

    HWINIT_GLOVAR(sysGenGlobalInfo.prvMiiDevNum) = devNum;

    return GT_OK;
}

/**
* @internal prvCpssGenNetIfMiiRemove function
* @endinternal
*
* @brief   This function is called upon Hot removal of a device, inorder to release
*         all Network Interface related structures.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device that was removed.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenNetIfMiiRemove
(
    IN  GT_U8   devNum
)
{
    /* to avoid warning */
    devNum = devNum;

    /* disable the ethernet port */
    return cpssExtDrvEthPortDisable();
}

/**
* @internal prvCpssGenNetIfMiiRxBufFree function
* @endinternal
*
* @brief   Frees a list of buffers, that where previously passed to the upper layer
*         in an Rx event.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number through which these buffers where
*                                      received.
* @param[in] rxQueue                  - The Rx queue number through which these buffers where
*                                      received.
* @param[in] rxBuffList[]             - List of Rx buffers to be freed.
* @param[in] buffListLen              - Length of rxBufList.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssGenNetIfMiiRxBufFree
(
    IN GT_U8    devNum,
    IN GT_U8    rxQueue,
    IN GT_U8    *rxBuffList[],
    IN GT_U32   buffListLen
)
{
    PRV_CPSS_NETIF_MII_RX_Q_CTRL_STC *qCtrl_PTR;
    GT_U32                  firstBuffOffset;/* offset to the actual start of
                                                   the FIRST buffer */

    /* to avoid warning */
    devNum = devNum;

    /* get the Rx control block */
    qCtrl_PTR = prvCpssGenNetIfMiiRxQCtrlGet();

    if (NULL == qCtrl_PTR)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if(rxBuffList == NULL || buffListLen == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    /* get the offset before the start of the first buffer pointer to the
       actual start of the first buffer */
    firstBuffOffset = PRV_CPSS_NETIF_MII_RX_ORIG_PCKT_TAG_BYTES_GET(rxBuffList[0], qCtrl_PTR->headerOffset);

    /* update the buffer pointer to the actual start of the buffer */
    rxBuffList[0] -= (firstBuffOffset);

    qCtrl_PTR->packetFreeFunc_PTR(rxBuffList, buffListLen, rxQueue);

    return GT_OK;
}

/**
* @internal prvCpssGenNetIfMiiTxBufferQueueGet function
* @endinternal
*
* @brief   This routine returns the caller the TxEnd parameters for a transmitted
*         packet.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hndl                     - Handle got from gtEventBind.
*
* @param[out] devPtr                   - The device the packet was transmitted from.
* @param[out] cookiePtr                - The user cookie handed on the transmit request.
* @param[out] queuePtr                 - The queue from which this packet was transmitted
* @param[out] statusPtr                - GT_OK if packet transmission was successful, GT_FAIL on
*                                      packet reject.
*                                       GT_OK on success, or
*
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - the CPSS not handling the ISR for the device
*                                       so function must not be called
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       GT_FAIL otherwise.
*/
GT_STATUS prvCpssGenNetIfMiiTxBufferQueueGet
(
    IN  GT_UINTPTR          hndl,
    OUT GT_U8               *devPtr,
    OUT GT_PTR              *cookiePtr,
    OUT GT_U8               *queuePtr,
    OUT GT_STATUS           *statusPtr
)
{
    PRV_CPSS_DRV_EVENT_HNDL_STC *evHndlPtr;  /* The event handle pointer        */
    PRV_CPSS_TX_BUF_QUEUE_FIFO_STC          *fifoPtr;    /* The Handle Tx-End FIFO          */
    PRV_CPSS_SW_TX_FREE_DATA_STC    *fifoElemPtr;/* FIFO element                    */

    evHndlPtr = (PRV_CPSS_DRV_EVENT_HNDL_STC*)hndl;

    /* validate handle */
    if (NULL == evHndlPtr || NULL == evHndlPtr->extDataPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* set alias FIFO pointer */
    fifoPtr = (PRV_CPSS_TX_BUF_QUEUE_FIFO_STC*)evHndlPtr->extDataPtr;

    /* validate FIFO is not empty */
    if (NULL == fifoPtr->headPtr)
    {
        return /* it's not error for log */ GT_NO_MORE;
    }

    /* lock section from ISR preemption */
    PRV_CPSS_INT_SCAN_LOCK();

    /* set the packet header from FIFO first element */
    fifoElemPtr = fifoPtr->headPtr;

    /* remove the first element from FIFO */
    fifoPtr->headPtr = fifoElemPtr->nextPtr;

    if (NULL == fifoPtr->headPtr)
    {
        /* last element in FIFO, update last pointer */
        fifoPtr->tailPtr = NULL;
    }

    PRV_CPSS_INT_SCAN_UNLOCK();

    /* update other output parameters */
    *devPtr     = fifoElemPtr->devNum;
    *queuePtr   = fifoElemPtr->queueNum;
    *cookiePtr  = fifoElemPtr->userData;
    *statusPtr  = GT_OK;

    /* return the packet header buffer to pool */
    if (prvCpssGenNetIfMiiPoolFreeBuf(PRV_CPSS_GEN_NETIF_MII_TX_POOL_E,
                                 (GT_U8*)fifoElemPtr) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssGenNetIfMiiRxQueueCountersGet function
* @endinternal
*
* @brief   This routine returns the caller rx packets and octets counters
*         for given queue.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] rxQueue                  - rx queue.
*
* @param[out] rxInPktsPtr              - Packets received on this queue.
* @param[out] rxInOctetsPtr            - Octets received on this queue.
*                                       GT_OK on success, or
*
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*                                       GT_FAIL otherwise.
*/
GT_STATUS prvCpssGenNetIfMiiRxQueueCountersGet
(
    IN  GT_U32               rxQueue,
    OUT GT_U32               *rxInPktsPtr,
    OUT GT_U32               *rxInOctetsPtr
)
{
    PRV_CPSS_NETIF_MII_RX_Q_CTRL_STC *qCtrl_PTR;        /* queue control struct     */
    qCtrl_PTR = prvCpssGenNetIfMiiRxQCtrlGet();

    if (NULL == qCtrl_PTR)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    *rxInOctetsPtr = qCtrl_PTR->rxInOctetsCounter[rxQueue];
    *rxInPktsPtr = qCtrl_PTR->rxInPktsCounter[rxQueue];

    return GT_OK;
}

/**
* @internal prvCpssGenNetIfMiiRxQueueCountersClear function
* @endinternal
*
* @brief   This routine claers rx packets and octets counters for given queue.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] rxQueue                  - rx queue.
*                                       GT_OK on success, or
*
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*                                       GT_FAIL otherwise.
*/
GT_STATUS prvCpssGenNetIfMiiRxQueueCountersClear
(
    IN  GT_U32               rxQueue
)
{
    PRV_CPSS_NETIF_MII_RX_Q_CTRL_STC *qCtrl_PTR;        /* queue control struct     */
    qCtrl_PTR = prvCpssGenNetIfMiiRxQCtrlGet();

    if (NULL == qCtrl_PTR)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    qCtrl_PTR->rxInOctetsCounter[rxQueue] = 0;
    qCtrl_PTR->rxInPktsCounter[rxQueue] = 0;

    return GT_OK;
}

