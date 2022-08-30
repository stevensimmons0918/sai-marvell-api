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
* @file prvCpssDxChCuckoo.h
*
* @brief PRV CPSS Cuckoo declarations.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChCuckooh
#define __prvCpssDxChCuckooh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>

/* max depths according to cuckoo algorithm definitions for {numOfBanks,numOfLines} */
#define _16_hash_depth   2
#define _8_hash_depth    3
#define _5_hash_depth    5
#define _4_hash_depth    5
#define _2_hash_depth    50

#define PRV_CPSS_DXCH_CUCKOO_MAX_KEY_SIZE_CNS           CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS
#define PRV_CPSS_DXCH_CUCKOO_INVALID_BANK_CNS           0x1F
#define PRV_CPSS_DXCH_CUCKOO_MAX_MANAGER_NUM_CNS        32
#define PRV_CPSS_DXCH_CUCKOO_DB_MAX_LOG_CNS             1024

/**
* @enum PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT
*
*  @brief cuckoo supported clients
*/
typedef enum{

    PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,

    PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E,

    PRV_CPSS_DXCH_CUCKOO_CLIENTS_LAST_E

} PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT;

/**
* @enum PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_ENT
*
* @brief Cuckoo key size
*/
typedef enum{

    PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_5B_E = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E,

    PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_19B_E = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E,

    PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_33B_E = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E,

    PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_47B_E = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E,

    PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_LAST_E /*TBD need to add also fdb later*/

} PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_ENT;

/**
* @struct PRV_CPSS_DXCH_CUCKOO_DB_LOG_ARRAY_STC
*
* @brief log operation for inserting new entry to HW.
*/
typedef struct
{
    GT_U8 data[PRV_CPSS_DXCH_CUCKOO_MAX_KEY_SIZE_CNS];
    /* aligned to 32 bits */

    GT_U32 sizeInBytes  : 6;    /* size in bytes of data; supporting max size 47 bytes */
    GT_U32 stage        : 7;    /* depth of recurse; supporting max stage 100 */
    GT_U32 line         : 14;   /* line num in DB; supporting max 16K lines */
    GT_U32 bank         : 4;    /* bank num in DB; supporting max 16 banks; values (0..15) */
    GT_U32 isFree       : 1;    /* is the entry free; true / false */
    /* aligned to 32 bits */

    GT_U32 isLock       : 1;    /* is the entry locked; true / false */
    GT_U32 isFirst      : 1;    /* is this the 'head' of the entry; true / false */

    /* left with 'spare' of : 32-(1+1) = 30 bits */
}PRV_CPSS_DXCH_CUCKOO_DB_LOG_ARRAY_STC;

/**
* @struct PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC
*
* @brief entry information for recurse.
*/
typedef struct
{
    GT_U8 data[PRV_CPSS_DXCH_CUCKOO_MAX_KEY_SIZE_CNS];
    /* aligned to 32 bits */

    GT_U32 sizeInBytes  : 6;    /* size in bytes of data; supporting max size 47 bytes */
    GT_U32 depth        : 7;    /* depth in recurse; supporting max 100 */
    GT_U32 line         : 14;   /* line num in DB; supporting max 16K lines */
    GT_U32 bank         : 5;    /* bank num in DB; supporting max 16 banks + invalid bank; values (0..15) + invalid value */

    /* left with 'spare' of : 32-(6+7+14+5) = 0 bit */
}PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC;

/**
* @struct PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_STC
*
* @brief entry information in DB.
*/
typedef struct
{
    GT_U8 data[PRV_CPSS_DXCH_CUCKOO_MAX_KEY_SIZE_CNS];
    /* aligned to 32 bits */

    GT_U32 sizeInBytes  : 6;    /* size in bytes of data; supporint max size 47 bytes */
    GT_U32 isFirst      : 1;    /* is this the 'head' of the entry; true / false */
    GT_U32 isLock       : 1;    /* is the entry locked; true / false */
    GT_U32 isFree       : 1;    /* is the entry free; true / false */

    /* left with 'spare' of : 32-(6+1+1+1) = 23 bits */
}PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_STC;

/**
* @struct PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC
*
* @brief statistics for cuckoo algorithm.
*/
typedef struct
{
    GT_U32 count;
    GT_U32 sizes[PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_LAST_E];
}PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC;

/**
 * @struct PRV_CPSS_DXCH_CUCKOO_DB_STC
 *
 *  @brief cuckoo information for DB
 *
*/
typedef struct
{

    /* statistics per bank */
    PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC *bank_statistics;
    /* statistics per size */
    GT_U32 size_statistics[PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_LAST_E];
    /* statistics number of recurse checks*/
    GT_U32 recurse_count;

    /* log operations*/
    PRV_CPSS_DXCH_CUCKOO_DB_LOG_ARRAY_STC log_array[PRV_CPSS_DXCH_CUCKOO_DB_MAX_LOG_CNS];
    /* pointer to last occupied location in log_array. the search should always start from the end */
    GT_U32 numOfOperations;

    /* entries information */
    PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_STC **entries_db;

    /* populated banks */
    GT_U32 *sorted_banks;

    /* system definition */
    GT_U32 db_lines;
    GT_U32 db_banks;

} PRV_CPSS_DXCH_CUCKOO_DB_STC;

/**
* @internal prvCpssDxChCuckooDbMaxDepthGet function
* @endinternal
*
* @brief Get max depth according to key size in bytes.
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] sizeInBytes     - key size in bytes
*
* @param[out] depthPtr       - (pointer to) depth
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
GT_STATUS prvCpssDxChCuckooDbMaxDepthGet
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  sizeInBytes,
    OUT GT_U32                                  *depthPtr
);

/**
* @internal prvCpssDxChCuckooDbDataGet function
* @endinternal
*
* @brief Get [bank][line] data from DB.
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] sizeInBytes     - number of bytes
*
* @param[out] dataPtr        - (pointer to) [bank][line] data in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
GT_STATUS prvCpssDxChCuckooDbDataGet
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN  GT_U32                                  sizeInBytes,
    OUT GT_U8                                   *dataPtr
);

/**
* @internal prvCpssDxChCuckooDbDataGet function
* @endinternal
*
* @brief Get entry [bank][line] info from DB.
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
*
* @param[out] entryPtr       - (pointer to) entry info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
GT_STATUS prvCpssDxChCuckooDbEntryGet
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_STC      *entryPtr
);

/**
* @internal prvCpssDxChCuckooDbDataSet function
* @endinternal
*
* @brief Set entry [bank][line] info into DB.
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] entryPtr       - (pointer to) entry info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
GT_STATUS prvCpssDxChCuckooDbEntrySet
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_STC        *entryPtr
);

/**
* @internal prvCpssDxChCuckooDbDataCheck function
* @endinternal
*
* @brief Try to find data in DB.
*
* @param[in] clientType           - client type
* @param[in] managerId            - manager identification
* @param[in] dataPtr              - (pointer to) data
* @param[in] sizeInBytes          - key size in bytes
*
* @param[out] isFoundPtr          - (pointer to) whether data is
*                                   found in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*/
GT_STATUS prvCpssDxChCuckooDbDataCheck
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U8                                   *dataPtr,
    IN  GT_U32                                  sizeInBytes,
    OUT GT_BOOL                                 *isFoundPtr
);

/**
* @internal prvCpssDxChCuckooDbRehashEntry function
* @endinternal
*
* @brief Move entry using calculated hash locations ordered according to populated banks.
*
* @param[in] clientType           - client type
* @param[in] managerId            - manager identification
* @param[in] entryInfoPtr         - (pointer to) entry to add
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on operation fail
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_SUPPORTED         - on not supported parameter
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
GT_STATUS prvCpssDxChCuckooDbRehashEntry
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC   *entryInfoPtr
);

/**
* @internal prvCpssDxChCuckooDbLogPtrGet function
* @endinternal
*
* @brief   The function returns pointer to log array in cuckoo DB.
*
* @param[in] clientType           - client type
* @param[in] managerId            - manager identification
* @param[out] numOfOperationsPtr  - (pointer to) num of
*                                   operations in log array
* @param[out] cuckooDbLogPtr      - (pointer to) log array
*
* @retval GT_OK             - on success
* @retval GT_NOT_FOUND      - on not found parameters
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
* @retval GT_BAD_PTR        - on NULL pointer value
*/
GT_STATUS prvCpssDxChCuckooDbLogPtrGet
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT            clientType,
    IN  GT_U32                                      managerId,
    OUT GT_U32                                      *numOfOperationsPtr,
    OUT PRV_CPSS_DXCH_CUCKOO_DB_LOG_ARRAY_STC       **cuckooDbLogPtr
);

/**
* @internal prvCpssDxChCuckooDbLogUpdateStages function
* @endinternal
*
* @brief    Update stages in cuckoo operation log.
*
* @param[in] clientType           - client type
* @param[in] managerId            - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
GT_STATUS prvCpssDxChCuckooDbLogUpdateStages
(
   IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
   IN GT_U32                                   managerId
);

/**
* @internal prvCpssDxChCuckooDbLogArrayDelete function
* @endinternal
*
* @brief   Delete all log operations.
*
* @param[in] clientType           - client type
* @param[in] managerId            - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
GT_STATUS prvCpssDxChCuckooDbLogArrayDelete
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId
);

/**
* @internal prvCpssDxChCuckooDbEntryAdd function
* @endinternal
*
* @brief Set data in DB in location [bank][line].
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] sizeInBytes     - data size in bytes
* @param[in] *dataPtr        - (pointer to) data
* @param[in] updateStatistics- whether to update statistics in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
GT_STATUS prvCpssDxChCuckooDbEntryAdd
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN GT_U32                                   bank,
    IN GT_U32                                   line,
    IN GT_U32                                   sizeInBytes,
    IN GT_U8                                    *dataPtr,
    IN GT_BOOL                                  updateStatistics
);

/**
* @internal prvCpssDxChCuckooDbDEntryDelete function
* @endinternal
*
* @brief Delete entry from DB in location [bank][line].
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] sizeInBytes     - data size in bytes
* @param[in] updateStatistics- whether to update statistics in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
GT_STATUS prvCpssDxChCuckooDbEntryDelete
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN GT_U32                                   bank,
    IN GT_U32                                   line,
    IN GT_U32                                   sizeInBytes,
    IN GT_BOOL                                  updateStatistics
);

/**
* @internal prvCpssDxChCuckooDbLogInfoSet function
* @endinternal
*
* @brief Set data in operation log in location [bank][line].
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] sizeInBytes     - data size in bytes
* @param[in] dataPtr         - (pointer to) data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
GT_STATUS prvCpssDxChCuckooDbLogInfoSet
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN GT_U32                                   bank,
    IN GT_U32                                   line,
    IN GT_U32                                   sizeInBytes,
    IN GT_U8                                    *dataPtr
);

/**
* @internal prvCpssDxChCuckooDbStatisticsPtrGet function
* @endinternal
*
* @brief   The function returns pointer to statistics in cuckoo DB.
*
* @param[in] clientType           - client type
* @param[in] managerId            - manager identification
* @param[out] numOfOperationsPtr  - (pointer to) num of
*                                   operations in log array
* @param[out] cuckooDbLogPtr      - (pointer to) log array
*
* @retval GT_OK             - on success
* @retval GT_NOT_FOUND      - on not found parameters
* @retval GT_BAD_PARAM      - wrong value in any of the parameters
* @retval GT_BAD_PTR        - on NULL pointer value
*/
GT_STATUS prvCpssDxChCuckooDbStatisticsPtrGet
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT            clientType,
    IN  GT_U32                                      managerId,
    OUT PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC         **cuckooDbBankStPtr,
    OUT GT_U32                                      **cuckooDbSizeStPtr
);

/**
* @internal prvCpssDxChCuckooDbInit function
* @endinternal
*
* @brief   Initialize cuckoo DB per client type and managerId.
*
* @param[in] clientType    - client type
* @param[in] managerId      - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_CPU_MEM        - on cpu memory allocation failure
*
*/
GT_STATUS prvCpssDxChCuckooDbInit
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT     clientType,
    IN GT_U32                               managerId
);

/**
* @internal prvCpssDxChCuckooDbDelete function
* @endinternal
*
* @brief   Delete cuckoo DB per client type and managerId.
*
* @param[in] clientType    - client type
* @param[in] managerId      - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
GT_STATUS prvCpssDxChCuckooDbDelete
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT     clientType,
    IN GT_U32                               managerId
);

/**
* @internal prvCpssDxChCuckooDbPrintStatistics function
* @endinternal
*
* @brief   Print cuckoo DB statistics per client type and
*          managerId.
*
* @param[in] clientType    - client type
* @param[in] managerId     - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
GT_STATUS prvCpssDxChCuckooDbPrintStatistics
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT     clientType,
    IN GT_U32                               managerId
);

/**
* @internal prvCpssDxChCuckooDbSyncStatistics function
* @endinternal
*
* @brief   Sync cuckoo DB statistics per client type and
*          managerId after failure.
*
* @param[in] clientType    - client type
* @param[in] managerId     - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
GT_STATUS prvCpssDxChCuckooDbSyncStatistics
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT     clientType,
    IN GT_U32                               managerId
);

/**
* @internal prvCpssDxChCuckooDbInitDebug function
* @endinternal
*
* @brief   Initialize cuckoo DB per client type and managerId
*          for debug.
*
* @param[in] clientType    - client type
* @param[in] managerId      - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_OUT_OF_CPU_MEM        - on cpu memory allocation failure
*
*/
GT_STATUS prvCpssDxChCuckooDbInitDebug
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT     clientType,
    IN GT_U32                               managerId,
    IN GT_U32                               db_banks,
    IN GT_U32                               db_lines
);

/*
* @internal prvCpssDxChCuckooRelocateEntriesDebug function
* @endinternal
*
* @brief    Add new entry to DB by finding free space or
*           relocate existing entries.
*
*   Entry at index Y will be copied to index Z, then new entry X will be copied to index Y
*   so traffic of existing entries won't be lossed and new entry could be added by using index X.
*
*   New entry X --> Y[i] -->Z[j]
*
* @param[in] clientType    - client type
* @param[in] managerId      - manager identification
*
*  NOTE: this function relocates entries only in DB.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
*
*/
GT_STATUS prvCpssDxChCuckooRelocateEntriesDebug
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT     clientType,
    IN GT_U32                               managerId
);

#ifdef __cplusplus
}
#endif

#endif   /* prvCpssDxChCuckooh */



