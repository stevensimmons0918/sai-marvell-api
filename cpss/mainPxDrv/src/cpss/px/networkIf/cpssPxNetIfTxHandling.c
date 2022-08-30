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
* @file cpssPxNetIfTxHandling.c
*
* @brief This file implements all needed functions for sending packets of upper
* layer to the cpu port of the PX device (SDMA-Tx Queues).
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

/*******************************************************************************
* cpssPxNetIfSdmaSyncTxPacketSend
*
*       This function transmit packet from the CPU to the PP via the SDMA interface.
*       Prepared descriptors to the PP's SDMA TX queues.
*       After the transmit ends, all transmitted packets descriptors are freed.
*        -- SDMA relate.
*       function activates Tx SDMA , function wait for PP to finish processing
*       the buffer(s).
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum       - Device number.
*       pcktParamsPtr- The internal packet params to be set into the packet
*                      descriptors.
*       buffList     - The packet data buffers list.
*       buffLenList  - A list of the buffers len in buffList.
*       numOfBufs    - Length of buffList.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK          - on success, or
*       GT_NO_RESOURCE - if there is not enough free elements in the fifo
*                        associated with the Event Request Handle.
*       GT_EMPTY       - if there are not enough descriptors to do the sending.
*       GT_HW_ERROR    - when after transmission last descriptor own bit wasn't
*                        changed for long time.
*       GT_BAD_PARAM   - the data buffer is longer than allowed.
*                        Buffer data can occupied up to the maximum number of descriptors defined.
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_INITIALIZED       - the library was not initialized
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_FAIL otherwise.
*
* COMMENTS:
*
*******************************************************************************/
static GT_STATUS pxNetIfSdmaTxPacketSend
(
    IN GT_SW_DEV_NUM                    devNum,
    IN CPSS_PX_NET_TX_PARAMS_STC      *pcktParamsPtr,
    IN GT_U8                            *buffList[],
    IN GT_U32                            buffLenList[],
    IN GT_U32                           *numOfBufsPtr,
    INOUT PRV_CPSS_TX_DESC_LIST_STC     *txDescListPtr,
    OUT GT_BOOL                         *txDescListUpdatedPtr
)
{
    GT_STATUS rc;
    GT_U32    txQueue;              /* The tx queue to send this packet to. */
    PRV_CPSS_TX_DESC_STC *firstDesc;  /* The first descriptor of the          */
                                /* currently transmitted packet.        */
    PRV_CPSS_SW_TX_DESC_STC  *currSwDesc;  /* The current handled descriptor of*/
                                /* the currently transmitted packet.    */
    PRV_CPSS_TX_DESC_STC  tmpTxDesc;  /* Temporary Tx descriptor used for     */
                                /* preparing the real descriptor data.  */
    PRV_CPSS_TX_DESC_STC  tmpFirstTxDesc; /* Temporary Tx descriptor used for */
                                /* preparing the real first descriptor  */
                                /* data.                                */
    GT_U32      descWord1;          /* The first word of the Tx descriptor. */
    GT_UINTPTR  tmpBuffPtr;         /* Holds the real buffer pointer.       */
    GT_U32      txCmdMask;          /* Mask for Tx command register.        */
    GT_U32      i;
    GT_U32      numOfBufs;
    GT_U32      portGroupId;/* port group Id for multi-port-groups device support */
    GT_U8*      shortBuffer;/* pointer to short buffer */
    GT_U32      shortBuffers_index = 0;/*index to short buffers array*/

    /* must be first line before any chance that function may return
      indicate that the descriptors not changed yet */
    *txDescListUpdatedPtr = GT_FALSE;

    /* we will use the 'First active port group' , to represent the whole device.
       that way we allow application to give SDMA memory to single port group instead
       of split it between all active port groups
    */
    portGroupId = PRV_CPSS_NETIF_SDMA_PORT_GROUP_ID_MAC(devNum);

    /* Set to 0, in the function body will be updated to real number of used buffers. */
    numOfBufs = *numOfBufsPtr;
    *numOfBufsPtr = 0;

    /* Set tx parameters */
    txQueue     = pcktParamsPtr->txQueue;
    PRV_CPSS_PX_COS_CHECK_TC_MAC(txQueue);

    if(buffLenList[0] <= TX_SHORT_BUFF_SIZE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "First buffer length must be more than [%d] bytes",
            TX_SHORT_BUFF_SIZE);
    }

    for(i = 0 ; i < numOfBufs ; i++)
    {
        if(buffLenList[i] == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "buffLenList at index[%d] must not be ZERO",
                i);
        }
    }

    /* Number of buffers added = 0 */
    /* the Application prepare packet 'AS IS' */
    if(numOfBufs > txDescListPtr->maxDescNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "NOT enough TX descriptors [%d] allocated at 'init' for this packet[%d]",
            txDescListPtr->maxDescNum,
            numOfBufs);
    }

    /* Check if there are enough descriptors for this packet. */
    if(numOfBufs > txDescListPtr->freeDescNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "NOT enough descriptors [%d] for this packet[%d]",
            txDescListPtr->freeDescNum,
            numOfBufs);
    }

    /* the descriptors start to change here */
    *txDescListUpdatedPtr = GT_TRUE;

    /* Set descriptor parameters */
    currSwDesc              = txDescListPtr->next2Feed;
    firstDesc               = currSwDesc->txDesc;
    /* Set the cookie in the userData - usually the packet pointer received by
       the application */
    currSwDesc->userData = 0;
    currSwDesc->evReqHndl= 0;

    /***********************************************/
    /* start with the original first buffer's data */
    /***********************************************/
    descWord1 = 0;

    if(pcktParamsPtr->recalcCrc == GT_TRUE)
    {
        descWord1 |= (1 << 12);
    }

    /* Set the first descriptor parameters. */
    TX_DESC_RESET_MAC(&tmpFirstTxDesc);

    tmpFirstTxDesc.word1 = descWord1;

    /* Set bit for first buffer of a frame for Temporary Tx descriptor */
    TX_DESC_SET_FIRST_BIT_MAC(&tmpFirstTxDesc,1);
    /* Add Source MAC Address and Destination MAC Address */
    TX_DESC_SET_BYTE_CNT_MAC(&tmpFirstTxDesc,buffLenList[0]);

    /* update the packet header to the first descriptor */
    rc = cpssOsVirt2Phy((GT_UINTPTR)buffList[0],&tmpBuffPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    CPSS_OS_PHY_ADDR_IN_32_BITS_CHECK(tmpBuffPtr);

    tmpFirstTxDesc.buffPointer = CPSS_32BIT_LE((GT_U32)tmpBuffPtr);

    /* in case last descriptor don't swap
       need to set: Enable Interrupt bit, Last buffer of frame bit */
    if(numOfBufs != 1)
    {
        /*******************************************/
        /* continue with the original next buffers */
        /*******************************************/

        /* Add rest of the buffers (starting from second buffer) */
        for(i = 1; i < numOfBufs; i++)
        {
            currSwDesc = currSwDesc->swNextDesc;

            TX_DESC_RESET_MAC(&tmpTxDesc);
            tmpTxDesc.word1 = descWord1;

            rc = cpssOsVirt2Phy((GT_UINTPTR)(buffList[i]),/*OUT*/&tmpBuffPtr);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* Check if the buffers length is larger than (TX_SHORT_BUFF_SIZE)  */
            /* or the alignment is OK */
            if(buffLenList[i] > TX_SHORT_BUFF_SIZE ||
                (0 == (tmpBuffPtr & (TX_SHORT_BUFF_SIZE-1))))
            {
                /* the physical address is valid for this buffer */
            }
            else
            {
                if(shortBuffers_index >= txDescListPtr->shortBuffers_number)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,
                        "NOT enough short buffers [%d] allocated at 'init'",
                        txDescListPtr->shortBuffers_number);
                }

                /* the physical address is valid for this short buffer */
                shortBuffer = &txDescListPtr->shortBuffers_arr[shortBuffers_index*TX_SHORT_BUFF_SIZE];
                /* we need to use valid address from the short buffers array of this queue */
                cpssOsMemCpy(shortBuffer,buffList[i],buffLenList[i]);
                shortBuffers_index++;

                rc = cpssOsVirt2Phy((GT_UINTPTR)(shortBuffer),&tmpBuffPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            CPSS_OS_PHY_ADDR_IN_32_BITS_CHECK(tmpBuffPtr);

            tmpTxDesc.buffPointer = CPSS_32BIT_LE((GT_U32)tmpBuffPtr);
            TX_DESC_SET_BYTE_CNT_MAC(&tmpTxDesc,buffLenList[i]);
            TX_DESC_SET_OWN_BIT_MAC(&tmpTxDesc,TX_DESC_DMA_OWN);

            /* in case last descriptor don't swap  */
            /* need to set: Enable Interrupt bit, Last buffer of frame bit */
            /* In case PRV_CPSS_PX_XCAT_TX_CPU_CORRUPT_BUFFER_WA_E is */
            /* to be done (3 == numOfAddedBuffs) the last buffer in the */
            /* received buffer list is not the last one.                */
            if((i != (numOfBufs - 1)))
            {
                tmpTxDesc.word1 = CPSS_32BIT_LE(tmpTxDesc.word1);
                tmpTxDesc.word2 = CPSS_32BIT_LE(tmpTxDesc.word2);
                TX_DESC_COPY_MAC(currSwDesc->txDesc,&tmpTxDesc);
            }
        }


        /**************************************/
        /* Set the last descriptor parameters */
        /**************************************/
        /* Set the bit for Last buffer of a frame */
        TX_DESC_SET_LAST_BIT_MAC(&tmpTxDesc,1);

        /* check if caller want to get the event invocation ,
           NOTE : we must not invoke when sending "synchronic" */
        /* Set the bit for Disable Interrupt */
        TX_DESC_SET_INT_BIT_MAC(&tmpTxDesc,0);

        tmpTxDesc.word1 = CPSS_32BIT_LE(tmpTxDesc.word1);
        tmpTxDesc.word2 = CPSS_32BIT_LE(tmpTxDesc.word2);
        TX_DESC_COPY_MAC(currSwDesc->txDesc,&tmpTxDesc);
    }
    else
    {
        /**************************************/
        /* Set the last descriptor parameters */
        /**************************************/
        /* Set the bit for Last buffer of a frame */
        TX_DESC_SET_LAST_BIT_MAC(&tmpFirstTxDesc,1);

        /* check if caller want to get the event invocation ,
           NOTE : we must not invoke when sending "synchronic" */
        /* Set the bit for Disable Interrupt */
        TX_DESC_SET_INT_BIT_MAC(&tmpFirstTxDesc,0);
    }


    txDescListPtr->freeDescNum -= (numOfBufs);
    txDescListPtr->next2Feed    = currSwDesc->swNextDesc;
    /* Set the sctual number of used buffers. */
    *numOfBufsPtr = numOfBufs;

    /* Make sure that all previous operations where */
    /* executed before changing the own bit of the  */
    /* first descriptor.                            */
    GT_SYNC;

    /* Set the first descriptor own bit to start transmitting.  */
    TX_DESC_SET_OWN_BIT_MAC(&tmpFirstTxDesc,TX_DESC_DMA_OWN);
    tmpFirstTxDesc.word1  = CPSS_32BIT_LE(tmpFirstTxDesc.word1);
    tmpFirstTxDesc.word2  = CPSS_32BIT_LE(tmpFirstTxDesc.word2);
    TX_DESC_COPY_MAC(firstDesc,&tmpFirstTxDesc);

    /* The Enable DMA operation should be done only */
    /* AFTER all desc. operations where completed.  */
    GT_SYNC;

    /* Enable the Tx DMA.   */
    txCmdMask = 1 << txQueue;
    rc = prvCpssHwPpPortGroupWriteRegister(CAST_SW_DEVNUM(devNum),portGroupId,PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.sdmaRegs.txQCmdReg,txCmdMask);
    if(rc != GT_OK)
        return rc;

    {
#ifndef ASIC_SIMULATION
        /* DMA procedure may take a long time for Jumbo packets.
          E.g. the TX of 10240 Byte packet may take 411 micro seconds for PEX Gen 1.1.
          PEX Gen 1.1 haz 250 MBytes per second maximal throughput. The TX of
          10240 bytes minimal time will take:
           (10240 + 16 (eDSA size) + 16 (descriptor size)) / 250 M = 41.088 microseconds
          The ARM v7 CPU with 1.3 Giga Core Clock and DDR3-1333 performs
          approximately 15 millions per second loops of OWN Bit check algorithm.
          It's required 41.088 * 15 = 613 loops for ARM CPU and maximal PEX 1.1 throughput.
          Choose approx 500 times more loops to be on safe side for high end CPUs - 312000.
          The "numOfBufs + numOfAddedBuffs"  is 3 or more.
          Use constant 104000 to get 312000 loop count.*/
        GT_U32 loopIndex = (numOfBufs) * 104000;
        GT_U32 doSleep = 0;
#else
        GT_U32 doSleep = 1;
        GT_U32 loopIndex = (numOfBufs) * 500;
#endif /*ASIC_SIMULATION*/
        GT_U32 sleepTime;/*time to sleep , to allow the 'Asic simulation' process the packet */

        if(cpssDeviceRunCheck_onEmulator())
        {
            doSleep = 1;
            loopIndex = (numOfBufs) * 500;
        }

        /* allow another task to process it if ready , without 1 millisecond penalty */
        /* this code of sleep 0 Vs sleep 1 boost the performance *20 in enhanced-UT !!! */
        sleepTime = 0;

        tryMore_lbl:
        /* Wait until PP sent the packet. Or HW error if while block
                   run more than loopIndex times */
        while (loopIndex && (TX_DESC_GET_OWN_BIT_MAC(devNum,currSwDesc->txDesc) == TX_DESC_DMA_OWN))
        {
            if(doSleep)
            {
                /* do some sleep allow the simulation process the packet */
                cpssOsTimerWkAfter(sleepTime);
            }
            loopIndex--;
        }

        if(loopIndex == 0)
        {
            if(doSleep)
            {
                if(sleepTime == 0)/* the TX was not completed ? we need to allow more retries with 'sleep (1)'*/
                {
                    loopIndex = (numOfBufs) * 500;
                    sleepTime = 1;
                    goto tryMore_lbl;
                }
            }
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, "devNum[%d] : After all iterations the device not finished processing the packet",
                devNum);
        }

    }


    return GT_OK;
}

/*******************************************************************************
* cpssPxNetIfSdmaSyncTxPacketSend
*
*       This function transmit packet from the CPU to the PP via the SDMA interface.
*       Prepared descriptors to the PP's SDMA TX queues.
*       After the transmit ends, all transmitted packets descriptors are freed.
*        -- SDMA relate.
*       function activates Tx SDMA , function wait for PP to finish processing
*       the buffer(s).
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum       - Device number.
*       pcktParamsPtr- The internal packet params to be set into the packet
*                      descriptors.
*       buffList     - The packet data buffers list.
*       buffLenList  - A list of the buffers len in buffList.
*       numOfBufs    - Length of buffList.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK          - on success, or
*       GT_NO_RESOURCE - if there is not enough free elements in the fifo
*                        associated with the Event Request Handle.
*       GT_EMPTY       - if there are not enough descriptors to do the sending.
*       GT_HW_ERROR    - when after transmission last descriptor own bit wasn't
*                        changed for long time.
*       GT_BAD_PARAM   - the data buffer is longer than allowed.
*                        Buffer data can occupied up to the maximum number of descriptors defined.
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_INITIALIZED       - the library was not initialized
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_FAIL otherwise.
*
* COMMENTS:
*
*******************************************************************************/
static GT_STATUS internal_cpssPxNetIfSdmaSyncTxPacketSend
(
    IN GT_SW_DEV_NUM                            devNum,
    IN CPSS_PX_NET_TX_PARAMS_STC                *pcktParamsPtr,
    IN GT_U8                                    *buffList[],
    IN GT_U32                                   buffLenList[],
    IN GT_U32                                   numOfBufs
)
{
    GT_STATUS rc0 = GT_OK;
    GT_STATUS rc1 = GT_OK;
    GT_BOOL   txDescListUpdated;/* do we need to restore descriptors */
    GT_U32 txQueue;              /* The tx queue to send this packet to. */
    PRV_CPSS_TX_DESC_LIST_STC *txDescList;   /* The Tx desc. list control structure. */
    PRV_CPSS_SW_TX_DESC_STC    *firstDescPtr;
    GT_U32                      numOfUsedDesc;

    GT_UINTPTR                  phyNext2Feed; /* The physicat address of the next2Feed*/
                                              /* field.                               */
    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddr;
    GT_U32         portGroupId;/* port group Id for multi-port-groups device support */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(pcktParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(buffList);
    CPSS_NULL_PTR_CHECK_MAC(buffLenList);
    /*check that the DB of networkIf library was initialized*/
    PRV_CPSS_PX_NETIF_INITIALIZED_CHECK_MAC(devNum);
    PRV_CPSS_PX_SDMA_USED_CHECK_MAC(devNum);

    /* we will use the 'First active port group' , to represent the whole device.
       that way we allow application to give SDMA memory to single port group instead
       of split it between all active port groups
    */
    portGroupId = PRV_CPSS_NETIF_SDMA_PORT_GROUP_ID_MAC(devNum);

    txQueue     = pcktParamsPtr->txQueue;
    PRV_CPSS_PX_COS_CHECK_TC_MAC(txQueue);

    txDescList  = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.txDescList[txQueue]);


    /* pxNetIfSdmaTxPacketSend changes txDescList->next2Feed, keep it here
    and return it back after cleaning. */
    firstDescPtr = txDescList->next2Feed;

    numOfUsedDesc = numOfBufs;

    /* synchronic sending */
    rc0 =  pxNetIfSdmaTxPacketSend(devNum,pcktParamsPtr,buffList,buffLenList,
                                    &numOfUsedDesc, txDescList ,
                                    &txDescListUpdated);

    if (txDescListUpdated == GT_TRUE)
    {
        /* Don't check rc here, need to restore data structure*/

        /* Restore next to feed descriptor. */
        txDescList->next2Feed = firstDescPtr;
        txDescList->freeDescNum += numOfUsedDesc;

        /* Returned the buffers to application. Return the state of descriptors as
         it was before calling internalTxPacketSend. Set txDescList->next2Feed
         to point to the same SW_DESC before calling internalTxPacketSend
         Then return the status to the caller (even in case of error). */
        while(numOfUsedDesc > 0)
        {
            firstDescPtr->txDesc->word1 = 0x0;
            firstDescPtr->txDesc->word2 = 0x0;
            firstDescPtr = firstDescPtr->swNextDesc;
            numOfUsedDesc--;
        }

        /* Set/Restore Tx Current Desc Pointer Register. */
        regsAddr    = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);
        rc1 = cpssOsVirt2Phy((GT_UINTPTR)(txDescList->next2Feed->txDesc),/*OUT*/&phyNext2Feed);
        if (rc1 != GT_OK)
        {
            return rc1;
        }

        CPSS_OS_PHY_ADDR_IN_32_BITS_CHECK(phyNext2Feed);

        rc1 = prvCpssHwPpPortGroupWriteRegister(CAST_SW_DEVNUM(devNum),portGroupId,regsAddr->MG.sdmaRegs.txDmaCdp[txQueue],(GT_U32)phyNext2Feed);
        if(rc1 != GT_OK)
            return rc1;
    }

    return rc0;
}

/*******************************************************************************
* cpssPxNetIfSdmaSyncTxPacketSend
*
*       This function transmit packet from the CPU to the PP via the SDMA interface.
*       Prepared descriptors to the PP's SDMA TX queues.
*       After the transmit ends, all transmitted packets descriptors are freed.
*        -- SDMA relate.
*       function activates Tx SDMA , function wait for PP to finish processing
*       the buffer(s).
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum       - Device number.
*       pcktParamsPtr- The internal packet params to be set into the packet
*                      descriptors.
*       buffList     - The packet data buffers list.
*       buffLenList  - A list of the buffers len in buffList.
*       numOfBufs    - Length of buffList.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK          - on success, or
*       GT_NO_RESOURCE - if there is not enough free elements in the fifo
*                        associated with the Event Request Handle.
*       GT_EMPTY       - if there are not enough descriptors to do the sending.
*       GT_HW_ERROR    - when after transmission last descriptor own bit wasn't
*                        changed for long time.
*       GT_BAD_PARAM   - the data buffer is longer than allowed.
*                        Buffer data can occupied up to the maximum number of descriptors defined.
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_INITIALIZED       - the library was not initialized
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_FAIL otherwise.
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS cpssPxNetIfSdmaSyncTxPacketSend
(
    IN GT_SW_DEV_NUM                            devNum,
    IN CPSS_PX_NET_TX_PARAMS_STC                *pcktParamsPtr,
    IN GT_U8                                    *buffList[],
    IN GT_U32                                   buffLenList[],
    IN GT_U32                                   numOfBufs
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxNetIfSdmaSyncTxPacketSend);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pcktParamsPtr, buffList, buffLenList, numOfBufs));

    rc = internal_cpssPxNetIfSdmaSyncTxPacketSend(devNum, pcktParamsPtr, buffList, buffLenList, numOfBufs);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pcktParamsPtr, buffList, buffLenList, numOfBufs));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssPxNetIfSdmaTxQueueEnable function
* @endinternal
*
* @brief   Enable/Disable the specified traffic class queue for TX
*         on all packet processors in the system. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*                                      queue     - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] enable                   - GT_TRUE,  queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on CPU port is not SDMA
*/
static GT_STATUS internal_cpssPxNetIfSdmaTxQueueEnable
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_U32               txQueue,
    IN  GT_BOOL              enable
)
{
    GT_U32  regAddr;/* The register address to write to.    */
    GT_U32  data;
    GT_U32  portGroupId;/* port group Id for multi-port-groups device support */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(txQueue);
    PRV_CPSS_PX_NETIF_INITIALIZED_CHECK_MAC(devNum);

    /* we will use the 'First active port group' , to represent the whole device.
       that way we allow application to give SDMA memory to single port group instead
       of split it between all active port groups
    */
    portGroupId = PRV_CPSS_NETIF_SDMA_PORT_GROUP_ID_MAC(devNum);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.sdmaRegs.txQCmdReg;

    if (enable == GT_TRUE)
    {
        data = (1 << txQueue);
    }
    else
    {
        data = (1 << (8 + txQueue));
    }

    return prvCpssHwPpPortGroupWriteRegister(CAST_SW_DEVNUM(devNum), portGroupId,regAddr, data);
}

/**
* @internal cpssPxNetIfSdmaTxQueueEnable function
* @endinternal
*
* @brief   Enable/Disable the specified traffic class queue for TX
*         on all packet processors in the system. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*                                      queue     - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] enable                   - GT_TRUE,  queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on CPU port is not SDMA
*/
GT_STATUS cpssPxNetIfSdmaTxQueueEnable
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_U32               txQueue,
    IN  GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxNetIfSdmaTxQueueEnable);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, txQueue, enable));

    rc = internal_cpssPxNetIfSdmaTxQueueEnable(devNum, txQueue, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, txQueue, enable));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssPxNetIfSdmaTxQueueEnableGet function
* @endinternal
*
* @brief   Get status of the specified traffic class queue for TX
*         packets from CPU. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] txQueue                  - traffic class queue
*
* @param[out] enablePtr                - GT_TRUE, enable queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on CPU port is not SDMA
* @retval GT_BAD_PTR               - on NULL ptr
*/
static GT_STATUS internal_cpssPxNetIfSdmaTxQueueEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_U32               txQueue,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;/* The register address to write to.    */
    GT_U32      hwData, hwValue;
    GT_U32      portGroupId;/* port group Id for multi-port-groups device support */


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(txQueue);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    PRV_CPSS_PX_NETIF_INITIALIZED_CHECK_MAC(devNum);

    /* we will use the 'First active port group' , to represent the whole device.
       that way we allow application to give SDMA memory to single port group instead
       of split it between all active port groups
    */
    portGroupId = PRV_CPSS_NETIF_SDMA_PORT_GROUP_ID_MAC(devNum);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.sdmaRegs.txQCmdReg;

    rc = prvCpssHwPpPortGroupReadRegister(CAST_SW_DEVNUM(devNum), portGroupId,regAddr, &hwValue);

    if(GT_OK != rc)
    {
        return rc;
    }

    hwData = hwValue & 0xFFFF;

    /* Need check only TxENQ bit to get Tx status.
      The TX SDMA is not active when TxENQ is 0. */
    if((hwData >> txQueue) & 1)
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
* @internal cpssPxNetIfSdmaTxQueueEnableGet function
* @endinternal
*
* @brief   Get status of the specified traffic class queue for TX
*         packets from CPU. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] txQueue                  - traffic class queue
*
* @param[out] enablePtr                - GT_TRUE, enable queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on CPU port is not SDMA
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssPxNetIfSdmaTxQueueEnableGet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_U32               txQueue,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxNetIfSdmaTxQueueEnableGet);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, txQueue, enablePtr));

    rc = internal_cpssPxNetIfSdmaTxQueueEnableGet(devNum, txQueue, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, txQueue, enablePtr));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssPxNetIfSdmaTxFreeDescripotrsNumberGet function
* @endinternal
*
* @brief   This function return the number of free Tx descriptors for given
*         device and txQueue
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - Tx queue number.
*
* @param[out] numberOfFreeTxDescriptorsPtr - pointer to number of free
*                                      descriptors for the given queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device or queue
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxNetIfSdmaTxFreeDescripotrsNumberGet
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           txQueue,
    OUT GT_U32          *numberOfFreeTxDescriptorsPtr
)
{
    PRV_CPSS_TX_DESC_LIST_STC       *txDescListPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(numberOfFreeTxDescriptorsPtr);
    PRV_CPSS_PX_NETIF_INITIALIZED_CHECK_MAC(devNum);
    PRV_CPSS_PX_SDMA_USED_CHECK_MAC(devNum);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(txQueue);

    /* the Tx descriptors for this Queue */
    txDescListPtr  = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.txDescList[txQueue]);

    *numberOfFreeTxDescriptorsPtr = txDescListPtr->freeDescNum;

    return GT_OK;
}

/**
* @internal cpssPxNetIfSdmaTxFreeDescripotrsNumberGet function
* @endinternal
*
* @brief   This function return the number of free Tx descriptors for given
*         device and txQueue
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - Tx queue number.
*
* @param[out] numberOfFreeTxDescriptorsPtr - pointer to number of free
*                                      descriptors for the given queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device or queue
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxNetIfSdmaTxFreeDescripotrsNumberGet
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           txQueue,
    OUT GT_U32          *numberOfFreeTxDescriptorsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxNetIfSdmaTxFreeDescripotrsNumberGet);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, txQueue, numberOfFreeTxDescriptorsPtr));

    rc = internal_cpssPxNetIfSdmaTxFreeDescripotrsNumberGet(devNum, txQueue, numberOfFreeTxDescriptorsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, txQueue, numberOfFreeTxDescriptorsPtr));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}



