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
* @file prvCpssPxHwTables.c
*
* @brief Private API implementation for tables access.
*
* @version   1
********************************************************************************
*/

#include <cpss/px/cpssHwInit/private/prvCpssPxHwTables.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/config/cpssPxCfgInit.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* Object that holds callback function to table HW access */
CPSS_PX_CFG_HW_ACCESS_OBJ_STC prvPxTableCpssHwAccessObj =
                                            {NULL, NULL, NULL, NULL};

/*calc number of words form number of bits */
#define NUM_WORDS_FROM_BITS_MAC(x)   (((x) + 31)>>5)

/* number of elements in static array (non dynamic alloc) */
#define NUM_ELEMENTS_IN_ARR_MAC(_array) \
    sizeof(_array)/sizeof(_array[0])

#ifdef STR
    #undef STR
#endif /*STR*/

#define STR(strname)    #strname
/* generate 'value' and 'name' out of variable/enum value */
#define VALUE_AND_NAME(table) table,STR(table)

/* bind the array of fields formats to the DB of tables format according to table index
    tblDbPtr -  PRV_CPSS_PX_PP_MAC(devNum)->tableFormatInfo
    tblFormatIndex - value from PRV_CPSS_PX_TABLE_FORMAT_ENT
    tblFormatArr - static array of type PRV_CPSS_ENTRY_FORMAT_TABLE_STC
*/
#define BIND_FIELDS_FORMAT_TO_TABLE_MAC(tblDbPtr , tblFormatIndex , tblFormatArr ) \
    tblDbPtr[tblFormatIndex].fieldsInfoPtr = tblFormatArr;                       \
    tblDbPtr[tblFormatIndex].numFields =  NUM_ELEMENTS_IN_ARR_MAC(tblFormatArr); \
    tblDbPtr[tblFormatIndex].patternNum = 0;                                     \
    tblDbPtr[tblFormatIndex].patternBits = 0

/* bind the array of fields formats to the DB of tables format according to table index
    tblDbPtr -  PRV_CPSS_PX_PP_MAC(devNum)->tableFormatInfo
    tblFormatIndex - value from PRV_CPSS_PX_TABLE_FORMAT_ENT
    tblFormatArr - static array of type PRV_CPSS_ENTRY_FORMAT_TABLE_STC
    tblPatternBits - number of bits in pattern
    tblPatterNum - number of patterns in table entry
*/
#define BIND_PATTERN_FIELDS_FORMAT_TO_TABLE_MAC(tblDbPtr , tblFormatIndex , tblFormatArr, tblPatternBits, tblPatternNum ) \
    tblDbPtr[tblFormatIndex].fieldsInfoPtr = tblFormatArr;                      \
    tblDbPtr[tblFormatIndex].numFields =  NUM_ELEMENTS_IN_ARR_MAC(tblFormatArr); \
    tblDbPtr[tblFormatIndex].patternBits = tblPatternBits;                       \
    tblDbPtr[tblFormatIndex].patternNum = tblPatternNum


/* access to info of the PX device tables */
#define PRV_CPSS_PX_DEV_TBLS_MAC(devNum)  \
    (PRV_CPSS_PX_PP_MAC(devNum)->accessTableInfoPtr)


/* macro to get the alignment as number of words from the number of bits in the entry */
#define BITS_TO_BYTES_ALIGNMENT_MAC(bits) \
    (((bits) > 512) ? 128 :      \
     ((bits) > 256) ?  64 :      \
     ((bits) > 128) ?  32 :      \
     ((bits) >  64) ?  16 :      \
     ((bits) >  32) ?   8 :   4)

/* max number of words in entry */
#define MAX_ENTRY_SIZE_CNS   64

/* check that the number of words needed can be supported by MAX_ENTRY_SIZE_CNS */
#define CHECK_NEEDED_HW_ENTRY_SIZE_MAC(hwEntryNeededWords,tableType)\
    if(hwEntryNeededWords > MAX_ENTRY_SIZE_CNS)                    \
    {                                                              \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "MAX_ENTRY_SIZE_CNS[%d] not supporting table [%d] with size[%d]", \
            MAX_ENTRY_SIZE_CNS,                                    \
            tableType,                                             \
            hwEntryNeededWords);                                   \
    }


/* cheetah default value for next word offset for table direct access */
#define _4_BYTES_CNS           4


typedef enum {
    SPECIAL_ADDRESS_NOT_EXISTS_E   = 0x0FFFFFFF /* non exists address */
}SPECIAL_ADDRESS_ENT;

/* this table hold ONLY valid tables relevant to this device */
/* it will be used to FILL pipeTablesInfo__final that can be indexed by 'tableType' */
static PRV_CPSS_PX_TABLES_INFO_STC pipeTablesInfo__tempo[] =
{
    /*BMA*/
     {VALUE_AND_NAME(CPSS_PX_TABLE_BMA_PORT_MAPPING_E), PRV_CPSS_DXCH_UNIT_BMA_E,
    {0x0005A000, (7), 17/*HW is 512 (because copied 'AS IS' from BC3) , but not allow more than 17 !*/, 0, 0}}

    ,{VALUE_AND_NAME(CPSS_PX_TABLE_BMA_MULTICAST_COUNTERS_E), PRV_CPSS_DXCH_UNIT_BMA_E,
    {0x00000000, (12), _8K, 0, 0}}

    /*TXQ-DQ*/
    ,{VALUE_AND_NAME(CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E), PRV_CPSS_DXCH_UNIT_TXQ_DQ_E,
    {0x00006000 , (488)/*61*8*/, 10, 0, 0}}

    ,{VALUE_AND_NAME(CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E), PRV_CPSS_DXCH_UNIT_TXQ_DQ_E,
    {0x0000A000, (61), 10, 0, 0}}

    ,{VALUE_AND_NAME(CPSS_PX_TABLE_TXQ_DQ_PFC_RESPONSE_PORT_MAPPING_E), PRV_CPSS_DXCH_UNIT_TXQ_DQ_E,
    {0x00011000, (4),20, 0, 0 }}

    /*TXQ-QUEUE*/

    /* Tail Drop Tables */
    ,{VALUE_AND_NAME(CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_MAX_QUEUE_LIMITS_E), PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,
    {0x000A1000, (36), 128, 0, 0}}

    ,{VALUE_AND_NAME(CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP0_E), PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,
    {0x000A1800, (57), 128, 0, 0}}

    ,{VALUE_AND_NAME(CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP1_E), PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,
    {0x000A3000, (57), 128, 0, 0}}

    ,{VALUE_AND_NAME(CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_LIMITS_DP2_E), PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,
    {0x000A3400, (57), 128, 0, 0}}

    ,{VALUE_AND_NAME(CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_BUF_LIMITS_E), PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,
    {0x000A2000, (36), 128, 0, 0}}

    ,{VALUE_AND_NAME(CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_DESC_LIMITS_E), PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,
    {0x000A2800, (16), 128, 0, 0}}

    ,{VALUE_AND_NAME(CPSS_PX_TABLE_TXQ_QUEUE_TAIL_DROP_COUNTERS_Q_MAIN_BUFF_E), PRV_CPSS_DXCH_UNIT_TXQ_QUEUE_E,
    {0x000d0000, (20), 160, 0, 0}}

    /*TXQ-PFC*/
    ,{VALUE_AND_NAME(CPSS_PX_TABLE_PFC_LLFC_COUNTERS_E), PRV_CPSS_DXCH_UNIT_TXQ_PFC_E,
    {0x00008000, (22), 136, 0, 0}}

    /*TXQ-QCN*/
    ,{VALUE_AND_NAME(CPSS_PX_TABLE_TXQ_QCN_CN_SAMPLE_INTERVALS_E), PRV_CPSS_DXCH_UNIT_TXQ_QCN_E,
    {0x00010000, (40), 160, 0, 0}}

    /*CNC*/
    ,{VALUE_AND_NAME(CPSS_PX_TABLE_CNC_0_COUNTERS_E) , PRV_CPSS_DXCH_UNIT_CNC_0_E ,
    {0x00010000, (64), _1K, 0, 0}}
    /*PCP*/
    ,{VALUE_AND_NAME(CPSS_PX_TABLE_PCP_DST_PORT_MAP_TABLE_E) , PRV_CPSS_PX_UNIT_PCP_E ,
    {0x00008000, (17), _8K, 0, 0}}
    ,{VALUE_AND_NAME(CPSS_PX_TABLE_PCP_PORT_FILTERING_TABLE_E) , PRV_CPSS_PX_UNIT_PCP_E ,
    {0x00018000, (17), _4K, 0, 0}}

    /*PHA*/
    ,{VALUE_AND_NAME(CPSS_PX_TABLE_PHA_SHARED_DMEM_E) ,      PRV_CPSS_PX_UNIT_PHA_E ,
    {0x007C0000, (32), _2K, 0, 0}}
    ,{VALUE_AND_NAME(CPSS_PX_TABLE_PHA_HA_TABLE_E) ,         PRV_CPSS_PX_UNIT_PHA_E ,
    {0x007F0000, (144), 544, 0, 0}}
    ,{VALUE_AND_NAME(CPSS_PX_TABLE_PHA_SRC_PORT_DATA_E) ,    PRV_CPSS_PX_UNIT_PHA_E ,
    {0x007F8000, (32), 17, 0, 0}}
    ,{VALUE_AND_NAME(CPSS_PX_TABLE_PHA_TARGET_PORT_DATA_E) , PRV_CPSS_PX_UNIT_PHA_E ,
    {0x007F8400, (64), 17, 0, 0}}
    ,{VALUE_AND_NAME(CPSS_PX_TABLE_PHA_FW_IMAGE_E) ,         PRV_CPSS_PX_UNIT_PHA_E ,
    {0x00040000, (128), 0x3fff, 0, 0}}

    /* MUST be last */
    ,{CPSS_PX_TABLE_LAST_E,NULL,0,
     {0, 0, 0, 0, 0}}
};

/**************************/
/* MULTI-instance support */
/**************************/
typedef struct{
    CPSS_PX_TABLE_ENT       origTable;
    CPSS_PX_TABLE_ENT       multiInstanceTable;
    PRV_CPSS_PX_TABLE_NAME  multiInstanceTableName;
    PRV_CPSS_DXCH_UNIT_ENT  multiInstanceUnit;
}MULTI_INSTANCE_UNITS_INFO;
#define DUP_DQ0_TO_DQ1(tableDq0,tableDq1) \
     tableDq0, VALUE_AND_NAME(tableDq1),PRV_CPSS_DXCH_UNIT_TXQ_DQ1_E
/* allow a table from duplicated unit to take automatically all info from original unit*/
static MULTI_INSTANCE_UNITS_INFO  pipeMultiInstanceUnitsTablesInfo[]=
{
     {DUP_DQ0_TO_DQ1(CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E,
        CPSS_PX_MULTI_INSTANCE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG____DQ_1___E)}
    ,{DUP_DQ0_TO_DQ1(CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E,
        CPSS_PX_MULTI_INSTANCE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG____DQ_1___E)}
    ,{DUP_DQ0_TO_DQ1(CPSS_PX_TABLE_TXQ_DQ_PFC_RESPONSE_PORT_MAPPING_E,
        CPSS_PX_MULTI_INSTANCE_TXQ_DQ_PFC_RESPONSE_PORT_MAPPING_E____DQ_1___E)}

    /* MUST be last */
    ,{CPSS_PX_TABLE_LAST_E,CPSS_PX_TABLE_LAST_E,NULL,0}
};

typedef struct{
    CPSS_PX_TABLE_ENT       origTable;
    CPSS_PX_TABLE_ENT       multiInstanceTable;
    PRV_CPSS_PX_TABLE_NAME  multiInstanceTableName;
    GT_U32                  multiInstanceBaseAddr;
}MULTI_INSTANCE_ADDR_INFO;

/* allow a table from same unit to take automatically all info from original table
   (just different baseAddr) */
static MULTI_INSTANCE_ADDR_INFO  pipeMultiInstanceAddrTablesInfo[]=
{
    {CPSS_PX_TABLE_CNC_0_COUNTERS_E,VALUE_AND_NAME(CPSS_PX_TABLE_CNC_1_COUNTERS_E),
     (0x10000 + 0x2000)}

    /* MUST be last */
    ,{CPSS_PX_TABLE_LAST_E,CPSS_PX_TABLE_LAST_E,NULL,0}
};

static PRV_CPSS_PX_TABLES_INFO_STC pipeTablesInfo__final[CPSS_PX_TABLE_LAST_E] =
{
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}},
    {0, NULL, 0, {0,0,0,0,0}}
    /* filled in RUNTIME based on :
     pipeTablesInfo__tempo,
     pipeMultiInstanceUnitsTablesInfo,
     pipeMultiInstanceAddrTablesInfo,
    */
};
/* indication that the pipeTablesInfo__final was already set */
static GT_BOOL pipeTablesInfo__final_ready = GT_FALSE;

/**
* @internal initTableInfoForDevice function
* @endinternal
*
* @brief   bind and Initializes all info needed for table access for the device.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*/
static GT_STATUS  initTableInfoForDevice
(
    IN GT_SW_DEV_NUM devNum
)
{
    GT_U32  ii;
    PRV_CPSS_PX_TABLES_INFO_STC *trgInfoPtr;
    PRV_CPSS_PX_TABLES_INFO_STC *srcInfoPtr;
    PRV_CPSS_PX_TABLES_INFO_DIRECT_STC *trgDirectInfoPtr;
    MULTI_INSTANCE_UNITS_INFO   *multiUnitInfoPtr;
    MULTI_INSTANCE_ADDR_INFO    *multiAddrInfoPtr;
    GT_BOOL                     didError;
    PRV_CPSS_DXCH_UNIT_ENT      orig_unitIndex,unitIndex;/* unit index */
    GT_U32                      orig_unitBaseAddr,unitBaseAddr;/* base address of the unit in the device */

    /********************************************************************/
    /* set to default 'not valid' of ALL entries in the DB about tables */
    /********************************************************************/
    trgInfoPtr = &pipeTablesInfo__final[0];
    for(ii = 0 ; ii < CPSS_PX_TABLE_LAST_E ; ii++,trgInfoPtr++)
    {
        trgInfoPtr->globalIndex = CPSS_PX_TABLE_LAST_E;
        trgInfoPtr->tableName = "unknown";
        trgInfoPtr->unitIndex = PRV_CPSS_DXCH_UNIT_LAST_E;
        trgInfoPtr->directAccessInfo.baseAddress = SPECIAL_ADDRESS_NOT_EXISTS_E;
        trgInfoPtr->directAccessInfo.numOfBits = 0;
        trgInfoPtr->directAccessInfo.maxNumOfEntries = 0;
        trgInfoPtr->directAccessInfo.alignmentWidthInBytes = 0;
        trgInfoPtr->directAccessInfo.entryWidthInWords = 0;
    }

    /***************************************************/
    /* fill DB entries that are listed for this family */
    /***************************************************/
    srcInfoPtr = &pipeTablesInfo__tempo[0];
    for(ii = 0 ; srcInfoPtr->globalIndex != CPSS_PX_TABLE_LAST_E ; ii++,srcInfoPtr++)
    {

        if (srcInfoPtr->globalIndex == CPSS_PX_TABLE_PCP_DST_PORT_MAP_TABLE_E)
        {
            if(PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
            {
                srcInfoPtr->directAccessInfo.numOfBits = 18;
            }
        }
        trgInfoPtr = &pipeTablesInfo__final[srcInfoPtr->globalIndex];

        *trgInfoPtr = *srcInfoPtr;

        trgDirectInfoPtr = &trgInfoPtr->directAccessInfo;

        unitIndex = trgInfoPtr->unitIndex;
        /* update the base address in indirectTablePtr */
        unitBaseAddr = prvCpssPxHwUnitBaseAddrGet(devNum,unitIndex,&didError);
        if(didError == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "unitIndex[%d] failed",unitIndex);
        }

        trgDirectInfoPtr->baseAddress += unitBaseAddr;

        trgDirectInfoPtr->alignmentWidthInBytes =
            BITS_TO_BYTES_ALIGNMENT_MAC(trgDirectInfoPtr->numOfBits);
        trgDirectInfoPtr->entryWidthInWords =
            BITS_TO_WORDS_MAC(trgDirectInfoPtr->numOfBits);
    }

    /***********************************/
    /* handle multi-instances of units */
    /***********************************/
    multiUnitInfoPtr = &pipeMultiInstanceUnitsTablesInfo[0];
    for(ii = 0 ; multiUnitInfoPtr->origTable != CPSS_PX_TABLE_LAST_E ; ii++,multiUnitInfoPtr++)
    {
        trgInfoPtr = &pipeTablesInfo__final[multiUnitInfoPtr->multiInstanceTable];
        srcInfoPtr = &pipeTablesInfo__final[multiUnitInfoPtr->origTable];

        orig_unitIndex = srcInfoPtr->unitIndex;

        *trgInfoPtr = *srcInfoPtr;
        trgInfoPtr->tableName =  multiUnitInfoPtr->multiInstanceTableName;
        trgInfoPtr->unitIndex =  multiUnitInfoPtr->multiInstanceUnit;

        trgDirectInfoPtr = &trgInfoPtr->directAccessInfo;


        orig_unitBaseAddr = prvCpssPxHwUnitBaseAddrGet(devNum,orig_unitIndex,&didError);
        if(didError == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "orig_unitIndex[%d] failed",orig_unitIndex);
        }

        unitIndex = trgInfoPtr->unitIndex;

        unitBaseAddr = prvCpssPxHwUnitBaseAddrGet(devNum,unitIndex,&didError);
        if(didError == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "unitIndex[%d] failed",unitIndex);
        }

        /* restore relative base address */
        trgDirectInfoPtr->baseAddress -= orig_unitBaseAddr;
        /* set new base address */
        trgDirectInfoPtr->baseAddress += unitBaseAddr;

    }

    /*************************************/
    /* handle multi-instances of Address */
    /*************************************/
    multiAddrInfoPtr = &pipeMultiInstanceAddrTablesInfo[0];
    for(ii = 0 ; multiAddrInfoPtr->origTable != CPSS_PX_TABLE_LAST_E ; ii++,multiAddrInfoPtr++)
    {
        trgInfoPtr = &pipeTablesInfo__final[multiAddrInfoPtr->multiInstanceTable];
        srcInfoPtr = &pipeTablesInfo__final[multiAddrInfoPtr->origTable];

        orig_unitIndex = srcInfoPtr->unitIndex;

        *trgInfoPtr = *srcInfoPtr;
        trgInfoPtr->tableName =  multiAddrInfoPtr->multiInstanceTableName;

        trgDirectInfoPtr = &trgInfoPtr->directAccessInfo;

        orig_unitBaseAddr = prvCpssPxHwUnitBaseAddrGet(devNum,orig_unitIndex,&didError);
        if(didError == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "orig_unitIndex[%d] failed",orig_unitIndex);
        }

        /* set new base address */
        trgDirectInfoPtr->baseAddress = orig_unitBaseAddr + multiAddrInfoPtr->multiInstanceBaseAddr;
    }

    return GT_OK;

}


GT_STATUS prvCpssPxPortGroupWriteTableEntry
(
    IN GT_SW_DEV_NUM     devNum,
    IN GT_U32                   portGroupId,
    IN CPSS_PX_TABLE_ENT  tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                  *entryValuePtr
);

static GT_STATUS prvCpssPxPortGroupWriteTableEntryField
(
    IN GT_SW_DEV_NUM   devNum,
    IN GT_U32                 portGroupId,
    IN CPSS_PX_TABLE_ENT    tableType,
    IN GT_U32                 entryIndex,
    IN GT_U32                 fieldWordNum,
    IN GT_U32                 fieldOffset,
    IN GT_U32                 fieldLength,
    IN GT_U32                 fieldValue
);

GT_STATUS prvCpssPxShadowLineWrite
(
    IN  GT_U8                 devNum,
    IN  CPSS_PX_TABLE_ENT     tableType,
    IN  GT_U32                lineIndex,
    IN  GT_U32                *entryValuePtr
);

void  copyBits(
    IN GT_U32                  *targetMemPtr,
    IN GT_U32                  targetStartBit,
    IN GT_U32                  *sourceMemPtr,
    IN GT_U32                  sourceStartBit,
    IN GT_U32                  numBits
);


/**
* @internal indexAsPortNumConvert function
* @endinternal
*
* @brief   convert index that is portNum ('Global port num') to portGroupId and
*         'Local port num'
*         for tables that are not with index = portNum , no modification in the
*         index , and portGroupId will be 'first active port group == 0'
*         for non multi-port-groups device : portGroupId is 'all port groups' , no index conversion
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
*
* @param[out] portGroupIdPtr           - (pointer to) the portGroupId to use
* @param[out] updatedEntryIndexPtr     - (pointer to) the updated index
* @param[out] updatedTableTypePtr      - (pointer to) the NEW specific table name to use
*                                       none
*/
static void indexAsPortNumConvert
(
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PX_TABLE_ENT        tableType,
    IN GT_U32                   entryIndex,
    OUT GT_U32                 *portGroupIdPtr,
    OUT GT_U32                 *updatedEntryIndexPtr,
    OUT CPSS_PX_TABLE_ENT      *updatedTableTypePtr
)
{
    GT_U32  offset;  /* temp offset of port from the base port index */
    GT_U32  newIndex;/* temp index for the port calculation */
    CPSS_PX_TABLE_ENT newTableOffset = 0;

    /* converts that done also for 'non-multi port group' device */
    *updatedTableTypePtr = tableType;

    /*default is 'unaware'*/
    *portGroupIdPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    switch(tableType)
    {
        case CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E:
        case CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E:
            offset = entryIndex / PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;
            newIndex = entryIndex % PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;

            if(tableType == CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E)
            {
                newTableOffset = CPSS_PX_MULTI_INSTANCE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG____DQ_1___E;
            }
            else
            if(tableType == CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E)
            {
                newTableOffset = CPSS_PX_MULTI_INSTANCE_TXQ_DQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG____DQ_1___E;
            }

            if(offset != 0)
            {
                *updatedTableTypePtr = newTableOffset + (offset-1);

                if(offset >= PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.multiDataPath.numTxqDq)
                {
                    /* ERROR */
                    *updatedTableTypePtr = CPSS_PX_TABLE_LAST_E;/* will generate error in the caller */
                    newIndex = 0;
                }
            }
            entryIndex = newIndex;
            break;
        default:
            break;
    }

    /* no conversion */
    *updatedEntryIndexPtr = entryIndex;

    return ;
}

/*******************************************************************************
* tableFieldsFormatInfoGet
*
* DESCRIPTION:
*       function return the table format that relate to the specific table.
*       for not supported table --> return NULL
*
* INPUTS:
*       devNum          - device number
*       tableType       - the specific table name
*
* OUTPUTS:
*       None.
*
*
* RETURNS:
*       pointer to 'Table format info'
*
* COMMENTS:
*
*
*******************************************************************************/
static PRV_CPSS_PX_PP_TABLE_FORMAT_INFO_STC *tableFieldsFormatInfoGet
(
    IN GT_SW_DEV_NUM devNum,
    IN CPSS_PX_TABLE_ENT tableType
)
{
    devNum = devNum;
    tableType = tableType;

    /* no table yet implemented 'fields' */

    return NULL;
}

/**
* @internal convertFieldInfoToGlobalBit function
* @endinternal
*
* @brief   function converts : fieldWordNum,fieldOffset,fieldLength
*         from value relate to PRV_CPSS_PX_TABLE_WORD_INDICATE_FIELD_NAME_CNS
* @param[in] devNum                   - device number
* @param[in] tableType                - the specific table name
* @param[in,out] fieldWordNumPtr          - field word number  PRV_CPSS_PX_TABLE_WORD_INDICATE_FIELD_NAME_CNS
* @param[in,out] fieldOffsetPtr           - field offset       field name
* @param[in,out] fieldLengthPtr           - field length        number of fields or bit offset in field
* @param[in] value                    -  is checked to not 'over flow' the number of bits.
*                                      this is to remove the need from the 'cpss APIs' to be aware to field size!
* @param[in,out] fieldWordNumPtr          - field word number PRV_CPSS_PX_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
* @param[in,out] fieldOffsetPtr           - field offset      global bit position
* @param[in,out] fieldLengthPtr           - field length       number of bits
*
* @retval GT_OK                    - success
* @retval GT_BAD_STATE             - fail
* @retval GT_OUT_OF_RANGE          - when value > (max valid value)
*/
static GT_STATUS    convertFieldInfoToGlobalBit(
    IN GT_SW_DEV_NUM   devNum,
    IN CPSS_PX_TABLE_ENT tableType,
    INOUT GT_U32   *fieldWordNumPtr,
    INOUT GT_U32   *fieldOffsetPtr,
    INOUT GT_U32   *fieldLengthPtr,
    IN    GT_U32    value
)
{
    PRV_CPSS_PX_PP_TABLE_FORMAT_INFO_STC *tableFieldsFormatInfoPtr;
    PRV_CPSS_ENTRY_FORMAT_TABLE_STC *tableFieldsFormatPtr;
    GT_U32   fieldId = *fieldOffsetPtr;
    GT_U32   totalLength;
    GT_U32   numOfFields = *fieldLengthPtr;
    GT_U32   tmpLen;
    GT_U32   tmpOffset;

    tableFieldsFormatInfoPtr = tableFieldsFormatInfoGet(devNum, tableType);
    if(tableFieldsFormatInfoPtr == NULL)
    {
        /* the table is not bound to fields format */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "tableType[%d] not bound to fields format (case1)",tableType);
    }

    tableFieldsFormatPtr = tableFieldsFormatInfoPtr->fieldsInfoPtr;

    if(tableFieldsFormatPtr == NULL)
    {
        /* the table is not bound to fields format */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "tableType[%d] not bound to fields format (case2)",tableType);
    }

    /* convert the value into 'global field offsets' */
    *fieldWordNumPtr = PRV_CPSS_PX_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS;
    GET_FIELD_INFO_BY_ID_MAC(tableFieldsFormatPtr, fieldId, *fieldOffsetPtr,totalLength);

    if(numOfFields)
    {
        /* indication to use the 'Auto calc' or to use 'Length caller' ...
           this to support setting of consecutive' fields ! */
        while(--numOfFields)
        {
            fieldId ++;
            GET_FIELD_INFO_BY_ID_MAC(tableFieldsFormatPtr, fieldId, tmpOffset,tmpLen);
            totalLength += tmpLen;
        }
    }

    *fieldLengthPtr = totalLength;

    if(totalLength < 32 &&
        (value > BIT_MASK_MAC(totalLength)))
    {
        /* value > (max valid value) */
        CPSS_DATA_CHECK_MAX_MAC(value,1<<totalLength);
    }

    return GT_OK;
}

/**
* @internal convertSubFieldInfoToGlobalBit function
* @endinternal
*
* @brief   the function retain parameters of 'sub-field' with in 'parent field'.
*         function converts : fieldWordNum,fieldOffset,fieldLength
*         from value relate to PRV_CPSS_PX_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*         to value relate to PRV_CPSS_PX_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
* @param[in] devNum                   - device number
* @param[in] tableType                - the specific table name
* @param[in] subFieldOffset           - the bit index of the sub field (within the 'parent field')
* @param[in] subFieldLength           - the length of the sub field
* @param[in,out] fieldWordNumPtr          - field word number  PRV_CPSS_PX_TABLE_WORD_INDICATE_FIELD_NAME_CNS
* @param[in,out] fieldOffsetPtr           - field offset       field name
* @param[in,out] fieldLengthPtr           - not used
* @param[in] value                    -  is checked to not 'over flow' the number of bits.
*                                      this is to remove the need from the 'cpss APIs' to be aware to field size!
* @param[in,out] fieldWordNumPtr          - field word number PRV_CPSS_PX_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
* @param[in,out] fieldOffsetPtr           - field offset      global bit position
* @param[in,out] fieldLengthPtr           - field length       number of bits
*
* @retval GT_OK                    - success
* @retval GT_BAD_STATE             - fail
*/
static GT_STATUS    convertSubFieldInfoToGlobalBit(
    IN GT_SW_DEV_NUM  devNum,
    IN CPSS_PX_TABLE_ENT tableType,
    IN    GT_U32   subFieldOffset,
    IN    GT_U32   subFieldLength,
    INOUT GT_U32   *fieldWordNumPtr,
    INOUT GT_U32   *fieldOffsetPtr,
    INOUT GT_U32   *fieldLengthPtr,
    IN    GT_U32    value
)
{
    GT_STATUS    rc;


    /* rest the fieldLength , before calling convertFieldInfoToGlobalBit */
    *fieldLengthPtr = PRV_CPSS_PX_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS;

    rc = convertFieldInfoToGlobalBit(devNum,tableType,fieldWordNumPtr,fieldOffsetPtr,fieldLengthPtr,value);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* (*fieldWordNumPtr) value is PRV_CPSS_PX_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS */

    /* calc global bit index of the sub field */
    (*fieldOffsetPtr) += subFieldOffset;

    /* use the length as defined by the caller for the sub field */
    *fieldLengthPtr = subFieldLength;

    return GT_OK;

}

/**
* @internal convertPatternFieldInfoToGlobalBit function
* @endinternal
*
* @brief   function converts : fieldWordNum,fieldOffset,fieldLength
*         from value related to PRV_CPSS_PX_TABLE_WORD_INDICATE_PATTERN_FIELD_NAME_CNS
*         to value relate to PRV_CPSS_PX_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
* @param[in] devNum                   - device number
* @param[in] tableType                - the specific table name
* @param[in,out] entryIndexPtr            - the entry index
* @param[in,out] fieldWordNumPtr          - field word number  PRV_CPSS_PX_TABLE_WORD_INDICATE_PATTERN_FIELD_NAME_CNS
* @param[in,out] fieldOffsetPtr           - field offset       field name
* @param[in,out] fieldLengthPtr           - not used
* @param[in] val                      - value is checked to not 'overflow' the number of bits.
*                                      this is to remove the need from the 'cpss APIs' to be aware to field size!
* @param[in,out] entryIndexPtr            - the table entry index
* @param[in,out] fieldWordNumPtr          - field word number PRV_CPSS_PX_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
* @param[in,out] fieldOffsetPtr           - field offset      global bit position
* @param[in,out] fieldLengthPtr           - field length       number of bits
*
* @retval GT_OK                    - success
* @retval GT_BAD_STATE             - fail
*/
static GT_STATUS    convertPatternFieldInfoToGlobalBit(
    IN GT_SW_DEV_NUM  devNum,
    IN CPSS_PX_TABLE_ENT tableType,
    INOUT GT_U32   *entryIndexPtr,
    INOUT GT_U32   *fieldWordNumPtr,
    INOUT GT_U32   *fieldOffsetPtr,
    INOUT GT_U32   *fieldLengthPtr,
    IN    GT_U32    val
)
{
    GT_STATUS    rc;
    PRV_CPSS_PX_PP_TABLE_FORMAT_INFO_STC *  fieldsInfoPtr;
    GT_U32 patternIndex;

    fieldsInfoPtr = tableFieldsFormatInfoGet(devNum, tableType);
    if(fieldsInfoPtr == NULL)
    {
        /* the table is not bound to fields format */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "tableType[%d] not bound to fields format",tableType);
    }

    /* Convert entry index to pattern index */
    patternIndex = (*entryIndexPtr) % fieldsInfoPtr->patternNum;

    /* Claculate global field bit and length */
    rc = convertFieldInfoToGlobalBit(devNum, tableType, fieldWordNumPtr, fieldOffsetPtr, fieldLengthPtr, val);
    if(rc != GT_OK)
    {
        return rc;
    }


    /* Calculate global bit index pattern field */
    (*fieldOffsetPtr) += patternIndex * fieldsInfoPtr->patternBits;

    /* Calculate table entry index */
    (*entryIndexPtr) /= fieldsInfoPtr->patternNum;

    return GT_OK;

}

/**
* @internal prvCpssPxReadTableEntry function
* @endinternal
*
* @brief   Read a whole entry from the table.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
*
* @param[out] entryValuePtr            - (pointer to) the data read from the table
*                                      may be NULL in the case of indirect table.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of entryValuePtr == NULL and indirect table the function
*       just send read entry command to device. And entry is stored in the
*       data registers of a indirect table
*
*/
GT_STATUS prvCpssPxReadTableEntry

(
    IN GT_SW_DEV_NUM    devNum,
    IN CPSS_PX_TABLE_ENT tableType,
    IN GT_U32                  entryIndex,
    OUT GT_U32                 *entryValuePtr
)
{
    GT_U32      portGroupId;/* support multi-port-groups device for table accesses with index = 'per port' */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    /* convert if needed the entry index , get specific portGroupId (or all port groups) */
    indexAsPortNumConvert(devNum,tableType,entryIndex, &portGroupId , &entryIndex,&tableType);

    return prvCpssPxPortGroupReadTableEntry(devNum,portGroupId,tableType,entryIndex,entryValuePtr);

}

/**
* @internal prvCpssPxWriteTableEntry function
* @endinternal
*
* @brief   Write a whole entry to the table.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] entryValuePtr            - (pointer to) the data that will be written to the table
*                                      may be NULL in the case of indirect table.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of entryValuePtr == NULL and indirect table the function
*       just send write entry command to device. And entry is taken from the
*       data registers of a indirect table.
*
*/
GT_STATUS prvCpssPxWriteTableEntry
(
    IN GT_SW_DEV_NUM           devNum,
    IN CPSS_PX_TABLE_ENT       tableType,
    IN GT_U32                  entryIndex,
    IN GT_U32                  *entryValuePtr
)
{
    GT_U32      portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;/* support multi-port-groups device for table accesses with index = 'per port' */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    /* convert if needed the entry index , get specific portGroupId (or all port groups) */
    indexAsPortNumConvert(devNum,tableType,entryIndex, &portGroupId , &entryIndex,&tableType);

    return prvCpssPxPortGroupWriteTableEntry(devNum,portGroupId,tableType,entryIndex,entryValuePtr);
}

/**
* @internal prvCpssPxWriteTableEntryField function
* @endinternal
*
* @brief   Write a field to the table.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] fieldWordNum             - field word number
*                                      use PRV_CPSS_PX_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
*                                      if need global offset in the field of fieldOffset
*                                      use PRV_CPSS_PX_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      if need 'field name' in the field of fieldOffset
* @param[in] fieldOffset              - field offset
* @param[in] fieldLength              - field length
*                                      when fieldWordNum == PRV_CPSS_PX_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      the field 'fieldLength' is used ONLY if  it's value != 0
*                                      and then it means 'Mumber of consecutive fields' !!!
* @param[in] fieldValue               - the data write to the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxWriteTableEntryField
(
    IN GT_SW_DEV_NUM   devNum,
    IN CPSS_PX_TABLE_ENT tableType,
    IN GT_U32                 entryIndex,
    IN GT_U32                 fieldWordNum,
    IN GT_U32                 fieldOffset,
    IN GT_U32                 fieldLength,
    IN GT_U32                 fieldValue
)
{
    GT_U32      portGroupId;/* support multi-port-groups device for table accesses with index = 'per port' */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    /* convert if needed the entry index , get specific portGroupId (or all port groups) */
    indexAsPortNumConvert(devNum,tableType,entryIndex, &portGroupId , &entryIndex,&tableType);

    return prvCpssPxPortGroupWriteTableEntryField(devNum,portGroupId ,
                tableType,entryIndex,fieldWordNum,
                fieldOffset,fieldLength,fieldValue);

}

/**
* @internal prvCpssPxTableNumEntriesGet function
* @endinternal
*
* @brief   get the number of entries in a table
*         needed for debug purpose
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
*
* @param[out] numEntriesPtr            - (pointer to) number of entries in the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxTableNumEntriesGet
(
    IN GT_SW_DEV_NUM     devNum,
    IN CPSS_PX_TABLE_ENT  tableType,
    OUT GT_U32                  *numEntriesPtr
)
{
    PRV_CPSS_PX_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(numEntriesPtr);

    /* validity check */
    CPSS_DATA_CHECK_MAX_MAC(tableType, PRV_CPSS_PX_TABLE_SIZE_GET_MAC(devNum));

    tableInfoPtr = PRV_PX_TABLE_INFO_PTR_GET_MAC(devNum,tableType);

    *numEntriesPtr = tableInfoPtr->directAccessInfo.maxNumOfEntries;

    return GT_OK;
}



/**
* @internal prvCpssPxTablesAccessInit function
* @endinternal
*
* @brief   Initializes all structures for table access.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*/
GT_STATUS  prvCpssPxTablesAccessInit
(
    IN GT_SW_DEV_NUM    devNum
)
{
    GT_STATUS                   rc;
    PRV_CPSS_PX_TABLES_INFO_STC *accessTableInfoPtr;/*pointer to tables info*/
    GT_U32                      accessTableInfoSize;/*tables info size */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    accessTableInfoPtr = pipeTablesInfo__final;
    accessTableInfoSize = NUM_ELEMENTS_IN_ARR_MAC(pipeTablesInfo__final);

    PRV_CPSS_PX_PP_MAC(devNum)->accessTableInfoPtr = accessTableInfoPtr;
    PRV_CPSS_PX_PP_MAC(devNum)->accessTableInfoSize = accessTableInfoSize;

    if(pipeTablesInfo__final_ready == GT_TRUE)
    {
        /* no more to do */
        return GT_OK;
    }

    rc = initTableInfoForDevice(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    pipeTablesInfo__final_ready = GT_TRUE;


    return GT_OK;
}

/**
* @internal prvCpssPxPortGroupWriteTableEntryFieldList function
* @endinternal
*
* @brief   Write a list of fields to the table.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - HW table Id
* @param[in] entryIndex               - entry Index
* @param[in] entryMemoBufArr[]        - the work memory for read, update and write the entry
* @param[in] fieldsAmount             - amount of updated fields in the entry
* @param[in] fieldOffsetArr[]         - (array) the offset of the field in bits
*                                      from the entry origin
* @param[in] fieldLengthArr[]         - (array) the length of the field in bits,
*                                      0 - means to skip this subfield
* @param[in] fieldValueArr[]          - (array) the value of the field
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssPxPortGroupWriteTableEntryFieldList
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32                                 portGroupId,
    IN CPSS_PX_TABLE_ENT                tableType,
    IN GT_U32                                 entryIndex,
    IN GT_U32                                 entryMemoBufArr[],
    IN GT_U32                                 fieldsAmount,
    IN GT_U32                                 fieldOffsetArr[],
    IN GT_U32                                 fieldLengthArr[],
    IN GT_U32                                 fieldValueArr[]
)
{
    GT_STATUS                  res;           /* return code               */
    GT_U32                     idx;           /* field index               */
    GT_U32                     wordIdx;       /* word index                */
    GT_U32                     wordShift;     /* Shift in the word         */
    GT_U32                     len0;          /* length in the 1-th word   */
    GT_U32                     len1;          /* length in the 2-th word   */

    /* read / write tableEntryField per port group */
    if(portGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
    {
        /* update table in all port groups */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)
        {
            res = prvCpssPxPortGroupWriteTableEntryFieldList(
                devNum, portGroupId, tableType, entryIndex,
                entryMemoBufArr, fieldsAmount,
                fieldOffsetArr, fieldLengthArr, fieldValueArr);
            if( res != GT_OK)
            {
                return res;
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum, portGroupId)

        return GT_OK;
    }

    res = prvCpssPxPortGroupReadTableEntry(
        devNum, portGroupId,tableType, entryIndex, entryMemoBufArr);
    if (res != GT_OK)
    {
        return res;
    }

    for (idx = 0; (idx < fieldsAmount); idx++)
    {
        if (fieldLengthArr[idx] > 32)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "field at index[%d] is with length[%d] (supporting 0..32)",
                idx,fieldLengthArr[idx]);
        }

        if (fieldLengthArr[idx] == 0)
            continue;

        wordIdx   = fieldOffsetArr[idx] >> 5;
        wordShift = fieldOffsetArr[idx] & 0x1F;

        /* the length of the field or of it's intersection with the first word */
        len0 = ((wordShift + fieldLengthArr[idx]) <= 32)
            ? fieldLengthArr[idx] : (32 - wordShift);

        /* copy the field or it's intersection with the first word */
        U32_SET_FIELD_MASKED_MAC(
           entryMemoBufArr[wordIdx], wordShift, len0, fieldValueArr[idx]);

        /* copy the field intersection with the second word */
        if (len0 != fieldLengthArr[idx])
        {
            len1 = fieldLengthArr[idx] - len0;
            U32_SET_FIELD_MASKED_MAC(
               entryMemoBufArr[wordIdx + 1] ,0 /*offset*/, len1,
               (fieldValueArr[idx] >> len0));
        }
    }

    return  prvCpssPxPortGroupWriteTableEntry(
        devNum, portGroupId,tableType, entryIndex, entryMemoBufArr);
}

/**
* @internal prvCpssPxPortGroupReadTableEntryFieldList function
* @endinternal
*
* @brief   Read a list of fields from the table.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableId                  - HW table Id
* @param[in] entryIndex               - entry Index
* @param[in] entryMemoBufArr[]        - the work memory for read, update and write the entry
* @param[in] fieldsAmount             - amount of updated fields in the entry
* @param[in] fieldOffsetArr[]         - (array) the offset of the field in bits
*                                      from the entry origin
* @param[in] fieldLengthArr[]         - (array) the length of the field in bits,
*                                      0 - means to skip this subfield
*
* @param[out] fieldValueArr[]          - (array) the value of the field
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssPxPortGroupReadTableEntryFieldList
(
    IN  GT_SW_DEV_NUM    devNum,
    IN GT_U32                                 portGroupId,
    IN  CPSS_PX_TABLE_ENT                tableId,
    IN  GT_U32                                 entryIndex,
    IN  GT_U32                                 entryMemoBufArr[],
    IN  GT_U32                                 fieldsAmount,
    IN  GT_U32                                 fieldOffsetArr[],
    IN  GT_U32                                 fieldLengthArr[],
    OUT GT_U32                                 fieldValueArr[]
)
{
    GT_STATUS                  res;           /* return code               */
    GT_U32                     idx;           /* field index               */
    GT_U32                     wordIdx;       /* word index                */
    GT_U32                     wordShift;     /* Shift in the word         */
    GT_U32                     len0;          /* length in the 1-th word   */
    GT_U32                     len1;          /* length in the 2-th word   */

    res = prvCpssPxPortGroupReadTableEntry(
        devNum, portGroupId, tableId, entryIndex, entryMemoBufArr);
    if (res != GT_OK)
    {
        return res;
    }

    for (idx = 0; (idx < fieldsAmount); idx++)
    {
        if (fieldLengthArr[idx] > 32)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "field at index[%d] is with length[%d] (supporting 0..32)",
                idx,fieldLengthArr[idx]);
        }

        if (fieldLengthArr[idx] == 0)
        {
            fieldValueArr[idx] = 0;
            continue;
        }

        wordIdx   = fieldOffsetArr[idx] >> 5;
        wordShift = fieldOffsetArr[idx] & 0x1F;

        /* the length of the field or of it's intersection with the first word */
        len0 = ((wordShift + fieldLengthArr[idx]) <= 32)
            ? fieldLengthArr[idx] : (32 - wordShift);

        /* copy the field or it's intersection with the first word */
        fieldValueArr[idx] =
            U32_GET_FIELD_MAC(entryMemoBufArr[wordIdx], wordShift, len0);

        /* copy the field intersection with the second word */
        if (len0 != fieldLengthArr[idx])
        {
            len1 = fieldLengthArr[idx] - len0;
            fieldValueArr[idx] |=
                ((U32_GET_FIELD_MAC(
                    entryMemoBufArr[wordIdx + 1], 0/*offset*/, len1))
                 << len0);
        }
    }

    return  GT_OK;
}

/**
* @internal prvCpssPxTableEngineMultiEntriesInLineIndexAndGlobalBitConvert function
* @endinternal
*
* @brief   check if the table entry is actually implemented as
*         'several entries' in single 'line'
*         NOTE: if the table is not of type 'multi entries' in line , the function
*         not update the INOUT parameters
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in,out] entryIndexPtr            - (pointer to) entry index , as 'anaware' that there are
*                                      multi entries in single line.
* @param[in,out] fieldOffsetPtr           - (pointer to) offset of the 'field' in the entry ,
*                                      as 'anaware' that there are multi entries in single line.
* @param[in,out] entryIndexPtr            - (pointer to) LINE index , as 'ware' that there are
*                                      multi entries in single line.
* @param[in,out] fieldOffsetPtr           - (pointer to) offset of the 'field' in the LINE ,
*                                      as 'aware' that there are multi entries in single line.
*
* @param[out] numBitsPerEntryPtr       - (pointer to) the number of bits that the 'entry' is
*                                      using in the LINE.
*                                      NOTE: can be NULL
* @param[out] numEntriesPerLinePtr     - (pointer to) the number of entries in single LINE.
*                                      NOTE: can be NULL
*                                      NOTE: if value hold FRACTION_INDICATION_CNS meaning that the
*                                      number in the lower 31 bits is 'fraction' (1/x) and not (x)
*
* @retval GT_TRUE                  - convert done
* @retval GT_FALSE                 - no convert done
*/
GT_BOOL prvCpssPxTableEngineMultiEntriesInLineIndexAndGlobalBitConvert
(
    IN GT_SW_DEV_NUM    devNum,
    IN CPSS_PX_TABLE_ENT     tableType,
    INOUT GT_U32               *entryIndexPtr,
    INOUT GT_U32               *fieldOffsetPtr,
    OUT GT_U32                 *numBitsPerEntryPtr,
    OUT GT_U32                 *numEntriesPerLinePtr
)
{
    devNum                = devNum               ;
    tableType             = tableType            ;
    entryIndexPtr         = entryIndexPtr        ;
    fieldOffsetPtr        = fieldOffsetPtr       ;
    numBitsPerEntryPtr    = numBitsPerEntryPtr   ;
    numEntriesPerLinePtr  = numEntriesPerLinePtr ;

    /* no table currently requires the convert */

    return GT_FALSE;
}

/**
* @internal prvCpssPxPortGroupWriteTableEntry function
* @endinternal
*
* @brief   Write a whole entry to the table. - for specific portGroupId
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] entryValuePtr            - (pointer to) the data that will be written to the table
*                                      may be NULL in the case of indirect table.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of entryValuePtr == NULL and indirect table the function
*       just send write entry command to device. And entry is taken from the
*       data registers of a indirect table.
*
*/
GT_STATUS prvCpssPxPortGroupWriteTableEntry
(
    IN GT_SW_DEV_NUM     devNum,
    IN GT_U32                   portGroupId,
    IN CPSS_PX_TABLE_ENT  tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                  *entryValuePtr
)
{
    GT_U32  entryMemoBufArr[MAX_ENTRY_SIZE_CNS];
    PRV_CPSS_PX_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */
    GT_U32      address;  /* address to write the value */
    /* pointer to direct access table information */
    PRV_CPSS_PX_TABLES_INFO_DIRECT_STC *tablePtr;
    GT_STATUS   status = GT_OK;
    GT_U32  tmpPortGroupId = portGroupId;/* temporary port group id */
    GT_U32  originPortGroupId = portGroupId;
    GT_U32  orig_entryIndex; /* orig entry index as calculate in this function */
    GT_U32  *orig_entryValuePtr;/* original pointer from the caller */
    GT_U32  startBit;/* start bit in the case of multi entries in single LINE */
    GT_U32  numBitsPerEntry;/*the number of bits that the 'entry' is using in the LINE.*/
    GT_U32  numEntriesPerLine;/*the number of entries in single LINE.
                        NOTE: if value hold FRACTION_INDICATION_CNS meaning that the
                        number in the lower 31 bits is 'fraction' (1/x) and not (x)*/
    GT_U32  numLinesPerEntry = 1;/* the number of lines that the entry occupy.
                        NOTE: value used only if MORE than 1 */
    GT_U32  lineIndex;/* line index when iteration over numLinesPerEntry */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if(portGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS ||
       (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) == 0))
    {
        /* this needed for direct calls to prvCpssPxPortGroupReadTableEntry with
           portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS , and still need convert
           of the entryIndex.
           for example :
           CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E
        */

        /* convert if needed the entry index , get specific portGroupId (or all port groups) */
        indexAsPortNumConvert(devNum,tableType,entryIndex, &tmpPortGroupId , &entryIndex,&tableType);
    }

    /* validity check */
    CPSS_DATA_CHECK_MAX_MAC(tableType, PRV_CPSS_PX_TABLE_SIZE_GET_MAC(devNum));

    tableInfoPtr = PRV_PX_TABLE_INFO_PTR_GET_MAC(devNum,tableType);

    orig_entryIndex = entryIndex;
    orig_entryValuePtr = entryValuePtr;
    startBit = 0;
    numBitsPerEntry = 0;/*dont care*/
    numEntriesPerLine = 0;/*dont care*/
    /* check if the table entry is actually implemented as
       'several entries' in single 'line' */
    if(GT_TRUE == prvCpssPxTableEngineMultiEntriesInLineIndexAndGlobalBitConvert(devNum,tableType,
                INOUT &entryIndex,INOUT &startBit ,
                OUT &numBitsPerEntry , OUT &numEntriesPerLine))
    {
        if(numEntriesPerLine & FRACTION_HALF_TABLE_INDICATION_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,"FRACTION_HALF_TABLE_INDICATION_CNS");
        }

        if(numEntriesPerLine & FRACTION_INDICATION_CNS)
        {
            /* indication that the entry uses multiple lines */
            numLinesPerEntry = numEntriesPerLine - FRACTION_INDICATION_CNS;
        }

        if(entryIndex >= tableInfoPtr->directAccessInfo.maxNumOfEntries)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "tableType[%d] index[%d] is out of range [0..%d]",
                tableType,
                entryIndex,
                tableInfoPtr->directAccessInfo.maxNumOfEntries-1);
        }

        CHECK_NEEDED_HW_ENTRY_SIZE_MAC((tableInfoPtr->directAccessInfo.entryWidthInWords * numLinesPerEntry),tableType);

        /* Read the a whole entry from table by direct access method */
        tablePtr = &tableInfoPtr->directAccessInfo;

        if(tablePtr->baseAddress == SPECIAL_ADDRESS_NOT_EXISTS_E)
        {
            /* the table not supported */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "tableType[%d] baseAddress was not set in the DB",tableType);
        }

        if(numLinesPerEntry == 1)
        {
            for(lineIndex = 0 ; lineIndex < numLinesPerEntry ; lineIndex++ , entryIndex++)
            {
                address = tablePtr->baseAddress + entryIndex * tablePtr->alignmentWidthInBytes;

                status  = prvCpssHwPpPortGroupReadRam (CAST_SW_DEVNUM(devNum), portGroupId, address,
                    tableInfoPtr->directAccessInfo.entryWidthInWords, &entryMemoBufArr[0]);
                if(status != GT_OK)
                {
                   return status;
                }
                /*
                   point the function to use the 'local' buffer that should support the
                   multi entries in single LINE , because the caller is not aware to
                   the actual size of the entry.
                */
                entryValuePtr = &entryMemoBufArr[0];

                /* we read full entry from HW , now update the relevant part with the
                   info from the caller  , so we can write it to the HW */
                copyBits(entryValuePtr,/* target */
                         startBit + (lineIndex * numBitsPerEntry),/* start bit in target */
                         orig_entryValuePtr,/*source*/
                         0,/*startBit In the source */
                         numBitsPerEntry);/* number of bits in the entry */

            }
            entryIndex -= numLinesPerEntry;/* restore entryIndex */
        }
        else
        {
            /* we write several lines ... not need to read any line */
            /* but we need to set the 'entry from caller' into 'lines info' */

            /*
               point the function to use the 'local' buffer that should support the
               multi entries in single LINE , because the caller is not aware to
               the actual size of the entry.
            */
            entryValuePtr = &entryMemoBufArr[0];

            for(lineIndex = 0 ; lineIndex < numLinesPerEntry ; lineIndex++)
            {
                /* we read full entry from HW , now update the relevant part with the
                   info from the caller  , so we can write it to the HW */
                copyBits(entryValuePtr,/* target */
                         lineIndex * tableInfoPtr->directAccessInfo.entryWidthInWords * 32,/* start bit in target */
                         orig_entryValuePtr,/*source*/
                         lineIndex*numBitsPerEntry,/*startBit In the source */
                         numBitsPerEntry);/* number of bits in the entry */
            }
        }
    }


    if(entryIndex >= tableInfoPtr->directAccessInfo.maxNumOfEntries)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "tableType[%d] index[%d] is out of range [0..%d]",
            tableType,
            entryIndex,
            tableInfoPtr->directAccessInfo.maxNumOfEntries-1);
    }


    /* Check if HW access required. */
    if(prvPxTableCpssHwAccessObj.hwAccessTableEntryWriteFunc != NULL)
    {
       status = prvPxTableCpssHwAccessObj.hwAccessTableEntryWriteFunc(
                                                  devNum, tmpPortGroupId,
                                                  tableType,
                                                  orig_entryIndex,
                                                  orig_entryValuePtr,
                                                  NULL,
                                                  originPortGroupId,
                                                  CPSS_DRV_HW_ACCESS_STAGE_PRE_E,
                                                  status);
       if (status != GT_OK)
       {
           return (status == GT_ABORTED) ? GT_OK : status;
       }
    }

    {
        /* there is no information to write */
        if(entryValuePtr == NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "tableType[%d] : there is no information to write",tableType);

        /* Write a whole entry to table by direct access method */
        tablePtr = &tableInfoPtr->directAccessInfo;

        if(tablePtr->baseAddress == SPECIAL_ADDRESS_NOT_EXISTS_E)
        {
            /* the table not supported */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "tableType[%d] baseAddress was not set in the DB",tableType);
        }

        for(lineIndex = 0 ; lineIndex < numLinesPerEntry ; lineIndex++ , entryIndex++)
        {
            /* update HW table shadow (if supported and requested) */
            status = prvCpssPxShadowLineWrite(devNum, tableType, entryIndex, entryValuePtr);
            if (status != GT_OK)
            {
                return status;
            }


            address = tablePtr->baseAddress + entryIndex * tablePtr->alignmentWidthInBytes;

            {
                status = prvCpssHwPpPortGroupWriteRam(CAST_SW_DEVNUM(devNum), portGroupId, address, tableInfoPtr->directAccessInfo.entryWidthInWords,
                                                                     entryValuePtr);
                if (status != GT_OK)
                {
                    return status;
                }
            }

            /* jump to next partial entry */
            entryValuePtr += tableInfoPtr->directAccessInfo.entryWidthInWords;
        }
        entryIndex -= numLinesPerEntry;/* restore entryIndex */

    }

    /* Check if HW access required. */
    if(prvPxTableCpssHwAccessObj.hwAccessTableEntryWriteFunc != NULL)
    {
       /* Don't care of returned status */
       prvPxTableCpssHwAccessObj.hwAccessTableEntryWriteFunc(
                                                  devNum, tmpPortGroupId,
                                                  tableType,
                                                  orig_entryIndex,
                                                  orig_entryValuePtr,
                                                  NULL,
                                                  originPortGroupId,
                                                  CPSS_DRV_HW_ACCESS_STAGE_POST_E,
                                                  status);
    }

    return status;
}


/**
* @internal prvCpssPxPortGroupReadTableEntry function
* @endinternal
*
* @brief   Read a whole entry from the table. - for specific portGroupId
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
*
* @param[out] entryValuePtr            - (pointer to) the data read from the table
*                                      may be NULL in the case of indirect table.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of entryValuePtr == NULL and indirect table the function
*       just send read entry command to device. And entry is stored in the
*       data registers of a indirect table
*
*/
GT_STATUS prvCpssPxPortGroupReadTableEntry
(
    IN GT_SW_DEV_NUM           devNum,
    IN GT_U32                  portGroupId,
    IN CPSS_PX_TABLE_ENT       tableType,
    IN GT_U32                  entryIndex,
    OUT GT_U32                 *entryValuePtr
)
{
    GT_U32  entryMemoBufArr[MAX_ENTRY_SIZE_CNS];
    PRV_CPSS_PX_TABLES_INFO_STC *tableInfoPtr; /* table info */
    GT_U32      address;  /* address the information is read from */
    /* pointer to direct access table information */
    PRV_CPSS_PX_TABLES_INFO_DIRECT_STC *tablePtr;
    GT_STATUS   status = GT_OK;
    GT_U32  tmpPortGroupId = portGroupId;/* temporary port group id */
    GT_U32  originPortGroupId = portGroupId;
    GT_U32  *orig_entryValuePtr;/* original pointer from the caller */
    GT_U32  startBit;/* start bit in the case of multi entries in single LINE */
    GT_U32  numBitsPerEntry;/*the number of bits that the 'entry' is using in the LINE.*/
    GT_BOOL tableOfMultiEntriesInLine;/* indication of multi entries in single LINE */
    GT_U32  numEntriesPerLine;/*the number of entries in single LINE.
                        NOTE: if value hold FRACTION_INDICATION_CNS meaning that the
                        number in the lower 31 bits is 'fraction' (1/x) and not (x)*/
    GT_U32  numLinesPerEntry = 1;/* the number of lines that the entry occupy.
                        NOTE: value used only if MORE than 1 */
    GT_U32  lineIndex;/* line index when iteration over numLinesPerEntry */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if(portGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS ||
       (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) == 0))
    {
        /* this needed for direct calls to prvCpssPxPortGroupReadTableEntry with
           portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS , and still need convert
           of the entryIndex.
           for example :
           CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E
        */

        /* convert if needed the entry index , get specific portGroupId (or all port groups) */
        indexAsPortNumConvert(devNum,tableType,entryIndex, &tmpPortGroupId , &entryIndex,&tableType);
    }

    /* validity check */
    CPSS_DATA_CHECK_MAX_MAC(tableType, PRV_CPSS_PX_TABLE_SIZE_GET_MAC(devNum));

    tableInfoPtr = PRV_PX_TABLE_INFO_PTR_GET_MAC(devNum,tableType);

    orig_entryValuePtr = entryValuePtr;
    startBit = 0;
    numBitsPerEntry = 0;/*dont care*/
    numEntriesPerLine = 0;/*dont care*/
    /* check if the table entry is actually implemented as
       'several entries' in single 'line' */
    if(GT_TRUE == prvCpssPxTableEngineMultiEntriesInLineIndexAndGlobalBitConvert(devNum,tableType,
                INOUT &entryIndex,INOUT &startBit ,
                OUT &numBitsPerEntry, OUT &numEntriesPerLine))
    {
        if(numEntriesPerLine & FRACTION_HALF_TABLE_INDICATION_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,"tableType[%d] : FRACTION_HALF_TABLE_INDICATION_CNS",tableType);
        }

        if(numEntriesPerLine & FRACTION_INDICATION_CNS)
        {
            /* indication that the entry uses multiple lines */
            numLinesPerEntry = numEntriesPerLine - FRACTION_INDICATION_CNS;
        }

        /*
           point the function to use the 'local' buffer that should support the
           multi entries in single LINE , because the caller is not aware to
           the actual size of the entry.
        */
        entryValuePtr = &entryMemoBufArr[0];

        tableOfMultiEntriesInLine = GT_TRUE;
    }
    else
    {
        tableOfMultiEntriesInLine = GT_FALSE;
    }

    if(entryIndex >= tableInfoPtr->directAccessInfo.maxNumOfEntries)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "tableType[%d] index[%d] is out of range [0..%d]",
            tableType,
            entryIndex,
            tableInfoPtr->directAccessInfo.maxNumOfEntries-1);
    }

    if(prvPxTableCpssHwAccessObj.hwAccessTableEntryReadFunc != NULL)
    {
       status = prvPxTableCpssHwAccessObj.hwAccessTableEntryReadFunc(
                                                  devNum, tmpPortGroupId, tableType,
                                                  entryIndex, entryValuePtr, originPortGroupId,
                                                  CPSS_DRV_HW_ACCESS_STAGE_PRE_E, status);
       if(status == GT_ABORTED)
        return GT_OK;

       if(status != GT_OK)
        return status;
    }

    for(lineIndex = 0 ; lineIndex < numLinesPerEntry ; lineIndex++ , entryIndex++)
    {
        {    /* Read a whole entry from table by direct access method */
            tablePtr = &tableInfoPtr->directAccessInfo;

            if(tablePtr->baseAddress == SPECIAL_ADDRESS_NOT_EXISTS_E)
            {
                /* the table not supported */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "tableType[%d] baseAddress was not set in the DB",tableType);
            }

            address = tablePtr->baseAddress + entryIndex * tablePtr->alignmentWidthInBytes;

            {
                status = prvCpssHwPpPortGroupReadRam(CAST_SW_DEVNUM(devNum), portGroupId,address, tableInfoPtr->directAccessInfo.entryWidthInWords,
                                                                     entryValuePtr);
            }
        }

        if(tableOfMultiEntriesInLine == GT_TRUE)
        {
            /* we are done reading the info from HW , but we need to prepare it for
               the 'caller' */
            copyBits(orig_entryValuePtr ,/* target */
                     numBitsPerEntry * lineIndex,/* start bit in target */
                     entryValuePtr,/*source*/
                     startBit,/*startBit In the source */
                     numBitsPerEntry);/* number of bits in the entry */
        }
    }
    entryIndex -= numLinesPerEntry;/* restore entryIndex */

    if(tableOfMultiEntriesInLine == GT_TRUE)
    {
        entryValuePtr = orig_entryValuePtr;
    }


    if(prvPxTableCpssHwAccessObj.hwAccessTableEntryReadFunc != NULL)
    {
       prvPxTableCpssHwAccessObj.hwAccessTableEntryReadFunc(
                                                  devNum, tmpPortGroupId, tableType,
                                                  entryIndex, entryValuePtr, originPortGroupId,
                                                  CPSS_DRV_HW_ACCESS_STAGE_POST_E, status);
    }

    return status;
}

/**
* @internal prvCpssPxWriteTableEntryMasked function
* @endinternal
*
* @brief   Either write a whole entry into HW table or update HW entry bits
*         specified by a mask. If Shadow DB table exists it will be updated too.
*         Works for specific portGroupId only.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*                                      portGroupId - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] entryValuePtr            - (pointer to) the data that will be written to the table
*                                      may be NULL in the case of indirect table.
* @param[in] entryMaskPtr             - (pointer to) a mask array.
*                                      Ignored if entryValuePtr is NULL.
*                                      If NULL -  the entry specified by entryValuePtr will be
*                                      written to HW table as is.
*                                      If not NULL  - only bits that are raised in the mask
*                                      will be updated in the HW entry.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of entryValuePtr == NULL and indirect table the function
*       just send write entry command to device. And entry is taken from the
*       data registers of a indirect table.
*
*/
GT_STATUS prvCpssPxWriteTableEntryMasked
(
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PX_TABLE_ENT        tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                  *entryValuePtr,
    IN GT_U32                  *entryMaskPtr
)
{
    GT_STATUS rc;

    GT_U32  entryMemoBufArr[MAX_ENTRY_SIZE_CNS];
    GT_U32  i;
    PRV_CPSS_PX_TABLES_INFO_STC * tableInfoPtr;

    tableInfoPtr = PRV_PX_TABLE_INFO_PTR_GET_MAC(devNum,tableType);

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    /*READ*/
    rc = prvCpssPxReadTableEntry(devNum, tableType, entryIndex, /*OUT*/&entryMemoBufArr[0]);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssPxReadTableEntry()");
    }

    /*MODIFY*/
    for (i = 0 ; i < tableInfoPtr->directAccessInfo.entryWidthInWords; i++)
    {
        GT_U32 tmpValue;

        tmpValue = entryValuePtr[i] & entryMaskPtr[i];  /* clear all bits outside mask at value */

        entryMemoBufArr[i] &= (~entryMaskPtr[i]);                    /* clear all bits at read value */
        entryMemoBufArr[i] |= tmpValue;                              /* set all bits  from masked value */
    }

    /*WRITE*/
    rc = prvCpssPxWriteTableEntry(devNum, tableType, entryIndex, &entryMemoBufArr[0]);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssPxWriteTableEntry()");
    }

    return GT_OK;
}


/**
* @internal prvCpssPxReadTableEntryField function
* @endinternal
*
* @brief   Read a field from the table. - for specific portGroupId
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*                                      portGroupId - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] fieldWordNum             - field word number
*                                      use PRV_CPSS_PX_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
*                                      if need global offset in the field of fieldOffset
*                                      use PRV_CPSS_PX_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      if need 'field name' in the field of fieldOffset
*                                      use PRV_CPSS_PX_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      if need 'field name' in the field of fieldOffset
* @param[in] fieldOffset              - field offset
* @param[in] fieldLength              - field length
*                                      when fieldWordNum == PRV_CPSS_PX_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      the field 'fieldLength' is used ONLY if  it's value != 0
*                                      and then it means 'Mumber of consecutive fields' !!!
*
* @param[out] fieldValuePtr            - (pointer to) the data read from the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxReadTableEntryField
(
    IN GT_SW_DEV_NUM          devNum,
    IN CPSS_PX_TABLE_ENT      tableType,
    IN GT_U32                 entryIndex,
    IN GT_U32                 fieldWordNum,
    IN GT_U32                 fieldOffset,
    IN GT_U32                 fieldLength,
    OUT GT_U32                *fieldValuePtr
)
{
    GT_U32  entryMemoBufArr[MAX_ENTRY_SIZE_CNS];
    PRV_CPSS_PX_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */
    GT_U32 address; /* address to read from */
    /* pointer to direct table info */
    PRV_CPSS_PX_TABLES_INFO_DIRECT_STC *tablePtr;
    GT_STATUS status = GT_OK;
    GT_U32  portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32  tmpPortGroupId = portGroupId;/* temporary port group id */
    GT_U32  originPortGroupId = portGroupId;
    GT_U32  subFieldOffset;/* the sub field offset */
    GT_U32  subFieldLength;/* the sub field length */
    PRV_CPSS_PX_PP_TABLE_FORMAT_INFO_STC *  fieldsInfoPtr;
    GT_U32  orig_entryIndex; /* orig entry index as calculate in this function */
    GT_U32  orig_fieldOffset;/* orig entry field Offset as calculate in this function */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if(portGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS ||
       (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) == 0))
    {
        /* this needed for direct calls to prvCpssPxPortGroupReadTableEntry with
           portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS , and still need convert
           of the entryIndex.
           for example :
           CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E
        */

        /* convert if needed the entry index , get specific portGroupId (or all port groups) */
        indexAsPortNumConvert(devNum,tableType,entryIndex, &tmpPortGroupId , &entryIndex,&tableType);
    }

    /* validity check */
    CPSS_DATA_CHECK_MAX_MAC(tableType, PRV_CPSS_PX_TABLE_SIZE_GET_MAC(devNum));

    tableInfoPtr = PRV_PX_TABLE_INFO_PTR_GET_MAC(devNum,tableType);

    if(fieldWordNum == PRV_CPSS_PX_TABLE_WORD_INDICATE_FIELD_NAME_CNS)
    {
        fieldsInfoPtr = tableFieldsFormatInfoGet(devNum, tableType);
        if(fieldsInfoPtr == NULL)
        {
            /* the table is not bound to fields format */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        if (fieldsInfoPtr->patternNum == 0)
        {
            /* convert field info to global bit position */
            status = convertFieldInfoToGlobalBit(devNum,tableType,&fieldWordNum,&fieldOffset,&fieldLength,0);
            if(status != GT_OK)
            {
                return status;
            }
        }
        else    /* pattern entry */
        {
            /* convert index and pattern field info to new index and global bit position */
            status = convertPatternFieldInfoToGlobalBit(devNum, tableType, &entryIndex, &fieldWordNum, &fieldOffset, &fieldLength, 0);
            if(status != GT_OK)
            {
                return status;
            }
        }
    }
    else if(fieldWordNum == PRV_CPSS_PX_TABLE_WORD_INDICATE_SUB_FIELD_IN_FIELD_NAME_CNS)
    {
        /* reverse the operation of PRV_CPSS_PX_TABLE_SUB_FIELD_OFFSET_AND_LENGTH_MAC */
        subFieldOffset = fieldLength & 0xFFFF;
        subFieldLength = fieldLength >> 16;

        /* convert field info to global bit position */
        status = convertSubFieldInfoToGlobalBit(devNum,tableType,subFieldOffset,subFieldLength,&fieldWordNum,&fieldOffset,&fieldLength,0);
        if(status != GT_OK)
        {
            return status;
        }
    }

    orig_entryIndex  = entryIndex;
    orig_fieldOffset = fieldOffset;

    if(fieldWordNum == PRV_CPSS_PX_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS)
    {
        /* check if the table entry is actually implemented as
           'several entries' in single 'line' */
        prvCpssPxTableEngineMultiEntriesInLineIndexAndGlobalBitConvert(devNum,tableType,
                    INOUT &entryIndex,INOUT &fieldOffset,NULL,NULL);

        /* the field of fieldOffset is used as 'Global offset' in the entry */
        if((entryIndex >= tableInfoPtr->directAccessInfo.maxNumOfEntries)    ||
           ((fieldOffset + fieldLength) > (tableInfoPtr->directAccessInfo.entryWidthInWords * 32)) ||
           ((fieldLength == 0) || (fieldLength > 32)))
        {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        /* check if the write is to single word or more */
        if(/*(fieldLength > 1) && */OFFSET_TO_WORD_MAC(fieldOffset) != OFFSET_TO_WORD_MAC(fieldOffset + fieldLength))
        {
            /* writing to more than single word in the entry */

            CHECK_NEEDED_HW_ENTRY_SIZE_MAC(tableInfoPtr->directAccessInfo.entryWidthInWords,tableType);

            return prvCpssPxPortGroupReadTableEntryFieldList(devNum,portGroupId ,
                tableType,orig_entryIndex,entryMemoBufArr,1,
                &orig_fieldOffset,&fieldLength,fieldValuePtr);
        }

        fieldWordNum = OFFSET_TO_WORD_MAC(fieldOffset);
        fieldOffset  = OFFSET_TO_BIT_MAC(fieldOffset);
    }
    else
    if((entryIndex >= tableInfoPtr->directAccessInfo.maxNumOfEntries)    ||
       ((fieldOffset + fieldLength) > 32)               ||
       (fieldLength == 0)                               ||
       (fieldWordNum >= tableInfoPtr->directAccessInfo.entryWidthInWords))
    {
        CPSS_DATA_CHECK_MAX_MAC(entryIndex,tableInfoPtr->directAccessInfo.maxNumOfEntries);
        CPSS_DATA_CHECK_MAX_MAC((fieldOffset + fieldLength),33);
        CPSS_DATA_CHECK_MAX_MAC(fieldWordNum,tableInfoPtr->directAccessInfo.entryWidthInWords);
        if(fieldLength == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "fieldLength must not be ZERO");
        }
    }


    /* Check if HW access required. */
    if(prvPxTableCpssHwAccessObj.hwAccessTableEntryFieldReadFunc != NULL)
    {
       status = prvPxTableCpssHwAccessObj.hwAccessTableEntryFieldReadFunc(
                                                  devNum, tmpPortGroupId, tableType,
                                                  entryIndex, fieldWordNum, fieldOffset,
                                                  fieldLength,fieldValuePtr, originPortGroupId,
                                                  CPSS_DRV_HW_ACCESS_STAGE_PRE_E, status);
       if(status == GT_ABORTED)
        return GT_OK;

       if(status != GT_OK)
        return status;
    }

    {
        /*Read a field of entry from table by direct access method. */
        tablePtr = &tableInfoPtr->directAccessInfo;

        if(tablePtr->baseAddress == SPECIAL_ADDRESS_NOT_EXISTS_E)
        {
            /* the table not supported */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "tableType[%d] baseAddress was not set in the DB",tableType);
        }

        address = tablePtr->baseAddress + entryIndex * tablePtr->alignmentWidthInBytes +
                                                               fieldWordNum * _4_BYTES_CNS;

        status = prvCpssHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId, address,fieldOffset, fieldLength,
                                                                fieldValuePtr);

    }

    /* Check if HW access required. */
    if(prvPxTableCpssHwAccessObj.hwAccessTableEntryFieldReadFunc != NULL)
    {
       prvPxTableCpssHwAccessObj.hwAccessTableEntryFieldReadFunc(
                                                  devNum, tmpPortGroupId, tableType,
                                                  entryIndex, fieldWordNum, fieldOffset,
                                                  fieldLength,fieldValuePtr, originPortGroupId,
                                                  CPSS_DRV_HW_ACCESS_STAGE_POST_E, status);
    }

    return status;

}

/**
* @internal prvCpssPxPortGroupWriteTableEntryField function
* @endinternal
*
* @brief   Write a field to the table. - for specific portGroupId
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] fieldWordNum             - field word number
*                                      use PRV_CPSS_PX_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
*                                      if need global offset in the field of fieldOffset
*                                      use PRV_CPSS_PX_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      if need 'field name' in the field of fieldOffset
*                                      use PRV_CPSS_PX_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      if need 'field name' in the field of fieldOffset
* @param[in] fieldOffset              - field offset
* @param[in] fieldLength              - field length
*                                      when fieldWordNum == PRV_CPSS_PX_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      the field 'fieldLength' is used ONLY if  it's value != 0
*                                      and then it means 'Mumber of consecutive fields' !!!
* @param[in] fieldValue               - the data write to the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat A1 and above devices the data is updated only when the last
*       word in the entry was written.
*
*/
static GT_STATUS prvCpssPxPortGroupWriteTableEntryField
(
    IN GT_SW_DEV_NUM   devNum,
    IN GT_U32                 portGroupId,
    IN CPSS_PX_TABLE_ENT    tableType,
    IN GT_U32                 entryIndex,
    IN GT_U32                 fieldWordNum,
    IN GT_U32                 fieldOffset,
    IN GT_U32                 fieldLength,
    IN GT_U32                 fieldValue
)
{
    GT_U32  entryMemoBufArr[MAX_ENTRY_SIZE_CNS];
    GT_STATUS   status = GT_OK;  /* return code */
    PRV_CPSS_PX_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */
    GT_U32 address; /* address to write to */
    /* pointer to direct table info */
    PRV_CPSS_PX_TABLES_INFO_DIRECT_STC *tablePtr;
    GT_U32      entrySize;  /* table entry size in words */
    GT_U32  tmpPortGroupId = portGroupId;/* temporary port group id */
    GT_U32  originPortGroupId = portGroupId;
    GT_U32  contWriteTable = 1;
    GT_U32  subFieldOffset;/* the sub field offset */
    GT_U32  subFieldLength;/* the sub field length */
    PRV_CPSS_PX_PP_TABLE_FORMAT_INFO_STC *  fieldsInfoPtr;
    GT_U32  orig_entryIndex; /* orig entry index as calculate in this function */
    GT_U32  orig_fieldOffset;/* orig entry field Offset as calculate in this function */
    GT_U32  orig_fieldWordNum;/* orig entry field wordNum as calculate in this function */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if(portGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS ||
       (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum) == 0))
    {
        /* this needed for direct calls to prvCpssPxPortGroupReadTableEntry with
           portGroupId = CPSS_PORT_GROUP_UNAWARE_MODE_CNS , and still need convert
           of the entryIndex.
           for example :
           CPSS_PX_TABLE_TXQ_DQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E
        */

        /* convert if needed the entry index , get specific portGroupId (or all port groups) */
        indexAsPortNumConvert(devNum,tableType,entryIndex, &tmpPortGroupId , &entryIndex,&tableType);
    }

    /* validity check */
    CPSS_DATA_CHECK_MAX_MAC(tableType, PRV_CPSS_PX_TABLE_SIZE_GET_MAC(devNum));

    tableInfoPtr = PRV_PX_TABLE_INFO_PTR_GET_MAC(devNum,tableType);

    entrySize = tableInfoPtr->directAccessInfo.entryWidthInWords;

    if(fieldWordNum == PRV_CPSS_PX_TABLE_WORD_INDICATE_FIELD_NAME_CNS)
    {
        fieldsInfoPtr = tableFieldsFormatInfoGet(devNum, tableType);
        if(fieldsInfoPtr == NULL)
        {
            /* the table is not bound to fields format */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        if (fieldsInfoPtr->patternNum == 0)
        {
            /* convert field info to global bit position */
            status = convertFieldInfoToGlobalBit(devNum,tableType,&fieldWordNum,&fieldOffset,&fieldLength,fieldValue);
            if(status != GT_OK)
            {
                return status;
            }
        }
        else    /* pattern entry */
        {
            /* convert index and pattern field info to new index and global bit position */
            status = convertPatternFieldInfoToGlobalBit(devNum, tableType, &entryIndex, &fieldWordNum, &fieldOffset, &fieldLength, fieldValue);
            if(status != GT_OK)
            {
                return status;
            }
        }
    }
    else if(fieldWordNum == PRV_CPSS_PX_TABLE_WORD_INDICATE_SUB_FIELD_IN_FIELD_NAME_CNS)
    {
        /* reverse the operation of PRV_CPSS_PX_TABLE_SUB_FIELD_OFFSET_AND_LENGTH_MAC */
        subFieldOffset = fieldLength & 0xFFFF;
        subFieldLength = fieldLength >> 16;

        /* convert field info to global bit position */
        status = convertSubFieldInfoToGlobalBit(devNum,tableType,subFieldOffset,subFieldLength,&fieldWordNum,&fieldOffset,&fieldLength,fieldValue);
        if(status != GT_OK)
        {
            return status;
        }
    }

    orig_entryIndex  = entryIndex;
    orig_fieldOffset = fieldOffset;
    orig_fieldWordNum = fieldWordNum;

    if(fieldWordNum == PRV_CPSS_PX_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS)
    {
        /* check if the table entry is actually implemented as
           'several entries' in single 'line' */
        prvCpssPxTableEngineMultiEntriesInLineIndexAndGlobalBitConvert(devNum,tableType,
                    INOUT &entryIndex,INOUT &fieldOffset,NULL,NULL);


        /* the field of fieldOffset is used as 'Global offset' in the entry */
        if((entryIndex >= tableInfoPtr->directAccessInfo.maxNumOfEntries)    ||
           ((fieldOffset + fieldLength) > (entrySize * 32)) ||
           ((fieldLength == 0) || (fieldLength > 32)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        /* check if the write is to single word or more */
        if(/*(fieldLength > 1) && */OFFSET_TO_WORD_MAC(fieldOffset) != OFFSET_TO_WORD_MAC(fieldOffset + fieldLength))
        {
            /* writing to more than single word in the entry */

            CHECK_NEEDED_HW_ENTRY_SIZE_MAC(entrySize,tableType);

            return prvCpssPxPortGroupWriteTableEntryFieldList(devNum,portGroupId ,
                tableType,orig_entryIndex,entryMemoBufArr,1,
                &orig_fieldOffset,&fieldLength,&fieldValue);
        }

        fieldWordNum = OFFSET_TO_WORD_MAC(fieldOffset);
        fieldOffset  = OFFSET_TO_BIT_MAC(fieldOffset);
    }
    else
    if((entryIndex >= tableInfoPtr->directAccessInfo.maxNumOfEntries)    ||
       ((fieldOffset + fieldLength) > 32)               ||
       (fieldLength == 0)                               ||
       (fieldWordNum >= entrySize))
    {
        CPSS_DATA_CHECK_MAX_MAC(entryIndex,tableInfoPtr->directAccessInfo.maxNumOfEntries);
        CPSS_DATA_CHECK_MAX_MAC((fieldOffset + fieldLength),33);
        CPSS_DATA_CHECK_MAX_MAC(fieldWordNum,entrySize);
        if(fieldLength == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "fieldLength must not be ZERO");
        }
    }

    /* Check if HW access required. */
    if(prvPxTableCpssHwAccessObj.hwAccessTableEntryFieldWriteFunc != NULL)
    {
       status = prvPxTableCpssHwAccessObj.hwAccessTableEntryFieldWriteFunc(
                                                  devNum, tmpPortGroupId, tableType,
                                                  entryIndex, fieldWordNum, fieldOffset,
                                                  fieldLength,fieldValue, originPortGroupId,
                                                  CPSS_DRV_HW_ACCESS_STAGE_PRE_E, status);
       if(status == GT_ABORTED)
        return GT_OK;

       if(status != GT_OK)
        return status;
    }

    {
        /* Write a field to table by direct access method. */
        tablePtr = &tableInfoPtr->directAccessInfo;

        if(tablePtr->baseAddress == SPECIAL_ADDRESS_NOT_EXISTS_E)
        {
            /* the table not supported */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "tableType[%d] baseAddress was not set in the DB",tableType);
        }

        /* read / write tableEntryField per port group */
        if(portGroupId == CPSS_PORT_GROUP_UNAWARE_MODE_CNS && PRV_CPSS_IS_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
        {
            GT_PORT_GROUPS_BMP  portGroupsBmp;/* bmp of port groups */
            if(GT_TRUE ==
                prvCpssDuplicatedMultiPortGroupsGet(CAST_SW_DEVNUM(devNum),portGroupId,tablePtr->baseAddress,
                    &portGroupsBmp,NULL,NULL))
            {
                /* when the entry involve 'per port info' and 'global info'
                 (like vlan entry) between hemispheres , need to update each port group
                 according to it's existing info and not according to 'representative'
                 port group */

                PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum, portGroupsBmp , portGroupId)
                {
                    status = prvCpssPxPortGroupWriteTableEntryField(
                        devNum, portGroupId, tableType,
                        orig_entryIndex, orig_fieldWordNum,
                        orig_fieldOffset, fieldLength, fieldValue);
                    if (status != GT_OK)
                    {
                        return status;
                    }
                }
                PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum, portGroupsBmp , portGroupId)

                contWriteTable = 0;
                status = GT_OK;
            }
        }
        if (contWriteTable)
        {
            /* In xCat and above devices the data is updated only when the last */
            /* word in the entry was written. */
            CHECK_NEEDED_HW_ENTRY_SIZE_MAC(entrySize,tableType);

            address = tablePtr->baseAddress + entryIndex * tablePtr->alignmentWidthInBytes;

            /* read whole entry */
            status = prvCpssHwPpPortGroupReadRam(CAST_SW_DEVNUM(devNum), portGroupId, address, entrySize,
                                                             entryMemoBufArr);
            if (status != GT_OK)
            {
                return status;
            }

            /* update field */
            U32_SET_FIELD_MAC(entryMemoBufArr[fieldWordNum], fieldOffset, fieldLength, fieldValue);

            /* update HW table shadow (if supported and requested) */
            status = prvCpssPxShadowLineWrite(devNum, tableType, entryIndex, entryMemoBufArr);
            if (status != GT_OK)
            {
               return status;
            }

            /* write whole entry */
            status = prvCpssHwPpPortGroupWriteRam(CAST_SW_DEVNUM(devNum), portGroupId, address, entrySize,
                                                             entryMemoBufArr);
            if (status != GT_OK)
            {
                return status;
            }
        } /*contWriteTable*/
    }

    if(prvPxTableCpssHwAccessObj.hwAccessTableEntryFieldWriteFunc != NULL)
    {
       prvPxTableCpssHwAccessObj.hwAccessTableEntryFieldWriteFunc(
                                                      devNum, tmpPortGroupId, tableType,
                                                      entryIndex, fieldWordNum, fieldOffset,
                                                      fieldLength,fieldValue, originPortGroupId,
                                                      CPSS_DRV_HW_ACCESS_STAGE_POST_E, status);

    }

    return status;
}

/**
* @internal prvCpssPxHwTblAddrStcInfoGet function
* @endinternal
*
* @brief   This function return the address of the tables struct of cheetah devices.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
*
* @param[out] tblAddrPtrPtr            - (pointer to) (pointer to) address of the tables struct.
* @param[out] sizePtr                  - (pointer to) size of tables struct.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong device type to operate
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxHwTblAddrStcInfoGet
(
    IN  GT_SW_DEV_NUM     devNum,
    OUT GT_U32    **tblAddrPtrPtr,
    OUT GT_U32    *sizePtr
)
{
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    *tblAddrPtrPtr = (GT_U32 *)PRV_CPSS_PX_DEV_TBLS_MAC(devNum);
    *sizePtr = PRV_CPSS_PX_TABLE_SIZE_GET_MAC(devNum);

    return GT_OK;
}

/**
* @internal prvCpssPxWriteTableMultiEntry function
* @endinternal
*
* @brief   Write number of entries to the table in consecutive indexes.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] startIndex               - index to the first table entry
* @param[in] numOfEntries             - the number of consecutive entries to write
* @param[in] entryValueArrayPtr       - (pointer to) the data that will be written to the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxWriteTableMultiEntry
(
    IN GT_SW_DEV_NUM   devNum,
    IN CPSS_PX_TABLE_ENT          tableType,
    IN GT_U32                           startIndex,
    IN GT_U32                           numOfEntries,
    IN GT_U32                           *entryValueArrayPtr
)
{
    GT_U32  ii;         /* loop iterator            */
    GT_U32  entrySize;  /* entry size in words      */
    GT_U32  rc;         /* return code              */
    PRV_CPSS_PX_TABLES_INFO_STC   *tableInfoPtr;

    CPSS_NULL_PTR_CHECK_MAC(entryValueArrayPtr);

    if( 0 == numOfEntries )
        return GT_OK;

    /* calculate entry size in words*/
    tableInfoPtr = PRV_PX_TABLE_INFO_PTR_GET_MAC(devNum,tableType);
    entrySize = tableInfoPtr->directAccessInfo.entryWidthInWords;

    for( ii = startIndex ; ii < startIndex + numOfEntries ; ii++ )
    {
        rc = prvCpssPxWriteTableEntry(devNum,
                                        tableType,
                                        ii,
                                        entryValueArrayPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        entryValueArrayPtr+=entrySize;
    }

    return GT_OK;
}

/**
* @internal prvCpssPxReadTableMultiEntry function
* @endinternal
*
* @brief   Read number of entries from the table in consecutive indexes.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] startIndex               - index to the first table entry
* @param[in] numOfEntries             - the number of consecutive entries to read
*
* @param[out] entryValueArrayPtr       - (pointer to) the data that will be read from the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxReadTableMultiEntry
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  CPSS_PX_TABLE_ENT          tableType,
    IN  GT_U32                           startIndex,
    IN  GT_U32                           numOfEntries,
    OUT GT_U32                           *entryValueArrayPtr
)
{
    GT_U32  ii;         /* loop iterator            */
    GT_U32  entrySize;  /* entry size in words      */
    GT_U32  rc;         /* return code              */
    PRV_CPSS_PX_TABLES_INFO_STC   *tableInfoPtr;

    CPSS_NULL_PTR_CHECK_MAC(entryValueArrayPtr);

    if( 0 == numOfEntries )
        return GT_OK;

    /* calculate entry size in words*/
    tableInfoPtr = PRV_PX_TABLE_INFO_PTR_GET_MAC(devNum,tableType);
    entrySize = tableInfoPtr->directAccessInfo.entryWidthInWords;

    for( ii = startIndex ; ii < startIndex + numOfEntries ; ii++ )
    {
        rc = prvCpssPxReadTableEntry(devNum,
                                       tableType,
                                       ii,
                                       entryValueArrayPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        entryValueArrayPtr+=entrySize;
    }

    return GT_OK;
}


/**
* @internal prvCpssPxTableBaseAddrGet function
* @endinternal
*
* @brief   This function retrieves base address of table.(for 'Direct access' tables)
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
*
* @param[out] errorPtr                 - (pointer to) indication that function did error. (can be NULL)
*
* @retval on success               - return the base address
* @retval on error                 - return address that ((address % 4) != 0)
*/
GT_U32  prvCpssPxTableBaseAddrGet
(
    IN GT_SW_DEV_NUM   devNum,
    IN CPSS_PX_TABLE_ENT          tableType,
    OUT GT_BOOL                         *errorPtr
)
{
    PRV_CPSS_PX_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */
    PRV_CPSS_PX_TABLES_INFO_DIRECT_STC *tablePtr;/* pointer to direct table info */
    GT_U32      address;

    tableInfoPtr = PRV_PX_TABLE_INFO_PTR_GET_MAC(devNum,tableType);
    tablePtr = &tableInfoPtr->directAccessInfo;

    address = tablePtr->baseAddress;
    if(errorPtr)
    {
        if(address == SPECIAL_ADDRESS_NOT_EXISTS_E)
        {
            *errorPtr = GT_TRUE;
        }
        else
        {
            *errorPtr = GT_FALSE;
        }
    }

    return address;
}



/**
* @internal prvCpssPxTableEngineToHwRatioGet function
* @endinternal
*
* @brief   return the ratio between the index that the 'table engine' gets from the
*         'cpss API' to the 'HW index' in the table.
*         NOTE: this to support 'multi entries' in line or fraction of entry in line.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
*
* @param[out] ratioPtr                 - (pointer to) number of entries for in single line or
*                                      number of lines  for in single entry.
* @param[out] isMultiplePtr            - (pointer to) indication that ratio is 'multiple' or 'fraction'.
*                                      GT_TRUE  - ratio is 'multiple' (number of entries for in single line)
*                                      GT_FALSE - ratio is 'fraction' (number of lines  for in single entry)
*                                       GT_OK
*/
GT_STATUS prvCpssPxTableEngineToHwRatioGet
(
    IN GT_SW_DEV_NUM    devNum,
    IN CPSS_PX_TABLE_ENT      tableType,
    OUT GT_U32                 *ratioPtr,
    OUT GT_BOOL                *isMultiplePtr
)
{
    GT_U32  numBitsPerEntry;
    GT_U32  numEntriesPerLine;
    GT_U32  entryIndex = 0;
    GT_U32  startBit = 0;

    if(GT_TRUE == prvCpssPxTableEngineMultiEntriesInLineIndexAndGlobalBitConvert(devNum,tableType,
                INOUT &entryIndex,INOUT &startBit ,
                OUT &numBitsPerEntry , OUT &numEntriesPerLine))
    {
        if(numEntriesPerLine & FRACTION_HALF_TABLE_INDICATION_CNS)
        {
            /* indication that the entry uses multiple lines but not consecutive */

            /* the DB of tables already knows that number of entries is updated*/
            *ratioPtr = numEntriesPerLine - FRACTION_HALF_TABLE_INDICATION_CNS;
            *isMultiplePtr = GT_FALSE;/* fraction */
            return GT_OK;
        }

        if(numEntriesPerLine & FRACTION_INDICATION_CNS)
        {
            /* indication that the entry uses multiple lines */
            *ratioPtr = numEntriesPerLine - FRACTION_INDICATION_CNS;
            *isMultiplePtr = GT_FALSE;/* fraction */
            return GT_OK;
        }

        *ratioPtr = numEntriesPerLine;
        *isMultiplePtr = GT_TRUE;/* Multiple */
        return GT_OK;
    }

    *ratioPtr = 1;
    *isMultiplePtr = GT_TRUE;/* Multiple */

    return GT_OK;

}




#define PX_TABLES_DEBUG

#ifdef PX_TABLES_DEBUG
GT_STATUS prvCpssPx_debugPrintTable
(
     IN GT_SW_DEV_NUM  devNum
)
{
    PRV_CPSS_PX_TABLES_INFO_STC *accessTableInfoPtr;/*pointer to tables info*/
    GT_U32                      accessTableInfoSize;/*tables info size */
    GT_U32 index;
    GT_U32 ii;
    char *caption[]       = {"Index", "tableName", "unitIndex", "baseAddress", "numOfBits", "maxNumOfEntries", "alignmentWidthInBytes","entryWidthInWords"};
    PRV_CPSS_PX_TABLES_INFO_STC *tableInfoPtr;/*pointer to tables info*/
    PRV_CPSS_PX_TABLES_INFO_DIRECT_STC *directTableInfoPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    cpssOsPrintf("prvCpssPx_debugPrintTable : started \n");

    cpssOsPrintf("%10s %40s %10s %10s %10s %10s %10s %10s\n",
              caption[0], caption[1], caption[2], caption[3], caption[4],
              caption[5], caption[6],caption[7]);

    accessTableInfoPtr  = PRV_CPSS_PX_PP_MAC(devNum)->accessTableInfoPtr ;
    accessTableInfoSize = PRV_CPSS_PX_PP_MAC(devNum)->accessTableInfoSize;

    tableInfoPtr = accessTableInfoPtr;

    index = 0;

    for(ii = 0 ; ii < accessTableInfoSize ; ii++,tableInfoPtr++)
    {
        directTableInfoPtr = &tableInfoPtr->directAccessInfo;

        if(directTableInfoPtr->baseAddress == SPECIAL_ADDRESS_NOT_EXISTS_E)
        {
            continue;
        }

        cpssOsPrintf("%10d  %40s %10d 0x%08x %10d %10d %10d %10d\n"
            ,index
            /*,tableInfoPtr->globalIndex*/
            ,tableInfoPtr->tableName
            ,tableInfoPtr->unitIndex

            ,directTableInfoPtr->baseAddress
            ,directTableInfoPtr->numOfBits
            ,directTableInfoPtr->maxNumOfEntries
            ,directTableInfoPtr->alignmentWidthInBytes
            ,directTableInfoPtr->entryWidthInWords
            );

        index++;
    }

    cpssOsPrintf("prvCpssPx_debugPrintTable : ended \n");

    return GT_OK;
}
#endif



