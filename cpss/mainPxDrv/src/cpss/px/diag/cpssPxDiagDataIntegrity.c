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
* @file cpssPxDiagDataIntegrity.c
*
* @brief CPSS PX Diagnostic Data Integrity API
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/diag/cpssPxDiagDataIntegrity.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/diag/private/prvCpssPxDiagDataIntegrityMainMappingDb.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxHwTables.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvExMxEventsPxPipe.h>
#include <cpss/px/diag/cpssPxDiagDataIntegrityTables.h>
#include <cpss/px/diag/private/prvCpssPxDiagLog.h>
#include <cpss/common/diag/private/prvCpssCommonDiag.h>
#include <cpss/px/diag/private/prvCpssPxDiag.h>

#define PRV_CPSS_PX_DATA_INTEGRITY_DFX_CLIENTS_BIT_MAP 0x22FF; /* clients 0-7,9,13*/


#define PRV_CPSS_PX_DIAG_DATA_INTEGRITY_CLIENTS_COUNT_CNS 28
/* macro to construct DataIntegrity DB item's key value from DFX pipe, clien, memory
 * values. DB item is PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC */
#define PRV_CPSS_PX_DIAG_DATA_INTEGRITY_GENERATE_KEY_MAC(pipeId, clientId, memId) \
          ((pipeId) << 12) | ((clientId) << 7) | (memId)

/* constant not matching any existent hw table (CPSS_PX_TABLE_ENT or PRV_CPSS_PX_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT)*/
#define PRV_CPSS_PX_DIAG_DATA_INTEGRITY_NONEXISTENT_HW_TABLE_CNS LAST_VALID_PX_TABLE_CNS

/* constant not matching any existent memory type */
#define PRV_CPSS_PX_DIAG_DATA_INTEGRITY_NONEXISTENT_MEM_TYPE_CNS ((GT_U32)-1)

/* value that should be treated as unknown */
#define PRV_CPSS_PX_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS ((GT_U32)-1)

/**
* @struct PRV_CPSS_PX_DIAG_DATA_INTEGRITY_CLIENT_STATUS_STC
*
* @brief Bitmap of memories errors in DFX Client
*/
typedef struct{

    GT_U32 bitmap[4];

} PRV_CPSS_PX_DIAG_DATA_INTEGRITY_CLIENT_STATUS_STC;




static GT_STATUS prvCpssPxDiagDataIntegrityErrorInfoGet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT        memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT *locationPtr,
    OUT GT_U32                                          *errorCounterPtr,
    OUT GT_U32                                          *failedRowPtr,
    OUT GT_U32                                          *failedSegmentPtr,
    OUT GT_U32                                          *failedSyndromePtr
);


GT_STATUS prvCpssPxDiagDataIntegrityTableHwMaxIndexGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PX_TABLE_ENT                   hwTable,
    OUT GT_U32                              *maxNumEntriesPtr
);

GT_STATUS prvCpssPxDiagDataIntegrityDfxErrorConvert
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC    *memLocationPtr,
    IN  GT_U32                                          failedRow,
    OUT CPSS_PX_HW_INDEX_INFO_STC                     *hwErrorInfoPtr
);

GT_BOOL prvCpssPxTableEngineMultiEntriesInLineIndexAndGlobalBitConvert
(
    IN GT_SW_DEV_NUM    devNum,
    IN CPSS_PX_TABLE_ENT     tableType,
    INOUT GT_U32               *entryIndexPtr,
    INOUT GT_U32               *fieldOffsetPtr,
    OUT GT_U32                 *numBitsPerEntryPtr,
    OUT GT_U32                 *numEntriesPerLinePtr
);

static GT_STATUS prvCpssPxDiagDfxMemoryEventFromDbGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_U32                                      pipeId,
    IN  GT_U32                                      clientId,
    IN  GT_U32                                      memId,
    OUT CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC     *eventPtr
);
/**
* @internal prvCpssPxDiagDataIntegrityHwTableCheck function
* @endinternal
*
* @brief   Function finds the table in Data Integrity Data Base
*
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] tableType                - table type
*
* @param[out] memTypePtr               - (pointer to) memory type
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - on not supported tableType
* @retval GT_NOT_FOUND             - on not found tableType in RAM DB
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - on other error
*/
GT_STATUS prvCpssPxDiagDataIntegrityHwTableCheck
(
    IN GT_U8   devNum,
    IN CPSS_PX_TABLE_ENT tableType,
    OUT CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      *memTypePtr
)
{
    GT_U32 i;
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr;
    GT_U32   dbArrayEntryNum;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(memTypePtr);

    if((GT_U32)tableType >=  CPSS_PX_HW_TABLE_LAST_E) /* all HW tables including internal*/
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* assign DB pointer and size */
    prvCpssPxDiagDataIntegrityDbPointerSet(&dbArrayPtr, &dbArrayEntryNum);
    if ((dbArrayPtr == NULL) || (dbArrayEntryNum == 0))
    {
        /* should not happen */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* loop over all memories in DB */
    for(i = 0; i < dbArrayEntryNum; i++)
    {
        if(dbArrayPtr[i].hwTableName == (GT_U32)tableType)
        {
            *memTypePtr = dbArrayPtr[i].memType;
            return GT_OK;
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssPxDiagDataIntegrityEccInterruptsMask function
* @endinternal
*
* @brief   Mask/Unmask (i.e disable/enable) interrupt(s) responsible
*         for specified ECC error types signalling.
*         IMPORTANT!
*         There is no protection against multiple simultaneous interrupt registers
*         access. Calling code is responsible for locking/unlocking!
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] errorType                - ECC error type: <single> / <multiple> / <single+multiple>
* @param[in] maskEn                   - mask (GT_TRUE) or unmask (GT_FALSE) interrupt(s)
* @param[in] intEccSingle             - interrupt number raised in case of single ECC error
* @param[in] intEccDouble             - interrupt number raised in case of double ECC error
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad param
*
* @note Calling code is responsible to locking/unlocking!
*
*/
static GT_STATUS prvCpssPxDiagDataIntegrityEccInterruptsMask
(
    IN GT_U8                                         devNum,
    IN CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT errorType,
    IN GT_BOOL                                       maskEn,
    IN GT_U32                                        intEccSingle,
    IN GT_U32                                        intEccDouble
)
{
    GT_STATUS rc = GT_OK;
    switch (errorType) {
        case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E:
            rc = prvCpssDrvEventsMask(devNum, intEccSingle, maskEn);
            if (GT_OK == rc)
            {
                rc = prvCpssDrvEventsMask(devNum, intEccDouble, maskEn);
            }
            break;
        case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E:
            rc = prvCpssDrvEventsMask(devNum, intEccDouble, maskEn);
            break;
        case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E:
            rc = prvCpssDrvEventsMask(devNum, intEccSingle, maskEn);
            break;
        default:
            rc = GT_BAD_PARAM;
    }
    return rc;
}



/**
* @internal
*           prvCpssPxDiagDataIntegrityExtMemoryProtectionEventMaskSet
*           function
* @endinternal
*
* @brief   Function configures mask/unmask for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
* @param[in] errorType                - error type
* @param[in] operation                - mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssPxDiagDataIntegrityExtMemoryProtectionEventMaskSet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT        memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
)
{
    GT_STATUS   rc = GT_OK; /* return code */
    GT_BOOL     intMaskEn; /* if interrupt should be masked or unmasked */
    GT_U32      intIndex; /* HW interrupt index */

    /* Unused parameter.
     * errorType is used only with memories with not-DFX ECC protection . */
    (void)errorType;


    intMaskEn = (operation == CPSS_EVENT_UNMASK_E) ? GT_FALSE : GT_TRUE;

    switch (memType)
    {
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_HEADER_IDDB_MEM_E:
            /* ECC */
            PRV_CPSS_INT_SCAN_LOCK();
            rc = prvCpssPxDiagDataIntegrityEccInterruptsMask (devNum, errorType, intMaskEn,
                                                              PRV_CPSS_PIPE_ECC_0_SINGLE_ERROR_INT_E,
                                                              PRV_CPSS_PIPE_ECC_0_DOUBLE_ERROR_INT_E);
            PRV_CPSS_INT_SCAN_UNLOCK();
            break;

        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_PAYLOAD_IDDB_MEM_E:
            /* ECC */
            PRV_CPSS_INT_SCAN_LOCK();
            rc = prvCpssPxDiagDataIntegrityEccInterruptsMask (devNum, errorType, intMaskEn,
                                                              PRV_CPSS_PIPE_ECC_1_SINGLE_ERROR_INT_E,
                                                              PRV_CPSS_PIPE_ECC_1_DOUBLE_ERROR_INT_E);
            PRV_CPSS_INT_SCAN_UNLOCK();
            break;
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E:
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_RX_IDDB_MEM_E:
            PRV_CPSS_INT_SCAN_LOCK();
            rc = prvCpssPxDiagDataIntegrityEccInterruptsMask (devNum, errorType, intMaskEn,
                                                              PRV_CPSS_PIPE_MPPM_ECC_INT_ECC_SINGLE_ERROR_E,
                                                              PRV_CPSS_PIPE_MPPM_ECC_INT_ECC_DOUBLE_ERROR_E);
            PRV_CPSS_INT_SCAN_UNLOCK();
            break;

        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E:
            /*parity */
            intIndex = PRV_CPSS_PIPE_BM_CORE_0_VALID_TABLE_PARITY_ERROR_INTERRUPT_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            break;

        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E:
            /* parity */
            intIndex = PRV_CPSS_PIPE_TXQ_QCN_BUFFER_FIFO_PARITY_ERR_INT_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            break;

        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E:
            intIndex = PRV_CPSS_PIPE_BMA_MC_CNT_PARITY_ERROR_E;
            rc = prvCpssDrvEventsMask(devNum, intIndex, intMaskEn);
            break;

        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HEADER_REORDER_MEM_IDDB_MEM_E:
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_PAYLOAD_REORDER_MEM_IDDB_MEM_E:
            /* ECC */
            PRV_CPSS_INT_SCAN_LOCK();
            rc = prvCpssPxDiagDataIntegrityEccInterruptsMask (devNum, errorType, intMaskEn,
                                                              PRV_CPSS_PIPE_TXDMA_ECC_SINGLE_ERROR_INT_E,
                                                              PRV_CPSS_PIPE_TXDMA_ECC_DOUBLE_ERROR_INT_E);
            PRV_CPSS_INT_SCAN_UNLOCK();
            break;


        default:
            rc = GT_BAD_PARAM;
    }

    return rc;
}

/**
* @internal
*           prvCpssPxDiagDataIntegrityExtMemoryProtectionEventMaskGet
*           function
* @endinternal
*
* @brief   Function gets mask state for non DFX protected memories.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] memType                  - memory type
*
* @param[out] errorTypePtr             - (pointer to) error type
* @param[out] operationPtr             - (pointer to) mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssPxDiagDataIntegrityExtMemoryProtectionEventMaskGet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT        memType,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
)
{
    GT_U32      intIndex  = PRV_CPSS_PIPE_LAST_INT_E; /* HW interrupt index for <parity>/<single ECC> error */
    GT_U32      intIndex2 = PRV_CPSS_PIPE_LAST_INT_E; /* HW interrupt index for  <double ECC> error */
    GT_BOOL     intEn;  /* <parity>/<single Ecc> error interrupt enabled */
    GT_BOOL     intEn2; /* <double Ecc> error interrupt enabled */

    switch (memType)
    {
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_HEADER_IDDB_MEM_E:
            intIndex = PRV_CPSS_PIPE_ECC_0_SINGLE_ERROR_INT_E;
            intIndex2 = PRV_CPSS_PIPE_ECC_0_DOUBLE_ERROR_INT_E;
            break;
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_PAYLOAD_IDDB_MEM_E:
            intIndex = PRV_CPSS_PIPE_ECC_1_SINGLE_ERROR_INT_E;
            intIndex2 = PRV_CPSS_PIPE_ECC_1_DOUBLE_ERROR_INT_E;
            break;
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E:
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_RX_IDDB_MEM_E:
            intIndex = PRV_CPSS_PIPE_MPPM_ECC_INT_ECC_SINGLE_ERROR_E;
            intIndex2 = PRV_CPSS_PIPE_MPPM_ECC_INT_ECC_DOUBLE_ERROR_E;
            break;
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E:
            intIndex = PRV_CPSS_PIPE_BM_CORE_0_VALID_TABLE_PARITY_ERROR_INTERRUPT_E;
            break;
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E:
            intIndex = PRV_CPSS_PIPE_TXQ_QCN_BUFFER_FIFO_PARITY_ERR_INT_E;
            break;
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E:
            intIndex = PRV_CPSS_PIPE_BMA_MC_CNT_PARITY_ERROR_E;
            break;
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HEADER_REORDER_MEM_IDDB_MEM_E:
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_PAYLOAD_REORDER_MEM_IDDB_MEM_E:
            intIndex = PRV_CPSS_PIPE_TXDMA_ECC_SINGLE_ERROR_INT_E;
            intIndex2 = PRV_CPSS_PIPE_TXDMA_ECC_DOUBLE_ERROR_INT_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    }

    if (intIndex == PRV_CPSS_PIPE_LAST_INT_E)
    {
        /* neither of memType matched */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssDrvEventsMaskGet(devNum, intIndex, &intEn);
    if (intIndex2 == PRV_CPSS_PIPE_LAST_INT_E)
    {
        /* Parity protection */
        *operationPtr = (GT_FALSE == intEn) ? CPSS_EVENT_MASK_E : CPSS_EVENT_UNMASK_E;
        *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
    }
    else
    {
        /* ECC protection */
        prvCpssDrvEventsMaskGet(devNum, intIndex2, &intEn2);
        if (GT_FALSE == intEn)
        {
            *errorTypePtr = (GT_FALSE == intEn2) ?
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E :
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
        }
        else
        {
            *errorTypePtr = (GT_FALSE == intEn2) ?
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E :
                CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
        }
        *operationPtr = (GT_FALSE == intEn && GT_FALSE == intEn2) ?
            CPSS_EVENT_MASK_E : CPSS_EVENT_UNMASK_E;
    }

    return GT_OK;
}


/**
* @internal
*           prvCpssPxDiagDataIntegrityDfxInterruptCheckAndRegGet
*           function
* @endinternal
*
* @brief   Check if interrupt is DFX interrupt and (optionally) return appropriate
*         DFX Interrupt Cause Register and/or DFX Interrupt Mask Register
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
*                                      portGroupId   - portGroup number
* @param[in] intNum                   - interrupt number
*
* @param[out] causeRegAddrPtr          - (pointer to) DFX/DFX1 Interrupt Cause register address.
*                                      Can be null.
* @param[out] maskRegAddrPtr           - (array of) Interrupt Mask address
*                                      Can be null.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device number
* @retval GT_BAD_STATE             - illegal HW state
* @retval GT_OUT_OF_RANGE          - if intNum is not DFX interrupt number
*/
GT_STATUS prvCpssPxDiagDataIntegrityDfxInterruptCheckAndRegGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_U32                                      intNum,
    OUT GT_U32                                      *causeRegAddrPtr,
    OUT GT_U32                                      *maskRegAddrPtr

)
{

    if (intNum > PRV_CPSS_PIPE_DFX_INTERRUPT_SUMMARY_E && intNum <= PRV_CPSS_PIPE_DFX_INTERRUPT_SUMMARY_E + 32)
    {
        if (causeRegAddrPtr != NULL)
        {
            *causeRegAddrPtr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.globalRegs.globalInterrupt.dfxInterruptCause;
        }

        if (maskRegAddrPtr != NULL)
        {
            *maskRegAddrPtr  = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MG.globalRegs.globalInterrupt.dfxInterruptMask;
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}



/**
* @internal
*           prvCpssPxaDataIntegrityDfxMemoryProtectionEventMaskSet
*           function
* @endinternal
*
* @brief   Function sets mask/unmask for ECC/Parity DFX
*          interrupt.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] memInfoPtr               - (pointer to) item of Device Data Integrity DB(PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC).
* @param[in] errorType                - error type
* @param[in] operation                - mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, memType, errorType
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxDiagDataIntegrityDfxMemoryProtectionEventMaskSet
(
    IN GT_U8                                            devNum,
    IN PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC      *memInfoPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
)
{
    GT_U32 pipeIndex   = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_PIPE_GET   (memInfoPtr->key);
    GT_U32 clientIndex = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_CLIENT_GET (memInfoPtr->key);
    GT_U32 memoryIndex = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_MEMORY_GET (memInfoPtr->key);
    GT_STATUS rc;
    GT_U32 fieldOffset;
    GT_U32 fieldLength;
    GT_U32 regData;

    /* prepare register Data */
    if(memInfoPtr->protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
    {
        switch(errorType)
        {
            case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E:
                fieldOffset = 2;
                fieldLength = 1;
                regData = operation;
                break;

            case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E:
                fieldOffset = 3;
                fieldLength = 1;
                regData = operation;
                break;

            case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E:
                fieldOffset = 2;
                fieldLength = 2;
                regData = (operation == CPSS_EVENT_MASK_E) ? 0 : 3;
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else if(memInfoPtr->protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E)
    {
        fieldOffset = 1;
        fieldLength = 1;
        regData = operation;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    /* write to Memory Interrupt Mask Register */
    rc = prvCpssDfxMemorySetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
               PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_INT_MASK_REG_CNS,
               fieldOffset, fieldLength, regData);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* it's take some time in DFX bus to configure register.
       Add this Read to guaranty that write done before exit from the function. */
    rc = prvCpssDfxMemoryGetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
              PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_INT_MASK_REG_CNS,
              fieldOffset, fieldLength, &regData);

    return rc;
}

/**
* @internal dataIntegrityDbCmp function
* @endinternal
*
* @brief   Function for compare two DB entries for binary search algorithm.
*
* @param[in] aPtr                     - (pointer to) key
* @param[in] bPtr                     - (pointer to) DB item key
*
* @retval 0                        - equal
* @retval else                     - not equal
*/
static int dataIntegrityDbCmp
(
    IN const GT_VOID *aPtr,
    IN const GT_VOID *bPtr
)
{
    int temp1 = *((int*)aPtr);
    int temp2 = (int)(((PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC*)bPtr)->key);

    return temp1 -  temp2;
}

/**
* @internal searchMemType function
* @endinternal
*
* @brief   Function performs binary search in map DB.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] dfxPipeIndex             - DFX pipe index
* @param[in] dfxClientIndex           - DFX client index
* @param[in] dfxMemoryIndex           - DFX memory index
*
* @param[out] dbItemPtrPtr             - (pointer to) DB entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS searchMemType
(
    IN  GT_U32                                          dfxPipeIndex,
    IN  GT_U32                                          dfxClientIndex,
    IN  GT_U32                                          dfxMemoryIndex,
    OUT PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    **dbItemPtrPtr
)
{
    GT_STATUS                           rc;    /* return code */
    GT_U32                              key;   /* compare key */
    GT_U32                              itemSize; /* size of compared item */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC   *dbItemTmpPtr; /* pointer to found item */
    CPSS_OS_COMPARE_ITEMS_FUNC_PTR      cmpFuncPtr; /* pointer to compare function */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */

    prvCpssPxDiagDataIntegrityDbPointerSet(&dbArrayPtr, &dbArrayEntryNum);

    cmpFuncPtr = dataIntegrityDbCmp;
    key = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_GENERATE_KEY_MAC(
        dfxPipeIndex, dfxClientIndex, dfxMemoryIndex);

    itemSize = sizeof(PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC);

    rc = cpssOsBsearch((void *)&key, (void *)dbArrayPtr, dbArrayEntryNum, itemSize, cmpFuncPtr, (void*)&dbItemTmpPtr);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "The dfx memory was not found");
    }

    *dbItemPtrPtr = dbItemTmpPtr;

    return GT_OK;
}



/**
* @internal prvCpssPxDiagDataIntegrityNonDfxEventsGet function
* @endinternal
*
* @brief   Function scans interrupt tree for non-DFX DataIntegrity event details.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] intNum                   - DFX interrupt number
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssPxDiagDataIntegrityNonDfxEventsGet
(
    IN  GT_U32                                      intNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC       eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_U32 i;                   /* loop iterators */
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT   memType;        /* memory type */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC *dbPtr = NULL;  /* Data Integrity DB */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC *dbItemPtr = NULL; /* Data Integrity DB item pointer */
    GT_U32                                       dbSize;  /* Data Integrity DB size */

    if (*eventsNumPtr == 0)
    {
        return GT_OK;
    }

    *isNoMoreEventsPtr = GT_TRUE;
    *eventsNumPtr = 1;          /* default value */
    eventsArr[0].location.isMppmInfoValid =
    eventsArr[0].memoryUseType    = CPSS_DIAG_DATA_INTEGRITY_MEMORY_USAGE_TYPE_UNKNOWN_E;
    eventsArr[0].correctionMethod = CPSS_DIAG_DATA_INTEGRITY_CORRECTION_METHOD_UNKNOWN_E;
    switch(intNum)
    {
        case PRV_CPSS_PIPE_MPPM_ECC_INT_ECC_SINGLE_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
            eventsArr[0].location.isMppmInfoValid = GT_TRUE;
            break;

        case PRV_CPSS_PIPE_MPPM_ECC_INT_ECC_DOUBLE_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
            eventsArr[0].location.isMppmInfoValid = GT_TRUE;
            break;

        case PRV_CPSS_PIPE_TXDMA_ECC_SINGLE_ERROR_INT_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HEADER_REORDER_MEM_IDDB_MEM_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
            break;

        case PRV_CPSS_PIPE_TXDMA_ECC_DOUBLE_ERROR_INT_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HEADER_REORDER_MEM_IDDB_MEM_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
            break;

        case PRV_CPSS_PIPE_ECC_0_DOUBLE_ERROR_INT_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_HEADER_IDDB_MEM_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
            eventsArr[0].location.isMppmInfoValid = GT_TRUE;
            break;

        case PRV_CPSS_PIPE_ECC_0_SINGLE_ERROR_INT_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_HEADER_IDDB_MEM_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
            break;

        case PRV_CPSS_PIPE_ECC_1_DOUBLE_ERROR_INT_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_PAYLOAD_IDDB_MEM_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
             break;

        case PRV_CPSS_PIPE_ECC_1_SINGLE_ERROR_INT_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_PAYLOAD_IDDB_MEM_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
            break;

        /* -- _PARITY_ -- */

        case PRV_CPSS_PIPE_BM_CORE_0_VALID_TABLE_PARITY_ERROR_INTERRUPT_E:
            eventsArr[0].location.ramEntryInfo.memType    = CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E;
            eventsArr[0].eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        case PRV_CPSS_PIPE_TXQ_QCN_BUFFER_FIFO_PARITY_ERR_INT_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E;
            eventsArr[0].eventsType       = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;

        case PRV_CPSS_PIPE_BMA_MC_CNT_PARITY_ERROR_E:
            eventsArr[0].location.ramEntryInfo.memType          = CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E;
            eventsArr[0].eventsType       = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
            break;


        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    /* look for the first DB item with specified memory type */
    prvCpssPxDiagDataIntegrityDbPointerSet(&dbPtr, &dbSize);
    if (NULL == dbPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Data Integrity DB not found for the device");
    }

    memType = eventsArr[0].location.ramEntryInfo.memType;
    for (i = 0; i < dbSize && dbPtr[i].memType != memType; i++ );
    if (i == dbSize)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND,
                                      "The Data Integrity DB item is not found");
    }

    dbItemPtr = &dbPtr[i];

    /* assign DFX coordinates */
    eventsArr[0].location.ramEntryInfo.memLocation.dfxPipeId     =
        PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_PIPE_GET(dbItemPtr->key);

    eventsArr[0].location.ramEntryInfo.memLocation.dfxClientId   =
        PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_CLIENT_GET(dbItemPtr->key);

    eventsArr[0].location.ramEntryInfo.memLocation.dfxMemoryId   =
        PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_MEMORY_GET(dbItemPtr->key);


    /* fill memory usage type and memory correction method */
    eventsArr[0].memoryUseType    = dbItemPtr->memUsageType;
    eventsArr[0].correctionMethod = dbItemPtr->correctionMethod;

    return GT_OK;
}


/**
* @internal prvCpssPxDiagDataIntegrityFillHwLogicalFromRam
*           function
* @endinternal
*
* @brief   fill HW and logical locations in
*          CPSS_PX_LOCATION_FULL_INFO_STC struct according to
*          RAM location.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in,out] locationPtr          - (pointer to) location. HW- and logical-
*                                       related parts will be modified.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  prvCpssPxDiagDataIntegrityFillHwLogicalFromRam
(
    IN    GT_U8 devNum,
    INOUT CPSS_PX_LOCATION_FULL_INFO_STC *locationPtr
)
{
    CPSS_PX_LOGICAL_TABLE_ENT      logicalTableName;
    CPSS_PX_RAM_INDEX_INFO_STC     *ramInfoPtr;
    GT_U32 logicalIndex = 0;    /* logical table entry index */
    CPSS_PX_TABLE_ENT           hwTable; /* HW table type */
    GT_U32                      hwIndex; /* HW table item index */
    GT_STATUS                   rc;

    ramInfoPtr = &locationPtr->ramEntryInfo;

    if (locationPtr->isMppmInfoValid)
    {
        /* MPPM memory could be the special case because of unusual
           locationPtr->ramEntryInfo.ramRow content:
           ramRow[19:18] = group number,
           ramRow[17:16] = bank number,
           ramRow[15:0]  = buffer number
           But there are no HW table relevant to MPPM so it will be handled
           like usual case when a RAM->HW mapping is absent */
        locationPtr->mppmMemLocation.bankId = (ramInfoPtr->ramRow >> 16) & 0x3;
        locationPtr->mppmMemLocation.mppmId = (ramInfoPtr->ramRow >> 18) & 0x3;
        locationPtr->mppmMemLocation.portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        return GT_OK;
    }

    rc = prvCpssPxDiagDataIntegrityDfxErrorConvert(devNum,
                                                   &ramInfoPtr->memLocation,
                                                   ramInfoPtr->ramRow,
                                                   &locationPtr->hwEntryInfo);
    if (rc != GT_OK)
    {
        return rc;
    }


    /* fill logical tables info */
    hwTable = locationPtr->hwEntryInfo.hwTableType;
    hwIndex = locationPtr->hwEntryInfo.hwTableEntryIndex;

    prvCpssPxDiagDataIntegrityTableHwToLogicalGet(hwTable, &logicalTableName);
    logicalIndex      = hwIndex;
    locationPtr->logicalEntryInfo.logicalTableType       = logicalTableName;
    locationPtr->logicalEntryInfo.logicalTableEntryIndex = logicalIndex;

    return GT_OK;
}
/**
* @internal prvCpssPxDiagDataIntegrityDfxInterruptPipeClientGet
*           function
* @endinternal
*
* @brief   Function scans DFX unit registers for DataIntegrity event details.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] dfxInt                   - DFX interrupt number
*                                      eventsNumPtr  - (pointer to) max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsTypeArr[] and
*                                      memTypeArr[] can retrieve.
*
* @param[out] pipePtr                  -   pipe number
* @param[out] clientPtr                - client number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on unknown device family or wrong dfxInt number
*/
static GT_STATUS prvCpssPxDiagDataIntegrityDfxInterruptPipeClientGet
(
    IN  GT_U32 dfxInt,
    OUT GT_U32 *pipePtr,
    OUT GT_U32 *clientPtr
)
{
    PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC *mapArr;
    GT_U32 ix;


    if (dfxInt >  PRV_CPSS_PIPE_DFX_INTERRUPT_SUMMARY_E &&
        dfxInt <=  PRV_CPSS_PIPE_DFX_INTERRUPT_SUM_31_E)
    {
        ix     = dfxInt - PRV_CPSS_PIPE_DFX_INTERRUPT_SUMMARY_E;
        mapArr = pipeDfxIntToPipeClientMapArr;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    *pipePtr   = mapArr[ix].dfxPipeIndex;
    *clientPtr = mapArr[ix].dfxClientIndex;

    return GT_OK;
}


/**
* @internal prvCpssPxDiagDataIntegrityDfxEventsViaMgUnitGet
*           function
* @endinternal
*
* @brief   Function scans DFX unit registers for DataIntegrity event details.
*         HW/Logical table coordinates are not filled here! Only RAM coordinates.
*         the function uses MG unit registers DFX
*         Interrupts Summary Cause Register" to get pipe/client
*         indexes of occured event.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] portGroupId              - portGroup number
* @param[in] intNum                   - DFX interrupt number
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssPxDiagDataIntegrityDfxEventsViaMgUnitGet
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      intNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC       eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc;
    GT_U32    dfxIntCauseReg;   /* DFX/DFX1 Interrupt Cause register address */
    GT_U32    dfxIntMaskReg;    /* DFX/DFX1 Interrupt Mask Register address */
    GT_U32    regData;          /* register data */
    GT_U32    regData2;         /* mask register data */
    GT_U32    pipeIndex;        /* DFX pipe number */
    GT_U32    clientIndex;      /* DFX client number */
    GT_U32    memoryNumber;     /* DFX memory number */
    GT_U32    i;                /* cycle counter */
    GT_U32    j;                /* cycle counter */
    GT_U32    eventCounter = 0; /* events counter */
    PRV_CPSS_PX_DIAG_DATA_INTEGRITY_CLIENT_STATUS_STC clientBmp = {{0,0,0,0}};  /* client status struct */

    if (*eventsNumPtr == 0)
    {
        return GT_OK;
    }

    *isNoMoreEventsPtr = GT_TRUE;

    /* - For DFX interrupt find DFX Pipe index and Client index.
       - Set "Pipe Select" register according to DFX pipe index
       - Read 4 memory bitmap registers according to Client index in current DFX pipe.
       - For every bit which is set (memory number) calculate interrupt cause register address:
             pipe index + client index + memory number => cause/mask register address.
    */

    rc = prvCpssPxDiagDataIntegrityDfxInterruptPipeClientGet(intNum,
                                                             &pipeIndex, &clientIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(pipeIndex   == DATA_INTEGRITY_ENTRY_NOT_USED_CNS ||
       clientIndex == DATA_INTEGRITY_ENTRY_NOT_USED_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    for(i = 0; i < 4; i++)
    {
        rc = prvCpssDfxClientRegRead(
            devNum, pipeIndex, clientIndex,
            PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_CLIENT_INT_STATUS0_REG_CNS + i * 4,
            &(clientBmp.bitmap[i]));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = prvCpssPxDiagDataIntegrityDfxInterruptCheckAndRegGet(devNum, intNum,
                                               &dfxIntCauseReg, &dfxIntMaskReg);
    if (rc != GT_OK)
    {
        return rc;
    }

    for(i = 0; i < 4; i++)
    {
        for(j = 0; j < 32; j++)
        {
            if(U32_GET_FIELD_MAC(clientBmp.bitmap[i], j, 1))
            {
                memoryNumber = i*32 + j;
                rc = prvCpssPxDiagDfxMemoryEventFromDbGet(
                    devNum, pipeIndex, clientIndex, memoryNumber, &eventsArr[eventCounter]);
                if (rc != GT_OK)
                {
                    return rc;
                }
                if(eventCounter == (*eventsNumPtr - 1))
                {
                    rc = prvCpssHwPpPortGroupGetRegField(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, dfxIntCauseReg, 31, 1, &regData);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }

                    rc = prvCpssHwPpPortGroupGetRegField(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, dfxIntMaskReg, 31, 1, &regData2);
                    if(GT_OK != rc)
                    {
                        return rc;
                    }


                    if((regData & regData2) != 0)
                    {
                        *isNoMoreEventsPtr = GT_FALSE;
                    }

                    return GT_OK;
                }

                eventCounter++;
            }
        }
    }
    *eventsNumPtr = eventCounter;

    return GT_OK;
}

/**
* @internal internal_cpssPxDiagDataIntegrityEventsGet function
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
static GT_STATUS internal_cpssPxDiagDataIntegrityEventsGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_U32                                      evExtData,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC       eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc;
    CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT location;
    GT_U32 failedRow = 0;
    GT_U32 i;
    GT_U32 intNum;

    intNum      = evExtData & 0xFFFF;
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(eventsNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(eventsArr);
    CPSS_NULL_PTR_CHECK_MAC(isNoMoreEventsPtr);

    if (GT_OK == prvCpssPxDiagDataIntegrityDfxInterruptCheckAndRegGet(
                                       devNum, intNum, NULL, NULL))
    {
        /* handle DFX events starting with reading MG unit registers
         * "DFX/DFX1 Interrupts Summary Cause Register" */
        rc = prvCpssPxDiagDataIntegrityDfxEventsViaMgUnitGet(devNum,
                                                             intNum,
                                                             eventsNumPtr,
                                                             eventsArr,
                                                             isNoMoreEventsPtr);

    }
    else
    {
         rc = prvCpssPxDiagDataIntegrityNonDfxEventsGet(
                                                    intNum,
                                                    eventsNumPtr, eventsArr,
                                                    isNoMoreEventsPtr);
    }
    if (rc != GT_OK)
    {
        return rc;
    }
        /* get failed row info */
    for (i=0; i < *eventsNumPtr; i++)
    {
        if (eventsArr[i].location.isMppmInfoValid == GT_TRUE)
        {
            location.mppmMemLocation = eventsArr[i].location.mppmMemLocation;
        }
        else
        {
            location.memLocation = eventsArr[i].location.ramEntryInfo.memLocation;
        }

        rc = prvCpssPxDiagDataIntegrityErrorInfoGet(
            devNum, eventsArr[i].location.ramEntryInfo.memType, &location,
            NULL, &failedRow, NULL, NULL);
        if (rc != GT_OK)
        {
            return rc;
        }
        eventsArr[i].location.ramEntryInfo.ramRow = failedRow;
        /* Fill HW, Logical coordinates */
        rc = prvCpssPxDiagDataIntegrityFillHwLogicalFromRam(
            devNum, &eventsArr[i].location);

    }
    return GT_OK;
}

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
    OUT CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC       eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagDataIntegrityEventsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, evExtData, eventsNumPtr, eventsArr, isNoMoreEventsPtr));

    rc = internal_cpssPxDiagDataIntegrityEventsGet(devNum, evExtData, eventsNumPtr, eventsArr, isNoMoreEventsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, evExtData, eventsNumPtr, eventsArr, isNoMoreEventsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxDiagDataIntegrityEventMaskSet
*           function
* @endinternal
*
* @brief  Function sets mask/unmask for ECC/Parity event. Event
*         is masked/unmasked for whole hw/logical table or for all memories
*         of specified type.
*         RAM DFX coordinates - pipeId, clientId, memoryId - and an entry index
*         info - ramRow, logicalTableEntryIndex, hwTableEntryIndex - are ignored.
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
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
static GT_STATUS internal_cpssPxDiagDataIntegrityEventMaskSet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC              *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                         operation
)
{
    GT_STATUS                   rc;          /* return code */
    GT_STATUS                   rc1;         /* return code */
    GT_U32                      i;           /* loop iterator */
    GT_U32                      j;           /* loop iterator */
    GT_BOOL                     memoryFound = GT_FALSE; /* memory found status */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC      *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT        memType;
    GT_U32                                          hwTable; /* CPSS_PX_TABLE_ENT or
                                                                PRV_CPSS_PX_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT */
    const CPSS_PX_TABLE_ENT                         *hwTablesPtr; /* pointer to array of hw tables names */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC              hwLocation; /* hw location */
    GT_U32                                          hwTableInDb;  /* value of HW Table in DB entry */
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(memEntryPtr);

    rc = cpssPxDiagDataIntegrityProtectionTypeGet(devNum, memEntryPtr, &protectionType);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (((protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E) &&
         (errorType != CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E)) ||
        ((protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E) &&
         (errorType == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(operation != CPSS_EVENT_MASK_E &&
       operation != CPSS_EVENT_UNMASK_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

     switch (memEntryPtr->type)
    {
        case CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE:
            /* call cpssPxDiagDataIntegrityEventMaskSet for every hw table
               relevant to logical table */
            cpssOsMemSet(&hwLocation, 0, sizeof(hwLocation));
            hwLocation.type = CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE;
            rc = prvCpssPxDiagDataIntegrityTableLogicalToHwListGet(
                memEntryPtr->info.logicalEntryInfo.logicalTableType,
                &hwTablesPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (NULL == hwTablesPtr)
            {/* something wrong with DB */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            /* if some table doesn't have protection (so can't be masked)
               or is absent in the DB skip it. But if ALL tables are
               skipped return error */
            rc1 = GT_NOT_FOUND;
            for (j=0; hwTablesPtr[j] != LAST_VALID_PX_TABLE_CNS; j++)
            {
                hwLocation.info.hwEntryInfo.hwTableType = hwTablesPtr[j];
                /* hwLocation.info.hwTableEntryIndex  doesn't matter */
                rc = internal_cpssPxDiagDataIntegrityEventMaskSet(
                    devNum, &hwLocation, errorType, operation);
                if (rc == GT_NOT_SUPPORTED || rc == GT_NOT_FOUND)
                {
                   /* ignore tables either not found in DB  or not supporting the feature */
                    continue;
                }
                else if (rc != GT_OK)
                {
                    return rc;
                }
                rc1 = rc;
            }
            /* no table that supports masking */
            if (rc1 != GT_OK)
            {
                return rc1;
            }
        /* done */
        return GT_OK;
        case CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE:
            memType = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_NONEXISTENT_MEM_TYPE_CNS;
            hwTable = memEntryPtr->info.hwEntryInfo.hwTableType;
            break;

        case CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE:
            hwTable = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_NONEXISTENT_HW_TABLE_CNS;
            memType = memEntryPtr->info.ramEntryInfo.memType;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssPxDiagDataIntegrityDbPointerSet(&dbArrayPtr, &dbArrayEntryNum);

    for(i = 0; i < dbArrayEntryNum; i++)
    {
        hwTableInDb = dbArrayPtr[i].hwTableName;
        if ((dbArrayPtr[i].memType == memType) || (hwTableInDb == hwTable))
        {
            memoryFound = GT_TRUE;

            if (dbArrayPtr[i].protectionType != CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
            {
                /* DFX memory mask operation */
                rc = prvCpssPxDiagDataIntegrityDfxMemoryProtectionEventMaskSet(devNum, &dbArrayPtr[i], errorType, operation);
                if(GT_OK != rc)
                {
                    return rc;
                }
            }
            else if (dbArrayPtr[i].externalProtectionType != CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
            {
                /* non-DFX memory mask operation */
                rc = prvCpssPxDiagDataIntegrityExtMemoryProtectionEventMaskSet(devNum, dbArrayPtr[i].memType, errorType, operation);
                if(GT_OK != rc)
                {
                    return rc;
                }
            }
            else
            {
                /* the memory is not protected at all */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
        }
    }

    if (memoryFound == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxDiagDataIntegrityEventMaskSet function
* @endinternal
*
* @brief   Function sets mask/unmask for ECC/Parity event.
*         Event is masked/unmasked for whole hw/logical table or for all memories
*         of specified type.
*         RAM DFX coordinates - pipeId, clientId, memoryId - and an entry index
*         info - ramRow, logicalTableEntryIndex, hwTableEntryIndex - are ignored.
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
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
GT_STATUS cpssPxDiagDataIntegrityEventMaskSet
(
    IN  GT_SW_DEV_NUM                                 devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT errorType,
    IN  CPSS_EVENT_MASK_SET_ENT                       operation
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagDataIntegrityEventMaskSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memEntryPtr, errorType, operation));

    rc = internal_cpssPxDiagDataIntegrityEventMaskSet(devNum, memEntryPtr, errorType, operation);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memEntryPtr, errorType, operation));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxDiagDataIntegrityDfxMemoryProtectEventsMaskGet function
* @endinternal
*
* @brief   Function gets mask/unmask for ECC/Parity DFX interrupt.
*
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
* @retval GT_BAD_PARAM             - on wrong device, portGroup, memType, errorType
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxDiagDataIntegrityDfxMemoryProtectEventsMaskGet
(
    IN GT_U8                                            devNum,
    IN PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC       *memInfoPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr

)
{
    GT_STATUS rc;
    GT_U32 regData;     /* register data */
    GT_U32 pipeIndex   = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_PIPE_GET   (memInfoPtr->key);
    GT_U32 clientIndex = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_CLIENT_GET (memInfoPtr->key);
    GT_U32 memoryIndex = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_MEMORY_GET (memInfoPtr->key);

    /* read the Interrupt Mask Register */
    rc = prvCpssDfxMemoryRegRead (devNum, pipeIndex, clientIndex, memoryIndex,
                                  PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_INT_MASK_REG_CNS,
                                  &regData);
    if(GT_OK != rc)
    {
        return rc;
    }


    if(memInfoPtr->protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
    {
        regData = U32_GET_FIELD_MAC(regData, 2, 2);
        switch(regData)
        {
            case 0:
                *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                *operationPtr = CPSS_EVENT_MASK_E;
                break;
            case 1:
                *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                *operationPtr = CPSS_EVENT_UNMASK_E;
                break;
            case 2:
                *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                *operationPtr = CPSS_EVENT_UNMASK_E;
                break;
            default: /* 3 */
                *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                *operationPtr = CPSS_EVENT_UNMASK_E;
                break;
        }
    }
    else if(memInfoPtr->protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E)
    {
        *operationPtr = U32_GET_FIELD_MAC(regData, 1, 1);

        *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal internal_cpssPxDiagDataIntegrityEventMaskGet function
* @endinternal
*
* @brief   Function gets mask/unmask for ECC/Parity interrupt.
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
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
static GT_STATUS internal_cpssPxDiagDataIntegrityEventMaskGet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC              *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                         *operationPtr
)
{
    GT_STATUS                   rc;          /* return code */
    GT_U32                      i;           /* loop iterator */
    GT_BOOL                     memoryFound = GT_FALSE; /* memory found status */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType;
    GT_U32                                          hwTable; /* CPSS_PX_TABLE_ENT or
                                                                PRV_CPSS_PX_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT */
    const CPSS_PX_TABLE_ENT                       *hwTablesPtr; /* pointer to array of hw tables names */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            hwLocation; /* hw location */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(memEntryPtr);
    CPSS_NULL_PTR_CHECK_MAC(errorTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(operationPtr);

    switch (memEntryPtr->type)
    {
        case CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE:
            /* get list of HW tables from first logical */
            rc = prvCpssPxDiagDataIntegrityTableLogicalToHwListGet(
                memEntryPtr->info.logicalEntryInfo.logicalTableType,
                &hwTablesPtr);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (NULL == hwTablesPtr)
            {
                /* something wrong with DB */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            /* return as result the status of first HW table supporting masking */
            cpssOsMemSet(&hwLocation, 0, sizeof(hwLocation));
            hwLocation.type = CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE;

            for (i=0; hwTablesPtr[i] != LAST_VALID_PX_TABLE_CNS; i++)
            {
                hwLocation.info.hwEntryInfo.hwTableType = hwTablesPtr[i];
                /* hwLocation.info.hwTableEntryIndex  doesn't matter */

                rc = internal_cpssPxDiagDataIntegrityEventMaskGet (
                    devNum, &hwLocation, errorTypePtr, operationPtr);

                /* ignore tables either not found in DB  or not supporting the feature */
                if (rc != GT_NOT_SUPPORTED && rc != GT_NOT_FOUND)
                {
                    return rc;
                }
            }
            /* done  */
            return rc;

        case CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE:
            memType = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_NONEXISTENT_MEM_TYPE_CNS;
            hwTable = memEntryPtr->info.hwEntryInfo.hwTableType;
            break;
        case CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE:
            hwTable = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_NONEXISTENT_HW_TABLE_CNS;
            memType = memEntryPtr->info.ramEntryInfo.memType;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssPxDiagDataIntegrityDbPointerSet(&dbArrayPtr, &dbArrayEntryNum);

    for(i = 0; i < dbArrayEntryNum; i++)
    {
        /* find first entry of given memType/hwTable */
        if ((dbArrayPtr[i].memType == memType) || (dbArrayPtr[i].hwTableName == hwTable))
        {
            memoryFound = GT_TRUE;

            if (dbArrayPtr[i].protectionType != CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
            {
                /* DFX memory mask operation */
                rc  = prvCpssPxDiagDataIntegrityDfxMemoryProtectEventsMaskGet(
                                           devNum, &dbArrayPtr[i], errorTypePtr, operationPtr);
                if(GT_OK != rc)
                {
                    return rc;
                }
            }
            else if (dbArrayPtr[i].externalProtectionType != CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
            {
                /* non-DFX memory mask operation */
                rc = prvCpssPxDiagDataIntegrityExtMemoryProtectionEventMaskGet(
                                                  devNum, dbArrayPtr[i].memType,
                                                  errorTypePtr, operationPtr);
                if(GT_OK != rc)
                {
                    return rc;
                }
            }
            else
            {
                /* memory is not protected at all */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }

            /* it's enough to read the first instance of certain memType */
            break;
        }
    }

    if (memoryFound == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxDiagDataIntegrityEventMaskGet function
* @endinternal
*
* @brief   Function gets mask/unmask for ECC/Parity interrupt.
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
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
GT_STATUS cpssPxDiagDataIntegrityEventMaskGet
(
    IN  GT_SW_DEV_NUM                                 devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT *errorTypePtr,
    OUT CPSS_EVENT_MASK_SET_ENT                       *operationPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagDataIntegrityEventMaskGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memEntryPtr, errorTypePtr, operationPtr));

    rc = internal_cpssPxDiagDataIntegrityEventMaskGet(devNum, memEntryPtr, errorTypePtr, operationPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memEntryPtr, errorTypePtr, operationPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxDiagDataIntegrityDfxErrorInfoGet function
* @endinternal
*
* @brief   Function gets ECC/Parity error info from DFX unit.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] locationPtr              - (pointer to) memory location indexes
*
* @param[out] failedRowPtr             - (pointer to) failed raw
* @param[out] failedSegmentPtr         - (pointer to) failed
*       segment
* @param[out] failedSyndromePtr        - (pointer to) failed
*       syndrome
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, memType
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxDiagDataIntegrityDfxErrorInfoGet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT  *locationPtr,
    OUT GT_U32                                          *failedRowPtr,
    OUT GT_U32                                          *failedSyndromePtr
)
{
    GT_U32    pipeIndex;
    GT_U32    clientIndex;
    GT_U32    memoryIndex;
    GT_STATUS rc;

    pipeIndex   = locationPtr->memLocation.dfxPipeId;
    clientIndex = locationPtr->memLocation.dfxClientId;
    memoryIndex = locationPtr->memLocation.dfxMemoryId;

    /* read Erroneous Address Register */
    if (failedRowPtr)
    {
        rc = prvCpssDfxMemoryGetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
                                         PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_ERRONEOUS_ADDR_REG_CNS,
                                         0, 16, failedRowPtr);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    /* read ECC Syndrome Register */
    if (failedSyndromePtr)
    {
        rc = prvCpssDfxMemoryGetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
                                         PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_ECC_SYNDROME_REG_CNS,
                                         0, 8, failedSyndromePtr);
        if(GT_OK != rc)
        {
            return rc;
        }
    }
    return GT_OK;
}


/**
* @internal prvCpssPxDiagDataIntegrityErrorInfoGet function
* @endinternal
*
* @brief   Function gets ECC/Parity error info.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] memType                  - type of memory(table)
* @param[in] locationPtr              - (pointer to) memory location indexes
*
* @param[out] errorCounterPtr          - (pointer to) error counter
* @param[out] failedRowPtr             - (pointer to) failed row
* @param[out] failedSyndromePtr        - (pointer to) failed syndrome
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup, memType
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Error status or counter that isn't supported returns 0xFFFFFFFF
*
*/
static GT_STATUS prvCpssPxDiagDataIntegrityErrorInfoGet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT        memType,
    IN  CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT  *locationPtr,
    OUT GT_U32                                          *errorCounterPtr,
    OUT GT_U32                                          *failedRowPtr,
    OUT GT_U32                                          *failedSegmentPtr,
    OUT GT_U32                                          *failedSyndromePtr
)
{
    GT_STATUS                   rc = GT_OK;  /* return code */
    GT_U32                      regAddr;     /* register address */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC *dbItemPtr; /* pointer to map DB item */
    /* init counter/status */
    if (errorCounterPtr)
    {
        *errorCounterPtr = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS;
    }
    if (failedRowPtr)
    {
        *failedRowPtr = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS;
    }
    if (failedSegmentPtr)
    {
        *failedSegmentPtr = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS;
    }

    if (failedSyndromePtr)
    {
        *failedSyndromePtr = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS;
    }

/* List of non DFX memories:
  CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_HEADER_IDDB_MEM_E:
  CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_PAYLOAD_IDDB_MEM_E:
  CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E:
  CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E:
  CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E:
  CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HEADER_REORDER_MEM_IDDB_MEM_E:
  CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_PAYLOAD_REORDER_MEM_IDDB_MEM_E:
  CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E:
*/
    switch(memType)
    {
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BM_VALID_TABLE_E:
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_HEADER_REORDER_MEM_IDDB_MEM_E:
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXDMA_PAYLOAD_REORDER_MEM_IDDB_MEM_E:
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_HEADER_IDDB_MEM_E:
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXFIFO_PAYLOAD_IDDB_MEM_E:
            /* there is no registers to get the info about this memories  */
            return GT_OK;

        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_TXQ_QCN_BUFFER_FIFO_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->TXQ.qcn.CNBufferFIFOParityErrorsCntr;
                rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 32, errorCounterPtr);
             }

            return rc;
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_MPPM_BANK_RAM_E:
            /* read last failing group, bank, buffer fields ase one field  */
            if (failedRowPtr)
            {
                regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MPPM.dataIntegrity.lastFailingBuffer;
                rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 20, failedRowPtr);
                /* note that *failedRowPtr is composed from group[19:18], bank[17:16], buffer[15:0] */
            }
            /* read failed segment */
            if (rc == GT_OK && failedSegmentPtr)
            {
                regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MPPM.dataIntegrity.lastFailingSegment;
                rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, failedSegmentPtr);
            }
            /* read failed syndrome */
            if (rc == GT_OK && failedSyndromePtr)
            {
                regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->MPPM.dataIntegrity.statusFailedSyndrome;
                rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 10, failedSyndromePtr);
            }

            return rc;
        case CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_BMA_MC_COUNTERS_FLOOR_BANK_E:
            if (errorCounterPtr)
            {
                regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->BMA.BmaMCCNTParityErrorCounter1;
                rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 7, errorCounterPtr);
            }
            return rc;
        default:
            break;
    }
    /* handle DFX memories */

    /* check correctness of memType input parameter */
    rc = searchMemType(locationPtr->memLocation.dfxPipeId,
                       locationPtr->memLocation.dfxClientId,
                       locationPtr->memLocation.dfxMemoryId,
                       &dbItemPtr);
    if(GT_OK != rc)
    {
        return rc;
    }
    if (memType != dbItemPtr->memType)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxDiagDataIntegrityDfxErrorInfoGet(devNum, locationPtr,
                                                     failedRowPtr,
                                                     failedSyndromePtr);
    return rc;
}

/**
* @internal internal_cpssPxDiagDataIntegrityErrorInfoGet function
* @endinternal
*
* @brief   Function gets ECC/Parity error details. An error counter info getting
*         is performed per whole hw/logical table or memory. An entry line number
*         information - ramRow, logicalTableEntryIndex, hwTableEntryIndex - is
*         ignored. A failed row/segment/syndrome info is available for RAM
*         location type only.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
*                                      locationPtr        - (pointer to) memory location indexes
* @param[in] mppmMemLocationPtr       - MPPM memory coordinates. For memories other
*                                      than MPPM should be null!
*                                      (APPLICABLE DEVICES Lion2)
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
static GT_STATUS internal_cpssPxDiagDataIntegrityErrorInfoGet
(
    IN  GT_SW_DEV_NUM                                     devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC                *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_MPPM_MEMORY_LOCATION_STC *mppmMemLocationPtr,
    OUT GT_U32                                            *errorCounterPtr,
    OUT GT_U32                                            *failedRowPtr,
    OUT GT_U32                                            *failedSegmentPtr,
    OUT GT_U32                                            *failedSyndromePtr
)
{
    GT_STATUS                                       rc; /* return code */
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT        memType = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS;
    CPSS_DIAG_DATA_INTEGRITY_PX_MEMORY_LOCATION_UNT location;
    const CPSS_PX_TABLE_ENT                         *hwTablesPtr; /* pointer to array of hw tables names */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC              hwLocation; /* hw location */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC     *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */
    GT_U32                                          hwTable; /* CPSS_PX_TABLE_ENT or
                                                                PRV_CPSS_PX_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT */
    GT_BOOL isSingleMem = GT_FALSE;
    GT_U32  errorSum = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS;
    GT_U32  errorCounter;
    GT_U32  failedRow=0;
    GT_U32  failedSegment=0;
    GT_U32  failedSyndrome=0;
    GT_U32  i;
    GT_U32  j;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(memEntryPtr);
    CPSS_NULL_PTR_CHECK_MAC(errorCounterPtr);
    CPSS_NULL_PTR_CHECK_MAC(failedRowPtr);
    CPSS_NULL_PTR_CHECK_MAC(failedSegmentPtr);
    CPSS_NULL_PTR_CHECK_MAC(failedSyndromePtr);

    switch (memEntryPtr->type) {
        case CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE:
            /* call API for every hw table relevant to logical table */
            cpssOsMemSet(&hwLocation, 0, sizeof(hwLocation));
            hwLocation.type = CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE;
            /* get HW tables  relevant to logical table */
            rc = prvCpssPxDiagDataIntegrityTableLogicalToHwListGet(
                memEntryPtr->info.logicalEntryInfo.logicalTableType,
                &hwTablesPtr);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (NULL == hwTablesPtr)
            {
                /* something wrong with DB */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            for (j = 0; hwTablesPtr[j] != LAST_VALID_PX_TABLE_CNS; j++)
            {
                hwLocation.info.hwEntryInfo.hwTableType = hwTablesPtr[j];
                /* hwLocation.info.hwTableEntryIndex  doesn't matter */
                rc = internal_cpssPxDiagDataIntegrityErrorInfoGet(
                    devNum, &hwLocation,  NULL,
                    &errorCounter, &failedRow, &failedSegment, &failedSyndrome);
                if (rc != GT_OK)
                {
                    return rc;
                }
                /* accumulate sum */
                if (errorCounter != PRV_CPSS_PX_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS)
                {
                    if (errorSum !=  PRV_CPSS_PX_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS)
                    {
                        errorSum +=  errorCounter;
                    }
                    else
                    {
                        errorSum = errorCounter;
                    }
                }

            }
            *errorCounterPtr =  errorSum;
            return GT_OK;

        case CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE:
            isSingleMem =  GT_FALSE;
            hwTable = memEntryPtr->info.hwEntryInfo.hwTableType;
            break;

        case CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE:
            /* make compilers happy */
            hwTable = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS;

            isSingleMem =  GT_TRUE;
            memType = memEntryPtr->info.ramEntryInfo.memType;
            if (mppmMemLocationPtr)
            {
                location.mppmMemLocation.mppmId      = mppmMemLocationPtr->mppmId;
                location.mppmMemLocation.bankId      = mppmMemLocationPtr->bankId;
            }
            else
            {
                location.memLocation.dfxPipeId   = memEntryPtr->info.ramEntryInfo.memLocation.dfxPipeId;
                location.memLocation.dfxClientId = memEntryPtr->info.ramEntryInfo.memLocation.dfxClientId;
                location.memLocation.dfxMemoryId = memEntryPtr->info.ramEntryInfo.memLocation.dfxMemoryId;
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* handle either RAM location or HW Table location */
    if (isSingleMem == GT_TRUE)
    {
        dbArrayEntryNum = 1; /* the cycle below will be iterated once */
        dbArrayPtr = NULL;
    }
    else
    {
        prvCpssPxDiagDataIntegrityDbPointerSet(&dbArrayPtr, &dbArrayEntryNum);
    }

    for(i = 0; i < dbArrayEntryNum; i++)
    {
        errorCounter = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS; /* initialization */
        if (isSingleMem == GT_FALSE && dbArrayPtr)
        {
            if  (dbArrayPtr[i].hwTableName == hwTable)
            {
                memType = dbArrayPtr[i].memType;
                location.memLocation.dfxPipeId   = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_PIPE_GET(dbArrayPtr[i].key);
                location.memLocation.dfxClientId = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_CLIENT_GET(dbArrayPtr[i].key);
                location.memLocation.dfxMemoryId = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_MEMORY_GET(dbArrayPtr[i].key);
            }
            else
            {
                continue;
            }
        }
        rc =  prvCpssPxDiagDataIntegrityErrorInfoGet(
                        devNum, memType, &location,
                        &errorCounter, &failedRow, &failedSegment, &failedSyndrome);

        if (rc != GT_OK)
        {
            return rc;
        }

        /* accumulate errors sum */
        if (errorCounter != PRV_CPSS_PX_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS)
        {
            if (errorSum !=  PRV_CPSS_PX_DIAG_DATA_INTEGRITY_UNASSIGNED_CNS)
            {
                errorSum +=  errorCounter;
            }
            else
            {
                errorSum = errorCounter;
            }
        }
    }


    *errorCounterPtr = errorSum;
    if (isSingleMem ==  GT_TRUE)
    {
        *failedRowPtr      = failedRow;
        *failedSegmentPtr  = failedSegment;
        *failedSyndromePtr = failedSyndrome;
    }

    return GT_OK;
}

/**
* @internal cpssPxDiagDataIntegrityErrorInfoGet function
* @endinternal
*
* @brief   Function gets ECC/Parity error details. An error counter info getting
*         is performed per whole hw/logical table or memory. An entry line number
*         information - ramRow, logicalTableEntryIndex, hwTableEntryIndex - is
*         ignored. A failed row/segment/syndrome info is available for RAM
*         location type only.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] memEntryPtr              - (pointer to) memory (table) location
*                                      locationPtr        - (pointer to) memory location indexes
* @param[in] mppmMemLocationPtr       - MPPM memory coordinates. For memories other
*                                      than MPPM should be null!
*                                      (APPLICABLE DEVICES Lion2)
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagDataIntegrityErrorInfoGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memEntryPtr,mppmMemLocationPtr,
                            errorCounterPtr, failedRowPtr, failedSegmentPtr,
                            failedSyndromePtr));

    rc = internal_cpssPxDiagDataIntegrityErrorInfoGet(devNum, memEntryPtr, mppmMemLocationPtr,
            errorCounterPtr, failedRowPtr, failedSegmentPtr, failedSyndromePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memEntryPtr,
                              errorCounterPtr, failedRowPtr, failedSegmentPtr,
                              failedSyndromePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxDiagDataIntegrityErrorInjectionConfigSet function
* @endinternal
*
* @brief   Function enables/disables injection of error during next write operation.
*         Injection is applied to whole hw/logical table or to all memories
*         of specified type.
*         RAM DFX coordinates - pipeId, clientId, memoryId - and an entry index
*         info - ramRow, logicalTableEntryIndex, hwTableEntryIndex - are ignored.
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
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*
* @note Memory with parity can not be configured with multiple error injection
*
*/
static GT_STATUS internal_cpssPxDiagDataIntegrityErrorInjectionConfigSet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC              *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  injectMode,
    IN  GT_BOOL                                         injectEnable
)
{
    GT_STATUS                   rc;          /* return code */
    GT_STATUS                   rc1;         /* return code */
    GT_U32                      regData;     /* register data */
    GT_U32                      pipeIndex;   /* pipe index */
    GT_U32                      clientIndex; /* client index */
    GT_U32                      memoryIndex; /* memory index */
    GT_U32                      i;           /* loop iterator */
    GT_U32                      j;           /* loop iterator */
    GT_BOOL                     memoryFound = GT_FALSE; /* memory found status */
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType; /* memory type */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC   *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                        dbArrayEntryNum; /* size of data integrity DB */
    GT_U32                                        hwTable; /* CPSS_PX_TABLE_ENT or CPSS_PX_INTERNAL_TABLE_ENT */
    const CPSS_PX_TABLE_ENT                       *hwTablesPtr; /* pointer to array of hw tables names */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            hwLocation;   /* hw location */
    GT_U32                                        hwTableInDb;  /* value of HW Table in DB entry */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(memEntryPtr);

    if (injectMode != CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E &&
        injectMode != CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (memEntryPtr->type) {
        case CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE:
            /* call cpssPxDiagDataIntegrityErrorInjectionConfigSet for every
               hw table relevant to logical table */
            cpssOsMemSet(&hwLocation, 0, sizeof(hwLocation));
            hwLocation.type = CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE;

            rc = prvCpssPxDiagDataIntegrityTableLogicalToHwListGet(
                memEntryPtr->info.logicalEntryInfo.logicalTableType,
                &hwTablesPtr);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (NULL == hwTablesPtr)
            {
                /* something wrong with DB */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            /* if some table doesn't support the feature DB skip it.
               But if ALL tables are skipped return error */
            rc1 = GT_NOT_FOUND;
            for (j=0; hwTablesPtr[j] != LAST_VALID_PX_TABLE_CNS; j++)
            {
                hwLocation.info.hwEntryInfo.hwTableType = hwTablesPtr[j];
                /* hwLocation.info.hwTableEntryIndex  doesn't matter */
                rc = internal_cpssPxDiagDataIntegrityErrorInjectionConfigSet(
                    devNum, &hwLocation, injectMode, injectEnable);
                if (rc == GT_NOT_SUPPORTED || rc == GT_NOT_FOUND)
                {
                    continue;
                }
                else if (rc != GT_OK)
                {
                    return rc;
                }
                rc1 = rc;
            }
            /* no table that supports Injection */
            if (rc1 != GT_OK)
            {
                return rc1;
            }
         return GT_OK;


        case CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE:
            hwTable = memEntryPtr->info.hwEntryInfo.hwTableType;
            memType = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_NONEXISTENT_MEM_TYPE_CNS;
            break;

        case CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE:
            hwTable = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_NONEXISTENT_HW_TABLE_CNS;
            memType = memEntryPtr->info.ramEntryInfo.memType;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* the code below is relevant to HW or RAM locations. */
    prvCpssPxDiagDataIntegrityDbPointerSet(&dbArrayPtr, &dbArrayEntryNum);

    for(i = 0; i < dbArrayEntryNum; i++)
    {
        hwTableInDb = dbArrayPtr[i].hwTableName;

        if ((dbArrayPtr[i].memType == memType) || (hwTableInDb == hwTable))
        {
            memoryFound = GT_TRUE;
            pipeIndex   = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_PIPE_GET(dbArrayPtr[i].key);
            clientIndex = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_CLIENT_GET(dbArrayPtr[i].key);
            memoryIndex = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_MEMORY_GET(dbArrayPtr[i].key);

            /* to switch error injection modes <Error Injection Enable> must be set to Disable */
            /* write to Memory Control Register - disable error injection */
            rc = prvCpssDfxMemorySetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
                      PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_CONTROL_REG_CNS,
                      1, 1, 0);
            if(GT_OK != rc)
            {
                return rc;
            }

            if(dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E)
            {
                regData = 0;
            }
            else
            {
                if (injectMode == CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E)
                {
                    regData = 0;
                }
                else if (injectMode == CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E)
                {
                    regData = 1;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }

            /* write to Memory Control Register - error injection mode configuration */
            rc = prvCpssDfxMemorySetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
                      PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_CONTROL_REG_CNS,
                      2,1, regData);
            if(GT_OK != rc)
            {
                return rc;
            }

            if(injectEnable == GT_TRUE)
            {
                /* write to Memory Control Register - enable error injection */
                rc = prvCpssDfxMemorySetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
                          PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_CONTROL_REG_CNS,
                          1, 1, 1);

                if(GT_OK != rc)
                {
                    return rc;
                }
            }

            /* it's take some time in DFX bus to configure register.
               Add this Read to guaranty that write done before exit from the function. */
            rc = prvCpssDfxMemoryGetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
                      PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_CONTROL_REG_CNS,
                      1, 1, &regData);

            if(GT_OK != rc)
            {
                return rc;
            }
        }
    }

    if (memoryFound == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxDiagDataIntegrityErrorInjectionConfigSet
*           function
* @endinternal
*
* @brief   Function enables/disables injection of error during next write operation.
*         Injection is applied to whole hw/logical table or to all memories
*         of specified type.
*         RAM DFX coordinates - pipeId, clientId, memoryId - and an entry index
*         info - ramRow, logicalTableEntryIndex, hwTableEntryIndex - are ignored.
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
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagDataIntegrityErrorInjectionConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memEntryPtr, injectMode, injectEnable));

    rc = internal_cpssPxDiagDataIntegrityErrorInjectionConfigSet(devNum,
                                    memEntryPtr, injectMode, injectEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memEntryPtr, injectMode, injectEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal
*           internal_cpssPxDiagDataIntegrityErrorInjectionConfigGet
*           function
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
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
static GT_STATUS internal_cpssPxDiagDataIntegrityErrorInjectionConfigGet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC              *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  *injectModePtr,
    OUT GT_BOOL                                         *injectEnablePtr
)
{
    GT_STATUS                   rc;          /* return code */
    GT_U32                      regData;     /* register data */
    GT_U32                      pipeIndex;   /* pipe index */
    GT_U32                      clientIndex; /* client index */
    GT_U32                      memoryIndex; /* memory index */
    GT_U32                      i;           /* loop iterator */
    GT_BOOL                     memoryFound = GT_FALSE; /* memory found status */
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType; /* memory type */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */
    GT_U32                                          hwTable; /* CPSS_PX_TABLE_ENT or
                                                                PRV_CPSS_PX_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT */
    const CPSS_PX_TABLE_ENT                       *hwTablesPtr; /* pointer to array of hw tables names */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            hwLocation; /* hw location */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(memEntryPtr);
    CPSS_NULL_PTR_CHECK_MAC(injectModePtr);
    CPSS_NULL_PTR_CHECK_MAC(injectEnablePtr);

    switch (memEntryPtr->type) {
        case CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE:

            /* get list of HW tables from first logical */

            rc = prvCpssPxDiagDataIntegrityTableLogicalToHwListGet(
                memEntryPtr->info.logicalEntryInfo.logicalTableType,
                &hwTablesPtr);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (NULL == hwTablesPtr)
            {
                /* something wrong with DB */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            /* return as result the status of first HW table supporting the error injection */
            cpssOsMemSet(&hwLocation, 0, sizeof(hwLocation));
            hwLocation.type = CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE;
            for (i=0; hwTablesPtr[i] != LAST_VALID_PX_TABLE_CNS; i++)
            {
                hwLocation.info.hwEntryInfo.hwTableType = hwTablesPtr[i];
                /* hwLocation.info.hwTableEntryIndex  doesn't matter */

                rc = internal_cpssPxDiagDataIntegrityErrorInjectionConfigGet (
                    devNum, &hwLocation, injectModePtr, injectEnablePtr);
                /* ignore tables either not found in DB  or not supporting the feature */
                if (rc != GT_NOT_SUPPORTED && rc != GT_NOT_FOUND)
                {
                    return rc;
                }
            }
            /* done  */
            return rc;

        case CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE:
            memType = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_NONEXISTENT_MEM_TYPE_CNS;
            hwTable = memEntryPtr->info.hwEntryInfo.hwTableType;
            break;
        case CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE:
            hwTable = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_NONEXISTENT_HW_TABLE_CNS;
            memType = memEntryPtr->info.ramEntryInfo.memType;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssPxDiagDataIntegrityDbPointerSet(&dbArrayPtr, &dbArrayEntryNum);

    for(i = 0; i < dbArrayEntryNum; i++)
    {
        /* find first entry of given memType/hwTable */
        if ((dbArrayPtr[i].memType == memType) || (dbArrayPtr[i].hwTableName == hwTable))
        {
            memoryFound = GT_TRUE;

            pipeIndex   = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_PIPE_GET(dbArrayPtr[i].key);
            clientIndex = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_CLIENT_GET(dbArrayPtr[i].key);
            memoryIndex = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_MEMORY_GET(dbArrayPtr[i].key);


            /* read Memory Control Register, */
            rc = prvCpssDfxMemoryRegRead(devNum, pipeIndex, clientIndex, memoryIndex,
                                         PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_CONTROL_REG_CNS,
                                         &regData);
            if(GT_OK != rc)
            {
                return rc;
            }

            *injectModePtr = (U32_GET_FIELD_MAC(regData, 2, 1) == 1) ?
                CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E :
                CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_SINGLE_E;

            *injectEnablePtr = (U32_GET_FIELD_MAC(regData, 1, 1) == 1) ? GT_TRUE : GT_FALSE;

            break;
        }
    }

    if (memoryFound == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

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
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
GT_STATUS cpssPxDiagDataIntegrityErrorInjectionConfigGet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC              *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_ENT  *injectModePtr,
    OUT GT_BOOL                                         *injectEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagDataIntegrityErrorInjectionConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memEntryPtr, injectModePtr, injectEnablePtr));

    rc = internal_cpssPxDiagDataIntegrityErrorInjectionConfigGet(devNum,
                             memEntryPtr, injectModePtr, injectEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memEntryPtr, injectModePtr, injectEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxDiagDataIntegrityErrorCountEnableSet function
* @endinternal
*
* @brief   Function enables/disable error counter. Enabling/disabling is performed
*         for whole hw/logical table or for all memories of specified type.
*         RAM DFX coordinates - pipeId, clientId, memoryId - and an entry index
*         info - ramRow, logicalTableEntryIndex, hwTableEntryIndex - are ignored.
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
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
static GT_STATUS internal_cpssPxDiagDataIntegrityErrorCountEnableSet
(
    IN  GT_SW_DEV_NUM                                  devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC             *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT  errorType,
    IN  GT_BOOL                                        countEnable
)
{
    GT_STATUS                   rc;          /* return code */
    GT_STATUS                   rc1;         /* return code */
    GT_U32                      regData;     /* register data */
    GT_U32                      pipeIndex;   /* DFX pipe number */
    GT_U32                      clientIndex; /* DFX client number */
    GT_U32                      memoryIndex; /* DFX memory number */
    GT_U32                      fieldOffset; /* register field offset */
    GT_U32                      fieldLength; /* register field length */
    GT_U32                      i;           /* loop iterator */
    GT_U32                      j;           /* loop iterator */
    GT_BOOL                     memoryFound = GT_FALSE; /* memory found status */
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT        memType; /* memory type */
    GT_U32                                          hwTable; /* CPSS_PX_TABLE_ENT or
                                                                PRV_CPSS_PX_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC      *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */
    const CPSS_PX_TABLE_ENT                         *hwTablesPtr; /* pointer to array of hw tables names */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC              hwLocation; /* hw location */
    GT_U32                                          hwTableInDb;  /* value of HW Table in DB entry */
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(memEntryPtr);

    switch (memEntryPtr->type) {
        case CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE:

            /* call API for every hw table relevant to logical table */
            cpssOsMemSet(&hwLocation, 0, sizeof(hwLocation));
            hwLocation.type = CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE;

            rc = prvCpssPxDiagDataIntegrityTableLogicalToHwListGet(
                memEntryPtr->info.logicalEntryInfo.logicalTableType,
                &hwTablesPtr);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (NULL == hwTablesPtr)
            {
                /* something wrong with DB */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            /* if some table doesn't support the feature DB skip it.
               But if ALL tables are skipped return error */
            rc1 = GT_NOT_FOUND;
            for (j=0; hwTablesPtr[j] != LAST_VALID_PX_TABLE_CNS; j++)
            {
                hwLocation.info.hwEntryInfo.hwTableType = hwTablesPtr[j];
                /* hwLocation.info.hwTableEntryIndex  doesn't matter */
                rc = internal_cpssPxDiagDataIntegrityErrorCountEnableSet(
                    devNum, &hwLocation, errorType, countEnable);
                if (rc == GT_NOT_SUPPORTED || rc == GT_NOT_FOUND)
                {
                    continue;
                }
                else if (rc != GT_OK)
                {
                    return rc;
                }
                rc1 = rc;
             }
                /* no table that supports counter enabling */
                if (rc1 != GT_OK)
                {
                    return rc1;
                }
            /* done */
            return GT_OK;

        case CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE:
            memType = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_NONEXISTENT_MEM_TYPE_CNS;
            hwTable = memEntryPtr->info.hwEntryInfo.hwTableType;

            break;

        case CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE:
            hwTable = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_NONEXISTENT_HW_TABLE_CNS;
            memType = memEntryPtr->info.ramEntryInfo.memType;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    };

    prvCpssPxDiagDataIntegrityDbPointerSet(&dbArrayPtr, &dbArrayEntryNum);

    for(i = 0; i < dbArrayEntryNum; i++)
    {
        hwTableInDb = dbArrayPtr[i].hwTableName;
        if ((dbArrayPtr[i].memType == memType) || (hwTableInDb == hwTable))
        {
            memoryFound = GT_TRUE;
            /* non-DFX memories doesn't support counter disable */
            if (dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
            {
                /* device specific handling of some special types of non-DFX memories */


                /* device independent handling of rest of non-DFX memory types */
                if (dbArrayPtr[i].externalProtectionType ==
                    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
                {
                    /* the memory is not protected at all (neither DFX nor non-DFX). */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                }
                else if (countEnable == GT_TRUE)
                {
                    return GT_OK;
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                }
            }


            pipeIndex   = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_PIPE_GET(dbArrayPtr[i].key);
            clientIndex = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_CLIENT_GET(dbArrayPtr[i].key);
            memoryIndex = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_MEMORY_GET(dbArrayPtr[i].key);

             /* NOTE: In order to set configuration to other value then Disable,
                     the follow configuration bits must be set to Disable in
                     advanced: <Address Monitoring Int Select> bits 12-13 in Memory Control Register: */
            if(countEnable == GT_TRUE)
            {
                /* write to Memory Control Register - disable address monitoring mechanism */
                rc = prvCpssDfxMemorySetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
                                                 PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_CONTROL_REG_CNS,
                                                 12, 2,0);
                if(GT_OK != rc)
                {
                    return rc;
                }
            }

            if(dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
            {
                fieldOffset = 5;
                fieldLength = 2;

                switch(errorType)
                {
                    case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E:
                        regData = BOOL2BIT_MAC(countEnable);
                        break;

                    case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E:
                        regData = BOOL2BIT_MAC(countEnable) ? 2 : 0;
                        break;

                    case CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E:
                        regData = BOOL2BIT_MAC(countEnable) ? 3 : 0;
                        break;

                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
            else if(dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E)
            {
                if(errorType == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E)
                {
                    fieldOffset = 6;
                    fieldLength = 1;
                    regData = BOOL2BIT_MAC(countEnable);
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* write to Memory Control Register - enable/disable error counter */
            rc = prvCpssDfxMemorySetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
                                             PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_CONTROL_REG_CNS,
                                             fieldOffset, fieldLength, regData);
            if(GT_OK != rc)
            {
                return rc;
            }

            /* it's take some time in DFX bus to configure register.
               Add this Read to guaranty that write done before exit from the function. */
            rc = prvCpssDfxMemoryGetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
                      PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_CONTROL_REG_CNS,
                      fieldOffset, fieldLength, &regData);

            if(GT_OK != rc)
            {
                return rc;
            }
        }
    }

    if (memoryFound == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxDiagDataIntegrityErrorCountEnableSet function
* @endinternal
*
* @brief   Function enables/disable error counter. Enabling/disabling is performed
*         for whole hw/logical table or for all memories of specified type.
*         RAM DFX coordinates - pipeId, clientId, memoryId - and an entry index
*         info - ramRow, logicalTableEntryIndex, hwTableEntryIndex - are ignored.
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
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
GT_STATUS cpssPxDiagDataIntegrityErrorCountEnableSet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC              *memEntryPtr,
    IN  CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT   errorType,
    IN  GT_BOOL                                         countEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagDataIntegrityErrorCountEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memEntryPtr, errorType, countEnable));

    rc = internal_cpssPxDiagDataIntegrityErrorCountEnableSet(devNum,
                                           memEntryPtr, errorType, countEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memEntryPtr, errorType, countEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxDiagDataIntegrityErrorCountEnableGet function
* @endinternal
*
* @brief   Function gets status of error counter per whole
*          hw/logical table or memory.
*         An entry line number -
*         ramRow, logicalTableEntryIndex, hwTableEntryIndex -
*         are ignored.
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
* @retval GT_BAD_VALUE             - if HW tables composing logical table
*                                       have different error enabling status
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
static GT_STATUS internal_cpssPxDiagDataIntegrityErrorCountEnableGet
(
    IN  GT_SW_DEV_NUM                                 devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT *errorTypePtr,
    OUT GT_BOOL                                       *countEnablePtr
)
{
    GT_STATUS                   rc;          /* return code */
    GT_STATUS                   rc1;         /* return code */
    GT_U32                      regData;     /* register data */
    GT_U32                      pipeIndex;   /* DFX pipe number */
    GT_U32                      clientIndex; /* DFX client number */
    GT_U32                      memoryIndex; /* DFX memory number */
    GT_U32                      i;           /* loop iterator */
    GT_U32                      j;           /* loop iterator */
    GT_BOOL                     memoryFound = GT_FALSE; /* memory found status */
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT      memType; /* memory type */
    GT_U32                                        hwTable; /* CPSS_PX_TABLE_ENT or
                                                                PRV_CPSS_PX_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                        dbArrayEntryNum; /* size of data integrity DB */
    const CPSS_PX_TABLE_ENT                       *hwTablesPtr; /* pointer to array of hw tables names */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            hwLocation; /* hw location */
    CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT errorType;
    GT_BOOL                                       countEnable;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(memEntryPtr);
    CPSS_NULL_PTR_CHECK_MAC(countEnablePtr);
    CPSS_NULL_PTR_CHECK_MAC(errorTypePtr);

    switch (memEntryPtr->type) {
        case CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE:
            cpssOsMemSet(&hwLocation, 0, sizeof(hwLocation));

            hwLocation.type = CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE;

            rc = prvCpssPxDiagDataIntegrityTableLogicalToHwListGet(
                memEntryPtr->info.logicalEntryInfo.logicalTableType,
                    &hwTablesPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }

                if (NULL == hwTablesPtr)
                {
                    /* something wrong with DB */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }

                rc1 = GT_NOT_FOUND;
                for (j = 0; hwTablesPtr[j] != LAST_VALID_PX_TABLE_CNS; j++)
                {
                    hwLocation.info.hwEntryInfo.hwTableType = hwTablesPtr[j];
                    /* hwLocation.info.hwTableEntryIndex  doesn't matter */

                    rc = internal_cpssPxDiagDataIntegrityErrorCountEnableGet(
                        devNum, &hwLocation, &errorType, &countEnable);
                    /* ignore tables not supporting error counting */
                    if (rc == GT_NOT_SUPPORTED || rc == GT_NOT_FOUND)
                    {
                        continue;
                    }
                    else if (rc != GT_OK)
                    {
                        return rc;
                    }

                    if (rc1 != GT_OK)
                    {
                        /* this is first table supporting the feature */
                        rc1 = rc;
                        *errorTypePtr = errorType;
                        *countEnablePtr = countEnable;
                    }
                    else if (*countEnablePtr != countEnable)
                    {
                        /* logical table has HW tables with different counter status */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
                    }
                    else if (*errorTypePtr != errorType)
                    {

                        /* don't treat the case when some table(s) has errorType SINGLE_AND_MULTIPLE_ECC
                         * and rest of tables have errorType any kind of ECC - SINGLE or MULTIPLE - as error.
                         * The logical table is treated as SINGLE_AND_MULTIPLE_ECC errorType in such cases */
                        if (
                            (*errorTypePtr == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E &&
                             errorType     != CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E /* any kind of ECC */)
                            ||
                            (errorType     == CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E &&
                             *errorTypePtr != CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E /* any kind of ECC */))
                        {
                            *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                        }
                        else
                        {
                            /* logical table has HW tables with different errorType */
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
                        }
                    }
                }
                /* neither of tables supports masking */
                if (rc1 != GT_OK)
                {
                    return rc1;
                }
            /* done */
            return GT_OK;

        case CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE:
            memType = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_NONEXISTENT_MEM_TYPE_CNS;
            hwTable = memEntryPtr->info.hwEntryInfo.hwTableType;

            break;

        case CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE:
            hwTable = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_NONEXISTENT_HW_TABLE_CNS;
            memType = memEntryPtr->info.ramEntryInfo.memType;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    };

    prvCpssPxDiagDataIntegrityDbPointerSet(&dbArrayPtr, &dbArrayEntryNum);

    for(i = 0; i < dbArrayEntryNum; i++)
    {
        /* find first entry of given memType/hwTable */
        if ((dbArrayPtr[i].memType == memType) || (dbArrayPtr[i].hwTableName == hwTable))
        {
            memoryFound = GT_TRUE;

            if (dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E)
            {
                /* device independent handling of rest of non-DFX memory types */
                switch (dbArrayPtr[i].externalProtectionType)
                {
                    case CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E:
                        /* the memory is not protected at all (neither DFX nor non-DFX). */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

                    case CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E:
                        *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                        break;
                    default:
                        *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                        break;
                }
                /* non-DFX memories don't support counter disable */
                *countEnablePtr = GT_TRUE;
                return GT_OK;
            }

            pipeIndex   = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_PIPE_GET(dbArrayPtr[i].key);
            clientIndex = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_CLIENT_GET(dbArrayPtr[i].key);
            memoryIndex = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_MEMORY_GET(dbArrayPtr[i].key);

            /* read from Memory Control Register - enable/disable error counter */
            rc = prvCpssDfxMemoryGetRegField(devNum, pipeIndex, clientIndex, memoryIndex,
                      PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_CONTROL_REG_CNS,
                      5, 2, &regData);
            if(GT_OK != rc)
            {
                return rc;
            }

            if(dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ECC_E)
            {
                switch(regData)
                {
                    case 0:
                        *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                        *countEnablePtr = GT_FALSE;
                        break;

                    case 1:
                        *countEnablePtr = GT_TRUE;
                        *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
                        break;

                    case 2:
                        *countEnablePtr = GT_TRUE;
                        *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
                        break;

                    default:
                        *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_AND_MULTIPLE_ECC_E;
                        *countEnablePtr = GT_TRUE;
                        break;
                }
            }
            else if(dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_PARITY_E)
            {
                *errorTypePtr = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
                *countEnablePtr = BIT2BOOL_MAC((regData >> 1) & 0x1);
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            break;
        }
    }

    if (memoryFound == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxDiagDataIntegrityErrorCountEnableGet function
* @endinternal
*
* @brief   Function gets status of error counter. Status getting is performed
*         to per whole hw/logical table or memory. An entry line number -
*         ramRow, logicalTableEntryIndex, hwTableEntryIndex - is ignored.
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
* @retval GT_BAD_VALUE             - if HW tables composing logical table
*                                       have different error enabling status
* @retval GT_FAIL                  - if logical table doesn't contain HW tables
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
GT_STATUS cpssPxDiagDataIntegrityErrorCountEnableGet
(
    IN  GT_SW_DEV_NUM                                 devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC            *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_ENT *errorTypePtr,
    OUT GT_BOOL                                       *countEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagDataIntegrityErrorCountEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memEntryPtr, errorTypePtr, countEnablePtr));

    rc = internal_cpssPxDiagDataIntegrityErrorCountEnableGet(devNum,
                                     memEntryPtr, errorTypePtr, countEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memEntryPtr, errorTypePtr, countEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxDiagDataIntegrityProtectionTypeGet function
* @endinternal
*
* @brief   Function gets memory protection type.
*         In case of "logical table":
*         if some of HW tables composing logical table are not protected
*         and some of HW tables are protected (with the same protection type)
*         - return protection type of protected tables
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
* @retval GT_BAD_VALUE             - if logical table consists of HW tables
*                                       with different protection types (parity or
*                                       ECC). So protectionTypePtr can't be
*                                       assigned uniquely.
*/
static GT_STATUS internal_cpssPxDiagDataIntegrityProtectionTypeGet
(
    IN  GT_SW_DEV_NUM                                           devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC                      *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  *protectionTypePtr
)
{
    GT_U32                                          i, j;    /* loop iterators */
    GT_STATUS                                       rc;      /* return code    */
    GT_STATUS                                       rc1;     /* return code    */
    CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT        memType; /* memory type    */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC      *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */
    GT_U32                                          hwTable; /* CPSS_PX_TABLE_ENT or
                                                                PRV_CPSS_PX_DIAG_DATA_INTEGRITY_SPECIAL_TABLE_ENT */
    CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  protectionType;
    const CPSS_PX_TABLE_ENT                       *hwTablesPtr; /* pointer to array of hw tables names */
    CPSS_PX_LOCATION_SPECIFIC_INFO_STC            hwLocation; /* hw location */


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(memEntryPtr);
    CPSS_NULL_PTR_CHECK_MAC(protectionTypePtr);

    switch (memEntryPtr->type) {
        case CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE:
            cpssOsMemSet(&hwLocation, 0, sizeof(hwLocation));
            hwLocation.type = CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE;

            rc = prvCpssPxDiagDataIntegrityTableLogicalToHwListGet(
                memEntryPtr->info.logicalEntryInfo.logicalTableType,
                &hwTablesPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (NULL == hwTablesPtr)
            {
                /* something wrong with DB */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            /* if some table doesn't support the feature skip it.
                       But if ALL tables are skipped return error */
            rc1 = GT_NOT_FOUND;
            for (j = 0; hwTablesPtr[j] != LAST_VALID_PX_TABLE_CNS; j++)
            {
                hwLocation.info.hwEntryInfo.hwTableType = hwTablesPtr[j];
                /* hwLocation.info.hwTableEntryIndex  doesn't matter */
                rc = internal_cpssPxDiagDataIntegrityProtectionTypeGet(
                    devNum, &hwLocation, &protectionType);
                if (rc == GT_NOT_SUPPORTED || rc == GT_NOT_FOUND)
                {
                       /* ignore table that not found in DB  or not supports the feature */
                    continue;
                }
                else if (rc != GT_OK)
                {
                    return rc;
                };

                if (rc1 != GT_OK)
                {
                    /* this is first table that supports the feature */
                    rc1 = rc;
                    *protectionTypePtr = protectionType;
                }
                else if ((*protectionTypePtr) != protectionType)
                {
                    /* logical table have HW tables with different protection type */
                    if (CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E
                        == *protectionTypePtr)
                    {
                        *protectionTypePtr = protectionType;
                    }
                    else if (CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E
                             != protectionType)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
                    }
                }
            }
                /* no table that supports the feature */
            if (rc1 != GT_OK)
            {
                return rc1;
            }
            /* done */
            return GT_OK;

        case CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE:
            memType = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_NONEXISTENT_MEM_TYPE_CNS;
            hwTable = memEntryPtr->info.hwEntryInfo.hwTableType;
            break;
        case CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE:
            hwTable = PRV_CPSS_PX_DIAG_DATA_INTEGRITY_NONEXISTENT_HW_TABLE_CNS;
            memType = memEntryPtr->info.ramEntryInfo.memType;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    prvCpssPxDiagDataIntegrityDbPointerSet(&dbArrayPtr, &dbArrayEntryNum);

    for(i = 0; i < dbArrayEntryNum; i++)
    {
        /* find first entry of given memType/hwTable */
        if ((dbArrayPtr[i].memType == memType) || (dbArrayPtr[i].hwTableName == hwTable))
        {
            *protectionTypePtr = (dbArrayPtr[i].protectionType == CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_NONE_E ?
                                  dbArrayPtr[i].externalProtectionType :
                                  dbArrayPtr[i].protectionType);
            break;
        }
    }

    if(i == dbArrayEntryNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    else
    {
        return GT_OK;
    }
}

/**
* @internal cpssPxDiagDataIntegrityProtectionTypeGet function
* @endinternal
*
* @brief   Function gets memory protection type.
*         In case of "logical table":
*         if some of HW tables composing logical table are not protected
*         and some of HW tables are protected (with the same protection type)
*         - return protection type of protected tables
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
* @retval GT_BAD_VALUE             - if logical table consists of HW tables
*                                       with different protection types (parity or
*                                       ECC). So protectionTypePtr can't be
*                                       assigned uniquely.
*/
GT_STATUS cpssPxDiagDataIntegrityProtectionTypeGet
(
    IN  GT_SW_DEV_NUM                                                   devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC                    *memEntryPtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  *protectionTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagDataIntegrityProtectionTypeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memEntryPtr, protectionTypePtr));

    rc = internal_cpssPxDiagDataIntegrityProtectionTypeGet(devNum, memEntryPtr, protectionTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memEntryPtr, protectionTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssPxDfxPipesBmpGet function
* @endinternal
*
* @brief   Return bitmap of DFX pipes relevant for the device .
*         If bit #N is raised pipeId #N exists in the device.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
*
* @param[out] pipesBmpPtr              - (pointer to) pipes bitmap
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvCpssPxDfxPipesBmpGet
(
    IN  GT_SW_DEV_NUM  devNum,
    OUT GT_U32 *pipesBmpPtr
)
{
    devNum = devNum; /* avoid compiler error*/
    /*  pipe 0 */
    *pipesBmpPtr = 1;
    return GT_OK;
}

/**
* @internal prvCpssPxDiagDataIntegrityMemoryIndexesGet function
* @endinternal
*
* @brief   Function gets all memory location coordinates for given memory type.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] memType                  - memory type
* @param[in,out] arraySizePtr        - in : size of input array
*                                      out: actual size of array
*
* @param[out] memLocationArr           - array of memory locations
* @param[out] protectionTypePtr        - (pointer to) protection memory type
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_SIZE              - if size of input array is not enough to store
*                                       coordinates of all found items
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
GT_STATUS prvCpssPxDiagDataIntegrityMemoryIndexesGet
(
    IN  GT_SW_DEV_NUM                                           devNum,
    IN  CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT                memType,
    INOUT GT_U32                                                *arraySizePtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC            *memLocationArr,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  *protectionTypePtr
)
{
    GT_U32                      i;
    GT_U32                      moduleCount = 0;
    GT_BOOL                     memoryFound = GT_FALSE;            /* memory found status */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr;     /* pointer to data integrity DB */
    GT_U32                                        dbArrayEntryNum; /* size of data integrity DB */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    prvCpssPxDiagDataIntegrityDbPointerSet(&dbArrayPtr, &dbArrayEntryNum);

    for(i = 0; i < dbArrayEntryNum; i++)
    {
        if(dbArrayPtr[i].memType == memType)
        {

            memoryFound = GT_TRUE;
            if(moduleCount >= *arraySizePtr)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, LOG_ERROR_NO_MSG);
            }

            *protectionTypePtr = dbArrayPtr[i].protectionType;
            memLocationArr[moduleCount].dfxPipeId   = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_PIPE_GET(dbArrayPtr[i].key);
            memLocationArr[moduleCount].dfxClientId = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_CLIENT_GET(dbArrayPtr[i].key);
            memLocationArr[moduleCount].dfxMemoryId = PRV_CPSS_DIAG_DATA_INTEGRITY_MAP_KEY_MEMORY_GET(dbArrayPtr[i].key);
            moduleCount++;
        }
    }

    *arraySizePtr = moduleCount;

    if (memoryFound == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


/**
* @internal prvCpssPxDiagDataIntegrityMemoryInstancesCountGet function
* @endinternal
*
* @brief   Function returns number of memory instances in DB.
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] memType                  - memory type
*
* @param[out] counterPtr               - (pointer to) memory counter
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
*/
GT_STATUS prvCpssPxDiagDataIntegrityMemoryInstancesCountGet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT        memType,
    OUT GT_U32                                          *counterPtr
)
{
    GT_U32                      i;
    GT_U32                      moduleCount = 0;
    GT_BOOL                     memoryFound = GT_FALSE; /* memory found status */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    prvCpssPxDiagDataIntegrityDbPointerSet(&dbArrayPtr, &dbArrayEntryNum);

    for(i = 0; i < dbArrayEntryNum; i++)
    {
        if(dbArrayPtr[i].memType == memType)
        {
            memoryFound = GT_TRUE;
            moduleCount++;
        }
    }

    *counterPtr = moduleCount;

    if (memoryFound == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


/**
* @internal prvCpssPxDiagDataIntegrityDfxErrorConvert function
* @endinternal
*
* @brief   Function converts from 'Specific RAM + failedRow(index)' to 'HW_table + index'
*         HW index means a value passed as IN-parameter "entryIndex" in API
*         prvCpssPxReadTableEntry/prvCpssPxWriteTableEntry.
*         I.e. it can differ from actual hardware geometry. See using of
*         VERTICAL_INDICATION_CNS, FRACTION_INDICATION_CNS, FRACTION_HALF_TABLE_INDICATION_CNS
*         for details
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] memLocationPtr           - (pointer to) DFX memory info
* @param[in] failedRow                - the failed row in the DFX memory.
*
* @param[out] hwErrorInfoPtr           - (pointer to) the converted 'HW_table + index'
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - the DFX location was not found in DB.
* @retval GT_EMPTY                 - the DFX location is not mapped to CPSS HW table
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxDiagDataIntegrityDfxErrorConvert
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC    *memLocationPtr,
    IN  GT_U32                                          failedRow,
    OUT CPSS_PX_HW_INDEX_INFO_STC                       *hwErrorInfoPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbItemPtr; /* pointer to map DB item */
    CPSS_PX_TABLE_ENT  hwTableType;   /* HW table type */
    GT_U32  tmpIndex;                   /* temp variable */
    GT_U32  hwTableEntryIndex;          /* HW table entry index */
    GT_U32  ramHwTableOffset;   /* first HW entry index for first line of RAM */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(memLocationPtr);
    CPSS_NULL_PTR_CHECK_MAC(hwErrorInfoPtr);
     /* Get entry index for DFX memories */
    rc = searchMemType(memLocationPtr->dfxPipeId  ,
                       memLocationPtr->dfxClientId,
                       memLocationPtr->dfxMemoryId,
                       &dbItemPtr);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "The dfx memory was not found");
    }

    cpssOsMemSet(hwErrorInfoPtr,0,sizeof(*hwErrorInfoPtr));

    ramHwTableOffset = 0;

    if(dbItemPtr->hwTableName < CPSS_PX_HW_TABLE_LAST_E)
    {
        hwTableType = dbItemPtr->hwTableName;
    }
    else
    {
        /* should not happen */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);

    }

    if (hwTableType == CPSS_PX_TABLE_PHA_FW_IMAGE_E)
    {
        /*for FW image hwIndex contains PPG numb in bits 14-15 and address offset in bits 0-13*/
       hwTableEntryIndex = (((memLocationPtr->dfxMemoryId  & 0xC ) << 12) + (failedRow << 6) + ((memLocationPtr->dfxMemoryId & 3) <<4));
    }
    else
    {
        /*calculate the entry index in the HW table*/
        /*  tmpIndex = firstTableLine + failedRow
         *  hwTableEntryIndex = startAddress +
         *                      (tmpIndex / logicalBlockSize) * logicalBlockAddressSteps +
         *                      (tmpIndex % logicalBlockSize) * addressIncrement
         *
         */
        tmpIndex = dbItemPtr->firstTableLine + failedRow;

        hwTableEntryIndex =
            dbItemPtr->startAddress +
            (tmpIndex / dbItemPtr->logicalBlockSize) * dbItemPtr->logicalBlockAddressSteps +
            (tmpIndex % dbItemPtr->logicalBlockSize) * dbItemPtr->addressIncrement;

        hwTableEntryIndex += ramHwTableOffset;

    }

    hwErrorInfoPtr->hwTableType = hwTableType;
    hwErrorInfoPtr->hwTableEntryIndex = hwTableEntryIndex;

    return GT_OK;
}

/**
* @internal prvCpssPxDiagDataIntegrityDfxParamsConvert function
* @endinternal
*
* @brief   Function converts from 'Specific RAM +
*          failedRow(index)' to 'HW_table that under CPSS
*          control + index'
*
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] memLocationPtr           - (pointer to) DFX memory info
* @param[in] failedRow                - the failed row in the DFX memory.
*
* @param[out] hwErrorInfoPtr           - (pointer to) the converted 'HW_table + index'
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - the DFX location was not found in DB.
* @retval GT_EMPTY                 - the DFX location is not mapped to CPSS HW table
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxDiagDataIntegrityDfxParamsConvert
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC    *memLocationPtr,
    IN  GT_U32                                          failedRow,
    OUT CPSS_PX_HW_INDEX_INFO_STC                       *hwErrorInfoPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbItemPtr; /* pointer to map DB item */
    CPSS_PX_TABLE_ENT  hwTableType;   /* HW table type */
    GT_U32  tmpIndex;                   /* temp variable */
    GT_U32  hwTableEntryIndex;          /* HW table entry index */
    GT_U32  ramHwTableOffset;   /* first HW entry index for first line of RAM */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(memLocationPtr);
    CPSS_NULL_PTR_CHECK_MAC(hwErrorInfoPtr);

    /* Get entry index for DFX memories */
    rc = searchMemType(memLocationPtr->dfxPipeId  ,
                       memLocationPtr->dfxClientId,
                       memLocationPtr->dfxMemoryId,
                       &dbItemPtr);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "The dfx memory was not found");
    }

    cpssOsMemSet(hwErrorInfoPtr,0,sizeof(*hwErrorInfoPtr));

    ramHwTableOffset = 0;

    if(dbItemPtr->hwTableName < CPSS_PX_TABLE_LAST_E)/* only the tables that under CPSS control*/
    {
        hwTableType = dbItemPtr->hwTableName;
    }
    else if (dbItemPtr->hwTableName <  CPSS_PX_HW_TABLE_LAST_E )
    {
        /*the memory hold no CPSS accessing support*/
        return /* this is not ERROR for the LOG !!! */ GT_EMPTY;
    }
    else
    {
        /* should not happen */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);

    }

    if (hwTableType == CPSS_PX_TABLE_PHA_FW_IMAGE_E)
    {
        /*for FW image hwIndex contains PPG numb in bits 14-15 and address offset in bits 0-13*/
       hwTableEntryIndex = (((memLocationPtr->dfxMemoryId  & 0xC ) << 12) + (failedRow << 6) + ((memLocationPtr->dfxMemoryId & 3) <<4));
    }
    else
    {
        /*calculate the entry index in the HW table*/
        /*  tmpIndex = firstTableLine + failedRow
         *  hwTableEntryIndex = startAddress +
         *                      (tmpIndex / logicalBlockSize) * logicalBlockAddressSteps +
         *                      (tmpIndex % logicalBlockSize) * addressIncrement
         *
         */
        tmpIndex = dbItemPtr->firstTableLine + failedRow;

        hwTableEntryIndex =
            dbItemPtr->startAddress +
            (tmpIndex / dbItemPtr->logicalBlockSize) * dbItemPtr->logicalBlockAddressSteps +
            (tmpIndex % dbItemPtr->logicalBlockSize) * dbItemPtr->addressIncrement;

        hwTableEntryIndex += ramHwTableOffset;
    }

    hwErrorInfoPtr->hwTableType = hwTableType;
    hwErrorInfoPtr->hwTableEntryIndex = hwTableEntryIndex;

    return GT_OK;
}

GT_STATUS test_prvCpssPxDiagDataIntegrityDfxErrorConvert
(
    IN  GT_SW_DEV_NUM                                           devNum
)
{
    GT_STATUS                                       rc;
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbArrayPtr; /* pointer to data integrity DB */
    GT_U32                                          dbArrayEntryNum; /* size of data integrity DB */
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *currentEntryPtr;
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *nextEntryPtr;
    CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC    currentRamInfo;
    GT_U32                                          failedRow;
    CPSS_PX_HW_INDEX_INFO_STC                     hwErrorInfo;
    static GT_U32                                   entryPtr[128];/*buffer for read entry*/
    GT_U32                                          ii,tmpIndex;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    prvCpssPxDiagDataIntegrityDbPointerSet(&dbArrayPtr, &dbArrayEntryNum);

    currentEntryPtr = &dbArrayPtr[0];

    for(ii = 0 ; ii < dbArrayEntryNum ; ii++ , currentEntryPtr++)
    {
        currentRamInfo.dfxPipeId   = U32_GET_FIELD_MAC(currentEntryPtr->key, 12, 3);
        currentRamInfo.dfxClientId = U32_GET_FIELD_MAC(currentEntryPtr->key, 7,  5);
        currentRamInfo.dfxMemoryId = U32_GET_FIELD_MAC(currentEntryPtr->key, 0,  7);

        /* use last index of this RAM ... try to get this info it from 'next entry' */
        failedRow = 31;
        nextEntryPtr = currentEntryPtr+1;
        tmpIndex = ii+1;
        while(1)
        {
            if(tmpIndex >= dbArrayEntryNum)
            {
                break;
            }

            if(nextEntryPtr->memType != currentEntryPtr->memType)
            {
                break;
            }

            if(nextEntryPtr->firstTableLine != 0)
            {
                failedRow = nextEntryPtr->firstTableLine - 1;
                break;
            }

            tmpIndex++;
            nextEntryPtr++;
        }

        rc = prvCpssPxDiagDataIntegrityDfxErrorConvert(devNum,
            &currentRamInfo,failedRow,
            &hwErrorInfo);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* try to access the table */
        rc = prvCpssPxReadTableEntry(devNum,
            hwErrorInfo.hwTableType,
            hwErrorInfo.hwTableEntryIndex,
            entryPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}


/**
* @internal prvCpssPxDfxClientsBmpGet function
* @endinternal
*
* @brief   return bitmap of DFX clients relevant for the specified DFX pipe.
*         If bit #N is raised client #N exists in the device.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] pipeId                   - pipe index
*
* @param[out] clientsBmpPtr            - (pinter to) clients bitmap
*
* @retval GT_OK                    - on success
* @retval GT_NOT_IMPLEMENTED       - if device is not supported by the function yet
*/
GT_STATUS prvCpssPxDfxClientsBmpGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   pipeId,
    OUT GT_U32   *clientsBmpPtr
)
{
    /* to avoid warnings*/
    devNum = devNum;
    pipeId = pipeId;

    *clientsBmpPtr = PRV_CPSS_PX_DATA_INTEGRITY_DFX_CLIENTS_BIT_MAP;
    return GT_OK;
}
#if 0
/**
* @internal prvCpssPxDiagDataIntegrityDfxEventsViaDfxServerGet function
* @endinternal
*
* @brief   Function scans DFX unit registers for DataIntegrity events details.
*         HW/Logical table coordinates are not filled here! Only RAM coordinates.
*         Unlike prvCpssPxDiagDataIntegrityDfxEventsViaMgUnitGet it doesn't
*         read MG unit registers "DFX/DFX1 Interrupts Summary Cause Register".
*         It reads DFX unit register "Server Interrupt Summary Cause Register"
*         instead.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in,out] eventsNumPtr        - in : max num of enabled events to
*                                      retrieve - this value refer to the number of
*                                      members that the array of eventsArr[]
*                                      can retrieve.
*                                      out: (pointer to) the actual num of found events
*
* @param[out] eventsArr[]              - (array of) events
* @param[out] isNoMoreEventsPtr        - (pointer to) status of events scan process
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - illegal HW state
*/
static GT_STATUS prvCpssPxDiagDataIntegrityDfxEventsViaDfxServerGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    INOUT GT_U32                                    *eventsNumPtr,
    OUT CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC       eventsArr[],
    OUT GT_BOOL                                     *isNoMoreEventsPtr
)
{
    GT_U32    regAddr;
    GT_U32    regData;
    GT_U32    clientRegData;
    GT_U32    eventCounter = 0; /* events counter */
    GT_U32    pipesBmp;
    GT_U32    pipeId;
    GT_U32    clientsBmp;
    GT_U32    clientId;
    GT_U32    memRangeIter;
    GT_U32    memBmp;
    GT_U32    memId;

    GT_STATUS rc;

    if (*eventsNumPtr == 0)
    {
        return GT_OK;
    }


    /* read Server Interrupt Summary Cause Register */
    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                       DFXServerUnits.DFXServerRegs.serverInterruptSummaryCause;
    rc = prvCpssDrvHwPpResetAndInitControllerReadReg(devNum, regAddr, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssPxDfxPipesBmpGet(devNum, &pipesBmp);
    if (rc != GT_OK)
    {
        return rc;
    }
    for (pipeId = 0; pipesBmp; pipeId++, pipesBmp >>= 1)
    {
        if (!(pipesBmp & 1))
        {
            continue;
        }

        /* bits 3..10 signals about interrupts in pipes 0..7. */
        if  (! (regData & (BIT_3 << pipeId)))
        {
            continue;
        }

        rc = prvCpssPxDfxClientsBmpGet(devNum, pipeId, &clientsBmp);
        if (rc != GT_OK)
        {
            return rc;
        }


        /* iterate all client: from 27 down to 0 */
        for (clientId = 27 + 1; clientId-- > 0;)
        {
            if (! ((clientsBmp >> clientId) & 1))
            {
                /* this client is not valid */
                continue;
            }

            rc = prvCpssDfxClientRegRead(
                devNum, pipeId, clientId,
                PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_CLIENT_INT_CAUSE_CNS,
                &clientRegData);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* check register bits 1..4 responsible for memories 0..127 interrupts
             * bit 1 - memories  0..31  (range 0)
             * bit 2 - memories 32..63  (range 1)
             * bit 3 - memories 64..95  (range 2)
             * bit 4 - memories 96..127 (range 3) */
            for (memRangeIter = 0; memRangeIter < 4; memRangeIter++)
            {
                if (clientRegData & (BIT_1 << memRangeIter))
                {
                    rc = prvCpssDfxClientRegRead(
                        devNum, pipeId, clientId,
                        PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_CLIENT_INT_STATUS0_REG_CNS + memRangeIter * 4,
                        &memBmp);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }

                    for (memId = memRangeIter * 32; memBmp; memId++, memBmp >>= 1)
                    {
                        if (memBmp & 0x1)
                        {
                            rc = prvCpssPxDiagDfxMemoryEventFromDbGet(
                                devNum, pipeId, clientId, memId, &eventsArr[eventCounter++]);
                            if (rc != GT_OK)
                            {
                                return rc;
                            }
                            if (eventCounter == *eventsNumPtr)
                            {
                                *isNoMoreEventsPtr = GT_FALSE;
                                return GT_OK;
                            }
                        }
                    }
                }
            }

            /* if bit #5 - "Previous Clients Interrupts Summary" - is not raised we can
             * skip rest of clients */
            if (! (clientRegData & BIT_5))
            {
                break;
            }
        }
    }

    *eventsNumPtr      = eventCounter;
    *isNoMoreEventsPtr = GT_TRUE;
    return GT_OK;
}
#endif
/**
* @internal prvCpssPxDiagDfxMemoryEventFromDbGet function
* @endinternal
*
* @brief   Function scans DFX unit registers for DataIntegrity event details.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] pipeId                   - DFX pipe index
* @param[in] clientId                 - DFX client index
* @param[in] memId                    - DFX memory index
*
* @param[out] eventPtr                 - (pointer to) occured event info
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssPxDiagDfxMemoryEventFromDbGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_U32                                      pipeId,
    IN  GT_U32                                      clientId,
    IN  GT_U32                                      memId,
    OUT CPSS_PX_DIAG_DATA_INTEGRITY_EVENT_STC     *eventPtr
)
{

    GT_U32 regData;
    GT_U32 rc;
    PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    *dbItemPtr; /* pointer to map DB item */

    /* Read Memory Interrupt Cause Register */
    rc = prvCpssDfxMemoryRegRead(
        devNum, pipeId, clientId, memId,
        PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_MEMORY_INT_CAUSE_REG_CNS,
        &regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = searchMemType(pipeId, clientId, memId, &dbItemPtr);
    if(GT_OK != rc)
    {
        return rc;
    }

    /* update output parameters */
    eventPtr->location.ramEntryInfo.memType                 = dbItemPtr->memType;
    eventPtr->location.ramEntryInfo.memLocation.dfxPipeId   = pipeId;
    eventPtr->location.ramEntryInfo.memLocation.dfxClientId = clientId;
    eventPtr->location.ramEntryInfo.memLocation.dfxMemoryId = memId;
    eventPtr->memoryUseType                                 = dbItemPtr->memUsageType;
    eventPtr->correctionMethod                              = dbItemPtr->correctionMethod;

    eventPtr->location.isMppmInfoValid = GT_FALSE;

    if(U32_GET_FIELD_MAC(regData, 1, 1))
    {
        eventPtr->eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E;
    }
    else if(U32_GET_FIELD_MAC(regData, 2, 1))
    {
        eventPtr->eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_SINGLE_ECC_E;
    }
    else if(U32_GET_FIELD_MAC(regData, 3, 1))
    {
        eventPtr->eventsType = CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_MULTIPLE_ECC_E;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}



/**
* @internal prvCpssPxDiagDataIntegrityDfxServerClientsIntMaskSet function
* @endinternal
*
* @brief   Mask/unmask bits indicating Data Integrity ECC/Parity errors in the
*         DFX registers "Server Interrupt Summary Mask Register",
*         "Client Interrupt Summary Mask Register".
*         It makes possible manage DFX memories events appearance inside this
*         registers via signle register - "Memory Interrupt Mask Resigter"
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] operation                - mask/unmask operation
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - if pipe number is out of range.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_IMPLEMENTED       - if feature is not implemented for the device yet
*/
GT_STATUS prvCpssPxDiagDataIntegrityDfxServerClientsIntMaskSet
(
    IN GT_U8                   devNum,
    IN CPSS_EVENT_MASK_SET_ENT operation
)
{
    GT_U32 pipesBmp; /* bitmap of all valid pipes */
    GT_U32 pipeId;
    GT_U32 clientsBmp; /* bitmap of all valid pipe's clients */
    GT_U32 clientId;
    GT_U32 regAddr;
    GT_U32 data;
    GT_STATUS rc;

    rc = prvCpssPxDfxPipesBmpGet(devNum, &pipesBmp);
    if (rc != GT_OK)
    {
        return rc;
    };

    if (pipesBmp >= BIT_7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* update Server Interrupt Summary Mask Register, bits 3..10
       which are responsible for masking pipes 0..7. */
    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
        DFXServerUnits.DFXServerRegs.serverInterruptSummaryMask;

    data = (CPSS_EVENT_UNMASK_E == operation) ? pipesBmp : 0;

    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField (devNum, regAddr, 3, 8, data);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* update Client Interrupt Summary Mask Register bits 0..5 for all clients:
     * Bits 1..4 summarize RAM interrupts for memories:
     *     0..31, 32..63, 64..95, 96..124
     * Bit 5 summarizes interrupts from previous client in the chain. */
    data =  (CPSS_EVENT_UNMASK_E == operation) ? 0x1f : 0;

    for (pipeId = 0; pipesBmp; pipeId++, pipesBmp >>= 1)
    {
        if (pipesBmp & 1)
        {
            /* iterate all clients 0..27 */
            rc = prvCpssPxDfxClientsBmpGet(devNum, pipeId, &clientsBmp);
            if (rc != GT_OK)
            {
                return rc;
            }

            for (clientId = 0; clientsBmp; clientId++, clientsBmp >>= 1)
            {
                if (clientsBmp & 1)
                {
                    rc = prvCpssDfxClientSetRegField(
                        devNum, pipeId, clientId,
                        PRV_CPSS_DIAG_DATA_INTEGRITY_ADDR_TYPE_CLIENT_INT_MASK_CNS,
                        1, 5, data);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
        }
    }
    return GT_OK;
}

#if 0
/**
* @internal prvCpssPxDiagDataIntegrityLogicalToHwTableMap function
* @endinternal
*
* @brief   Convert logical table entries range to an appropriate HW table entries
*         range. If logical range is too big the HW range relevant to sub-range
*         applicable for the HW table will be returned instead.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - The PP device number
* @param[in] logicalTable             - logical table
* @param[in] logicalStart             - logical item index starting the range
* @param[in] logicalRange             - range size
* @param[in] hwTable                  - HW table
*
* @param[out] hwStartPtr               - (pointer to) HW entry index corresponding to logicalStart.
*                                      Can be NULL.
* @param[out] hwRangePtr               - (pointer to) HW indexes range size.
*                                      Can be NULL.
* @param[out] hwLogicalFirstPtr        - (pointer to) the logical index corresponding
*                                      to the first entry of HW table.
*                                      Can be NULL.
* @param[out] hwLogicalMaxPtr          - (pointer to) index of last+1 logical entry
*                                      corresponding to the full HW table size.
*                                      Can be NULL.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on impossible logicalRange
* @retval GT_BAD_PARAM             - on unknown logical table or HW table.
* @retval GT_BAD_VALUE             - if HW table belongs to logical table not covered by the
*                                       function
*
* @note Use logicalStart=0, logicalRange = 0xFFFFFFFF to get full HW range
*       relevant to the logical table
*
*/
GT_STATUS prvCpssPxDiagDataIntegrityLogicalToHwTableMap
(
    IN  GT_U32                       devNum,
    IN  CPSS_PX_LOGICAL_TABLE_ENT    logicalTable,
    IN  GT_U32                       logicalStart,
    IN  GT_U32                       logicalRange,
    IN  CPSS_PX_TABLE_ENT             hwTable,
    OUT GT_U32                       *hwStartPtr,
    OUT GT_U32                       *hwRangePtr,
    OUT GT_U32                       *hwLogicalFirstPtr,
    OUT GT_U32                       *hwLogicalMaxPtr
)
{
    GT_STATUS rc;
    GT_U32 tableSize;
    GT_U32 isLimitedRange = (logicalRange != (GT_U32)-1); /* true if logical range is not 0xFFFFFFFF */
    GT_U32 hwMin;        /* index of first hw entry relevant to logical table */
    GT_U32 hwMax;        /* index of last+1 hw entry relevant to logical table */
    GT_U32 hwRangeStart; /* hw entry index appropriate to logicalStart */
    GT_U32 hwRangeMax;   /* hw entry index appropriate to (logicalStart + logicalRange) */
    GT_U32 hwLogicalFirst; /* index of first logical entry relevant to HW table */
    GT_U32 hwLogicalMax;   /* index of last+1 logical entry relevant to HW table */

    rc = prvCpssPxDiagDataIntegrityTableHwMaxIndexGet((GT_U8)devNum, hwTable, &tableSize);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* init default values */
    hwMin   = 0;
    hwMax   = tableSize;


    if(!isLimitedRange)
    {
        hwRangeStart = 0;
        hwRangeMax = tableSize;
    }
    else
    {
        hwRangeStart = logicalStart;
        hwRangeMax   = logicalStart + logicalRange;
    }
    hwLogicalFirst = hwMin;
    hwLogicalMax = hwMax;

    hwRangeStart = MIN(hwRangeStart, hwMax);
    hwRangeMax   = MIN(hwRangeMax, hwMax);

    /* assign OUT parameters */
    if (hwStartPtr)
    {
        *hwStartPtr = logicalStart;
    }

    if (hwRangePtr)
    {
        *hwRangePtr = (hwRangeMax < hwRangeStart) ? 0 : hwRangeMax - hwRangeStart;
    }

    if (hwLogicalFirstPtr)
    {
        *hwLogicalFirstPtr =hwLogicalFirst;
    }

    if (hwLogicalMaxPtr)
    {
        *hwLogicalMaxPtr =hwLogicalMax;
    }

    return GT_OK;
}
#endif
