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
* @file prvCpssDxChBrgFdbManager_debug.h
*
* @brief FDB manager support.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChBrgFdbManager_debug_h
#define __prvCpssDxChBrgFdbManager_debug_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/bridgeFdbManager/cpssDxChBrgFdbManager.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrg.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdbRouting.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>

#define PRINT_d_FIELD_MAC(field)            \
    cpssOsPrintf("%d ",field)

#define PRINT_x_FIELD_MAC(field)            \
    cpssOsPrintf("0x%x ",field)

#define PRINT_3x_FIELD_MAC(field)           \
    cpssOsPrintf("0x%3.3x ",field)

#define PRINT_s_FIELD_MAC(field)            \
    cpssOsPrintf("%s ",#field)

#define PRINT_space_MAC                     \
    cpssOsPrintf(" ")

#define PRINT_3space_MAC                    \
    cpssOsPrintf("   ")

#define PRINT_index_FIELD_MAC(field)        \
    cpssOsPrintf("0x%5.5x ",field)

#define PRINT_NA_PTR                        \
    cpssOsPrintf(" - NA - ")


#define PRINT_SIZEOF_MAC(stc)               \
    cpssOsPrintf("sizeof[%s] = [%d] \n",    \
        #stc,                               \
        sizeof(stc))

#define PRINT_IPV6_ADDRESS_MAC(ipv6Addr)    \
{                                           \
    GT_U32 n;                               \
    for (n=0; n<16; n++)                    \
    {                                       \
        cpssOsPrintf("%2x:",ipv6Addr[n]);   \
    }                                       \
}


#define PRINT_8s_FIELD_MAC(field)    \
    cpssOsPrintf("%8s", #field)

#define PRINT_8d_FIELD_MAC(field)    \
    cpssOsPrintf("%8d",field)

#define PRINT_IPV4_UC(entryPtr) \
cpssOsPrintf("[%3.3x.%3.3x.%3.3x.%3.3x] ",                                                                              \
             entryPtr->format.fdbEntryIpv4UcFormat.ipv4Addr.arIP[0],                                                    \
             entryPtr->format.fdbEntryIpv4UcFormat.ipv4Addr.arIP[1],                                                    \
             entryPtr->format.fdbEntryIpv4UcFormat.ipv4Addr.arIP[2],                                                    \
             entryPtr->format.fdbEntryIpv4UcFormat.ipv4Addr.arIP[3])

#define PRINT_IPV4_MC(entryPtr) \
cpssOsPrintf("[%3.3x.%3.3x.%3.3x.%3.3x.%3.3x.%3.3x.%3.3x.%3.3x] ",                                                      \
             entryPtr->format.fdbEntryIpv4McFormat.sipAddr[0],                                                          \
             entryPtr->format.fdbEntryIpv4McFormat.sipAddr[1],                                                          \
             entryPtr->format.fdbEntryIpv4McFormat.sipAddr[2],                                                          \
             entryPtr->format.fdbEntryIpv4McFormat.sipAddr[3],                                                          \
             entryPtr->format.fdbEntryIpv4McFormat.dipAddr[0],                                                          \
             entryPtr->format.fdbEntryIpv4McFormat.dipAddr[1],                                                          \
             entryPtr->format.fdbEntryIpv4McFormat.dipAddr[2],                                                          \
             entryPtr->format.fdbEntryIpv4McFormat.dipAddr[3])

#define PRINT_MAC(entryPtr) \
cpssOsPrintf("[%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x] ",                                                                  \
             entryPtr->format.fdbEntryMacAddrFormat.macAddr.arEther[0],                                                 \
             entryPtr->format.fdbEntryMacAddrFormat.macAddr.arEther[1],                                                 \
             entryPtr->format.fdbEntryMacAddrFormat.macAddr.arEther[2],                                                 \
             entryPtr->format.fdbEntryMacAddrFormat.macAddr.arEther[3],                                                 \
             entryPtr->format.fdbEntryMacAddrFormat.macAddr.arEther[4],                                                 \
             entryPtr->format.fdbEntryMacAddrFormat.macAddr.arEther[5])

#define PRINT_IPV6_UC_KEY_MAC(entryPtr) \
cpssOsPrintf("[%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d] ",      \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[0],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[1],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[2],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[3],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[4],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[5],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[6],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[7])

#define PRINT_IPV6_UC_MAC(entryPtr) \
cpssOsPrintf("[%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d] ",      \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[0],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[1],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[2],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[3],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[4],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[5],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[6],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[7],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[8],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[9],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[10],                                                    \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[11],                                                    \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[12],                                                    \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[13],                                                    \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[14],                                                    \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[15])

#define F_PRINT_IPV6_UC_MAC(hFile, entryPtr)                                                                            \
cpssOsFprintf(hFile, "[%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d.%3.3d] \n",     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[0],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[1],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[2],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[3],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[4],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[5],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[6],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[7],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[8],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[9],                                                     \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[10],                                                    \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[11],                                                    \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[12],                                                    \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[13],                                                    \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[14],                                                    \
            entryPtr->format.fdbEntryIpv6UcFormat.ipv6Addr.arIP[15])

#define PRINT_BANKS_ADDRESSES(calcPtr, bank) \
    rc = prvCpssDxChBrgFdbManagerAllBanksAddressesPrint(fdbManagerPtr, calcPtr, bank); \
    if(rc != GT_OK)                                                     \
    {                                                                   \
        return rc;                                                      \
    }                                                                   \
    cpssOsPrintf("\n")

#define PRINT_CURRENT_IPV4_UC_BANK_STAGE(entryPtr, bank, stage, hwIndex)    \
    cpssOsPrintf("IPv4 UC: ");                          \
    PRINT_IPV4_UC(entryPtr);                            \
    cpssOsPrintf("Bank[%d], Stage[%d], HW index[0x%x]\n", bank, stage, hwIndex)

#define PRINT_CURRENT_IPV4_MC_BANK_STAGE(entryPtr, bank, stage, hwIndex)    \
    cpssOsPrintf("IPv4 MC: ");                                  \
    PRINT_IPV4_MC(entryPtr);                                \
    cpssOsPrintf("Bank[%d], Stage[%d], HW index[0x%x]\n", bank, stage, hwIndex)

#define PRINT_CURRENT_MAC_BANK_STAGE(entryPtr, bank, stage, hwIndex)    \
    cpssOsPrintf("MAC: ");                          \
    PRINT_MAC(entryPtr);                            \
    cpssOsPrintf("Bank[%d], Stage[%d], HW index[0x%x]\n", bank, stage, hwIndex)

#define PRINT_MAC_PAIRS(rehashStageId, bankId, counter, currentPtr, nextPtr)                                                      \
    cpssOsPrintf("Current stage [%d], bankId[%d], counter[%5.5d]: ", rehashStageId, bankId, counter);                             \
    PRINT_MAC(currentPtr);  cpssOsPrintf("-->");  PRINT_MAC(nextPtr);                                                             \
    cpssOsPrintf("\n")

#define PRINT_CURRENT_IPV6_UC_BANK_STAGE(entryPtr, bank, stage, hwIndex)     \
    cpssOsPrintf("IPV6 UC: ");                                      \
    PRINT_IPV6_UC_MAC(entryPtr);                                            \
    cpssOsPrintf("Bank[%d], Stage[%d], HW index[0x%x]\n", bank, stage, hwIndex)


#define PRINT_DB_ENTRY(dbEntryPtr)  \
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
    cpssOsPrintf("    hwFdbEntryType                %x\n", dbEntryPtr->hwFdbEntryType              )

#define PRINT_DB_ENTRY_CURRENT(dbEntryPtr)  \
    if (dbEntryPtr) {                                                                                   \
        cpssOsPrintf("---- Current dbEntryPtr -------------\n");                                        \
        PRINT_DB_ENTRY(dbEntryPtr);                                                                     \
    } else {                                                                                            \
        cpssOsPrintf("---- Current dbEntryPtr == NULL  -------------\n");                               \
    }

#define PRINT_DB_ENTRY_NEXT(dbEntryPtr)  \
    if (dbEntryPtr) {                                                                                   \
        cpssOsPrintf("---- Next dbEntryPtr -------------\n");                                           \
        PRINT_DB_ENTRY(dbEntryPtr);                                                                     \
    } else {                                                                                            \
        cpssOsPrintf("---- Next dbEntryPtr == NULL  -------------\n");                                  \
    }



/* debug function to print the FDB manager valid entries (from the DB) ...
   no HW access

   fdbManagerId - the manager Id
   withMetadata - indication to print per entry the 'metadata' in addition to 'entry format'
                  0 - print only 'entry format' without 'metadata'
                  1 - print      'entry format' and the 'metadata'
*/
GT_STATUS prvCpssDxChFdbManagerDebugPrintValidEntries
(
    IN GT_U32   fdbManagerId,
    IN GT_U32   withMetadata
);

/* debug function to print the FDB manager valid MAC entries (from the DB) ...
   no HW access

   fdbManagerId - the manager Id
*/
GT_STATUS prvCpssDxChFdbManagerDebugPrintValidEntries_mac
(
    IN GT_U32   fdbManagerId
);

/* debug function to print the FDB manager valid IPV4UC entries (from the DB) ...
   no HW access

   fdbManagerId - the manager Id
*/
GT_STATUS prvCpssDxChFdbManagerDebugPrintValidEntries_ipv4Uc
(
    IN GT_U32   fdbManagerId
);

/* debug function to print the FDB manager valid IPV6UC entries (from the DB) ...
   no HW access

   fdbManagerId - the manager Id
*/
GT_STATUS prvCpssDxChFdbManagerDebugPrintValidEntries_ipv6Uc
(
    IN GT_U32   fdbManagerId
);

/* debug function to print the FDB manager counters (from the DB) ...
   no HW access

   fdbManagerId - the manager Id
*/
GT_STATUS prvCpssDxChFdbManagerDebugPrintCounters
(
    IN GT_U32   fdbManagerId
);

/* debug function to print the FDB manager counters (from the DB) ...
   no HW access

   fdbManagerId - the manager Id
*/
GT_STATUS prvCpssDxChFdbManagerDebugPrintStatistics
(
    IN GT_U32   fdbManagerId
);

/* debug function to print MAC addresses from calcInfo structure
   no HW access

   fdbManagerPtr - (pointer to) FDB manager
   calcInfoPtr   - (pointer to) calcInfo structure
   selectedBank  - selected bank
*/
GT_STATUS prvCpssDxChBrgFdbManagerAllBanksAddressesPrint
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC   *fdbManagerPtr,
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC *calcInfoPtr,
    IN  GT_U32                                           selectedBank
);

/* debug function to Back up FDB manager memory

   fdbManagerId - FDB Manager instance ID
*/
GT_STATUS prvCpssDxChBrgFdbManagerBackUp
(
    IN GT_U32           fdbManagerId
);

/* debug function to Verify the current FDB Manager Instance entries, with the backed-up FDB manager instance
   Support for HA verification & HW corruption (SER) test cases

   fdbManagerId - FDB Manager instance ID
*/
GT_STATUS prvCpssDxChBrgFdbManagerBackUpInstanceVerify
(
    IN GT_U32           fdbManagerId
);

/* debug function to simulate HW corruption

   fdbManagerId - FDB Manager instance ID
   indexesArray - Array of hwIndexs
   indexesCount - total number of indexes present in indexesArray

*/
GT_STATUS prvCpssDxChBrgFdbManagerInjectHwCorruption
(
    IN GT_U32           fdbManagerId,
    IN GT_U32           indexesArray[],
    IN GT_U32           indexesCount
);

/* debug function to print entry to be rehashed and bank entries according to calcInfo structure
   no HW access

   fdbManagerPtr        - (pointer to) FDB manager
   currentEntryPtr      - (pointer to) FDB entry to be rehashed
   currentCalcInfoPtr   - (pointer to) calcInfo structure
   totalStages          - number of total rehashed stages
   currentStage         - current rehash stage
   selectedBank         - selected bank
*/
GT_STATUS prvCpssDxChBrgFdbManagerRehashInfoPrint
(
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC          *fdbManagerPtr,
    IN  CPSS_DXCH_BRG_FDB_MANAGER_ENTRY_STC                     *currentEntryPtr,
    IN  PRV_CPSS_DXCH_FDB_MANAGER_DB_HASH_CALC_ENTRY_STC        *calcInfoPtr,
    IN  GT_U32                                                  currentStage,
    IN  GT_U32                                                  selectedBank
);

/* debug function to Verify the current FDB Manager banks population, with the total FDB Manager banks population.
   Checks only FDB manager data base counters, HW counters are not checked.
*/
GT_STATUS prvCpssDxChBrgFdbManagerBanksPopulationCheck
(
    IN GT_U32                                           fdbManagerId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChBrgFdbManager_debug_h */


