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
* @file prvCpssDxChExactMatchManager_debug.h
*
* @brief Exact Match manager support.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChExactMatchManager_debug_h
#define __prvCpssDxChExactMatchManager_debug_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/exactMatchManager/cpssDxChExactMatchManager.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrg.h>
#include <cpss/dxCh/dxChxGen/exactMatch/cpssDxChExactMatch.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>

#define EM_PRINT_d_FIELD_MAC(field)            \
    cpssOsPrintf("%d ",field)

#define EM_PRINT_x_FIELD_MAC(field)            \
    cpssOsPrintf("0x%x ",field)

#define EM_PRINT_3x_FIELD_MAC(field)           \
    cpssOsPrintf("0x%3.3x ",field)

#define EM_PRINT_s_FIELD_MAC(field)            \
    cpssOsPrintf("%s ",#field)

#define EM_PRINT_space_MAC                     \
    cpssOsPrintf(" ")

#define EM_PRINT_3space_MAC                    \
    cpssOsPrintf("   ")

#define EM_PRINT_index_FIELD_MAC(field)        \
    cpssOsPrintf("0x%5.5x ",field)

#define EM_PRINT_NA_PTR                        \
    cpssOsPrintf(" - NA - ")


#define EM_PRINT_SIZEOF_MAC(stc)               \
    cpssOsPrintf("sizeof[%s] = [%d] \n",    \
        #stc,                               \
        sizeof(stc))

#define EM_PRINT_BANKS_ENTRY_PARAMS(exactMatchManagerPtr, calcPtr)                          \
    rc = prvCpssDxChExactMatchManagerAllBanksParamsPrint(exactMatchManagerPtr, calcPtr);    \
    if(rc != GT_OK)                                                                         \
    {                                                                                       \
        return rc;                                                                          \
    }                                                                                       \
    cpssOsPrintf("\n")

#define EM_PRINT_KEY(entryPtr,keySize)                                  \
{                                                                       \
    GT_U32 n;                                                           \
    cpssOsPrintf("keySize:[%d]\n",keySize);                             \
    cpssOsPrintf("pattern:");                                           \
    for (n=0; n<keySize; n++)                                           \
    {                                                                   \
        cpssOsPrintf("[%d]",entryPtr->exactMatchEntry.key.pattern[n]);  \
    }                                                                   \
}

#define EM_PRINT_CURRENT_5BYTES_BANK_STAGE(entryPtr, bank, line, stage) \
    cpssOsPrintf("5BYTES key size: ");                                  \
    EM_PRINT_KEY(entryPtr,5);                                           \
    cpssOsPrintf(" Bank[%d], Line[%d] Stage[%d]\n", bank, line, stage)

#define EM_PRINT_CURRENT_19BYTES_BANK_STAGE(entryPtr, bank, line, stage)\
    cpssOsPrintf("19BYTES key size: ");                                 \
    EM_PRINT_KEY(entryPtr,19);                                          \
    cpssOsPrintf(" Bank[%d], Line[%d] Stage[%d]\n", bank, line, stage)

#define EM_PRINT_CURRENT_33BYTES_BANK_STAGE(entryPtr, bank, line, stage)\
    cpssOsPrintf("33BYTES key size: ");                                 \
    EM_PRINT_KEY(entryPtr,33);                                          \
    cpssOsPrintf(" Bank[%d], Line[%d] Stage[%d]\n", bank, line, stage)

#define EM_PRINT_CURRENT_47BYTES_BANK_STAGE(entryPtr, bank, line, stage)\
    cpssOsPrintf("47BYTES key size: ");                                 \
    EM_PRINT_KEY(entryPtr,47);                                          \
    cpssOsPrintf(" Bank[%d], Line[%d] Stage[%d]\n", bank, line, stage)

#define EM_PRINT_MAC_PAIRS(rehashStageId, bankId, counter, currentPtr, nextPtr)                                                      \
    cpssOsPrintf("Current stage [%d], bankId[%d], counter[%5.5d]: ", rehashStageId, bankId, counter);                             \
    PRINT_MAC(currentPtr);  cpssOsPrintf("-->");  PRINT_MAC(nextPtr);                                                             \
    cpssOsPrintf("\n")

#define EM_PRINT_DB_ENTRY(dbEntryPtr)  \
    cpssOsPrintf("    hwIndex                       %x\n", dbEntryPtr->hwIndex                     );     \
    cpssOsPrintf("    ageBinIndex                   %x\n", dbEntryPtr->ageBinIndex                 );     \
    cpssOsPrintf("    isValid_prevEntryPointer      %x\n", dbEntryPtr->isValid_prevEntryPointer    );     \
    cpssOsPrintf("    prevEntryPointer              %x\n", dbEntryPtr->prevEntryPo inter            );    \
    cpssOsPrintf("    isValid_nextEntryPointer      %x\n", dbEntryPtr->isValid_nextEntryPointer    );     \
    cpssOsPrintf("    nextEntryPointer_high         %x\n", dbEntryPtr->nextEntryPointer_high       );     \
    cpssOsPrintf("    nextEntryPointer_low          %x\n", dbEntryPtr->nextEntryPointer_low        );     \
    cpssOsPrintf("    isValid_age_prevEntryPointer  %x\n", dbEntryPtr->isValid_age_prevEntryPointer);     \
    cpssOsPrintf("    age_prevEntryPointer          %x\n", dbEntryPtr->age_prevEntryPointer        );     \
    cpssOsPrintf("    isValid_age_nextEntryPointer  %x\n", dbEntryPtr->isValid_age_nextEntryPointer);     \
    cpssOsPrintf("    age_nextEntryPointer          %x\n", dbEntryPtr->age_nextEntryPointer        );     \
    cpssOsPrintf("    isUsedEntry                   %x\n", dbEntryPtr->isUsedEntry                 );     \
    cpssOsPrintf("    hwExactMatchEntryType         %x\n", dbEntryPtr->hwExactMatchEntryType       )

#define EM_PRINT_DB_ENTRY_CURRENT(dbEntryPtr)  \
    if (dbEntryPtr) {                                                                                   \
        cpssOsPrintf("---- Current dbEntryPtr -------------\n");                                        \
        EM_PRINT_DB_ENTRY(dbEntryPtr);                                                                     \
    } else {                                                                                            \
        cpssOsPrintf("---- Current dbEntryPtr == NULL  -------------\n");                               \
    }

#define EM_PRINT_DB_ENTRY_NEXT(dbEntryPtr)  \
    if (dbEntryPtr) {                                                                                   \
        cpssOsPrintf("---- Next dbEntryPtr -------------\n");                                           \
        EM_PRINT_DB_ENTRY(dbEntryPtr);                                                                     \
    } else {                                                                                            \
        cpssOsPrintf("---- Next dbEntryPtr == NULL  -------------\n");                                  \
    }


/* debug flag to allow to bypass the function prvCpssDxChExactMatchManagerHwBuildHwFormatFromDbEntry
   purpose : is to measure the time of operations without the convert of formats.
*/
extern GT_U32   prvCpssDxChExactMatchManagerDebugBypass_prvCpssDxChExactMatchManagerHwBuildHwFormatFromDbEntry /* default = 0 */;

/* debug function to print the Exact Match manager valid entries (from the DB) ...
   no HW access

   exactMatchManagerId - the manager Id
   withMetadata - indication to print per entry the 'metadata' in addition to 'entry format'
                  0 - print only 'entry format' without 'metadata'
                  1 - print      'entry format' and the 'metadata'
*/
GT_STATUS prvCpssDxChExactMatchManagerDebugPrintValidEntries
(
    IN GT_U32   exactMatchManagerId,
    IN GT_U32   withMetadata
);

/* debug function to print the Exact Match manager counters (from the DB) ...
   no HW access

   exactMatchManagerId - the manager Id
*/
GT_STATUS prvCpssDxChExactMatchManagerDebugPrintCounters
(
    IN GT_U32   exactMatchManagerId
);

/* debug function to print the Exact Match manager counters (from the DB) ...
   no HW access

   exactMatchManagerId - the manager Id
*/
GT_STATUS prvCpssDxChExactMatchManagerDebugPrintStatistics
(
    IN GT_U32   exactMatchManagerId
);

/* debug function to print parameters from calcInfo structure
   no HW access

   exactMatchManagerPtr - (pointer to) Exact Match manager
   calcInfoPtr          - (pointer to) calcInfo structure
*/
GT_STATUS prvCpssDxChExactMatchManagerAllBanksParamsPrint
(
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr
);


/* debug function to Back up Exact Match manager memory
   exactMatchanagerId - Exact Match Manager instance ID
*/
GT_STATUS prvCpssDxChExactMatchManagerBackUp
(
    IN GT_U32           exactMatchManagerId
);

/* debug function to Verify the current Exact Match Manager Instance entries,
   with the backed-up Exact Match Manager instance
   Support for HW corruption (SER) test cases

   exactMatchManagerId - Exact Match Manager instance ID
*/
GT_STATUS prvCpssDxChExactMatchManagerBackUpInstanceVerify
(
    IN GT_U32           exactMatchManagerId
);

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
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChExactMatchManager_debug_h */
