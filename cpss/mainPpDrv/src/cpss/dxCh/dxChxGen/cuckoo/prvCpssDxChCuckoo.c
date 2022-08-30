#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/exactMatch/cpssDxChExactMatch.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/cpssDxChExactMatchManagerTypes.h>
#include <cpss/dxCh/dxChxGen/exactMatch/private/prvCpssDxChExactMatchHash.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_utils.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_db.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_utils.h>
#include <cpss/dxCh/dxChxGen/bridgeFdbManager/private/prvCpssDxChBrgFdbManager_db.h>

#include <cpss/dxCh/dxChxGen/cuckoo/prvCpssDxChCuckoo.h>

/* global variables macros */

#define PRV_SHARED_CUCKOO_DIR_CUCOO_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.cuckooDir.cuckooSrc._var,_value)

#define PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.cuckooDir.cuckooSrc._var)

void prvCpssDxChCuckooDebug1PrintEnableSet(GT_U32 enable)
{
    GT_BOOL bEnable = (enable) ? GT_TRUE : GT_FALSE;
    PRV_SHARED_CUCKOO_DIR_CUCOO_SRC_GLOBAL_VAR_SET(debug1Print,bEnable);
}

void prvCpssDxChCuckooDebug2PrintEnableSet(GT_U32 enable)
{
    GT_BOOL bEnable = (enable) ? GT_TRUE : GT_FALSE;
    PRV_SHARED_CUCKOO_DIR_CUCOO_SRC_GLOBAL_VAR_SET(debug2Print,bEnable);
}

static GT_STATUS prvCpssDxChCuckooDbIsFree
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_BOOL                                 *isFreePtr
);

#define PRV_CPSS_DXCH_CUCKOO_MIN_BANK_STEP_CNS          1 /* for PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_5B_E */
#define PRV_CPSS_DXCH_CUCKOO_MAX_BANK_STEP_CNS          4 /* for PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_47B_E */
#define PRV_CPSS_DXCH_CUCKOO_MAX_BANKS_NUM_CNS          16

/* get pointer to cuckooDB */
#define CUCKOO_DB_GET_MAC(clientType,managerId)  PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId]

#define CUCKOO_PRINT_KEY(data,keySize)                                  \
{                                                                       \
    cpssOsPrintf("keySize [%d] ",keySize);                              \
    cpssOsPrintf("pattern:");                                           \
    for (n=0; n<keySize; n++)                                           \
    {                                                                   \
        cpssOsPrintf("[%d]",data[n]);                                   \
    }                                                                   \
    cpssOsPrintf("\n");                                                 \
}

#define CUCKOO_PRINT_LOG_ENTRY(logEntry,index)                                   \
    cpssOsPrintf("%d. log_array[%d][%d]:\n",index+1,logEntry.bank,logEntry.line);\
    cpssOsPrintf("   ");                                                         \
    CUCKOO_PRINT_KEY(logEntry.data,logEntry.sizeInBytes);                        \
    cpssOsPrintf("   isFirst [%d] isLock [%d] isFree [%d] stage [%d]\n",logEntry.isFirst, logEntry.isLock, logEntry.isFree, logEntry.stage);\
    cpssOsPrintf("\n");

#define CUCKOO_PRINT_DB_ENTRY(dbEntry,index,bank,line)                           \
    cpssOsPrintf("%d. db[%d][%d]:\n",index+1,bank,line);                         \
    cpssOsPrintf("   ");                                                         \
    CUCKOO_PRINT_KEY(dbEntry.data,dbEntry.sizeInBytes);                          \
    cpssOsPrintf("   isFirst [%d] isLock [%d] isFree [%d]\n",dbEntry.isFirst, dbEntry.isLock, dbEntry.isFree);\
    cpssOsPrintf("\n");

#define CUCKOO_PRINT_DB_ENTRY1(dbEntry,bank,line)                                \
    cpssOsPrintf("db[%d][%d]:\n",bank,line);                                     \
    CUCKOO_PRINT_KEY(dbEntry.data,dbEntry.sizeInBytes);                          \
    cpssOsPrintf("isFirst [%d] isLock [%d] isFree [%d]\n",dbEntry.isFirst, dbEntry.isLock, dbEntry.isFree);\
    cpssOsPrintf("\n");

/* macro to check that x == 0 is GT_BAD_STATE */
#define CUCKOO_CHECK_X_NOT_ZERO_MAC(x)                  \
    if((x) == 0)                                        \
    {                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,     \
            "[%s] must not be ZERO",                    \
            #x);                                        \
    }

/**
* @enum PRV_CPSS_DXCH_CUCKOO_MHT_ENT
*
* @brief Enum values represent num of Multiple Hash Tables
*/
typedef enum{

    /* Four Multiple Hash Tables */
    PRV_CPSS_DXCH_CUCKOO_MHT_4_E,

    /* Eight Multiple Hash Tables */
    PRV_CPSS_DXCH_CUCKOO_MHT_8_E,

    /* Sixteen Multiple Hash Tables */
    PRV_CPSS_DXCH_CUCKOO_MHT_16_E,

    /* last value */
    PRV_CPSS_DXCH_CUCKOO_MHT_LAST_E

}PRV_CPSS_DXCH_CUCKOO_MHT_ENT;

/**
* @struct PRV_CPSS_DXCH_CUCKOO_DB_HASH_STC
*
* @brief hash entry information.
*/
typedef struct
{
    GT_U32 line;
    GT_U32 bank;
}PRV_CPSS_DXCH_CUCKOO_DB_HASH_STC;

/* max depths according to cuckoo algorithm {Multiple Hash Tables,key size} */
static const GT_U32 hash_depth[PRV_CPSS_DXCH_CUCKOO_MHT_LAST_E][PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_LAST_E] ={
/*         5B             19B           33B           47B           */
/*4MTH*/ {_4_hash_depth, _2_hash_depth,            2,            2},
/*8MTH*/ {_8_hash_depth, _4_hash_depth,_2_hash_depth,_2_hash_depth},
/*16MTH*/{_16_hash_depth,_8_hash_depth,_5_hash_depth,_4_hash_depth}};

static GT_STATUS prvCpssDxChCuckooDbRecursiveEntryAdd
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC   *entryInfoPtr
);

/**
* @internal prvCpssDxChCuckooHashFuncBind function
* @endinternal
*
* @brief Bind hash function specific to client.
*
* @param[in] clientType            - client type
* @param[in] managerId             - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameters in entryKeyPtr.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
*/
static GT_STATUS prvCpssDxChCuckooHashFuncBind
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT    clientType,
    IN  GT_U32                              managerId,
    ...
)
{
    va_list argParam;
    GT_U32 i,sizeInBytes;
    GT_U32 *numOfElem, *crcMultiHashArr;
    GT_U8 data[PRV_CPSS_DXCH_CUCKOO_MAX_KEY_SIZE_CNS];
    /* exact match */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    CPSS_DXCH_EXACT_MATCH_KEY_STC emEntryKey;
    CPSS_EXACT_MATCH_MHT_ENT exactMatchBanksNum;
    GT_U32 exactMatchSize;

    /* fdb */
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr;
    CPSS_MAC_ENTRY_EXT_KEY_STC fdbEntryKey;

    /* init variable argument list */
    va_start(argParam, managerId);

    switch (clientType)
    {
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:
        sizeInBytes = va_arg(argParam, GT_U32);
        for (i=0; i<sizeInBytes; i++)
            data[i] = (GT_U8)va_arg(argParam, GT_U32);
        numOfElem = va_arg(argParam, GT_U32*);
        crcMultiHashArr = va_arg(argParam, GT_U32*);

        /* free VA list pointer */
        va_end(argParam);

        switch(sizeInBytes)
        {
        case 5:
            emEntryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
            break;
        case 19:
            emEntryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
            break;
        case 33:
            emEntryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
            break;
        case 47:
            emEntryKey.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(sizeInBytes);
        }
        cpssOsMemCpy(emEntryKey.pattern, data, sizeof(GT_U8)*sizeInBytes);

        EM_MANAGER_ID_CHECK(managerId);
        exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);

        switch(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes)
        {
            case 4:
                exactMatchBanksNum=CPSS_EXACT_MATCH_MHT_4_E;
                break;
            case 8:
                exactMatchBanksNum=CPSS_EXACT_MATCH_MHT_8_E;
                break;
            case 16:
                exactMatchBanksNum=CPSS_EXACT_MATCH_MHT_16_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes);
        }

        switch(exactMatchManagerPtr->cpssHashParams.size)
        {
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_8K_E:
                exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_8KB;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_16K_E:
                exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_16KB;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_32K_E:
                exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_32KB;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_64K_E:
                exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_64KB;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_128K_E:
                exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_128KB;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_256K_E:
                exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_256KB;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(exactMatchManagerPtr->cpssHashParams.size);
        }
        /*******************************/
        /* calc the 4/8/16 hash values */
        /*******************************/

        return prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalcExt(exactMatchSize,exactMatchBanksNum,
                                                                       &emEntryKey,numOfElem,crcMultiHashArr);
        break;

    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E:

        for (i=0; i<6; i++) /* data contains only mac 6 bytes */
            data[i] = (GT_U8)va_arg(argParam, GT_U32);
        crcMultiHashArr = va_arg(argParam, GT_U32*);

        /* free VA list pointer */
        va_end(argParam);

        FDB_MANAGER_ID_CHECK(managerId);
        fdbManagerPtr = MANAGER_GET_MAC(managerId);

        fdbEntryKey.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
        for (i=0; i<6; i++)
            fdbEntryKey.key.macVlan.macAddr.arEther[i] = data[i];
        fdbEntryKey.key.macVlan.vlanId = 1; /* TBD temp only for test */
        fdbEntryKey.vid1 = 1;

        /*******************************/
        /* calc the 4/8/16 hash values */
        /*******************************/

        return cpssDxChBrgFdbHashCrcMultiResultsByParamsCalc(&fdbManagerPtr->cpssHashParams,&fdbEntryKey,0,/*multiHashStartBankIndex*/
                                                             fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes,crcMultiHashArr);

        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(clientType);
        break;
    }
}

/* check that the key sizeInBytes is in range */
#define PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,sizeInBytes)\
{                                                                               \
    switch (clientType)                                                         \
    {                                                                           \
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:                            \
        switch (sizeInBytes)                                                    \
        {                                                                       \
        case 5:                                                                 \
        case 19:                                                                \
        case 33:                                                                \
        case 47:                                                                \
            break;                                                              \
        default:                                                                \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in sizeInBytes [%d]",sizeInBytes);\
        }                                                                       \
        break;                                                                  \
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E:                                    \
        switch (sizeInBytes)                                                    \
        {                                                                       \
        case 6:     /* TBD to check */                                         \
            break;                                                              \
        default:                                                                \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in sizeInBytes [%d]",sizeInBytes);\
        }                                                                       \
        break;                                                                  \
    default:                                                                    \
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(clientType);                      \
    }                                                                           \
}

/* check that bank step is in range */
#define PRV_CPSS_DXCH_CUCKOO_DB_BANK_STEP_ID_CHECK_MAC(clientType,bankStep)     \
{                                                                               \
    switch (clientType)                                                         \
    {                                                                           \
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:                            \
        switch (bankStep)                                                       \
        {                                                                       \
        case 1:                                                                 \
        case 2:                                                                 \
        case 3:                                                                 \
        case 4:                                                                 \
            break;                                                              \
        default:                                                                \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in bankStep [%d]",bankStep);\
        }                                                                       \
        break;                                                                  \
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E:                                    \
        switch (bankStep)                                                       \
        {                                                                       \
        case 2: /* TBD to check */                                             \
            break;                                                              \
        default:                                                                \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in bankStep [%d]",bankStep);\
        }                                                                       \
        break;                                                                  \
    default:                                                                    \
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(clientType);                      \
    }                                                                           \
}

/* check that the bank is in range */
#define PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank)    \
    if (bank >= PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId]->db_banks)                      \
    {                                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in bank [%d]",bank); \
    }

/* check that the line is in range */
#define PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line)    \
    if (line >= PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId]->db_lines)                      \
    {                                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in line [%d]",line); \
    }

/* check that DB was initialized  */
#define PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId)            \
    if (clientType >= PRV_CPSS_DXCH_CUCKOO_CLIENTS_LAST_E)                      \
    {                                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in client type [%d]",clientType); \
    }                                                                           \
    if (managerId >= PRV_CPSS_DXCH_CUCKOO_MAX_MANAGER_NUM_CNS)                  \
    {                                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in manageId type [%d]",managerId); \
    }                                                                           \
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId] == NULL)                                \
    {                                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error DB is not initialized"); \
    }

/* check that client type is supported */
#define PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType)            \
    if (clientType >= PRV_CPSS_DXCH_CUCKOO_CLIENTS_LAST_E)                      \
    {                                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in client type [%d]",clientType); \
    }

#ifdef CPSS_LOG_ENABLE
    /* check that [bank][line] is not free in DB */
    #define PRV_CPSS_DXCH_CUCKOO_DB_BANK_LINE_IS_VALID_CHECK_MAC(clientType,managerId,bank,line) \
    {                                                                               \
        PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);\
        rc = prvCpssDxChCuckooDbIsFree(clientType,managerId,bank,line,&is_free);    \
        if (rc != GT_OK)                                                            \
        {                                                                           \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in one of the parameters: client type [%d] managerId [%d] bank [%d] line [%d]",clientType,managerId,bank,line);\
        }                                                                           \
        if (is_free)                                                                \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: trying to access free entry DB[%d][%d] isFree [%d]",bank,line,cuckooDbPtr->entries_db[bank][line].isFree);\
    }
#else
    #define PRV_CPSS_DXCH_CUCKOO_DB_BANK_LINE_IS_VALID_CHECK_MAC(clientType,managerId,bank,line) \
    {                                                                               \
        rc = prvCpssDxChCuckooDbIsFree(clientType,managerId,bank,line,&is_free);    \
        if (rc != GT_OK)                                                            \
        {                                                                           \
            return rc;                                                              \
        }                                                                           \
        if (is_free)                                                                \
            return /*no log info*/ GT_BAD_STATE;                                    \
    }
#endif

/* macro to convert cuckoo key size in bytes to bank step */
#define PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(sizeInBytes,bankStep)  \
    switch (sizeInBytes)                                                                    \
    {                                                                                       \
    case 5:                                                                                 \
        bankStep = 1;                                                                       \
        break;                                                                              \
    case 19:                                                                                \
        bankStep = 2;                                                                       \
        break;                                                                              \
    case 33:                                                                                \
        bankStep = 3;                                                                       \
        break;                                                                              \
    case 47:                                                                                \
        bankStep = 4;                                                                       \
        break;                                                                              \
    default:                                                                                \
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(sizeInBytes);                                 \
    }

/**
* @internal prvCpssDxChCuckooDbIsLock function
* @endinternal
*
* @brief Check if [bank][line] for number of banks is locked in DB.
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] size            - number of banks to check
*
* @param[out] isLockPtr      - (pointer to) whether [bank][line] for
*                               number of banks is locked in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*/
static GT_STATUS prvCpssDxChCuckooDbIsLock
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN  GT_U32                                  size,
    OUT GT_BOOL                                 *isLockPtr
)
{
    GT_BOOL is_lock=GT_FALSE;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    GT_U32 i;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_STEP_ID_CHECK_MAC(clientType,size);
    CPSS_NULL_PTR_CHECK_MAC(isLockPtr);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    for (i=0; i<size; i++)
    {
        PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank+i);

        is_lock = (cuckooDbPtr->entries_db[bank+i][line].isLock == GT_TRUE);
        if (is_lock)
        {
            /* if one of the banks is locked; no need to check all other banks */
            *isLockPtr = is_lock;
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
            {
                cpssOsPrintf("cuckooDB.entries_db[%d][%d] isLocked [%d]\n",bank,line,*isLockPtr);
            }
            return GT_OK;
        }
    }
    *isLockPtr = is_lock;
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("cuckooDB.entries_db[%d][%d] isLocked [%d]\n",bank,line,*isLockPtr);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChCuckooDbSetLock function
* @endinternal
*
* @brief lock/unlock [bank][line] for number of banks in DB.
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] size            - number of banks to lock/unlock
* @param[in] lock            - whether to lock/unlock [bank][line]
*                              for number of banks in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_STATE             - on invalid parameter
*
*/
static GT_STATUS prvCpssDxChCuckooDbSetLock
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN GT_U32                                  managerId,
    IN GT_U32                                  bank,
    IN GT_U32                                  line,
    IN GT_U32                                  size,
    IN GT_BOOL                                 lock
)
{
    GT_U32 i;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_STEP_ID_CHECK_MAC(clientType,size);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    for (i=0; i<size; i++)
    {
        PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank+i);

        if (lock)
        {
            cuckooDbPtr->entries_db[bank+i][line].isLock = GT_TRUE;
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
            {
                cpssOsPrintf("lock cuckooDB.entries_db[%d][%d] isLocked [%d]\n",bank+i,line,cuckooDbPtr->entries_db[bank+i][line].isLock);
            }
        }
        else
        {
            /* this is a problem */
            if (cuckooDbPtr->entries_db[bank+i][line].isLock == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: trying to unlock not locked entry cuckooDB[%d][%d] isLocked [%d]",bank+i,line,cuckooDbPtr->entries_db[bank+i][line].isLock);

            }
            cuckooDbPtr->entries_db[bank+i][line].isLock = GT_FALSE;
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
            {
                cpssOsPrintf("unlock cuckooDB.entries_db[%d][%d] isLocked [%d]\n",bank+i,line,cuckooDbPtr->entries_db[bank+i][line].isLock);
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChCuckooDbIsFree function
* @endinternal
*
* @brief Check if [bank][line] is free in DB.
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
*
* @param[out] isFreePtr      - (pointer to) whether [bank][line]
*                               is free in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*/
static GT_STATUS prvCpssDxChCuckooDbIsFree
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_BOOL                                 *isFreePtr
)
{
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    CPSS_NULL_PTR_CHECK_MAC(isFreePtr);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    *isFreePtr = (cuckooDbPtr->entries_db[bank][line].isFree == GT_TRUE);
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("cuckooDB.entries_db[%d][%d] isFree [%d]\n",bank,line,cuckooDbPtr->entries_db[bank][line].isFree);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChCuckooDbLogIsFree function
* @endinternal
*
* @brief Check if [bank][line] is free in log operations.
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
*
* @param[out] isFreePtr      - (pointer to) whether [bank][line]
*                               is free in log operations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_FOUND             - on not found parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*/
static GT_STATUS prvCpssDxChCuckooDbLogIsFree
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_BOOL                                 *isFreePtr
)
{
    GT_U32 j;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    CPSS_NULL_PTR_CHECK_MAC(isFreePtr);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    /* start to scan from the end untill numOfOperations */
    for (j=cuckooDbPtr->numOfOperations; j>0; j--)
    {
        if ((cuckooDbPtr->log_array[j-1].bank == bank) && (cuckooDbPtr->log_array[j-1].line == line))
        {
            *isFreePtr = (cuckooDbPtr->log_array[j-1].isFree == 1);
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
            {
                cpssOsPrintf("cuckooDB.log_array[%d] Bank [%d] Line [%d] isFree [%d]\n",j-1,bank,line,cuckooDbPtr->log_array[j-1].isFree);
            }
            return GT_OK;
        }
    }

    /* didn't find [bank][line] in log */
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("didn't find [%d][%d] in operation log\n",bank,line);
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChCuckooDbSizeGet function
* @endinternal
*
* @brief Get [bank][line] size in Bytes from DB.
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
*
* @param[out] sizeInBytesPtr      - (pointer to) [bank][line] size in bytes in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*/
static GT_STATUS prvCpssDxChCuckooDbSizeGet
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_U32                                  *sizeInBytesPtr
)
{
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    GT_STATUS rc;
    GT_BOOL is_free;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    CPSS_NULL_PTR_CHECK_MAC(sizeInBytesPtr);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_LINE_IS_VALID_CHECK_MAC(clientType,managerId,bank,line);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    *sizeInBytesPtr = cuckooDbPtr->entries_db[bank][line].sizeInBytes;

    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,*sizeInBytesPtr);
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("cuckooDB.entries_db[%d][%d] sizeInBytes [%d]\n",bank,line,cuckooDbPtr->entries_db[bank][line].sizeInBytes);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChCuckooDbLogSizeGet function
* @endinternal
*
* @brief Get [bank][line] size in Bytes from operation log.
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
*
* @param[out] sizeInBytesPtr      - (pointer to) [bank][line]
*                                   size in bytes in operation log
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_FOUND             - on not found parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_BAD_STATE             - on invalid parameter
*
*/
static GT_STATUS prvCpssDxChCuckooDbLogSizeGet
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_U32                                  *sizeInBytesPtr
)
{
    GT_U32 j;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    CPSS_NULL_PTR_CHECK_MAC(sizeInBytesPtr);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    /* start to scan from the end untill numOfOperations */
    for (j=cuckooDbPtr->numOfOperations; j>0; j--)
    {
        if ((cuckooDbPtr->log_array[j-1].bank == bank) && (cuckooDbPtr->log_array[j-1].line == line))
        {
            if (cuckooDbPtr->log_array[j-1].isFree == GT_TRUE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: trying to access free entry in log[%d] Bank [%d] Line [%d] isFree [%d]",j-1,bank,line,cuckooDbPtr->log_array[j-1].isFree);
            }

            *sizeInBytesPtr = cuckooDbPtr->log_array[j-1].sizeInBytes;

            PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,*sizeInBytesPtr);
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
            {
                cpssOsPrintf("cuckooDB.log_array[%d] Bank [%d] Line [%d] sizeInBytes [%d]\n",j-1,bank,line,cuckooDbPtr->log_array[j-1].sizeInBytes);
            }
            return GT_OK;
        }
    }

    /* didn't find [bank][line] in log */
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("didn't find [%d][%d] in operation log\n",bank,line);
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}

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
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN  GT_U32                                  sizeInBytes,
    OUT GT_U8                                   *dataPtr
)
{
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    GT_STATUS rc;
    GT_BOOL is_free;
    GT_U32 n;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,sizeInBytes);
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_LINE_IS_VALID_CHECK_MAC(clientType,managerId,bank,line);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    /* copy all data from DB */
    cpssOsMemCpy(dataPtr,cuckooDbPtr->entries_db[bank][line].data,sizeof(GT_U8)*sizeInBytes);
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("cuckooDB.entries_db[%d][%d]: ",bank,line);
        CUCKOO_PRINT_KEY(cuckooDbPtr->entries_db[bank][line].data,sizeInBytes);
        cpssOsPrintf("\n");
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChCuckooDbEntryGet function
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
)
{
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    GT_STATUS rc;
    GT_BOOL is_free;
    GT_U32 n;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_LINE_IS_VALID_CHECK_MAC(clientType,managerId,bank,line);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    entryPtr->isFree = cuckooDbPtr->entries_db[bank][line].isFree;
    if (cuckooDbPtr->entries_db[bank][line].isFree == GT_TRUE)
    {
        return GT_OK;
    }

    entryPtr->isLock = cuckooDbPtr->entries_db[bank][line].isLock;
    entryPtr->isFirst = cuckooDbPtr->entries_db[bank][line].isFirst;
    entryPtr->sizeInBytes = cuckooDbPtr->entries_db[bank][line].sizeInBytes;
    /* copy all data from DB */
    cpssOsMemCpy(entryPtr->data,cuckooDbPtr->entries_db[bank][line].data,sizeof(GT_U8)*cuckooDbPtr->entries_db[bank][line].sizeInBytes);
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("cuckooDB.entries_db[%d][%d]: isFree[%d] isLock[%d] isFirst[%d]",bank,line,entryPtr->isFree,entryPtr->isLock,entryPtr->isFirst);
        CUCKOO_PRINT_KEY(cuckooDbPtr->entries_db[bank][line].data,cuckooDbPtr->entries_db[bank][line].sizeInBytes);
        cpssOsPrintf("\n");
    }

    return GT_OK;
}

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
)
{
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    GT_STATUS rc;
    GT_BOOL is_free;
    GT_U32 n;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_LINE_IS_VALID_CHECK_MAC(clientType,managerId,bank,line);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    cuckooDbPtr->entries_db[bank][line].isFree = entryPtr->isFree;
    if (entryPtr->isFree == GT_TRUE)
    {
        return GT_OK;
    }

    cuckooDbPtr->entries_db[bank][line].isLock = entryPtr->isLock;
    cuckooDbPtr->entries_db[bank][line].isFirst = entryPtr->isFirst;
    cuckooDbPtr->entries_db[bank][line].sizeInBytes = entryPtr->sizeInBytes;
    /* copy all data from DB */
    cpssOsMemCpy(cuckooDbPtr->entries_db[bank][line].data,entryPtr->data,sizeof(GT_U8)*entryPtr->sizeInBytes);
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("cuckooDB.entries_db[%d][%d]: isFree[%d] isLock[%d] isFirst[%d]",bank,line,entryPtr->isFree,entryPtr->isLock,entryPtr->isFirst);
        CUCKOO_PRINT_KEY(cuckooDbPtr->entries_db[bank][line].data,cuckooDbPtr->entries_db[bank][line].sizeInBytes);
        cpssOsPrintf("\n");
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChCuckooDbLogDataGet function
* @endinternal
*
* @brief Get [bank][line] data from operation log.
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] sizeInBytes     - number of bytes
*
* @param[out] data      - (pointer to) [bank][line] data in
*                         operation log
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_FOUND             - on not found parameters
* @retval GT_BAD_PTR               - on NULL pointer value
* @retval GT_BAD_STATE             - on invalid parameter
*
*/
GT_STATUS prvCpssDxChCuckooDbLogDataGet
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN  GT_U32                                  sizeInBytes,
    OUT GT_U8                                   *dataPtr
)
{
    GT_BOOL is_free;
    GT_U32 j,n;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,sizeInBytes);
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    /* start to scan from the end untill numOfOperations */
    for (j=cuckooDbPtr->numOfOperations; j>0; j--)
    {
        if ((cuckooDbPtr->log_array[j-1].bank == bank) && (cuckooDbPtr->log_array[j-1].line == line))
        {
            is_free = (cuckooDbPtr->log_array[j-1].isFree == GT_TRUE);
            if (is_free)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: trying to access free entry in log[%d] Bank [%d] Line [%d] isFree [%d]",j-1,bank,line,cuckooDbPtr->log_array[j-1].isFree);
            }

            /* copy all data from log */
            cpssOsMemCpy(dataPtr,cuckooDbPtr->log_array[j-1].data,sizeof(GT_U8)*sizeInBytes);
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
            {
                cpssOsPrintf("cuckooDB.log_array[%d] Bank [%d] Line [%d]: ",j-1,bank,line);
                CUCKOO_PRINT_KEY(cuckooDbPtr->log_array[j-1].data,sizeInBytes);
                cpssOsPrintf("\n");
            }
            return GT_OK;
        }
    }

    /* didn't find [bank][line] in log */
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("didn't find [%d][%d] in operation log\n",bank,line);
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChCuckooDbIsFirst function
* @endinternal
*
* @brief Check if [bank][line] is first (head) in DB.
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
*
* @param[out] isFirstPtr      - (pointer to) whether
*                               [bank][line] is first in DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
static GT_STATUS prvCpssDxChCuckooDbIsFirst
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_BOOL                                 *isFirstPtr
)
{
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    GT_STATUS rc;
    GT_BOOL is_free;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    CPSS_NULL_PTR_CHECK_MAC(isFirstPtr);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_LINE_IS_VALID_CHECK_MAC(clientType,managerId,bank,line);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    *isFirstPtr = (cuckooDbPtr->entries_db[bank][line].isFirst == GT_TRUE);
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("cuckooDB.entries_db[%d][%d] isFirst [%d]\n",bank,line,cuckooDbPtr->entries_db[bank][line].isFirst);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChCuckooDbLogIsFirst function
* @endinternal
*
* @brief Check if [bank][line] is first (head) in log operations.
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
*
* @param[out] isFirstPtr      - (pointer to) whether
*                               [bank][line] is first in log
*                               operations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_FOUND             - on not found parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
static GT_STATUS prvCpssDxChCuckooDbLogIsFirst
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    OUT GT_BOOL                                 *isFirstPtr
)
{
    GT_BOOL is_free,j;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    CPSS_NULL_PTR_CHECK_MAC(isFirstPtr);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    /* start to scan from the end untill numOfOperations */
    for (j=cuckooDbPtr->numOfOperations; j>0; j--)
    {
        if ((cuckooDbPtr->log_array[j-1].bank == bank) && (cuckooDbPtr->log_array[j-1].line == line))
        {
            is_free = (cuckooDbPtr->log_array[j-1].isFree == GT_TRUE);
            if (is_free)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: trying to access free entry in log[%d] Bank [%d] Line [%d] isFree [%d]",j-1,bank,line,cuckooDbPtr->log_array[j-1].isFree);
            }
            *isFirstPtr = (cuckooDbPtr->log_array[j-1].isFirst == GT_TRUE);
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
            {
                cpssOsPrintf("cuckooDB.log_array[%d] Bank [%d] Line [%d] isFirst [%d]\n",j-1,bank,line,cuckooDbPtr->log_array[j-1].isFirst);
            }
            return GT_OK;
        }
    }

    /* didn't find [bank][line] in log */
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("didn't find [%d][%d] in operation log\n",bank,line);
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
}

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
)
{
    PRV_CPSS_DXCH_CUCKOO_MHT_ENT mhtIndex;
    PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_ENT keySize;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,sizeInBytes);
    CPSS_NULL_PTR_CHECK_MAC(depthPtr);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    switch (cuckooDbPtr->db_banks)
    {
    case 4:
        mhtIndex = PRV_CPSS_DXCH_CUCKOO_MHT_4_E;
        break;
    case 8:
        mhtIndex = PRV_CPSS_DXCH_CUCKOO_MHT_8_E;
        break;
    case 16:
        mhtIndex = PRV_CPSS_DXCH_CUCKOO_MHT_16_E;
        break;
     default:
         CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(cuckooDbPtr->db_banks);
    }

    switch (sizeInBytes)
    {
    case 5:
        keySize = PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_5B_E;
        break;
    case 19:
        keySize = PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_19B_E;
        break;
    case 33:
        keySize = PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_33B_E;
        break;
    case 47:
        keySize = PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_47B_E;
        break;
    default:
         CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(sizeInBytes);
    }
    *depthPtr = hash_depth[mhtIndex][keySize];
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("depth [%d] for sizeInBytes [%d]\n",*depthPtr,sizeInBytes);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChCuckooDbCountFreeGet function
* @endinternal
*
* @brief Count consecutive free banks from [bank][line] untill size.
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] size            - number of banks
*
* @param[out] numFreePtr      - (pointer to) consecutive free banks
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_FOUND             - on not found parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
static GT_STATUS prvCpssDxChCuckooDbCountFreeGet
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN  GT_U32                                  size,
    OUT GT_U32                                  *numFreePtr
)
{
    GT_STATUS rc;
    GT_U32 count=0, i;
    GT_BOOL is_free;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

     /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_STEP_ID_CHECK_MAC(clientType,size);
    CPSS_NULL_PTR_CHECK_MAC(numFreePtr);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    for (i=0; i<size; i++)
    {
        /* no more banks */
        if (bank+i >= cuckooDbPtr->db_banks)
        {
            break;
        }

        PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank+i);

        /* check if [bank+i][line] is free in log */
        rc = prvCpssDxChCuckooDbLogIsFree(clientType,managerId,bank+i,line,&is_free);
        if (rc != GT_OK && rc != GT_NOT_FOUND)
        {
            return rc;
        }
        /* didn't find [bank+i][line] in log; look in DB */
        if (rc == GT_NOT_FOUND)
        {
            /* check if [bank+i][line] is free in DB */
            rc = prvCpssDxChCuckooDbIsFree(clientType,managerId,bank+i,line,&is_free);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (is_free)
                count++; /* found free entry in DB */
            else
                break; /* no more consecutive free banks */
        }
        else
        {
            if (is_free)
                count++; /* found free entry in log */
            else
                break; /* no more consecutive free banks */
        }
    }
    *numFreePtr = count;
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("number of free consecutive banks [%d] for Bank [%d] Line [%d] size [%d]\n",*numFreePtr,bank,line,size);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChCuckooSortPopulatedBanks function
* @endinternal
*
* @brief Sort populated banks.
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
static GT_STATUS prvCpssDxChCuckooSortPopulatedBanks
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId
)
{
    GT_U32 i,bank1,bank2,count;
    GT_BOOL move=GT_TRUE;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    while (move == GT_TRUE)
    {
       move = GT_FALSE;
       for (i=0; i<cuckooDbPtr->db_banks-1; i++)
       {
           bank1 = cuckooDbPtr->sorted_banks[i];
           bank2 = cuckooDbPtr->sorted_banks[i+1];
           if (cuckooDbPtr->bank_statistics[bank1].count < cuckooDbPtr->bank_statistics[bank2].count)
           {
               cuckooDbPtr->sorted_banks[i] = bank2;
               cuckooDbPtr->sorted_banks[i+1] = bank1;
               move = GT_TRUE;
           }
       }
    }

    /* after sorting the most populated banks are at the begining of the array.
       If the bank is full move it to the end of the array. */
    count=0;
    while (count<cuckooDbPtr->db_banks-1)
    {
        bank1 = cuckooDbPtr->sorted_banks[0];
        if (cuckooDbPtr->bank_statistics[bank1].count == cuckooDbPtr->db_lines)
        {
            for (i=0; i<cuckooDbPtr->db_banks-1; i++)
            {
                cuckooDbPtr->sorted_banks[i] = cuckooDbPtr->sorted_banks[i+1];
            }
            cuckooDbPtr->sorted_banks[i] = bank1;
            count++;
        }
        else
        {
            break;
        }

    }

    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
    {
        cpssOsPrintf("sort bank [count]: ");
        for (i=0; i< cuckooDbPtr->db_banks; i++)
            cpssOsPrintf(" %d [%d] ", cuckooDbPtr->sorted_banks[i], cuckooDbPtr->bank_statistics[cuckooDbPtr->sorted_banks[i]].count );
        cpssOsPrintf("\n");
    }

    return GT_OK;
}

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
)
{
    GT_U32 size,offset,n;
    GT_BOOL is_free,is_lock;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,sizeInBytes);
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);

    PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(sizeInBytes,size);
    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    for (offset = 0; offset < size; offset++)
    {
        /* sanity check; set only on free entries */
        rc = prvCpssDxChCuckooDbLogIsFree(clientType,managerId,bank+offset,line,&is_free);
        if (rc != GT_OK && rc != GT_NOT_FOUND)
        {
            return rc;
        }
        /* [bank+offset][line] doesn't exist in log; check in DB */
        if (rc == GT_NOT_FOUND)
        {
            rc = prvCpssDxChCuckooDbIsFree(clientType,managerId,bank+offset,line,&is_free);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (is_free == GT_FALSE)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: trying to set value to occupied entry in DB Bank [%d] Line [%d] isFree [%d]",bank+offset,line,is_free);

        }
        if (is_free == GT_FALSE)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: trying to set value to occupied entry in log Bank [%d] Line [%d] isFree [%d]",bank+offset,line,is_free);


        rc = prvCpssDxChCuckooDbIsLock(clientType,managerId,bank+offset,line,1,&is_lock);
        if (rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank+offset);

        /* insert info into log */
        if (offset == 0)
            cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].isFirst = GT_TRUE;
        else
            cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].isFirst = GT_FALSE;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].bank = bank+offset;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].line = line;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].sizeInBytes = sizeInBytes;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].isLock = is_lock;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].isFree = GT_FALSE;
        cpssOsMemCpy(cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].data,dataPtr,sizeof(GT_U8)*sizeInBytes);
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("Added Info into operation log:\n");
            CUCKOO_PRINT_LOG_ENTRY(cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations],cuckooDbPtr->numOfOperations);
        }
        cuckooDbPtr->numOfOperations++;
        cuckooDbPtr->bank_statistics[bank+offset].count++;
        cuckooDbPtr->bank_statistics[bank+offset].sizes[size-1]++;
        cuckooDbPtr->size_statistics[size-1]++;
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("** increment bank [%d] count [%d]\n",bank+offset,cuckooDbPtr->bank_statistics[bank+offset].count);
        }
    }

    return GT_OK;
}

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
)
{
    GT_U32 size,offset,n;
    GT_BOOL is_free;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,sizeInBytes);
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);

    PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(sizeInBytes,size);
    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    for (offset = 0; offset < size; offset++)
    {
        /* sanity check; set only on free entries */
        rc = prvCpssDxChCuckooDbIsFree(clientType,managerId,bank+offset,line,&is_free);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (is_free == GT_FALSE)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: trying to set value to occupied entry in DB Bank [%d] Line [%d] isFree [%d]",bank+offset,line,is_free);

        PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank+offset);

        /* insert info into DB */
        if (offset == 0)
            cuckooDbPtr->entries_db[bank+offset][line].isFirst = GT_TRUE;
        else
            cuckooDbPtr->entries_db[bank+offset][line].isFirst = GT_FALSE;
        cuckooDbPtr->entries_db[bank+offset][line].sizeInBytes = sizeInBytes;
        cuckooDbPtr->entries_db[bank+offset][line].isFree = GT_FALSE;
        cuckooDbPtr->entries_db[bank+offset][line].isLock = GT_FALSE;
        cpssOsMemCpy(cuckooDbPtr->entries_db[bank+offset][line].data,dataPtr,sizeof(GT_U8)*sizeInBytes);
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("Updated cuckooDB Entry add:\n");
            CUCKOO_PRINT_DB_ENTRY1(cuckooDbPtr->entries_db[bank+offset][line],bank+offset,line);
        }
        if (updateStatistics) {
            cuckooDbPtr->bank_statistics[bank+offset].count++;
            cuckooDbPtr->bank_statistics[bank+offset].sizes[size-1]++;
            cuckooDbPtr->size_statistics[size-1]++;
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
            {
                cpssOsPrintf("** increment bank [%d] count [%d]\n",bank+offset,cuckooDbPtr->bank_statistics[bank+offset].count);
            }
        }
    }

    /* sort populated banks after log insertion */
    rc = prvCpssDxChCuckooSortPopulatedBanks(clientType,managerId);

    return rc;
}

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
)
{
    GT_U32 size,offset,n;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,sizeInBytes);

    PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(sizeInBytes,size);
    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    for (offset = 0; offset < size; offset++)
    {
        PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank+offset);

        /* insert info into DB */
        cuckooDbPtr->entries_db[bank+offset][line].isFirst = GT_FALSE;
        cuckooDbPtr->entries_db[bank+offset][line].sizeInBytes = 0;
        cuckooDbPtr->entries_db[bank+offset][line].isFree = GT_TRUE;
        cuckooDbPtr->entries_db[bank+offset][line].isLock = GT_FALSE;
        cpssOsMemSet(cuckooDbPtr->entries_db[bank+offset][line].data,0,sizeof(GT_U8)*PRV_CPSS_DXCH_CUCKOO_MAX_KEY_SIZE_CNS);
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("Updated cuckooDB Entry delete:\n");
            CUCKOO_PRINT_DB_ENTRY1(cuckooDbPtr->entries_db[bank+offset][line],bank+offset,line);
        }

        if (updateStatistics)
        {
            /* coherency check -- can't be ZERO because we are going to decrement it */
            CUCKOO_CHECK_X_NOT_ZERO_MAC(cuckooDbPtr->bank_statistics[bank+offset].count);
            CUCKOO_CHECK_X_NOT_ZERO_MAC(cuckooDbPtr->bank_statistics[bank+offset].sizes[size-1]);
            CUCKOO_CHECK_X_NOT_ZERO_MAC(cuckooDbPtr->size_statistics[size-1]);

            cuckooDbPtr->bank_statistics[bank+offset].count--;
            cuckooDbPtr->bank_statistics[bank+offset].sizes[size-1]--;
            cuckooDbPtr->size_statistics[size-1]--;
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
            {
                cpssOsPrintf("** decrement bank [%d] count [%d]\n",bank+offset,cuckooDbPtr->bank_statistics[bank+offset].count);
            }
        }
    }

    /* sort populated banks after log insertion */
    rc = prvCpssDxChCuckooSortPopulatedBanks(clientType,managerId);

    return rc;
}

/**
* @internal prvCpssDxChCuckooDbEntryFirstBankGet function
* @endinternal
*
* @brief Get entry's first bank.
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] size            - number of banks
*
* @param[out] firstBankPtr      - (pointer to) entry's first bank
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_FOUND             - on not found parameters
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
static GT_STATUS prvCpssDxChCuckooDbEntryFirstBankGet
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  bank,
    IN  GT_U32                                  line,
    IN  GT_U32                                  size,
    OUT GT_U32                                  *firstBankPtr
)
{
    GT_U32 free_count, back_index;
    GT_STATUS rc;
    GT_BOOL found_first;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_BANK_STEP_ID_CHECK_MAC(clientType,size);
    CPSS_NULL_PTR_CHECK_MAC(firstBankPtr);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    /* check we did not overreach banks */
    if ( bank >= cuckooDbPtr->db_banks)
    {
        *firstBankPtr = cuckooDbPtr->db_banks;
        return GT_OK;
    }

    /* count number of adjacent free banks */
    rc = prvCpssDxChCuckooDbCountFreeGet(clientType,managerId,bank,line,size,&free_count);
    if (free_count)
    {
        if (free_count == size)
        {
            *firstBankPtr = (bank + free_count + 1);
            return GT_OK;
        }
        *firstBankPtr = (bank + free_count);
        return GT_OK;
    }
    /* bank start with free - first start after free */
    else
    {
        /* check if bank is the first */
        rc = prvCpssDxChCuckooDbLogIsFirst(clientType,managerId,bank,line,&found_first);
        if (rc != GT_OK && rc != GT_NOT_FOUND)
        {
            return rc;
        }
        /* entry doesn't exist in log; search in db */
        if (rc == GT_NOT_FOUND)
        {
            rc = prvCpssDxChCuckooDbIsFirst(clientType,managerId,bank,line,&found_first);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        if (found_first)
        {
            *firstBankPtr = bank;
            return GT_OK;
        }
        /* entry begin before bank */
        else
        {
            /* go backwords untill finding first bank */
            for (back_index = PRV_CPSS_DXCH_CUCKOO_MIN_BANK_STEP_CNS; back_index < PRV_CPSS_DXCH_CUCKOO_MAX_BANK_STEP_CNS; back_index++)
            {
                /* check if bank-back_index is the first */
                rc = prvCpssDxChCuckooDbLogIsFirst(clientType,managerId,bank-back_index,line,&found_first);
                if (rc != GT_OK && rc != GT_NOT_FOUND)
                {
                    return rc;
                }
                /* entry doesn't exist in log; search in db */
                if (rc == GT_NOT_FOUND)
                {
                    rc = prvCpssDxChCuckooDbIsFirst(clientType,managerId,bank-back_index,line,&found_first);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
                if (found_first)
                {
                    *firstBankPtr = (bank - back_index);
                    return GT_OK;
                }
            }
            *firstBankPtr = (bank - back_index);
            return GT_OK;
        }
    }
}

/**
* @internal prvCpssDxChCuckooDbLogInfoClear function
* @endinternal
*
* @brief Clear entry in operation log in location [bank][line].
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] bank            - bank number
* @param[in] line            - line number
* @param[in] sizeInBytes     - data size in bytes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
static GT_STATUS prvCpssDxChCuckooDbLogInfoClear
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN GT_U32                                   bank,
    IN GT_U32                                   line,
    IN GT_U32                                   sizeInBytes
)
{
    GT_U32 i,size,n;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,line);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,sizeInBytes);

    PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(sizeInBytes,size);
    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    for (i = 0; i < size; i++)
    {
        PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,bank+i);

        /* insert free entry info into log */
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].bank = bank+i;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].line = line;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].sizeInBytes = sizeInBytes;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].isFirst = GT_FALSE;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].isLock = GT_FALSE;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].isFree = GT_TRUE;
        cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].stage = 0;
        cpssOsMemSet(cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations].data,0,sizeof(GT_U8)*PRV_CPSS_DXCH_CUCKOO_MAX_KEY_SIZE_CNS);

        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("Added Clear Info into operation log:\n");
            CUCKOO_PRINT_LOG_ENTRY(cuckooDbPtr->log_array[cuckooDbPtr->numOfOperations],cuckooDbPtr->numOfOperations);
        }

        /* coherency check -- can't be ZERO because we are going to decrement it */
        CUCKOO_CHECK_X_NOT_ZERO_MAC(cuckooDbPtr->bank_statistics[bank+i].count);
        CUCKOO_CHECK_X_NOT_ZERO_MAC(cuckooDbPtr->bank_statistics[bank+i].sizes[size-1]);
        CUCKOO_CHECK_X_NOT_ZERO_MAC(cuckooDbPtr->size_statistics[size-1]);

        cuckooDbPtr->numOfOperations++;
        cuckooDbPtr->bank_statistics[bank+i].count--;
        cuckooDbPtr->bank_statistics[bank+i].sizes[size-1]--;
        cuckooDbPtr->size_statistics[size-1]--;
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("** decrement bank [%d] count [%d]\n",bank+i,cuckooDbPtr->bank_statistics[bank+i].count);
        }
    }

    return GT_OK;

}

/**
* @internal prvCpssDxChCuckooDbClientHashPrepare function
* @endinternal
*
* @brief Prepare hash array results per client in cuckoo format.
*
* @param[in] clientType      - client type
* @param[in] managerId       - manager identification
* @param[in] numHashArr      - number of hash array results
* @param[in] crcMultiHashArr - (pointer to) hash array results
*
* @param[out] hashArr        - (pointer to) hash array results
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_SUPPORTED         - on not supported parameter
* @retval GT_BAD_PTR               - on NULL pointer value
*
*/
static GT_STATUS prvCpssDxChCuckooDbClientHashPrepare
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN  GT_U32                                  numHashArr,
    IN  GT_U32                                  crcMultiHashArr[],
    IN  GT_U32                                  sizeInBytes,
    OUT PRV_CPSS_DXCH_CUCKOO_DB_HASH_STC        hashArr[]
)
{
    GT_U32 shift,i, numOfHashes,size,bank;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,sizeInBytes);
    CPSS_NULL_PTR_CHECK_MAC(crcMultiHashArr);
    CPSS_NULL_PTR_CHECK_MAC(hashArr);

    /* prepare hash array */
    switch (clientType) {
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:

        EM_MANAGER_ID_CHECK(managerId);
        exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);
        numOfHashes = exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes;

        break;
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E:

        FDB_MANAGER_ID_CHECK(managerId);
        fdbManagerPtr = MANAGER_GET_MAC(managerId);
        numOfHashes = fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes;

        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"Error: client is not supported in cuckoo algorithm");
    }

    if (numOfHashes == 4)
    {
        shift = 2;
    }
    else if (numOfHashes == 8)
    {
        shift = 3;
    }
    else
    {
        shift = 4;
    }

    PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(sizeInBytes,size);

    /* cuckoo hash array holds only heads (first banks) */
    for (i=0, bank=0; i<numHashArr/size; i++, bank +=size)
    {
        hashArr[i].line = (crcMultiHashArr[bank] >> shift);
        /* validate line */
        PRV_CPSS_DXCH_CUCKOO_DB_LINE_ID_CHECK_MAC(clientType,managerId,hashArr[i].line);
        hashArr[i].bank = bank;
        /* validate bank */
        PRV_CPSS_DXCH_CUCKOO_DB_BANK_ID_CHECK_MAC(clientType,managerId,hashArr[i].bank);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChCuckooDbClientHashArrGet function
* @endinternal
*
* @brief Get hash array results per client.
*
* @param[in] clientType      - client type
* @param[in] managerId            - manager identification
* @param[in] dataPtr         - key array in bytes
* @param[in] sizeInBytes     - key size in bytes
*
* @param[out] numHashArrPtr  - (pointer to) number of hash array results
* @param[out] hashArr        - (pointer to) hash array results
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
* @retval GT_NOT_SUPPORTED         - on not supported parameter
*
*/
static GT_STATUS prvCpssDxChCuckooDbClientHashArrGet
(
    IN  PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT        clientType,
    IN  GT_U32                                  managerId,
    IN  GT_U8                                   *dataPtr,
    IN  GT_U32                                  sizeInBytes,
    OUT GT_U32                                  *numHashArrPtr,
    OUT PRV_CPSS_DXCH_CUCKOO_DB_HASH_STC        hashArr[]
)
{
    GT_STATUS rc;
    GT_U32 i,size=0;
    GT_U32 crcMultiHashArr[PRV_CPSS_DXCH_CUCKOO_MAX_BANKS_NUM_CNS];

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(clientType,sizeInBytes);
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);
    CPSS_NULL_PTR_CHECK_MAC(numHashArrPtr);
    CPSS_NULL_PTR_CHECK_MAC(hashArr);

    /* calc hash array */
    switch (clientType) {
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:

        switch (sizeInBytes)
        {
        case 5:
            rc = prvCpssDxChCuckooHashFuncBind(clientType,managerId,sizeInBytes,dataPtr[0],dataPtr[1],dataPtr[2],dataPtr[3],dataPtr[4],numHashArrPtr,crcMultiHashArr);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case 19:
            rc = prvCpssDxChCuckooHashFuncBind(clientType,managerId,sizeInBytes,dataPtr[0],dataPtr[1],dataPtr[2],dataPtr[3],dataPtr[4],dataPtr[5],
                                               dataPtr[6],dataPtr[7],dataPtr[8],dataPtr[9],dataPtr[10],dataPtr[11],dataPtr[12],dataPtr[13],dataPtr[14],
                                               dataPtr[15],dataPtr[16],dataPtr[17],dataPtr[18],numHashArrPtr,crcMultiHashArr);
            if (rc != GT_OK)
            {
                return rc;
            }
            break;
        case 33:
            rc = prvCpssDxChCuckooHashFuncBind(clientType,managerId,sizeInBytes,dataPtr[0],dataPtr[1],dataPtr[2],dataPtr[3],dataPtr[4],dataPtr[5],
                                               dataPtr[6],dataPtr[7],dataPtr[8],dataPtr[9],dataPtr[10],dataPtr[11],dataPtr[12],dataPtr[13],dataPtr[14],
                                               dataPtr[15],dataPtr[16],dataPtr[17],dataPtr[18],dataPtr[19],dataPtr[20],dataPtr[21],dataPtr[22],
                                               dataPtr[23],dataPtr[24],dataPtr[25],dataPtr[26],dataPtr[27],dataPtr[28],dataPtr[29],dataPtr[30],
                                               dataPtr[31],dataPtr[32],numHashArrPtr,crcMultiHashArr);
            break;
        case 47:
            rc = prvCpssDxChCuckooHashFuncBind(clientType,managerId,sizeInBytes,dataPtr[0],dataPtr[1],dataPtr[2],dataPtr[3],dataPtr[4],dataPtr[5],
                                               dataPtr[6],dataPtr[7],dataPtr[8],dataPtr[9],dataPtr[10],dataPtr[11],dataPtr[12],dataPtr[13],dataPtr[14],
                                               dataPtr[15],dataPtr[16],dataPtr[17],dataPtr[18],dataPtr[19],dataPtr[20],dataPtr[21],dataPtr[22],
                                               dataPtr[23],dataPtr[24],dataPtr[25],dataPtr[26],dataPtr[27],dataPtr[28],dataPtr[29],dataPtr[30],
                                               dataPtr[31],dataPtr[32],dataPtr[33],dataPtr[34],dataPtr[35],dataPtr[36],dataPtr[37],dataPtr[38],
                                               dataPtr[39],dataPtr[40],dataPtr[41],dataPtr[42],dataPtr[43],dataPtr[44],dataPtr[45],dataPtr[46],
                                               numHashArrPtr,crcMultiHashArr);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(sizeInBytes);
        }

        rc = prvCpssDxChCuckooDbClientHashPrepare(clientType,managerId,*numHashArrPtr,crcMultiHashArr,sizeInBytes,hashArr);
        if (rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(sizeInBytes,size);
        *numHashArrPtr = *numHashArrPtr/size;
        break;
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E:
        /*rc = prvCpssDxChCuckooHashFuncBind(cuckooDbPtr->client_type,dataPtr[0],dataPtr[1],dataPtr[2],dataPtr[3],dataPtr[4],dataPtr[5],crcMultiHashArr);
        if (rc != GT_OK)
        {
            return rc;
        }*/ /* TBD */

        /*rc = prvCpssDxChCuckooDbClientHashPrepare(0,crcMultiHashArr,sizeInBytes,hashArr);
        if (rc != GT_OK)
        {
            return rc;
        }  TBD */

        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"Error: client is not supported in cuckoo algorithm");
    }

    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
    {
        cpssOsPrintf("\nnum hash array %d: ",*numHashArrPtr);
        cpssOsPrintf("\nhashArr:\n");
        for (i=0; i<*numHashArrPtr; i++)
        {
            cpssOsPrintf("%d. bank [%d] line [%d]\n",i+1,hashArr[i].bank,hashArr[i].line);
        }
        cpssOsPrintf("\ncrcMultiHashArr:\n");
        for (i=0; i<(*numHashArrPtr*size); i++)
        {
            cpssOsPrintf("%d. %d\n",i+1,crcMultiHashArr[i]);
        }
    }

    return GT_OK;
}

/*
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
)
{
    GT_U32 bank,line,i,n;
    PRV_CPSS_DXCH_CUCKOO_DB_HASH_STC hash_result_array[PRV_CPSS_DXCH_CUCKOO_MAX_BANKS_NUM_CNS];
    GT_U32 num_of_hash_results;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    PRV_CPSS_DXCH_CUCKOO_DB_SIZE_IN_BYTES_ID_CHECK_MAC(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,sizeInBytes);
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);
    CPSS_NULL_PTR_CHECK_MAC(isFoundPtr);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    /* calc hash array */
    rc = prvCpssDxChCuckooDbClientHashArrGet(clientType,managerId,dataPtr,sizeInBytes,&num_of_hash_results,hash_result_array);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (i=0; i<num_of_hash_results; i++)
    {
        bank = hash_result_array[i].bank;
        line = hash_result_array[i].line;
        if (cpssOsMemCmp(cuckooDbPtr->entries_db[bank][line].data,dataPtr,sizeof(GT_U8)*sizeInBytes) == 0 &&
            cuckooDbPtr->entries_db[bank][line].sizeInBytes == sizeInBytes)
        {
            *isFoundPtr = GT_TRUE;
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
            {
                CUCKOO_PRINT_KEY(cuckooDbPtr->entries_db[bank][line].data,sizeInBytes);
                cpssOsPrintf("is found in DB\n");
            }
            return GT_OK;
        }

    }

    *isFoundPtr = GT_FALSE;
    return GT_OK;
}

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
)
{
    GT_STATUS rc,recursive_ret=GT_FALSE;
    GT_U32 num_of_hash_results, hash_index;
    GT_U32 ibank, revers_bank;
    GT_U32 min_depth,depth;
    GT_U32 bank,line,size,free_count,n;
    GT_BOOL is_lock;
    PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC entryInfoCurrent;
    PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC entryInfoNew;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    PRV_CPSS_DXCH_CUCKOO_DB_HASH_STC hash_result_array[PRV_CPSS_DXCH_CUCKOO_MAX_BANKS_NUM_CNS];

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    CPSS_NULL_PTR_CHECK_MAC(entryInfoPtr);

    /* Copy current entry's info for next recursion step */
    entryInfoCurrent = *entryInfoPtr;

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    /* calc max depth for the entry to move */
    rc = prvCpssDxChCuckooDbMaxDepthGet(clientType,managerId,entryInfoCurrent.sizeInBytes,&depth);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* do min between new_depth and current depth */
    min_depth = (entryInfoCurrent.depth >= depth) ? depth : entryInfoCurrent.depth;

    /* Get hash array according to client */
    rc = prvCpssDxChCuckooDbClientHashArrGet(clientType,managerId,entryInfoCurrent.data,entryInfoCurrent.sizeInBytes,&num_of_hash_results,hash_result_array);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* check all hash calc first before diving into next depth; sort according to most populated banks */
    for (ibank = 0; ibank < cuckooDbPtr->db_banks; ibank ++) {
        revers_bank = ibank;

        for (hash_index = 0; hash_index < num_of_hash_results; hash_index++)
        {
            /* try to move to new bank according to most populated bank */
            if (hash_result_array[hash_index].bank != cuckooDbPtr->sorted_banks[revers_bank]) {
                continue;
            }

            /* skip my bank */
            if ( entryInfoCurrent.bank == hash_result_array[hash_index].bank)
            {
                continue;
            }

            bank = hash_result_array[hash_index].bank;
            line = hash_result_array[hash_index].line;
            PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(entryInfoCurrent.sizeInBytes,size);

            /* Check if [bank][line] for number of banks is locked in DB */
            rc = prvCpssDxChCuckooDbIsLock(clientType,managerId,bank,line,size,&is_lock);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* if location is locked we can't move it; continue to next hash_index */
            if (is_lock == GT_TRUE)
            {
                continue;
            }

            /* count number of adjacent free banks */
            rc = prvCpssDxChCuckooDbCountFreeGet(clientType, managerId, bank, line, size, &free_count);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (free_count >= size)
            {
                if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
                {
                    cpssOsPrintf("*************3 insert to empty db[%d][%d]:\n",bank,line);
                    CUCKOO_PRINT_KEY(entryInfoCurrent.data,entryInfoCurrent.sizeInBytes);
                }

                /* found empty space; update log */
                rc = prvCpssDxChCuckooDbLogInfoSet(clientType, managerId, bank, line, entryInfoCurrent.sizeInBytes, entryInfoCurrent.data);
                if (rc != GT_OK)
                {
                    return rc;
                }
                return GT_OK;
            }
        }
    }

    /* for entries try all new hash locations untill moved or failed all */
    for (ibank = 0; ibank < cuckooDbPtr->db_banks; ibank++) {
        revers_bank = ibank;

        for (hash_index = 0; hash_index < num_of_hash_results; hash_index++)
        {
            /* try to move to new bank according to most populated bank */
            if (hash_result_array[hash_index].bank != cuckooDbPtr->sorted_banks[revers_bank]) {
                continue;
            }

            /* skip my bank */
            if ( entryInfoCurrent.bank == hash_result_array[hash_index].bank)
            {
                continue;
            }

            /* insert to hash location data in current location */
            /* prepare entryInfo */
            entryInfoNew.bank = hash_result_array[hash_index].bank;
            entryInfoNew.line = hash_result_array[hash_index].line;
            entryInfoNew.sizeInBytes = entryInfoCurrent.sizeInBytes;
            cpssOsMemCpy(entryInfoNew.data,entryInfoCurrent.data,sizeof(GT_U8)*entryInfoCurrent.sizeInBytes);
            entryInfoNew.depth = min_depth;

            recursive_ret = prvCpssDxChCuckooDbRecursiveEntryAdd(clientType,managerId,&entryInfoNew);
            /* moved the entry */
            if (recursive_ret == GT_OK)
            {
                return GT_OK;
            }
        }
    }

    /* failed to set */
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssDxChCuckooDbRoolbackLogArray function
* @endinternal
*
* @brief   Delete all log operations from startIndex untill the
*          end due to failure in moving last entries.
*
* @param[in] clientType           - client type
* @param[in] managerId            - manager identification
* @param[in] startIndex           - start location in log operations.
* @param[in] updateCounters       - whether to update counters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong value in any of the parameters
*
*/
static GT_STATUS prvCpssDxChCuckooDbRoolbackLogArray
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN GT_U32                                   startIndex,
    IN GT_BOOL                                  updateCounters
)
{
    GT_U32 i;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    GT_U32 size;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    if (startIndex >= cuckooDbPtr->numOfOperations)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Error: startIndex should be less then cuckooDB.numOfOperations [%d]\n",cuckooDbPtr->numOfOperations);

    if (updateCounters)
    {
        /* update statistics before delete */
        for (i=startIndex; i<cuckooDbPtr->numOfOperations; i++)
        {
            PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(cuckooDbPtr->log_array[i].sizeInBytes,size);
            if (cuckooDbPtr->log_array[i].isFree == GT_FALSE)
            {
                /* coherency check -- can't be ZERO because we are going to decrement it */
                CUCKOO_CHECK_X_NOT_ZERO_MAC(cuckooDbPtr->bank_statistics[cuckooDbPtr->log_array[i].bank].count);
                CUCKOO_CHECK_X_NOT_ZERO_MAC(cuckooDbPtr->bank_statistics[cuckooDbPtr->log_array[i].bank].sizes[size-1]);
                CUCKOO_CHECK_X_NOT_ZERO_MAC(cuckooDbPtr->size_statistics[size-1]);

                cuckooDbPtr->bank_statistics[cuckooDbPtr->log_array[i].bank].count--;
                cuckooDbPtr->bank_statistics[cuckooDbPtr->log_array[i].bank].sizes[size-1]--;
                cuckooDbPtr->size_statistics[size-1]--;
                if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
                {
                    cpssOsPrintf("** decrement bank [%d] count [%d]\n",cuckooDbPtr->log_array[i].bank,cuckooDbPtr->bank_statistics[cuckooDbPtr->log_array[i].bank].count);
                }
            }
            else
            {
                cuckooDbPtr->bank_statistics[cuckooDbPtr->log_array[i].bank].count++;
                cuckooDbPtr->bank_statistics[cuckooDbPtr->log_array[i].bank].sizes[size-1]++;
                cuckooDbPtr->size_statistics[size-1]++;
                if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
                {
                    cpssOsPrintf("** increment bank [%d] count [%d]\n",cuckooDbPtr->log_array[i].bank,cuckooDbPtr->bank_statistics[cuckooDbPtr->log_array[i].bank].count);
                }
            }
        }
    }

    for (i=startIndex; i<cuckooDbPtr->numOfOperations; i++)
    {
        cuckooDbPtr->log_array[i].bank = 0;
        cuckooDbPtr->log_array[i].line = 0;
        cpssOsMemSet(cuckooDbPtr->log_array[i].data, 0, sizeof(GT_U8)*PRV_CPSS_DXCH_CUCKOO_MAX_KEY_SIZE_CNS);
        cuckooDbPtr->log_array[i].isFirst = 0;
        cuckooDbPtr->log_array[i].sizeInBytes = 0;
        cuckooDbPtr->log_array[i].isFree = GT_TRUE;
        cuckooDbPtr->log_array[i].stage = 0;
    }
    cuckooDbPtr->numOfOperations =  startIndex;
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug2Print))
    {
        cpssOsPrintf("cleared operation log from index [%d]\n",startIndex);
    }

    return GT_OK;
}

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
)
{
    return prvCpssDxChCuckooDbRoolbackLogArray(clientType,managerId,0,GT_FALSE);
}

/*
* @internal prvCpssDxChCuckooDbRecursiveEntryAdd function
* @endinternal
*
* @brief Add new entry in calculated hash index. If location
*        is occupied try to move all entries from this location.
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
static GT_STATUS prvCpssDxChCuckooDbRecursiveEntryAdd
(
    IN PRV_CPSS_DXCH_CUCKOO_CLIENTS_ENT         clientType,
    IN GT_U32                                   managerId,
    IN PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC   *entryInfoPtr
)
{
    GT_STATUS rc,recursive_ret;
    GT_U32 entry_first_bank,entries_index, log_current_index,n;
    GT_U32 free_count; /* number of adjacent free banks */
    GT_U32 size;       /* number of banks */
    GT_U32 size_to_replaceInBytes;
    GT_U32 entry_size=0, entry_sizeInBytes=0;
    GT_U32 lock_size, lock_start, lock_end, bank_check_end; /* parameters for locking entries */
    GT_U8 entry_data[PRV_CPSS_DXCH_CUCKOO_MAX_KEY_SIZE_CNS];
    GT_BOOL is_lock;
    PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC entryInfoCurrent;
    PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC entryInfoNew;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    typedef struct
    {
        GT_U32 size;        /* number of banks data occupy */
        GT_U32 sizeInBytes; /* number of bytes data occupy */
        GT_U32 bank;
    }entry_stc;
    GT_U32 entries_max;     /*number of entries to move in first_entries array */
    entry_stc first_entries[PRV_CPSS_DXCH_CUCKOO_MAX_BANKS_NUM_CNS]; /* array of entries to move */

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);
    CPSS_NULL_PTR_CHECK_MAC(entryInfoPtr);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    /* Copy current entry's info for next recursion step */
    entryInfoCurrent = *entryInfoPtr;

    PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(entryInfoCurrent.sizeInBytes,size);

    /* if location is free insert - first recursive stop */
    cuckooDbPtr->recurse_count++;

    /* Check if [bank][line] for number of banks is locked in DB */
    rc = prvCpssDxChCuckooDbIsLock(clientType,managerId,entryInfoCurrent.bank,entryInfoCurrent.line,size,&is_lock);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* if location is locked we can't move it; return fail */
    if (is_lock == GT_TRUE)
    {
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("ERROR entry is locked  bank [%d] line [%d] depth [%d]\n",entryInfoCurrent.bank,entryInfoCurrent.line,entryInfoCurrent.depth);
            CUCKOO_PRINT_KEY(entryInfoCurrent.data,entryInfoCurrent.sizeInBytes);
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* count number of adjacent free banks */
    rc = prvCpssDxChCuckooDbCountFreeGet(clientType,managerId,entryInfoCurrent.bank, entryInfoCurrent.line, size, &free_count);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* found adjacent free banks sufficient for current entry */
    if (free_count >= size)
    {
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("*************1 insert to empty db[%d][%d]:\n",entryInfoCurrent.bank,entryInfoCurrent.line);
            CUCKOO_PRINT_KEY(entryInfoCurrent.data,entryInfoCurrent.sizeInBytes);
        }

        /* found empty space; update log */
        rc = prvCpssDxChCuckooDbLogInfoSet(clientType,managerId,entryInfoCurrent.bank, entryInfoCurrent.line, entryInfoCurrent.sizeInBytes, entryInfoCurrent.data);
        if (rc != GT_OK)
        {
            return rc;
        }
        return GT_OK;
    }

    /* if depth is 0 - second recursive stop */
    CUCKOO_CHECK_X_NOT_ZERO_MAC(entryInfoCurrent.depth);
    entryInfoCurrent.depth--;
    if (entryInfoCurrent.depth <= 0)
    {
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("ERROR entry reached max depth  bank [%d] line [%d] depth [%d]\n",entryInfoCurrent.bank,entryInfoCurrent.line,entryInfoCurrent.depth);
            CUCKOO_PRINT_KEY(entryInfoCurrent.data,entryInfoCurrent.sizeInBytes);
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* find entry's first bank  */
    rc = prvCpssDxChCuckooDbEntryFirstBankGet(clientType,managerId,entryInfoCurrent.bank, entryInfoCurrent.line, size, &entry_first_bank);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* set lock boundaries */
    lock_start = (entry_first_bank < entryInfoCurrent.bank) ? entry_first_bank : entryInfoCurrent.bank;
    /* check that lock end did not over reach max banks */
    bank_check_end = lock_end = (entryInfoCurrent.bank + size >= cuckooDbPtr->db_banks) ? cuckooDbPtr->db_banks : entryInfoCurrent.bank + size;

    /* zero entries array */
    cpssOsMemSet(first_entries,0,sizeof(entry_stc)*PRV_CPSS_DXCH_CUCKOO_MAX_BANKS_NUM_CNS);

    /* create array of entries to change location from lock_start untill bank_check_end */
    for (entries_max = 0; entry_first_bank < bank_check_end; entries_max++)
    {
        rc = prvCpssDxChCuckooDbLogSizeGet(clientType, managerId,entry_first_bank, entryInfoCurrent.line, &entry_sizeInBytes);
        if (rc != GT_OK && rc != GT_NOT_FOUND)
        {
            return rc;
        }
        /* [entry_first_bank][line] doesn't exist in operation log; look in DB */
        if (rc == GT_NOT_FOUND) {
            rc = prvCpssDxChCuckooDbSizeGet(clientType, managerId, entry_first_bank, entryInfoCurrent.line, &entry_sizeInBytes);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* dont move locked entries */
        PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(entry_sizeInBytes,entry_size);
        rc = prvCpssDxChCuckooDbIsLock(clientType,managerId,entry_first_bank,entryInfoCurrent.line,entry_size,&is_lock);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (is_lock == GT_TRUE)
        {
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
            {
                cpssOsPrintf("ERROR entry is locked  bank [%d] line [%d] depth [%d]\n",entry_first_bank,entryInfoCurrent.line,entryInfoCurrent.depth);
            }
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        first_entries[entries_max].bank = entry_first_bank;
        first_entries[entries_max].size = entry_size;
        first_entries[entries_max].sizeInBytes = entry_sizeInBytes;

        /* update last */
        if (entry_size + entry_first_bank > lock_end)
        {
            lock_end = entry_size + entry_first_bank;
        }

        /* find entry inside array it's first bank  */
        rc = prvCpssDxChCuckooDbEntryFirstBankGet(clientType,managerId,entry_first_bank + entry_size, entryInfoCurrent.line, size, &entry_first_bank);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* set lock */
    lock_size = lock_end - lock_start;

    /* lock [lock_start][line] for lock_size inorder not to have loops in recurse */
    rc = prvCpssDxChCuckooDbSetLock(clientType,managerId, lock_start, entryInfoCurrent.line, lock_size, GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
    {
        cpssOsPrintf("locked myself bank [%d] line [%d] lock size [%d]\n",lock_start,entryInfoCurrent.line,lock_size);
    }

    /* try to move all entries; save log current index incase of failure for roolback */
    log_current_index = cuckooDbPtr->numOfOperations;
    for (entries_index = 0; entries_index < entries_max; entries_index++)
    {
        size_to_replaceInBytes = first_entries[entries_index].sizeInBytes;

        /* data of entry to move */
        rc = prvCpssDxChCuckooDbLogDataGet(clientType,managerId,first_entries[entries_index].bank, entryInfoCurrent.line, size_to_replaceInBytes, entry_data);
        if (rc != GT_OK && rc != GT_NOT_FOUND)
        {
            return rc;
        }
        /* [first_entries[entries_index].bank][line] doesn't exist in log; look in DB */
        if (rc == GT_NOT_FOUND) {
            rc = prvCpssDxChCuckooDbDataGet(clientType,managerId,first_entries[entries_index].bank, entryInfoCurrent.line, size_to_replaceInBytes, entry_data);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        /* for entries try all new hash locations untill move succeeded or tried and failed all */
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("try to move from db[%d][%d]:\n",first_entries[entries_index].bank,entryInfoCurrent.line);
            CUCKOO_PRINT_KEY(entry_data,size_to_replaceInBytes);
        }

        /* prepare entryInfoNew */
        entryInfoNew.bank = entryInfoCurrent.bank;
        entryInfoNew.sizeInBytes = size_to_replaceInBytes;
        cpssOsMemCpy(entryInfoNew.data,entry_data,sizeof(GT_U8)*size_to_replaceInBytes);
        entryInfoNew.depth = entryInfoCurrent.depth;
        entryInfoNew.line = entryInfoCurrent.line;

        /* try to move entryInfoNew */
        recursive_ret = prvCpssDxChCuckooDbRehashEntry(clientType,managerId,&entryInfoNew);
        /* failed to move - stop and return false */
        if (recursive_ret == GT_FAIL)
        {
            /* unlock [lock_start][line] for lock_size */
            rc = prvCpssDxChCuckooDbSetLock(clientType,managerId,lock_start, entryInfoCurrent.line, lock_size, GT_FALSE);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
            {
                cpssOsPrintf("failed to move from db[%d][%d]:\n",first_entries[entries_index].bank,entryInfoCurrent.line);
                CUCKOO_PRINT_KEY(entry_data,size_to_replaceInBytes);
                cpssOsPrintf("FAILURE unlocked myself bank [%d] line [%d] lock size [%d]\n",lock_start,entryInfoCurrent.line,lock_size);
            }

            /* roolback log operations */
            if (cuckooDbPtr->numOfOperations > log_current_index) {
                rc = prvCpssDxChCuckooDbRoolbackLogArray(clientType,managerId,log_current_index,GT_TRUE);
                if (rc != GT_OK)
                {
                    return rc;
                }
                if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
                {
                    cpssOsPrintf("reset log from %d to %d\n", cuckooDbPtr->numOfOperations, log_current_index);
                }
            }

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        /* move succeeded; mark entry as free in operation log */
        rc = prvCpssDxChCuckooDbLogInfoClear(clientType,managerId,first_entries[entries_index].bank, entryInfoCurrent.line, size_to_replaceInBytes);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            cpssOsPrintf("free location db[%d][%d] sizeInBytes [%d]\n",first_entries[entries_index].bank,entryInfoCurrent.line,size_to_replaceInBytes);
        }
    }

    /* moved all entries */
    /* unlock [lock_start][line] for lock_size */
    rc = prvCpssDxChCuckooDbSetLock(clientType,managerId,lock_start, entryInfoCurrent.line, lock_size, GT_FALSE);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
    {
        cpssOsPrintf("SUCCESS unlocked myself bank [%d] line [%d] lock size [%d]\n",lock_start,entryInfoCurrent.line,lock_size);
        cpssOsPrintf("*************2 insert after moving all entries db[%d][%d]:\n",entryInfoCurrent.bank,entryInfoCurrent.line);
        CUCKOO_PRINT_KEY(entryInfoCurrent.data,entryInfoCurrent.sizeInBytes);
    }

    /* set data in log */
    rc = prvCpssDxChCuckooDbLogInfoSet(clientType,managerId,entryInfoCurrent.bank, entryInfoCurrent.line, entryInfoCurrent.sizeInBytes, entryInfoCurrent.data);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

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
)
{
    GT_U32 i, line, prevLine,n;
    GT_U32 stageCounter=0;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    /* count max stage */
    for (i=0, prevLine=cuckooDbPtr->log_array[0].line; i<cuckooDbPtr->numOfOperations; i++)
    {
        line = cuckooDbPtr->log_array[i].line;
        /* don't count clear operations */
        if (cuckooDbPtr->log_array[i].isFree != GT_TRUE)
        {
            /* parts are on same stage */
            if (prevLine != line)
                stageCounter++;

            prevLine = line;
        }
    }

    /* update stage in log operations starting from the last element */
    for (i=0, prevLine=cuckooDbPtr->log_array[0].line; i<cuckooDbPtr->numOfOperations; i++)
    {
        line = cuckooDbPtr->log_array[i].line;
        /* don't count clear operations */
        if (cuckooDbPtr->log_array[i].isFree != GT_TRUE)
        {
            /* parts are on same stage */
            if (prevLine != line)
            {
                CUCKOO_CHECK_X_NOT_ZERO_MAC(stageCounter);
                stageCounter--;
            }

            prevLine = line;
        }
        cuckooDbPtr->log_array[i].stage = stageCounter;

        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            CUCKOO_PRINT_LOG_ENTRY(cuckooDbPtr->log_array[i],i);
        }
    }

    return GT_OK;
}

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
)
{
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    CPSS_NULL_PTR_CHECK_MAC(numOfOperationsPtr);
    CPSS_NULL_PTR_CHECK_MAC(cuckooDbLogPtr);
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    *cuckooDbLogPtr = cuckooDbPtr->log_array;
    *numOfOperationsPtr = cuckooDbPtr->numOfOperations;

    return GT_OK;
}

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
)
{
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;

    /* check parameters */
    CPSS_NULL_PTR_CHECK_MAC(cuckooDbBankStPtr);
    CPSS_NULL_PTR_CHECK_MAC(cuckooDbSizeStPtr);
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    *cuckooDbBankStPtr = cuckooDbPtr->bank_statistics;
    *cuckooDbSizeStPtr = cuckooDbPtr->size_statistics;

    return GT_OK;
}

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
)
{
    GT_U32 bank, line;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr=NULL;
    PRV_CPSS_DXCH_FDB_MANAGER_DB_INSTANCE_INFO_STC  *fdbManagerPtr=NULL;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr=NULL;
    GT_U32 tblSize;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    if (managerId >= PRV_CPSS_DXCH_CUCKOO_MAX_MANAGER_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error in manageId type [%d]",managerId);
    }

    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId] != NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: cuckooDB for clientType [%d] managerId [%d] already exist",clientType,managerId);
    }

    PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId] = (PRV_CPSS_DXCH_CUCKOO_DB_STC *)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_CUCKOO_DB_STC));
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId] == NULL)
    {
        goto exit_cleanly_lbl;
    }
    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    cpssOsMemSet(cuckooDbPtr,0,sizeof(PRV_CPSS_DXCH_CUCKOO_DB_STC));

    cpssOsMemSet(cuckooDbPtr->log_array,0,sizeof(PRV_CPSS_DXCH_CUCKOO_DB_LOG_ARRAY_STC)*PRV_CPSS_DXCH_CUCKOO_DB_MAX_LOG_CNS);
    cuckooDbPtr->numOfOperations = 0;
    cuckooDbPtr->recurse_count = 0;

    switch (clientType)
    {
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:

        EM_MANAGER_ID_CHECK(managerId);
        exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);
        cuckooDbPtr->db_banks = exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes;

        switch(exactMatchManagerPtr->cpssHashParams.size)
        {
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_8K_E:
                tblSize=_8KB;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_16K_E:
                tblSize=_16KB;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_32K_E:
                tblSize=_32KB;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_64K_E:
                tblSize=_64KB;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_128K_E:
                tblSize=_128KB;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_256K_E:
                tblSize=_256KB;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(exactMatchManagerPtr->cpssHashParams.size);
        }
        cuckooDbPtr->db_lines = tblSize/cuckooDbPtr->db_banks;
        break;

    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E:

        FDB_MANAGER_ID_CHECK(managerId);
        fdbManagerPtr = MANAGER_GET_MAC(managerId);
        cuckooDbPtr->db_banks = fdbManagerPtr->capacityInfo.hwCapacity.numOfHashes;

        switch(fdbManagerPtr->cpssHashParams.size)
        {
            case CPSS_DXCH_BRG_FDB_TBL_SIZE_8K_E:
                tblSize=_8KB;
                break;
            case CPSS_DXCH_BRG_FDB_TBL_SIZE_16K_E:
                tblSize=_16KB;
                break;
            case CPSS_DXCH_BRG_FDB_TBL_SIZE_32K_E:
                tblSize=_32KB;
                break;
            case CPSS_DXCH_BRG_FDB_TBL_SIZE_64K_E:
                tblSize=_64KB;
                break;
            case CPSS_DXCH_BRG_FDB_TBL_SIZE_128K_E:
                tblSize=_128KB;
                break;
            case CPSS_DXCH_BRG_FDB_TBL_SIZE_256K_E:
                tblSize=_256KB;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(fdbManagerPtr->cpssHashParams.size);
        }
        cuckooDbPtr->db_lines = tblSize/cuckooDbPtr->db_banks;
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(clientType);
    }

    /* alocate memory */
    cuckooDbPtr->entries_db = (PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_STC **)cpssOsMalloc(cuckooDbPtr->db_banks*sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_STC*));
    if (cuckooDbPtr->entries_db == NULL)
    {
        goto exit_cleanly_lbl;
    }
    for (bank = 0; bank < cuckooDbPtr->db_banks; bank++)
    {
         cuckooDbPtr->entries_db[bank] = (PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_STC *)cpssOsMalloc(cuckooDbPtr->db_lines*sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_STC));
         if (cuckooDbPtr->entries_db[bank] == NULL)
         {
             goto exit_cleanly_lbl;
         }
    }

    cuckooDbPtr->bank_statistics = (PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC *)cpssOsMalloc(cuckooDbPtr->db_banks*sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC));
    if (cuckooDbPtr->bank_statistics == NULL)
    {
        goto exit_cleanly_lbl;
    }

    cuckooDbPtr->sorted_banks = (GT_U32 *)cpssOsMalloc(cuckooDbPtr->db_banks*sizeof(GT_U32));
    if (cuckooDbPtr->sorted_banks == NULL)
    {
        goto exit_cleanly_lbl;
    }

    /* set default values */
    for (bank = 0; bank < cuckooDbPtr->db_banks; bank++)
    {
        for (line = 0; line < cuckooDbPtr->db_lines; line++)
        {
            cpssOsMemSet(&(cuckooDbPtr->entries_db[bank][line]),0,sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_STC));
            cuckooDbPtr->entries_db[bank][line].isFree = 1;
        }
        cpssOsMemSet(&(cuckooDbPtr->bank_statistics[bank]), 0, sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC));
        cuckooDbPtr->sorted_banks[bank] = bank;
    }

    return GT_OK;

exit_cleanly_lbl:

    CPSS_LOG_ERROR_MAC("Error: Failed to allocate memory for cuckoo DB\n");
    if (cuckooDbPtr)
    {
        if (cuckooDbPtr->entries_db)
        {
            for (bank=0; bank<cuckooDbPtr->db_banks; bank++)
            {
                if (cuckooDbPtr->entries_db[bank])
                {
                    cpssOsFree(cuckooDbPtr->entries_db[bank]);
                    cuckooDbPtr->entries_db[bank] = NULL;
                }
            }
            cpssOsFree(cuckooDbPtr->entries_db);
            cuckooDbPtr->entries_db = NULL;
        }
        if (cuckooDbPtr->bank_statistics)
        {
            cpssOsFree(cuckooDbPtr->bank_statistics);
            cuckooDbPtr->bank_statistics = NULL;
        }
        if (cuckooDbPtr->sorted_banks)
        {
            cpssOsFree(cuckooDbPtr->sorted_banks);
            cuckooDbPtr->sorted_banks = NULL;
        }
        cpssOsFree(cuckooDbPtr);
        cuckooDbPtr = NULL;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
}

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
)
{
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    GT_U32 bank;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);

    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId] == NULL)
    {
        return GT_OK;
    }

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    if (cuckooDbPtr)
    {
        if (cuckooDbPtr->entries_db)
        {
            for (bank=0; bank<cuckooDbPtr->db_banks; bank++)
            {
                if (cuckooDbPtr->entries_db[bank])
                {
                    cpssOsFree(cuckooDbPtr->entries_db[bank]);
                    cuckooDbPtr->entries_db[bank] = NULL;
                }
            }
            cpssOsFree(cuckooDbPtr->entries_db);
            cuckooDbPtr->entries_db = NULL;
        }
        if (cuckooDbPtr->bank_statistics)
        {
            cpssOsFree(cuckooDbPtr->bank_statistics);
            cuckooDbPtr->bank_statistics = NULL;
        }
        if (cuckooDbPtr->sorted_banks)
        {
            cpssOsFree(cuckooDbPtr->sorted_banks);
            cuckooDbPtr->sorted_banks = NULL;
        }
        cpssOsFree(PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId]);
        PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId] = NULL;
    }

    PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId] = NULL;

    return GT_OK;
}

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
)
{
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr;
    GT_U32 i,j,bank,line,count_banks=0;
    GT_BOOL is_free;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    for (line = 0; line < cuckooDbPtr->db_lines; line++)
    {
        for (bank = 0; bank < cuckooDbPtr->db_banks; bank++)
        {
            rc = prvCpssDxChCuckooDbIsFree(clientType,managerId,bank,line,&is_free);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (is_free == GT_FALSE)
            {
                count_banks++;
            }
        }
    }

    cpssOsPrintf("CuckooDB Statistics:\n\n");
    cpssOsPrintf("Capacity percents (%d%)\n",(count_banks*100)/(cuckooDbPtr->db_banks*cuckooDbPtr->db_lines));
    cpssOsPrintf("entry size ");
    for (i=0; i<PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_LAST_E; i++)
    {
        cpssOsPrintf("[%d]: %d ",i+1,cuckooDbPtr->size_statistics[i]);
    }
    cpssOsPrintf("\n\n");

    for (i=0; i<cuckooDbPtr->db_banks; i++)
    {
        cpssOsPrintf("bank [%d]: %d\n", i,cuckooDbPtr->bank_statistics[i].count);
        for (j=0; j<PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_LAST_E; j++)
        {
            cpssOsPrintf("\t size[%d] = %d\n",j+1,cuckooDbPtr->bank_statistics[i].sizes[j]);
        }
    }
    cpssOsPrintf("\n");

    cpssOsPrintf("sort bank [count]: ");
    for (i=0; i< cuckooDbPtr->db_banks; i++) {
        cpssOsPrintf(" %d [%d] ", cuckooDbPtr->sorted_banks[i], cuckooDbPtr->bank_statistics[cuckooDbPtr->sorted_banks[i]].count);
    }
    cpssOsPrintf("\n");

    return GT_OK;
}

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
)
{
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr=NULL;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr=NULL;
    GT_U32 i,bank,line;
    GT_U32 hwIndex,dbIndex;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC *entryPtr;
    GT_U32 sizeInBytes,size,step;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    /* set default values */
    for (bank = 0; bank < cuckooDbPtr->db_banks; bank++)
    {
        for (line = 0; line < cuckooDbPtr->db_lines; line++)
        {
            cpssOsMemSet(&(cuckooDbPtr->entries_db[bank][line]),0,sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_STC));
            cuckooDbPtr->entries_db[bank][line].isFree = 1;
        }
        cpssOsMemSet(&(cuckooDbPtr->bank_statistics[bank]), 0, sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC));
        cuckooDbPtr->sorted_banks[bank] = bank;
    }

    switch (clientType)
    {
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:

        EM_MANAGER_ID_CHECK(managerId);
        exactMatchManagerPtr = EM_MANAGER_GET_MAC(managerId);
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(clientType);
        break;
    }

    /* check all exact match entries defined in the DB */
    for(hwIndex = 0 ; hwIndex < exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes; /*hwIndex updated according to entry keySize */)
    {
        /* Validate used list content */
        if(exactMatchManagerPtr->indexArr[hwIndex].isValid_entryPointer)
        {
            dbIndex = exactMatchManagerPtr->indexArr[hwIndex].entryPointer;

            /* protect access to array out of range */
            EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex, exactMatchManagerPtr->capacityInfo.maxTotalEntries);

            entryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];

            /* convert Exact Match key enum to size in bytes */
            EM_CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(entryPtr->hwExactMatchKeySize,sizeInBytes);
            PRV_CPSS_DXCH_CUCKOO_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(sizeInBytes,size);

            /* calc bank from hwIndex */
            EM_BANK_FROM_HW_INDEX_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,hwIndex,bank);

            /* update statistics */
            for (i=0; i<size; i++)
            {
                cuckooDbPtr->bank_statistics[bank+i].count++;
                cuckooDbPtr->bank_statistics[bank+i].sizes[size-1]++;
                cuckooDbPtr->size_statistics[size-1]++;
            }

              /* jump to the next hwIndex to check */
            step = exactMatchManagerPtr->entryPoolPtr[dbIndex].hwExactMatchKeySize + 1;
            hwIndex = hwIndex+step;
        }
        else
        {
            /* else - In case of invalid entry dbIndex will not be valid to check */
            hwIndex++;
        }
    }

    return GT_OK;
}

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
)
{
    GT_U32 bank, line;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr=NULL;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);

    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId] != NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error: cuckooDB for clientType [%d] managerId [%d] already exist",clientType,managerId);
    }

    PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId] = (PRV_CPSS_DXCH_CUCKOO_DB_STC *)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_CUCKOO_DB_STC));
    if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(cuckooDB)[clientType][managerId] == NULL)
    {
        goto exit_cleanly_lbl;
    }
    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);
    cpssOsMemSet(cuckooDbPtr,0,sizeof(PRV_CPSS_DXCH_CUCKOO_DB_STC));

    cpssOsMemSet(cuckooDbPtr->log_array,0,sizeof(PRV_CPSS_DXCH_CUCKOO_DB_LOG_ARRAY_STC)*PRV_CPSS_DXCH_CUCKOO_DB_MAX_LOG_CNS);
    cuckooDbPtr->numOfOperations = 0;
    cuckooDbPtr->recurse_count = 0;

    switch (clientType)
    {
    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E:

        cuckooDbPtr->db_banks = db_banks;
        cuckooDbPtr->db_lines = db_lines;
        break;

    case PRV_CPSS_DXCH_CUCKOO_CLIENTS_FDB_E:

        cuckooDbPtr->db_banks = db_banks;
        cuckooDbPtr->db_lines = db_lines;
        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(clientType);
    }

    /* alocate memory */
    cuckooDbPtr->entries_db = (PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_STC **)cpssOsMalloc(cuckooDbPtr->db_banks*sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_STC*));
    if (cuckooDbPtr->entries_db == NULL)
    {
        goto exit_cleanly_lbl;
    }
    for (bank = 0; bank < cuckooDbPtr->db_banks; bank++)
    {
         cuckooDbPtr->entries_db[bank] = (PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_STC *)cpssOsMalloc(cuckooDbPtr->db_lines*sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_STC));
         if (cuckooDbPtr->entries_db[bank] == NULL)
         {
             goto exit_cleanly_lbl;
         }
    }

    cuckooDbPtr->bank_statistics = (PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC *)cpssOsMalloc(cuckooDbPtr->db_banks*sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC));
    if (cuckooDbPtr->bank_statistics == NULL)
    {
        goto exit_cleanly_lbl;
    }

    cuckooDbPtr->sorted_banks = (GT_U32 *)cpssOsMalloc(cuckooDbPtr->db_banks*sizeof(GT_U32));
    if (cuckooDbPtr->sorted_banks == NULL)
    {
        goto exit_cleanly_lbl;
    }

    /* set default values */
    for (bank = 0; bank < cuckooDbPtr->db_banks; bank++)
    {
        for (line = 0; line < cuckooDbPtr->db_lines; line++)
        {
            cpssOsMemSet(&(cuckooDbPtr->entries_db[bank][line]),0,sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_STC));
            cuckooDbPtr->entries_db[bank][line].isFree = 1;
        }
        cpssOsMemSet(&(cuckooDbPtr->bank_statistics[bank]), 0, sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC));
        cuckooDbPtr->sorted_banks[bank] = bank;
    }

    return GT_OK;

exit_cleanly_lbl:

    CPSS_LOG_ERROR_MAC("Error: Failed to allocate memory for cuckoo DB\n");
    if (cuckooDbPtr)
    {
        if (cuckooDbPtr->entries_db)
        {
            for (bank=0; bank<cuckooDbPtr->db_banks; bank++)
            {
                if (cuckooDbPtr->entries_db[bank])
                {
                    cpssOsFree(cuckooDbPtr->entries_db[bank]);
                    cuckooDbPtr->entries_db[bank] = NULL;
                }
            }
            cpssOsFree(cuckooDbPtr->entries_db);
            cuckooDbPtr->entries_db = NULL;
        }
        if (cuckooDbPtr->bank_statistics)
        {
            cpssOsFree(cuckooDbPtr->bank_statistics);
            cuckooDbPtr->bank_statistics = NULL;
        }
        if (cuckooDbPtr->sorted_banks)
        {
            cpssOsFree(cuckooDbPtr->sorted_banks);
            cuckooDbPtr->sorted_banks = NULL;
        }
        cpssOsFree(cuckooDbPtr);
        cuckooDbPtr = NULL;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
}

/**
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
)
{
    GT_U32 i, bank, line, n;
    PRV_CPSS_DXCH_CUCKOO_DB_STC *cuckooDbPtr=NULL;

    /* check parameters */
    PRV_CPSS_DXCH_CUCKOO_DB_CLIENT_TYPE_ID_CHECK_MAC(clientType);
    PRV_CPSS_DXCH_CUCKOO_DB_INIT_CHECK_MAC(clientType,managerId);

    cuckooDbPtr = CUCKOO_DB_GET_MAC(clientType,managerId);

    /* update all the log operations starting from the last element */
    for (i=0; i<cuckooDbPtr->numOfOperations; i++)
    {
        if (PRV_SHARED_CUCKOO_DIR_CUCKOO_SRC_GLOBAL_VAR_GET(debug1Print))
        {
            CUCKOO_PRINT_LOG_ENTRY(cuckooDbPtr->log_array[i],i);
        }
        bank = cuckooDbPtr->log_array[i].bank;
        line = cuckooDbPtr->log_array[i].line;

        cpssOsMemCpy(cuckooDbPtr->entries_db[bank][line].data, cuckooDbPtr->log_array[i].data, sizeof(GT_U8)*cuckooDbPtr->log_array[i].sizeInBytes);
        cuckooDbPtr->entries_db[bank][line].isFirst = cuckooDbPtr->log_array[i].isFirst;
        cuckooDbPtr->entries_db[bank][line].sizeInBytes = cuckooDbPtr->log_array[i].sizeInBytes;
        cuckooDbPtr->entries_db[bank][line].isFree = cuckooDbPtr->log_array[i].isFree;
        cuckooDbPtr->entries_db[bank][line].isLock = cuckooDbPtr->log_array[i].isLock;
    }

    return GT_OK;
}
