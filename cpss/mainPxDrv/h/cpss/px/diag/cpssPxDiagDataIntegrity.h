/*******************************************************************************
*              (c), Copyright 2006, Marvell International Ltd.                 *
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
* @file cpssPxDiagDataIntegrity.h
*
* @brief Diag Data Integrity module APIs for CPSS Px.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssPxDiagDataIntegrityh
#define __cpssPxDiagDataIntegrityh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/px/cpssHwInit/cpssPxHwInit.h>
#include <cpss/generic/events/cpssGenEventRequests.h>
#include <cpss/common/diag/cpssCommonDiag.h>

/**
* @enum CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT
*
* @brief This enum defines Data Integrity module Memory types
*/
typedef enum{
  CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_MGCAM_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_RXDATA_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_RXHDR_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_SDMA_TX_FIFO_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_PEX2_TXDATA_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MG_CONFI_PROCESSOR_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MUNIT_MUNIT_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PPA_IMEM_BANK_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PPA_PPN_SCRATCHPAD_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PPA_SHARED_DMEM_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PHA_HA_TABLE_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_BC_CT_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_CLEAR_FIFO_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_FINAL_UDB_CLEAR_FIFO_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_FREE_BUFFERS_FIFO_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HA_DESC_IDDB_FREE_ID_FIFO_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HA_DESC_IDDB_MEM_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HEADER_REORDER_MEM_IDDB_MEM_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_INTERNAL_DESC_IDDB_FREE_ID_FIFO_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_INTERNAL_DESC_IDDB_MEM_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_CLEAR_SHIFTER_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_NEXT_CT_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_NEXT_TABLE_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_NUMBER_OF_BUFFERS_TABLE_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_PAYLOAD_REORDER_MEM_IDDB_MEM_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_PREF_MEM_ID_FIFO_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_BMX_DATA_BASE_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_DQ_Q_BUF_LIMIT_DP0_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_DQ_Q_DESC_LIMIT_DP0_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_EQ_Q_LIMIT_DP0_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_EQ_Q_LIMIT_DP12_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_BMX_FREE_ENTRY_FIFO_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_CPFC_IND_FIFO_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_LL_LINK_LIST_PTR_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_SAMPLE_INTERVALS_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_Q_SHARED_Q_LIMIT_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_RX_IDDB_MEM_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_TXDMA_RD_BURST_FIFO_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_CT_BC_IDDB_FREE_ID_FIFO_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_CT_BC_IDDB_MEM_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_DESCRIPTOR_MEM_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_HEADER_IDDB_MEM_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_HEADER_LL_FREE_BUFS_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_HEADER_LL_LINK_LIST_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_RXDMA_IBUF_HDR_IDDB_MEM_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_PAYLOAD_IDDB_MEM_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_PAYLOAD_LL_FREE_BUFS_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_PAYLOAD_LL_LINK_LIST_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_CG_CG_RX_FIFO_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_CG_CG_TX_FIFO_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_CG_DESKEW_MEM_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_CG_RS_FEC_MEM_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_CG_RX_PREAMBLE_MEM_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_CNC_IP_COUNTERS_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PCP_IP_DST_PORT_MAP_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_SIP_PCP_IP_SRC_PORT_MAP_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_DDU_FIFO_E
 ,CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TABLE_LAST_E

} CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT;
/**
* @enum CPSS_PX_LOCATION_ENT
*
* @brief A enum to hold location types
*/
typedef enum{

    /** hw tables */
    CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE,
    /** logical tables */
    CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE,
   /** ram */
    CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE

} CPSS_PX_LOCATION_ENT;

/**
* @struct CPSS_PX_HW_INDEX_INFO_STC
*
* @brief A struct to hold hw location information
*/
typedef struct{

    /** type of the hw table */
    CPSS_PX_TABLE_ENT hwTableType;

    /** entry index */
    GT_U32 hwTableEntryIndex;

} CPSS_PX_HW_INDEX_INFO_STC;

/**
* @struct CPSS_PX_RAM_INDEX_INFO_STC
*
* @brief A struct to hold ram location information
*/
typedef struct{

    /** enum of memory types that relate to the DFX RAM location */
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT memType;

    /** the row index in the DFX RAM */
    GT_U32                                   ramRow;

    /** info about the DFX RAM location */
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC memLocation;

} CPSS_PX_RAM_INDEX_INFO_STC;

/**
* @struct CPSS_PX_LOGICAL_TABLE_INFO_STC
*
* @brief A struct to hold logical table information
*/
typedef struct{

    /** enum of memory types that relate to the DFX RAM location */
    CPSS_PX_LOGICAL_TABLE_ENT logicalTableType;

    /** entry index */
    GT_U32                    logicalTableEntryIndex;


} CPSS_PX_LOGICAL_TABLE_INFO_STC;

/**
* @union CPSS_PX_LOCATION_UNT
*
* @brief A union to hold location information
*
*/

typedef union{
    /** logical info */
    CPSS_PX_LOGICAL_TABLE_INFO_STC logicalEntryInfo;
    /* hw info */
    CPSS_PX_HW_INDEX_INFO_STC      hwEntryInfo;
    /* ram info */
    CPSS_PX_RAM_INDEX_INFO_STC     ramEntryInfo;

} CPSS_PX_LOCATION_UNT;

/**
* @struct CPSS_PX_LOCATION_SPECIFIC_INFO_STC
*
* @brief A struct to hold location information
*/
typedef struct{
    /** type of the location */
    CPSS_PX_LOCATION_ENT type;

    /** union CPSS_PX_LOCATION_UNT */
    CPSS_PX_LOCATION_UNT info;

} CPSS_PX_LOCATION_SPECIFIC_INFO_STC;

/**
* @union CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_UNT
 *
 * @brief Holds the two possible memory location formats.
 *
*/

typedef union{
    /** regular memory location indexes */
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC memLocation;

    /** special memory location indexes for MPPM memory */
    CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC mppmMemLocation;

} CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT;




/**
* @struct CPSS_PX_LOCATION_FULL_INFO_STC
*
* @brief A struct to hold location information
*/
typedef struct{

    /** logical info */
    CPSS_PX_LOGICAL_TABLE_INFO_STC logicalEntryInfo;

    /** hw info */
    CPSS_PX_HW_INDEX_INFO_STC hwEntryInfo;

    /** ram info */
    CPSS_PX_RAM_INDEX_INFO_STC ramEntryInfo;

    /** if use mppmMemLocation instead of ramEntryInfo.memLocation. */
    GT_BOOL isMppmInfoValid;

    /** MPPM memory location indexws */
    CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC mppmMemLocation;

} CPSS_PX_LOCATION_FULL_INFO_STC;

/**
* @struct CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC
*
* @brief Data Integrity Event structure
*/
typedef struct{

    /** event type */
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT eventsType;

    /** memory location */
    CPSS_PX_LOCATION_FULL_INFO_STC location;

    /** memory category */
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_ENT memoryUseType;

    /** @brief error correction method
     *  Comments:
     *  None
     */
    CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_ENT correctionMethod;

} CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC;

/**
* @internal cpssPxDiagDataIntegrityEventsGet function
* @endinternal
*
* @brief   Function returns array of data integrity events.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] evExtData                - event external data
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - array of ECC/parity events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*                                      GT_TRUE - no more events found
*                                      GT_FALSE - there are more events found
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagDataIntegrityEventsGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_U32                                      evExtData,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC       eventsArr[], /*arrSizeVarName=eventsNumPtr*/
    OUT GT_BOOL                                     *isNoMoreEventsPtr
);

/**
* @internal cpssPxDiagDataIntegrityEventMaskSet function
* @endinternal
*
* @brief   Function sets/clears mask for ECC/Parity
*         event. Event is masked/unmasked for whole hw table or
*         for all memories of specified type. RAM DFX
*         coordinates - pipeId, clientId, memoryId - and an entry index
*         info - ramRow, hwTableEntryIndex - are ignored.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
* @param[in] errorType                - type of error interrupt
*                                      relevant only for ECC protected memories
* @param[in] operation                - mask/unmask interrupt
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, errorType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - if memEntry exists but doesn't support
*                                       event masking
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
GT_STATUS cpssPxDiagDataIntegrityEventMaskSet
(
    IN  GT_SW_DEV_NUM                                 devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                       operation
);


/**
* @internal cpssPxDiagDataIntegrityEventMaskGet function
* @endinternal
*
* @brief   Function gets gets per table or per memory ECC/Parity
*          interrupt mask.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
*
* @param[out] errorTypePtr             - (pointer to) type of error interrupt
* @param[out] operationPtr             - (pointer to) mask/unmask interrupt
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, memEntry, errorType
* @retval GT_NOT_SUPPORTED         - if memEntry doesn't support masking/unmasking
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
GT_STATUS cpssPxDiagDataIntegrityEventMaskGet
(
    IN  GT_SW_DEV_NUM                                 devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC              *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
);


/**
* @internal cpssPxDiagDataIntegrityErrorInfoGet function
* @endinternal
*
* @brief   Function gets PER WHOLE HW TABLE OR
*         MEMORY ECC/Parity error details.An entry line number
*         information - ramRow, hwTableEntryIndex
*         - are ignored. A failed
*         row/segment/syndrome info is available for RAM
*         location type only.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
* @param[in] mppmMemLocationPtr       - MPPM memory coordinates. For memories other
*                                      than MPPM should be null!
*
* @param[out] errorCounterPtr          - (pointer to) error counter
* @param[out] failedRowPtr             - (pointer to) failed raw
* @param[out] failedSegmentPtr         - (pointer to) failed segment
* @param[out] failedSyndromePtr        - (pointer to) failed syndrome
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, memEntry
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Error status or counter that isn't supported returns 0xFFFFFFFF
*
*/
GT_STATUS cpssPxDiagDataIntegrityErrorInfoGet
(
    IN  GT_SW_DEV_NUM                                     devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC                *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC *mppmMemLocationPtr,
    OUT GT_U32                                            *errorCounterPtr,
    OUT GT_U32                                            *failedRowPtr,
    OUT GT_U32                                            *failedSegmentPtr,
    OUT GT_U32                                            *failedSyndromePtr
);


/**
* @internal cpssPxDiagDataIntegrityErrorInjectionConfigSet function
* @endinternal
*
* @brief   Function enables/disables injection of error during next write operation.
*         Injection is applied to whole hw table or to all
*         memories of specified type. RAM DFX coordinates -
*         pipeId, clientId, memoryId - and an entry index info -
*         ramRow, hwTableEntryIndex - are ignored.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location.
* @param[in] injectMode               - error injection mode
*                                      relevant only for ECC protected memories
* @param[in] injectEnable             - enable/disable error injection
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, injectMode
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*
* @note Memory with parity can not be configured with multiple error injection
*
*/
GT_STATUS cpssPxDiagDataIntegrityErrorInjectionConfigSet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC              *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  injectMode,
    IN  GT_BOOL                                         injectEnable
);


/**
* @internal cpssPxDiagDataIntegrityErrorInjectionConfigGet function
* @endinternal
*
* @brief   Function gets status of error injection.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
*
* @param[out] injectModePtr            - (pointer to) error injection mode
* @param[out] injectEnablePtr          - (pointer to) enable/disable error injection
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, memEntry
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
GT_STATUS cpssPxDiagDataIntegrityErrorInjectionConfigGet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC              *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  *injectModePtr,
    OUT GT_BOOL                                         *injectEnablePtr
);


/**
* @internal cpssPxDiagDataIntegrityErrorCountEnableSet function
* @endinternal
*
* @brief   Function enables/disable error counter. Enabling/disabling is performed
*         for whole hw table or for all memories of specified
*         type. RAM DFX coordinates - pipeId, clientId, memoryId
*         - and an entry index info - ramRow,
*         hwTableEntryIndex - are ignored.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
* @param[in] errorType                - error type
* @param[in] countEnable              - enable/disable error counter
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, errorType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not suppoted memory type
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
GT_STATUS cpssPxDiagDataIntegrityErrorCountEnableSet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC              *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  GT_BOOL                                         countEnable
);


/**
* @internal cpssPxDiagDataIntegrityErrorCountEnableGet function
* @endinternal
*
* @brief   Function gets status of error counter. Status getting is performed
*         to per whole hw table or memory. An entry line number
*         - ramRow, hwTableEntryIndex - are ignored.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
*
* @param[out] errorTypePtr             - (pointer to) error type
* @param[out] countEnablePtr           - (pointer to) status of error counter
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, errorType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagDataIntegrityErrorCountEnableGet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC              *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT GT_BOOL                                         *countEnablePtr
);


/**
* @internal cpssPxDiagDataIntegrityProtectionTypeGet function
* @endinternal
*
* @brief   Function gets memory protection type.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
*
* @param[out] protectionTypePtr        - (pointer to) memory protection type
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, memEntry
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
GT_STATUS cpssPxDiagDataIntegrityProtectionTypeGet
(
    IN  GT_SW_DEV_NUM                                           devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC                      *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  *protectionTypePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxDiagDataIntegrityh */
