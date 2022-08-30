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
* @file cpssPxNetIfRxHandling.c
*
* @brief PX : This file implements functions to allow application to receive RX
* packets from PP , and to free the resources when Rx packet's buffers not
* needed.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/networkIf/private/prvCpssPxNetworkIfLog.h>
#include <cpss/px/networkIf/cpssPxNetIf.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/cos/private/prvCpssPxCoS.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*******************************************************************************
* External usage environment parameters
*******************************************************************************/

/*******************************************************************************
* Internal usage environment parameters
*******************************************************************************/
static GT_BOOL debug_dumpFullPacket = GT_FALSE;

/**
* @internal pxNetIfSdmaRxBufFreeWithSize function
* @endinternal
*
* @brief   Frees a list of buffers, that where previously passed to the upper layer
*         in an Rx event. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number through which these buffers where
*                                      received.
* @param[in] rxQueue                  - The Rx queue number through which these buffers where
*                                      received.
* @param[in] rxBuffList[]             - List of Rx buffers to be freed.
* @param[in] rxBuffSizeList[]         - List of Rx Buffers sizes , to set to the free
*                                      descriptor
*                                      if this parameter is NULL --> we ignore it.
* @param[in] buffListLen              - Length of rxBufList.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS pxNetIfSdmaRxBufFreeWithSize
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32    rxQueue,
    IN GT_U8    *rxBuffList[],
    IN GT_U32   rxBuffSizeList[],
    IN GT_U32   buffListLen
)
{
    GT_STATUS                        rc;
    PRV_CPSS_RX_DESC_LIST_STC       *rxDescList;
    PRV_CPSS_RX_DESC_STC            *rxDesc;
    GT_U32                          tmpData;
    GT_UINTPTR                      phyAddr;
    GT_U32                          i;
    PRV_CPSS_SW_RX_DESC_STC         *first2Return;

    /* we will use the 'First active port group' , to represent the whole device.
       that way we allow application to give SDMA memory to single port group instead
       of split it between all active port groups
    */

    rxDescList = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.rxDescList[rxQueue]);

    first2Return = rxDescList->next2Return;
    for(i = 0; i < buffListLen; i++)
    {
        /* If rxDescList->rxDescList->next2Return equal to rxDescList->rxDescList->next2Receive
          and rxDescList->freeDescNum is bigger then 0 this means that all descriptors
          in the ring pointing to buffer, therefore we cannot add new buufer to the ring.
          This may indicate a problem in the application, that tries to free the buffers
          more than once before receiving it from PP.
         */
        if((rxDescList->freeDescNum > 0) &&
           (rxDescList->next2Return == rxDescList->next2Receive) )
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);

        rxDescList->freeDescNum++;
        rc = cpssOsVirt2Phy((GT_UINTPTR)(rxBuffList[i]),&phyAddr);
        if (rc != GT_OK)
        {
            return rc;
        }

        CPSS_OS_PHY_ADDR_IN_32_BITS_CHECK(phyAddr);

        tmpData = CPSS_32BIT_LE((GT_U32)phyAddr);
        rxDesc = rxDescList->next2Return->rxDesc;
        rxDesc->buffPointer = tmpData;

        if(i != 0)
        {/* first descriptor will be freed last */
            GT_SYNC;
            RX_DESC_RESET_MAC(devNum,rxDescList->next2Return->rxDesc);
            if(rxBuffSizeList)
            {
                RX_DESC_SET_BUFF_SIZE_FIELD_MAC(devNum,rxDescList->next2Return->rxDesc,rxBuffSizeList[i]);
            }
            GT_SYNC;
        }
        rxDescList->next2Return = rxDescList->next2Return->swNextDesc;
    }

    /* return first descriptor in chain to sdma ownership, it's done last to prevent multiple resource errors,
     * when jumbo packets arrive to cpu on high speed and cpu frees descriptors one by one in simple sequence
     * sdma immediately uses them and reaches cpu owned descriptor and again causes resource error
     */
    GT_SYNC;

    if(rxBuffSizeList)
    {
        RX_DESC_SET_BUFF_SIZE_FIELD_MAC(devNum, first2Return->rxDesc, rxBuffSizeList[0]);
    }
    RX_DESC_RESET_MAC(devNum, first2Return->rxDesc);
    GT_SYNC;

    return GT_OK;
}

/**
* @internal internal_cpssPxNetIfRxBufFree function
* @endinternal
*
* @brief   Frees a list of buffers, that where previously passed to the upper layer
*         in an Rx event.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number through which these buffers where
*                                      received.
* @param[in] rxQueue                  - The Rx queue number through which these buffers where
*                                      received (APPLICABLE RANGES: 0..7).
* @param[in] rxBuffList[]             - List of Rx buffers to be freed.
* @param[in] buffListLen              - Length of rxBufList.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. This function is not supported in CPSS_RX_BUFF_NO_ALLOC_E buffer
*       allocation method, GT_NOT_SUPPORTED returned.
*
*/
static GT_STATUS internal_cpssPxNetIfRxBufFree
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32    rxQueue,
    IN GT_U8    *rxBuffList[],
    IN GT_U32   buffListLen
)
{

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(rxBuffList);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(rxQueue);
    PRV_CPSS_PX_NETIF_INITIALIZED_CHECK_MAC(devNum);

    if(buffListLen == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "buffListLen must not be ZERO");
    }

    if(PRV_CPSS_PX_PP_MAC(devNum)->netIf.allocMethod == CPSS_RX_BUFF_NO_ALLOC_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "allocMethod must not be 'CPSS_RX_BUFF_NO_ALLOC_E'");
    }

    /* SDMA buffer free */
    return pxNetIfSdmaRxBufFreeWithSize(devNum,rxQueue,rxBuffList,NULL,buffListLen);
}

/**
* @internal cpssPxNetIfRxBufFree function
* @endinternal
*
* @brief   Frees a list of buffers, that where previously passed to the upper layer
*         in an Rx event.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number through which these buffers where
*                                      received.
* @param[in] rxQueue                  - The Rx queue number through which these buffers where
*                                      received (APPLICABLE RANGES: 0..7).
* @param[in] rxBuffList[]             - List of Rx buffers to be freed.
* @param[in] buffListLen              - Length of rxBufList.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. This function is not supported in CPSS_RX_BUFF_NO_ALLOC_E buffer
*       allocation method, GT_NOT_SUPPORTED returned.
*
*/
GT_STATUS cpssPxNetIfRxBufFree
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32    rxQueue,
    IN GT_U8    *rxBuffList[],
    IN GT_U32   buffListLen
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxNetIfRxBufFree);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, rxQueue, rxBuffList, buffListLen));

    rc = internal_cpssPxNetIfRxBufFree(devNum, rxQueue, rxBuffList, buffListLen);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, rxQueue, rxBuffList, buffListLen));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssPxNetIfRxBufFreeWithSize function
* @endinternal
*
* @brief   Frees a list of buffers, that where previously passed to the upper layer
*         in an Rx event.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number through which these buffers where
*                                      received.
* @param[in] rxQueue                  - The Rx queue number through which these buffers where
*                                      received (APPLICABLE RANGES: 0..7).
* @param[in] rxBuffList[]             - List of Rx buffers to be freed.
* @param[in] rxBuffSizeList[]         - List of Rx Buffers sizes.
* @param[in] buffListLen              - Length of rxBuffList and rxBuffSize lists.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported buffer allocation method
*
* @note 1. This function supported only in CPSS_RX_BUFF_NO_ALLOC_E buffer
*       allocation method, otherwise GT_NOT_SUPPORTED returned.
*       2. Buffer size must be multiple of 8 bytes and buffer address must be
*       128-byte aligned, otherwise GT_BAD_PARAM returned.
*
*/
static GT_STATUS internal_cpssPxNetIfRxBufFreeWithSize
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32    rxQueue,
    IN GT_U8    *rxBuffList[],
    IN GT_U32   rxBuffSizeList[],
    IN GT_U32   buffListLen
)
{
    GT_U32 i;            /* Iterator */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(rxBuffList);
    CPSS_NULL_PTR_CHECK_MAC(rxBuffSizeList);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(rxQueue);
    if(buffListLen == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "buffListLen must not be ZERO");
    }

    /*check that the DB of networkIf library was initialized*/
    PRV_CPSS_PX_NETIF_INITIALIZED_CHECK_MAC(devNum);

    if(PRV_CPSS_PX_PP_MAC(devNum)->netIf.allocMethod != CPSS_RX_BUFF_NO_ALLOC_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "allocMethod must not be 'CPSS_RX_BUFF_NO_ALLOC_E'");
    }

    for (i=0;i<buffListLen;i++)
    {
        /* check the buffer size*/
        if (rxBuffSizeList[i] % RX_BUFF_SIZE_MULT)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "rxBuffSizeList at index[%d] value[0x%x] must be aligned on [0x%x]",
                i,rxBuffSizeList[i],RX_BUFF_SIZE_MULT);
        }

        /* check the buffer address alignment*/
        if ((GT_UINTPTR)rxBuffList[i] % RX_BUFF_ALIGN)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "rxBuffSizeList ADDRESS at index[%d] value[0x%x] must be aligned on [0x%x]",
                i,(GT_UINTPTR)rxBuffList[i],RX_BUFF_ALIGN);
        }
    }

    return pxNetIfSdmaRxBufFreeWithSize(devNum,rxQueue,rxBuffList,rxBuffSizeList,buffListLen);
}

/**
* @internal cpssPxNetIfRxBufFreeWithSize function
* @endinternal
*
* @brief   Frees a list of buffers, that where previously passed to the upper layer
*         in an Rx event.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number through which these buffers where
*                                      received.
* @param[in] rxQueue                  - The Rx queue number through which these buffers where
*                                      received (APPLICABLE RANGES: 0..7).
* @param[in] rxBuffList[]             - List of Rx buffers to be freed.
* @param[in] rxBuffSizeList[]         - List of Rx Buffers sizes.
* @param[in] buffListLen              - Length of rxBuffList and rxBuffSize lists.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported buffer allocation method
*
* @note 1. This function supported only in CPSS_RX_BUFF_NO_ALLOC_E buffer
*       allocation method, otherwise GT_NOT_SUPPORTED returned.
*       2. Buffer size must be multiple of 8 bytes and buffer address must be
*       128-byte aligned, otherwise GT_BAD_PARAM returned.
*
*/
GT_STATUS cpssPxNetIfRxBufFreeWithSize
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32    rxQueue,
    IN GT_U8    *rxBuffList[],
    IN GT_U32   rxBuffSizeList[],
    IN GT_U32   buffListLen
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxNetIfRxBufFreeWithSize);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, rxQueue, rxBuffList, rxBuffSizeList, buffListLen));

    rc = internal_cpssPxNetIfRxBufFreeWithSize(devNum, rxQueue, rxBuffList, rxBuffSizeList, buffListLen);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, rxQueue, rxBuffList, rxBuffSizeList, buffListLen));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal debug_dumpFullPacket_print function
* @endinternal
*
* @brief   debug : This function print packet content.
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] rxQueue                  - The Rx queue number through which these buffers where
*                                      received.
*                                      numOfBuffPtr- Num of buffs in packetBuffsArrPtr.
*                                      lastBufferSize - last buffer size
*/
static void debug_dumpFullPacket_print
(
    IN GT_SW_DEV_NUM                        devNum,
    IN GT_U32                               rxQueue,
    OUT GT_U8                               *packetBuffsArrPtr[],
    OUT GT_U32                              buffLenArr[]
)
{
    GT_U32 iter;
    GT_U8*  bufferPtr = &packetBuffsArrPtr[0][0];
    GT_U32  length = buffLenArr[0];

    cpssOsPrintf("debug_dumpFullPacket_print: device[%d] queue[%d] packet to CPU with [%d] bytes \n",
        devNum,rxQueue,length);
    for(iter = 0; iter < length ; iter++)
    {
        if((iter & 0x0F) == 0)
        {
            cpssOsPrintf("0x%4.4x :", iter);
        }

        cpssOsPrintf(" %2.2x", bufferPtr[iter]);

        if((iter & 0x0F) == 0x0F)
        {
            cpssOsPrintf("\n");
        }
    }/*iter*/
    cpssOsPrintf("\n");

    return;
}


/**
* @internal pxNetIfSdmaRxResErrPacketHandle function
* @endinternal
*
* @brief   This function free the descriptors of packets from PP destined to the
*         CPU port with Rx Resource Error. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      Px1; Px2; Px3; xCat; xCat3; Lion; xCat2.
* @note   NOT APPLICABLE DEVICES:  Px1_Diamond.
*
* @param[in] devNum                   - Device number.
* @param[in] rxQueue                  - The queue from which this packet was received.
*
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
static GT_STATUS pxNetIfSdmaRxResErrPacketHandle
(
    IN GT_SW_DEV_NUM                        devNum,
    IN GT_U32                               rxQueue
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC      *regsAddr;/* alias to register address*/
    PRV_CPSS_SW_RX_DESC_STC   *swRxDesc;    /* A pointer to a Rx descriptor   */
    PRV_CPSS_SW_RX_DESC_STC   *swN2fRxDesc; /* A pointer to the Next 2        */
                                            /* free Rx descriptor.            */
    GT_U32                    rxSdmaRegMask;/* Rx SDMA register mask          */
    PRV_CPSS_RX_DESC_LIST_STC        *rxDescList;    /* alias to queue descriptor list   */
    GT_U32         portGroupId;/* port group Id for multi-port-groups device support */

    portGroupId = 0;

    regsAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);

    rxDescList = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.rxDescList[rxQueue]);

    if (GT_FALSE == rxDescList->forbidQEn)
    {
        /* Disable the corresponding Queue. */
        rxSdmaRegMask = (1 << (rxQueue + 8));
        rc = prvCpssHwPpPortGroupWriteRegister(CAST_SW_DEVNUM(devNum), portGroupId,regsAddr->MG.sdmaRegs.rxQCmdReg, rxSdmaRegMask);
        if(rc != GT_OK)
            return rc;

        /* Check that the SDMA is disabled. */
        /* wait for bit 0 to clear */
        rc = prvCpssPortGroupBusyWait(CAST_SW_DEVNUM(devNum),portGroupId,regsAddr->MG.sdmaRegs.rxQCmdReg,rxQueue,GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }

        rxDescList->forbidQEn = GT_TRUE;
    }

    /* set the SW descriptor shadow pointers */
    swRxDesc        = rxDescList->next2Receive;
    swN2fRxDesc     = rxDescList->next2Return;

    /* copy the Rx descriptor to SW shadow */
    swRxDesc->shadowRxDesc.word1 = CPSS_32BIT_LE(swRxDesc->rxDesc->word1);
    swRxDesc->shadowRxDesc.word2 = CPSS_32BIT_LE(swRxDesc->rxDesc->word2);

    /* Reset all descriptors with set resource error bit. */
    while (RX_DESC_GET_REC_ERR_BIT(&(swRxDesc->shadowRxDesc)) == 1 ||
           (RX_DESC_GET_OWN_BIT(&(swRxDesc->shadowRxDesc)) == RX_DESC_CPU_OWN &&
            RX_DESC_GET_FIRST_BIT(&(swRxDesc->shadowRxDesc)) != 1))
    {
        swN2fRxDesc->rxDesc->buffPointer = swRxDesc->rxDesc->buffPointer;

        /* copy buffer size */
        swN2fRxDesc->rxDesc->word2 = swRxDesc->rxDesc->word2;

        GT_SYNC;

        RX_DESC_RESET_MAC(devNum,swN2fRxDesc->rxDesc);

        GT_SYNC;

        swN2fRxDesc = swN2fRxDesc->swNextDesc;
        swRxDesc = swRxDesc->swNextDesc;
        swRxDesc->shadowRxDesc.word1 = CPSS_32BIT_LE(swRxDesc->rxDesc->word1);
        swRxDesc->shadowRxDesc.word2 = CPSS_32BIT_LE(swRxDesc->rxDesc->word2);
    }

    /* update the Rx desc list if error was detected */
    rxDescList->next2Receive = swRxDesc;
    rxDescList->next2Return  = swN2fRxDesc;

    /* Enable the Rx SDMA only if there are free descriptors in the Rx queue. */
    if (rxDescList->freeDescNum > 0)
    {
        rxSdmaRegMask = 1 << rxQueue;
        rc = prvCpssHwPpPortGroupWriteRegister(CAST_SW_DEVNUM(devNum),portGroupId,regsAddr->MG.sdmaRegs.rxQCmdReg, rxSdmaRegMask);
        if(rc != GT_OK)
            return rc;
    }
    rxDescList->forbidQEn = GT_FALSE;

    return GT_OK;
}

/**
* @internal internal_cpssPxNetIfSdmaRxPacketGet function
* @endinternal
*
* @brief   This function returns packets from PP destined to the CPU port.
*         -- SDMA relate.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] rxQueue                  - The queue from which this packet was received (APPLICABLE RANGES: 0..7).
* @param[in,out] numOfBuffPtr             Num of buffs in packetBuffsArrPtr.
* @param[in,out] numOfBuffPtr             - Num of used buffs in packetBuffsArrPtr.
*
* @param[out] packetBuffsArrPtr[]      - (pointer to)The received packet buffers list.
* @param[out] buffLenArr[]             - List of buffer lengths for packetBuffsArrPtr.
*                                      rxParamsPtr         - (Pointer to)information parameters of received packets
*
* @retval GT_NO_MORE               - no more packets on the device/queue
* @retval GT_OK                    - packet got with no error.
* @retval GT_FAIL                  - failed to get the packet
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong devNum,portNum,rxQueue.
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_DSA_PARSING_ERROR     - DSA tag parsing error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note It is recommended to call cpssPxNetIfRxBufFree for this queue
*       i.e. return the buffer to their original queue.
*
*/
static GT_STATUS internal_cpssPxNetIfSdmaRxPacketGet
(
    IN GT_SW_DEV_NUM                        devNum,
    IN GT_U32                               rxQueue,
    INOUT GT_U32                            *numOfBuffPtr,
    OUT GT_U8                               *packetBuffsArrPtr[],
    OUT GT_U32                              buffLenArr[]
)
{
    GT_U32              i;              /* iterator                         */
    PRV_CPSS_RX_DESC_LIST_STC   *rxDescList;
    PRV_CPSS_SW_RX_DESC_STC   *firstRxDesc;   /* First Rx descriptor        */
    PRV_CPSS_SW_RX_DESC_STC   *swRxDesc;      /* Rx descriptor              */
    GT_U32              descNum;        /* Num of desc this packet occupies */
    GT_U32              packetLen;      /* Length of packet in bytes        */
    GT_U32              temp;           /* temporary word                   */
    GT_UINTPTR          virtAddr;
    GT_U8               *tempBufferPtr;
    GT_U32              tempBufferSize = 0; /* Size of a single buffer.         */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(rxQueue);
    CPSS_NULL_PTR_CHECK_MAC(numOfBuffPtr);
    CPSS_NULL_PTR_CHECK_MAC(packetBuffsArrPtr);
    packetBuffsArrPtr[0] = NULL;/*avoid crash on LOG 'ALL' opened */
    CPSS_NULL_PTR_CHECK_MAC(buffLenArr);
    buffLenArr[0] =0;

    /*check that the DB of networkIf library was initialized*/
    PRV_CPSS_PX_NETIF_INITIALIZED_CHECK_MAC(devNum);

    firstRxDesc = NULL;
    rxDescList = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.rxDescList[rxQueue]);

    if(rxDescList->freeDescNum == 0)
    {
        *numOfBuffPtr = 0;
        return /* it's not error for log */ GT_NO_MORE;
    }

    swRxDesc = rxDescList->next2Receive;

    swRxDesc->shadowRxDesc.word1 = CPSS_32BIT_LE(swRxDesc->rxDesc->word1);
    swRxDesc->shadowRxDesc.word2 = CPSS_32BIT_LE(swRxDesc->rxDesc->word2);

    /* Handle resource error if happened. */
    if ( RX_DESC_GET_REC_ERR_BIT(&(swRxDesc->shadowRxDesc)) == 1 ||
         ( RX_DESC_GET_OWN_BIT(&(swRxDesc->shadowRxDesc)) == RX_DESC_CPU_OWN &&
           RX_DESC_GET_FIRST_BIT(&(swRxDesc->shadowRxDesc)) != 1))
    {
        /* handle SDMA error */
        pxNetIfSdmaRxResErrPacketHandle(devNum,rxQueue);

        /* update the shadow pointer , due to SDMA error */
        swRxDesc = rxDescList->next2Receive;

        swRxDesc->shadowRxDesc.word1 = CPSS_32BIT_LE(swRxDesc->rxDesc->word1);
        swRxDesc->shadowRxDesc.word2 = CPSS_32BIT_LE(swRxDesc->rxDesc->word2);
    }

    /* No more Packets to process, return */
    if(RX_DESC_GET_OWN_BIT(&(swRxDesc->shadowRxDesc)) != RX_DESC_CPU_OWN)
    {
        *numOfBuffPtr = 0;
        return /* it's not error for log */ GT_NO_MORE;
    }

    descNum     = 1;
    firstRxDesc = swRxDesc;

    /* Get the packet's descriptors.        */
    while(RX_DESC_GET_LAST_BIT(&(swRxDesc->shadowRxDesc)) == 0)
    {
        swRxDesc = swRxDesc->swNextDesc;
        swRxDesc->shadowRxDesc.word1 = CPSS_32BIT_LE(swRxDesc->rxDesc->word1);
        swRxDesc->shadowRxDesc.word2 = CPSS_32BIT_LE(swRxDesc->rxDesc->word2);
        descNum++;
    }

    rxDescList->next2Receive = swRxDesc->swNextDesc;
    rxDescList->freeDescNum -= descNum;

    swRxDesc = firstRxDesc;

    /* Validate that the given packet array length is big enough. */
    if (descNum > *numOfBuffPtr)
    {
        /* Drop the packet */
        /* Free all buffers - free buffers one by one */
        for(i = 0; i < descNum ; i++)
        {
            temp = swRxDesc->rxDesc->buffPointer;
            cpssOsPhy2Virt((GT_UINTPTR)(CPSS_32BIT_LE(temp)), &virtAddr);
            tempBufferPtr = (GT_U8*)virtAddr;
            tempBufferSize = RX_DESC_GET_BUFF_SIZE_FIELD_MAC(&(swRxDesc->shadowRxDesc));

            (void) pxNetIfSdmaRxBufFreeWithSize(devNum,rxQueue,&tempBufferPtr,&tempBufferSize,1);
            swRxDesc = swRxDesc->swNextDesc;
        }

        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, "descNum[0x%x] is more than[0x%x]",
            descNum , (*numOfBuffPtr));
    }

    packetLen   = RX_DESC_GET_BYTE_COUNT_FIELD(&(firstRxDesc->shadowRxDesc));

    /* build the array of buffers , and array of buffer length*/
    for(i = 0; i < descNum ; i++)
    {
        tempBufferSize = RX_DESC_GET_BUFF_SIZE_FIELD_MAC(&(swRxDesc->shadowRxDesc));
        if(packetLen > tempBufferSize)
        {
            buffLenArr[i] = tempBufferSize;
        }
        else
        {
            buffLenArr[i] = packetLen;
        }
        packetLen -= buffLenArr[i];

        temp = swRxDesc->rxDesc->buffPointer;
        cpssOsPhy2Virt((GT_UINTPTR)(CPSS_32BIT_LE(temp)), &virtAddr);
        packetBuffsArrPtr[i] = (GT_U8*)virtAddr;

        /* Invalidate data cache for cached buffer */
        if(GT_TRUE == PRV_CPSS_PX_PP_MAC(devNum)->netIf.buffersInCachedMem)
        {
            /* invalidate data cache */
            cpssExtDrvMgmtCacheInvalidate(CPSS_MGMT_DATA_CACHE_E, packetBuffsArrPtr[i], buffLenArr[i]);
        }

        swRxDesc  = swRxDesc->swNextDesc;
    }

    /* update the actual number of buffers in packet */
    *numOfBuffPtr = descNum;

    if(debug_dumpFullPacket)
    {
        debug_dumpFullPacket_print(devNum, rxQueue, packetBuffsArrPtr, buffLenArr);
    }

    return GT_OK;
}

/**
* @internal cpssPxNetIfSdmaRxPacketGet function
* @endinternal
*
* @brief   This function returns packets from PP destined to the CPU port.
*         -- SDMA relate.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] rxQueue                  - The queue from which this packet was received (APPLICABLE RANGES: 0..7).
* @param[in,out] numOfBuffPtr             Num of buffs in packetBuffsArrPtr.
* @param[in,out] numOfBuffPtr             - Num of used buffs in packetBuffsArrPtr.
*
* @param[out] packetBuffsArrPtr[]      - (pointer to)The received packet buffers list.
* @param[out] buffLenArr[]             - List of buffer lengths for packetBuffsArrPtr.
*                                      rxParamsPtr         - (Pointer to)information parameters of received packets
*
* @retval GT_NO_MORE               - no more packets on the device/queue
* @retval GT_OK                    - packet got with no error.
* @retval GT_FAIL                  - failed to get the packet
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong devNum,portNum,rxQueue.
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_DSA_PARSING_ERROR     - DSA tag parsing error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note It is recommended to call cpssPxNetIfRxBufFree for this queue
*       i.e. return the buffer to their original queue.
*
*/
GT_STATUS cpssPxNetIfSdmaRxPacketGet
(
    IN GT_SW_DEV_NUM                        devNum,
    IN GT_U32                               rxQueue,
    INOUT GT_U32                            *numOfBuffPtr,
    OUT GT_U8                               *packetBuffsArrPtr[],
    OUT GT_U32                              buffLenArr[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxNetIfSdmaRxPacketGet);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, rxQueue, numOfBuffPtr, packetBuffsArrPtr, buffLenArr));

    rc = internal_cpssPxNetIfSdmaRxPacketGet(devNum, rxQueue, numOfBuffPtr, packetBuffsArrPtr, buffLenArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, rxQueue, numOfBuffPtr, packetBuffsArrPtr, buffLenArr));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssPxNetIfSdmaRxCountersGet function
* @endinternal
*
* @brief   For each packet processor, get the Rx packet counters from its SDMA
*         packet interface. Return the aggregate counter values for the given
*         traffic class queue. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] rxQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
*
* @param[out] rxCountersPtr            - (pointer to) rx counters on this queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Counters are reset on every read.
*
*/
static GT_STATUS internal_cpssPxNetIfSdmaRxCountersGet
(
    IN    GT_SW_DEV_NUM devNum,
    IN    GT_U32        rxQueue,
    OUT   CPSS_PX_NET_SDMA_RX_COUNTERS_STC *rxCountersPtr
)
{
    GT_U32  regAddr;        /* The register address to write to.    */
    GT_U32  data;           /* Data read from the register.         */
    GT_STATUS rc = GT_OK;
    GT_U32         portGroupId;/* port group Id for multi-port-groups device support */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(rxCountersPtr);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(rxQueue);
    PRV_CPSS_PX_NETIF_INITIALIZED_CHECK_MAC(devNum);

    portGroupId = 0;

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.sdmaRegs.rxDmaPcktCnt[rxQueue];
    rc = prvCpssHwPpPortGroupReadRegister(CAST_SW_DEVNUM(devNum),portGroupId,regAddr,&data);
    if(rc != GT_OK)
        return rc;
    rxCountersPtr->rxInPkts = data;
    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.sdmaRegs.rxDmaByteCnt[rxQueue];
    rc = prvCpssHwPpPortGroupReadRegister(CAST_SW_DEVNUM(devNum),portGroupId,regAddr,&data);
    if(rc != GT_OK)
        return rc;

    rxCountersPtr->rxInOctets = data;

    return GT_OK;
}

/**
* @internal cpssPxNetIfSdmaRxCountersGet function
* @endinternal
*
* @brief   For each packet processor, get the Rx packet counters from its SDMA
*         packet interface. Return the aggregate counter values for the given
*         traffic class queue. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] rxQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
*
* @param[out] rxCountersPtr            - (pointer to) rx counters on this queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Counters are reset on every read.
*
*/
GT_STATUS cpssPxNetIfSdmaRxCountersGet
(
    IN    GT_SW_DEV_NUM devNum,
    IN    GT_U32        rxQueue,
    OUT   CPSS_PX_NET_SDMA_RX_COUNTERS_STC *rxCountersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxNetIfSdmaRxCountersGet);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, rxQueue, rxCountersPtr));

    rc = internal_cpssPxNetIfSdmaRxCountersGet(devNum, rxQueue, rxCountersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, rxQueue, rxCountersPtr));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssPxNetIfSdmaRxErrorCountGet function
* @endinternal
*
* @brief   Returns the total number of Rx resource errors that occurred on a given
*         Rx queue . -- SDMA relate
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] rxErrCountPtr            - (pointer to) The total number of Rx resource errors on
*                                      the device for all the queues.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counters returned by this function reflects the number of Rx errors
*       that occurred since the last call to this function.
*
*/
static GT_STATUS internal_cpssPxNetIfSdmaRxErrorCountGet
(
    IN GT_SW_DEV_NUM                            devNum,
    OUT CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC     *rxErrCountPtr
)
{
    GT_U32      regAddr;    /* The register address to read from.   */
    GT_U32      data;       /* Data read from register.             */
    GT_U32      ii;
    GT_STATUS   rc = GT_OK;
    GT_U32         portGroupId;/* port group Id for multi-port-groups device support */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(rxErrCountPtr);
    PRV_CPSS_PX_NETIF_INITIALIZED_CHECK_MAC(devNum);

    portGroupId = 0;

    /* get the counters of Queues 0..7 */
    for(ii = 0; ii < (CPSS_TC_RANGE_CNS); ii++)
    {
        regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.sdmaRegs.rxSdmaResourceErrorCountAndMode[ii];
        rc = prvCpssHwPpPortGroupReadRegister(CAST_SW_DEVNUM(devNum), portGroupId,
                                                 regAddr, &data);
        if(rc != GT_OK)
            return rc;
        rxErrCountPtr->counterArray[ii] = U32_GET_FIELD_MAC(data, 0, 8);
    }

    return GT_OK;
}

/**
* @internal cpssPxNetIfSdmaRxErrorCountGet function
* @endinternal
*
* @brief   Returns the total number of Rx resource errors that occurred on a given
*         Rx queue . -- SDMA relate
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] rxErrCountPtr            - (pointer to) The total number of Rx resource errors on
*                                      the device for all the queues.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counters returned by this function reflects the number of Rx errors
*       that occurred since the last call to this function.
*
*/
GT_STATUS cpssPxNetIfSdmaRxErrorCountGet
(
    IN GT_SW_DEV_NUM                            devNum,
    OUT CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC     *rxErrCountPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxNetIfSdmaRxErrorCountGet);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, rxErrCountPtr));

    rc = internal_cpssPxNetIfSdmaRxErrorCountGet(devNum, rxErrCountPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, rxErrCountPtr));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssPxNetIfSdmaRxQueueEnable function
* @endinternal
*
* @brief   Enable/Disable the specified traffic class queue for RX
*         on all packet processors in the system. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] rxQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] enable                   - GT_TRUE,  queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxNetIfSdmaRxQueueEnable
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_U32              rxQueue,
    IN  GT_BOOL             enable
)
{
    GT_U32  regAddr;        /* The register address to write to.    */
    GT_U32  data;
    GT_U32         portGroupId;/* port group Id for multi-port-groups device support */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(rxQueue);
    PRV_CPSS_PX_NETIF_INITIALIZED_CHECK_MAC(devNum);

    portGroupId = 0;

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.sdmaRegs.rxQCmdReg;

    if (enable == GT_TRUE)
    {
        data = (1 << rxQueue);
    }
    else
    {
        data = (1 << (8 + rxQueue));
    }

    return prvCpssHwPpPortGroupWriteRegister(CAST_SW_DEVNUM(devNum), portGroupId,regAddr, data);
}

/**
* @internal cpssPxNetIfSdmaRxQueueEnable function
* @endinternal
*
* @brief   Enable/Disable the specified traffic class queue for RX
*         on all packet processors in the system. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] rxQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] enable                   - GT_TRUE,  queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxNetIfSdmaRxQueueEnable
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_U32              rxQueue,
    IN  GT_BOOL             enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxNetIfSdmaRxQueueEnable);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, rxQueue, enable));

    rc = internal_cpssPxNetIfSdmaRxQueueEnable(devNum, rxQueue, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, rxQueue, enable));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssPxNetIfSdmaRxQueueEnableGet function
* @endinternal
*
* @brief   Get status of the specified traffic class queue for RX
*         packets in CPU. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] rxQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
*
* @param[out] enablePtr                - GT_TRUE, enable queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
static GT_STATUS internal_cpssPxNetIfSdmaRxQueueEnableGet
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_U32              rxQueue,
    OUT GT_BOOL             *enablePtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;     /* The register address to write to.    */
    GT_U32      hwData, hwValue;
    GT_U32      portGroupId; /* port group Id for multi-port-groups device support */


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(rxQueue);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    PRV_CPSS_PX_NETIF_INITIALIZED_CHECK_MAC(devNum);

    portGroupId = 0;

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.sdmaRegs.rxQCmdReg;

    rc = prvCpssHwPpPortGroupReadRegister(CAST_SW_DEVNUM(devNum), portGroupId,regAddr, &hwValue);
    if(GT_OK != rc)
    {
        return rc;
    }

    hwData = hwValue & 0xFFFF;

    /* Need check only RxENQ[7:0] bit to get Rx status.
      The RX SDMA is not active when RxENQ[7:0] is 0. */
    if((hwData >> rxQueue) & 1)
    {
        *enablePtr = GT_TRUE;
    }
    else
    {
        *enablePtr = GT_FALSE;
    }

    return rc;
}

/**
* @internal cpssPxNetIfSdmaRxQueueEnableGet function
* @endinternal
*
* @brief   Get status of the specified traffic class queue for RX
*         packets in CPU. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] rxQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
*
* @param[out] enablePtr                - GT_TRUE, enable queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssPxNetIfSdmaRxQueueEnableGet
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_U32              rxQueue,
    OUT GT_BOOL             *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxNetIfSdmaRxQueueEnableGet);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, rxQueue, enablePtr));

    rc = internal_cpssPxNetIfSdmaRxQueueEnableGet(devNum, rxQueue, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, rxQueue, enablePtr));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/* debug function to allow print (dump) of the Rx Packet include DSA tag. (before DSA tag is removed) */
void pxNetIfRxPacketParse_DebugDumpEnable
(
    IN GT_BOOL  enableDumpRxPacket
)
{
    debug_dumpFullPacket = enableDumpRxPacket;
}

