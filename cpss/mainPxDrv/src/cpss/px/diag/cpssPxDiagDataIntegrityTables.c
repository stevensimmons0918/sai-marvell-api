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
* @file cpssPxChDiagDataIntegrityTables.c
*
* @brief API implementation for tables Diag Data Integrity.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/diag/cpssPxDiagDataIntegrity.h>
#include <cpss/px/diag/private/prvCpssPxDiagDataIntegrityMainMappingDb.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxHwTables.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvExMxEventsPxPipe.h>
#include <cpss/px/diag/cpssPxDiagDataIntegrityTables.h>
#include <cpss/px/diag/private/prvCpssPxDiagLog.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/* hw tables array max capacity */
#define PRV_CPSS_PX_HW_IN_LOGICAL_MAX_CNS 6
/* logical table entry */
typedef struct
{
   const CPSS_PX_LOGICAL_TABLE_ENT logicalName;
   const CPSS_PX_TABLE_ENT     hwNameArr[PRV_CPSS_PX_HW_IN_LOGICAL_MAX_CNS];
} PRV_CPSS_PX_LOGICAL_ENTRY_STC;


extern GT_U32 ppa_fw_imem_data[];

/* !!! debug mode that force us to not update the shadow !!! */
GT_U32  debugMode_PxforceNoUpdateOfShadow = 0;

void  copyBits(
    IN GT_U32                  *targetMemPtr,
    IN GT_U32                  targetStartBit,
    IN GT_U32                  *sourceMemPtr,
    IN GT_U32                  sourceStartBit,
    IN GT_U32                  numBits
);

GT_STATUS prvCpssPxPortGroupWriteTableEntry
(
    IN GT_SW_DEV_NUM     devNum,
    IN GT_U32                   portGroupId,
    IN CPSS_PX_TABLE_ENT  tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                  *entryValuePtr
);

GT_STATUS prvCpssPxDiagDataIntegrityDfxErrorConvert
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC    *memLocationPtr,
    IN  GT_U32                                          failedRow,
    OUT CPSS_PX_HW_INDEX_INFO_STC                      *hwErrorInfoPtr
);

GT_STATUS prvCpssPxDiagDataIntegrityDfxParamsConvert
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC    *memLocationPtr,
    IN  GT_U32                                          failedRow,
    OUT CPSS_PX_HW_INDEX_INFO_STC                       *hwErrorInfoPtr
);

GT_STATUS prvCpssPxShadowLineUpdateMasked
(
    IN GT_SW_DEV_NUM        devNum,
    IN CPSS_PX_TABLE_ENT     tableType,
    IN GT_U32                lineIndex,
    IN GT_U32                *entryValuePtr,
    IN GT_U32                *entryMaskPtr
);

/* function to set <debugMode_forceNoUpdateOfShadow> */
void debugMode_cpssPxDiagDataIntegrity_forceNoUpdateOfShadow
(
    IN GT_U32  forceNoUpdateOfShadow
)
{
    debugMode_PxforceNoUpdateOfShadow = forceNoUpdateOfShadow;
}

/* max number of words in entry */
#define MAX_ENTRY_SIZE_CNS   64

/* array of the hw tables that should not be in the shadow :
    - tables that are not supported by Data Integrity feature
    - counters
    - tables that not under CPSS control
*/
static const GT_U32  hwTablesNoNeedShadowArr[] = {

    /** BMA multicast counters */
    CPSS_PX_TABLE_BMA_MULTICAST_COUNTERS_E /* counters*/

    /** @brief txq shaper per port per TC token bucket configuration
     *  CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E : DQ[1]
     */
    ,CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E /* not supported by Data Integrity*/

    /** @brief txq shaper per port token bucket configuration
     *  CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E : DQ[1]
     */
    ,CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E         /* not supported by Data Integrity*/

    /** @brief Map the ingress port to TxQ port for PFC response
     *  CPSS_PX_TABLE_TXQ_DQ_PFC_RESPONSE_PORT_MAPPING_E : DQ[1]
     */
    ,CPSS_PX_TABLE_TXQ_DQ_PFC_RESPONSE_PORT_MAPPING_E                  /* not supported by Data Integrity*/

    /** Tail Drop Maximum Queue Limits */
    ,CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_MAX_QUEUE_LIMITS_E              /* not supported by Data Integrity*/


    /** @brief Tail Drop Counters -
     *  Buffers Queue Maintenance counters
     */
    ,CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_COUNTERS_Q_MAIN_BUFF_E          /* not supported by Data Integrity*/


    /** PFC Counters */
    ,CPSS_PX_TABLE_PFC_LLFC_COUNTERS_E /* counters*/

    /** cnc block 0 */
    ,CPSS_PX_TABLE_CNC_0_COUNTERS_E    /* counters*/

    /** cnc block 1 */
    ,CPSS_PX_TABLE_CNC_1_COUNTERS_E    /* counters*/

    /** PHA source port data table. */
    ,CPSS_PX_TABLE_PHA_SRC_PORT_DATA_E                                  /* not supported by Data Integrity*/

    /** PHA target port data table. */
    ,CPSS_PX_TABLE_PHA_TARGET_PORT_DATA_E /* not supported by Data Integrity*/

    ,CPSS_PX_MULTI_INSTANCE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG____DQ_1___E /* not supported by Data Integrity*/

    ,CPSS_PX_MULTI_INSTANCE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG____DQ_1___E          /* not supported by Data Integrity*/

    ,CPSS_PX_MULTI_INSTANCE_TXQ_DQ_PFC_RESPONSE_PORT_MAPPING_E____DQ_1___E                   /* not supported by Data Integrity*/

    ,CPSS_PX_INTERNAL_TABLE_MANAGEMENT_E                          /* not under CPSS control */
                                                                  /* not under CPSS control */
    ,CPSS_PX_INTERNAL_TABLE_PACKET_DATA_ECC_E                     /* not under CPSS control */
                                                                  /* not under CPSS control */
    ,CPSS_PX_INTERNAL_TABLE_PACKET_DATA_PARITY_E                  /* not under CPSS control */
                                                                  /* not under CPSS control */
    ,CPSS_PX_INTERNAL_TABLE_CM3_RAM_E                             /* not under CPSS control */
                                                                  /* not under CPSS control */
    ,CPSS_PX_INTERNAL_TABLE_PHA_RAM_E                             /* not under CPSS control */
                                                                  /* not under CPSS control */
    ,CPSS_PX_INTERNAL_TABLE_PACKET_DATA_CONTROL_ECC_E             /* not under CPSS control */
                                                                  /* not under CPSS control */
    ,CPSS_PX_INTERNAL_TABLE_PACKET_DATA_CONTROL_PARITY_E          /* not under CPSS control */
                                                                  /* not under CPSS control */
    ,CPSS_PX_INTERNAL_TABLE_DESCRIPTORS_CONTROL_E                 /* not under CPSS control */

    ,CPSS_PX_HW_TABLE_LAST_E/* must be last*/
};


/*
 * cpssLogicalToHwTablesDB
 *
 * Description:
 *      Database with 'logical' tables content - hw tables list for each logical table
 *
 */
static const PRV_CPSS_PX_LOGICAL_ENTRY_STC cpssLogicalToHwTablesDB[] =
{
    { CPSS_PX_LOGICAL_TABLE_INGRESS_DST_PORT_MAP_TABLE_E, {
        CPSS_PX_TABLE_PCP_DST_PORT_MAP_TABLE_E,/*8K range*/
        LAST_VALID_PX_TABLE_CNS }  },

    { CPSS_PX_LOGICAL_TABLE_INGRESS_PORT_FILTERING_TABLE_E, {
        CPSS_PX_TABLE_PCP_PORT_FILTERING_TABLE_E,/*4K range*/
        LAST_VALID_PX_TABLE_CNS }  },


    { CPSS_PX_LOGICAL_TABLE_TAIL_DROP_LIMITS_E, {
        CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP0_E,
        CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP1_E,
        CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP2_E,
        CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_BUF_LIMITS_E,
        CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_DESC_LIMITS_E,
        LAST_VALID_PX_TABLE_CNS }  },

   { CPSS_PX_LOGICAL_TABLE_BUFFERS_MANAGER_MULTICAST_COUNTERS_E, {
        CPSS_PX_TABLE_BMA_MULTICAST_COUNTERS_E,
        LAST_VALID_PX_TABLE_CNS } },

    { CPSS_PX_LOGICAL_TABLE_CNC_0_COUNTERS_E, {
        CPSS_PX_TABLE_CNC_0_COUNTERS_E,
        LAST_VALID_PX_TABLE_CNS } },

    { CPSS_PX_LOGICAL_TABLE_CNC_1_COUNTERS_E, {
        CPSS_PX_TABLE_CNC_1_COUNTERS_E,
        LAST_VALID_PX_TABLE_CNS } },

    { CPSS_PX_LOGICAL_TABLE_TXQ_QCN_CN_SAMPLE_INTERVALS_E, {
        CPSS_PX_TABLE_TXQ_QCN_CN_SAMPLE_INTERVALS_E,
        LAST_VALID_PX_TABLE_CNS } },

    { CPSS_PX_LOGICAL_TABLE_EGRESS_HEADER_ALTERATION_TABLE_E, {
        CPSS_PX_TABLE_PHA_HA_TABLE_E,    /*(0..16)*32 range = 544 range */
        LAST_VALID_PX_TABLE_CNS } },

    { CPSS_PX_LOGICAL_TABLE_PHA_SHARED_DMEM_E, {
        CPSS_PX_TABLE_PHA_SHARED_DMEM_E,
        LAST_VALID_PX_TABLE_CNS } },

    { CPSS_PX_LOGICAL_TABLE_PHA_FW_IMAGE_E, {
        CPSS_PX_TABLE_PHA_FW_IMAGE_E,
        LAST_VALID_PX_TABLE_CNS } },

    { CPSS_PX_LOGICAL_INTERNAL_TABLE_MANAGEMENT_E, {
        CPSS_PX_INTERNAL_TABLE_MANAGEMENT_E,
        LAST_VALID_PX_TABLE_CNS } },

    {CPSS_PX_LOGICAL_INTERNAL_TABLE_PACKET_DATA_ECC_E, {
        CPSS_PX_INTERNAL_TABLE_PACKET_DATA_ECC_E ,
        LAST_VALID_PX_TABLE_CNS } },

    {CPSS_PX_LOGICAL_INTERNAL_TABLE_PACKET_DATA_PARITY_E, {
        CPSS_PX_INTERNAL_TABLE_PACKET_DATA_PARITY_E ,
        LAST_VALID_PX_TABLE_CNS } },

    {CPSS_PX_LOGICAL_INTERNAL_TABLE_CM3_RAM_E, {
        CPSS_PX_INTERNAL_TABLE_CM3_RAM_E ,
        LAST_VALID_PX_TABLE_CNS } },

    {CPSS_PX_LOGICAL_INTERNAL_TABLE_PHA_RAM_E , {
        CPSS_PX_INTERNAL_TABLE_PHA_RAM_E   ,
        LAST_VALID_PX_TABLE_CNS } },

    {CPSS_PX_LOGICAL_INTERNAL_TABLE_PACKET_DATA_CONTROL_ECC_E , {
        CPSS_PX_INTERNAL_TABLE_PACKET_DATA_CONTROL_ECC_E   ,
        LAST_VALID_PX_TABLE_CNS } },

    {CPSS_PX_LOGICAL_INTERNAL_TABLE_PACKET_DATA_CONTROL_PARITY_E , {
        CPSS_PX_INTERNAL_TABLE_PACKET_DATA_CONTROL_PARITY_E   ,
        LAST_VALID_PX_TABLE_CNS } },

    {CPSS_PX_LOGICAL_INTERNAL_TABLE_DESCRIPTORS_CONTROL_E , {
        CPSS_PX_INTERNAL_TABLE_DESCRIPTORS_CONTROL_E   ,
        LAST_VALID_PX_TABLE_CNS } },


    /********* must be last ***************/
    { CPSS_PX_LOGICAL_TABLE_LAST_E, {  LAST_VALID_PX_TABLE_CNS} }
};




/**
* @internal prvCpssPxDiagDataIntegrityTableHwToLogicalGet function
* @endinternal
*
* @brief  Function returns logical tables for specific hw table.
* @param[in] hwTable                  - the hw table name
* @param[in,out] startPtr             - (pointer to) a marker where from to start search.
*                                      0 on NULL means "search from beginning".
*                                      An out-value is used as input for next call
*                                      to continue the search.
*
* @param[out] logicalTablePtr          - (pointer to) the logical table name
*
* @retval GT_OK                    - logical table is found
* @retval GT_NOT_FOUND             - logical table is not found
*
*
*/
GT_VOID prvCpssPxDiagDataIntegrityTableHwToLogicalGet
(
    IN    CPSS_PX_TABLE_ENT              hwTable,
    OUT   CPSS_PX_LOGICAL_TABLE_ENT      *logicalTablePtr
)
{
    GT_U32                                 ii, jj;
    for(ii = 0; ii< CPSS_PX_LOGICAL_TABLE_LAST_E; ii++)
    {
        if (cpssLogicalToHwTablesDB[ii].logicalName == CPSS_PX_LOGICAL_TABLE_TAIL_DROP_LIMITS_E )
        {
            for (jj = 0; jj < PRV_CPSS_PX_HW_IN_LOGICAL_MAX_CNS; jj++)
            {
                if (cpssLogicalToHwTablesDB[ii].hwNameArr[jj] == hwTable)
                {
                    *logicalTablePtr = cpssLogicalToHwTablesDB[ii].logicalName;
                    break;
                }
            }
        }
        else if (cpssLogicalToHwTablesDB[ii].hwNameArr[0] == hwTable)
        {
            *logicalTablePtr = cpssLogicalToHwTablesDB[ii].logicalName;
            break;
        }
    }

}

/**
* @internal prvCpssPxDiagDataIntegrityTableLogicalToHwListGet function
* @endinternal
*
* @brief   Function returns the list of HW tables that relate to the logical table
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @param[in] logicalTable             - the logical table
*
* @param[out] hwTableslistPtr          - (pointer to) the list of the HW tables
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if table not supported
*/
GT_STATUS prvCpssPxDiagDataIntegrityTableLogicalToHwListGet
(
    IN  CPSS_PX_LOGICAL_TABLE_ENT         logicalTable,
    OUT const CPSS_PX_TABLE_ENT           **hwTableslistPtr
)
{
    GT_U32  ii;
    const PRV_CPSS_PX_LOGICAL_ENTRY_STC *logicalToHwDb = cpssLogicalToHwTablesDB;

    for(ii = 0 ; logicalToHwDb[ii].logicalName != CPSS_PX_LOGICAL_TABLE_LAST_E ; ii++)
    {
        if(logicalToHwDb[ii].logicalName == logicalTable)
        {
            *hwTableslistPtr =  &logicalToHwDb[ii].hwNameArr[0];
            return GT_OK;
        }
    }

    *hwTableslistPtr =  &logicalToHwDb[0].hwNameArr[0];

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "the logical table not found in the DB");
}


/**
* @internal prvCpssPxDiagDataIntegrityTableHwMaxIndexGet
*           function
* @endinternal
*
* @brief   Function returns the number of entries,lines that HW table hold
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] hwTable                  - the IPLR table
*
* @param[out] maxNumEntriesPtr         - (pointer to) the number of entries supported by the table
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if table not covered by this function
*/
GT_STATUS prvCpssPxDiagDataIntegrityTableHwMaxIndexGet
(
    IN  GT_SW_DEV_NUM                      devNum,
    IN  CPSS_PX_TABLE_ENT                   hwTable,
    OUT GT_U32                              *maxNumEntriesPtr
)
{
    GT_STATUS rc;
    GT_U32    maxNumLines;
    GT_U32    maxNumEntries;
    GT_U32    ratio;
    GT_BOOL   isMultiple;

    /* default value */
    maxNumLines = PRV_PX_TABLE_INFO_PTR_GET_MAC(devNum,hwTable)->directAccessInfo.maxNumOfEntries;

    rc = prvCpssPxTableEngineToHwRatioGet(devNum,hwTable,&ratio,&isMultiple);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(ratio == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    maxNumEntries = maxNumLines;

    if(ratio != 1)
    {
        if(isMultiple == GT_TRUE)
        {
            /* ratio is 'multiple' (number of entries in single line) */
            maxNumEntries *= ratio;
        }
        else
        {
            /* ratio is 'fraction' (number of lines  in single entry) */
            maxNumEntries /= ratio;
        }
    }

    if(maxNumEntriesPtr)
    {
        *maxNumEntriesPtr = maxNumEntries;
    }
    return GT_OK;
}

/**
* @internal checkIsDevSupportHwTable function
* @endinternal
*
* @brief   Function checks if the device supports the HW table
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] hwTableType              - the HW table
*
* @retval GT_TRUE                  - the device   supports the table
* @retval GT_FALSE                 - the device NOT supports the table
*/
static GT_BOOL  checkIsDevSupportHwTable(IN CPSS_PX_TABLE_ENT         hwTableType)
{
    if((GT_U32)hwTableType >=  CPSS_PX_TABLE_LAST_E)/* only tables that under CPSS control*/
    {
        return GT_FALSE;
    }

    return GT_TRUE;
}


/**
* @internal
*           prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTable
*           function
* @endinternal
*
* @brief   Function checks if the referenced HW table entry
*          exists
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] hwTable                  - HW table
* @param[in] entryIndex               - entry index
*
* @retval GT_TRUE                  - the entry exists
* @retval GT_FALSE                 - the device NOT supports the table or entry
*/
GT_BOOL  prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTable(
    IN GT_SW_DEV_NUM       devNum ,
    IN CPSS_PX_TABLE_ENT    hwTableType,
    IN GT_U32               entryIndex
)
{
    GT_STATUS   rc;
    GT_U32  maxEntries;

    if(GT_FALSE == checkIsDevSupportHwTable(hwTableType))
    {
        /* the HW table is not valid  */
        return GT_FALSE;
    }
    /* get number of entries in the table */
    rc = prvCpssPxDiagDataIntegrityTableHwMaxIndexGet(devNum,hwTableType,&maxEntries);
    if(rc != GT_OK)
    {
        return GT_FALSE;
    }
    if (hwTableType == CPSS_PX_TABLE_PHA_FW_IMAGE_E)
    {
         /*for FW image hwIndex contains address offset in bits 0-13*/
        entryIndex = entryIndex & 0x3FFF;
    }
    if (entryIndex >= maxEntries)
    {
        /* index is above valid range of this table ... ignore it !!! */
        return GT_FALSE;
    }


    return GT_TRUE;
}

/**
* @internal
*           prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTableShadow
*           function
* @endinternal
*
* @brief   Function returns the 'shadow type' that the CPSS may hold for the HW table
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] hwTable                  - HW table
*
* @retval                             - HW table shadow type
*/
CPSS_PX_SHADOW_TYPE_ENT  prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTableShadow(
    IN GT_SW_DEV_NUM       devNum ,
    IN CPSS_PX_TABLE_ENT  hwTableType
)
{
    PRV_CPSS_PX_HW_TABLES_SHADOW_STC *tableInfoPtr;

    if(GT_FALSE == checkIsDevSupportHwTable(hwTableType))
    {
        /* the HW table is not valid  */
        return GT_FALSE;
    }

    tableInfoPtr = &(PRV_CPSS_PX_PP_MAC(devNum)->shadowInfoArr[hwTableType]);

    return tableInfoPtr->shadowType;
}

/**
* @internal prvCpssPxReadTableEntries function
* @endinternal
*
* @brief   function reads number of table entries
*
* @param[in] devNum                   - device number,
* @param[in] hwTable                  - hw table name,
* @param[in] entryIndex               - start entry index,
* @param[in] numOfEntries             - number of entries to read (0xFFFFFFFF means till end of table)
* @param[in] sizeLimit                - number of entries on
*                                      which the 'wrap around'
*                                      happen value ignored if
*                                      '0'
*
* @param[out] nextEntryIndexPtr        - next entry index pointer
* @param[out] wasWrapAroundPtr         - (pointer to) was wrap around indication
*                                      GT_TRUE - if 'wrap around' happen,
*                                      otherwise - not modified!
*                                      NOTE:
*                                      Ignored if NULL
*
* @retval GT_OK                    - no errors
* @retval GT_BAD_PARAM             - bad param given
*/
static GT_STATUS prvCpssPortGroupReadTableEntries
(
    IN  GT_SW_DEV_NUM            devNum,
    IN  CPSS_PX_TABLE_ENT        hwTable,
    IN  GT_U32                   entryIndex,
    IN  GT_U32                   numOfEntries,
    OUT GT_U32                   *nextEntryIndexPtr,
    OUT GT_BOOL                  *wasWrapAroundPtr,
    IN GT_U32                    sizeLimit
)
{
    GT_STATUS  rc = GT_FAIL;
    GT_U32     valueArr[MAX_ENTRY_SIZE_CNS];
    GT_U32     maxTableSize;
    GT_U32     origEntryIndex;
    GT_U32  ppg;
    GT_U32  regAddr;

    /**********************/
    /* table validity was done by calling to prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTable(...) */
    /**********************/
    /* get number of entries in the table */
    maxTableSize = PRV_PX_TABLE_INFO_PTR_GET_MAC(devNum,hwTable)->directAccessInfo.maxNumOfEntries;
    if(sizeLimit == 0)
    {
        sizeLimit = maxTableSize;
    }

    if(entryIndex >= sizeLimit)
    {
        /*CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);*/
        return GT_OK;
    }

    if(numOfEntries == 0xFFFFFFFF)
    {
        numOfEntries = sizeLimit - entryIndex;
    }

    if(numOfEntries > sizeLimit)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    origEntryIndex = entryIndex;
    if (hwTable == CPSS_PX_TABLE_PHA_FW_IMAGE_E)
    {
        for(entryIndex = (origEntryIndex & 0x3ff0);entryIndex < ((origEntryIndex  + numOfEntries) & 0x3ff0);entryIndex+=4)
        {
            ppg = (entryIndex >> 14) & 3;
            regAddr = PRV_PX_REG1_UNIT_PHA_MAC(devNum).PPG[ppg].PPG_IMEM_base_addr;
            regAddr = PRV_PX_REG1_UNIT_PHA_MAC(devNum).PPG[ppg].PPG_IMEM_base_addr + entryIndex;
            rc = prvCpssHwPpReadRam(CAST_SW_DEVNUM(devNum), regAddr, 4, valueArr);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        /* if wrap..*/
        if ((origEntryIndex  + numOfEntries) > 0x3ff0)
        {
            for(entryIndex = 0;entryIndex < ((origEntryIndex  + numOfEntries - 0x3ff0) & 0x3ff0);entryIndex+=4)
            {
                ppg = (entryIndex >> 14) & 3;
                regAddr = PRV_PX_REG1_UNIT_PHA_MAC(devNum).PPG[ppg].PPG_IMEM_base_addr;
                regAddr = PRV_PX_REG1_UNIT_PHA_MAC(devNum).PPG[ppg].PPG_IMEM_base_addr + entryIndex;
                rc = prvCpssHwPpReadRam(CAST_SW_DEVNUM(devNum), regAddr, 4, valueArr);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }

        }
        entryIndex = origEntryIndex + numOfEntries;
    }
    else
    {
        for(entryIndex = origEntryIndex ;entryIndex < (origEntryIndex + numOfEntries);entryIndex++)
        {
            rc = prvCpssPxReadTableEntry(devNum, hwTable,entryIndex % maxTableSize,valueArr/*not relevant*/);
                if (rc != GT_OK)
                {
                    return rc;
                }

        } /* end: entries iteration */
    }

    if(entryIndex > sizeLimit && wasWrapAroundPtr)
    {
        *wasWrapAroundPtr = GT_TRUE;
    }
    *nextEntryIndexPtr = entryIndex % sizeLimit;

    return GT_OK;
}



/**
* @internal internal_cpssPxDiagDataIntegrityTableScan function
* @endinternal
*
* @brief   Data integrity SW daemon API
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - the device number
* @param[in] locationPtr              - (pointer to) location information,
*                                      ram location is not supported
* @param[in] numOfEntries             - number of entries to read (0xFFFFFFFF means till end of table)
*                                      NOTE: for 'logical table' this value is ignored and taken from :
* @param[in] locationPtr
*
* @param[out] nextEntryIndexPtr        - (pointer to) next index
* @param[out] wasWrapAroundPtr         - (pointer to) was wrap around indication
*                                      NOTE: ignored if NULL
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxDiagDataIntegrityTableScan
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC   *locationPtr,
    IN  GT_U32                                numOfEntries,
    OUT GT_U32                               *nextEntryIndexPtr,
    OUT GT_BOOL                              *wasWrapAroundPtr
)
{
    GT_STATUS                    rc = GT_FAIL;
    GT_U32                       j;
    GT_U32                       firstEntryIndex;
    CPSS_PX_TABLE_ENT            hwTableType;
    const CPSS_PX_TABLE_ENT      *hwNameArr;
    GT_BOOL                      wasWrapAround;
    GT_U32                       nextEntryIndex;
    CPSS_PX_LOGICAL_TABLE_ENT    logicalTable;

    /* check parameters */
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(locationPtr);
    CPSS_NULL_PTR_CHECK_MAC(nextEntryIndexPtr);

    /* must be initialized here ...
       the code along the way only make it 'GT_TRUE' */
    wasWrapAround = GT_FALSE;

    switch(locationPtr->type)
    {
        case CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE:
            hwTableType = locationPtr->info.hwEntryInfo.hwTableType;

            if(GT_FALSE == checkIsDevSupportHwTable(hwTableType))
            {
                  CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "HW table is not applicable");
            }

            rc = prvCpssPortGroupReadTableEntries(devNum,
                                                  locationPtr->info.hwEntryInfo.hwTableType,
                                                  locationPtr->info.hwEntryInfo.hwTableEntryIndex,
                                                  numOfEntries,
                                                  &nextEntryIndex,
                                                  &wasWrapAround,0/*ignore*/);
            if (rc != GT_OK)
            {
                return rc;
            }
        break;

        case CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE:
            nextEntryIndex = 0;
            logicalTable = locationPtr->info.logicalEntryInfo.logicalTableType;

            if((GT_U32)logicalTable >= CPSS_PX_LOGICAL_INTERNAL_TABLE_MANAGEMENT_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            firstEntryIndex = locationPtr->info.logicalEntryInfo.logicalTableEntryIndex;

            rc = prvCpssPxDiagDataIntegrityTableLogicalToHwListGet(logicalTable, &hwNameArr);
            if(rc != GT_OK)
            {
                return rc;
            }
            wasWrapAround = GT_FALSE;
            nextEntryIndex = firstEntryIndex;
            for(j = 0; hwNameArr[j] != LAST_VALID_PX_TABLE_CNS; j++)
            { /* hw tables iteration */

                hwTableType = hwNameArr[j];
                if(GT_FALSE == checkIsDevSupportHwTable(hwTableType))
                {
                    /* no Data Integrity support for this HW table */
                    continue;
                }

                /* logical entries range is the same as HW range  ; all HW tables that are members of loical table have the same size
                so, no need to keep the info per HW table*/
              rc = prvCpssPortGroupReadTableEntries(devNum,
                                                    hwTableType,
                                                    firstEntryIndex,
                                                    numOfEntries,
                                                    &nextEntryIndex,
                                                    &wasWrapAround,
                                                    0  /* ignored */);
                if (rc != GT_OK)
                {
                    return rc;
                }

            } /* end: hw tables iteration */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    } /* end of switch */

    if (wasWrapAroundPtr)
    {
        *wasWrapAroundPtr = wasWrapAround;
    }
    *nextEntryIndexPtr = nextEntryIndex;
    return rc;
}

/**
* @internal cpssPxDiagDataIntegrityTableScan function
* @endinternal
*
* @brief   Data integrity SW daemon API
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - the device number
* @param[in] locationPtr              - (pointer to) location information,
*                                      ram location is not supported
* @param[in] numOfEntries             - number of entries to read (0xFFFFFFFF means till end of table)
*                                      NOTE: for 'logical table' this value is ignored and taken from :
* @param[in] locationPtr
*
* @param[out] nextEntryIndexPtr        - (pointer to) next index
* @param[out] wasWrapAroundPtr         - (pointer to) was wrap around indication
*                                      NOTE: ignored if NULL
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagDataIntegrityTableScan
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC *locationPtr,
    IN  GT_U32                               numOfEntries,
    OUT GT_U32                               *nextEntryIndexPtr,
    OUT GT_BOOL                              *wasWrapAroundPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagDataIntegrityTableScan);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, locationPtr, numOfEntries, nextEntryIndexPtr, wasWrapAroundPtr));

    rc = internal_cpssPxDiagDataIntegrityTableScan(devNum, locationPtr, numOfEntries,
                                                    nextEntryIndexPtr, wasWrapAroundPtr);
    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, devNum, locationPtr, numOfEntries, nextEntryIndexPtr, wasWrapAroundPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

typedef enum{
    PRV_CPSS_PX_SHADOW_INDEX_TYPE_ENTRY_E,
    PRV_CPSS_PX_SHADOW_INDEX_TYPE_LINE_E
}PRV_CPSS_PX_SHADOW_INDEX_TYPE_ENT;

/**
* @internal prvCpssPxPortGroupShadowEntryGet function
* @endinternal
*
* @brief   return address of the specified bit of entry in the CPSS shadow.
*         NOTE: the 'entryIndex' is the same one that used by
*         prvCpssPxPortGroupWriteTableEntry
*         the 'line index' is the same as the actual HW keeps it's 'lines'
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the 'HW table' type
* @param[in] index                    - table  index'
*
* @retval GT_OK                    - on success.
* @retval GT_EMPTY                 - if the table hold no shadow
*
* @note 1. will return pointer to the entry/line in the shadow (no 'copy' is done)
*       2. caller must check that the 'table' holds shadow (otherwise will get 'pointer to NULL').
*
*/
static GT_STATUS prvCpssPxPortGroupShadowEntryGet
(
    IN GT_SW_DEV_NUM                      devNum,
    IN CPSS_PX_TABLE_ENT                  tableType,
    IN GT_U32                             index,
    OUT GT_U32                            **entryInShadowPtrPtr, /* will return pointer to the entry in the shadow (no 'copy' is done)*/
    OUT GT_U32                            *numBitsPerEntryPtr
)
{
    PRV_CPSS_PX_HW_TABLES_SHADOW_STC   *shadowInfoPtr       = &(PRV_CPSS_PX_PP_MAC(devNum)->shadowInfoArr[tableType]);
    GT_U32                              numBitsPerEntry;
    GT_U32                              numOfWordsPerEntry;
    GT_U32                             *entryInShadowPtr;
    GT_U32                              firstWordInShadow;

    if(NULL == shadowInfoPtr->hwTableMemPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_EMPTY, "no shadow for the table [%d]",
            tableType);
    }



    if(index >= (PRV_PX_TABLE_INFO_PTR_GET_MAC(devNum,tableType)->directAccessInfo.maxNumOfEntries))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "shadow 'get' try to access index[%d] out of range[%d]",index,
                                      PRV_PX_TABLE_INFO_PTR_GET_MAC(devNum,tableType)->directAccessInfo.maxNumOfEntries - 1);
    }


    numBitsPerEntry = PRV_PX_TABLE_INFO_PTR_GET_MAC(devNum,tableType)->directAccessInfo.numOfBits;
    numOfWordsPerEntry = (numBitsPerEntry % 32)? (numBitsPerEntry / 32 + 1) : (numBitsPerEntry / 32);

    firstWordInShadow = index * numOfWordsPerEntry;

    if(firstWordInShadow >= (shadowInfoPtr->hwTableMemSize/4))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "shadow 'get' try to access word[%d] out of range[%d]",
            firstWordInShadow,
            (shadowInfoPtr->hwTableMemSize/4) - 1);
    }


    entryInShadowPtr = &shadowInfoPtr->hwTableMemPtr[firstWordInShadow];

    *entryInShadowPtrPtr = entryInShadowPtr;

    if(numBitsPerEntryPtr)
    {
        *numBitsPerEntryPtr = numBitsPerEntry;
    }


    return  GT_OK;
}

/**
* @internal prvCpssPxPortGroupShadowEntryWrite function
* @endinternal
*
* @brief   Writes to PP's shadow at specific 'entry index'
*         (same index as in functions of prvCpssPxPortGroupWriteTableEntry)
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] tableType                - the 'HW table'
* @param[in] entryIndex               - index in the table
*                                      (same index as in functions of prvCpssPxPortGroupWriteTableEntry)
* @param[in] entryValuePtr            - (pointer to) An array containing the data to be
*                                      copied into the shadow
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware of logical error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortGroupShadowEntryWrite
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  CPSS_PX_TABLE_ENT     tableType,
    IN  GT_U32                entryIndex,
    IN  GT_U32                *entryValuePtr
)
{
    GT_STATUS   rc;
    GT_U32  *shadowPtr;/* pointer to the shadow table line for the index in the table */
    PRV_CPSS_PX_HW_TABLES_SHADOW_STC *shadowInfoPtr = &(PRV_CPSS_PX_PP_MAC(devNum)->shadowInfoArr[tableType]);
    GT_U32  numBitsPerEntry;/* number of bits per entry */

    if(debugMode_PxforceNoUpdateOfShadow)
    {
        /* !!! debug mode that force us to not update the shadow !!! */
        return GT_OK;
    }

    if(CPSS_PX_SHADOW_TYPE_CPSS_E != shadowInfoPtr->shadowType)
    {
        /* the device not supports it or the table not requires shadow */
        return GT_OK;
    }

    /* get pointer to the shadow  */
    rc = prvCpssPxPortGroupShadowEntryGet(devNum,tableType,
                                          entryIndex,    /* index is of 'entry'  */
                                          &shadowPtr,    /* pointer to shadow*/
                                          &numBitsPerEntry/* number of bits for the 'entry' in the 'line(s)' */
                                          );
    if(rc != GT_OK)
    {
        return rc;
    }

    copyBits(shadowPtr,        /* target */
             0,                /* start bit in the target */
             entryValuePtr,    /* source */
             0,                /* start bit in the source */
             numBitsPerEntry); /* number of bits to copy */


    return  GT_OK;
}

/**
* @internal prvCpssPxShadowLineWrite function
* @endinternal
*
* @brief   Writes to PP's shadow at specific 'line index'
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] tableType                - the 'HW table'
* @param[in] lineIndex                - the line index in the 'HW table'
*                                      (not the same index as the functions of 'table engine' get (like
*                                      prvCpssPxPortGroupWriteTableEntry))
* @param[in] entryValuePtr            - (pointer to) An array containing the data to be
*                                      copied into the shadow
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxShadowLineWrite
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  CPSS_PX_TABLE_ENT     tableType,
    IN  GT_U32                lineIndex,
    IN  GT_U32                *entryValuePtr
)
{
    return prvCpssPxShadowLineUpdateMasked(devNum,tableType, lineIndex,entryValuePtr, NULL);
}
/* @internal prvFwImageTableEntryRead function
* @endinternal
*
* @brief   read referenced entry in
*          CPSS_PX_TABLE_PHA_FW_IMAGE_E HW table.
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in]  devNum                   - the device number
* @param[in]  hwTableEntryIndex        - index in the 'HW table'
* @param[out] readValue                - read value
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvFwImageTableEntryRead
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_U32                                  entryIndex,
    IN  GT_U32                                  ppg,
    OUT GT_U32                                  *readValue
)
{
    GT_STATUS  rc;                          /* return code              */
    GT_U32  regAddr;


    regAddr = PRV_PX_REG1_UNIT_PHA_MAC(devNum).PPG[ppg].PPG_IMEM_base_addr;
    regAddr = PRV_PX_REG1_UNIT_PHA_MAC(devNum).PPG[ppg].PPG_IMEM_base_addr + entryIndex;
    rc = prvCpssHwPpReadRam(CAST_SW_DEVNUM(devNum), regAddr, 4, readValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal prvFwImageTableEntryWrite function
* @endinternal
*
* @brief   write referenced entry to
*          CPSS_PX_TABLE_PHA_FW_IMAGE_E HW table.
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - the device number
* @param[in] hwTableEntryIndex        - index in the 'HW table'
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvFwImageTableEntryWrite
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_U32                                  hwTableEntryIndex
)
{
    GT_STATUS  rc;                          /* return code              */
    GT_U32  ppg;
    GT_U32  regAddr;

    for (ppg = 0; ppg < 4 ; ppg+= 4)
    {
        regAddr = PRV_PX_REG1_UNIT_PHA_MAC(devNum).PPG[ppg].PPG_IMEM_base_addr +  (hwTableEntryIndex & 0x3ffC);
        rc = prvCpssHwPpWriteRam(CAST_SW_DEVNUM(devNum),regAddr,4,
                                 (GT_U32*)(&ppa_fw_imem_data[hwTableEntryIndex]));
        if (rc != GT_OK)
        {
            return rc;
        }
     }
    return GT_OK;
}



/**
* @internal shadowTableEntryFix function
* @endinternal
*
* @brief   Data integrity : fix specific table entry that is corrupted in the HW,
*         using CPSS shadow for this table.
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - the device number
* @param[in] hwTableType              - the 'HW table' to fix
* @param[in] hwTableEntryIndex        - the 'corrupted' index in the 'HW table'
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS shadowTableEntryFix
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  CPSS_PX_TABLE_ENT                       hwTableType,
    IN  GT_U32                                  hwTableEntryIndex
)
{
    GT_STATUS  rc;                          /* return code              */
    GT_U32  *shadowPtr;/* pointer to the shadow for the index in the table */
    GT_U32  numBitsPerEntry;/* number of bits per entry */
    GT_U32  valueArr[MAX_ENTRY_SIZE_CNS];
    PRV_CPSS_PX_HW_TABLES_SHADOW_STC *shadowInfoPtr;

    if(GT_FALSE == prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTable(devNum,
                                                                           hwTableType,
                                                                           hwTableEntryIndex))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "HW table is not applicable or index out of range");
    }

    if (hwTableType == CPSS_PX_TABLE_PHA_FW_IMAGE_E)
    {
        return prvFwImageTableEntryWrite(devNum,hwTableEntryIndex);
    }

    shadowInfoPtr = &(PRV_CPSS_PX_PP_MAC(devNum)->shadowInfoArr[hwTableType]);

    if(CPSS_PX_SHADOW_TYPE_CPSS_E != shadowInfoPtr->shadowType)
    {
        /* the device not supports it or the table not requires shadow */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                      "The CPSS hold no shadow for HW table[%d]",
                                      hwTableType);
    }

    /* get pointer to the shadow */
    rc = prvCpssPxPortGroupShadowEntryGet(devNum, hwTableType,
                                          hwTableEntryIndex, &shadowPtr,
                                          &numBitsPerEntry);
    if(rc != GT_OK)
    {
        return rc;
    }

    copyBits(valueArr,
             0        , /* start bit in target */
             shadowPtr, /* source*/
             0        , /* start bit in the source */
             numBitsPerEntry); /* number of bits in the entry */
    /* write from the shadow to the HW */
    rc = prvCpssPxPortGroupWriteTableEntry(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,hwTableType,
                                           hwTableEntryIndex, shadowPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}



/**
* @internal internal_cpssPxDiagDataIntegrityTableEntryFix function
* @endinternal
*
* @brief   Data integrity : fix specific table entry that is corrupted in the HW,
*         using CPSS shadow for this table.
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - the device number
* @param[in] locationPtr              - (pointer to) location information
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_EMPTY                 - the DFX location is not mapped to CPSS HW table.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxDiagDataIntegrityTableEntryFix
(
    IN  GT_SW_DEV_NUM                         devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC    *locationPtr
)
{
    GT_STATUS   rc;
    GT_U32     jj;
    CPSS_PX_HW_INDEX_INFO_STC hwEntryInfo;
    const CPSS_PX_TABLE_ENT   *hwNameArr;
    CPSS_PX_LOGICAL_TABLE_ENT logicalTable;

    /* check parameters */
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(locationPtr);

    switch(locationPtr->type)
    {
        case CPSS_PX_LOCATION_HW_INDEX_INFO_TYPE:
            rc = shadowTableEntryFix(devNum,locationPtr->info.hwEntryInfo.hwTableType,
                                     locationPtr->info.hwEntryInfo.hwTableEntryIndex);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;

        case CPSS_PX_LOCATION_LOGICAL_INDEX_INFO_TYPE:
            logicalTable = locationPtr->info.logicalEntryInfo.logicalTableType;
            rc = GT_BAD_PARAM;/* if no tables */
            if((GT_U32)logicalTable >= CPSS_PX_LOGICAL_TABLE_LAST_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            rc = prvCpssPxDiagDataIntegrityTableLogicalToHwListGet(logicalTable, &hwNameArr);
            if(rc != GT_OK)
            {
                return rc;
            }
            /* if all HW tables are not applicable ... this value will be returned */
            rc = GT_BAD_PARAM;
            for(jj = 0; hwNameArr[jj] != LAST_VALID_PX_TABLE_CNS; jj++)
            { /* hw tables iteration */
                CPSS_PX_TABLE_ENT                     hwTableType;

                hwTableType = hwNameArr[jj];
                rc = shadowTableEntryFix(devNum,hwTableType,locationPtr->info.logicalEntryInfo.logicalTableEntryIndex);
                if (rc != GT_OK)
                {
                    return rc;
                }
            } /* end: hw tables iteration */
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "all HW tables are not applicable");
            }
            break;

        case CPSS_PX_LOCATION_RAM_INDEX_INFO_TYPE:
            /* convert RAM info to 'HW info' */
            rc = prvCpssPxDiagDataIntegrityDfxParamsConvert(devNum,&locationPtr->info.ramEntryInfo.memLocation,
                                                           locationPtr->info.ramEntryInfo.ramRow,&hwEntryInfo);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = shadowTableEntryFix(devNum,hwEntryInfo.hwTableType,hwEntryInfo.hwTableEntryIndex);
            if (rc != GT_OK)
            {
                return rc;
            }

            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    } /* end of switch */

    return GT_OK;
}

/**
* @internal cpssPxDiagDataIntegrityTableEntryFix function
* @endinternal
*
* @brief   Data integrity : fix specific table entry that is corrupted in the HW,
*         using CPSS shadow for this table.
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - the device number
* @param[in] locationPtr              - (pointer to) location information
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_EMPTY                 - the DFX location is not mapped to CPSS HW table.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagDataIntegrityTableEntryFix
(
    IN  GT_SW_DEV_NUM                      devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC *locationPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagDataIntegrityTableEntryFix);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, locationPtr));

    rc = internal_cpssPxDiagDataIntegrityTableEntryFix(devNum, locationPtr);
    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, devNum, locationPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssCalculateHwTableShadowSize function
* @endinternal
*
* @brief   This function performs cpss shadow hw entry size calculation
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] hwTable                  - hw table name
*
* @retval size                     - memory size (in bytes) required for hw table shadow
*/
static GT_U32 prvCpssCalculateHwTableShadowSize
(
    IN GT_SW_DEV_NUM       devNum,
    IN CPSS_PX_TABLE_ENT   hwTable
)
{
    GT_U32 maxNumOfEntries = PRV_PX_TABLE_INFO_PTR_GET_MAC(devNum,hwTable)->directAccessInfo.maxNumOfEntries;
    GT_U32 entrySizeInWords;
    GT_U32 size;


    entrySizeInWords = PRV_PX_TABLE_INFO_PTR_GET_MAC(devNum,hwTable)->directAccessInfo.numOfBits / 32;

    if (PRV_PX_TABLE_INFO_PTR_GET_MAC(devNum,hwTable)->directAccessInfo.numOfBits % 32)
    {
        entrySizeInWords++;
    }
    size = maxNumOfEntries * entrySizeInWords *4;

/*#define SHADOW_DEBUG_INFO_PRINT*/
#ifdef SHADOW_DEBUG_INFO_PRINT
    {
        static GT_U32 total = 0;

        total += size;
        cpssOsPrintf("Shadow memory table size %d, total %d \n ", size, total);
    }
#endif

    return size;
}

/**
* @internal hwTableShadowDefaultValueSet function
* @endinternal
*
* @brief   This function sets default values of HW into cpss shadow.
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] hwTable                  - hw table name
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS hwTableShadowDefaultValueSet
(
    IN GT_SW_DEV_NUM        devNum,
    IN CPSS_PX_TABLE_ENT    hwTable
)
{
    PRV_CPSS_PX_HW_TABLES_SHADOW_STC *tableInfoPtr;
    GT_U32     index;
    GT_U32     maxTableSize;
    GT_STATUS  rc;
    GT_U32     valueArr[MAX_ENTRY_SIZE_CNS];


    tableInfoPtr = &(PRV_CPSS_PX_PP_MAC(devNum)->shadowInfoArr[hwTable]);

    /* clear the memory first */
    cpssOsMemSet(tableInfoPtr->hwTableMemPtr,0,tableInfoPtr->hwTableMemSize);

    rc = prvCpssPxDiagDataIntegrityTableHwMaxIndexGet(devNum,hwTable,&maxTableSize);
    if(rc != GT_OK)
    {
        return rc;
    }


    for(index = 0; index < maxTableSize; index++)
    {
        if(GT_FALSE ==
            prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTable(devNum, hwTable, index))
        {
            /* support the gaps */
            continue;
        }

        /* read hw table entry */
        rc = prvCpssPxReadTableEntry(devNum, hwTable, index, valueArr);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* init hw table shadow */
        rc = prvCpssPxPortGroupShadowEntryWrite(devNum, hwTable, index, valueArr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }


    return GT_OK;
}
/**
* @internal prvCpssInitHwTableShadow function
* @endinternal
*
* @brief   This function performs cpss shadow hw entry init
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] hwTable                  - hw table name
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate CPU memory,
*/
static GT_STATUS prvCpssInitHwTableShadow
(
    IN GT_SW_DEV_NUM        devNum,
    IN CPSS_PX_TABLE_ENT    hwTable
)
{
    GT_STATUS   rc;
    GT_U32 ii;
    PRV_CPSS_PX_HW_TABLES_SHADOW_STC *tableInfoPtr;

    tableInfoPtr = &(PRV_CPSS_PX_PP_MAC(devNum)->shadowInfoArr[hwTable]);

    if(GT_FALSE == checkIsDevSupportHwTable(hwTable))
    {
        /* the shadow type for such entries is NONE by default */
        return GT_OK;
    }

    /* filter tables that no meaning to hold SW shadow for them */
    for (ii = 0; hwTablesNoNeedShadowArr[ii] != CPSS_PX_HW_TABLE_LAST_E ; ii++)
    {
        if(hwTable == hwTablesNoNeedShadowArr[ii])
        {
            tableInfoPtr->shadowType = CPSS_PX_SHADOW_TYPE_NONE_E;
            return GT_OK;
        }
    }

    tableInfoPtr->shadowType  = CPSS_PX_SHADOW_TYPE_CPSS_E;
    /* get the size in bytes */
    if (hwTable != CPSS_PX_TABLE_PHA_FW_IMAGE_E)
    {
        tableInfoPtr->hwTableMemSize = prvCpssCalculateHwTableShadowSize(devNum, hwTable);
        tableInfoPtr->hwTableMemPtr  = (GT_U32*)cpssOsMalloc(tableInfoPtr->hwTableMemSize);
        if(NULL == tableInfoPtr->hwTableMemPtr)
        {
            tableInfoPtr->hwTableMemSize = 0;
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        /* initialize the shadow with 'HW default values' */
        rc = hwTableShadowDefaultValueSet(devNum,hwTable);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPxHwPpPhase1ShadowInit function
* @endinternal
*
* @brief   This function performs basic cpss shadow configuration
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] ppPhase1ParamsPtr        - Packet processor hardware specific parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - failed to allocate memory
*/
GT_STATUS prvCpssPxHwPpPhase1ShadowInit
(
    IN GT_SW_DEV_NUM         devNum,
    IN CPSS_PX_INIT_INFO_STC *ppInitParamsPtr
)
{
    GT_STATUS rc;
    GT_BOOL   allTheTablesIteration;
    GT_U32    index;


    CPSS_NULL_PTR_CHECK_MAC(ppInitParamsPtr);
    switch(ppInitParamsPtr->numOfDataIntegrityElements)
    {
        case 0:
            /* nothing to do */
            return GT_OK;
        case CPSS_PX_SHADOW_TYPE_ALL_CPSS_CNS:
            allTheTablesIteration = GT_TRUE;
            break;
        default:
            allTheTablesIteration = GT_FALSE;
            break;
    }

    if(GT_TRUE == allTheTablesIteration)
    {
        /* iterate all the hw tables that under CPSS control*/
        for(index = 0; index < CPSS_PX_TABLE_LAST_E; index++)
        {
            rc = prvCpssInitHwTableShadow(devNum, index);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

    }
    else
    {
        GT_U32 j;
        const CPSS_PX_TABLE_ENT *hwNameArr;

        /* iterate requested list of logical tables */
        for(index = 0; index <ppInitParamsPtr->numOfDataIntegrityElements; index++)
        {
            CPSS_PX_LOGICAL_TABLES_SHADOW_STC *logicalEntryPtr = &(ppInitParamsPtr->dataIntegrityShadowPtr[index]);
            switch(logicalEntryPtr->shadowType)
            {
                case CPSS_PX_SHADOW_TYPE_NONE_E:
                    /* skip inactive entry */
                    continue;
                case CPSS_PX_SHADOW_TYPE_CPSS_E:
                     break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            rc = prvCpssPxDiagDataIntegrityTableLogicalToHwListGet(logicalEntryPtr->logicalTableName, &hwNameArr);
            if(rc != GT_OK)
            {
                return rc;
            }

            for(j = 0; hwNameArr[j] != LAST_VALID_PX_TABLE_CNS; j++)
            {
                rc = prvCpssInitHwTableShadow(devNum, hwNameArr[j]);
                if(rc != GT_OK)
                {
                    return rc;
                }
            } /* end of hw tables iteration */

        } /* end of logical tables iteration */
    }

    return GT_OK;
}


/**
* @internal internal_cpssPxDiagDataIntegrityShadowTableSizeGet
*           function
* @endinternal
*
* @brief   This function calculates size of memory required for
*          each shadow table in the list and total shadow memory
*          size
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in,out] tablesInfoPtr        - in : info for which logical tables need to
*                                            calculate size of shadow
*                                       out: info filled with size per supported table
*
* @param[out] totalMemSizePtr          - (pointer to) total memory size required for the shadow
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxDiagDataIntegrityShadowTableSizeGet
(
    IN     GT_SW_DEV_NUM                             devNum,
    INOUT  CPSS_PX_LOGICAL_SHADOW_TABLES_INFO_STC    *tablesInfoPtr,
    OUT    GT_U32                                    *totalMemSizePtr
)
{
    GT_STATUS rc;

    GT_BOOL   allTheTablesIteration;
    GT_U32    index;
    GT_U32    j;
    const CPSS_PX_TABLE_ENT *hwNameArr;
    CPSS_PX_LOGICAL_TABLE_SHADOW_INFO_STC *logicalEntryPtr;

    /* check parameters */
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(tablesInfoPtr);
    CPSS_NULL_PTR_CHECK_MAC(totalMemSizePtr);

    switch(tablesInfoPtr->numOfDataIntegrityElements)
    {
        case 0:
        case CPSS_PX_SHADOW_TYPE_ALL_CPSS_CNS:
            allTheTablesIteration = GT_TRUE;
            break;
        default:
            allTheTablesIteration = GT_FALSE;
            break;
    }

    *totalMemSizePtr = 0;

    if(GT_TRUE == allTheTablesIteration)
    {
        /* iterate all the hw tables */
        for(index = 0; index < CPSS_PX_LOGICAL_TABLE_LAST_E; index++)
        {
            rc = prvCpssPxDiagDataIntegrityTableLogicalToHwListGet(index, &hwNameArr);
            if(rc != GT_OK)
            {
                return rc;
            }
            for(j = 0; hwNameArr[j] != LAST_VALID_PX_TABLE_CNS; j++)
            {
                if(GT_FALSE ==
                    prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTable(devNum,hwNameArr[j],0))
                {
                    /*skip HW table that is not relevant to current device*/
                    continue;
                }

                *totalMemSizePtr += prvCpssCalculateHwTableShadowSize(devNum, hwNameArr[j]);
            }
        }
    }
    else
    {
        CPSS_NULL_PTR_CHECK_MAC(tablesInfoPtr->logicalTablesArr);


        /* iterate requested list of logical tables */
        for(index = 0; index < tablesInfoPtr->numOfDataIntegrityElements; index++)
        {
            logicalEntryPtr = &(tablesInfoPtr->logicalTablesArr[index]);
            CPSS_NULL_PTR_CHECK_MAC(logicalEntryPtr);

            switch(logicalEntryPtr->shadowType)
            {
                case CPSS_PX_SHADOW_TYPE_NONE_E:
                    continue;
                case CPSS_PX_SHADOW_TYPE_CPSS_E:
                    break; /* ok */
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }


            rc = prvCpssPxDiagDataIntegrityTableLogicalToHwListGet(logicalEntryPtr->logicalTableName, &hwNameArr);
            if(rc != GT_OK)
            {
                return rc;
            }

            logicalEntryPtr->isSupported = GT_OK;
            logicalEntryPtr->numOfBytes = 0;

            for(j = 0; hwNameArr[j] != LAST_VALID_PX_TABLE_CNS; j++)
            {
                if(GT_FALSE ==
                    prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTable(devNum,hwNameArr[j],0))
                {
                    /*skip HW table that is not relevant to current device*/
                    continue;
                }

                /* add this HW table size to total of this logical table */
                logicalEntryPtr->numOfBytes  +=
                    prvCpssCalculateHwTableShadowSize(devNum, hwNameArr[j]);

            } /* end of hw tables iteration */

            /* add this logical table size to total */
            *totalMemSizePtr += logicalEntryPtr->numOfBytes;
        } /* end of logical tables iteration */
    }

    return GT_OK;
}

/**
* @internal cpssPxDiagDataIntegrityShadowTableSizeGet function
* @endinternal
*
* @brief   This function calculates size of memory required for
*          each shadow table and total shadow memory size
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in,out] tablesInfoPtr        - in : info for which logical tables need to
*                                            calculate size of shadow
*                                       out: info filled with size per supported table
*
* @param[out] totalMemSizePtr          - (pointer to) total memory size required for the shadow
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagDataIntegrityShadowTableSizeGet
(
    IN     GT_SW_DEV_NUM                            devNum,
    INOUT  CPSS_PX_LOGICAL_SHADOW_TABLES_INFO_STC   *tablesInfoPtr,
    OUT    GT_U32                                   *totalMemSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxDiagDataIntegrityShadowTableSizeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tablesInfoPtr, totalMemSizePtr));

    rc = internal_cpssPxDiagDataIntegrityShadowTableSizeGet(devNum, tablesInfoPtr, totalMemSizePtr);
    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, devNum, tablesInfoPtr, totalMemSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssPxPortGroupReadTableEntry_fromShadow function
* @endinternal
*
* @brief   Read a whole entry from the 'SHADOW' table.
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
*
* @param[out numBitsPerEntryPtr       - pointer to number of
*                                       bits per entry *
* @param[out] entryValuePtr           - (pointer to) the
*                                       data read from the entry
*                                       in the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxReadTableEntry_fromShadow
(
    IN GT_SW_DEV_NUM           devNum,
    IN CPSS_PX_TABLE_ENT       tableType,
    IN GT_U32                  entryIndex,
    OUT GT_U32                 *numBitsPerEntryPtr,/* number of bits per entry */
    OUT GT_U32                 *entryValuePtr
)
{
    GT_STATUS  rc;                          /* return code              */
    GT_U32  *shadowPtr;/* pointer to the shadow for the index in the table */
    PRV_CPSS_PX_HW_TABLES_SHADOW_STC *shadowInfoPtr;

    /* check parameters */
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(entryValuePtr);


    if(GT_FALSE == prvCpssPxDiagDataIntegrityTableCheckIsDevSupportHwTable(devNum,tableType,entryIndex))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "HW table is not applicable or index out of range");
    }

    shadowInfoPtr = &(PRV_CPSS_PX_PP_MAC(devNum)->shadowInfoArr[tableType]);

    if(CPSS_PX_SHADOW_TYPE_CPSS_E != shadowInfoPtr->shadowType)
    {
        /* the device not supports it or the table not requires shadow */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "The CPSS hold no shadow for HW table[%d]",tableType);
    }

    rc = prvCpssPxPortGroupShadowEntryGet(devNum,tableType,
                                          entryIndex, &shadowPtr,
                                          numBitsPerEntryPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    copyBits(&entryValuePtr[0], 0,     /* source base, offset */
                 shadowPtr, 0, /* target base, offset */
                 (*numBitsPerEntryPtr));         /* number of bits */

    return GT_OK;

}

/**
* @internal prvCpssPxDiagDataIntegrityShadowAndHwSynch function
* @endinternal
*
* @brief   Synchronize all cpss shadow tables with values from HW.
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxDiagDataIntegrityShadowAndHwSynch
(
    IN GT_SW_DEV_NUM                               devNum
)
{
    GT_STATUS   rc;
    PRV_CPSS_PX_HW_TABLES_SHADOW_STC *tableInfoPtr;
    CPSS_PX_TABLE_ENT  hwTable;
    GT_U32              maxHwTable;

    /* check parameters */
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    maxHwTable = PRV_CPSS_PX_TABLE_SIZE_GET_MAC(devNum);

    for(hwTable = 0 ; (GT_U32)hwTable < maxHwTable; hwTable++)
    {
        tableInfoPtr = &(PRV_CPSS_PX_PP_MAC(devNum)->shadowInfoArr[hwTable]);

        if(tableInfoPtr->shadowType != CPSS_PX_SHADOW_TYPE_CPSS_E)
        {
            continue;
        }

        /* initialize the shadow with 'HW default values' */
        rc = hwTableShadowDefaultValueSet(devNum,hwTable);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPxDiagDataIntegrityShadowAndHwSynchTable function
* @endinternal
*
* @brief   Synchronize specific table cpss shadow with values from HW.
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] hwTable                  - the HW table to Synchronize
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_SUPPORTED         - the table not supports the shadow
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxDiagDataIntegrityShadowAndHwSynchTable
(
    IN GT_SW_DEV_NUM        devNum,
    IN CPSS_PX_TABLE_ENT    hwTable
)
{
    GT_STATUS   rc;
    PRV_CPSS_PX_HW_TABLES_SHADOW_STC *tableInfoPtr;
    GT_U32                            maxHwTable;

    /* check parameters */
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    maxHwTable = PRV_CPSS_PX_TABLE_SIZE_GET_MAC(devNum);

    if((GT_U32)hwTable >= maxHwTable)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    tableInfoPtr = &(PRV_CPSS_PX_PP_MAC(devNum)->shadowInfoArr[hwTable]);

    if(tableInfoPtr->shadowType != CPSS_PX_SHADOW_TYPE_CPSS_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* initialize the shadow with 'HW default values' */
    rc = hwTableShadowDefaultValueSet(devNum,hwTable);
    if(rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;

}


/**
* @internal prvCpssPxShadowLineUpdateMasked function
* @endinternal
*
* @brief   Either write a whole entry (if mask is not specified) into PP's shadow
*         DB table at specific 'line index' or update the shadow table entry
*         according to the specified mask
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] tableType                - the 'HW table'
* @param[in] lineIndex                - the line index in the 'HW table'
*                                      (not the same index as the functions of 'table engine'
*                                      get (like prvCpssPxPortGroupWriteTableEntry))
* @param[in] entryValuePtr            - (pointer to) an array containing the data to be
*                                      copied into the shadow
* @param[in] entryMaskPtr             - (pointer to) array of mask.
*                                      If NULL - the entry specified by entryValuePtr will
*                                      be written to shadow.
*                                      If not NULL  - only bits that are raised in the mask
*                                      will be updated in HW entry.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxShadowLineUpdateMasked
(
    IN GT_SW_DEV_NUM         devNum,
    IN CPSS_PX_TABLE_ENT     tableType,
    IN GT_U32                lineIndex,
    IN GT_U32                *entryValuePtr,
    IN GT_U32                *entryMaskPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_PX_HW_TABLES_SHADOW_STC *shadowInfoPtr = &(PRV_CPSS_PX_PP_MAC(devNum)->shadowInfoArr[tableType]);
    GT_U32 *entryInShadowPtr;/* pointer to the entry in the shadow */
    GT_U32 numBitsPerEntry;
    GT_U32 entrySize;
    GT_U32 entrySizeInBytes;

    if(debugMode_PxforceNoUpdateOfShadow)
    {
        /* !!! debug mode that force us to not update the shadow !!! */
        return GT_OK;
    }

    if(CPSS_PX_SHADOW_TYPE_CPSS_E != shadowInfoPtr->shadowType)
    {
        /* the device not supports it or the table not requires shadow */
        return GT_OK;
    }

    rc = prvCpssPxPortGroupShadowEntryGet(devNum,tableType,
                                          lineIndex,&entryInShadowPtr,
                                          &numBitsPerEntry);
    if(rc != GT_OK)
    {
        return rc;
    }

    entrySizeInBytes = (numBitsPerEntry % 8)? (numBitsPerEntry / 8 + 1) : (numBitsPerEntry / 8);
    entrySize = (numBitsPerEntry % 32)? (numBitsPerEntry / 32 + 1) : (numBitsPerEntry / 32);;

    if (NULL == entryMaskPtr)
    {
        if (entryValuePtr !=  entryInShadowPtr)
        {
            /* write full entry as is */
            cpssOsMemCpy(entryInShadowPtr, entryValuePtr, entrySizeInBytes);
        }
    }
    else
    {
        GT_U32 i;

        for (i=0; i < entrySize; i++)
        {
            if (entryMaskPtr[i])
            {
                entryInShadowPtr[i] =
                    (entryValuePtr[i] & entryMaskPtr[i]) | (entryInShadowPtr[i] & ~entryMaskPtr[i]) ;
            };
        }
    }

    return  GT_OK;
}

