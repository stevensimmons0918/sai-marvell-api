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
* @file cpssPxCnc.h
*
* @brief CPSS PX Centralized Counters (CNC) API.
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPxCnch
#define __cpssPxCnch

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* get public types */
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/px/cpssHwInit/cpssPxHwInit.h>

/**
* @enum CPSS_PX_CNC_CLIENT_ENT
 *
 * @brief CNC clients.
*/
typedef enum{

    /** Ingress processing client */
    CPSS_PX_CNC_CLIENT_INGRESS_PROCESSING_E,

    /** Egress processing programmable client */
    CPSS_PX_CNC_CLIENT_EGRESS_PROCESSING_E,

    /** Egress queue pass/tail-drop and QCN client */
    CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E

} CPSS_PX_CNC_CLIENT_ENT;

/**
* @enum CPSS_PX_CNC_COUNTER_FORMAT_ENT
 *
 * @brief CNC modes of counter formats.
*/
typedef enum{

    /** @brief PKT_29_BC_35;
     *  Partitioning of the 64 bits entry is as following:
     *  Packets counter: 29 bits, Byte Count counter: 35 bits
     */
    CPSS_PX_CNC_COUNTER_FORMAT_MODE_0_E,

    /** @brief PKT_27_BC_37;
     *  Partitioning of the 64 bits entry is as following:
     *  Packets counter: 27 bits, Byte Count counter: 37 bits
     */
    CPSS_PX_CNC_COUNTER_FORMAT_MODE_1_E,

    /** @brief PKT_37_BC_27;
     *  Partitioning of the 64 bits entry is as following:
     *  Packets counter: 37 bits, Byte Count counter: 27 bits
     */
    CPSS_PX_CNC_COUNTER_FORMAT_MODE_2_E,

    /** @brief PKT_64_BC_0;
     *  Partitioning of the 64 bits entry is as following:
     *  Packets counter: 64 bits, Byte Count counter: 0 bits
     */
    CPSS_PX_CNC_COUNTER_FORMAT_MODE_3_E,

    /** @brief PKT_0_BC_64
     *  Partitioning of the 64 bits entry is as following:
     *  Packets counter: 0 bits, Byte Count counter: 64 bits
     */
    CPSS_PX_CNC_COUNTER_FORMAT_MODE_4_E

} CPSS_PX_CNC_COUNTER_FORMAT_ENT;

/**
* @struct CPSS_PX_CNC_COUNTER_STC
 *
 * @brief The counter entry contents.
*/
typedef struct{

    /** byte count */
    GT_U64 byteCount;

    /** @brief packets count
     *  Comment:
     *  See the possible counter HW formats in CPSS_PX_CNC_COUNTER_FORMAT_ENT
     *  description.
     */
    GT_U64 packetCount;

} CPSS_PX_CNC_COUNTER_STC;

/**
* @enum CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT
 *
 * @brief CNC modes of Egress Queue counting.
*/
typedef enum{

    /** count egress queue pass and taildropped packets. */
    CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_E,

    /** count Congestion Notification messages. */
    CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_CN_E,

    /** count egress queue pass and taildropped packets with unaware DP. */
    CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E

} CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT;

/**
* @enum CPSS_PX_CNC_BYTE_COUNT_MODE_ENT
 *
 * @brief CNC modes for byte count counters.
*/
typedef enum{

    /** @brief The Byte Count counter counts the
     *  entire packet byte count for all packet type.
     */
    CPSS_PX_CNC_BYTE_COUNT_MODE_L2_E,

    /** @brief Byte Count counters counts the
     *  packet L3 fields (the entire packet
     *  minus the L3 offset).
     */
    CPSS_PX_CNC_BYTE_COUNT_MODE_L3_E

} CPSS_PX_CNC_BYTE_COUNT_MODE_ENT;

/**
* @internal cpssPxCncUploadInit function
* @endinternal
*
* @brief   Initialize the CNC upload configuration, including DMA queue. Should be
*         used once before first cpssPxCncBlockUploadTrigger call.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device to initialize.
* @param[in] cncCfgPtr                - (pointer to) DMA queue configuration.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - invalid cncCfgPtr.
* @retval GT_OUT_OF_RANGE          - bad address
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssPxCncUploadInit
(
    IN GT_SW_DEV_NUM          devNum,
    IN CPSS_DMA_QUEUE_CFG_STC *cncCfgPtr
);

/**
* @internal cpssPxCncBlockClientEnableSet function
* @endinternal
*
* @brief   The function binds/unbinds the selected client to/from a counter block.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
* @param[in] client                   - CNC client
* @param[in] updateEnable             - the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncBlockClientEnableSet
(
    IN  GT_SW_DEV_NUM             devNum,
    IN  GT_U32                    blockNum,
    IN  CPSS_PX_CNC_CLIENT_ENT    client,
    IN  GT_BOOL                   updateEnable
);

/**
* @internal cpssPxCncBlockClientEnableGet function
* @endinternal
*
* @brief   The function gets bind/unbind of the selected client to a counter block.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
* @param[in] client                   - CNC client.
*
* @param[out] updateEnablePtr          - (pointer to) the client enable to update the block
*                                      GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncBlockClientEnableGet
(
    IN  GT_SW_DEV_NUM             devNum,
    IN  GT_U32                    blockNum,
    IN  CPSS_PX_CNC_CLIENT_ENT    client,
    OUT GT_BOOL                   *updateEnablePtr
);

/**
* @internal cpssPxCncBlockClientRangesSet function
* @endinternal
*
* @brief   The function sets index ranges per CNC client and Block
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
* @param[in] indexRangesBmp           - the counter index ranges bitmap
*                                      There are 64 ranges of counter indexes, each 1024
*                                      indexes i.e. 0..1K-1, 1K..2K-1, ,63K..64K-1.
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (1024 counters).
*                                      Allowed to map more than one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncBlockClientRangesSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          blockNum,
    IN  GT_U64          indexRangesBmp
);

/**
* @internal cpssPxCncBlockClientRangesGet function
* @endinternal
*
* @brief   The function gets index ranges per CNC client and Block
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
*
* @param[out] indexRangesBmpPtr        - (pointer to) the counter index ranges bitmap
*                                      Pipe has 10 ranges of counter indexes, each 1024
*                                      indexes i.e. 0..1K-1, 1K..2K-1, ,63K..64K-1.
*                                      the n-th bit 1 value maps the n-th index range
*                                      to the block (1024 counters).
*                                      Allowed to map more than one range to the block
*                                      but it will cause updating the same counters via
*                                      different indexes.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncBlockClientRangesGet
(
    IN  GT_SW_DEV_NUM             devNum,
    IN  GT_U32                    blockNum,
    OUT GT_U64                    *indexRangesBmpPtr
);

/**
* @internal cpssPxCncCounterFormatSet function
* @endinternal
*
* @brief   The function sets format of CNC counter
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
* @param[in] format                   - CNC counter format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncCounterFormatSet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  GT_U32                            blockNum,
    IN  CPSS_PX_CNC_COUNTER_FORMAT_ENT    format
);

/**
* @internal cpssPxCncCounterFormatGet function
* @endinternal
*
* @brief   The function gets format of CNC counter
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
*
* @param[out] formatPtr                - (pointer to) CNC counter format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_STATE             - on wrong counting mode read
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       on not supported client for device.
*/
GT_STATUS cpssPxCncCounterFormatGet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  GT_U32                            blockNum,
    OUT CPSS_PX_CNC_COUNTER_FORMAT_ENT    *formatPtr
);

/**
* @internal cpssPxCncCounterClearByReadEnableSet function
* @endinternal
*
* @brief   The function enable/disables clear by read mode of CNC counters read
*         operation.
*         If clear by read mode is disable the counters after read
*         keep the current value and continue to count normally.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncCounterClearByReadEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
);

/**
* @internal cpssPxCncCounterClearByReadEnableGet function
* @endinternal
*
* @brief   The function gets clear by read mode status of CNC counters read
*         operation.
*         If clear by read mode is disable the counters after read
*         keep the current value and continue to count normally.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncCounterClearByReadEnableGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_BOOL          *enablePtr
);

/**
* @internal cpssPxCncCounterClearByReadValueSet function
* @endinternal
*
* @brief   The function sets the counter clear by read globally configured value.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] format                   - CNC counter HW format.
* @param[in] counterPtr               - (pointer to) counter contents.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncCounterClearByReadValueSet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  CPSS_PX_CNC_COUNTER_FORMAT_ENT    format,
    IN  CPSS_PX_CNC_COUNTER_STC           *counterPtr
);

/**
* @internal cpssPxCncCounterClearByReadValueGet function
* @endinternal
*
* @brief   The function gets the counter clear by read globally configured value.
*         If clear by read mode is enable the counters load a globally configured
*         value instead of the current value and continue to count normally.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] format                   - CNC counter HW format.
*
* @param[out] counterPtr               - (pointer to) counter contents
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncCounterClearByReadValueGet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  CPSS_PX_CNC_COUNTER_FORMAT_ENT    format,
    OUT CPSS_PX_CNC_COUNTER_STC           *counterPtr
);

/**
* @internal cpssPxCncBlockUploadTrigger function
* @endinternal
*
* @brief   The function triggers the Upload of the given counters block.
*         In order to initialize storage needed for uploaded counters,
*         cpssPxCncUploadInit must be called first.
*         An application may check that CNC upload finished by
*         cpssPxCncBlockUploadInProcessGet.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_INITIALIZED       - cpssPxCncUploadInit not called
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - if the previous CNC upload in process.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncBlockUploadTrigger
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      blockNum
);

/**
* @internal cpssPxCncBlockUploadInProcessGet function
* @endinternal
*
* @brief   The function gets bitmap of numbers of such counters blocks that upload
*         of them yet in process. The HW cannot keep more than one block in such
*         state, but an API matches the original HW representation of the state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] inProcessBlocksBmpPtr    - (pointer to) bitmap of in-process blocks
*                                      value 1 of the bit#n means that CNC upload
*                                      is not finished yet for block n
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncBlockUploadInProcessGet
(
    IN  GT_SW_DEV_NUM               devNum,
    OUT GT_U32                      *inProcessBlocksBmpPtr
);

/**
* @internal cpssPxCncUploadedBlockGet function
* @endinternal
*
* @brief   The function return a block (array) of CNC counter values,
*         the maximal number of elements defined by the caller.
*         The CNC upload may triggered by cpssPxCncBlockUploadTrigger.
*         The CNC upload transfers whole CNC block (1K CNC counters)
*         to address upload queue. An application must get all transferred
*         counters. An application may sequentially upload several CNC blocks
*         before start to retrieve uploaded counters.
*         The device may transfer only part of CNC block because of DMA queue
*         full. In this case the cpssPxCncUploadedBlockGet may return only part of
*         the CNC block with return GT_OK. An application must to call
*         cpssPxCncUploadedBlockGet one more time to get rest of the block.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in,out] numOfCounterValuesPtr    - (pointer to) maximal number of CNC counters
*                                      values to get.This is the size of
* @param[in] counterValuesPtr         array allocated by caller.
* @param[in] format                   - CNC counter HW format.
* @param[in,out] numOfCounterValuesPtr    - (pointer to) actual number of CNC
*                                      counters values in counterValuesPtr.
*
* @param[out] counterValuesPtr         - (pointer to) array that holds received CNC
*                                      counters values. Array must be allocated by
*                                      caller.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more
*                                       waiting
*                                       CNC counter value
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                       continued after queue rewind but yet
*                                       not paused due to queue full and yet not finished.
*                                       The part of queue memory does not contain uploaded counters yet.
*                                       No counters retrieved.
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_SUPPORTED         - block upload operation not supported
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncUploadedBlockGet
(
    IN     GT_SW_DEV_NUM                     devNum,
    INOUT  GT_U32                            *numOfCounterValuesPtr,
    IN     CPSS_PX_CNC_COUNTER_FORMAT_ENT    format,
    OUT    CPSS_PX_CNC_COUNTER_STC           *counterValuesPtr
);

/**
* @internal cpssPxCncCounterWraparoundEnableSet function
* @endinternal
*
* @brief   The function enables/disables wraparound for all CNC counters
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncCounterWraparoundEnableSet
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_BOOL          enable
);

/**
* @internal cpssPxCncCounterWraparoundEnableGet function
* @endinternal
*
* @brief   The function gets status of wraparound for all CNC counters
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) enable
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncCounterWraparoundEnableGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_BOOL          *enablePtr
);

/**
* @internal cpssPxCncCounterWraparoundIndexesGet function
* @endinternal
*
* @brief   The function gets the counter Wrap Around last 8 indexes
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
* @param[in,out] indexNumPtr              - (pointer to) maximal size of array of indexes.
* @param[in,out] indexNumPtr              - (pointer to) actual size of array of indexes.
*
* @param[out] indexesArr[]             - (pointer to) array of indexes of counters wrapped around.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The wraparound indexes are cleared on read.
*       Since in the HW each pair of wraparound indexes resides in one register,
*       cleared by read, in case not all the 8 possible indexes are read at once,
*       some data may be lost.
*       Recommendation: Always request 8 indexes => indexNumPtr=8 as input.
*
*/
GT_STATUS cpssPxCncCounterWraparoundIndexesGet
(
    IN    GT_SW_DEV_NUM    devNum,
    IN    GT_U32           blockNum,
    INOUT GT_U32           *indexNumPtr,
    OUT   GT_U32           indexesArr[] /*arrSizeVarName=indexNumPtr*/
);

/**
* @internal cpssPxCncCounterGet function
* @endinternal
*
* @brief   The function gets the counter contents
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] blockNum                 - CNC block number (APPLICABLE RANGES: 0..1).
* @param[in] index                    - counter  in the block
*                                      (APPLICABLE RANGES: 0..1023).
* @param[in] format                   - CNC counter HW format.
*
* @param[out] counterPtr               - (pointer to) received CNC counter value.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncCounterGet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  GT_U32                            blockNum,
    IN  GT_U32                            index,
    IN  CPSS_PX_CNC_COUNTER_FORMAT_ENT    format,
    OUT CPSS_PX_CNC_COUNTER_STC           *counterPtr
);

/**
* @internal cpssPxCncEgressQueueClientModeSet function
* @endinternal
*
* @brief   The function sets Egress Queue client counting mode
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mode                     - Egress Queue client counting mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncEgressQueueClientModeSet
(
    IN  GT_SW_DEV_NUM                              devNum,
    IN  CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT   mode
);

/**
* @internal cpssPxCncEgressQueueClientModeGet function
* @endinternal
*
* @brief   The function gets Egress Queue client counting mode
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] modePtr                  - (pointer to) Egress Queue client counting mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncEgressQueueClientModeGet
(
    IN  GT_SW_DEV_NUM                              devNum,
    OUT CPSS_PX_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT   *modePtr
);

/**
* @internal cpssPxCncClientByteCountModeSet function
* @endinternal
*
* @brief   The function sets byte count mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] client                   - CNC  (APPLICABLE VALUES: CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E).
* @param[in] mode                     - byte count mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncClientByteCountModeSet
(
    IN GT_SW_DEV_NUM                     devNum,
    IN CPSS_PX_CNC_CLIENT_ENT            client,
    IN CPSS_PX_CNC_BYTE_COUNT_MODE_ENT   mode
);

/**
* @internal cpssPxCncClientByteCountModeGet function
* @endinternal
*
* @brief   The function gets byte count mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] client                   - CNC  (APPLICABLE VALUES: CPSS_PX_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_QCN_E).
*
* @param[out] modePtr                  - (pointer to) byte count mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxCncClientByteCountModeGet
(
    IN  GT_SW_DEV_NUM                     devNum,
    IN  CPSS_PX_CNC_CLIENT_ENT            client,
    OUT CPSS_PX_CNC_BYTE_COUNT_MODE_ENT   *modePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxCnch */

