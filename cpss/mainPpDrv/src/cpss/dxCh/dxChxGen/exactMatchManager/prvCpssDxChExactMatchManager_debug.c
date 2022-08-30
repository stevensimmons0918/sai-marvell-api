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
* @file prvCpssDxChExactMatchManager_debug.c
*
* @brief Exact Match manager support - debug functions
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_utils.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_db.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_hw.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_debug.h>
#include <cpss/dxCh/dxChxGen/cuckoo/prvCpssDxChCuckoo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* global variables macros */

#define PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxchEmMgrDir.emMgrDbgSrc._var)

#define PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.dxchEmMgrDir.emMgrDbgSrc._var,_value)

GT_STATUS prvCpssDxChExactMatchManagerDebug_bindDedicatedMallocAndFree(
    IN CPSS_OS_MALLOC_FUNC            mallocFunc, /* NULL to 'restore' default usage */
    IN CPSS_OS_FREE_FUNC              freeFunc    /* NULL to 'restore' default usage */
)
{
    PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_SET(prvCpssDxChExactMatchManagerDebug_cpssOsMalloc,mallocFunc);
    PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_SET(prvCpssDxChExactMatchManagerDebug_cpssOsFree,freeFunc);
    return GT_OK;
}
/* macro to allocate memory and copy from the src */
#define EM_MEM_CALLOC_AND_CPY_MAC(_dst, _src, _sizeInBytes)                     \
   {                                                                            \
       if(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDebug_cpssOsMalloc))\
           _dst = PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDebug_cpssOsMalloc)(_sizeInBytes); \
       else                                                                     \
           _dst = cpssOsMalloc(_sizeInBytes);                                   \
       if(NULL != _dst)                                                         \
       {                                                                        \
           cpssOsMemCpy(_dst, _src, (_sizeInBytes));                            \
       }                                                                        \
   }

/* macro to free memory (associated with the manager) */
#define DEBUG_MEM_FREE_MAC(_pointer)                                    \
    if(_pointer)                                                        \
{                                                                       \
    if(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDebug_cpssOsFree))\
    {                                                                   \
        if (_pointer)                                                   \
            PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDebug_cpssOsFree)(_pointer);     \
        _pointer = NULL;                                                \
    }                                                                   \
    else                                                                \
    {                                                                   \
        FREE_PTR_MAC(_pointer);                                         \
    }                                                                   \
}

/* debug function to print parameters from calcInfo structure
   no HW access

   exactMatchManagerPtr - (pointer to) Exact Match manager
   calcInfoPtr          - (pointer to) calcInfo structure
*/
GT_STATUS prvCpssDxChExactMatchManagerAllBanksParamsPrint
(
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC    calcInfo;
    GT_STATUS                                                   rc;
    GT_U32                                                      bankId;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                     currentEntry;
    GT_U32                                                      sizeInBytes,n;

    calcInfo = *calcInfoPtr;

    for (bankId = 0; bankId < exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes; bankId += calcInfoPtr->bankStep)
    {
        rc = prvCpssDxChExactMatchManagerRehashBankEntryGet(exactMatchManagerPtr, bankId, &calcInfo, &currentEntry);
        if(rc != GT_OK)
        {
            return rc;
        }
        if (calcInfo.dbEntryPtr)
        {
            cpssOsPrintf("\n");
            cpssOsPrintf("hwIndex[%d]\n", calcInfo.dbEntryPtr->hwIndex);
            cpssOsPrintf("isUsedEntry should be 1 [%d]\n", calcInfo.dbEntryPtr->isUsedEntry);
            cpssOsPrintf("isFirst should be 1 [%d]\n", calcInfo.dbEntryPtr->isFirst);
            cpssOsPrintf("hwExactMatchEntryType: 0=TTI, 1=PCL, 2=EPCL [%d]\n", calcInfo.dbEntryPtr->hwExactMatchEntryType);
            cpssOsPrintf("exactMatchUserDefined[%d]\n", calcInfo.dbEntryPtr->exactMatchUserDefined);
            cpssOsPrintf("hwExactMatchExpandedActionIndex[%d]\n", calcInfo.dbEntryPtr->hwExactMatchExpandedActionIndex);
            cpssOsPrintf("hwExactMatchLookupNum:0=firstLookup, 1=secondLookup [%d]\n", calcInfo.dbEntryPtr->hwExactMatchLookupNum);
            switch (calcInfo.dbEntryPtr->hwExactMatchKeySize)
            {
            case 0:
                sizeInBytes=5;
                break;
            case 1:
                sizeInBytes=19;
                break;
            case 2:
                sizeInBytes=33;
                break;
            case 3:
                sizeInBytes=47;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(calcInfo.dbEntryPtr->hwExactMatchKeySize);
            }
            cpssOsPrintf("keySize:[%d]\n",sizeInBytes);
            cpssOsPrintf("pattern:");
            for (n=0; n<sizeInBytes; n++)
            {
                cpssOsPrintf("[%d]",calcInfo.dbEntryPtr->pattern[n]);
            }
            cpssOsPrintf("\n");
        }
        else
        {
            cpssOsPrintf("[---------------------------------------------------]\n");
        }

        cpssOsPrintf("bankId[%d]\n", bankId);
    }

    return GT_OK;
}


/* debug function to Back up Exact Match manager memory

   exactMatchanagerId - Exact Match Manager instance ID
*/
GT_STATUS prvCpssDxChExactMatchManagerBackUp
(
    IN GT_U32           exactMatchManagerId
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC      *exactMatchManagerPtr;
    GT_U32                                                      sizeInBytes;

    EM_MANAGER_ID_CHECK(exactMatchManagerId);
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /* Clean last backUp */
    if(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr))
    {
        DEBUG_MEM_FREE_MAC(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->agingBinInfo.ageBinListPtr);
        DEBUG_MEM_FREE_MAC(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->agingBinInfo.level1UsageBitmapPtr);
        DEBUG_MEM_FREE_MAC(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->indexArr);
        DEBUG_MEM_FREE_MAC(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->entryPoolPtr);
        DEBUG_MEM_FREE_MAC(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr));
    }

    /* BackUp to global memory */
    sizeInBytes = sizeof(PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC);
    EM_MEM_CALLOC_AND_CPY_MAC(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr), exactMatchManagerPtr, sizeInBytes);

    /* BackUp ageBinListPtr */
    sizeInBytes = exactMatchManagerPtr->agingBinInfo.totalAgeBinAllocated * sizeof(PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_BIN_STC);
    EM_MEM_CALLOC_AND_CPY_MAC(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->agingBinInfo.ageBinListPtr,
                           exactMatchManagerPtr->agingBinInfo.ageBinListPtr,
                           sizeInBytes);

    /* BackUp level1UsageBitmapPtr */
    sizeInBytes = sizeof(GT_U32) * (1 + (exactMatchManagerPtr->agingBinInfo.totalAgeBinAllocated >> 5));
    EM_MEM_CALLOC_AND_CPY_MAC(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->agingBinInfo.level1UsageBitmapPtr,
                           exactMatchManagerPtr->agingBinInfo.level1UsageBitmapPtr,
                           sizeInBytes);

    /* BackUp indexArr */
    sizeInBytes = exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes*sizeof(PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INDEX_STC);
    EM_MEM_CALLOC_AND_CPY_MAC(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->indexArr, exactMatchManagerPtr->indexArr, sizeInBytes);

    /* BackUp entryPoolPtr */
    sizeInBytes = exactMatchManagerPtr->capacityInfo.maxTotalEntries*sizeof(PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC);
    EM_MEM_CALLOC_AND_CPY_MAC(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->entryPoolPtr, exactMatchManagerPtr->entryPoolPtr, sizeInBytes);
    return GT_OK;
}

/* debug function to Verify the current Exact Match Manager Instance entries,
   with the backed-up Exact Match Manager instance
   Support for HW corruption (SER) test cases

   exactMatchManagerId - Exact Match Manager instance ID
*/
GT_STATUS prvCpssDxChExactMatchManagerBackUpInstanceVerify
(
    IN GT_U32           exactMatchManagerId
)
{
    GT_STATUS                                                   rc = GT_OK;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC      *exactMatchManagerPtr;
    GT_U32                                                      dbIndex, oldDbIndex;
    GT_U32                                                      hwIndex,step;
    GT_U32                                                      sizeInBytes;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT                            counterClientType;
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT                          counterKeySize;
    GT_U32                                                      bankNum=0;
    PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC                         *cuckooDbBankStPtr;
    GT_U32                                                      *cuckooDbSizeStPtr;
    PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_ENT                           i;
    GT_U32                                                      usedEntriesIndexes=0;
    GT_U32                                                      cuckooUsedEntriesIndexes=0;


    EM_MANAGER_ID_CHECK(exactMatchManagerId);
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    if(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)==NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Exact Match Manager SER: globalExactMatchManagerPtr==NULL\n");
    }

    /* Aging info(Usage bitmap and age-bin ID) verification */
    if(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->agingBinInfo.level2UsageBitmap != exactMatchManagerPtr->agingBinInfo.level2UsageBitmap)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Exact Match Manager SER: Level2 Usage Bitmap recovered wrongly\n");
    }

    sizeInBytes = sizeof(GT_U32) * (1 + (exactMatchManagerPtr->agingBinInfo.totalAgeBinAllocated >> 5));
    if(cpssOsMemCmp(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->agingBinInfo.level1UsageBitmapPtr,
                    exactMatchManagerPtr->agingBinInfo.level1UsageBitmapPtr,
                    sizeInBytes))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Exact Match Manager SER: Level1 Usage Bitmap recovered wrongly\n");
    }

    /* Index and entry Pool verification */
    for(hwIndex = 0; hwIndex < exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes; /*hwIndex updated according to entry keySize */)
    {
        /* Validate entry content in case entry is valid */
        if(exactMatchManagerPtr->indexArr[hwIndex].isValid_entryPointer)
        {
            dbIndex     = exactMatchManagerPtr->indexArr[hwIndex].entryPointer;
            oldDbIndex  = PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->indexArr[hwIndex].entryPointer;

            if (exactMatchManagerPtr->entryPoolPtr[dbIndex].isFirst !=
                PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->entryPoolPtr[oldDbIndex].isFirst)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Exact Match Manager SER not recovered for hwIndex[%d], isFirst is not the same\n", hwIndex);
            }
            if (exactMatchManagerPtr->entryPoolPtr[dbIndex].isFirst==GT_TRUE)
            {
                if (exactMatchManagerPtr->entryPoolPtr[dbIndex].hwExactMatchEntryType !=
                    PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->entryPoolPtr[oldDbIndex].hwExactMatchEntryType)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Exact Match Manager SER not recovered for hwIndex[%d],wrong hwExactMatchEntryType\n", hwIndex);
                }
                if (exactMatchManagerPtr->entryPoolPtr[dbIndex].hwExactMatchExpandedActionIndex !=
                    PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->entryPoolPtr[oldDbIndex].hwExactMatchExpandedActionIndex)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Exact Match Manager SER not recovered for hwIndex[%d],wrong hwExactMatchExpandedActionIndex\n", hwIndex);
                }
                if (exactMatchManagerPtr->entryPoolPtr[dbIndex].hwExactMatchKeySize !=
                    PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->entryPoolPtr[oldDbIndex].hwExactMatchKeySize)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Exact Match Manager SER not recovered for hwIndex[%d],wrong hwExactMatchKeySize\n", hwIndex);
                }
                if (exactMatchManagerPtr->entryPoolPtr[dbIndex].hwExactMatchLookupNum !=
                    PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->entryPoolPtr[oldDbIndex].hwExactMatchLookupNum)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Exact Match Manager SER not recovered for hwIndex[%d],wrong hwExactMatchLookupNum\n", hwIndex);
                }
                if (exactMatchManagerPtr->entryPoolPtr[dbIndex].exactMatchUserDefined !=
                    PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->entryPoolPtr[oldDbIndex].exactMatchUserDefined)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Exact Match Manager SER not recovered for hwIndex[%d],wrong exactMatchUserDefined\n", hwIndex);
                }

                if(cpssOsMemCmp(&exactMatchManagerPtr->entryPoolPtr[dbIndex].specificFormat,
                            &(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->entryPoolPtr[oldDbIndex].specificFormat),
                            sizeof(exactMatchManagerPtr->entryPoolPtr[dbIndex].specificFormat)))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Exact Match Manager HA not recovered for hwIndex[%d],wrong specificFormat\n", hwIndex);
                }

                /* jump to the next hwIndex to check */
                step = exactMatchManagerPtr->entryPoolPtr[dbIndex].hwExactMatchKeySize + 1;
                hwIndex = hwIndex+step;
            }
            else
            {
                hwIndex++;
            }
        }
        else
        {
            if(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->indexArr[hwIndex].isValid_entryPointer == GT_TRUE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Exact Match Manager SER not recovered for hwIndex[%d]\n", hwIndex);
            }
            else
            {
                /* else - In case of invalid entry dbIndex will not be valid to check */
                hwIndex++;
            }
        }
    }

    /* Verify actionType+keySize counters */
    counterClientType=CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;
    for(;counterClientType < CPSS_DXCH_EXACT_MATCH_CLIENT_LAST_E; counterClientType++)
    {
        counterKeySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        for (;counterKeySize < CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E; counterKeySize++)
        {
            if (exactMatchManagerPtr->typeCountersArr[counterClientType][counterKeySize].entryTypePopulation!=
                PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->typeCountersArr[counterClientType][counterKeySize].entryTypePopulation)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Exact Match Manager SER: Entry Type Counter recovered wrongly\n");
            }
             /* each entry hold diferrent number of indexes
                    5  Bytes key size = 1 index
                    19 Bytes key size = 2 index
                    33 Bytes key size = 3 index
                    47 Bytes key size = 4 index */
            usedEntriesIndexes += (exactMatchManagerPtr->typeCountersArr[counterClientType][counterKeySize].entryTypePopulation
                                   * (counterKeySize+1));
        }
    }

    /* Bank Counter verification */
    for(bankNum = 0; bankNum < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_NUM_BANKS_CNS; bankNum++)
    {
        if(exactMatchManagerPtr->bankInfoArr[bankNum].bankPopulation !=
                PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->bankInfoArr[bankNum].bankPopulation)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Exact Match Manager SER: Entry Bank Counter recovered wrongly\n");
        }
    }

    rc = prvCpssDxChCuckooDbStatisticsPtrGet(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId,&cuckooDbBankStPtr,&cuckooDbSizeStPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Exact Match Manager SER: fail to read Cuckoo statistics\n");
    }
    for (i=0; i<=PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_47B_E; i++)
        cuckooUsedEntriesIndexes += cuckooDbSizeStPtr[i];

    if (cuckooUsedEntriesIndexes != usedEntriesIndexes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Exact Match Manager SER: Entry cuckoo Counter recovered wrongly\n");
    }

    /* Release memory after after verification */
    if(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr))
    {
        DEBUG_MEM_FREE_MAC(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->agingBinInfo.ageBinListPtr);
        DEBUG_MEM_FREE_MAC(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->agingBinInfo.level1UsageBitmapPtr);
        DEBUG_MEM_FREE_MAC(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->indexArr);
        DEBUG_MEM_FREE_MAC(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr)->entryPoolPtr);
        DEBUG_MEM_FREE_MAC(PRV_SHARED_EMM_DIR_EMM_DBG_SRC_GLOBAL_VAR_GET(globalExactMatchManagerPtr));
    }

    return GT_OK;
}

/* debug function to simulate HW corruption

   exactMatchManagerId  - Exact Match Manager instance ID
   indexesArray         - Array of hwIndexs
   indexesCount         - total number of indexes present in indexesArray

*/
GT_STATUS prvCpssDxChExactMatchManagerInjectHwCorruption
(
    IN GT_U32           exactMatchManagerId,
    IN GT_U32           indexesArray[],
    IN GT_U32           indexesCount
)
{
    GT_STATUS                                                   rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC      *exactMatchManagerPtr;
    GT_U8                                                       devNum = 0, firstDevNum = 0;
    GT_PORT_GROUPS_BMP                                          portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32                                                      portGroupId;/*the port group Id - support multi-port-groups device */
    GT_U32                                                      indexNum;
    GT_U32                                                      hwData[PRV_CPSS_DXCH_EXACT_MATCH_ENTRY_BANK_SIZE_IN_WORDS_CNS];/* data for one bank - 115 bits */

    EM_MANAGER_ID_CHECK(exactMatchManagerId);
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr, GT_TRUE, &firstDevNum, &portGroupsBmp);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(firstDevNum, portGroupsBmp, portGroupId);

    for(indexNum=0; indexNum<indexesCount; indexNum++)
    {
        EM_LOCK_DEV_NUM(firstDevNum);

        PRV_CPSS_DXCH_EXACT_MATCH_INDEX_CHECK_MAC(firstDevNum,indexesArray[indexNum]);

        /* Read single bank of specific index */
        rc = prvCpssDxChPortGroupReadTableEntry(firstDevNum,
                                                portGroupId,
                                                CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,
                                                indexesArray[indexNum],
                                                &hwData[0]);
        EM_UNLOCK_DEV_NUM(firstDevNum);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Toggle 5th bit of first byte in HW Entry */
        hwData[0] ^= (1 << 5);

        /* Write the Corrupted entry to all devices */
        rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr, GT_TRUE, &devNum, &portGroupsBmp);
        if (rc!=GT_OK)
        {
            return rc;
        }
        while (rc==GT_OK)
        {
            portGroupId=0;

            PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_EM_E);

            EM_LOCK_DEV_NUM(devNum);
            /* loop on all active port groups in the bmp */
            PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)
            {

                    rc = prvCpssDxChPortGroupWriteTableEntry(devNum,
                            portGroupId,
                            CPSS_DXCH_SIP6_TABLE_EXACT_MATCH_E,
                            indexesArray[indexNum],
                            &hwData[0]);
                if(rc != GT_OK)
                {
                    EM_UNLOCK_DEV_NUM(devNum);
                    return rc;
                }
            }
            PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

            EM_UNLOCK_DEV_NUM(devNum);

            rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr, GT_FALSE, &devNum, &portGroupsBmp);
        }
    }
    return GT_OK;
}

