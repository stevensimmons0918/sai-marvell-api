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
* @file cpssPxNetIf.h
*
* @brief Include PX network interface API functions
*
*
* @version   1
********************************************************************************
*/
#ifndef __cpssPxNetIf_h
#define __cpssPxNetIf_h

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/px/networkIf/cpssPxNetIfTypes.h>

#if defined CPSS_TRAFFIC_API_LOCK_DISABLE
#define CPSS_TRAFFIC_API_LOCK_MAC(_devNum,_functionality)
#define CPSS_TRAFFIC_API_UNLOCK_MAC(_devNum,_functionality)

#else
#define CPSS_TRAFFIC_API_LOCK_MAC(_devNum,_functionality) CPSS_API_LOCK_MAC(_devNum,_functionality)
#define CPSS_TRAFFIC_API_UNLOCK_MAC(_devNum,_functionality) CPSS_API_UNLOCK_MAC(_devNum,_functionality)

#endif

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
);

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
    IN GT_U8    *rxBuffList[], /*arrSizeVarName=buffListLen*/
    IN GT_U32   buffListLen
);

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
    IN GT_U8    *rxBuffList[], /*arrSizeVarName=buffListLen*/
    IN GT_U32   rxBuffSizeList[], /*arrSizeVarName=buffListLen*/
    IN GT_U32   buffListLen
);

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
    OUT GT_U8                               *packetBuffsArrPtr[], /*arrSizeVarName=numOfBuffPtr*/
    OUT GT_U32                              buffLenArr[] /*arrSizeVarName=numOfBuffPtr*/
);

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
);

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
);

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
);

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
);

/**
* @internal cpssPxNetIfSdmaRxResourceErrorModeSet function
* @endinternal
*
* @brief   Set a bit per TC queue which defines the behavior in case of
*         RX resource error
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] queue                    - traffic class  (APPLICABLE RANGES: 0..7)
* @param[in] mode                     - current packet mode: retry to send or abort
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number, queue, mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the request is not supported
*/
GT_STATUS cpssPxNetIfSdmaRxResourceErrorModeSet
(
    IN  GT_SW_DEV_NUM                devNum,
    IN  GT_U32                       queue,
    IN  CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT   mode
);

/**
* @internal cpssPxNetIfSdmaRxResourceErrorModeGet function
* @endinternal
*
* @brief   Get a bit per TC queue which defines the behavior in case of
*         RX resource error
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] queue                    - traffic class  (APPLICABLE RANGES: 0..7)
*
* @param[out] modePtr                  - current packet mode: retry to send or abort
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number, queue, mode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - the request is not supported
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssPxNetIfSdmaRxResourceErrorModeGet
(
    IN  GT_SW_DEV_NUM                devNum,
    IN  GT_U32                       queue,
    OUT CPSS_PX_NET_RESOURCE_ERROR_MODE_ENT   *modePtr
);


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
    IN GT_U8                                    *buffList[], /*arrSizeVarName=numOfBufs*/
    IN GT_U32                                   buffLenList[], /*arrSizeVarName=numOfBufs*/
    IN GT_U32                                   numOfBufs
);


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
);

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
* @param[in] queue                    - traffic class queue
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
    IN  GT_U32               queue,
    OUT GT_BOOL              *enablePtr
);

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
);

#ifdef __cplusplus
}
#endif

#endif  /* __cpssPxNetIf_h */


