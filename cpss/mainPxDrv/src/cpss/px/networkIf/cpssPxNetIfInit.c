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
* @file cpssPxNetIfInit.c
*
* @brief CPSS PX SDMA network interface initialization functions.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/networkIf/private/prvCpssPxNetworkIfLog.h>
#include <cpss/px/networkIf/cpssPxNetIf.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define ALIGN_4B_CNS        4

GT_STATUS internal_cpssPxNetIfRxStop /* replace cpssPxNetIfRemove*/
(
    IN  GT_SW_DEV_NUM   devNum
);

/**
* @internal sdmaTxRegConfig function
* @endinternal
*
* @brief   Set the needed values for SDMA registers to enable Tx activity.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The Pp device number.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS sdmaTxRegConfig
(
    IN GT_SW_DEV_NUM devNum
)
{
    GT_U8       i;  /* SDMA queue iterator */
    GT_STATUS   rc; /* return code */
    GT_U32      portGroupId;/* port group Id for multi-port-groups device support */

    /* we will use the 'First active port group' , to represent the whole device.
       that way we allow application to give SDMA memory to single port group instead
       of split it between all active port groups
    */
    portGroupId = PRV_CPSS_NETIF_SDMA_PORT_GROUP_ID_MAC(devNum);

    /* Since working in SP the configure transmit queue WRR value to 0 */
    for(i = 0; i < NUM_OF_TX_QUEUES; i++)
    {
       rc = prvCpssHwPpPortGroupWriteRegister(CAST_SW_DEVNUM(devNum),portGroupId, PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.sdmaRegs.txQWrrPrioConfig[i], 0);
       if(rc != GT_OK)
           return rc;

        /********* Tx SDMA Token-Bucket Queue<n> Counter ************/
        rc = prvCpssHwPpWriteRegister(CAST_SW_DEVNUM(devNum), PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.sdmaRegs.txSdmaTokenBucketQueueCnt[i], 0);
        if(rc != GT_OK)
            return rc;

        /********** Tx SDMA Token Bucket Queue<n> Configuration ***********/
        rc = prvCpssHwPpWriteRegister(CAST_SW_DEVNUM(devNum), PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.sdmaRegs.txSdmaTokenBucketQueueConfig[i] , 0xfffffcff);
        if(rc != GT_OK)
            return rc;
    }

    /*********************/
    rc = prvCpssHwPpWriteRegister(CAST_SW_DEVNUM(devNum), PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.sdmaRegs.txSdmaWrrTokenParameters, 0xffffffc1);
    if(rc != GT_OK)
        return rc;

    /*********** Set all queues to Fix priority **********/
    rc = prvCpssHwPpPortGroupWriteRegister(CAST_SW_DEVNUM(devNum), portGroupId,PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.sdmaRegs.txQFixedPrioConfig, 0xFF);

    return rc;
}
/**
* @internal sdmaTxInit function
* @endinternal
*
* @brief   This function initializes the Core Tx module, by allocating the cyclic
*         Tx descriptors list, and the tx Headers buffers.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number to init the Tx unit for.
* @param[in] descBlockPtr             - A block to allocate the descriptors from.
* @param[in] descBlockSize            - Size in bytes of descBlockPtr.
*
* @param[out] numOfDescsPtr            - Number of allocated Tx descriptors.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS sdmaTxInit
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U8    *descBlockPtr,
    IN GT_U32   descBlockSize,
    OUT GT_U32  *numOfDescsPtr
)
{
    GT_STATUS rc = GT_OK;

    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddr;
    PRV_CPSS_TX_DESC_LIST_STC *txDescList;   /* Points to the relevant Tx desc. list */

    PRV_CPSS_TX_DESC_STC *firstTxDesc;/* Points to the first Tx desc in list. */
    GT_U8 txQueue;              /* Index of the Tx Queue.               */
    GT_U32 numOfTxDesc;         /* Number of Tx desc. that may be       */
                                /* allocated from the given block.      */
    GT_U32 sizeOfDesc;          /* The amount of memory (in bytes) that*/
                                /* a single desc. will occupy, including*/
                                /* the alignment.                       */
    GT_U32 descPerQueue;        /* Number of descriptors per Tx Queue.  */
    GT_U32 i;
    PRV_CPSS_SW_TX_DESC_STC *swTxDesc = NULL;/* Points to the Sw Tx desc to   */
                                /* init.                                */
    PRV_CPSS_SW_TX_DESC_STC *firstSwTxDesc;/* Points to the first Sw Tx desc  */
                                /* in list.                             */
    GT_UINTPTR  phyNext2Feed;       /* The physical address of the next2Feed*/
                                /* field.                               */
    GT_UINTPTR nextDescPointer;
    GT_U32      shortBuffers_number;
    GT_UINTPTR  tmpBuffPtr;         /* Holds the real buffer pointer.       */
    GT_U8*      shortBuffer;/* pointer to short buffer */

    txDescList  = PRV_CPSS_PP_MAC(devNum)->intCtrl.txDescList;
    regsAddr    = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);

    /* Set the descBlockPtr to point to an aligned start address. */
    if(((GT_UINTPTR)descBlockPtr % TX_DESC_ALIGN) != 0)
    {
        descBlockSize -= TX_DESC_ALIGN - (((GT_UINTPTR)descBlockPtr) % TX_DESC_ALIGN);

        descBlockPtr =
            (GT_U8*)((GT_UINTPTR)descBlockPtr +
                     (TX_DESC_ALIGN - (((GT_UINTPTR)descBlockPtr) % TX_DESC_ALIGN)));
    }

    /* Tx Descriptor list initialization.   */
    sizeOfDesc  = sizeof(PRV_CPSS_TX_DESC_STC);

    if((sizeOfDesc % TX_DESC_ALIGN) != 0)
    {
        sizeOfDesc += (TX_DESC_ALIGN-(sizeof(PRV_CPSS_TX_DESC_STC) % TX_DESC_ALIGN));

    }

    numOfTxDesc = descBlockSize / sizeOfDesc;

    /******************************************************
    NOTE: unlike the DXCH code , the PX code NOT allocating 'short' buffers per TX descriptor.
        those needed for sending buffers in size of 1..8 that must be aligned to 8 bytes
        physical address


        meaning that single packet can be send from CPU with single buffer and single descriptor
        (not adding descriptor and buffer for DSA tag)
    ******************************************************/
    /* Descriptors are already aligned to 16 bytes. */
    descPerQueue = numOfTxDesc / NUM_OF_TX_QUEUES;
    /* Number of descriptors must be 'even' (allowed to divided by 2) */
    descPerQueue -= (descPerQueue & 0x1);

    /* use 16 short buffers per queue */
    shortBuffers_number = 16;
    /* the short buffer size is 8 and tx-descriptor size is 16 ,
       we not want to take more short buffers than number of descriptors
       each descriptor that we take used for 2 short buffers.
    */
    if((descPerQueue / 4) < shortBuffers_number)
    {
        /* we cant afford 16 descriptors per queue ... so use less */
        shortBuffers_number = (descPerQueue / 4) * 2;
    }

    /* steal the short buffers from the descriptors memory */
    descPerQueue -= shortBuffers_number;

    *numOfDescsPtr = descPerQueue * NUM_OF_TX_QUEUES;

    for(txQueue = 0; txQueue < NUM_OF_TX_QUEUES; txQueue++)
    {
        txDescList[txQueue].freeDescNum = descPerQueue;
        txDescList[txQueue].maxDescNum = descPerQueue;

        firstTxDesc = (PRV_CPSS_TX_DESC_STC*)descBlockPtr;

        /* hotSynch/shutdown treatment - allocate Mem only once */
        if (txDescList[txQueue].swTxDescBlock == NULL)
        {
            firstSwTxDesc = (PRV_CPSS_SW_TX_DESC_STC *)
                         cpssOsMalloc(sizeof(PRV_CPSS_SW_TX_DESC_STC )*descPerQueue);
            if(firstSwTxDesc == NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }
            cpssOsMemSet(firstSwTxDesc, 0, sizeof(PRV_CPSS_SW_TX_DESC_STC )*descPerQueue);

            /* save pointer to allocated Mem block on per queue structure */
            txDescList[txQueue].swTxDescBlock = firstSwTxDesc;
        }
        else
        {
            firstSwTxDesc = txDescList[txQueue].swTxDescBlock;
        }

        if(firstSwTxDesc == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        /* create Tx End FIFO in request driven mode */
        txDescList[txQueue].poolId = 0;

        for(i = 0; i < descPerQueue; i++)
        {

            swTxDesc = firstSwTxDesc + i;


            swTxDesc->txDesc = (PRV_CPSS_TX_DESC_STC*)descBlockPtr;
            descBlockPtr   = (GT_U8*)((GT_UINTPTR)descBlockPtr + sizeOfDesc);

            TX_DESC_RESET_MAC(swTxDesc->txDesc);

            if((descPerQueue - 1) != i)
            {
                /* Next descriptor should not be configured for the last one.*/
                swTxDesc->swNextDesc  = firstSwTxDesc + i + 1;
                rc = cpssOsVirt2Phy((GT_UINTPTR)descBlockPtr, /*OUT*/&nextDescPointer);
                if (rc != GT_OK)
                {
                    return rc;
                }

                CPSS_OS_PHY_ADDR_IN_32_BITS_CHECK(nextDescPointer);

                swTxDesc->txDesc->nextDescPointer = CPSS_32BIT_LE((GT_U32)nextDescPointer);
            }

            /* in PX we not use short buffer per descriptor ! */
            /* see the use of short buffers per queue :
                txDescList[txQueue].shortBuffers_arr[] */
            swTxDesc->shortBuffer = 0;
        }

        /* Close the cyclic desc. list. */
        swTxDesc->swNextDesc = firstSwTxDesc;
        rc = cpssOsVirt2Phy((GT_UINTPTR)firstTxDesc, /*OUT*/&nextDescPointer);
        if (rc != GT_OK)
        {
            return rc;
        }

        CPSS_OS_PHY_ADDR_IN_32_BITS_CHECK(nextDescPointer);

        swTxDesc->txDesc->nextDescPointer = CPSS_32BIT_LE((GT_U32)nextDescPointer);

        txDescList[txQueue].next2Feed   = firstSwTxDesc;
        txDescList[txQueue].next2Free   = firstSwTxDesc;
    }

    /****************************************/
    /* handle the short buffers (per queue) */
    /****************************************/
    for(txQueue = 0; txQueue < NUM_OF_TX_QUEUES; txQueue++)
    {
        /* give each queue it's short buffers */
        txDescList[txQueue].shortBuffers_number = shortBuffers_number;
        txDescList[txQueue].shortBuffers_arr =
            (GT_U8*)((GT_UINTPTR)descBlockPtr + (txQueue * shortBuffers_number * TX_SHORT_BUFF_SIZE));

        shortBuffer = &txDescList[txQueue].shortBuffers_arr[0];

        for( i = 0 ; i < shortBuffers_number ; i++)
        {
            /* check that each short buffer is in proper aligned physical address */
            rc = cpssOsVirt2Phy((GT_UINTPTR)shortBuffer, /*OUT*/&tmpBuffPtr);
            if (rc != GT_OK)
            {
                return rc;
            }

            CPSS_OS_PHY_ADDR_IN_32_BITS_CHECK(tmpBuffPtr);

            if(0 != (tmpBuffPtr & (TX_SHORT_BUFF_SIZE-1)))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                    "could not get short buffer physical address aligned to [%d] bytes",
                    TX_SHORT_BUFF_SIZE);
            }

            /* jump to next short buffer */
            shortBuffer += TX_SHORT_BUFF_SIZE;
        }
    }

    for(txQueue = 0; txQueue < NUM_OF_TX_QUEUES; txQueue++)
    {
        rc = cpssOsVirt2Phy((GT_UINTPTR)(txDescList[txQueue].next2Feed->txDesc),/*OUT*/&phyNext2Feed);
        if (rc != GT_OK)
        {
            return rc;
        }

        CPSS_OS_PHY_ADDR_IN_32_BITS_CHECK(phyNext2Feed);

        rc = prvCpssHwPpWriteRegister(
            CAST_SW_DEVNUM(devNum),
            regsAddr->MG.sdmaRegs.txDmaCdp[txQueue],(GT_U32)phyNext2Feed);

        if(rc != GT_OK)
            return rc;
    }

    return sdmaTxRegConfig(devNum);
}

/**
* @internal sdmaRxInit function
* @endinternal
*
* @brief   This function initializes the Core Rx module, by allocating the cyclic
*         Rx descriptors list, and the rx buffers. -- SDMA
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number to init the Rx unit for.
* @param[in] descBlockPtr             - A block to allocate the descriptors from.
* @param[in] descBlockSize            - Size in bytes of descBlock.
* @param[in] rxBufInfoPtr             - A block to allocate the Rx buffers from.
*
* @param[out] numOfDescsPtr            - Number of Rx descriptors allocated.
* @param[out] numOfBufsPtr             - Number of Rx buffers allocated.
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS sdmaRxInit
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U8                    *descBlockPtr,
    IN GT_U32                   descBlockSize,
    IN CPSS_RX_BUF_INFO_STC     *rxBufInfoPtr,
    OUT GT_U32                  *numOfDescsPtr,
    OUT GT_U32                  *numOfBufsPtr
)
{
    GT_STATUS rc = GT_OK;

    PRV_CPSS_PX_PP_REGS_ADDR_VER1_STC *regsAddr;
    PRV_CPSS_RX_DESC_LIST_STC *rxDescList;   /* Points to the relevant Rx desc. list */
    PRV_CPSS_RX_DESC_STC *rxDesc = NULL; /* Points to the Rx desc to init.    */
    PRV_CPSS_RX_DESC_STC *firstRxDesc;/* Points to the first Rx desc in list. */
    PRV_CPSS_RX_DESC_STC *theFirstRxDesc;/* Points to the first Rx desc. */
    GT_U8 rxQueue;              /* Index of the Rx Queue.               */
    GT_U32 numOfRxDesc;         /* Number of Rx desc. that may be       */
                                /* allocated from the given block.      */
    GT_U32 sizeOfDesc;          /* The amount of memory (in bytes) that*/
                                /* a single desc. will occupy, including*/
                                /* the alignment.                       */
    GT_U32 actualBuffSize;      /* Size of a single buffer, after taking*/
                                /* the required alignment into account.*/
    PRV_CPSS_SW_RX_DESC_STC *swRxDesc = NULL;/* Points to the Sw Rx desc to   */
                                /* init.                                */
    PRV_CPSS_SW_RX_DESC_STC *firstSwRxDesc;/* Points to the first Sw Rx desc  */
                                /* in list.                             */
    GT_U32 headerOffsetSize;    /* Size in bytes of the header offset   */
                                /* after alignment to RX_BUFF_ALIGN.    */
    GT_UINTPTR virtBuffAddr;    /* The virtual address of the Rx buffer */
                                /* To be enqueued into the current Rx   */
                                /* Descriptor.                          */
    GT_UINTPTR phyAddr;         /* Physical Rx descriptor's address.    */
    GT_U32 i;                   /* Loop counter                         */
    GT_U32 rxSdmaRegVal;        /* Rx SDMA Queues Register value        */

    /* The following vars. will hold the data from rxBufInfoPtr, and only  */
    /* some of them will be used, depending on the allocation method.   */
    CPSS_RX_BUFF_ALLOC_METHOD_ENT allocMethod;
    GT_U32 buffBlockSize = 0;   /* The static buffer block size (bytes) */
    GT_U8 *buffBlock = NULL;    /* A pointer to the static buffers block*/
    CPSS_RX_BUFF_MALLOC_FUNC mallocFunc = NULL;/* Function for allocating the buffers. */
    GT_U32         buffPercentageSum;/* buffer percentage summary       */
    GT_BOOL        needNetIfInit; /* network interface initialization or reconstruction.*/
    GT_U32         portGroupId;/* port group Id for multi-port-groups device support */
    GT_U32         headerOffset = rxBufInfoPtr->headerOffset;
    GT_U32         buffSize = rxBufInfoPtr->rxBufSize;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    /* we will use the 'First active port group' , to represent the whole device.
       that way we allow application to give SDMA memory to single port group instead
       of split it between all active port groups
    */
    portGroupId = PRV_CPSS_NETIF_SDMA_PORT_GROUP_ID_MAC(devNum);

    buffPercentageSum = 0;

    *numOfDescsPtr = 0;
    *numOfBufsPtr  = 0;

    rxDescList  = PRV_CPSS_PP_MAC(devNum)->intCtrl.rxDescList;
    regsAddr    = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum);

    /* get that state of the device , check if the device need network interface */
    /* initialization or reconstruction.                                         */
    rc = prvCpssPpConfigDevDbNetIfInitGet(CAST_SW_DEVNUM(devNum), &needNetIfInit);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* we need to get the values that were last used , when the HW didn't do reset */
    if(needNetIfInit == GT_FALSE)
    {
        /* get the info to synch with current HW state
           NOTE: the 'save' of this info was done in
           prvCpssPpConfigDevDbPrvInfoSet(...) */
        rc = prvCpssPpConfigDevDbPrvNetInfInfoGet(CAST_SW_DEVNUM(devNum), 0, NUM_OF_RX_QUEUES, rxDescList);
        if (rc != GT_OK)
        {
            return rc;
        }

        for(rxQueue = 0; rxQueue < NUM_OF_RX_QUEUES; rxQueue++)
        {
            /* update the total number of desc */
            *numOfDescsPtr += rxDescList[rxQueue].freeDescNum;

            /* update the total number of buffers */
            *numOfBufsPtr += rxDescList[rxQueue].freeDescNum;

            /* Enable Rx SDMA Queue */
            rc = cpssPxNetIfSdmaRxQueueEnable(devNum, rxQueue, GT_TRUE);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        return GT_OK;
    }

    allocMethod = rxBufInfoPtr->allocMethod;
    if(allocMethod == CPSS_RX_BUFF_DYNAMIC_ALLOC_E)
    {
        mallocFunc = rxBufInfoPtr->buffData.dynamicAlloc.mallocFunc;
    }
    else
    {
        buffBlock = (GT_U8*)rxBufInfoPtr->buffData.staticAlloc.rxBufBlockPtr;
        buffBlockSize = rxBufInfoPtr->buffData.staticAlloc.rxBufBlockSize;
    }

    /* Set the descBlockPtr to point to an aligned start address. */
    if(((GT_UINTPTR)descBlockPtr % RX_DESC_ALIGN) != 0)
    {
        descBlockSize = descBlockSize - (RX_DESC_ALIGN - ((GT_UINTPTR)descBlockPtr % RX_DESC_ALIGN));
        descBlockPtr = (GT_U8 *)((GT_UINTPTR)descBlockPtr + (RX_DESC_ALIGN - ((GT_UINTPTR)descBlockPtr % RX_DESC_ALIGN)));
    }

    /* Rx Descriptor list initialization.   */
    sizeOfDesc = sizeof(PRV_CPSS_RX_DESC_STC);

    if((sizeOfDesc % RX_DESC_ALIGN) != 0)
    {
        sizeOfDesc += (RX_DESC_ALIGN -(sizeof(PRV_CPSS_RX_DESC_STC) % RX_DESC_ALIGN));
    }

    /* The buffer size must be a multiple of RX_BUFF_SIZE_MULT  */
    actualBuffSize = buffSize - (buffSize % RX_BUFF_SIZE_MULT);
    if(actualBuffSize == 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, "Got calculated actualBuffSize == 0");

    if(allocMethod == CPSS_RX_BUFF_STATIC_ALLOC_E)
    {
        /* Number of Rx descriptors is calculated according to the  */
        /* size of the given Rx Buffers block.                      */
        /* Take the "dead" block in head of the buffers block as a  */
        /* result of the allignment.                                */
        numOfRxDesc = buffBlockSize / actualBuffSize;

        /* Set numOfRxDesc according to the number of descriptors that  */
        /* may be allocated from descBlockPtr and the number of buffers */
        /* that may be allocated from buffBlock.                        */
        if((descBlockSize / sizeOfDesc) < numOfRxDesc)
            numOfRxDesc = descBlockSize / sizeOfDesc;
    }
    else /* dynamic and no buffer allocation */
    {
        /* Number of Rx descriptors is calculated according to the  */
        /* (fixed) size of the given Rx Descriptors block.                  */
        numOfRxDesc = descBlockSize / sizeOfDesc;
    }

    theFirstRxDesc = (PRV_CPSS_RX_DESC_STC*)descBlockPtr;

    for(rxQueue = 0; rxQueue < NUM_OF_RX_QUEUES; rxQueue++)
    {
        buffPercentageSum += rxBufInfoPtr->bufferPercentage[rxQueue];

        /* validate the total percentage */
        CPSS_PARAM_CHECK_MAX_MAC(buffPercentageSum,101);

        /* set the desc count according to the percentage */
        rxDescList[rxQueue].freeDescNum =
            (rxBufInfoPtr->bufferPercentage[rxQueue] * numOfRxDesc) / 100;
        rxDescList[rxQueue].maxDescNum = rxDescList[rxQueue].freeDescNum;
        /* update the total number of desc */
        *numOfDescsPtr += rxDescList[rxQueue].freeDescNum;

        rxDescList[rxQueue].forbidQEn = GT_FALSE;

        firstRxDesc = (PRV_CPSS_RX_DESC_STC*)descBlockPtr;

        /* hotSynch/shutdown treatment - Allocate Mem only once */
        if (rxDescList[rxQueue].swRxDescBlock == NULL)
        {
            firstSwRxDesc = (PRV_CPSS_SW_RX_DESC_STC*)
                         cpssOsMalloc(sizeof(PRV_CPSS_SW_RX_DESC_STC)*
                                        rxDescList[rxQueue].freeDescNum);
            /* save pointer to allocated Mem block on per queue structure */
            rxDescList[rxQueue].swRxDescBlock = firstSwRxDesc;
        }
        else
        {
            firstSwRxDesc = rxDescList[rxQueue].swRxDescBlock;
        }

        if(firstSwRxDesc == NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);

        /* loop on all descriptors in the queue*/
        for(i = 0; i < rxDescList[rxQueue].freeDescNum; i++)
        {
            swRxDesc = firstSwRxDesc + i;

            rxDesc      = (PRV_CPSS_RX_DESC_STC*)descBlockPtr;
            descBlockPtr   = (GT_U8*)((GT_UINTPTR)descBlockPtr + sizeOfDesc);
            swRxDesc->rxDesc = rxDesc;
            RX_DESC_RESET_MAC(devNum,swRxDesc->rxDesc);

            /* make sure that all reserved fields of word2 are 0 */
            RX_DESC_RESET_WORD2_MAC(devNum,swRxDesc->rxDesc);

            if((rxDescList[rxQueue].freeDescNum - 1) != i)
            {
                /* Next descriptor should not be configured for the last one.*/
                swRxDesc->swNextDesc  = (firstSwRxDesc + i + 1);
                rc = cpssOsVirt2Phy((GT_UINTPTR)descBlockPtr, &phyAddr);
                if (rc != GT_OK)
                {
                    return rc;
                }

                CPSS_OS_PHY_ADDR_IN_32_BITS_CHECK(phyAddr);

                swRxDesc->rxDesc->nextDescPointer = CPSS_32BIT_LE((GT_U32)phyAddr);
            }
        }

        /* Close the cyclic desc. list. */
        swRxDesc->swNextDesc = firstSwRxDesc;
        rc = cpssOsVirt2Phy((GT_UINTPTR)firstRxDesc, &phyAddr);
        if (rc != GT_OK)
        {
            return rc;
        }

        CPSS_OS_PHY_ADDR_IN_32_BITS_CHECK(phyAddr);

        swRxDesc->rxDesc->nextDescPointer = CPSS_32BIT_LE((GT_U32)phyAddr);

        rxDescList[rxQueue].next2Receive    = firstSwRxDesc;
        rxDescList[rxQueue].next2Return     = firstSwRxDesc;
    }

    /* save Rx descriptor initial configuration parameters for later use after SW restart */
    rc  = prvCpssPpConfigDevDbPrvNetInfInfoSet(CAST_SW_DEVNUM(devNum), theFirstRxDesc, rxDescList);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Rx Buffers initialization.           */
    if (allocMethod == CPSS_RX_BUFF_STATIC_ALLOC_E ||
        allocMethod == CPSS_RX_BUFF_DYNAMIC_ALLOC_E)
    {
       if(allocMethod == CPSS_RX_BUFF_STATIC_ALLOC_E)
        {
            /* Set the buffers block to point to a properly alligned block. */
            if(((GT_UINTPTR)buffBlock % RX_BUFF_ALIGN) != 0)
            {
                buffBlockSize = (buffBlockSize -
                                 (RX_BUFF_ALIGN -
                                  ((GT_UINTPTR)buffBlock % RX_BUFF_ALIGN)));

                buffBlock =
                    (GT_U8*)((GT_UINTPTR)buffBlock +
                             (RX_BUFF_ALIGN - ((GT_UINTPTR)buffBlock % RX_BUFF_ALIGN)));
            }

            /* Check if the given buffers block, is large enough to be cut  */
            /* into the needed number of buffers.                           */
            if((buffBlockSize / (*numOfDescsPtr)) < RX_BUFF_SIZE_MULT)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        headerOffsetSize = headerOffset;
        if((headerOffsetSize % RX_BUFF_ALIGN) != 0)
        {
            headerOffsetSize += (RX_BUFF_ALIGN -(headerOffsetSize % RX_BUFF_ALIGN));
        }

        for(rxQueue = 0; rxQueue < NUM_OF_RX_QUEUES; rxQueue++)
        {
            swRxDesc = rxDescList[rxQueue].next2Receive;
            rxDescList[rxQueue].headerOffset = headerOffset;

            /* update the total number of buffers */
            *numOfBufsPtr += rxDescList[rxQueue].freeDescNum;

            for(i = 0; i < rxDescList[rxQueue].freeDescNum; i++)
            {
                RX_DESC_SET_BUFF_SIZE_FIELD_MAC(devNum,swRxDesc->rxDesc,
                                            (actualBuffSize - headerOffsetSize));

                /* Set the Rx desc. buff pointer field. */
                if(allocMethod == CPSS_RX_BUFF_STATIC_ALLOC_E)
                {
                    virtBuffAddr = ((GT_UINTPTR)buffBlock) + headerOffsetSize;
                    rc = cpssOsVirt2Phy(virtBuffAddr, &phyAddr);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }

                    CPSS_OS_PHY_ADDR_IN_32_BITS_CHECK(phyAddr);

                    swRxDesc->rxDesc->buffPointer = CPSS_32BIT_LE((GT_U32)phyAddr);

                    buffBlock = (GT_U8*)(((GT_UINTPTR)buffBlock) + actualBuffSize);

                    /* Set the buffers block to point to a properly alligned block*/
                    if(((GT_UINTPTR)buffBlock % RX_BUFF_ALIGN) != 0)
                    {
                        buffBlock =
                            (GT_U8*)((GT_UINTPTR)buffBlock +
                                     (RX_BUFF_ALIGN -
                                      ((GT_UINTPTR)buffBlock % RX_BUFF_ALIGN)));
                    }
                }
                else
                {
                    virtBuffAddr = (GT_UINTPTR)mallocFunc(actualBuffSize,RX_BUFF_ALIGN);
                    if(virtBuffAddr == 0)
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);

                    rc = cpssOsVirt2Phy(virtBuffAddr + headerOffsetSize, &phyAddr);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }

                    CPSS_OS_PHY_ADDR_IN_32_BITS_CHECK(phyAddr);

                    swRxDesc->rxDesc->buffPointer = CPSS_32BIT_LE((GT_U32)phyAddr);
                }

                swRxDesc = swRxDesc->swNextDesc;
            }
        }
    }
    else /* no buffer allocation*/
    {
        for(rxQueue = 0; rxQueue < NUM_OF_RX_QUEUES; rxQueue++)
        {
            rxDescList[rxQueue].freeDescNum = 0;
        }
    }


    /* read rxSDMA Queue Register */
    rc = prvCpssHwPpPortGroupReadRegister(CAST_SW_DEVNUM(devNum),portGroupId,regsAddr->MG.sdmaRegs.rxQCmdReg,&rxSdmaRegVal);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* fix of CQ 106607 : disable SDMA if it is enabled (during cpss restart)
       because if traffic is send to the CPU queues, then it will change the
       'link list' that we try to set to the PP during this function, that may
       cause asynchronous settings between the SW descriptors and the HW descriptors

       --> so we disable the queues at this stage and will enable right after the
       settings of the first descriptor into each queue.
    */
    if(rxSdmaRegVal & 0xFF)/* one of the queues enabled */
    {
        for(i = 0; i < NUM_OF_RX_QUEUES; i++)
        {
            /* for each queue set Enable bit to 0, and Disable bit to 1 */
            rxSdmaRegVal |= 1 << (i + NUM_OF_RX_QUEUES);
            rxSdmaRegVal &= (~(1 << i));

            /* write Rx SDMA Queues Reg - Disable Rx SDMA Queues */
            rc = prvCpssHwPpPortGroupWriteRegister(CAST_SW_DEVNUM(devNum),portGroupId,regsAddr->MG.sdmaRegs.rxQCmdReg,rxSdmaRegVal);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        /* wait for DMA operations to end on all queues */
        /* wait for bits 0xff to clear */
        rc = prvCpssPortGroupBusyWaitByMask(CAST_SW_DEVNUM(devNum),portGroupId,regsAddr->MG.sdmaRegs.rxQCmdReg,0xff,GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* read rxSDMA Queue Register */
        rc = prvCpssHwPpPortGroupReadRegister(CAST_SW_DEVNUM(devNum),portGroupId,regsAddr->MG.sdmaRegs.rxQCmdReg,&rxSdmaRegVal);
        if(rc != GT_OK)
        {
            return rc;
        }
    }


    for(i = 0; i < NUM_OF_RX_QUEUES; i++)
    {
        /* set the first descriptor (start of link list) to the specific queue */
        rc = cpssOsVirt2Phy((GT_UINTPTR)(rxDescList[i].next2Receive->rxDesc),  /*OUT*/&phyAddr);
        if (rc != GT_OK)
        {
            return rc;
        }

        CPSS_OS_PHY_ADDR_IN_32_BITS_CHECK(phyAddr);

        rc = prvCpssHwPpPortGroupWriteRegister(CAST_SW_DEVNUM(devNum),portGroupId,regsAddr->MG.sdmaRegs.rxDmaCdp[i],(GT_U32)phyAddr);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Set the Receive Interrupt Frame Boundaries   */
        rc = prvCpssHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum),portGroupId,regsAddr->MG.sdmaRegs.sdmaCfgReg,0,1,1);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* for each queue set Enable bit to 1, and Disable bit to 0.*/
        rxSdmaRegVal |= 1 << i;
        rxSdmaRegVal &= (~(1 << (i + NUM_OF_RX_QUEUES)));
    }
    /* Do not enable the queues in CPSS_RX_BUFF_NO_ALLOC_E mode. */
    /* Application responsibility to enable the queue after all buffers attached. */
    if (allocMethod != CPSS_RX_BUFF_NO_ALLOC_E)
    {
        /* if rxsdma init is going under recovery process as PP is disabled to write CPU memory, enable Rx SDMA queues just  */
        /* after PP would be able to access CPU memory(in catch up stage)- otherwise syncronization may be lost between      */
        /* PP and descriptors                                                                                                */
        rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (tempSystemRecovery_Info.systemRecoveryMode.haCpuMemoryAccessBlocked == GT_FALSE )
        {
            /* write Rx SDMA Queues Reg - Enable Rx SDMA Queues */
            rc = prvCpssHwPpPortGroupWriteRegister(CAST_SW_DEVNUM(devNum),portGroupId,regsAddr->MG.sdmaRegs.rxQCmdReg,rxSdmaRegVal);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    for ( i = 0 ; i < 8 ; i++ )
    {
        /* write Rx SDMA Resource Error Count and Mode - Retry, re-enable queue */
        rc = prvCpssHwPpPortGroupSetRegField(
                    CAST_SW_DEVNUM(devNum),
                    portGroupId,
                    regsAddr->MG.sdmaRegs.rxSdmaResourceErrorCountAndMode[i],
                    9, 1, 1);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}


/**
* @internal internal_cpssPxNetIfInit function
* @endinternal
*
* @brief   Initialize the network interface SDMA structures, Rx descriptors & buffers
*         and Tx descriptors.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device to initialize.
* @param[in] netIfCfgPtr              - (pointer to) the info about Rx/Tx descriptors,buffers,memories.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxNetIfInit
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  CPSS_NET_IF_CFG_STC   *netIfCfgPtr
)
{
    GT_STATUS          rc;
    GT_U32             numRxDataBufs;   /* These vars. are used to be   */
    GT_U32             numRxDescriptors;/* sent to Tx /Rx initialization*/
    GT_U32             numTxDescriptors;/* functions. and hold the      */
                                        /* number of the different descs*/
                                        /* and buffers allocated.       */
    CPSS_SYSTEM_RECOVERY_INFO_STC   oldSystemRecoveryInfo,newSystemRecoveryInfo;
    /* save current recovery state */
    rc = cpssSystemRecoveryStateGet(&oldSystemRecoveryInfo);
    if (rc != GT_OK)
    {
        return rc;
    }
    newSystemRecoveryInfo = oldSystemRecoveryInfo;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    /* SDMA mode */
    /* Check for valid configuration parameters */
    CPSS_NULL_PTR_CHECK_MAC(netIfCfgPtr->rxDescBlock);
    if((netIfCfgPtr->rxDescBlockSize == 0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "netIfCfgPtr->rxDescBlockSize must not be ZERO");
    }
    CPSS_NULL_PTR_CHECK_MAC(netIfCfgPtr->txDescBlock);
    if((netIfCfgPtr->txDescBlockSize == 0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "netIfCfgPtr->txDescBlockSize must not be ZERO");
    }

    if(netIfCfgPtr->rxBufInfo.allocMethod == CPSS_RX_BUFF_STATIC_ALLOC_E)
    {
        CPSS_NULL_PTR_CHECK_MAC(netIfCfgPtr->rxBufInfo.buffData.staticAlloc.rxBufBlockPtr);
        if((netIfCfgPtr->rxBufInfo.buffData.staticAlloc.rxBufBlockSize == 0))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "netIfCfgPtr->rxBufInfo.buffData.staticAlloc.rxBufBlockSize must not be ZERO");
        }
    }
    else
    if(netIfCfgPtr->rxBufInfo.allocMethod == CPSS_RX_BUFF_DYNAMIC_ALLOC_E)
    {
        CPSS_NULL_PTR_CHECK_MAC(netIfCfgPtr->rxBufInfo.buffData.dynamicAlloc.mallocFunc);
    }

    newSystemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);

    if ((oldSystemRecoveryInfo.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) &&
        (oldSystemRecoveryInfo.systemRecoveryMode.haCpuMemoryAccessBlocked == GT_TRUE) )
    {
        if (oldSystemRecoveryInfo.systemRecoveryMode.continuousRx == GT_FALSE)
        {
            /* HA event took place. Shut down DMA before init */
            rc =  internal_cpssPxNetIfRxStop(devNum);
            if (rc != GT_OK)
            {
                prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
    }

    if( (oldSystemRecoveryInfo.systemRecoveryState == CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E) ||
        ((oldSystemRecoveryInfo.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E)&&
         (oldSystemRecoveryInfo.systemRecoveryMode.continuousRx == GT_FALSE)) )
    {
        /* This point is reachable only in case of System Recovery rx disable mode or*/
        /* any other init action  not connected with System Recovery process. In both*/
        /* cases hw write is enable*/

        /* init RX */
        rc = sdmaRxInit(devNum,(GT_U8*)netIfCfgPtr->rxDescBlock,
                        netIfCfgPtr->rxDescBlockSize,
                        &(netIfCfgPtr->rxBufInfo),
                        &numRxDescriptors,&numRxDataBufs);
        if (GT_OK != rc)
        {
            prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    if( (oldSystemRecoveryInfo.systemRecoveryState == CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E) ||
        ((oldSystemRecoveryInfo.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E)&&
         (oldSystemRecoveryInfo.systemRecoveryMode.continuousTx == GT_FALSE)) )
    {
        /* This point is reachable only in case of System Recovery rx disable mode or*/
        /* any other init action  not connected with System Recovery process. In both*/
        /* cases hw write is enable*/

        /* init TX */
        rc = sdmaTxInit(devNum,(GT_U8*)netIfCfgPtr->txDescBlock,
                        netIfCfgPtr->txDescBlockSize,
                        &numTxDescriptors);
        if (GT_OK != rc)
        {
            prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
    /* !!! NO !!! bind the DSMA Tx completed callback to the driver */
    /*prvCpssDrvInterruptPpTxEndedCbBind(CAST_SW_DEVNUM(devNum),NULL);*/

    PRV_CPSS_PP_MAC(devNum)->cpuPortMode = CPSS_NET_CPU_PORT_MODE_SDMA_E;
    PRV_CPSS_PX_PP_MAC(devNum)->netIf.allocMethod = netIfCfgPtr->rxBufInfo.allocMethod;
    PRV_CPSS_PX_PP_MAC(devNum)->netIf.buffersInCachedMem = netIfCfgPtr->rxBufInfo.buffersInCachedMem;

    /* indicate that network interface init is done and parameters for SW restart are saved */
    rc = prvCpssPpConfigDevDbNetIfInitSet(CAST_SW_DEVNUM(devNum), GT_FALSE);
    return rc;
}

/**
* @internal cpssPxNetIfInit function
* @endinternal
*
* @brief   Initialize the network interface SDMA structures, Rx descriptors & buffers
*         and Tx descriptors.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device to initialize.
* @param[in] netIfCfgPtr              - (pointer to) the info about Rx/Tx descriptors,buffers,memories.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The application must call only one of the following APIs per device:
*       cpssPxNetIfMiiInit - for MII/RGMII Ethernet port networkIf initialization.
*       cpssPxNetIfInit - for SDMA networkIf initialization.
*       In case more than one of the mentioned above API is called
*       GT_FAIL will be return.
*       2. In case CPSS_RX_BUFF_NO_ALLOC_E is set, the application must enable
*       RX queues after attaching the buffers. See: cpssPxNetIfSdmaRxQueueEnable.
*       1.  The Driver allocates Tx descriptors, Rx Descriptors, and Rx data
*       buffers out of the respective blocks of memory provided by the user.
*       The number of allocated items is a function of the block size, item
*       structure size, and the HW alignment requirement for the specific
*       item (and the Rx Buffer headerOffset as defined in sysConfig.h)
*       2.  The Prestera PP PCI Host Interface directly updates the Rx/Tx
*       descriptors, Rx data buffers, and the Address Update Queue.
*       It is the user's responsibility to determine whether these memory
*       blocks (txDescBlock, rxDescBlock, rxbufBlock, AUQ) are from
*       cachable. non-cachable memory regions. For example if the
*       system supports cache-coherency between the CPU & the
*       PCI, those blocks can be in a cacheble area with no
*       special handling
*       If there is no cache coherency, then:
*       1) the Rx/Tx descriptors must be allocated from non-cachable
*       memory.
*       2) the Rx data buffers and AUQ may be from cachable memory,
*       if the user flushes the buffer memory cache prior to
*       returning the buffer to the Rx pool, and cache-line
*       reads do not cross data buffer boundaries.
*       3.  The number of Rx descriptors to be allocated per Rx queue will be
*       calculated as follows:
*       -  if <allocation method> = STATIC_ALLOC, then the number of
*       Rx descs. per Rx queue will be:
*       (rxBufBlockSize / rxBufSize) / <number of Rx queues>
*       while The Rx buffer size must be a multiple of 8.
*       In this case the user must take care to provide an Rx Desc.
*       block which is large enough to allocate an Rx desc. per
*       allocated Rx buffer.
*       -  if <allocation method> = DYNAMIC_ALLOC, then the number of
*       Rx descriptors per queue will be:
*       (rxDescBlockSize / <size of a single Rx desc.>) /
*       <number of Rx queues>.
*       The size of a single Rx descriptor can be obtained by
*       calling cpssPxHwDescSizeGet().
*       rxBufSize in the above calculations is received via TAPI
*       sysConfig.h sysConfig() API.
*       5.  The number of Tx descriptors to be allocated per Tx queue will be
*       calculated as follows:
*       (txDescBlockSize / <size of a single Tx desc.>) /
*       <number of Rx queues>.
*       The size of a single Tx descriptor can be obtained by calling
*       cpssPxHwDescSizeGet().
*
*/
GT_STATUS cpssPxNetIfInit
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  CPSS_NET_IF_CFG_STC   *netIfCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxNetIfInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum,netIfCfgPtr));

    rc = internal_cpssPxNetIfInit(devNum,netIfCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum,netIfCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}
/**
* @internal internal_cpssPxNetIfRxStop function
* @endinternal
*
* @brief   write Rx SDMA Queues Reg - Disable Rx SDMA Queues.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device that need to stop Rx SDMA.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPxNetIfRxStop
(
    IN  GT_SW_DEV_NUM   devNum
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    rxSdmaRegVal; /* Rx SDMA Queues Register value*/
    GT_U32    portGroupId;  /* port group Id for multi-port-groups device support */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    /* we will use the 'First active port group' , to represent the whole device.
       that way we allow application to give SDMA memory to single port group instead
       of split it between all active port groups
    */
    portGroupId = PRV_CPSS_NETIF_SDMA_PORT_GROUP_ID_MAC(devNum);

    /* Delete the Tx descriptors linked-list    */

    /* read Rx SDMA Queues Register. for each queue set disable */
    /* bit to 1, and enable bit to 0.                           */
    rc = prvCpssHwPpPortGroupReadRegister(CAST_SW_DEVNUM(devNum),portGroupId,
        PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.sdmaRegs.rxQCmdReg,&rxSdmaRegVal);

    if(rc != GT_OK)
    {
        return rc;
    }

    rxSdmaRegVal |= 0x0000FF00;/* set 8 bits */
    rxSdmaRegVal &= 0xFFFFFF00;/* clear 8 bits */

    /* write Rx SDMA Queues Reg - Disable Rx SDMA Queues */
    rc = prvCpssHwPpPortGroupWriteRegister(CAST_SW_DEVNUM(devNum),portGroupId,
        PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.sdmaRegs.rxQCmdReg,rxSdmaRegVal);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}



