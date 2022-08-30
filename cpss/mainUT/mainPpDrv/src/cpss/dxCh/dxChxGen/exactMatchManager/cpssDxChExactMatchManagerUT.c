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
* @file cpssDxChExactMatchManagerUT.c
*
* @brief Unit tests for cpssDxChExactMatchManager.c/h, that provides
* 'Exact Match manager' CPSS DxCh facility implementation.
*
* @version   1
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/exactMatchManager/cpssDxChExactMatchManager.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/cpssDxChExactMatchManagerTypes.h>

/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_utils.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_db.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_debug.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_hw.h>
#include <cpss/dxCh/dxChxGen/cuckoo/prvCpssDxChCuckoo.h>
#include <cpss/dxCh/dxChxGen/exactMatch/private/prvCpssDxChExactMatchHash.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiagDataIntegrityTables.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfExactMatchGen.h>
#include <time.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*#include <extUtils/tgf/tgfBridgeGen.h>*/

/* use Exact Match size that match the fineTunning */
#define EXACT_MATCH_SIZE_FULL (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->fineTuning.tableSize.emNum)
#define EXACT_MATCH_MAX_BANKS (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->exactMatchInfo.exactMatchNumOfBanks)

/* macro to convert Exact Match key enum to size in bytes */
#define CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(_keyEnum,_sizeInBytes)                   \
    switch (_keyEnum)                                                               \
    {                                                                               \
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:                                       \
        _sizeInBytes = 5;                                                           \
        break;                                                                      \
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:                                      \
        _sizeInBytes = 19;                                                          \
        break;                                                                      \
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:                                      \
        _sizeInBytes = 33;                                                          \
        break;                                                                      \
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:                                      \
        _sizeInBytes = 47;                                                          \
        break;                                                                      \
    default:                                                                        \
         UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_BAD_PARAM,"bad key enumeration [%d]",_keyEnum);\
    }

extern GT_VOID prvSetExactMatchActionDefaultValues
(
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *actionPtr,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType
);

extern GT_STATUS prvCpssDxChExactMatchManagerEntryGetFromHw
(
    IN      GT_U32                                        exactMatchManagerId,
    INOUT   CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC       *entryPtr
);

extern void *malloc(size_t size);
extern void free(void *ptr);
GT_STATUS prvCpssDxChExactMatchManagerDebug_bindDedicatedMallocAndFree(
    IN CPSS_OS_MALLOC_FUNC            mallocFunc, /* NULL to 'restore' default usage */
    IN CPSS_OS_FREE_FUNC              freeFunc    /* NULL to 'restore' default usage */
);

extern GT_STATUS prvCompareExactMatchExpandedStructs
(
    IN GT_CHAR                                              *name,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT                actionType,
    IN CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT     *firstActionPtr,
    IN CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT     *secondActionPtr
);

extern GT_STATUS prvCompareExactMatchActionStructs
(
    IN GT_U8                                    devNum,
    IN GT_CHAR                                  *name,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *firstActionPtr,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_UNT         *secondActionPtr
);

typedef struct {
    GT_U32      expErrArrSize;
    char       *errString;
    GT_U32      expErrArr[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E];
} EXACT_MATCH_MANAGER_DEBUG_ERROR_INFO;

/* Global dbCheck(All test selected) */
static CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC                 dbCheckParam = {1, 1, 1, 1, 1, 1, 1, 1, 1};

/* Used List 1st pointer error, skips other checks in used list */
#define EXACT_MATCH_SELF_TEST_ALL_ERR       {0,1,2,3,4,5,6,/*7,8,9,10,11,*/12,13,14,15,16,17,18,19,20,/*21,*/22,/*23,24,25,*/26,27}

static const EXACT_MATCH_MANAGER_DEBUG_ERROR_INFO debugErrorArr[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E+1] =
{
    /*Error Count*/ /*Error String*/                                                          /*Exp Errors*/    /*Enum Num*/
    {1              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E"          , {0}            }, /* 0*/

    {2              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_TYPE_COUNTERS_INVALID_E"            , {1,3}          }, /* 1*/
    {2              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E"            , {2,3}          }, /* 2*/
    {2              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_CUCKOO_BANK_COUNTERS_MISMATCH_E"    , {2,3}          }, /* 3*/

    {1              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_FREE_LIST_1ST_PTR_INVALID_E"        , {4}            }, /* 4*/
    {3              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E"   , {5,11,15}      }, /* 5*/

    {3              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E"        , {5,6,12}       }, /* 6*/
    {3              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_LAST_PTR_INVALID_E"       , {5,7,12}       }, /* 7*/
    {4              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E"    , {8,14,15,17}   }, /* 8*/
    {1              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_DELETE_SCAN_PTR_INVALID_E", {9}            },  /* 9*/
    {1              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_ENTRY_PREV_PTR_INVALID_E" , {10}           }, /*10*/
    {3              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E"   , {5,11,15}      }, /*11*/

    {3              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E"        , {5,6,12}       }, /*12*/
    {2              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E"              , {2,13}         }, /*13*/

    {3              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E"   , {14,18,19}     }, /*14*/
    {3              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E"         , {5,11,15}      }, /*15*/
    {4              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_ENTRY_BIN_INDEX_INVALID_E"    , {14,15,16,17}  }, /*16*/
    {4              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E"           , {8,14,15,17}   }, /*17*/
    {3              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E"    , {14,18,19}     }, /*18*/
    {1              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E"    , {19}           }, /*19*/

    {2              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E"                 , {20,27}        }, /*20*/
    {2              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_CLIENT_INVALID_E"         , {0,21}         }, /*21*/
    {1              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_PROFILE_ID_INVALID_E"     , {22}           }, /*22*/
    {1              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_KEY_PARAMS_INVALID_E"     , {23}           }, /*23*/
    {1              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_DEFAULT_ACTION_ENABLE_INVALID_E" , {24}    }, /*24*/
    {1              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_DEFAULT_ACTION_INVALID_E" , {25}           }, /*25*/
    {1              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_EXPANDER_CONFIG_INVALID_E"       , {26}           }, /*26*/
    {1              , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_CUCKOO_ENTRY_INVALID_E"             , {27}           },/*27*/
    {19             , "CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_ALL_E"                              , EXACT_MATCH_SELF_TEST_ALL_ERR}
};

typedef struct
{
    GT_U8 data[47];
}rand_data_stc;

/* fixed array to hold constant random data for several random test with different keys in preformance tests */
rand_data_stc  randomArr[131072];

typedef struct
{
    GT_U32 capacity;
    GT_U32 numEntriesMissed;
    GT_BOOL noFirstMiss;
}first_miss_stc;

typedef struct
{
    GT_U32 capacity;
    GT_U32 seconds;
    GT_U32 nanoSec;
    GT_U32 succeededIndexes;
    GT_U32 secondsEntryMax;
    GT_U32 nanoSecEntryMax;
}utilization_stc;

typedef struct
{
    first_miss_stc firstMiss;
    utilization_stc utilization;
}random_results_stc;

typedef struct
{
    first_miss_stc firstMiss;
    utilization_stc utilization;
    GT_FLOAT64 time;
    GT_FLOAT64 entryTime;
}random_results_final_stc;

/* array that hold results for 100 random iterations for average test */
random_results_stc randomResArray[101]; /* 1..100 */
/* array that holds results per keyType */
random_results_final_stc randomFinalResArray[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E];
/**
* @enum UTF_PRV_CPSS_API_STATISTICS_ENT
 *
 * @brief This enum defines API statistics types
*/
typedef enum {
    /** @brief add API statistics */
     UTF_PRV_CPSS_API_STATISTICS_ADD_E
    /** @brief delet API statistics */
    ,UTF_PRV_CPSS_API_STATISTICS_DELETE_E
    /** @brief update API statistics */
    ,UTF_PRV_CPSS_API_STATISTICS_UPDATE_E
} UTF_PRV_CPSS_EXACT_MATCH_API_STATISTICS_ENT;

static CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC        statistics_global;
static void exactMatchManagerEntry_updateDbStatistics(
    IN  UTF_PRV_CPSS_EXACT_MATCH_API_STATISTICS_ENT         statisticType,
    IN  GT_STATUS                                           status,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC        *statisticsPtr
);

#define EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId, entryPtr, paramsPtr, st)               \
    st = cpssDxChExactMatchManagerEntryAdd(exactMatchManagerId, entryPtr, paramsPtr);                    \
    exactMatchManagerEntry_updateDbStatistics(UTF_PRV_CPSS_API_STATISTICS_ADD_E, st, &statistics_global)

#define EM_MANAGER_ENTRY_DELETE_WITH_STATISTICS(exactMatchManagerId, entryPtr, st)                      \
    st = cpssDxChExactMatchManagerEntryDelete(exactMatchManagerId, entryPtr);                           \
    exactMatchManagerEntry_updateDbStatistics(UTF_PRV_CPSS_API_STATISTICS_DELETE_E, st, &statistics_global)

#define EM_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(exactMatchManagerId, entryPtr, st)                      \
    st = cpssDxChExactMatchManagerEntryUpdate(exactMatchManagerId, entryPtr);                           \
    exactMatchManagerEntry_updateDbStatistics(UTF_PRV_CPSS_API_STATISTICS_UPDATE_E, st, &statistics_global)

#define EM_MANAGER_EXPANDED_ACTION_ENTRY_UPDATE(exactMatchManagerId, expandedActionIndex, paramsPtr, st) \
    st = cpssDxChExactMatchManagerExpandedActionUpdate(exactMatchManagerId, expandedActionIndex, paramsPtr);

#define EM_MANAGER_EXPANDED_ACTION_ENTRY_GET(exactMatchManagerId, expandedActionIndex, paramsPtr, st)    \
    st = cpssDxChExactMatchManagerExpandedActionGet(exactMatchManagerId, expandedActionIndex, paramsPtr);

#define START_TIME(rc)                                                              \
        rc = cpssOsTimeRT(&secondsStart,&nanoSecondsStart);

#define HOW_MUCH_TIME(numIter,totalNum,doPrint,rc)                                  \
    rc = cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd);                                 \
                                                                                    \
    seconds = secondsEnd-secondsStart;                                              \
    if(nanoSecondsEnd >= nanoSecondsStart)                                          \
    {                                                                               \
        nanoSec = nanoSecondsEnd-nanoSecondsStart;                                  \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        nanoSec = (1000000000 - nanoSecondsStart) + nanoSecondsEnd;                 \
        seconds--;                                                                  \
    }                                                                               \
    if (doPrint==GT_TRUE)                                                           \
    {                                                                               \
        cpssOsPrintf("utilization: %d%% %d sec., %d nanosec.\n",(100*numIter)/totalNum, seconds, nanoSec);\
    }

#define CALC_DIFF(rc)                                  \
    rc = cpssOsTimeRT(&secondsEndEntry,&nanoSecondsEndEntry);                                 \
                                                                                    \
    secondsEntry = secondsEndEntry-secondsStartEntry;                                              \
    if(nanoSecondsEndEntry >= nanoSecondsStartEntry)                                          \
    {                                                                               \
        nanoSecEntry = nanoSecondsEndEntry-nanoSecondsStartEntry;                                  \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        nanoSecEntry = (1000000000 - nanoSecondsStartEntry) + nanoSecondsEndEntry;                 \
        secondsEntry--;                                                                  \
    }                                                                               \


extern GT_STATUS cpssDxChExactMatchRulesDump
(
    IN GT_U8                              devNum,
    IN GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN GT_U32                             startIndex,
    IN GT_U32                             numOfRules
);

static void exactMatchManagerHwEntryCheck(
    IN CPSS_DXCH_EXACT_MATCH_ENTRY_STC              *entryPtr
);

/* wrapper for standard malloc to avoid warnings about size_t type */
GT_VOID * utfMallocExactMatch
(
    IN GT_U32 size
)
{
   return malloc(size);
}

/**
* @enum UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC
 *
*  @brief Structure returns information as a result of Exact
*         Match API call
*/
typedef struct {
    /** @brief (pointer to) Exact Match manager entry */
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC     *entryPtr;
    /** @brief (pointer to) result of API add function
     *  GT_TRUE     - entry added to Exact Match
     *  GT_FALSE    - entry not added to Exact Match */
    GT_BOOL                                 *entryAddedPtr;
    /** @brief status for adding Exact Match entry to manager */
    GT_STATUS                               rc;
} UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC;

/*************************************************************/
/* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :             */
/* osMalloc return NULL size in range 0x10000 - 0x200000     */
/* after 206 iterations - although first 205 freed           */
/*************************************************************/
/* Bug fixed - tests restored to use cpssOsMalloc/cpssOsFree */
/* instead of workaround (utfMallocExactMatch,free)          */
/*************************************************************/
#define BIND_MALLOC_AND_FREE    \
    prvCpssDxChExactMatchManagerDebug_bindDedicatedMallocAndFree(NULL,NULL)

#define RESTORE_OS_MALLOC_AND_OS_FREE    \
    prvCpssDxChExactMatchManagerDebug_bindDedicatedMallocAndFree(NULL,NULL)


/* defines */
/* we force application to give 'num of entries' in steps of 256 */
#define NUM_ENTRIES_STEPS_CNS 256


/* cleanup on specific manager */
#define CLEANUP_MANAGER(exactMatchManagerId) \
    (void)cpssDxChExactMatchManagerDelete(exactMatchManagerId);

/* cleanup on ALL managers */
#define CLEANUP_ALL_MANAGERS                                                    \
    {                                                                           \
        GT_U32  ii;                                                             \
        for(ii = 0 ; ii < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS; ii++) \
        {                                                                       \
            (void)cpssDxChExactMatchManagerDelete(ii);                          \
        }                                                                       \
    }

static GT_U32   checkNonSupportedValues[32] = {
    /*0*/ 0xFFFFFFFF,
    /*1*/ 0xA5A5A5A5,
    /*2*/ 0x7FFFFFFF
};
/* check bounders of numeric field , and high values (31,32 bits value)  :
expected to fail */
#define CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(               \
    fieldName,                                                                      \
    /*IN GT_U32                                          */ testedField_minValue,   \
    /*IN GT_U32                                          */ testedField_maxValue,   \
    /*IN GT_U32                                          */ step                    \
)                                                                                   \
{                                                                                   \
    GT_U32  ii,iiMax = 3,iiMin;                                                     \
    GT_U32  origValue = fieldName;                                                  \
    iiMax = 3;                                                                      \
    if((testedField_minValue) != 0) /* we check 0xFFFFFFFF anyway */                \
    {                                                                               \
        checkNonSupportedValues[iiMax++] = (GT_U32)((testedField_minValue) - 1);    \
    }                                                                               \
    checkNonSupportedValues[iiMax++] = (GT_U32)((testedField_maxValue) + 1);        \
                                                                                    \
    for(ii = 0; ii < iiMax; ii++)                                                   \
    {                                                                               \
        fieldName = checkNonSupportedValues[ii];                                    \
        st = cpssDxChExactMatchManagerCreate(exactMatchManagerId,                   \
            &capacityInfo,&lookupInfo,&entryAttrInfo,&agingInfo);                   \
        /* restore orig value */                                                    \
        fieldName = origValue;                                                      \
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,                              \
            "cpssDxChExactMatchManagerCreate: %s expected to GT_BAD_PARAM on value [%d]",\
            #fieldName,                                                             \
            checkNonSupportedValues[ii]);                                           \
    }                                                                               \
                                                                                    \
    iiMax = testedField_maxValue;                                                   \
    iiMin = testedField_minValue;                                                   \
    for(ii = iiMin ; ii <= iiMax; ii+=step)                                         \
    {                                                                               \
        fieldName = ii;                                                             \
        st = cpssDxChExactMatchManagerCreate(exactMatchManagerId,                   \
            &capacityInfo,&lookupInfo,&entryAttrInfo,&agingInfo);                   \
        /* kill the manager ... for next time */                                    \
        CLEANUP_MANAGER(exactMatchManagerId);                                       \
        /* restore orig value */                                                    \
        fieldName = origValue;                                                      \
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,                                     \
            "cpssDxChExactMatchManagerCreate: %s expected to GT_OK on value [%d]",  \
            #fieldName,                                                             \
            ii);                                                                    \
        if(ii < step || ii == 0) break;                                             \
    }                                                                               \
}

/* check specific value of numeric field , and high values (31,32 bits value) :
expected to fail */
#define FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(      \
    fieldName,                                                                  \
    /*IN GT_U32                                          */ testedField_Value   \
)                                                                               \
{                                                                               \
    GT_U32  origValue = fieldName;                                              \
                                                                                \
    fieldName = (testedField_Value);                                            \
    st = cpssDxChExactMatchManagerCreate(exactMatchManagerId,                   \
            &capacityInfo,&lookupInfo,&entryAttrInfo,&agingInfo);               \
    /* restore orig value */                                                    \
    fieldName = origValue;                                                      \
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,                              \
        "cpssDxChExactMatchManagerCreate: %s expected to GT_BAD_PARAM on value [%d]",\
        #fieldName,                                                             \
        (testedField_Value));                                                   \
}

/* check NULL pointer param , expected to fail */
#define FAIL_NULL_PTR_cpssDxChExactMatchManagerCreate(                          \
    paramPtr /*one of : ptr1,ptr2,ptr3,ptr4*/,                                  \
    ptr1,ptr2,ptr3,ptr4                                                         \
)                                                                               \
{                                                                               \
    GT_VOID*  origPtr = (GT_VOID*)paramPtr;                                     \
                                                                                \
    paramPtr = NULL;                                                            \
                                                                                \
    st = cpssDxChExactMatchManagerCreate(exactMatchManagerId,                   \
        ptr1,ptr2,ptr3,ptr4);                                                   \
    /* restore orig value */                                                    \
    paramPtr = origPtr;                                                         \
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,                                \
        "cpssDxChExactMatchManagerCreate: %s is NULL pointer expected GT_BAD_PTR",  \
        #paramPtr);                                                             \
}

/* check NULL pointer param , expected to fail */
#define FAIL_NULL_PTR_cpssDxChExactMatchManagerConfigGet(                       \
    paramPtr /*one of : ptr1,ptr2,ptr3,ptr4*/,                                  \
    ptr1,ptr2,ptr3,ptr4                                                         \
)                                                                               \
{                                                                               \
    GT_VOID*  origPtr = (GT_VOID*)paramPtr;                                     \
                                                                                \
    paramPtr = NULL;                                                            \
                                                                                \
    st = cpssDxChExactMatchManagerConfigGet(exactMatchManagerId,                \
        ptr1,ptr2,ptr3,ptr4);                                                   \
    /* restore orig value */                                                    \
    paramPtr = origPtr;                                                         \
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,                                \
        "cpssDxChExactMatchManagerConfigGet: %s is NULL pointer expected GT_BAD_PTR",  \
        #paramPtr);                                                             \
}


extern GT_U32 utfFamilyTypeGet(IN GT_U8 devNum);
/* add additional macro to treat devices like Ironman that not hold dedicated
   family in UTF_PP_FAMILY_BIT_ENT , and uses the UTF_AC5X_E */
#define PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC                             \
    if(0 == PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.parametericTables.numEntriesEm) \
    {                                                                    \
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,utfFamilyTypeGet(prvTgfDevNum));         \
    }


#define EXACT_MATCH_MANAGER_NON_SUPPORTED_DEV_LIST  ~(UTF_CPSS_PP_ALL_SIP6_CNS)

/* macro to skip devices that not applicable for the Exact Match manager 'device-less APIs'*/
#define DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE                                          \
    {                                                                                       \
        GT_U8   dev;                                                                        \
        /* prepare device iterator */                                                       \
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, EXACT_MATCH_MANAGER_NON_SUPPORTED_DEV_LIST); \
        PRV_TGF_NON_EM_APPLIC_DEV_RESET_MAC;                                                \
        if (GT_OK != prvUtfNextDeviceGet(&dev, GT_TRUE))                                    \
        {                                                                                   \
            /* the current running device is NOT in the supporting list */                  \
            SKIP_TEST_MAC;                                                                  \
        }                                                                                   \
    }

/*UTF_SIP6_GM_NOT_READY_SKIP_MAC(_log_string)*/

#define TESTED_EXACT_MATCH_MANAGER_ID_CNS 17

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchManagerCreate
(
    IN GT_U32                                                exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC            *capacityPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC              *lookupPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC    *entryAttrPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC               *agingPtr
);
*/
static void internal_cpssDxChExactMatchManagerCreateUT(void)
{
    GT_STATUS                                             st;
    GT_U32                                                exactMatchManagerId;
    CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC            capacityInfo;
    CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC            *capacityInfoPtr;
    CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC              lookupInfo;
    CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC              *lookupInfoPtr;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC    entryAttrInfo;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC    *entryAttrInfoPtr;
    CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC               agingInfo;
    CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC               *agingInfoPtr;

    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               actionType;
    GT_U32                                              expandedActionIndex;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    actionData;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    actionDataGet;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    expandedActionOriginData;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    expandedActionOriginDataGet;
    GT_U32                                              i=0;

    capacityInfoPtr  = &capacityInfo;
    entryAttrInfoPtr = &entryAttrInfo;
    lookupInfoPtr    = &lookupInfo;
    agingInfoPtr     = &agingInfo;

    /************************/
    /* create valid manager */
    /* proof of good parameters */
    /************************/
    exactMatchManagerId = TESTED_EXACT_MATCH_MANAGER_ID_CNS;

    /* capacity info configuration */
    capacityInfo.hwCapacity.numOfHwIndexes = EXACT_MATCH_SIZE_FULL;
    capacityInfo.hwCapacity.numOfHashes    = EXACT_MATCH_MAX_BANKS;
    capacityInfo.maxTotalEntries           = 1024;
    capacityInfo.maxEntriesPerAgingScan    = NUM_ENTRIES_STEPS_CNS; /*256*/
    capacityInfo.maxEntriesPerDeleteScan   = NUM_ENTRIES_STEPS_CNS; /*256*/

    /* lookup configuration */

    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupEnable=GT_TRUE;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClient=CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsNum=1;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].ttiMappingElem.keyType=CPSS_DXCH_TTI_KEY_ETH_E;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].ttiMappingElem.enableExactMatchLookup=GT_TRUE;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].ttiMappingElem.profileId=1;

    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupEnable=GT_TRUE;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClient=CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsNum=1;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[0].pclMappingElem.packetType=CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[0].pclMappingElem.subProfileId = 1;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[0].pclMappingElem.enableExactMatchLookup=GT_TRUE;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[0].pclMappingElem.profileId=8;

    /* configure profileId=1 for TTI client */
    cpssOsMemSet(lookupInfo.profileEntryParamsArray,0,sizeof(lookupInfo.profileEntryParamsArray));

    lookupInfo.profileEntryParamsArray[1].keyParams.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
    lookupInfo.profileEntryParamsArray[1].keyParams.keyStart=1;
    cpssOsMemSet(lookupInfo.profileEntryParamsArray[1].keyParams.mask,0,sizeof(lookupInfo.profileEntryParamsArray[1].keyParams.mask));
    lookupInfo.profileEntryParamsArray[1].defaultActionType=CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
    lookupInfo.profileEntryParamsArray[1].defaultActionEn = GT_TRUE;
    prvSetExactMatchActionDefaultValues(&lookupInfo.profileEntryParamsArray[1].defaultAction,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

    /* configure profileId=8 for EPCL client */
    lookupInfo.profileEntryParamsArray[8].keyParams.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
    lookupInfo.profileEntryParamsArray[8].keyParams.keyStart=2;
    cpssOsMemSet(lookupInfo.profileEntryParamsArray[8].keyParams.mask,0,sizeof(lookupInfo.profileEntryParamsArray[8].keyParams.mask));
    lookupInfo.profileEntryParamsArray[8].defaultActionType=CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
    lookupInfo.profileEntryParamsArray[8].defaultActionEn = GT_TRUE;
    prvSetExactMatchActionDefaultValues(&lookupInfo.profileEntryParamsArray[8].defaultAction,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

    /* Expander configuration */
    actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;

    cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
    cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));

    cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
    cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionActionStop=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCommand=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionFlowId=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

    prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

    /* configure half array with TTI configuration and half with PCL configuration*/
    for (expandedActionIndex=0; expandedActionIndex<CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS/2;expandedActionIndex++)
    {
        entryAttrInfo.expandedArray[expandedActionIndex].exactMatchExpandedEntryValid=GT_TRUE;
        entryAttrInfo.expandedArray[expandedActionIndex].expandedActionType=actionType;
        cpssOsMemCpy(&entryAttrInfo.expandedArray[expandedActionIndex].expandedAction,&actionData,sizeof(actionData));
        cpssOsMemCpy(&entryAttrInfo.expandedArray[expandedActionIndex].expandedActionOrigin.ttiExpandedActionOrigin,
                     &expandedActionOriginData,
                     sizeof(expandedActionOriginData));
    }

    actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
    cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
    cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId=GT_TRUE;
    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_TRUE;
    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup=GT_TRUE;
    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

    prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

    for (;expandedActionIndex<CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS;expandedActionIndex++)
    {
        entryAttrInfo.expandedArray[expandedActionIndex].exactMatchExpandedEntryValid=GT_TRUE;
        entryAttrInfo.expandedArray[expandedActionIndex].expandedActionType=actionType;
        cpssOsMemCpy(&entryAttrInfo.expandedArray[expandedActionIndex].expandedAction,&actionData,sizeof(actionData));
        cpssOsMemCpy(&entryAttrInfo.expandedArray[expandedActionIndex].expandedActionOrigin.pclExpandedActionOrigin,
                     &expandedActionOriginData,
                     sizeof(expandedActionOriginData));

    }

    /* aging configuration */
    agingInfo.agingRefreshEnable=GT_TRUE;

    st = cpssDxChExactMatchManagerCreate(exactMatchManagerId,
        &capacityInfo,&lookupInfo,&entryAttrInfo,&agingInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChExactMatchManagerCreate ");

    CLEANUP_MANAGER(exactMatchManagerId);

    /* check double lookup configuration - return fail */
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsNum=2;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].ttiMappingElem.keyType=CPSS_DXCH_TTI_KEY_ETH_E;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].ttiMappingElem.enableExactMatchLookup=GT_TRUE;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].ttiMappingElem.profileId=1;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[1].ttiMappingElem.keyType=CPSS_DXCH_TTI_KEY_ETH_E;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[1].ttiMappingElem.enableExactMatchLookup=GT_TRUE;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[1].ttiMappingElem.profileId=2;

    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsNum=2;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[0].pclMappingElem.packetType=CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[0].pclMappingElem.subProfileId = 1;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[0].pclMappingElem.enableExactMatchLookup=GT_TRUE;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[0].pclMappingElem.profileId=8;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[1].pclMappingElem.packetType=CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[1].pclMappingElem.subProfileId = 1;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[1].pclMappingElem.enableExactMatchLookup=GT_TRUE;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[1].pclMappingElem.profileId=9;

    st = cpssDxChExactMatchManagerCreate(exactMatchManagerId,
        &capacityInfo,&lookupInfo,&entryAttrInfo,&agingInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st,"cpssDxChExactMatchManagerCreate ");

    CLEANUP_MANAGER(exactMatchManagerId);
    /* reset values back to legal default */
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsNum=1;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].ttiMappingElem.keyType=CPSS_DXCH_TTI_KEY_ETH_E;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].ttiMappingElem.enableExactMatchLookup=GT_TRUE;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].ttiMappingElem.profileId=1;

    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsNum=1;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[0].pclMappingElem.packetType=CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[0].pclMappingElem.subProfileId = 1;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[0].pclMappingElem.enableExactMatchLookup=GT_TRUE;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[0].pclMappingElem.profileId=8;

    /***** we have good parameters , lets check changes in them ***/

    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        capacityInfo.hwCapacity.numOfHwIndexes,
        (EXACT_MATCH_SIZE_FULL-1));

    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        capacityInfo.hwCapacity.numOfHwIndexes,
        (_256K+1));

    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        capacityInfo.hwCapacity.numOfHashes,
        15);

    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        capacityInfo.hwCapacity.numOfHashes,
        17);

    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        capacityInfo.maxTotalEntries,
        0);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        capacityInfo.maxTotalEntries,
        NUM_ENTRIES_STEPS_CNS,/* must not be ZERO (and must be in steps of 256)*/
        capacityInfo.hwCapacity.numOfHwIndexes,
        NUM_ENTRIES_STEPS_CNS/*step of NUM_ENTRIES_STEPS_CNS*/);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        capacityInfo.maxEntriesPerAgingScan,
        NUM_ENTRIES_STEPS_CNS,
        capacityInfo.maxTotalEntries,
        NUM_ENTRIES_STEPS_CNS/*step*/);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        capacityInfo.maxEntriesPerDeleteScan,
        NUM_ENTRIES_STEPS_CNS,
        capacityInfo.maxTotalEntries,
        NUM_ENTRIES_STEPS_CNS/*step*/);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        lookupInfo.lookupsArray[0].lookupClient,
        CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E,
        CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E,
        1/*step*/);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        lookupInfo.lookupsArray[0].lookupClientMappingsNum,
        0,
        (MAX_NUM_PROFILE_ID_MAPPING_ENTRIES_CNS-1),
        1/*step*/);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        lookupInfo.lookupsArray[0].lookupClientMappingsArray[0].ttiMappingElem.keyType,/* this is a TTI configuration set above */
        CPSS_DXCH_TTI_KEY_IPV4_E,
        CPSS_DXCH_TTI_KEY_UDB_UDE6_E,
        1/*step*/);

    /* configure profileId 1-7 to hold TTI configutration */
    for (i=1;i<8;i++)
    {
        lookupInfo.profileEntryParamsArray[i].keyParams.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        lookupInfo.profileEntryParamsArray[i].keyParams.keyStart=1;
        cpssOsMemSet(lookupInfo.profileEntryParamsArray[1].keyParams.mask,0,sizeof(lookupInfo.profileEntryParamsArray[i].keyParams.mask));
        lookupInfo.profileEntryParamsArray[i].defaultActionType=CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
        lookupInfo.profileEntryParamsArray[i].defaultActionEn = GT_TRUE;
        prvSetExactMatchActionDefaultValues(&lookupInfo.profileEntryParamsArray[i].defaultAction,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);
    }
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        lookupInfo.lookupsArray[0].lookupClientMappingsArray[0].ttiMappingElem.profileId,/* this is a TTI configuration set above. profileId 1-7 are valid for TTI */
        1,
        7,
        1/*step*/);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        lookupInfo.lookupsArray[1].lookupClientMappingsArray[0].pclMappingElem.packetType,/* this is a PCL configuration set above */
        CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E,
        (CPSS_DXCH_PCL_PACKET_TYPE_LAST_E-1),
        1/*step*/);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        lookupInfo.lookupsArray[1].lookupClientMappingsArray[0].pclMappingElem.subProfileId,/* this is a PCL configuration set above */
        0,
        15,/* AC5P max is 15, Falcon max is 7 - at this point we check the max value, since we do not have a devNum */
        1/*step*/);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        lookupInfo.profileEntryParamsArray[1].keyParams.keySize,
        CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E,
        (CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E-1),
        1/*step*/);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        lookupInfo.profileEntryParamsArray[1].keyParams.keyStart,
        0,
        BIT_6,
        1/*step*/);

    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
       lookupInfo.profileEntryParamsArray[1].defaultActionType,
       CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
       lookupInfo.profileEntryParamsArray[1].defaultActionType,
       CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E);


    /* configure profileId 8-15 to hold PCL configutration */
    for (i=8;i<=15;i++)
    {
        lookupInfo.profileEntryParamsArray[i].keyParams.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
        cpssOsMemSet(lookupInfo.profileEntryParamsArray[i].keyParams.mask,0,sizeof(lookupInfo.profileEntryParamsArray[i].keyParams.mask));
        lookupInfo.profileEntryParamsArray[i].defaultActionType=CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
        lookupInfo.profileEntryParamsArray[i].defaultActionEn = GT_TRUE;
        prvSetExactMatchActionDefaultValues(&lookupInfo.profileEntryParamsArray[i].defaultAction,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);
    }
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        lookupInfo.lookupsArray[1].lookupClientMappingsArray[0].pclMappingElem.profileId,/* this is a PCL configuration set above. profileId 8-15 are valid for PCL*/
        8,
        15,
        1/*step*/);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        lookupInfo.profileEntryParamsArray[8].keyParams.keySize,
        CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E,
        (CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E-1),
        1/*step*/);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        lookupInfo.profileEntryParamsArray[8].keyParams.keyStart,
        0,
        BIT_6,
        1/*step*/);

     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        lookupInfo.profileEntryParamsArray[8].defaultActionType,
        CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);
     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        lookupInfo.profileEntryParamsArray[8].defaultActionType,
        CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        entryAttrInfo.expandedArray[0].expandedActionType,
        CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E,
        (CPSS_DXCH_EXACT_MATCH_ACTION_LAST_E-1),
        1/*step*/);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerCreate(
        entryAttrInfo.expandedArray[1].expandedActionType,
        CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E,
        (CPSS_DXCH_EXACT_MATCH_ACTION_LAST_E-1),
        1/*step*/);

    /* check NULL pointers */
    FAIL_NULL_PTR_cpssDxChExactMatchManagerCreate(capacityInfoPtr/*checked for NULL*/,
        capacityInfoPtr,lookupInfoPtr,entryAttrInfoPtr,agingInfoPtr);
    /* check NULL pointers */
    FAIL_NULL_PTR_cpssDxChExactMatchManagerCreate(lookupInfoPtr/*checked for NULL*/,
        capacityInfoPtr,lookupInfoPtr,entryAttrInfoPtr,agingInfoPtr);
    /* check NULL pointers */
    FAIL_NULL_PTR_cpssDxChExactMatchManagerCreate(entryAttrInfoPtr/*checked for NULL*/,
        capacityInfoPtr,lookupInfoPtr,entryAttrInfoPtr,agingInfoPtr);
    /* check NULL pointers */
    FAIL_NULL_PTR_cpssDxChExactMatchManagerCreate(agingInfoPtr/*checked for NULL*/,
        capacityInfoPtr,lookupInfoPtr,entryAttrInfoPtr,agingInfoPtr);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchManagerCreate
(
    IN GT_U32                                                exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC            *capacityPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC              *lookupPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC    *entryAttrPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC               *agingPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerCreate)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    internal_cpssDxChExactMatchManagerCreateUT();

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchManagerConfigGet
(
    IN  GT_U32                                              exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC          *capacityPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC            *lookupPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC  *entryAttrPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC             *agingPtr
);
*/
static void internal_cpssDxChExactMatchManagerConfigGetUT(void)
{
    GT_STATUS                                             st;
    GT_U32                                                exactMatchManagerId;
    CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC            capacityInfo;
    CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC            *capacityInfoPtr;
    CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC              lookupInfo;
    CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC              *lookupInfoPtr;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC    entryAttrInfo;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC    *entryAttrInfoPtr;
    CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC               agingInfo;
    CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC               *agingInfoPtr;

    CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC           capacityInfoGet;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC   entryAttrInfoGet;
    CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC             lookupInfoGet;
    CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC              agingInfoGet;

    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               actionType;
    GT_U32                                              expandedActionIndex;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    actionData;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    actionDataGet;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    expandedActionOriginData;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    expandedActionOriginDataGet;

    cpssOsMemSet(&capacityInfo,     0,  sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC));
    cpssOsMemSet(&capacityInfoGet,  0,  sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC));

    cpssOsMemSet(&entryAttrInfo,    0,  sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC));
    cpssOsMemSet(&entryAttrInfoGet, 0,  sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC));

    cpssOsMemSet(&lookupInfo,       0,  sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC));
    cpssOsMemSet(&lookupInfoGet,    0,  sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC));

    cpssOsMemSet(&agingInfo,        0,  sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC));
    cpssOsMemSet(&agingInfoGet,     0,  sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC));


    capacityInfoPtr  = &capacityInfoGet  ;
    entryAttrInfoPtr = &entryAttrInfoGet ;
    lookupInfoPtr    = &lookupInfoGet    ;
    agingInfoPtr     = &agingInfoGet     ;

    /************************/
   /* create valid manager */
   /* proof of good parameters */
   /************************/
   exactMatchManagerId = TESTED_EXACT_MATCH_MANAGER_ID_CNS;

   /* capacity info configuration */
   capacityInfo.hwCapacity.numOfHwIndexes = EXACT_MATCH_SIZE_FULL;
   capacityInfo.hwCapacity.numOfHashes    = EXACT_MATCH_MAX_BANKS;
   capacityInfo.maxTotalEntries           = 1024;
   capacityInfo.maxEntriesPerAgingScan    = NUM_ENTRIES_STEPS_CNS; /*256*/
   capacityInfo.maxEntriesPerDeleteScan   = NUM_ENTRIES_STEPS_CNS; /*256*/

   /* lookup configuration */

   lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupEnable=GT_TRUE;
   lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClient=CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;
   lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsNum=1;
   lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].ttiMappingElem.keyType=CPSS_DXCH_TTI_KEY_ETH_E;
   lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].ttiMappingElem.enableExactMatchLookup=GT_TRUE;
   lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].ttiMappingElem.profileId=1;

   lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupEnable=GT_TRUE;
   lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClient=CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E;
   lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsNum=1;
   lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[0].pclMappingElem.packetType=CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E;
   lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[0].pclMappingElem.subProfileId = 1;
   lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[0].pclMappingElem.enableExactMatchLookup=GT_TRUE;
   lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[0].pclMappingElem.profileId=8;

   /* configure profileId=1 for TTI client */
   cpssOsMemSet(lookupInfo.profileEntryParamsArray,0,sizeof(lookupInfo.profileEntryParamsArray));

   lookupInfo.profileEntryParamsArray[1].keyParams.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
   lookupInfo.profileEntryParamsArray[1].keyParams.keyStart=1;
   cpssOsMemSet(lookupInfo.profileEntryParamsArray[1].keyParams.mask,0,sizeof(lookupInfo.profileEntryParamsArray[1].keyParams.mask));
   lookupInfo.profileEntryParamsArray[1].defaultActionType=CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
   lookupInfo.profileEntryParamsArray[1].defaultActionEn = GT_TRUE;
   prvSetExactMatchActionDefaultValues(&lookupInfo.profileEntryParamsArray[1].defaultAction,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

   /* configure profileId=8 for EPCL client */
   lookupInfo.profileEntryParamsArray[8].keyParams.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
   lookupInfo.profileEntryParamsArray[8].keyParams.keyStart=2;
   cpssOsMemSet(lookupInfo.profileEntryParamsArray[8].keyParams.mask,0,sizeof(lookupInfo.profileEntryParamsArray[8].keyParams.mask));
   lookupInfo.profileEntryParamsArray[8].defaultActionType=CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
   lookupInfo.profileEntryParamsArray[8].defaultActionEn = GT_TRUE;
   prvSetExactMatchActionDefaultValues(&lookupInfo.profileEntryParamsArray[8].defaultAction,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

   /* Expander configuration */
   actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;

   cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
   cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));

   cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
   cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

   expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionActionStop=GT_TRUE;
   expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass=GT_TRUE;
   expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCommand=GT_TRUE;
   expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;
   expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionFlowId=GT_TRUE;
   expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

   prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

   /* configure half array with TTI configuration and half with PCL configuration*/
   for (expandedActionIndex=0; expandedActionIndex<CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS/2;expandedActionIndex++)
   {
       entryAttrInfo.expandedArray[expandedActionIndex].exactMatchExpandedEntryValid=GT_TRUE;
       entryAttrInfo.expandedArray[expandedActionIndex].expandedActionType=actionType;
       cpssOsMemCpy(&entryAttrInfo.expandedArray[expandedActionIndex].expandedAction,&actionData,sizeof(actionData));
       cpssOsMemCpy(&entryAttrInfo.expandedArray[expandedActionIndex].expandedActionOrigin.ttiExpandedActionOrigin,
                    &expandedActionOriginData,
                    sizeof(expandedActionOriginData));
   }

   actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
   cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
   cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

   expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId=GT_TRUE;
   expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_TRUE;
   expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup=GT_TRUE;
   expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

   prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

   for (;expandedActionIndex<CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS;expandedActionIndex++)
   {
       entryAttrInfo.expandedArray[expandedActionIndex].exactMatchExpandedEntryValid=GT_TRUE;
       entryAttrInfo.expandedArray[expandedActionIndex].expandedActionType=actionType;
       cpssOsMemCpy(&entryAttrInfo.expandedArray[expandedActionIndex].expandedAction,&actionData,sizeof(actionData));
       cpssOsMemCpy(&entryAttrInfo.expandedArray[expandedActionIndex].expandedActionOrigin.pclExpandedActionOrigin,
                    &expandedActionOriginData,
                    sizeof(expandedActionOriginData));

   }

   /* aging configuration */
   agingInfo.agingRefreshEnable=GT_TRUE;

   st = cpssDxChExactMatchManagerCreate(exactMatchManagerId,
       &capacityInfo,&lookupInfo,&entryAttrInfo,&agingInfo);
   UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChExactMatchManagerCreate ");


    /* Retrieve data */
    st = cpssDxChExactMatchManagerConfigGet(exactMatchManagerId,
         &capacityInfoGet,&lookupInfoGet,&entryAttrInfoGet,&agingInfoGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,
        "cpssDxChExactMatchManagerConfigGet ");

    UTF_VERIFY_EQUAL1_STRING_MAC(0, cpssOsMemCmp(
                                &capacityInfo, &capacityInfoGet,
                                sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC)),
        "cpssDxChExactMatchManagerConfigGet : expected 'get' entry to match 'set' entry in manager [%d]",
        exactMatchManagerId);

    UTF_VERIFY_EQUAL1_STRING_MAC(0, cpssOsMemCmp(
                                &entryAttrInfo, &entryAttrInfoGet,
                                sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC)),
        "cpssDxChExactMatchManagerConfigGet : expected 'get' entry to match 'set' entry in manager [%d]",
        exactMatchManagerId);

    for (expandedActionIndex = 0; expandedActionIndex<CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS; expandedActionIndex++)
    {
        if ((entryAttrInfo.expandedArray[expandedActionIndex].exactMatchExpandedEntryValid == 1)&&
            (entryAttrInfo.expandedArray[expandedActionIndex].exactMatchExpandedEntryValid ==
                                entryAttrInfoGet.expandedArray[expandedActionIndex].exactMatchExpandedEntryValid))
        {
            if (entryAttrInfo.expandedArray[expandedActionIndex].expandedActionType ==
                entryAttrInfoGet.expandedArray[expandedActionIndex].expandedActionType)
            {
                st = prvCompareExactMatchExpandedStructs("expandedActionOriginData",
                                                     entryAttrInfo.expandedArray[expandedActionIndex].expandedActionType,
                                                     &entryAttrInfo.expandedArray[expandedActionIndex].expandedActionOrigin,
                                                     &entryAttrInfoGet.expandedArray[expandedActionIndex].expandedActionOrigin);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"expandedActionOriginData != expandedActionOriginDataGet: %d, %d",expandedActionIndex, entryAttrInfo.expandedArray[expandedActionIndex].expandedActionType);
            }
            else
            {
                 UTF_VERIFY_EQUAL0_STRING_MAC(GT_FAIL, st, "cpssDxChExactMatchManagerConfigGet ");
            }

        }
        else
        {
            if (entryAttrInfo.expandedArray[expandedActionIndex].exactMatchExpandedEntryValid != 1)
            {
                /* continue to the next valid entry */
                continue;
            }
            else
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_FAIL, st, "cpssDxChExactMatchManagerConfigGet ");
            }
        }
    }
    UTF_VERIFY_EQUAL1_STRING_MAC(0, cpssOsMemCmp(
                                &lookupInfo, &lookupInfoGet,
                                sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC)),
        "cpssDxChExactMatchManagerConfigGet : expected 'get' entry to match 'set' entry in manager [%d]",
        exactMatchManagerId);

    UTF_VERIFY_EQUAL1_STRING_MAC(0, cpssOsMemCmp(
                                &agingInfo, &agingInfoGet,
                                sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC)),
        "cpssDxChExactMatchManagerConfigGet : expected 'get' entry to match 'set' entry in manager [%d]",
        exactMatchManagerId);

    /* check NULL pointers */
    FAIL_NULL_PTR_cpssDxChExactMatchManagerConfigGet(capacityInfoPtr/*checked for NULL*/,
        capacityInfoPtr,lookupInfoPtr,entryAttrInfoPtr,agingInfoPtr);
    /* check NULL pointers */
    FAIL_NULL_PTR_cpssDxChExactMatchManagerConfigGet(lookupInfoPtr/*checked for NULL*/,
        capacityInfoPtr,lookupInfoPtr,entryAttrInfoPtr,agingInfoPtr);
    /* check NULL pointers */
    FAIL_NULL_PTR_cpssDxChExactMatchManagerConfigGet(entryAttrInfoPtr/*checked for NULL*/,
        capacityInfoPtr,lookupInfoPtr,entryAttrInfoPtr,agingInfoPtr);
    /* check NULL pointers */
    FAIL_NULL_PTR_cpssDxChExactMatchManagerConfigGet(agingInfoPtr/*checked for NULL*/,
        capacityInfoPtr,lookupInfoPtr,entryAttrInfoPtr,agingInfoPtr);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChExactMatchManagerConfigGet
(
    IN  GT_U32                                              exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC          *capacityPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC            *lookupPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC  *entryAttrPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC             *agingPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerConfigGet)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    internal_cpssDxChExactMatchManagerConfigGetUT();

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}

/* create exact match manager - minimal parameters for tests */
static GT_STATUS createExactMatchManager
(
    IN GT_U32                                               exactMatchManagerId,
    IN GT_U32                                               maxTotalEntries,/* up to 128K (limited by numOfHwIndexes and sharedTableMode) */
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_HW_CAPACITY_STC        *hwCapacity
)
{
    GT_STATUS   st=GT_OK;
    CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC           capacityInfo;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC   entryAttrInfo;
    CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC             lookupInfo;
    CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC              agingInfo;

    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               actionType;
    GT_U32                                              expandedActionIndex;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    actionData;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    actionDataGet;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    expandedActionOriginData;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    expandedActionOriginDataGet;

    /* capacity info configuration */
    if(hwCapacity != NULL)
    {
        capacityInfo.hwCapacity.numOfHwIndexes = hwCapacity->numOfHwIndexes;
        capacityInfo.hwCapacity.numOfHashes    = hwCapacity->numOfHashes;
    }
    else
    {
        capacityInfo.hwCapacity.numOfHwIndexes = EXACT_MATCH_SIZE_FULL;
        capacityInfo.hwCapacity.numOfHashes    = EXACT_MATCH_MAX_BANKS;
    }
    capacityInfo.maxTotalEntries           = maxTotalEntries;
    capacityInfo.maxEntriesPerAgingScan    = NUM_ENTRIES_STEPS_CNS; /*256*/
    capacityInfo.maxEntriesPerDeleteScan   = NUM_ENTRIES_STEPS_CNS; /*256*/


    /* lookup configuration */
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupEnable=GT_TRUE;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClient=CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsNum=1;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].ttiMappingElem.keyType=CPSS_DXCH_TTI_KEY_ETH_E;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].ttiMappingElem.enableExactMatchLookup=GT_TRUE;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E].lookupClientMappingsArray[0].ttiMappingElem.profileId=1;

    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupEnable=GT_TRUE;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClient=CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsNum=1;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[0].pclMappingElem.packetType=CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[0].pclMappingElem.subProfileId = 1;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[0].pclMappingElem.enableExactMatchLookup=GT_TRUE;
    lookupInfo.lookupsArray[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E].lookupClientMappingsArray[0].pclMappingElem.profileId=8;

    /* configure profileId=1 for TTI client */
    cpssOsMemSet(lookupInfo.profileEntryParamsArray,0,sizeof(lookupInfo.profileEntryParamsArray));

    lookupInfo.profileEntryParamsArray[1].keyParams.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
    lookupInfo.profileEntryParamsArray[1].keyParams.keyStart=1;
    cpssOsMemSet(lookupInfo.profileEntryParamsArray[1].keyParams.mask,0,sizeof(lookupInfo.profileEntryParamsArray[1].keyParams.mask));
    lookupInfo.profileEntryParamsArray[1].defaultActionType=CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
    lookupInfo.profileEntryParamsArray[1].defaultActionEn = GT_TRUE;
    prvSetExactMatchActionDefaultValues(&lookupInfo.profileEntryParamsArray[1].defaultAction,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

    /* configure profileId=8 for PCL client */
    lookupInfo.profileEntryParamsArray[8].keyParams.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
    lookupInfo.profileEntryParamsArray[8].keyParams.keyStart=2;
    cpssOsMemSet(lookupInfo.profileEntryParamsArray[8].keyParams.mask,0,sizeof(lookupInfo.profileEntryParamsArray[8].keyParams.mask));
    lookupInfo.profileEntryParamsArray[8].defaultActionType=CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
    lookupInfo.profileEntryParamsArray[8].defaultActionEn = GT_TRUE;
    prvSetExactMatchActionDefaultValues(&lookupInfo.profileEntryParamsArray[8].defaultAction,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

    /* Expander configuration */
    actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;

    cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
    cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));

    cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
    cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionActionStop=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCommand=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionFlowId=GT_TRUE;
    expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

    prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

    /* configure half array with TTI configuration and half with PCL configuration*/
    for (expandedActionIndex=0; expandedActionIndex<CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS/2;expandedActionIndex++)
    {
        entryAttrInfo.expandedArray[expandedActionIndex].exactMatchExpandedEntryValid=GT_TRUE;
        entryAttrInfo.expandedArray[expandedActionIndex].expandedActionType=actionType;
        cpssOsMemCpy(&entryAttrInfo.expandedArray[expandedActionIndex].expandedAction,&actionData,sizeof(actionData));
        cpssOsMemCpy(&entryAttrInfo.expandedArray[expandedActionIndex].expandedActionOrigin.ttiExpandedActionOrigin,
                     &expandedActionOriginData,
                     sizeof(expandedActionOriginData));
    }

    actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
    cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
    cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId=GT_TRUE;
    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_TRUE;
    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup=GT_TRUE;
    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

    prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

    for (;expandedActionIndex<CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS;expandedActionIndex++)
    {
        entryAttrInfo.expandedArray[expandedActionIndex].exactMatchExpandedEntryValid=GT_TRUE;
        entryAttrInfo.expandedArray[expandedActionIndex].expandedActionType=actionType;
        cpssOsMemCpy(&entryAttrInfo.expandedArray[expandedActionIndex].expandedAction,&actionData,sizeof(actionData));
        cpssOsMemCpy(&entryAttrInfo.expandedArray[expandedActionIndex].expandedActionOrigin.pclExpandedActionOrigin,
                     &expandedActionOriginData,
                     sizeof(expandedActionOriginData));

    }

    /* aging configuration */
    agingInfo.agingRefreshEnable=GT_TRUE;

    st = cpssDxChExactMatchManagerCreate(exactMatchManagerId,
                                         &capacityInfo,&lookupInfo,&entryAttrInfo,&agingInfo);

    return st;
}

/**
GT_STATUS cpssDxChExactMatchManagerDelete
()
    IN GT_U32 exactMatchManagerId
)
*/
static void internal_cpssDxChExactMatchManagerDeleteUT(void)
{
    GT_STATUS   st=GT_OK;
    GT_U32      ii;
    GT_U32      exactMatchManagerId = TESTED_EXACT_MATCH_MANAGER_ID_CNS;

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        st = cpssDxChExactMatchManagerDelete(ii);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChExactMatchManagerDelete: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    /* check out of range managers ... expect 'GT_OUT_OF_RANGE' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        st = cpssDxChExactMatchManagerDelete(ii);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
            "cpssDxChExactMatchManagerDelete: expected to GT_OUT_OF_RANGE on manager [%d]",
            ii);
    }

    /* create single valid manager */
    st = createExactMatchManager(exactMatchManagerId, EXACT_MATCH_SIZE_FULL, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "createExactMatchManager: expected to GT_OK on manager [%d]",
        exactMatchManagerId);

    /* delete it */
    st = cpssDxChExactMatchManagerDelete(exactMatchManagerId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChExactMatchManagerDelete: expected to GT_OK on manager [%d]",
        exactMatchManagerId);
}

/**
GT_STATUS cpssDxChExactMatchManagerDelete
(
    IN GT_U32 exactMatchManagerId
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerDelete)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    internal_cpssDxChExactMatchManagerDeleteUT();

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}


/**
GT_STATUS cpssDxChExactMatchManagerDevListAdd
(
    IN GT_U32                                                   exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[],
    IN GT_U32                                                   numOfPairs
);
*/
static void internal_cpssDxChExactMatchManagerDevListAddUT(void)
{
    GT_STATUS   st=GT_OK, expectedReturnValue=GT_OK;
    GT_U32      ii;
    GT_U32      exactMatchManagerId = TESTED_EXACT_MATCH_MANAGER_ID_CNS;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC pairListArr[2];
    GT_U32      numOfPairs = 1;
    GT_PORT_GROUPS_BMP tempTti_portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_PORT_GROUPS_BMP tempEm_portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_PORT_GROUPS_BMP tempPcl_portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_PORT_GROUPS_BMP tempTcam_portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT  testResultArray[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                              errorNum=0, errorNumItr=0;

    pairListArr[0].devNum = prvTgfDevNum;
    pairListArr[0].portGroupsBmp = 0x1;

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        st = cpssDxChExactMatchManagerDevListAdd(ii,pairListArr,numOfPairs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChExactMatchManagerDevListAdd: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    /* check out of range managers ... expect 'GT_OUT_OF_RANGE' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        st = cpssDxChExactMatchManagerDevListAdd(ii,pairListArr,numOfPairs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
            "cpssDxChExactMatchManagerDevListAdd: expected to GT_OUT_OF_RANGE on manager [%d]",
            ii);
    }

    /* create single valid manager */
    st = createExactMatchManager(exactMatchManagerId, EXACT_MATCH_SIZE_FULL, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "createExactMatchManager: expected to GT_OK on manager [%d]",
        exactMatchManagerId);

    /* add device to it */
    st = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChExactMatchManagerDevListAdd: expected to GT_OK on manager [%d]",
        exactMatchManagerId);

    /* Verify DB */
    st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                errorNumItr,
                testResultArray[errorNumItr],
                debugErrorArr[testResultArray[errorNumItr]].errString);
    }

    /* create another valid manager */
    st = createExactMatchManager(exactMatchManagerId+1, EXACT_MATCH_SIZE_FULL, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "createExactMatchManager: expected to GT_OK on manager [%d]",
        exactMatchManagerId);

    /* add device to it - the device was already added to a different manager */
    st = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId+1,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_ALREADY_EXIST, st,
        "cpssDxChExactMatchManagerDevListAdd: expected to GT_ALREADY_EXIST on manager [%d], since the devNum+portGroup was already added to manager[%d] ",
        exactMatchManagerId+1,exactMatchManagerId);

    /* add same device different group
       in case of multi device with more than one tile we expect  the test to pass
       in case of single device we expect the test to fail with GT_BAD_PARAM */
     pairListArr[0].portGroupsBmp = 0x4;

     st = prvCpssMultiPortGroupsBmpCheckSpecificUnit(pairListArr[0].devNum,&tempEm_portGroupsBmp,PRV_CPSS_DXCH_UNIT_EM_E);
     if(st != GT_OK)
     {
         expectedReturnValue = GT_BAD_PARAM;
     }
     else
     {
         /* according  to the values of temp bitMap we will decide if we expect the return value to be GT_OK or GT_BAD_PARAM */
         if (tempEm_portGroupsBmp!=1)
         {
             /* EM have a shared table per one tile (2 pipes share the same table)
                so if we get the value 1 back from prvCpssMultiPortGroupsBmpCheckSpecificUnit
                it means we have a single tile device and any value of portGroupBmp will be ok
                - the function know how to translate the portGroupBmp to the correct value */
            if(pairListArr[0].portGroupsBmp & (~tempEm_portGroupsBmp))
            {
                expectedReturnValue = GT_BAD_PARAM;
            }
         }
         else
         {
             st = prvCpssMultiPortGroupsBmpCheckSpecificUnit(pairListArr[0].devNum,&tempTcam_portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
             if(st != GT_OK)
             {
                 expectedReturnValue = GT_BAD_PARAM;
             }
             else
             {
                 if (tempTcam_portGroupsBmp!=1)
                 {
                     /* TCAM have a shared table per one tile (2 pipes share the same table)
                        so if we get the value 1 back from prvCpssMultiPortGroupsBmpCheckSpecificUnit
                        it means we have a single tile device and any value of portGroupBmp will be ok
                        - the function know how to translate the portGroupBmp to the correct value */
                    if(pairListArr[0].portGroupsBmp & (~tempTcam_portGroupsBmp))
                    {
                        expectedReturnValue = GT_BAD_PARAM;
                    }
                 }
                 else
                 {
                     st = prvCpssMultiPortGroupsBmpCheckSpecificUnit(pairListArr[0].devNum,&tempPcl_portGroupsBmp,PRV_CPSS_DXCH_UNIT_PCL_E);
                     if(st != GT_OK)
                     {
                         expectedReturnValue = GT_BAD_PARAM;
                     }
                     else
                     {
                        /* For a single tile we have 2 tables for PCL one for each pipe - bits 0 and 1 is on in the bitmap*/
                        if(pairListArr[0].portGroupsBmp & (~tempPcl_portGroupsBmp))
                        {
                            expectedReturnValue = GT_BAD_PARAM;
                        }
                        else
                        {
                             st = prvCpssMultiPortGroupsBmpCheckSpecificUnit(pairListArr[0].devNum,&tempTti_portGroupsBmp,PRV_CPSS_DXCH_UNIT_TTI_E);
                             if(st != GT_OK)
                             {
                                 expectedReturnValue = GT_BAD_PARAM;
                             }
                             else
                             {
                                /* For a single tile we have 2 tables for TTI one for each pipe - bits 0 and 1 is on in the bitmap*/
                                if(pairListArr[0].portGroupsBmp & (~tempTti_portGroupsBmp))
                                {
                                    expectedReturnValue = GT_BAD_PARAM;
                                }
                             }
                        }
                     }
                }
             }
         }
     }

     st = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId + 1, pairListArr, numOfPairs);
     UTF_VERIFY_EQUAL2_STRING_MAC(expectedReturnValue, st,
        "cpssDxChExactMatchManagerDevListAdd: expected to [%d] on manager [%d]",
        expectedReturnValue,exactMatchManagerId+1);

   /* Verify DB */
    st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId+1, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                errorNumItr,
                testResultArray[errorNumItr],
                debugErrorArr[testResultArray[errorNumItr]].errString);
    }

    numOfPairs=1;
    st = cpssDxChExactMatchManagerDevListRemove(exactMatchManagerId+1, pairListArr, numOfPairs);
    UTF_VERIFY_EQUAL2_STRING_MAC(expectedReturnValue, st,
        "cpssDxChExactMatchManagerDevListAdd: expected to [%d] on manager [%d]",
        expectedReturnValue,exactMatchManagerId);
     CLEANUP_MANAGER(exactMatchManagerId+1);

    /* add same device with different portGroup to exactMatchManagerId */
    st = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL2_STRING_MAC(expectedReturnValue, st,
        "cpssDxChExactMatchManagerDevListAdd: expected to [%d] on manager [%d]",
        expectedReturnValue,exactMatchManagerId);
    /* remove it */
    st = cpssDxChExactMatchManagerDevListRemove(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL2_STRING_MAC(expectedReturnValue, st,
        "cpssDxChExactMatchManagerDevListAdd: expected to [%d] on manager [%d]",
        expectedReturnValue,exactMatchManagerId);
    /* try to remove it again */
    if (expectedReturnValue==GT_OK)
    {
        expectedReturnValue = GT_NO_SUCH;
    }
    st = cpssDxChExactMatchManagerDevListRemove(exactMatchManagerId, pairListArr, numOfPairs);
    UTF_VERIFY_EQUAL2_STRING_MAC(expectedReturnValue, st,
        "cpssDxChExactMatchManagerDevListAdd: expected to [%d] on manager [%d]",
        expectedReturnValue,exactMatchManagerId);

    pairListArr[0].portGroupsBmp = 0x1;

    /* re-add device to it */
    st = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_ALREADY_EXIST, st,
        "cpssDxChExactMatchManagerDevListAdd: expected to GT_ALREADY_EXIST on manager [%d]",
        exactMatchManagerId);

    /* remove the device from it */
    st = cpssDxChExactMatchManagerDevListRemove(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChExactMatchManagerDevListRemove: expected to GT_OK on manager [%d]",
        exactMatchManagerId);


    /* add 2 of the same device */
    pairListArr[1].devNum = pairListArr[0].devNum;
    pairListArr[0].portGroupsBmp = 0x1;
    pairListArr[1].portGroupsBmp = 0x1;
    numOfPairs = 2;

    st = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
        "cpssDxChExactMatchManagerDevListAdd: expected to GT_BAD_PARAM on manager [%d]",
        exactMatchManagerId);

    /* add valid device and non exist device */
    pairListArr[1].devNum = 255;/*8 bits value*/
    pairListArr[0].portGroupsBmp = 0x1;
    pairListArr[1].portGroupsBmp = 0x1;
    numOfPairs = 2;

    st = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
        "cpssDxChExactMatchManagerDevListAdd: expected to GT_BAD_PARAM on manager [%d]",
        exactMatchManagerId);

    /* add valid device with different port groups */
    pairListArr[1].devNum = pairListArr[0].devNum;
    pairListArr[0].portGroupsBmp = 0x1;
    pairListArr[1].portGroupsBmp = 0x4;
    numOfPairs = 2;
    expectedReturnValue=GT_OK;

    /* according  to the values of temp bitMap we will decide if we expect the return value to be GT_OK or GT_BAD_PARAM */
     if (tempEm_portGroupsBmp==1)
     {
         /* EM have a shared table per one tile (2 pipes share the same table)
            so if we get the value 1 back from the macro it means we have a single tile device
            and any value of portGroupBmp will be ok - the macro know how to translate the
            portGroupBmp to the correct value*/
         expectedReturnValue=GT_OK;

         /* continue to check ...*/
         if (tempTcam_portGroupsBmp==1)
         {
             /* TCAM have a shared table per one tile (2 pipes share the same table)
                so if we get the value 1 back from the macro it means we have a single tile device
                and any value of portGroupBmp will be ok - the macro know how to translate the
                portGroupBmp to the correct value*/
             expectedReturnValue=GT_OK;

             /* continue to check ...*/
             /* For a single tile we have 2 tables for TTI/PCL one for each pipe - bits 0 and 1 is on in the bitmap*/
            if((pairListArr[1].portGroupsBmp & (~tempPcl_portGroupsBmp))||
               (pairListArr[1].portGroupsBmp & (~tempTti_portGroupsBmp)))
            {
                expectedReturnValue = GT_BAD_PARAM;
            }
            else
            {
                expectedReturnValue=GT_OK;
            }
         }
         else
         {
            if(pairListArr[1].portGroupsBmp & (~tempTcam_portGroupsBmp))
            {
                expectedReturnValue = GT_BAD_PARAM;
            }
         }
     }
     else
     {
        if(pairListArr[1].portGroupsBmp & (~tempEm_portGroupsBmp))
        {
            expectedReturnValue = GT_BAD_PARAM;
        }
     }


    st = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL2_STRING_MAC(expectedReturnValue, st,
        "cpssDxChExactMatchManagerDevListAdd: expected to [%d] on manager [%d]",
        expectedReturnValue,exactMatchManagerId);

    st = cpssDxChExactMatchManagerDevListRemove(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL2_STRING_MAC(expectedReturnValue, st,
        "cpssDxChExactMatchManagerDevListRemove: expected to [%d] on manager [%d]",
        expectedReturnValue,exactMatchManagerId);

    /**************************************************/
    /* check that we can still add valid device to it */
    /**************************************************/
    numOfPairs     = 1;
    /* add device to it */
    st = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChExactMatchManagerDevListAdd: expected to GT_OK on manager [%d]",
        exactMatchManagerId);
    /* remove the device from it */
    st = cpssDxChExactMatchManagerDevListRemove(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChExactMatchManagerDevListRemove: expected to GT_OK on manager [%d]",
        exactMatchManagerId);


    /* NULL pointer */
    st = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId,NULL,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChExactMatchManagerDevListAdd: expected to GT_BAD_PTR on manager [%d]",
        exactMatchManagerId);
    /* 0 devices */
    st = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
        "cpssDxChExactMatchManagerDevListAdd: expected to GT_BAD_PARAM on manager [%d]",
        exactMatchManagerId);
}

/**
GT_STATUS cpssDxChExactMatchManagerDevListAdd
(
    IN GT_U32                                                   exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[],
    IN GT_U32                                                   numOfPairs
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerDevListAdd)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    internal_cpssDxChExactMatchManagerDevListAddUT();

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}

/**
GT_STATUS internal_cpssDxChExactMatchManagerDevListRemoveUT (
(
    IN GT_U32                                                   exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[],
    IN GT_U32                                                   numOfPairs
);
*/
static void internal_cpssDxChExactMatchManagerDevListRemoveUT(void)
{
    GT_STATUS   st=GT_OK;
    GT_U32      ii;
    GT_U32      exactMatchManagerId = TESTED_EXACT_MATCH_MANAGER_ID_CNS;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC pairListArr[2];
    GT_U32      numOfPairs = 1;

    pairListArr[0].devNum = prvTgfDevNum;
    pairListArr[0].portGroupsBmp = 0x1;

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        st = cpssDxChExactMatchManagerDevListRemove(ii,pairListArr,numOfPairs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChExactMatchManagerDevListRemove: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    /* check out of range managers ... expect 'GT_OUT_OF_RANGE' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        st = cpssDxChExactMatchManagerDevListRemove(ii,pairListArr,numOfPairs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
            "cpssDxChExactMatchManagerDevListRemove: expected to GT_OUT_OF_RANGE on manager [%d]",
            ii);
    }

    /* create single valid manager */
    st = createExactMatchManager(exactMatchManagerId, EXACT_MATCH_SIZE_FULL, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "createExactMatchManager: expected to GT_OK on manager [%d]",
        exactMatchManagerId);

    /* remove a device from it (device not in DB) */
    st = cpssDxChExactMatchManagerDevListRemove(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_SUCH, st,
        "cpssDxChExactMatchManagerDevListRemove: expected to GT_NO_SUCH on manager [%d]",
        exactMatchManagerId);


    /* add a device */
    st = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChExactMatchManagerDevListAdd: expected to GT_OK on manager [%d]",
        exactMatchManagerId);

    /* remove valid device and non exist device */
    pairListArr[1].devNum = 225;/*8 bits value - bad parameter */
    pairListArr[1].portGroupsBmp=0x1;
    numOfPairs     = 2;

    st = cpssDxChExactMatchManagerDevListRemove(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
        "cpssDxChExactMatchManagerDevListAdd: expected to GT_NO_SUCH on manager [%d]",
        exactMatchManagerId);

    /*****************************************************/
    /* check that we can still remove valid device to it */
    /*****************************************************/
    numOfPairs = 1;
    st = cpssDxChExactMatchManagerDevListRemove(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChExactMatchManagerDevListRemove: expected to GT_OK on manager [%d]",
        exactMatchManagerId);

    /* NULL pointer */
    st = cpssDxChExactMatchManagerDevListRemove(exactMatchManagerId,NULL,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChExactMatchManagerDevListRemove: expected to GT_BAD_PTR on manager [%d]",
        exactMatchManagerId);
    /* 0 devices */
    st = cpssDxChExactMatchManagerDevListRemove(exactMatchManagerId,pairListArr,0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
        "cpssDxChExactMatchManagerDevListRemove: expected to GT_BAD_PARAM on manager [%d]",
        exactMatchManagerId);
}
/**
GT_STATUS cpssDxChExactMatchManagerDevListRemove
(
    IN GT_U32 exactMatchManagerId,
    IN GT_U8  devListArr[],
    IN GT_U32 numOfDevs
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerDevListRemove)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    internal_cpssDxChExactMatchManagerDevListRemoveUT();

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}

/**
GT_STATUS internal_cpssDxChExactMatchManagerDevListGetUT
(
    IN    GT_U32                                                   exactMatchManagerId,
    INOUT GT_U32                                                   *numOfPairsPtr,
    OUT   CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListManagedArray[]
);
*/
static void internal_cpssDxChExactMatchManagerDevListGetUT(void)
{
    GT_STATUS   st=GT_OK, expectedReturnValue=GT_OK;
    GT_U32      ii;
    GT_U32      exactMatchManagerId = TESTED_EXACT_MATCH_MANAGER_ID_CNS;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC pairListArr[2];
    CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC pairListArrGet[2];
    GT_U32      numOfPairs;
    GT_PORT_GROUPS_BMP tempTti_portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_PORT_GROUPS_BMP tempEm_portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_PORT_GROUPS_BMP tempPcl_portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_PORT_GROUPS_BMP tempTcam_portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;



    pairListArr[0].devNum = prvTgfDevNum;
    pairListArr[1].devNum = prvTgfDevNum;
    pairListArr[0].portGroupsBmp = 0x1;
    pairListArr[1].portGroupsBmp = 0x4;

    pairListArrGet[0].devNum = 0xAB;
    pairListArrGet[0].portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    pairListArrGet[1].devNum = 0xCD;
    pairListArrGet[1].portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;


    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        st = cpssDxChExactMatchManagerDevListGet(ii, &numOfPairs, pairListArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChExactMatchManagerDevListGet: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    /* check out of range managers ... expect 'GT_OUT_OF_RANGE' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        st = cpssDxChExactMatchManagerDevListGet(ii, &numOfPairs, pairListArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
            "cpssDxChExactMatchManagerDevListGet: expected to GT_OUT_OF_RANGE on manager [%d]",
            ii);
    }

    /* create single valid manager */
    st = createExactMatchManager(exactMatchManagerId, EXACT_MATCH_SIZE_FULL, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "createExactMatchManager: expected to GT_OK on manager [%d]",
        exactMatchManagerId);

    numOfPairs = 1;

    /* add one device */
    st = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChExactMatchManagerDevListAdd: expected to GT_OK on manager [%d]",
        exactMatchManagerId);

    /*****************************************************/
    /* check that we can retrieve one valid device       */
    /*****************************************************/
    numOfPairs = 2;
    st = cpssDxChExactMatchManagerDevListGet(exactMatchManagerId, &numOfPairs, pairListArrGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChExactMatchManagerDevListGet: expected to GT_OK on manager [%d]",
        exactMatchManagerId);

    if (st == GT_OK)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(1, numOfPairs,
            "cpssDxChExactMatchManagerDevListGet: expected to one added device [%d]",
            exactMatchManagerId);

        UTF_VERIFY_EQUAL1_STRING_MAC(pairListArr[0].devNum, pairListArrGet[0].devNum,
            "cpssDxChExactMatchManagerDevListGet: expected to device ID[%d]",
            pairListArr[0].devNum);

        UTF_VERIFY_EQUAL1_STRING_MAC(pairListArr[0].portGroupsBmp, pairListArrGet[0].portGroupsBmp,
            "cpssDxChExactMatchManagerDevListGet: expected to device portGroupsBmp[%d]",
            pairListArr[0].portGroupsBmp);

        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(pairListArr[1].devNum, pairListArrGet[1].devNum,
            "cpssDxChExactMatchManagerDevListGet: not expected to more than one device[%d]",
            exactMatchManagerId);
    }

    /* remove it */
    st = cpssDxChExactMatchManagerDevListRemove(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChExactMatchManagerDevListAdd: expected to GT_OK on manager [%d]",
        exactMatchManagerId);

    /* add 2 devices (same device different port group) so in the get
       it should be merged to one device with extended portGroup - this will work for multi tile device */
    numOfPairs = 2;

    st = prvCpssMultiPortGroupsBmpCheckSpecificUnit(pairListArr[0].devNum,&tempEm_portGroupsBmp,PRV_CPSS_DXCH_UNIT_EM_E);
    if(st != GT_OK)
    {
        expectedReturnValue = GT_BAD_PARAM;
    }
    else
    {
        /* according  to the values of temp bitMap we will decide if we expect the return value to be GT_OK or GT_BAD_PARAM */
        if (tempEm_portGroupsBmp!=1)
        {
            /* EM have a shared table per one tile (2 pipes share the same table)
               so if we get the value 1 back from prvCpssMultiPortGroupsBmpCheckSpecificUnit
               it means we have a single tile device and any value of portGroupBmp will be ok
               - the function know how to translate the portGroupBmp to the correct value */
           if(pairListArr[1].portGroupsBmp & (~tempEm_portGroupsBmp))
           {
               expectedReturnValue = GT_BAD_PARAM;
           }
        }
        else
        {
            st = prvCpssMultiPortGroupsBmpCheckSpecificUnit(pairListArr[0].devNum,&tempTcam_portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
            if(st != GT_OK)
            {
                expectedReturnValue = GT_BAD_PARAM;
            }
            else
            {
                if (tempTcam_portGroupsBmp!=1)
                {
                    /* TCAM have a shared table per one tile (2 pipes share the same table)
                       so if we get the value 1 back from prvCpssMultiPortGroupsBmpCheckSpecificUnit
                       it means we have a single tile device and any value of portGroupBmp will be ok
                       - the function know how to translate the portGroupBmp to the correct value */
                   if(pairListArr[1].portGroupsBmp & (~tempTcam_portGroupsBmp))
                   {
                       expectedReturnValue = GT_BAD_PARAM;
                   }
                }
                else
                {
                    st = prvCpssMultiPortGroupsBmpCheckSpecificUnit(pairListArr[0].devNum,&tempPcl_portGroupsBmp,PRV_CPSS_DXCH_UNIT_PCL_E);
                    if(st != GT_OK)
                    {
                        expectedReturnValue = GT_BAD_PARAM;
                    }
                    else
                    {
                       /* For a single tile we have 2 tables for PCL one for each pipe - bits 0 and 1 is on in the bitmap*/
                       if(pairListArr[1].portGroupsBmp & (~tempPcl_portGroupsBmp))
                       {
                           expectedReturnValue = GT_BAD_PARAM;
                       }
                       else
                       {
                            st = prvCpssMultiPortGroupsBmpCheckSpecificUnit(pairListArr[0].devNum,&tempTti_portGroupsBmp,PRV_CPSS_DXCH_UNIT_TTI_E);
                            if(st != GT_OK)
                            {
                                expectedReturnValue = GT_BAD_PARAM;
                            }
                            else
                            {
                               /* For a single tile we have 2 tables for TTI one for each pipe - bits 0 and 1 is on in the bitmap*/
                               if(pairListArr[1].portGroupsBmp & (~tempTti_portGroupsBmp))
                               {
                                   expectedReturnValue = GT_BAD_PARAM;
                               }
                            }
                       }
                    }
               }
            }
        }
    }

    st = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
    UTF_VERIFY_EQUAL2_STRING_MAC(expectedReturnValue, st,
        "cpssDxChExactMatchManagerDevListAdd: expected to [%d] on manager [%d]",
        expectedReturnValue,exactMatchManagerId);

    pairListArrGet[0].devNum = 0xAB;
    pairListArrGet[0].portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    pairListArrGet[1].devNum = 0xCD;
    pairListArrGet[1].portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    st = cpssDxChExactMatchManagerDevListGet(exactMatchManagerId, &numOfPairs, pairListArrGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChExactMatchManagerDevListGet: expected to GT_OK on manager [%d]",
        exactMatchManagerId);

    if (st == GT_OK)
    {
        if (expectedReturnValue==GT_BAD_PARAM)
        {
            /* this is the case that we do not succed in adding the second portGroupBmp since we are
               a single tile device - so the entire add fail and no device is added --> no device in the get operation */
            UTF_VERIFY_EQUAL1_STRING_MAC(0, numOfPairs,
            "cpssDxChExactMatchManagerDevListGet: expected to one added device [%d]",
            exactMatchManagerId);

            /* remove it numOfPairs=0 should get GT_BAD_PARAM */
            st = cpssDxChExactMatchManagerDevListRemove(exactMatchManagerId,pairListArr,numOfPairs);
            UTF_VERIFY_EQUAL2_STRING_MAC(expectedReturnValue, st,
                "cpssDxChExactMatchManagerDevListAdd: expected to [%d] on manager [%d]",
                expectedReturnValue,exactMatchManagerId);
        }
        else
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(1, numOfPairs,
            "cpssDxChExactMatchManagerDevListGet: expected to one added device [%d]",
            exactMatchManagerId);

            UTF_VERIFY_EQUAL1_STRING_MAC(pairListArr[0].devNum, pairListArrGet[0].devNum,
            "cpssDxChExactMatchManagerDevListGet: expected to device ID[%d]",
            pairListArr[0].devNum);

            UTF_VERIFY_EQUAL1_STRING_MAC((pairListArr[0].portGroupsBmp|pairListArr[1].portGroupsBmp), pairListArrGet[0].portGroupsBmp,
                                         "cpssDxChExactMatchManagerDevListGet: expected to device portGroupsBmp[%d]",
                                         pairListArr[0].portGroupsBmp);

             /* remove it */
            st = cpssDxChExactMatchManagerDevListRemove(exactMatchManagerId,pairListArr,numOfPairs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                "cpssDxChExactMatchManagerDevListAdd: expected to [%d] on manager [%d]",
                expectedReturnValue,exactMatchManagerId);
        }
    }

    /* NULL pointer */
    st = cpssDxChExactMatchManagerDevListGet(exactMatchManagerId, &numOfPairs, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChExactMatchManagerDevListGet: expected to GT_BAD_PTR on manager [%d]",
        exactMatchManagerId);

    /* 0 devices */
    st = cpssDxChExactMatchManagerDevListGet(exactMatchManagerId, NULL, pairListArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChExactMatchManagerDevListGet: expected to GT_BAD_PTR on manager [%d]",
        exactMatchManagerId);
}

/**
GT_STATUS cpssDxChExactMatchManagerDevListGet
(
    IN    GT_U32                                                   exactMatchManagerId,
    INOUT GT_U32                                                   *numOfPairsPtr,
    OUT   CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListManagedArray[]
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerDevListGet)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    internal_cpssDxChExactMatchManagerDevListGetUT();

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}


/* indication that the test should run without device bound to the manager            */
/* the default is run with device from the start (all entries inserted to the device) */
static GT_BOOL  run_device_less = GT_FALSE;
/* indication that the test should run add device in runtime to the manager           */
/* the default is run with device from the start (all entries inserted to the device) */
static GT_BOOL  add_device_in_runtime = GT_FALSE;
/* indication that the test should check for entries to exist in the HW               */
/* the default is not to check it , because it slow done the test                     */
static GT_BOOL  check_hw_hold_entry = GT_FALSE;

/* allow to debug limited number of entries inserted to the manager */
static GT_U32   em_debug_limited_entries = 0;
GT_STATUS  em_debug_limited_entries_set(IN GT_U32  numEntries)
{
    em_debug_limited_entries = numEntries ?
            /* rounded UP to nearest 256 ... otherwise fail to 'create' manager */
            (((NUM_ENTRIES_STEPS_CNS-1)+numEntries)/NUM_ENTRIES_STEPS_CNS)*NUM_ENTRIES_STEPS_CNS :
            /* set to ZERO --> disable the 'debug' */
            0;
    return GT_OK;
}

/* create Exact Match manager - minimal parameters for tests */
GT_STATUS createExactMatchManagerWithDevice(
    IN GT_U32                                           exactMatchManagerId,
    IN GT_U32                                           maxTotalEntries,/* up to 128K (limited by numOfHwIndexes) */
    IN GT_U8                                            devNum,
    IN GT_PORT_GROUPS_BMP                               portGroupsBmp,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_HW_CAPACITY_STC    *hwCapacity
)
{
    GT_STATUS                                               st;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC   pairListArr[1];
    GT_U32                                                  numOfPairs=1;

    pairListArr[0].devNum=devNum;
    pairListArr[0].portGroupsBmp=portGroupsBmp;

    /* create single valid manager */
    st = createExactMatchManager(exactMatchManagerId, maxTotalEntries, hwCapacity);
    if (st==GT_OK)
    {
        if (run_device_less == GT_TRUE)
        {
            /* we do not add the device to the manager */
            return GT_OK;
        }

        /* add device to it */
        st = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
        if (st!=GT_OK)
        {
            PRV_UTF_LOG0_MAC("Fail to create a manager with device - cpssDxChExactMatchManagerDevListAdd fail");
            return st;
        }
    }
    else
    {
        PRV_UTF_LOG0_MAC("Fail to create a manager with device");
    }
    return st;
}


#define UT_EXACT_MATCH_ENTRY_TYPE_VALID_NUM 2 /*3*/
static CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT exactMatchEntryType_valid_arr[UT_EXACT_MATCH_ENTRY_TYPE_VALID_NUM+1] =
{
      CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E
    , CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E
    /*, CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E - currently UT working only on TTI and PCL - 2 lookups */
    , GT_NA
};

/* add exact match entry to the manager :destination common info */
static void exactMatchManagerEntryBuild_DstInterfaceInfo(
    IN  GT_U32                  rand32Bits,
    OUT CPSS_INTERFACE_INFO_STC *dstInterfacePtr
)
{
    cpssOsMemSet(dstInterfacePtr, 0, sizeof(CPSS_INTERFACE_INFO_STC));

    dstInterfacePtr->type         = rand32Bits % 3;

    switch(dstInterfacePtr->type)
    {
        case CPSS_INTERFACE_PORT_E:
            dstInterfacePtr->devPort.hwDevNum   = (rand32Bits >> 1) & 0x3FF;    /* 10 bits */
            dstInterfacePtr->devPort.portNum    = (rand32Bits >> 2) & 0x1FFF;   /* 13 bits */
            break;
        case CPSS_INTERFACE_TRUNK_E:
            dstInterfacePtr->trunkId = (rand32Bits >> 3) & 0xFFF; /* 12 bits */
            break;
        case CPSS_INTERFACE_VIDX_E:
            dstInterfacePtr->vidx    = (rand32Bits >> 4) & 0x3FFF; /* 14 bits */
            break;
        default:
            break;
    }
}

/* add exact match entry to the manager : type :tti */
static void specificExactMatchManagerEntryAdd(
    IN  GT_U32                                              exactMatchManagerId,
    IN  GT_U32                                              rand32Bits,
    IN  CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               actionType,
    UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC    *outputResultsPtr
)
{
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                     entryGet;
    static CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC   exactMatchEntryAddInfo;
    static GT_BOOL isFirstTime = GT_TRUE;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                     *entryPtr;
    GT_BOOL                                                     *entryAddedPtr;
    GT_STATUS                                                   st;
    GT_U32                                                      value;

    entryPtr        = outputResultsPtr->entryPtr;
    entryAddedPtr   = outputResultsPtr->entryAddedPtr;

    if(isFirstTime)
    {
        cpssOsMemSet(&exactMatchEntryAddInfo,0,sizeof(exactMatchEntryAddInfo));
        isFirstTime = GT_FALSE;
        exactMatchEntryAddInfo.rehashEnable    = GT_FALSE;
    }

    entryPtr->exactMatchActionType=actionType;
    prvSetExactMatchActionDefaultValues(&entryPtr->exactMatchAction,actionType);

    /* set the key */
    switch(actionType)
    {
    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
        entryPtr->exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E; /* first lookup is set to TTI */
        entryPtr->expandedActionIndex = rand32Bits & 0x7; /* expanded entries 0-7 are configured for TTI */
        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
        entryPtr->exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E; /* second lookup is set to PCL */
        value = rand32Bits & 0xf;
        entryPtr->expandedActionIndex = (value < 8) ? 8 : value; /* expanded entries 8-15 are configured for PCL */
        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:/* UT is written with TTI and IPCL configuration */
    default:
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_BAD_PARAM,
            "exactMatchManagerEntryAdd: actionType for manager [%d]",
            exactMatchManagerId);
    }

    entryPtr->exactMatchUserDefined = rand32Bits & 0xFFFF;/* 16 bits */

    if (actionType==CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E)
    {
        value = (rand32Bits & 0x3)%3; /*  bits - legal values for TTI 0-2 */
    }
    else
    {
        value = rand32Bits & 0x3; /* 2 bits - legal values 0-3 */
    }
    cpssOsMemSet(entryPtr->exactMatchEntry.key.pattern,0,sizeof(entryPtr->exactMatchEntry.key.pattern));
    switch (value)
    {
        case 0:
            entryPtr->exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
            entryPtr->exactMatchEntry.key.pattern[0]=(GT_U8)(rand32Bits >> 24);
            entryPtr->exactMatchEntry.key.pattern[1]=(GT_U8)(rand32Bits >> 16);
            entryPtr->exactMatchEntry.key.pattern[2]=(GT_U8)(rand32Bits >>  8);
            entryPtr->exactMatchEntry.key.pattern[3]=(GT_U8)(rand32Bits >>  0);

            break;
        case 1:
            entryPtr->exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
            entryPtr->exactMatchEntry.key.pattern[0]=(GT_U8)(rand32Bits >> 24);
            entryPtr->exactMatchEntry.key.pattern[1]=(GT_U8)(rand32Bits >> 16);
            entryPtr->exactMatchEntry.key.pattern[2]=(GT_U8)(rand32Bits >>  8);
            entryPtr->exactMatchEntry.key.pattern[3]=(GT_U8)(rand32Bits >>  0);

            entryPtr->exactMatchEntry.key.pattern[15]=(GT_U8)(rand32Bits >> 24);
            entryPtr->exactMatchEntry.key.pattern[16]=(GT_U8)(rand32Bits >> 16);
            entryPtr->exactMatchEntry.key.pattern[17]=(GT_U8)(rand32Bits >>  8);
            entryPtr->exactMatchEntry.key.pattern[18]=(GT_U8)(rand32Bits >>  0);
            break;
        case 2:
            entryPtr->exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
            entryPtr->exactMatchEntry.key.pattern[0]=(GT_U8)(rand32Bits >> 24);
            entryPtr->exactMatchEntry.key.pattern[1]=(GT_U8)(rand32Bits >> 16);
            entryPtr->exactMatchEntry.key.pattern[2]=(GT_U8)(rand32Bits >>  8);
            entryPtr->exactMatchEntry.key.pattern[3]=(GT_U8)(rand32Bits >>  0);

            entryPtr->exactMatchEntry.key.pattern[29]=(GT_U8)(rand32Bits >> 24);
            entryPtr->exactMatchEntry.key.pattern[30]=(GT_U8)(rand32Bits >> 16);
            entryPtr->exactMatchEntry.key.pattern[31]=(GT_U8)(rand32Bits >>  8);
            entryPtr->exactMatchEntry.key.pattern[32]=(GT_U8)(rand32Bits >>  0);
            break;
        case 3:
            entryPtr->exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
            entryPtr->exactMatchEntry.key.pattern[0]=(GT_U8)(rand32Bits >> 24);
            entryPtr->exactMatchEntry.key.pattern[1]=(GT_U8)(rand32Bits >> 16);
            entryPtr->exactMatchEntry.key.pattern[2]=(GT_U8)(rand32Bits >>  8);
            entryPtr->exactMatchEntry.key.pattern[3]=(GT_U8)(rand32Bits >>  0);

            entryPtr->exactMatchEntry.key.pattern[43]=(GT_U8)(rand32Bits >> 24);
            entryPtr->exactMatchEntry.key.pattern[44]=(GT_U8)(rand32Bits >> 16);
            entryPtr->exactMatchEntry.key.pattern[45]=(GT_U8)(rand32Bits >>  8);
            entryPtr->exactMatchEntry.key.pattern[46]=(GT_U8)(rand32Bits >>  0);
            break;
    }

    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId, entryPtr, &exactMatchEntryAddInfo, st);
    if (st == GT_OK)
    {
        *entryAddedPtr = GT_TRUE;
    }
    else if (st != GT_FULL)
    {
       UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "exactMatchManagerEntryAdd: unexpected error in manager [%d]",
            exactMatchManagerId);
    }

    if((*entryAddedPtr) == GT_TRUE)
    {
        /* the entry was added ... check if the entry retrieved as was set */

        /* start -- the parts of the key */
        cpssOsMemSet(&entryGet,0,sizeof(entryGet));
        cpssOsMemCpy(&entryGet.exactMatchEntry.lookupNum,&entryPtr->exactMatchEntry.lookupNum,sizeof(entryPtr->exactMatchEntry.lookupNum));
        cpssOsMemCpy(&entryGet.exactMatchEntry.key.keySize,&entryPtr->exactMatchEntry.key.keySize,sizeof(entryPtr->exactMatchEntry.key.keySize));
        cpssOsMemCpy(&entryGet.exactMatchEntry.key.pattern,&entryPtr->exactMatchEntry.key.pattern,sizeof(entryPtr->exactMatchEntry.key.pattern));

        /* end -- the parts of the key */
        entryGet.exactMatchActionType = actionType;

        st = cpssDxChExactMatchManagerEntryGet(exactMatchManagerId,&entryGet);
        if (st!=GT_OK)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                        "cpssDxChExactMatchManagerEntryGet: unexpected error in manager [%d]",
                                        exactMatchManagerId);
        }

        /***************************************************************/
        /* compare the full entry */
        /***************************************************************/
       st = prvCompareExactMatchActionStructs(prvTgfDevNum, "action", actionType, &entryPtr->exactMatchAction, &entryGet.exactMatchAction);
       UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",prvTgfDevNum, entryPtr->expandedActionIndex, actionType);
    }
    return;
}

/* add exact match entry to the manager : type :tti */
static void exactMatchManagerEntryAdd(
    IN  GT_U32                                              exactMatchManagerId,
    IN  GT_U32                                              iterationNum,
    IN  CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               actionType,
    UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC    *outputResultsPtr
)
{
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                     entryGet;
    static CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC   exactMatchEntryAddInfo;
    static GT_BOOL isFirstTime = GT_TRUE;
    GT_U32  rand32Bits; /* random value in 32 bits (random 30 bits + 2 bits from iterationNum) */
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                     *entryPtr;
    GT_BOOL                                                     *entryAddedPtr;
    GT_STATUS                                                   st;
    GT_U32                                                      value;

    entryPtr        = outputResultsPtr->entryPtr;
    entryAddedPtr   = outputResultsPtr->entryAddedPtr;

    if(isFirstTime)
    {
        cpssOsMemSet(&exactMatchEntryAddInfo,0,sizeof(exactMatchEntryAddInfo));
        isFirstTime = GT_FALSE;
        exactMatchEntryAddInfo.rehashEnable    = GT_FALSE;
    }

    entryPtr->exactMatchActionType=actionType;
    prvSetExactMatchActionDefaultValues(&entryPtr->exactMatchAction,actionType);

    /* since we are using cpssOsRand - we migth have a case that we try to add
       an entry that was already added, so we need to try another cpssOsRand */
    while (GT_TRUE)
    {
        rand32Bits = (cpssOsRand() & 0x7FFF) << 15 |
                     (cpssOsRand() & 0x7FFF)       |
                     iterationNum << 30;

        /* set the key */
        switch(actionType)
        {
        case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
            entryPtr->exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E; /* first lookup is set to TTI */
            entryPtr->expandedActionIndex = cpssOsRand() & 0x7; /* expanded entries 0-7 are configured for TTI */
            break;
        case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
            entryPtr->exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E; /* second lookup is set to PCL */
            value = cpssOsRand() & 0xf;
            entryPtr->expandedActionIndex = (value < 8) ? 8 : value; /* expanded entries 8-15 are configured for PCL */
            break;
        case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:/* UT is written with TTI and IPCL configuration */
        default:
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_BAD_PARAM,
                "exactMatchManagerEntryAdd: actionType for manager [%d]",
                exactMatchManagerId);
        }

        entryPtr->exactMatchUserDefined = cpssOsRand() & 0xFFFF;/* 16 bits */

        if (actionType==CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E)
        {
            value = (cpssOsRand() & 0x3)%3; /*  bits - legal values for TTI 0-2 */
        }
        else
        {
            value = cpssOsRand() & 0x3; /* 2 bits - legal values 0-3 */
        }
        cpssOsMemSet(entryPtr->exactMatchEntry.key.pattern,0,sizeof(entryPtr->exactMatchEntry.key.pattern));
        switch (value)
        {
            case 0:
                entryPtr->exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
                entryPtr->exactMatchEntry.key.pattern[0]=(GT_U8)(rand32Bits >> 24);
                entryPtr->exactMatchEntry.key.pattern[1]=(GT_U8)(rand32Bits >> 16);
                entryPtr->exactMatchEntry.key.pattern[2]=(GT_U8)(rand32Bits >>  8);
                entryPtr->exactMatchEntry.key.pattern[3]=(GT_U8)(rand32Bits >>  0);

                break;
            case 1:
                entryPtr->exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
                entryPtr->exactMatchEntry.key.pattern[0]=(GT_U8)(rand32Bits >> 24);
                entryPtr->exactMatchEntry.key.pattern[1]=(GT_U8)(rand32Bits >> 16);
                entryPtr->exactMatchEntry.key.pattern[2]=(GT_U8)(rand32Bits >>  8);
                entryPtr->exactMatchEntry.key.pattern[3]=(GT_U8)(rand32Bits >>  0);

                entryPtr->exactMatchEntry.key.pattern[15]=(GT_U8)(rand32Bits >> 24);
                entryPtr->exactMatchEntry.key.pattern[16]=(GT_U8)(rand32Bits >> 16);
                entryPtr->exactMatchEntry.key.pattern[17]=(GT_U8)(rand32Bits >>  8);
                entryPtr->exactMatchEntry.key.pattern[18]=(GT_U8)(rand32Bits >>  0);
                break;
            case 2:
                entryPtr->exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
                entryPtr->exactMatchEntry.key.pattern[0]=(GT_U8)(rand32Bits >> 24);
                entryPtr->exactMatchEntry.key.pattern[1]=(GT_U8)(rand32Bits >> 16);
                entryPtr->exactMatchEntry.key.pattern[2]=(GT_U8)(rand32Bits >>  8);
                entryPtr->exactMatchEntry.key.pattern[3]=(GT_U8)(rand32Bits >>  0);

                entryPtr->exactMatchEntry.key.pattern[29]=(GT_U8)(rand32Bits >> 24);
                entryPtr->exactMatchEntry.key.pattern[30]=(GT_U8)(rand32Bits >> 16);
                entryPtr->exactMatchEntry.key.pattern[31]=(GT_U8)(rand32Bits >>  8);
                entryPtr->exactMatchEntry.key.pattern[32]=(GT_U8)(rand32Bits >>  0);
                break;
            case 3:
                entryPtr->exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
                entryPtr->exactMatchEntry.key.pattern[0]=(GT_U8)(rand32Bits >> 24);
                entryPtr->exactMatchEntry.key.pattern[1]=(GT_U8)(rand32Bits >> 16);
                entryPtr->exactMatchEntry.key.pattern[2]=(GT_U8)(rand32Bits >>  8);
                entryPtr->exactMatchEntry.key.pattern[3]=(GT_U8)(rand32Bits >>  0);

                entryPtr->exactMatchEntry.key.pattern[43]=(GT_U8)(rand32Bits >> 24);
                entryPtr->exactMatchEntry.key.pattern[44]=(GT_U8)(rand32Bits >> 16);
                entryPtr->exactMatchEntry.key.pattern[45]=(GT_U8)(rand32Bits >>  8);
                entryPtr->exactMatchEntry.key.pattern[46]=(GT_U8)(rand32Bits >>  0);
                break;
        }

        EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId, entryPtr, &exactMatchEntryAddInfo, st);
        if (st==GT_ALREADY_EXIST)
        {
            continue;
        }
        else
        {
            if (st == GT_OK)
            {
                *entryAddedPtr = GT_TRUE;
            }
            else if (st != GT_FULL)
            {
               UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "exactMatchManagerEntryAdd: unexpected error in manager [%d]",
                    exactMatchManagerId);
            }
        }

        if((*entryAddedPtr) == GT_TRUE)
        {
            /* the entry was added ... check if the entry retrieved as was set */

            /* start -- the parts of the key */
            cpssOsMemSet(&entryGet,0,sizeof(entryGet));
            cpssOsMemCpy(&entryGet.exactMatchEntry.lookupNum,&entryPtr->exactMatchEntry.lookupNum,sizeof(entryPtr->exactMatchEntry.lookupNum));
            cpssOsMemCpy(&entryGet.exactMatchEntry.key.keySize,&entryPtr->exactMatchEntry.key.keySize,sizeof(entryPtr->exactMatchEntry.key.keySize));
            cpssOsMemCpy(&entryGet.exactMatchEntry.key.pattern,&entryPtr->exactMatchEntry.key.pattern,sizeof(entryPtr->exactMatchEntry.key.pattern));

            /* end -- the parts of the key */
            entryGet.exactMatchActionType = actionType;

            st = cpssDxChExactMatchManagerEntryGet(exactMatchManagerId,&entryGet);
            if (st!=GT_OK)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                            "cpssDxChExactMatchManagerEntryGet: unexpected error in manager [%d]",
                                            exactMatchManagerId);
            }

            /***************************************************************/
            /* compare the full entry */
            /***************************************************************/
           st = prvCompareExactMatchActionStructs(prvTgfDevNum, "action", actionType, &entryPtr->exactMatchAction, &entryGet.exactMatchAction);
           UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",prvTgfDevNum, entryPtr->expandedActionIndex, actionType);
        }
        return;
    }
}

/* add exact match entry to the manager : type :tti */
static void exactMatchManagerEntryAdd_tti(
    IN  GT_U32                                              exactMatchManagerId,
    IN  GT_U32                                              iterationNum,
    UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC    *outputResultsPtr
)
{
    exactMatchManagerEntryAdd(exactMatchManagerId,iterationNum,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E,outputResultsPtr);
    return;
}
/* add exact match entry to the manager : type :pcl */
static void exactMatchManagerEntryAdd_pcl(
    IN  GT_U32                                              exactMatchManagerId,
    IN  GT_U32                                              iterationNum,
    UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC    *outputResultsPtr
)
{
    exactMatchManagerEntryAdd(exactMatchManagerId,iterationNum,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E,outputResultsPtr);
    return;
}
/* add exact match entry to the manager : type :epcl */
static void exactMatchManagerEntryAdd_epcl(
    IN  GT_U32                                              exactMatchManagerId,
    IN  GT_U32                                              iterationNum,
    UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC    *outputResultsPtr
)
{
    exactMatchManagerEntryAdd(exactMatchManagerId,iterationNum,CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E,outputResultsPtr);
    return;
}

/* add exact match entry to the manager */
static void exactMatchManagerEntryCuckooAdd(
    IN  GT_U32                                              exactMatchManagerId,
    IN  GT_U32                                              iterationNum,
    IN  CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               actionType,
    UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC    *outputResultsPtr,
    IN  GT_BOOL                                             isRandom,
    IN  CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT                  keySize,
    IN  GT_BOOL                                             skipValidity,
    IN  GT_BOOL                                             sameHash
)
{
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                     entryGet;
    static CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC   exactMatchEntryAddInfo;
    static GT_BOOL isFirstTime = GT_TRUE;
    GT_U32  rand32Bits; /* random value in 32 bits (random 30 bits + 2 bits from iterationNum) */
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                     *entryPtr;
    GT_BOOL                                                     *entryAddedPtr;
    GT_STATUS                                                   st;
    GT_U32                                                      value,i;

    entryPtr        = outputResultsPtr->entryPtr;
    entryAddedPtr   = outputResultsPtr->entryAddedPtr;

    if(isFirstTime)
    {
        cpssOsMemSet(&exactMatchEntryAddInfo,0,sizeof(exactMatchEntryAddInfo));
        isFirstTime = GT_FALSE;
        exactMatchEntryAddInfo.rehashEnable    = GT_TRUE;
    }

    entryPtr->exactMatchActionType=actionType;
    prvSetExactMatchActionDefaultValues(&entryPtr->exactMatchAction,actionType);

    /* since we are using cpssOsRand - we migth have a case that we try to add
       an entry that was already added, so we need to try another cpssOsRand */
    while (GT_TRUE)
    {
        if (isRandom) {
            rand32Bits = (cpssOsRand() & 0x7FFF) << 15 |
                         (cpssOsRand() & 0x7FFF)       |
                         iterationNum << 30;
        }
        else
        {
            rand32Bits = iterationNum;
        }

        /* set the key */
        switch(actionType)
        {
        case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
            entryPtr->exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E; /* first lookup is set to TTI */
            entryPtr->expandedActionIndex = cpssOsRand() & 0x7; /* expanded entries 0-7 are configured for TTI */
            break;
        case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
            entryPtr->exactMatchEntry.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E; /* second lookup is set to PCL */
            value = cpssOsRand() & 0xf;
            entryPtr->expandedActionIndex = (value < 8) ? 8 : value; /* expanded entries 8-15 are configured for PCL */
            break;
        case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:/* UT is written with TTI and IPCL configuration */
        default:
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, GT_BAD_PARAM,
                "exactMatchManagerEntryAdd: actionType for manager [%d]",
                exactMatchManagerId);
        }

        entryPtr->exactMatchUserDefined = cpssOsRand() & 0xFFFF;/* 16 bits */

        cpssOsMemSet(entryPtr->exactMatchEntry.key.pattern,0,sizeof(entryPtr->exactMatchEntry.key.pattern));
        if (isRandom)
        {
            switch (keySize)
            {

                case 0:
                    entryPtr->exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
                    for (i=0; i<5; i++)
                    {
                        if (skipValidity == GT_FALSE) {
                            entryPtr->exactMatchEntry.key.pattern[i]=(GT_U8)(rand32Bits >> (i%4)*8);
                        }
                        else
                        {
                            entryPtr->exactMatchEntry.key.pattern[i]=randomArr[iterationNum].data[i];
                        }
                    }
                    break;
                case 1:
                    entryPtr->exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
                    for (i=0; i<19; i++)
                    {
                        if (skipValidity == GT_FALSE)
                        {
                            entryPtr->exactMatchEntry.key.pattern[i]=(GT_U8)(rand32Bits >> (i%4)*8);
                        }
                        else
                        {
                            entryPtr->exactMatchEntry.key.pattern[i]=randomArr[iterationNum].data[i];
                        }
                    }
                    break;
                case 2:
                    entryPtr->exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
                    for (i=0; i<33; i++)
                    {
                        if (skipValidity == GT_FALSE)
                        {
                            entryPtr->exactMatchEntry.key.pattern[i]=(GT_U8)(rand32Bits >> (i%4)*8);
                        }
                        else
                        {
                            entryPtr->exactMatchEntry.key.pattern[i]=randomArr[iterationNum].data[i];
                        }
                    }
                    break;
                case 3:
                    entryPtr->exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
                    for (i=0; i<47; i++)
                    {
                        if (skipValidity == GT_FALSE)
                        {
                            entryPtr->exactMatchEntry.key.pattern[i]=(GT_U8)(rand32Bits >> (i%4)*8);
                        }
                        else
                        {
                            entryPtr->exactMatchEntry.key.pattern[i]=randomArr[iterationNum].data[i];
                        }
                    }
                    break;
                default:
                    break;
            }
        }
        else /* incremental */
        {
            entryPtr->exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;

            entryPtr->exactMatchEntry.key.pattern[0] = (rand32Bits & 0xF);
            entryPtr->exactMatchEntry.key.pattern[1] = (rand32Bits >> 4) & 0xF;
            entryPtr->exactMatchEntry.key.pattern[2] = (rand32Bits >> 8) & 0xF;
            entryPtr->exactMatchEntry.key.pattern[3] = (rand32Bits >> 12) & 0xF;
            entryPtr->exactMatchEntry.key.pattern[4] = (rand32Bits >> 16) & 0xF;

            if (keySize > 0)
            {
                entryPtr->exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;

                if (sameHash == GT_FALSE)
                {
                    entryPtr->exactMatchEntry.key.pattern[5] = rand32Bits & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[7] = (rand32Bits >> 4) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[9] = (rand32Bits >> 8) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[11] = (rand32Bits >> 12) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[13] = (rand32Bits >> 16) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[15] = (rand32Bits >> 4) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[17] = (rand32Bits >> 8) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[6] = (rand32Bits >> 12) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[8] = rand32Bits & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[10] = (rand32Bits >> 4) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[12] = (rand32Bits >> 8) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[14] = (rand32Bits >> 12) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[16] = (rand32Bits >> 16) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[18] = (rand32Bits >> 4) & 0xF;
                }
            }
            if (keySize > 1)
            {
                entryPtr->exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;

                if (sameHash == GT_FALSE)
                {
                    entryPtr->exactMatchEntry.key.pattern[19] = rand32Bits & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[20] = (rand32Bits >> 4) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[22] = (rand32Bits >> 8) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[21] = (rand32Bits >> 12) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[23] = rand32Bits & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[24] = (rand32Bits >> 16) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[26] = (rand32Bits >> 4) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[25] = (rand32Bits >> 8) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[27] = (rand32Bits >> 12) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[28] = (rand32Bits >> 16) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[30] = rand32Bits & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[29] = (rand32Bits >> 4) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[31] = (rand32Bits >> 8) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[32] = (rand32Bits >> 12) & 0xF;
                }
            }
            if (keySize > 2)
            {
                entryPtr->exactMatchEntry.key.keySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;

                if (sameHash == GT_FALSE)
                {
                    entryPtr->exactMatchEntry.key.pattern[46] = rand32Bits & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[45] = (rand32Bits >> 4) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[44] = (rand32Bits >> 8) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[43] = (rand32Bits >> 12) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[42] = (rand32Bits >> 16) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[41] = (rand32Bits >> 16) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[40] = (rand32Bits >> 4) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[39] = (rand32Bits >> 8) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[38] = (rand32Bits >> 12) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[37] = (rand32Bits >> 16) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[36] = (rand32Bits >> 16) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[35] = (rand32Bits >> 4) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[34] = (rand32Bits >> 8) & 0xF;
                    entryPtr->exactMatchEntry.key.pattern[33] = (rand32Bits >> 12) & 0xF;
                }
            }
        }
        EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId, entryPtr, &exactMatchEntryAddInfo, st);
        outputResultsPtr->rc = st;
        if (st==GT_ALREADY_EXIST)
        {
            return;
        }
        else
        {
            if(st == GT_OK)
            {
                *entryAddedPtr = GT_TRUE;
            }
            else if (st != GT_FULL && st != GT_ALREADY_EXIST)
            {
               UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "exactMatchManagerEntryAdd: unexpected error in manager [%d]",
                    exactMatchManagerId);
            }
        }

        if (skipValidity == GT_FALSE)
        {
            if((*entryAddedPtr) == GT_TRUE)
            {
                /* the entry was added ... check if the entry retrieved as was set */

                /* start -- the parts of the key */
                cpssOsMemSet(&entryGet,0,sizeof(entryGet));
                cpssOsMemCpy(&entryGet.exactMatchEntry.lookupNum,&entryPtr->exactMatchEntry.lookupNum,sizeof(entryPtr->exactMatchEntry.lookupNum));
                cpssOsMemCpy(&entryGet.exactMatchEntry.key.keySize,&entryPtr->exactMatchEntry.key.keySize,sizeof(entryPtr->exactMatchEntry.key.keySize));
                cpssOsMemCpy(&entryGet.exactMatchEntry.key.pattern,&entryPtr->exactMatchEntry.key.pattern,sizeof(entryPtr->exactMatchEntry.key.pattern));

                /* end -- the parts of the key */
                entryGet.exactMatchActionType = actionType;

                st = cpssDxChExactMatchManagerEntryGet(exactMatchManagerId,&entryGet);
                if (st!=GT_OK)
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                                "cpssDxChExactMatchManagerEntryGet: unexpected error in manager [%d]",
                                                exactMatchManagerId);
                }

                /***************************************************************/
                /* compare the full entry */
                /***************************************************************/
               st = prvCompareExactMatchActionStructs(prvTgfDevNum, "action", actionType, &entryPtr->exactMatchAction, &entryGet.exactMatchAction);
               UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",prvTgfDevNum, entryPtr->expandedActionIndex, actionType);

            }
        }
        return;
    }
}

/* add exact match entry to the manager : type :tti */
static void exactMatchManagerEntryAddCuckoo_tti(
    IN  GT_U32                                              exactMatchManagerId,
    IN  GT_U32                                              iterationNum,
    UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC    *outputResultsPtr,
    IN  GT_BOOL                                             isRandom,
    IN  CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT                  keySize
)
{
    exactMatchManagerEntryCuckooAdd(exactMatchManagerId,iterationNum,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E,outputResultsPtr,isRandom,keySize,GT_FALSE,GT_FALSE);
    return;
}
/* add exact match entry to the manager : type :pcl */
static void exactMatchManagerEntryAddCuckoo_pcl(
    IN  GT_U32                                              exactMatchManagerId,
    IN  GT_U32                                              iterationNum,
    UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC    *outputResultsPtr,
    IN  GT_BOOL                                             isRandom,
    IN  CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT                  keySize,
    IN  GT_BOOL                                             skipValidity,
    IN  GT_BOOL                                             sameHash
)
{
    exactMatchManagerEntryCuckooAdd(exactMatchManagerId,iterationNum,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E,outputResultsPtr,isRandom,keySize,skipValidity,sameHash);
    return;
}
/* add exact match entry to the manager : type :epcl */
static void exactMatchManagerEntryAddCuckoo_epcl(
    IN  GT_U32                                              exactMatchManagerId,
    IN  GT_U32                                              iterationNum,
    UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC    *outputResultsPtr,
    IN  GT_BOOL                                             isRandom,
    IN  CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT                  keySize
)
{
    exactMatchManagerEntryCuckooAdd(exactMatchManagerId,iterationNum,CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E,outputResultsPtr,isRandom,keySize,GT_FALSE,GT_FALSE);
    return;
}


/* check bounders of numeric field , and high values (31,32 bits value)  :
expected to fail */
static void CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd
(
    IN GT_U32                                               exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC   *paramsPtr,
    IN GT_U32                                               *fieldPtr,
    IN GT_U32                                               testedField_minValue,
    IN GT_U32                                               testedField_maxValue
)
{
    GT_STATUS   st=GT_OK;
    GT_U32      ii,iiMax = 3;
    GT_U32      origValue = *fieldPtr;

    if(testedField_minValue != 0) /* we check 0xFFFFFFFF anyway */
    {
        checkNonSupportedValues[iiMax++] = (GT_U32)((testedField_minValue) - 1);
    }
    if(testedField_maxValue!= 0)
    {
        checkNonSupportedValues[iiMax++] = (GT_U32)((testedField_maxValue)+1);
    }

    PRV_UTF_LOG0_MAC("check field with invalid field values\n");

    for(ii = 0; ii < iiMax; ii++)
    {
        *fieldPtr = checkNonSupportedValues[ii];
        EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId, entryPtr, paramsPtr, st);
        /* restore orig value */
        *fieldPtr = origValue;
        if (st != GT_OUT_OF_RANGE)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                "cpssDxChExactMatchManagerEntryAdd: [%d] expected to GT_BAD_PARAM on value [%d]",
                *fieldPtr,
                checkNonSupportedValues[ii]);
        }
    }
}
/* check bounders of numeric field , and high values (31,32 bits value)  :
expected to fail */
static void FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd
(
    IN GT_U32                                               exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC   *paramsPtr,
    IN GT_U32                                               *fieldPtr,
    IN GT_U32                                               testedField_Value
)
{
    GT_STATUS   st=GT_OK;
    GT_U32      origValue = *fieldPtr;

    PRV_UTF_LOG0_MAC("check field with invalid field value\n");

    *fieldPtr = testedField_Value;
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId, entryPtr, paramsPtr, st);
    /* restore orig value */
    *fieldPtr = origValue;
    if (st != GT_OUT_OF_RANGE)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChExactMatchManagerEntryAdd: [%d] expected to GT_BAD_PARAM on value [%d]",
            *fieldPtr,
            testedField_Value);
    }
}

/* check bounders of numeric field , and high values (31,32 bits value)  :
expected to fail */
static void CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate
(
    IN GT_U32                                               exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC              *entryPtr,
    IN GT_U32                                               *fieldPtr,
    IN GT_U32                                               testedField_minValue,
    IN GT_U32                                               testedField_maxValue
)
{
    GT_STATUS   st=GT_OK;
    GT_U32      ii,iiMax = 3;
    GT_U32      origValue = *fieldPtr;

    if(testedField_minValue != 0) /* we check 0xFFFFFFFF anyway */
    {
        checkNonSupportedValues[iiMax++] = (GT_U32)((testedField_minValue) - 1);
    }
    if(testedField_maxValue!= 0)
    {
        checkNonSupportedValues[iiMax++] = (GT_U32)((testedField_maxValue)+1);
    }

    PRV_UTF_LOG0_MAC("check field with invalid field values\n");

    for(ii = 0; ii < iiMax; ii++)
    {
        *fieldPtr = checkNonSupportedValues[ii];
        EM_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(exactMatchManagerId, entryPtr, st);
        /* restore orig value */
        *fieldPtr = origValue;
        if (st != GT_OUT_OF_RANGE)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
                "cpssDxChExactMatchManagerEntryAdd: [%d] expected to GT_BAD_PARAM on value [%d]",
                *fieldPtr,
                checkNonSupportedValues[ii]);
        }
    }
}

/* check bounders of numeric field , and high values (31,32 bits value)  :
expected to fail */
static void FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate
(
    IN GT_U32                                               exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC              *entryPtr,
    IN GT_U32                                               *fieldPtr,
    IN GT_U32                                               testedField_Value
)
{
    GT_STATUS   st=GT_OK;
    GT_U32      origValue = *fieldPtr;

    PRV_UTF_LOG0_MAC("check field with invalid field value\n");

    *fieldPtr = testedField_Value;
    EM_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(exactMatchManagerId, entryPtr, st);
    /* restore orig value */
    *fieldPtr = origValue;
    if (st != GT_OUT_OF_RANGE)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChExactMatchManagerEntryUpdate: [%d] expected to GT_BAD_PARAM on value [%d]",
            *fieldPtr,
            testedField_Value);
    }
}
/* check 4 valid values of numeric field , and values 0..28 and high values (31,32 bits value)  :
expected to fail */
static void CHECK_BOUNDERS_4_VALUES_FIELD_cpssDxChExactMatchManagerEntryAdd
(
    IN GT_U32                                               exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC   *paramsPtr,
    IN CPSS_INTERFACE_TYPE_ENT                              *fieldPtr,
    IN CPSS_INTERFACE_TYPE_ENT                              v1,
    IN CPSS_INTERFACE_TYPE_ENT                              v2,
    IN CPSS_INTERFACE_TYPE_ENT                              v3,
    IN CPSS_INTERFACE_TYPE_ENT                              v4
)
{
    GT_STATUS   st=GT_OK;
    GT_U32  ii, iiMax;
    GT_U32  origValue = *fieldPtr;
    iiMax = 32;
    for(ii=3; ii<iiMax;ii++)
    {
        checkNonSupportedValues[ii] = ii-3;
    }

    PRV_UTF_LOG0_MAC("check enum with invalid values \n");

    for(ii = 0; ii < iiMax; ii++)
    {
        *fieldPtr = checkNonSupportedValues[ii];
        if(*fieldPtr == v1 || *fieldPtr == v2 || *fieldPtr == v3 || *fieldPtr == v4)
        {   /* valid value */
            continue;
        }
        EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId, entryPtr, paramsPtr, st);
        /* restore orig value */
        *fieldPtr = origValue;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChExactMatchManagerEntryAdd: %d expected to GT_BAD_PARAM on value [%d]",
            *fieldPtr,
            checkNonSupportedValues[ii]);
    }
}
/* check 4 valid values of numeric field , and values 0..28 and high values (31,32 bits value)  :
expected to fail */
static void CHECK_BOUNDERS_4_VALUES_FIELD_cpssDxChExactMatchManagerEntryUpdate
(
    IN GT_U32                                               exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_INTERFACE_TYPE_ENT                              *fieldPtr,
    IN CPSS_INTERFACE_TYPE_ENT                              v1,
    IN CPSS_INTERFACE_TYPE_ENT                              v2,
    IN CPSS_INTERFACE_TYPE_ENT                              v3,
    IN CPSS_INTERFACE_TYPE_ENT                              v4
)
{
    GT_STATUS   st=GT_OK;
    GT_U32  ii, iiMax;
    GT_U32  origValue = *fieldPtr;
    iiMax = 32;
    for(ii=3; ii<iiMax;ii++)
    {
        checkNonSupportedValues[ii] = ii-3;
    }

    PRV_UTF_LOG0_MAC("check enum with invalid values \n");

    for(ii = 0; ii < iiMax; ii++)
    {
        *fieldPtr = checkNonSupportedValues[ii];
        if(*fieldPtr == v1 || *fieldPtr == v2 || *fieldPtr == v3 || *fieldPtr == v4)
        {   /* valid value */
            continue;
        }
        EM_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(exactMatchManagerId, entryPtr, st);
        /* restore orig value */
        *fieldPtr = origValue;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChExactMatchManagerEntryUpdate: %d expected to GT_BAD_PARAM on value [%d]",
            *fieldPtr,
            checkNonSupportedValues[ii]);
    }
}

static void bad_param_check_exactMatchManagerEntryAdd_tti(
    IN GT_U32   exactMatchManagerId
)
{
    GT_STATUS                                           st;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC             entry;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC  exactMatchEntryAddInfo;
    CPSS_DXCH_TTI_ACTION_STC                            *ttiActionFormatPtr = &entry.exactMatchAction.ttiAction;

    /* set valid key parameters */
    entry.exactMatchEntry.key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
    entry.exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
    entry.expandedActionIndex = 1;
    entry.exactMatchUserDefined = 1;

    /* build 'valid' entry : tti */
    entry.exactMatchActionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
    prvSetExactMatchActionDefaultValues(&entry.exactMatchAction,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

    exactMatchEntryAddInfo.rehashEnable    = GT_FALSE;

    PRV_UTF_LOG0_MAC("check that info for the entry is valid \n");
    /* check that info for the entry is valid */
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                 "exactMatchManagerEntryAdd_tti: unexpected GT_OK [%d]",
                                 exactMatchManagerId);

    PRV_UTF_LOG0_MAC("remove the entry \n");
    /* remove the entry */
    EM_MANAGER_ENTRY_DELETE_WITH_STATISTICS(exactMatchManagerId,&entry,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                 "cpssDxChExactMatchManagerEntryDelete: unexpected GT_OK [%d]",
                                 exactMatchManagerId);

    /**************************/
    /* start to modify fields */
    /**************************/
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        &entry.exactMatchUserDefined,
        BIT_16);
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        &entry.expandedActionIndex,
        8);/* 0-7 are defined as TTI */
     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        &entry.expandedActionIndex,
        16);/* max legal value is 15 */
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&entry.exactMatchEntry.key.keySize,
        CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E);
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&entry.exactMatchEntry.lookupNum,
        CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E);
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&entry.exactMatchActionType,
        CPSS_DXCH_EXACT_MATCH_ACTION_LAST_E);

     ttiActionFormatPtr->tunnelTerminate = GT_TRUE;
     CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&ttiActionFormatPtr->ttPassengerPacketType,
        CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E/*min*/,
        CPSS_DXCH_TTI_PASSENGER_MPLS_E/*max*/);
     ttiActionFormatPtr->tunnelTerminate = GT_FALSE;

     ttiActionFormatPtr->tunnelStart = GT_TRUE;
     CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&ttiActionFormatPtr->tsPassengerPacketType,
        CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E/*min*/,
        CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E/*max*/);
     ttiActionFormatPtr->tunnelStart = GT_FALSE;

     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
         exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
         &ttiActionFormatPtr->ttHeaderLength,
         127);/* 0-126 valid range */

     CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&ttiActionFormatPtr->tunnelHeaderLengthAnchorType,
        CPSS_DXCH_TUNNEL_HEADER_LENGTH_L3_ANCHOR_TYPE_E/*min*/,
        CPSS_DXCH_TUNNEL_HEADER_LENGTH_PROFILE_BASED_ANCHOR_TYPE_E/*max*/);

     CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&ttiActionFormatPtr->mplsCommand,
        CPSS_DXCH_TTI_MPLS_NOP_CMD_E/*min*/,
        CPSS_DXCH_TTI_MPLS_POP3_CMD_E/*max*/);

     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
         exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
         &ttiActionFormatPtr->mplsTtl,
         256);/* 0-255 valid range */

     CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&ttiActionFormatPtr->passengerParsingOfTransitMplsTunnelMode,
        CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_DISABLE_E/*min*/,
        CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_CW_ETH_E/*max*/);

     CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&ttiActionFormatPtr->redirectCommand,
        CPSS_DXCH_TTI_NO_REDIRECT_E/*min*/,
        CPSS_DXCH_TTI_LOGICAL_PORT_ASSIGN_E/*max*/);

     ttiActionFormatPtr->redirectCommand=CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E;
     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
         exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
         &ttiActionFormatPtr->routerLttPtr,
         BIT_15);

     ttiActionFormatPtr->redirectCommand=CPSS_DXCH_TTI_VRF_ID_ASSIGN_E;
     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
         exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
         &ttiActionFormatPtr->vrfId,
         VRF_ID_MAX_CNS);

     ttiActionFormatPtr->redirectCommand=CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
     CHECK_BOUNDERS_4_VALUES_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        &ttiActionFormatPtr->egressInterface.type,
        CPSS_INTERFACE_PORT_E,
        CPSS_INTERFACE_TRUNK_E,
        CPSS_INTERFACE_VIDX_E,
        CPSS_INTERFACE_VID_E);

     ttiActionFormatPtr->sourceIdSetEnable=GT_TRUE;
     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
         exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
         &ttiActionFormatPtr->sourceId,
         BIT_12);

     CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&ttiActionFormatPtr->tag0VlanCmd,
        CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E/*min*/,
        CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E/*max*/);

     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
         exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
         (GT_U32 *)&ttiActionFormatPtr->tag0VlanId,
         BIT_16);

     /* only support CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E and CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E */
     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
         exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
         (GT_U32 *)&ttiActionFormatPtr->tag1VlanCmd,
         CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E);
     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
         exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
         (GT_U32 *)&ttiActionFormatPtr->tag1VlanCmd,
         CPSS_DXCH_TTI_VLAN_MODIFY_TAGGED_E);

     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
         exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
         (GT_U32 *)&ttiActionFormatPtr->tag1VlanId,
         BIT_16);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&ttiActionFormatPtr->tag0VlanPrecedence,
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E/*min*/,
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E/*max*/);

    ttiActionFormatPtr->bindToPolicerMeter=GT_TRUE;
    ttiActionFormatPtr->bindToPolicer=GT_FALSE;
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
         exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
         &ttiActionFormatPtr->policerIndex,
         BIT_16);

    ttiActionFormatPtr->bindToPolicerMeter=GT_FALSE;
    ttiActionFormatPtr->bindToPolicer=GT_TRUE;
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
         exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
         &ttiActionFormatPtr->policerIndex,
         BIT_16);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&ttiActionFormatPtr->qosPrecedence,
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E/*min*/,
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E/*max*/);

    ttiActionFormatPtr->keepPreviousQoS=GT_FALSE;
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
         exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
         &ttiActionFormatPtr->qosProfile,
         TTI_QOS_PROFILE_E_ARCH_MAX_CNS+1);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&ttiActionFormatPtr->modifyTag0Up,
        CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E/*min*/,
        (CPSS_DXCH_TTI_MODIFY_UP_RESERVED_E-1)/*max*/);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&ttiActionFormatPtr->tag1UpCommand,
        CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E/*min*/,
        CPSS_DXCH_TTI_TAG1_UP_ASSIGN_ALL_E/*max*/);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&ttiActionFormatPtr->modifyDscp,
        CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E/*min*/,
        (CPSS_DXCH_TTI_MODIFY_DSCP_RESERVED_E-1)/*max*/);
  CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        &ttiActionFormatPtr->tag0Up,
        0/*min*/,
        (CPSS_USER_PRIORITY_RANGE_CNS-1)/*max*/);
  CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        &ttiActionFormatPtr->tag1Up,
        0/*min*/,
        (CPSS_USER_PRIORITY_RANGE_CNS-1)/*max*/);

  ttiActionFormatPtr->qosUseUpAsIndexEnable=GT_TRUE;
  CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        &ttiActionFormatPtr->qosMappingTableIndex,
        0/*min*/,
        (QOS_MAPPING_TABLE_INDEX_MAX_CNS-1)/*max*/);

  ttiActionFormatPtr->redirectCommand=CPSS_DXCH_TTI_NO_REDIRECT_E;
  CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&ttiActionFormatPtr->pcl0OverrideConfigIndex,
        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E/*min*/,
        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E/*max*/);
  CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&ttiActionFormatPtr->pcl0_1OverrideConfigIndex,
        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E/*min*/,
        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E/*max*/);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&ttiActionFormatPtr->pcl1OverrideConfigIndex,
        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E/*min*/,
        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E/*max*/);
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
         exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
         &ttiActionFormatPtr->iPclConfigIndex,
         ((_4K + 1024)));

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&ttiActionFormatPtr->command,
        CPSS_PACKET_CMD_FORWARD_E/*min*/,
        CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E/*max*/);

    ttiActionFormatPtr->command = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
         exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
         (GT_U32 *)&ttiActionFormatPtr->userDefinedCpuCode,
         ((CPSS_NET_LAST_USER_DEFINED_E+1)));
/*
    ttiActionFormatPtr->bindToCentralCounter = GT_TRUE;
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
         exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
         (GT_U32 *)&ttiActionFormatPtr->centralCounterIndex,
         ((CENTRAL_COUNTER_MAX_CNS+1)));
*/
    ttiActionFormatPtr->bindToCentralCounter = GT_FALSE;

    ttiActionFormatPtr->mirrorToIngressAnalyzerEnable = GT_TRUE;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        &ttiActionFormatPtr->mirrorToIngressAnalyzerIndex,
        0/*min*/,
        (BIT_3-1)/*max*/);


    ttiActionFormatPtr->redirectCommand = CPSS_DXCH_TTI_NO_REDIRECT_E;
    ttiActionFormatPtr->iPclUdbConfigTableEnable = GT_TRUE;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
       (GT_U32 *)&ttiActionFormatPtr->iPclUdbConfigTableIndex,
        CPSS_DXCH_PCL_PACKET_TYPE_UDE_E/*min*/,
        CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E/*max*/);

    ttiActionFormatPtr->qosUseUpAsIndexEnable = GT_TRUE;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        &ttiActionFormatPtr->qosMappingTableIndex,
        0/*min*/,
        (QOS_MAPPING_TABLE_INDEX_MAX_CNS-1)/*max*/);

    ttiActionFormatPtr->isPtpPacket = GT_TRUE;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&ttiActionFormatPtr->ptpTriggerType,
        CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_PTP_OVER_L2_E/*min*/,
        (CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_RESERVED_E-1)/*max*/);

    ttiActionFormatPtr->sourceEPortAssignmentEnable=GT_TRUE;
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
         exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
         &ttiActionFormatPtr->sourceEPort,
         8192);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        &ttiActionFormatPtr->hashMaskIndex,
        0/*min*/,
        (BIT_4-1)/*max*/);

    ttiActionFormatPtr->isPtpPacket=GT_TRUE;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&ttiActionFormatPtr->ptpTriggerType,
        CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_PTP_OVER_L2_E/*min*/,
        (CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_RESERVED_E-1)/*max*/);
}

static void bad_param_check_exactMatchManagerEntryAdd_pcl(
    IN GT_U32   exactMatchManagerId
)
{
    GT_STATUS                                           st;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC             entry;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC  exactMatchEntryAddInfo;
    CPSS_DXCH_PCL_ACTION_STC                            *pclActionFormatPtr = &entry.exactMatchAction.pclAction;

    /* set valid key parameters */
    entry.exactMatchEntry.key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
    entry.exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E; /* PCL_0 defined as second client */
    entry.expandedActionIndex = 8;
    entry.exactMatchUserDefined = 18;

    /* build 'valid' entry : pcl */
    entry.exactMatchActionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
    prvSetExactMatchActionDefaultValues(&entry.exactMatchAction,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

    exactMatchEntryAddInfo.rehashEnable    = GT_FALSE;

    PRV_UTF_LOG0_MAC("check that info for the entry is valid \n");
    /* check that info for the entry is valid */
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                 "exactMatchManagerEntryAdd_tti: unexpected GT_OK [%d]",
                                 exactMatchManagerId);

    PRV_UTF_LOG0_MAC("remove the entry \n");
    /* remove the entry */
    EM_MANAGER_ENTRY_DELETE_WITH_STATISTICS(exactMatchManagerId,&entry,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                 "cpssDxChExactMatchManagerEntryDelete: unexpected GT_OK [%d]",
                                 exactMatchManagerId);

    /**************************/
    /* start to modify fields */
    /**************************/
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        &entry.exactMatchUserDefined,
        BIT_16);
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        &entry.expandedActionIndex,
        7);/* 8-15 are defined as PCL */
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        &entry.expandedActionIndex,
        16);/* 8-15 are defined as PCL */

    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&entry.exactMatchEntry.key.keySize,
        CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E);
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&entry.exactMatchEntry.lookupNum,
        CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E);
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&entry.exactMatchActionType,
        CPSS_DXCH_EXACT_MATCH_ACTION_LAST_E);

    /* add pcl fields */
    pclActionFormatPtr=pclActionFormatPtr;
#if 0
TBD

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&pclActionFormatPtr->,
        /*min*/,
        /*max*/);
#endif

}

static void bad_param_check_exactMatchManagerEntryAdd_epcl(
    IN GT_U32   exactMatchManagerId
)
{
    GT_STATUS                                           st;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC             entry;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC  exactMatchEntryAddInfo;
    CPSS_DXCH_PCL_ACTION_STC                            *pclActionFormatPtr = &entry.exactMatchAction.pclAction;

    /* set valid key parameters */
    entry.exactMatchEntry.key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
    entry.exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E; /* PCL_0 defined as second client */
    entry.expandedActionIndex = 10;
    entry.exactMatchUserDefined = 28;

    /* build 'valid' entry : epcl */
    entry.exactMatchActionType = CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E;
    prvSetExactMatchActionDefaultValues(&entry.exactMatchAction,CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E);

    exactMatchEntryAddInfo.rehashEnable    = GT_FALSE;

    PRV_UTF_LOG0_MAC("check that info for the entry is valid \n");
    /* check that info for the entry is valid */
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                 "exactMatchManagerEntryAdd_tti: unexpected GT_OK [%d]",
                                 exactMatchManagerId);

    PRV_UTF_LOG0_MAC("remove the entry \n");
    /* remove the entry */
    EM_MANAGER_ENTRY_DELETE_WITH_STATISTICS(exactMatchManagerId,&entry,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                 "cpssDxChExactMatchManagerEntryDelete: unexpected GT_OK [%d]",
                                 exactMatchManagerId);

    /**************************/
    /* start to modify fields */
    /**************************/
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        &entry.exactMatchUserDefined,
        BIT_16);
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        &entry.expandedActionIndex,
        7);/* 8-15 are defined as PCL */
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        &entry.expandedActionIndex,
        16);/* 8-15 are defined as PCL */

    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&entry.exactMatchEntry.key.keySize,
        CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E);
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&entry.exactMatchEntry.lookupNum,
        CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E);
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&entry.exactMatchActionType,
        CPSS_DXCH_EXACT_MATCH_ACTION_LAST_E);

    /* add pcl fields */
    pclActionFormatPtr=pclActionFormatPtr;
#if 0
TBD

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryAdd(
        exactMatchManagerId, &entry, &exactMatchEntryAddInfo,
        (GT_U32 *)&pclActionFormatPtr->,
        /*min*/,
        /*max*/);
#endif

}

static void bad_param_check_exactMatchManagerEntryUpdate_tti(
    IN GT_U32   exactMatchManagerId
)
{
    GT_STATUS                                           st;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC             entry;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC  exactMatchEntryAddInfo;
    CPSS_DXCH_TTI_ACTION_STC                            *ttiActionFormatPtr = &entry.exactMatchAction.ttiAction;

    /* set valid key parameters */
    entry.exactMatchEntry.key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
    entry.exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
    entry.expandedActionIndex = 1;
    entry.exactMatchUserDefined = 1;

    /* build 'valid' entry : tti */
    entry.exactMatchActionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
    prvSetExactMatchActionDefaultValues(&entry.exactMatchAction,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

    exactMatchEntryAddInfo.rehashEnable=GT_FALSE;

    PRV_UTF_LOG0_MAC("check that info for the entry is valid \n");
    /* check that info for the entry is valid */
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                 "exactMatchManagerEntryAdd_tti: unexpected GT_OK [%d]",
                                 exactMatchManagerId);

    /**************************/
    /* start to modify fields */
    /**************************/
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        &entry.exactMatchUserDefined,
        BIT_16);
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        &entry.expandedActionIndex,
        8);/* 0-7 are defined as TTI */
     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        &entry.expandedActionIndex,
        16);/* max legal value is 15 */
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&entry.exactMatchEntry.key.keySize,
        CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E);
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&entry.exactMatchEntry.lookupNum,
        CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E);
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&entry.exactMatchActionType,
        CPSS_DXCH_EXACT_MATCH_ACTION_LAST_E);

     ttiActionFormatPtr->tunnelTerminate = GT_TRUE;
     CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&ttiActionFormatPtr->ttPassengerPacketType,
        CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E/*min*/,
        CPSS_DXCH_TTI_PASSENGER_MPLS_E/*max*/);
     ttiActionFormatPtr->tunnelTerminate = GT_FALSE;

     ttiActionFormatPtr->tunnelStart = GT_TRUE;
     CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&ttiActionFormatPtr->tsPassengerPacketType,
        CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E/*min*/,
        CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E/*max*/);
     ttiActionFormatPtr->tunnelStart = GT_FALSE;

     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
         exactMatchManagerId, &entry,
         &ttiActionFormatPtr->ttHeaderLength,
         127);/* 0-126 valid range */

     CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&ttiActionFormatPtr->tunnelHeaderLengthAnchorType,
        CPSS_DXCH_TUNNEL_HEADER_LENGTH_L3_ANCHOR_TYPE_E/*min*/,
        CPSS_DXCH_TUNNEL_HEADER_LENGTH_PROFILE_BASED_ANCHOR_TYPE_E/*max*/);

     CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&ttiActionFormatPtr->mplsCommand,
        CPSS_DXCH_TTI_MPLS_NOP_CMD_E/*min*/,
        CPSS_DXCH_TTI_MPLS_POP3_CMD_E/*max*/);

     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
         exactMatchManagerId, &entry,
         &ttiActionFormatPtr->mplsTtl,
         256);/* 0-255 valid range */

     CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&ttiActionFormatPtr->passengerParsingOfTransitMplsTunnelMode,
        CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_DISABLE_E/*min*/,
        CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_CW_ETH_E/*max*/);

     CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&ttiActionFormatPtr->redirectCommand,
        CPSS_DXCH_TTI_NO_REDIRECT_E/*min*/,
        CPSS_DXCH_TTI_LOGICAL_PORT_ASSIGN_E/*max*/);

     ttiActionFormatPtr->redirectCommand=CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E;
     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
         exactMatchManagerId, &entry,
         &ttiActionFormatPtr->routerLttPtr,
         BIT_15);

     ttiActionFormatPtr->redirectCommand=CPSS_DXCH_TTI_VRF_ID_ASSIGN_E;
     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
         exactMatchManagerId, &entry,
         &ttiActionFormatPtr->vrfId,
         VRF_ID_MAX_CNS);

     ttiActionFormatPtr->redirectCommand=CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
     CHECK_BOUNDERS_4_VALUES_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        &ttiActionFormatPtr->egressInterface.type,
        CPSS_INTERFACE_PORT_E,
        CPSS_INTERFACE_TRUNK_E,
        CPSS_INTERFACE_VIDX_E,
        CPSS_INTERFACE_VID_E);

     ttiActionFormatPtr->sourceIdSetEnable=GT_TRUE;
     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
         exactMatchManagerId, &entry,
         &ttiActionFormatPtr->sourceId,
         BIT_12);

     CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&ttiActionFormatPtr->tag0VlanCmd,
        CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E/*min*/,
        CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E/*max*/);

     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
         exactMatchManagerId, &entry,
         (GT_U32 *)&ttiActionFormatPtr->tag0VlanId,
         BIT_16);

     /* only support CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E and CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E */
     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
         exactMatchManagerId, &entry,
         (GT_U32 *)&ttiActionFormatPtr->tag1VlanCmd,
         CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E);
     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
         exactMatchManagerId, &entry,
         (GT_U32 *)&ttiActionFormatPtr->tag1VlanCmd,
         CPSS_DXCH_TTI_VLAN_MODIFY_TAGGED_E);

     FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
         exactMatchManagerId, &entry,
         (GT_U32 *)&ttiActionFormatPtr->tag1VlanId,
         BIT_16);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&ttiActionFormatPtr->tag0VlanPrecedence,
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E/*min*/,
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E/*max*/);

    ttiActionFormatPtr->bindToPolicerMeter=GT_TRUE;
    ttiActionFormatPtr->bindToPolicer=GT_FALSE;
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
         exactMatchManagerId, &entry,
         &ttiActionFormatPtr->policerIndex,
         BIT_16);

    ttiActionFormatPtr->bindToPolicerMeter=GT_FALSE;
    ttiActionFormatPtr->bindToPolicer=GT_TRUE;
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
         exactMatchManagerId, &entry,
         &ttiActionFormatPtr->policerIndex,
         BIT_16);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&ttiActionFormatPtr->qosPrecedence,
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E/*min*/,
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E/*max*/);

    ttiActionFormatPtr->keepPreviousQoS=GT_FALSE;
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
         exactMatchManagerId, &entry,
         &ttiActionFormatPtr->qosProfile,
         TTI_QOS_PROFILE_E_ARCH_MAX_CNS+1);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&ttiActionFormatPtr->modifyTag0Up,
        CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E/*min*/,
        (CPSS_DXCH_TTI_MODIFY_UP_RESERVED_E-1)/*max*/);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&ttiActionFormatPtr->tag1UpCommand,
        CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E/*min*/,
        CPSS_DXCH_TTI_TAG1_UP_ASSIGN_ALL_E/*max*/);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&ttiActionFormatPtr->modifyDscp,
        CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E/*min*/,
        (CPSS_DXCH_TTI_MODIFY_DSCP_RESERVED_E-1)/*max*/);
  CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        &ttiActionFormatPtr->tag0Up,
        0/*min*/,
        (CPSS_USER_PRIORITY_RANGE_CNS-1)/*max*/);
  CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        &ttiActionFormatPtr->tag1Up,
        0/*min*/,
        (CPSS_USER_PRIORITY_RANGE_CNS-1)/*max*/);

  ttiActionFormatPtr->qosUseUpAsIndexEnable=GT_TRUE;
  CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        &ttiActionFormatPtr->qosMappingTableIndex,
        0/*min*/,
        (QOS_MAPPING_TABLE_INDEX_MAX_CNS-1)/*max*/);

  ttiActionFormatPtr->redirectCommand=CPSS_DXCH_TTI_NO_REDIRECT_E;
  CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&ttiActionFormatPtr->pcl0OverrideConfigIndex,
        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E/*min*/,
        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E/*max*/);
  CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&ttiActionFormatPtr->pcl0_1OverrideConfigIndex,
        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E/*min*/,
        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E/*max*/);
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&ttiActionFormatPtr->pcl1OverrideConfigIndex,
        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E/*min*/,
        CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E/*max*/);
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
         exactMatchManagerId, &entry,
         &ttiActionFormatPtr->iPclConfigIndex,
         ((_4K + 1024)));

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&ttiActionFormatPtr->command,
        CPSS_PACKET_CMD_FORWARD_E/*min*/,
        CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E/*max*/);

    ttiActionFormatPtr->command = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
         exactMatchManagerId, &entry,
         (GT_U32 *)&ttiActionFormatPtr->userDefinedCpuCode,
         ((CPSS_NET_LAST_USER_DEFINED_E+1)));
/*
    ttiActionFormatPtr->bindToCentralCounter = GT_TRUE;
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
         exactMatchManagerId, &entry,
         (GT_U32 *)&ttiActionFormatPtr->centralCounterIndex,
         ((CENTRAL_COUNTER_MAX_CNS+1)));
*/
    ttiActionFormatPtr->bindToCentralCounter = GT_FALSE;

    ttiActionFormatPtr->mirrorToIngressAnalyzerEnable = GT_TRUE;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        &ttiActionFormatPtr->mirrorToIngressAnalyzerIndex,
        0/*min*/,
        (BIT_3-1)/*max*/);


    ttiActionFormatPtr->redirectCommand = CPSS_DXCH_TTI_NO_REDIRECT_E;
    ttiActionFormatPtr->iPclUdbConfigTableEnable = GT_TRUE;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
       (GT_U32 *)&ttiActionFormatPtr->iPclUdbConfigTableIndex,
        CPSS_DXCH_PCL_PACKET_TYPE_UDE_E/*min*/,
        CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E/*max*/);

    ttiActionFormatPtr->qosUseUpAsIndexEnable = GT_TRUE;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        &ttiActionFormatPtr->qosMappingTableIndex,
        0/*min*/,
        (QOS_MAPPING_TABLE_INDEX_MAX_CNS-1)/*max*/);

    ttiActionFormatPtr->isPtpPacket = GT_TRUE;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&ttiActionFormatPtr->ptpTriggerType,
        CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_PTP_OVER_L2_E/*min*/,
        (CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_RESERVED_E-1)/*max*/);

    ttiActionFormatPtr->sourceEPortAssignmentEnable=GT_TRUE;
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
         exactMatchManagerId, &entry,
         &ttiActionFormatPtr->sourceEPort,
         8192);

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        &ttiActionFormatPtr->hashMaskIndex,
        0/*min*/,
        (BIT_4-1)/*max*/);

    ttiActionFormatPtr->isPtpPacket=GT_TRUE;
    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&ttiActionFormatPtr->ptpTriggerType,
        CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_PTP_OVER_L2_E/*min*/,
        (CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_RESERVED_E-1)/*max*/);


    PRV_UTF_LOG0_MAC("remove the entry \n");
    /* remove the entry */
    EM_MANAGER_ENTRY_DELETE_WITH_STATISTICS(exactMatchManagerId,&entry,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                 "cpssDxChExactMatchManagerEntryDelete: unexpected GT_OK [%d]",
                                 exactMatchManagerId);
}

static void bad_param_check_exactMatchManagerEntryUpdate_pcl(
    IN GT_U32   exactMatchManagerId
)
{
    GT_STATUS                                           st;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC             entry;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC  exactMatchEntryAddInfo;
    CPSS_DXCH_PCL_ACTION_STC                            *pclActionFormatPtr = &entry.exactMatchAction.pclAction;

    /* set valid key parameters */
    entry.exactMatchEntry.key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
    entry.exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E; /* PCL_0 defined as second client */
    entry.expandedActionIndex = 8;
    entry.exactMatchUserDefined = 18;

    /* build 'valid' entry : pcl */
    entry.exactMatchActionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
    prvSetExactMatchActionDefaultValues(&entry.exactMatchAction,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

    exactMatchEntryAddInfo.rehashEnable    = GT_FALSE;

    PRV_UTF_LOG0_MAC("check that info for the entry is valid \n");
    /* check that info for the entry is valid */
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                 "exactMatchManagerEntryAdd_tti: unexpected GT_OK [%d]",
                                 exactMatchManagerId);

    /**************************/
    /* start to modify fields */
    /**************************/
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        &entry.exactMatchUserDefined,
        BIT_16);
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        &entry.expandedActionIndex,
        7);/* 8-15 are defined as PCL */
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        &entry.expandedActionIndex,
        16);/* 8-15 are defined as PCL */

    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&entry.exactMatchEntry.key.keySize,
        CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E);
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&entry.exactMatchEntry.lookupNum,
        CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E);
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&entry.exactMatchActionType,
        CPSS_DXCH_EXACT_MATCH_ACTION_LAST_E);

    /* add pcl fields */
    pclActionFormatPtr=pclActionFormatPtr;
#if 0
TBD

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&pclActionFormatPtr->,
        /*min*/,
        /*max*/);
#endif

    PRV_UTF_LOG0_MAC("remove the entry \n");
    /* remove the entry */
    EM_MANAGER_ENTRY_DELETE_WITH_STATISTICS(exactMatchManagerId,&entry,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                 "cpssDxChExactMatchManagerEntryDelete: unexpected GT_OK [%d]",
                                 exactMatchManagerId);

}

static void bad_param_check_exactMatchManagerEntryUpdate_epcl(
    IN GT_U32   exactMatchManagerId
)
{
    GT_STATUS                                           st;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC             entry;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC  exactMatchEntryAddInfo;
    CPSS_DXCH_PCL_ACTION_STC                            *pclActionFormatPtr = &entry.exactMatchAction.pclAction;

    /* set valid key parameters */
    entry.exactMatchEntry.key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
    entry.exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E; /* PCL_0 defined as second client */
    entry.expandedActionIndex = 10;
    entry.exactMatchUserDefined = 28;

    /* build 'valid' entry : epcl */
    entry.exactMatchActionType = CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E;
    prvSetExactMatchActionDefaultValues(&entry.exactMatchAction,CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E);

    exactMatchEntryAddInfo.rehashEnable    = GT_FALSE;

    PRV_UTF_LOG0_MAC("check that info for the entry is valid \n");
    /* check that info for the entry is valid */
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                 "exactMatchManagerEntryAdd_tti: unexpected GT_OK [%d]",
                                 exactMatchManagerId);

    /**************************/
    /* start to modify fields */
    /**************************/
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        &entry.exactMatchUserDefined,
        BIT_16);
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        &entry.expandedActionIndex,
        7);/* 8-15 are defined as PCL */
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        &entry.expandedActionIndex,
        16);/* 8-15 are defined as PCL */

    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&entry.exactMatchEntry.key.keySize,
        CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E);
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&entry.exactMatchEntry.lookupNum,
        CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E);
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&entry.exactMatchActionType,
        CPSS_DXCH_EXACT_MATCH_ACTION_LAST_E);

    /* add pcl fields */
    pclActionFormatPtr=pclActionFormatPtr;
#if 0
TBD

    CHECK_BOUNDERS_NUMERIC_FIELD_cpssDxChExactMatchManagerEntryUpdate(
        exactMatchManagerId, &entry,
        (GT_U32 *)&pclActionFormatPtr->,
        /*min*/,
        /*max*/);
#endif

    PRV_UTF_LOG0_MAC("remove the entry \n");
    /* remove the entry */
    EM_MANAGER_ENTRY_DELETE_WITH_STATISTICS(exactMatchManagerId,&entry,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                 "cpssDxChExactMatchManagerEntryDelete: unexpected GT_OK [%d]",
                                 exactMatchManagerId);

}

/**
GT_STATUS cpssDxChExactMatchManagerEntryAdd
(
    IN GT_U32                                               exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC   *paramsPtr
);
*/
static void internal_cpssDxChExactMatchManagerEntryAddUT(void)
{
    GT_STATUS                                               st;
    GT_U32                                                  ii;
    GT_U32                                                  numEntriesRemoved=0;/* total number of entries removed from the DB */
    GT_U32                                                  numEntriesAdded=0;  /* total number of entries added to the DB */
    GT_U32                                                  numIndexesAdded;    /* number of entries added to each bank;
                                                                                if one entry is added with keysize=47 then:
                                                                                numEntriesAdded=1 and numIndexesAdded=4 */
    GT_BOOL                                                 entryAdded;/* indication that the entry was added to the DB */
    GT_BOOL                                                 getFirst;  /* indication to get first entry from the DB */
    GT_U32                                                  exactMatchManagerId = TESTED_EXACT_MATCH_MANAGER_ID_CNS;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                 entry;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC      params;
    GT_U32                                                  maxTotalEntries;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT                   exactMatchEntryType;
    UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC    outputResults;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      testResultArray[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNum=0, errorNumItr=0;

    cpssOsMemSet(&entry ,0,sizeof(entry));
    cpssOsMemSet(&entry.exactMatchEntry.key.pattern ,0,sizeof(entry.exactMatchEntry.key.pattern));
    cpssOsMemSet(&params,0,sizeof(params));
    cpssOsMemSet(testResultArray,0,sizeof(testResultArray));

    outputResults.entryPtr          = &entry;
    outputResults.entryAddedPtr     = &entryAdded;

    PRV_UTF_LOG0_MAC("Check non-initialized managers \n");

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(ii,&entry,&params,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChExactMatchManagerEntryAdd: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    PRV_UTF_LOG0_MAC("Check check out of range managers \n");
    /* check out of range managers ... expect 'GT_OUT_OF_RANGE' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(ii,&entry,&params,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
            "cpssDxChExactMatchManagerEntryAdd: expected to GT_OUT_OF_RANGE on manager [%d]",
            ii);
    }

    numEntriesAdded = 0;
    numIndexesAdded = 0;
    maxTotalEntries = EXACT_MATCH_SIZE_FULL;

    if(em_debug_limited_entries)
    {
        maxTotalEntries = em_debug_limited_entries;
    }

    if(run_device_less == GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("create manager with device \n");
    }
    else
    {
        PRV_UTF_LOG0_MAC("create manager without device \n");
    }
    /* create manager with device */
    st = createExactMatchManagerWithDevice(exactMatchManagerId,maxTotalEntries,prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "createExactMatchManagerWithDevice: expected to GT_OK on manager [%d]",
                exactMatchManagerId);
    if (st==GT_OK)
    {
        PRV_UTF_LOG1_MAC("start - add [%d] entries \n", maxTotalEntries);
        for(ii = 0 ; ii < maxTotalEntries; /*update according to entry size*/)
        {
            entryAdded = GT_FALSE;

            exactMatchEntryType = exactMatchEntryType_valid_arr[ii % UT_EXACT_MATCH_ENTRY_TYPE_VALID_NUM];

            switch(exactMatchEntryType)
            {
                case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
                    exactMatchManagerEntryAdd_tti(exactMatchManagerId, ii, &outputResults);
                    break;
                case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
                    exactMatchManagerEntryAdd_pcl(exactMatchManagerId, ii, &outputResults);
                    break;
                case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
                    exactMatchManagerEntryAdd_epcl(exactMatchManagerId, ii, &outputResults);
                    break;
                default:
                    break;
            }

            if(entryAdded)
            {
                numEntriesAdded++;
                numIndexesAdded+=(outputResults.entryPtr->exactMatchEntry.key.keySize+1);
            }

            if(add_device_in_runtime == GT_TRUE &&
               (ii == (maxTotalEntries/2)))
            {
                CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[1];
                GT_U32                                                   numOfPairs=1;

                /* now in the middle of the insertions we add the device. */
                PRV_UTF_LOG0_MAC("now in the middle of the insertions we add the device \n");

                pairListArr[0].devNum = prvTgfDevNum;
                pairListArr[0].portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

                /* add device to it */
                st = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChExactMatchManagerDevListAdd: expected to GT_OK on manager [%d]",
                    exactMatchManagerId);
            }
            if(entryAdded)
            {
                ii+=(outputResults.entryPtr->exactMatchEntry.key.keySize+1);
            }
            else
            {
                ii++;
            }
        }
        PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
            maxTotalEntries,numIndexesAdded,
                (100*numIndexesAdded)/maxTotalEntries);

        if(em_debug_limited_entries)
        {
             cpssDxChExactMatchRulesDump(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,0,(EXACT_MATCH_SIZE_FULL-1));
        }

        /* Verify DB */
        st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                    errorNumItr,
                    testResultArray[errorNumItr],
                    debugErrorArr[testResultArray[errorNumItr]].errString);
        }

        numEntriesRemoved = 0;
        getFirst = GT_TRUE;

        PRV_UTF_LOG1_MAC("start - remove the [%d] entries \n",numEntriesAdded);
        if(check_hw_hold_entry == GT_TRUE)
        {
            PRV_UTF_LOG0_MAC("(slow mode) for each entry check that HW hold the entry in proper index according to it's index \n");
        }
        /* remove all entries from the DB (one by one) */
        while(GT_OK == cpssDxChExactMatchManagerEntryGetNext(exactMatchManagerId,getFirst,&entry))
        {
            getFirst = GT_FALSE;

            if(check_hw_hold_entry == GT_TRUE)
            {
                /* just before we going to remove the entry ... lets check that it exists in the HW ! */
                exactMatchManagerHwEntryCheck(&entry.exactMatchEntry);
            }

            EM_MANAGER_ENTRY_DELETE_WITH_STATISTICS(exactMatchManagerId,&entry,st);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChExactMatchManagerEntryDelete: expected to GT_OK on manager [%d]",
                exactMatchManagerId);
            numEntriesRemoved++;

            if((numEntriesRemoved % 1000) == 0)
            {
                PRV_UTF_LOG1_MAC("removed [%d] entries \n",numEntriesRemoved);
            }
        }

        PRV_UTF_LOG2_MAC("ended - removed [%d] entries (out of expected [%d]) \n",numEntriesRemoved,numEntriesAdded);

        UTF_VERIFY_EQUAL2_STRING_MAC(numEntriesAdded, numEntriesRemoved,
            "expected number of removed entries [%d] to be the same as the added entries[%d]",
            numEntriesRemoved,numEntriesAdded);

        /* Verify DB */
        st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                    errorNumItr,
                    testResultArray[errorNumItr],
                    debugErrorArr[testResultArray[errorNumItr]].errString);
        }

        /* check invalid parameters */
        for(ii = 0 ; ii < UT_EXACT_MATCH_ENTRY_TYPE_VALID_NUM; ii++)
        {
            exactMatchEntryType = exactMatchEntryType_valid_arr[ii % UT_EXACT_MATCH_ENTRY_TYPE_VALID_NUM];

            switch(exactMatchEntryType)
            {
                case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
                    PRV_UTF_LOG0_MAC("check invalid parameters : tti \n");
                    bad_param_check_exactMatchManagerEntryAdd_tti(exactMatchManagerId);
                    break;
                case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
                    PRV_UTF_LOG0_MAC("check invalid parameters : pcl \n");
                    bad_param_check_exactMatchManagerEntryAdd_pcl(exactMatchManagerId);
                    break;
                case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
                    PRV_UTF_LOG0_MAC("check invalid parameters : epcl \n");
                    bad_param_check_exactMatchManagerEntryAdd_epcl(exactMatchManagerId);
                    break;
                default:
                    break;
            }
        }

        PRV_UTF_LOG0_MAC("check NULL pointers \n");
        /* NULL pointer */
        EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,NULL,&params,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
            "cpssDxChExactMatchManagerEntryAdd: expected to GT_BAD_PTR on manager [%d]",
            exactMatchManagerId);

        EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,NULL,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
            "cpssDxChExactMatchManagerEntryAdd: expected to GT_BAD_PTR on manager [%d]",
            exactMatchManagerId);

        /* Verify DB */
        st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                    errorNumItr,
                    testResultArray[errorNumItr],
                    debugErrorArr[testResultArray[errorNumItr]].errString);
        }

        PRV_UTF_LOG0_MAC("Ended \n");
    }
    else
    {
        PRV_UTF_LOG0_MAC("Ended - Test fail , could not create manager - createExactMatchManagerWithDevice fail \n");
    }
    return;
}
/**
GT_STATUS cpssDxChExactMatchManagerEntryAdd
(
    IN GT_U32                                               exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC   *paramsPtr
);
*/
static void internal_cpssDxChExactMatchManagerCuckooEntryAddUT
(
    IN  GT_BOOL                                             isRandom,
    IN  CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT                  keySize
)
{
    GT_STATUS                                               st=GT_OK;
    GT_U32                                                  ii;
    GT_U32                                                  numEntriesRemoved=0;/* total number of entries removed from the DB */
    GT_U32                                                  numEntriesAdded=0;  /* total number of entries added to the DB */
    GT_U32                                                  numIndexesAdded;    /* number of entries added to each bank;
                                                                                if one entry is added with keysize=47 then:
                                                                                numEntriesAdded=1 and numIndexesAdded=4 */
    GT_BOOL                                                 entryAdded;/* indication that the entry was added to the DB */
    GT_BOOL                                                 getFirst;  /* indication to get first entry from the DB */
    GT_U32                                                  exactMatchManagerId = TESTED_EXACT_MATCH_MANAGER_ID_CNS;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                 entry;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC      params;
    GT_U32                                                  maxTotalEntries;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT                   exactMatchEntryType;
    UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC    outputResults;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      testResultArray[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNum=0, errorNumItr=0,size;
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT                      currKeySize;
    GT_U32                                                  sizeInBytes=0;
    GT_BOOL                                                 found = GT_FALSE;

    GT_UNUSED_PARAM(isRandom);

    cpssOsMemSet(&entry ,0,sizeof(entry));
    cpssOsMemSet(&entry.exactMatchEntry.key.pattern ,0,sizeof(entry.exactMatchEntry.key.pattern));
    cpssOsMemSet(&params,0,sizeof(params));
    cpssOsMemSet(testResultArray,0,sizeof(testResultArray));

    outputResults.entryPtr          = &entry;
    outputResults.entryAddedPtr     = &entryAdded;

    numEntriesAdded = 0;
    numIndexesAdded = 0;
    maxTotalEntries = EXACT_MATCH_SIZE_FULL;

    if(em_debug_limited_entries)
    {
        maxTotalEntries = em_debug_limited_entries;
    }

    PRV_UTF_LOG0_MAC("create manager with device \n");

    /* create manager with device */
    st = createExactMatchManagerWithDevice(exactMatchManagerId,maxTotalEntries,prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "createExactMatchManagerWithDevice: expected to GT_OK on manager [%d]",
                exactMatchManagerId);

    if (st==GT_OK)
    {
        switch (keySize)
        {
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [5B]\n", maxTotalEntries);
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [19B]\n", maxTotalEntries);
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [33B]\n", maxTotalEntries);
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [47B]\n", maxTotalEntries);
            break;
        default: /* random key */
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [random]\n", maxTotalEntries);
        }

        currKeySize = keySize;

        for(ii = 0 ; ii < maxTotalEntries; /*update according to entry size*/)
        {
            entryAdded = GT_FALSE;

            exactMatchEntryType = exactMatchEntryType_valid_arr[ii % UT_EXACT_MATCH_ENTRY_TYPE_VALID_NUM];
            if (keySize == CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E) {
                if (exactMatchEntryType==CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E)
                {
                    size = (cpssOsRand() & 0x3)%3; /*  bits - legal values for TTI 0-2 */
                }
                else
                {
                    size = cpssOsRand() & 0x3; /* 2 bits - legal values 0-3 */
                }
                switch (size)
                {
                case 0:
                    currKeySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
                    break;
                case 1:
                    currKeySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
                    break;
                case 2:
                    currKeySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
                    break;
                case 3:
                    currKeySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
                    break;
                default:
                    break;
                }
            }
            else
            {
                if(exactMatchEntryType==CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E)
                {
                    /*  for TTI only size47 is not valid */
                    currKeySize = currKeySize%3;
                }
            }
            switch(exactMatchEntryType)
            {
                case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
                    exactMatchManagerEntryAddCuckoo_tti(exactMatchManagerId, ii, &outputResults,isRandom,currKeySize);
                    break;
                case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
                    exactMatchManagerEntryAddCuckoo_pcl(exactMatchManagerId, ii, &outputResults,isRandom,currKeySize,GT_FALSE,GT_FALSE);
                    break;
                case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
                    exactMatchManagerEntryAddCuckoo_epcl(exactMatchManagerId, ii, &outputResults,isRandom,currKeySize);
                    break;
                default:
                    break;
            }

            if(entryAdded)
            {
                numEntriesAdded++;
                numIndexesAdded+=(outputResults.entryPtr->exactMatchEntry.key.keySize+1);
                ii+=(outputResults.entryPtr->exactMatchEntry.key.keySize+1);
            }
            else
            {
                ii++;
            }
        }
        PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
            maxTotalEntries,numIndexesAdded,
                (100*numIndexesAdded)/maxTotalEntries);

        prvCpssDxChCuckooDbPrintStatistics(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);

        if(em_debug_limited_entries)
        {
             cpssDxChExactMatchRulesDump(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,0,(EXACT_MATCH_SIZE_FULL-1));
        }

        /* Verify DB */
        st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                    errorNumItr,
                    testResultArray[errorNumItr],
                    debugErrorArr[testResultArray[errorNumItr]].errString);
        }

        numEntriesRemoved = 0;
        getFirst = GT_TRUE;

        PRV_UTF_LOG1_MAC("start - remove the [%d] entries \n",numEntriesAdded);
        if(check_hw_hold_entry == GT_TRUE)
        {
            PRV_UTF_LOG0_MAC("(slow mode) for each entry check that HW hold the entry in proper index according to it's index \n");
        }
        /* remove all entries from the DB (one by one) */
        while(GT_OK == cpssDxChExactMatchManagerEntryGetNext(exactMatchManagerId,getFirst,&entry))
        {
            getFirst = GT_FALSE;

            if(check_hw_hold_entry == GT_TRUE)
            {
                /* just before we going to remove the entry ... lets check that it exists in the HW ! */
                exactMatchManagerHwEntryCheck(&entry.exactMatchEntry);

                /* converst from key enum to size in bytes */
                CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(entry.exactMatchEntry.key.keySize,sizeInBytes);

                /* check entry exists also in cuckoo DB */
                st = prvCpssDxChCuckooDbDataCheck(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId,entry.exactMatchEntry.key.pattern,sizeInBytes,&found);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                UTF_VERIFY_EQUAL0_PARAM_MAC(found, GT_TRUE);
            }

            EM_MANAGER_ENTRY_DELETE_WITH_STATISTICS(exactMatchManagerId,&entry,st);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChExactMatchManagerEntryDelete: expected to GT_OK on manager [%d]",
                exactMatchManagerId);
            numEntriesRemoved++;

            if((numEntriesRemoved % 1000) == 0)
            {
                PRV_UTF_LOG1_MAC("removed [%d] entries \n",numEntriesRemoved);
            }
        }

        PRV_UTF_LOG2_MAC("ended - removed [%d] entries (out of expected [%d]) \n",numEntriesRemoved,numEntriesAdded);

        UTF_VERIFY_EQUAL2_STRING_MAC(numEntriesAdded, numEntriesRemoved,
            "expected number of removed entries [%d] to be the same as the added entries[%d]",
            numEntriesRemoved,numEntriesAdded);

        /* Verify DB */
        st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                    errorNumItr,
                    testResultArray[errorNumItr],
                    debugErrorArr[testResultArray[errorNumItr]].errString);
        }

        PRV_UTF_LOG0_MAC("Ended \n");
    }
    else
    {
        PRV_UTF_LOG0_MAC("Ended - Test fail , could not create manager - createExactMatchManagerWithDevice fail \n");
    }
    return;
}

/**
GT_STATUS cpssDxChExactMatchManagerEntryAdd
(
    IN GT_U32                                               exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC   *paramsPtr
);

GT_STATUS cpssDxChExactMatchManagerDelete
(
    IN GT_U32 exactMatchManagerId
);
*/
static void internal_cpssDxChExactMatchManagerCuckooEntryAddDeleteAddUT
(
    IN  GT_BOOL                                             isRandom,
    IN  CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT                  keySize
)
{
    GT_STATUS                                               st=GT_OK;
    GT_U32                                                  ii;
    GT_U32                                                  numEntriesRemoved=0;/* total number of entries removed from the DB */
    GT_U32                                                  numEntriesAdded=0,numEntriesAdded1=0;  /* total number of entries added to the DB */
    GT_U32                                                  numIndexesAdded,numIndexesAdded1;    /* number of entries added to each bank;
                                                                                if one entry is added with keysize=47 then:
                                                                                numEntriesAdded=1 and numIndexesAdded=4 */
    GT_BOOL                                                 entryAdded;/* indication that the entry was added to the DB */
    GT_BOOL                                                 getFirst;  /* indication to get first entry from the DB */
    GT_U32                                                  exactMatchManagerId = TESTED_EXACT_MATCH_MANAGER_ID_CNS;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                 entry;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC      params;
    GT_U32                                                  maxTotalEntries;
    UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC    outputResults;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      testResultArray[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNum=0, errorNumItr=0;
    GT_U32                                                  sizeInBytes=0;
    GT_BOOL                                                 found = GT_FALSE;

    /* avoid warnings */
    isRandom=isRandom;

    cpssOsMemSet(&entry ,0,sizeof(entry));
    cpssOsMemSet(&entry.exactMatchEntry.key.pattern ,0,sizeof(entry.exactMatchEntry.key.pattern));
    cpssOsMemSet(&params,0,sizeof(params));
    cpssOsMemSet(testResultArray,0,sizeof(testResultArray));

    outputResults.entryPtr          = &entry;
    outputResults.entryAddedPtr     = &entryAdded;

    numEntriesAdded = 0;
    numIndexesAdded = 0;
    maxTotalEntries = EXACT_MATCH_SIZE_FULL;

    if(em_debug_limited_entries)
    {
        maxTotalEntries = em_debug_limited_entries;
    }

    PRV_UTF_LOG0_MAC("create manager with device \n");

    /* create manager with device */
    st = createExactMatchManagerWithDevice(exactMatchManagerId,maxTotalEntries,prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "createExactMatchManagerWithDevice: expected to GT_OK on manager [%d]",
                exactMatchManagerId);

    if (st==GT_OK)
    {
        switch (keySize)
        {
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [5B]\n", maxTotalEntries);
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [19B]\n", maxTotalEntries);
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [33B]\n", maxTotalEntries);
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [47B]\n", maxTotalEntries);
            break;
        default: /* random key */
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [random]\n", maxTotalEntries);
        }

        for(ii = 0 ; ii < maxTotalEntries; /*update according to entry size*/)
        {
            entryAdded = GT_FALSE;
            exactMatchManagerEntryAddCuckoo_pcl(exactMatchManagerId, ii, &outputResults,GT_FALSE,keySize,GT_FALSE,GT_FALSE);
            if ((ii%1024) == 0)
            {
                cpssOsPrintf("%dk \n",ii/1024);
            }
            if(entryAdded)
            {
                numEntriesAdded++;
                numIndexesAdded+=(outputResults.entryPtr->exactMatchEntry.key.keySize+1);
                ii+=(outputResults.entryPtr->exactMatchEntry.key.keySize+1);
            }
            else
            {
                ii++;
            }
        }
        PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
            maxTotalEntries,numIndexesAdded,
                (100*numIndexesAdded)/maxTotalEntries);

        prvCpssDxChCuckooDbPrintStatistics(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);

        /* Verify DB */
        st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                    errorNumItr,
                    testResultArray[errorNumItr],
                    debugErrorArr[testResultArray[errorNumItr]].errString);
        }

        numEntriesRemoved = 0;
        getFirst = GT_TRUE;

        PRV_UTF_LOG1_MAC("start - remove first half [%d] entries \n",numEntriesAdded/2);
        if(check_hw_hold_entry == GT_TRUE)
        {
            PRV_UTF_LOG0_MAC("(slow mode) for each entry check that HW hold the entry in proper index according to it's index \n");
        }
        /* remove first half of the entries from the DB (one by one) */
        while(GT_OK == cpssDxChExactMatchManagerEntryGetNext(exactMatchManagerId,getFirst,&entry))
        {
            getFirst = GT_FALSE;

            if(check_hw_hold_entry == GT_TRUE)
            {
                /* just before we going to remove the entry ... lets check that it exists in the HW ! */
                exactMatchManagerHwEntryCheck(&entry.exactMatchEntry);

                /* converst from key enum to size in bytes */
                CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(entry.exactMatchEntry.key.keySize,sizeInBytes);

                /* check entry exists also in cuckoo DB */
                st = prvCpssDxChCuckooDbDataCheck(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId,entry.exactMatchEntry.key.pattern,sizeInBytes,&found);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                UTF_VERIFY_EQUAL0_PARAM_MAC(found, GT_TRUE);
            }

            EM_MANAGER_ENTRY_DELETE_WITH_STATISTICS(exactMatchManagerId,&entry,st);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChExactMatchManagerEntryDelete: expected to GT_OK on manager [%d]",
                exactMatchManagerId);
            numEntriesRemoved++;

            if((numEntriesRemoved % 1000) == 0)
            {
                PRV_UTF_LOG1_MAC("removed [%d] entries \n",numEntriesRemoved);
            }
            if(numEntriesRemoved == numEntriesAdded/2)
            {
                PRV_UTF_LOG1_MAC("removed first half [%d] entries \n",numEntriesRemoved);
                break;
            }
        }

        PRV_UTF_LOG2_MAC("ended - removed [%d] entries (out of expected [%d]) \n",numEntriesRemoved,numEntriesAdded/2);

        prvCpssDxChCuckooDbPrintStatistics(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);

        UTF_VERIFY_EQUAL2_STRING_MAC(numEntriesAdded/2, numEntriesRemoved,
            "expected number of removed entries [%d] to be half of the added entries[%d]",
            numEntriesRemoved,numEntriesAdded/2);

        /* Verify DB */
        st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                    errorNumItr,
                    testResultArray[errorNumItr],
                    debugErrorArr[testResultArray[errorNumItr]].errString);
        }

        /* add again all entries */
        numEntriesAdded1 = 0;
        numIndexesAdded1 = 0;
        switch (keySize)
        {
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [5B]\n", maxTotalEntries/2);
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [19B]\n", maxTotalEntries/2);
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [33B]\n", maxTotalEntries/2);
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [47B]\n", maxTotalEntries/2);
            break;
        default: /* random key */
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [random]\n", maxTotalEntries/2);
        }

        for(ii = 0 ; ii < maxTotalEntries; /*update according to entry size*/)
        {
            entryAdded = GT_FALSE;
            exactMatchManagerEntryAddCuckoo_pcl(exactMatchManagerId, ii, &outputResults,GT_FALSE,keySize,GT_FALSE,GT_FALSE);
            if ((ii%1024) == 0)
            {
                cpssOsPrintf("%dk \n",ii/1024);
            }
            if(entryAdded)
            {
                numEntriesAdded1++;
                numIndexesAdded1+=(outputResults.entryPtr->exactMatchEntry.key.keySize+1);
                ii+=(outputResults.entryPtr->exactMatchEntry.key.keySize+1);
            }
            else
            {
                ii++;
            }
        }
        PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
            maxTotalEntries,numIndexesAdded1,
                (100*numIndexesAdded1)/maxTotalEntries);

        if (numEntriesAdded1 < numEntriesRemoved)
        {
            PRV_UTF_LOG2_MAC("expected number of added entries [%d] to be equal or more than the number of removed entries[%d]",
                numEntriesAdded1,numEntriesRemoved);
        }

        prvCpssDxChCuckooDbPrintStatistics(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);

        /* Verify DB */
        st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                    errorNumItr,
                    testResultArray[errorNumItr],
                    debugErrorArr[testResultArray[errorNumItr]].errString);
        }

        /* remove second half of the entries from the DB (one by one) */
        numEntriesRemoved = 0;
        getFirst = GT_TRUE;
        while(GT_OK == cpssDxChExactMatchManagerEntryGetNext(exactMatchManagerId,getFirst,&entry))
        {
            getFirst = GT_FALSE;

            if(check_hw_hold_entry == GT_TRUE)
            {
                /* just before we going to remove the entry ... lets check that it exists in the HW ! */
                exactMatchManagerHwEntryCheck(&entry.exactMatchEntry);

                /* converst from key enum to size in bytes */
                CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(entry.exactMatchEntry.key.keySize,sizeInBytes);

                /* check entry exists also in cuckoo DB */
                st = prvCpssDxChCuckooDbDataCheck(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId,entry.exactMatchEntry.key.pattern,sizeInBytes,&found);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                UTF_VERIFY_EQUAL0_PARAM_MAC(found, GT_TRUE);
            }

            EM_MANAGER_ENTRY_DELETE_WITH_STATISTICS(exactMatchManagerId,&entry,st);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChExactMatchManagerEntryDelete: expected to GT_OK on manager [%d]",
                exactMatchManagerId);
            numEntriesRemoved++;

            if((numEntriesRemoved % 1000) == 0)
            {
                PRV_UTF_LOG1_MAC("removed [%d] entries \n",numEntriesRemoved);
            }
            if(numEntriesRemoved == numEntriesAdded/2)
            {
                PRV_UTF_LOG1_MAC("removed second half [%d] entries \n",numEntriesRemoved);
                break;
            }
        }

        PRV_UTF_LOG2_MAC("ended - removed [%d] entries (out of expected [%d]) \n",numEntriesRemoved,numEntriesAdded/2);

        prvCpssDxChCuckooDbPrintStatistics(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);

        UTF_VERIFY_EQUAL2_STRING_MAC(numEntriesAdded/2, numEntriesRemoved,
            "expected number of removed entries [%d] to be half of the added entries[%d]",
            numEntriesRemoved,numEntriesAdded/2);

        /* Verify DB */
        st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                    errorNumItr,
                    testResultArray[errorNumItr],
                    debugErrorArr[testResultArray[errorNumItr]].errString);
        }

        /* add again all entries */
        switch (keySize)
        {
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [5B]\n", maxTotalEntries/2);
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [19B]\n", maxTotalEntries/2);
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [33B]\n", maxTotalEntries/2);
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [47B]\n", maxTotalEntries/2);
            break;
        default: /* random key */
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [random]\n", maxTotalEntries/2);
        }
        numEntriesAdded1 = 0;
        numIndexesAdded1 = 0;
        for(ii = 0 ; ii < maxTotalEntries; /*update according to entry size*/)
        {
            entryAdded = GT_FALSE;
            exactMatchManagerEntryAddCuckoo_pcl(exactMatchManagerId, ii, &outputResults,GT_FALSE,keySize,GT_FALSE,GT_FALSE);
            if ((ii%1024) == 0)
            {
                cpssOsPrintf("%dk \n",ii/1024);
            }
            if(entryAdded)
            {
                numEntriesAdded1++;
                numIndexesAdded1+=(outputResults.entryPtr->exactMatchEntry.key.keySize+1);
                ii+=(outputResults.entryPtr->exactMatchEntry.key.keySize+1);
            }
            else
            {
                ii++;
            }
        }
        PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
            maxTotalEntries,numIndexesAdded1,
                (100*numIndexesAdded1)/maxTotalEntries);

        UTF_VERIFY_EQUAL2_STRING_MAC(numEntriesAdded1, numEntriesRemoved,
            "expected number of added entries [%d] to be equal to the number of removed entries[%d]",
            numEntriesAdded1,numEntriesRemoved);

        prvCpssDxChCuckooDbPrintStatistics(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);

        /* Verify DB */
        st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                    errorNumItr,
                    testResultArray[errorNumItr],
                    debugErrorArr[testResultArray[errorNumItr]].errString);
        }


         /* remove all the entries from the DB (one by one) */
        numEntriesRemoved = 0;
        getFirst = GT_TRUE;
        while(GT_OK == cpssDxChExactMatchManagerEntryGetNext(exactMatchManagerId,getFirst,&entry))
        {
            getFirst = GT_FALSE;

            if(check_hw_hold_entry == GT_TRUE)
            {
                /* just before we going to remove the entry ... lets check that it exists in the HW ! */
                exactMatchManagerHwEntryCheck(&entry.exactMatchEntry);

                /* converst from key enum to size in bytes */
                CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(entry.exactMatchEntry.key.keySize,sizeInBytes);

                /* check entry exists also in cuckoo DB */
                st = prvCpssDxChCuckooDbDataCheck(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId,entry.exactMatchEntry.key.pattern,sizeInBytes,&found);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
                UTF_VERIFY_EQUAL0_PARAM_MAC(found, GT_TRUE);
            }

            EM_MANAGER_ENTRY_DELETE_WITH_STATISTICS(exactMatchManagerId,&entry,st);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChExactMatchManagerEntryDelete: expected to GT_OK on manager [%d]",
                exactMatchManagerId);
            numEntriesRemoved++;

            if((numEntriesRemoved % 1000) == 0)
            {
                PRV_UTF_LOG1_MAC("removed [%d] entries \n",numEntriesRemoved);
            }
            if(numEntriesRemoved == numEntriesAdded)
            {
                PRV_UTF_LOG1_MAC("removed [%d] entries \n",numEntriesRemoved);
                break;
            }
        }

        PRV_UTF_LOG2_MAC("ended - removed [%d] entries (out of expected [%d]) \n",numEntriesRemoved,numEntriesAdded);

        UTF_VERIFY_EQUAL2_STRING_MAC(numEntriesAdded, numEntriesRemoved,
            "expected number of removed entries [%d] to be the same as added entries[%d]",
            numEntriesRemoved,numEntriesAdded);

        /* Verify DB */
        st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                    errorNumItr,
                    testResultArray[errorNumItr],
                    debugErrorArr[testResultArray[errorNumItr]].errString);
        }

        PRV_UTF_LOG0_MAC("Ended \n");
    }
    else
    {
        PRV_UTF_LOG0_MAC("Ended - Test fail , could not create manager - createExactMatchManagerWithDevice fail \n");
    }
    return;
}

static void internal_cpssDxChExactMatchManagerCuckooEntryAddSingleUT
(
    IN  CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT                  keySize
)
{
    GT_STATUS                                               st=GT_OK;
    GT_U32                                                  maxTotalEntries;
    GT_BOOL                                                 entryAdded;/* indication that the entry was added to the DB */
    GT_U32                                                  exactMatchManagerId = TESTED_EXACT_MATCH_MANAGER_ID_CNS;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                 entry;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC      params;
    UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC    outputResults;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      testResultArray[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNum=0, errorNumItr=0;
    static GT_BOOL                                          isFirstTime = GT_TRUE;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    static GT_U32                                           randomId = 0;

    cpssOsMemSet(&entry ,0,sizeof(entry));
    cpssOsMemSet(&entry.exactMatchEntry.key.pattern ,0,sizeof(entry.exactMatchEntry.key.pattern));
    cpssOsMemSet(&params,0,sizeof(params));
    cpssOsMemSet(testResultArray,0,sizeof(testResultArray));

    outputResults.entryPtr          = &entry;
    outputResults.entryAddedPtr     = &entryAdded;
    maxTotalEntries = EXACT_MATCH_SIZE_FULL;

    if (isFirstTime)
    {
        PRV_UTF_LOG0_MAC("create manager with device \n");

        /* create manager with device */
        st = createExactMatchManagerWithDevice(exactMatchManagerId,maxTotalEntries,prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "createExactMatchManagerWithDevice: expected to GT_OK on manager [%d]",
                    exactMatchManagerId);
        randomId = prvUtfSeedFromStreamNameGet();

        isFirstTime = GT_FALSE;
    }

    if (st==GT_OK)
    {
        switch (keySize)
        {
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
            PRV_UTF_LOG0_MAC("add single entry for keySize [5B]     ");
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
            PRV_UTF_LOG0_MAC("add single entry for keySize [19B]    ");
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
            PRV_UTF_LOG0_MAC("add single entry for keySize [33B]    ");
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
            PRV_UTF_LOG0_MAC("add single entry for keySize [47B]    ");
            break;
        default:
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_PARAM,"random key not supported");

        }

        entryAdded = GT_FALSE;

        exactMatchManagerEntryAddCuckoo_pcl(exactMatchManagerId,randomId,&outputResults,GT_FALSE,keySize,GT_FALSE,GT_TRUE);

        exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

        if(entryAdded)
        {
            PRV_UTF_LOG0_MAC("entry was added\n");
        }
        else
        {
            PRV_UTF_LOG0_MAC("failed to add entry\n");
            if ((exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes == 4) && (keySize == CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E))
            {
                UTF_VERIFY_NOT_EQUAL1_STRING_MAC(GT_TRUE, entryAdded,
                    "cpssDxChExactMatchManagerEntryAdd: expected to FAIL on manager [%d]",
                    exactMatchManagerId);
            }
            else
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, entryAdded,
                    "cpssDxChExactMatchManagerEntryAdd: expected to succeed on manager [%d]",
                    exactMatchManagerId);
            }

        }

        if (keySize == CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E)
        {
            prvCpssDxChCuckooDbPrintStatistics(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);
        }

        /* Verify DB */
        st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                    errorNumItr,
                    testResultArray[errorNumItr],
                    debugErrorArr[testResultArray[errorNumItr]].errString);
        }
    }
    else
    {
        PRV_UTF_LOG0_MAC("Ended - Test fail , could not create manager - createExactMatchManagerWithDevice fail \n");
    }
    return;
}

/**
GT_STATUS cpssDxChExactMatchManagerEntryAdd
(
    IN GT_U32                                               exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC   *paramsPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerEntryAdd)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    check_hw_hold_entry = GT_TRUE;/* very slow mode ! */

    internal_cpssDxChExactMatchManagerEntryAddUT();

    check_hw_hold_entry = GT_FALSE;

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}

/* update Exact Match entry in the manager */
static void exactMatchManagerEntryUpdate(
    IN      GT_U32                                   exactMatchManagerId,
    IN      GT_U32                                   iterationNum,
    IN      CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC  *entryPtr,
    IN      CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    INOUT   GT_BOOL                                  *entryUpfdatedPtr
)
{
    GT_STATUS   st=GT_OK;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                         entryGet;
    GT_BOOL                                                         checkCounters;
    CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC                      counters, counters1;
    CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC                    statistics, statistics1;
    GT_U32                                                          *statisticValuePtr = (GT_U32 *)&statistics;
    GT_U32                                                          *statisticValuePtr1 =(GT_U32 *)&statistics1;
    GT_U32                                                          ii, maxStatistics;
    GT_U32                                                          rand32Bits = cpssOsRand() | iterationNum << 16;

    maxStatistics = sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC) / 4;

    *entryUpfdatedPtr = GT_FALSE;

    checkCounters = GT_TRUE;
    st = cpssDxChExactMatchManagerCountersGet(exactMatchManagerId, &counters);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "cpssDxChExactMatchManagerCountersGet: expected GT_OK in manager [%d]",
        exactMatchManagerId);

    st = cpssDxChExactMatchManagerStatisticsGet(exactMatchManagerId, &statistics);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
        "cpssDxChExactMatchManagerStatisticsGet: expected GT_OK in manager [%d]",
        exactMatchManagerId);

    /* change some field and update the entry */
    switch(actionType)
    {
    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
        exactMatchManagerEntryBuild_DstInterfaceInfo(rand32Bits,  &entryPtr->exactMatchAction.ttiAction.egressInterface);
        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
        entryPtr->exactMatchAction.pclAction.terminateCutThroughMode = (rand32Bits%2)?GT_TRUE:GT_FALSE;
        entryPtr->exactMatchAction.pclAction.skipFdbSaLookup = (rand32Bits%2)?GT_TRUE:GT_FALSE;
        entryPtr->exactMatchAction.pclAction.pktCmd = (rand32Bits%2)?CPSS_PACKET_CMD_DROP_HARD_E:CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
        entryPtr->exactMatchAction.pclAction.pktCmd = (rand32Bits%2)?CPSS_PACKET_CMD_DROP_HARD_E:CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
        entryPtr->exactMatchAction.pclAction.skipFdbSaLookup = (rand32Bits%2)?GT_TRUE:GT_FALSE;
        break;
    default:
        break;
    }

    EM_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(exactMatchManagerId,entryPtr,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "cpssDxChExactMatchManagerEntryUpdate: unexpected error in manager [%d]",
        exactMatchManagerId);

    if(st == GT_OK)
    {
        /* the entry was upated ... check if the entry retrieved as was set */

        /* start -- the parts of the key */
        cpssOsMemSet(&entryGet,0,sizeof(entryGet));
        cpssOsMemCpy(&entryGet.exactMatchEntry.lookupNum,&entryPtr->exactMatchEntry.lookupNum,sizeof(entryPtr->exactMatchEntry.lookupNum));
        cpssOsMemCpy(&entryGet.exactMatchEntry.key.keySize,&entryPtr->exactMatchEntry.key.keySize,sizeof(entryPtr->exactMatchEntry.key.keySize));
        cpssOsMemCpy(&entryGet.exactMatchEntry.key.pattern,&entryPtr->exactMatchEntry.key.pattern,sizeof(entryPtr->exactMatchEntry.key.pattern));

        /* end -- the parts of the key */
        entryGet.exactMatchActionType = actionType;

        st = cpssDxChExactMatchManagerEntryGet(exactMatchManagerId, &entryGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
            "cpssDxChExactMatchManagerEntryGet: expected GT_OK in manager [%d]",
            exactMatchManagerId);

        /***************************************************************/
        /* compare the full entry action */
        /***************************************************************/

        st = prvCompareExactMatchActionStructs(prvTgfDevNum, "action", actionType, &entryPtr->exactMatchAction, &entryGet.exactMatchAction);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,"action != actionGet: %d, %d, %d, %d\n",prvTgfDevNum, entryPtr->expandedActionIndex, actionType);

        UTF_VERIFY_EQUAL1_STRING_MAC(entryPtr->exactMatchActionType, entryGet.exactMatchActionType,
            "cpssDxChExactMatchManagerEntryGet: expected exactMatchActionType in 'get' entry to match 'set' in manager [%d]",
            exactMatchManagerId);

        if (checkCounters == GT_TRUE)
        {
            st = cpssDxChExactMatchManagerCountersGet(exactMatchManagerId, &counters1);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
                "cpssDxChExactMatchManagerCountersGet: expected GT_OK in manager [%d]",
                exactMatchManagerId);

            switch (actionType)
            {
            case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
                 /* make sure no change in counters was done in update */
                UTF_VERIFY_EQUAL1_STRING_MAC(0, counters.ttiClientKeySizeEntriesArray[entryPtr->exactMatchEntry.key.keySize] -
                                  counters1.ttiClientKeySizeEntriesArray[entryPtr->exactMatchEntry.key.keySize],
                                 "cpssDxChExactMatchManagerCountersGet: expected 'get' entry not equal 'set' entry in manager [%d]",
                                 exactMatchManagerId);
                break;
            case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
                UTF_VERIFY_EQUAL1_STRING_MAC(0, counters.ipcl0ClientKeySizeEntriesArray[entryPtr->exactMatchEntry.key.keySize] -
                                  counters1.ipcl0ClientKeySizeEntriesArray[entryPtr->exactMatchEntry.key.keySize],
                                 "cpssDxChExactMatchManagerCountersGet: expected 'get' entry not equal 'set' entry in manager [%d]",
                                 exactMatchManagerId);
                UTF_VERIFY_EQUAL1_STRING_MAC(0, counters.ipcl1ClientKeySizeEntriesArray[entryPtr->exactMatchEntry.key.keySize] -
                                  counters1.ipcl1ClientKeySizeEntriesArray[entryPtr->exactMatchEntry.key.keySize],
                                 "cpssDxChExactMatchManagerCountersGet: expected 'get' entry not equal 'set' entry in manager [%d]",
                                 exactMatchManagerId);
                UTF_VERIFY_EQUAL1_STRING_MAC(0, counters.ipcl2ClientKeySizeEntriesArray[entryPtr->exactMatchEntry.key.keySize] -
                                  counters1.ipcl2ClientKeySizeEntriesArray[entryPtr->exactMatchEntry.key.keySize],
                                 "cpssDxChExactMatchManagerCountersGet: expected 'get' entry not equal 'set' entry in manager [%d]",
                                 exactMatchManagerId);
                break;
            case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
                UTF_VERIFY_EQUAL1_STRING_MAC(0, counters.epclClientKeySizeEntriesArray[entryPtr->exactMatchEntry.key.keySize] -
                                  counters1.epclClientKeySizeEntriesArray[entryPtr->exactMatchEntry.key.keySize],
                                 "cpssDxChExactMatchManagerCountersGet: expected 'get' entry not equal 'set' entry in manager [%d]",
                                 exactMatchManagerId);
                break;
            default:
                st=GT_BAD_PARAM;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
                                             "cpssDxChExactMatchManagerCountersGet: expected GT_OK in manager [%d]",
                                             exactMatchManagerId);
                break;
            }


            st = cpssDxChExactMatchManagerStatisticsGet(exactMatchManagerId, &statistics1);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
                "cpssDxChExactMatchManagerStatisticsGet: expected GT_OK in manager [%d]",
                exactMatchManagerId);

            for (ii = 0; ii < maxStatistics; ii++)
            {

                if (&statisticValuePtr[ii] == &statistics.entryUpdateOk)
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(1, statistics1.entryUpdateOk - statistics.entryUpdateOk,
                                                 "cpssDxChExactMatchManagerStatisticsGet: expected 'get' entry not equal 'set' entry in manager [%d]",
                                                 exactMatchManagerId);
                }
                else
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(0, statisticValuePtr1[ii] - statisticValuePtr[ii],
                                                 "cpssDxChExactMatchManagerStatisticsGet: expected 'get' entry not equal 'set' entry in manager [%d]",
                                                 exactMatchManagerId);
                }
            }

        }
        *entryUpfdatedPtr = GT_TRUE;
    }
}

/* Update exact match entry to the manager : type :tti */
static void exactMatchManagerEntryUpdate_tti(
    IN      GT_U32                                   exactMatchManagerId,
    IN      GT_U32                                   iterationNum,
    IN      CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC  *entryPtr,
    INOUT   GT_BOOL                                  *entryUpfdatedPtr
)
{
    exactMatchManagerEntryUpdate(exactMatchManagerId,iterationNum,entryPtr, CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E,entryUpfdatedPtr);
    return;
}
/* Update exact match entry to the manager : type :pcl */
static void exactMatchManagerEntryUpdate_pcl(
    IN      GT_U32                                   exactMatchManagerId,
    IN      GT_U32                                   iterationNum,
    IN      CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC  *entryPtr,
    INOUT   GT_BOOL                                  *entryUpfdatedPtr
)
{
    exactMatchManagerEntryUpdate(exactMatchManagerId,iterationNum,entryPtr, CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E,entryUpfdatedPtr);
    return;
}
/* Update exact match entry to the manager : type :epcl */
static void exactMatchManagerEntryUpdate_epcl(
    IN      GT_U32                                   exactMatchManagerId,
    IN      GT_U32                                   iterationNum,
    IN      CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC  *entryPtr,
    INOUT   GT_BOOL                                  *entryUpfdatedPtr
)
{
    exactMatchManagerEntryUpdate(exactMatchManagerId,iterationNum,entryPtr, CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E,entryUpfdatedPtr);
    return;
}
/**
GT_STATUS cpssDxChExactMatchManagerEntryUpdate
(
    IN GT_U32                                                exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC               *entryPtr
);
*/
static void internal_cpssDxChExactMatchManagerEntryUpdateUT(void)
{
    GT_STATUS   st=GT_OK;
    GT_U32      ii;
    GT_U32      numEntriesRemoved=0;/* total number of entries removed from the DB */
    GT_U32      numEntriesAdded;  /* total number of entries added to the DB */
    GT_U32      numIndexesAdded;/* number of entries added to each bank;
                                   if one entry is added with keysize=47 then:
                                   numEntriesAdded=1 and numIndexesAdded=4 */
    GT_U32      numEntriesUpdated;  /* total number of entries updated in the DB */
    GT_BOOL     entryAdded;/* indication that the entry was added to the DB */
    GT_BOOL     entryUpdated;/* indication that the entry was updated in the DB */
    GT_BOOL     getFirst;  /* indication to get first entry from the DB */
    GT_U32      exactMatchManagerId = TESTED_EXACT_MATCH_MANAGER_ID_CNS;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                 entry;
    GT_U32                                                  maxTotalEntries;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT                   exactMatchEntryType;
    UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC    outputResults;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      testResultArray[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNum, errorNumItr;

    cpssOsMemSet(&entry ,0,sizeof(entry));

    outputResults.entryPtr          = &entry;
    outputResults.entryAddedPtr     = &entryAdded;

    PRV_UTF_LOG0_MAC("Check non-initialized managers \n");

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        EM_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(ii,&entry,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChExactMatchManagerEntryUpdate: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    PRV_UTF_LOG0_MAC("Check check out of range managers \n");
    /* check out of range managers ... expect 'GT_OUT_OF_RANGE' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        EM_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(ii,&entry,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
            "cpssDxChExactMatchManagerEntryUpdate: expected to GT_OUT_OF_RANGE on manager [%d]",
            ii);
    }

    numEntriesAdded = 0;
    numIndexesAdded = 0;
    maxTotalEntries = EXACT_MATCH_SIZE_FULL;

    if(em_debug_limited_entries)
    {
        maxTotalEntries = em_debug_limited_entries;
    }

    if(run_device_less == GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("create manager with device \n");
    }
    else
    {
        PRV_UTF_LOG0_MAC("create manager without device \n");
    }
    /* create manager with device */
    st = createExactMatchManagerWithDevice(exactMatchManagerId,maxTotalEntries,prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "createExactMatchManagerWithDevice: expected to GT_OK on manager [%d]",
                exactMatchManagerId);
    if (st==GT_OK)
    {
        PRV_UTF_LOG1_MAC("start - add [%d] entries \n",maxTotalEntries);
        for(ii = 0 ; ii < maxTotalEntries; /*update according to entry size*/)
        {
            entryAdded = GT_FALSE;

            exactMatchEntryType = exactMatchEntryType_valid_arr[ii % UT_EXACT_MATCH_ENTRY_TYPE_VALID_NUM];
            switch(exactMatchEntryType)
            {
                case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
                    exactMatchManagerEntryAdd_tti(exactMatchManagerId, ii, &outputResults);
                    break;
                case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
                    exactMatchManagerEntryAdd_pcl(exactMatchManagerId, ii, &outputResults);
                    break;
                case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
                    exactMatchManagerEntryAdd_epcl(exactMatchManagerId, ii, &outputResults);
                    break;
                default:
                    break;
            }
            if(entryAdded)
            {
                numEntriesAdded++;
                numIndexesAdded+=(outputResults.entryPtr->exactMatchEntry.key.keySize+1);
            }
            if(add_device_in_runtime == GT_TRUE &&
               (ii == (maxTotalEntries/2)))
            {
                CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[1];
                GT_U32                                                   numOfPairs=1;

                /* now in the middle of the insertions we add the device. */
                PRV_UTF_LOG0_MAC("now in the middle of the insertions we add the device \n");

                pairListArr[0].devNum = prvTgfDevNum;
                pairListArr[0].portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

                /* add device to it */
                st = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChExactMatchManagerDevListAdd: expected to GT_OK on manager [%d]",
                    exactMatchManagerId);
            }
            if(entryAdded)
            {
                ii+=(outputResults.entryPtr->exactMatchEntry.key.keySize+1);
            }
            else
            {
                ii++;
            }
        }
        PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
            maxTotalEntries,numIndexesAdded,
                (100*numIndexesAdded)/maxTotalEntries);

        if(em_debug_limited_entries)
        {
            cpssDxChExactMatchRulesDump(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,0,(EXACT_MATCH_SIZE_FULL-1));
        }

        /* Verify DB */
        st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                    errorNumItr,
                    testResultArray[errorNumItr],
                    debugErrorArr[testResultArray[errorNumItr]].errString);
        }

        numEntriesRemoved = 0;
        numEntriesUpdated = 0;
        getFirst = GT_TRUE;

        PRV_UTF_LOG1_MAC("start - remove the [%d] entries \n",numEntriesAdded);
        if(check_hw_hold_entry == GT_TRUE)
        {
            PRV_UTF_LOG0_MAC("(slow mode) for each entry check that HW hold the entry in proper index according to it's index \n");
        }

        ii = 0;

        /* update all entries from the DB (one by one) */
        while(GT_OK == cpssDxChExactMatchManagerEntryGetNext(exactMatchManagerId,getFirst,&entry))
        {
            getFirst = GT_FALSE;

            if(check_hw_hold_entry == GT_TRUE)
            {
                /* just before we going to remove the entry ... lets check that it exists in the HW ! */
                 exactMatchManagerHwEntryCheck(&entry.exactMatchEntry);
            }

            entryUpdated = GT_TRUE;

            switch(entry.exactMatchActionType)
            {
                case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
                    exactMatchManagerEntryUpdate_tti(exactMatchManagerId, ii, &entry, &entryUpdated);
                    break;
                case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
                    exactMatchManagerEntryUpdate_pcl(exactMatchManagerId, ii, &entry, &entryUpdated);
                    break;
                case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
                    exactMatchManagerEntryUpdate_epcl(exactMatchManagerId, ii, &entry, &entryUpdated);
                    break;
                default:
                    break;
            }

            if (entryUpdated == GT_TRUE)
            {
                numEntriesUpdated++;
            }
            if((numEntriesUpdated % 1000) == 0)
            {
                PRV_UTF_LOG1_MAC("updated [%d] entries \n",numEntriesUpdated);
            }

            EM_MANAGER_ENTRY_DELETE_WITH_STATISTICS(exactMatchManagerId,&entry,st);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChExactMatchManagerEntryDelete: expected to GT_OK on manager [%d]",
                exactMatchManagerId);
            numEntriesRemoved++;

            if((numEntriesRemoved % 1000) == 0)
            {
                PRV_UTF_LOG1_MAC("removed [%d] entries \n",numEntriesRemoved);
            }
            ii++;
        }
    }
    else
    {
        PRV_UTF_LOG0_MAC("Ended - Test fail , could not create manager - createExactMatchManagerWithDevice fail \n");
    }
    PRV_UTF_LOG2_MAC("ended - removed [%d] entries (out of expected [%d]) \n",numEntriesRemoved,numEntriesAdded);

    UTF_VERIFY_EQUAL2_STRING_MAC(numEntriesAdded, numEntriesRemoved,
        "expected number of removed entries [%d] to be the same as the added entries[%d]",
        numEntriesRemoved,numEntriesAdded);

    /* Verify DB */
    st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                errorNumItr,
                testResultArray[errorNumItr],
                debugErrorArr[testResultArray[errorNumItr]].errString);
    }

    /* check invalid parameters */
    for(ii = 0 ; ii < UT_EXACT_MATCH_ENTRY_TYPE_VALID_NUM; ii++)
    {
        exactMatchEntryType = exactMatchEntryType_valid_arr[ii % UT_EXACT_MATCH_ENTRY_TYPE_VALID_NUM];
        switch(exactMatchEntryType)
        {
            case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
                PRV_UTF_LOG0_MAC("check invalid parameters : tti \n");
                bad_param_check_exactMatchManagerEntryUpdate_tti(exactMatchManagerId);
                break;
            case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
                PRV_UTF_LOG0_MAC("check invalid parameters : pcl \n");
                bad_param_check_exactMatchManagerEntryUpdate_pcl(exactMatchManagerId);
                break;
            case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
                PRV_UTF_LOG0_MAC("check invalid parameters : epcl \n");
                bad_param_check_exactMatchManagerEntryUpdate_epcl(exactMatchManagerId);
                break;
            default:
                break;
        }

    }

    PRV_UTF_LOG0_MAC("check NULL pointers \n");
    /* NULL pointer */
    EM_MANAGER_ENTRY_UPDATE_WITH_STATISTICS(exactMatchManagerId,NULL,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChExactMatchManagerEntryAdd: expected to GT_BAD_PTR on manager [%d]",
        exactMatchManagerId);

    /* Verify DB */
    st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                errorNumItr,
                testResultArray[errorNumItr],
                debugErrorArr[testResultArray[errorNumItr]].errString);
    }

    PRV_UTF_LOG0_MAC("Ended \n");
    return;
}

/**
GT_STATUS cpssDxChExactMatchManagerEntryUpdate
(
    IN GT_U32                                                exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC               *entryPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerEntryUpdate)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    check_hw_hold_entry = GT_TRUE;/* very slow mode ! */

    internal_cpssDxChExactMatchManagerEntryUpdateUT();

    check_hw_hold_entry = GT_FALSE;

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}

static void exactMatchManagerEntry_updateDbAddStatistics(
    IN  GT_STATUS                                           status,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC        *statisticsPtr
)
{
    switch (status)
    {
        case GT_OK:
            statisticsPtr->entryAddOkRehashingStage0++;
            break;
        case GT_BAD_PARAM:
        case GT_BAD_PTR:
        case GT_OUT_OF_RANGE:
            statisticsPtr->entryAddErrorInputInvalid++;
            break;
        case GT_FULL:
            statisticsPtr->entryAddErrorTableFull++;
            break;
        case GT_BAD_STATE:
            statisticsPtr->entryAddErrorBadState++;
            break;
        case GT_HW_ERROR:
            break;
        case GT_ALREADY_EXIST:
            statisticsPtr->entryAddErrorEntryExist++;
            break;
        case GT_NOT_FOUND:
            statisticsPtr->entryAddErrorReplayEntryNotFound++;
            break;
        default:
            PRV_UTF_LOG1_MAC("exactMatchManagerEntry_updateDbAddStatistics: bad parameter [%d]) \n", status);
    }
}

static void exactMatchManagerEntry_updateDbUpdateStatistics(
    IN  GT_STATUS                                           status,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC        *statisticsPtr
)
{
    switch (status)
    {
        case GT_OK:
            statisticsPtr->entryUpdateOk++;
            break;
        case GT_BAD_PARAM:
        case GT_BAD_PTR:
        case GT_OUT_OF_RANGE:
            statisticsPtr->entryUpdateErrorInputInvalid++;
            break;
        case GT_NOT_FOUND:
            statisticsPtr->entryUpdateErrorNotFound++;
            break;
        default:
            PRV_UTF_LOG1_MAC("exactMatchManagerEntry_updateDbUpdateStatistics: bad parameter [%d]) \n", status);
    }
}

static void exactMatchManagerEntry_updateDbDeleteStatistics(
    IN  GT_STATUS                                           status,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC        *statisticsPtr
)
{
    switch (status)
    {
        case GT_OK:
            statisticsPtr->entryDeleteOk++;
            break;
        case GT_BAD_PARAM:
        case GT_BAD_PTR:
        case GT_OUT_OF_RANGE:
            statisticsPtr->entryDeleteErrorInputInvalid++;
            break;
        case GT_NOT_FOUND:
            statisticsPtr->entryDeleteErrorNotfound++;
            break;
        default:
            PRV_UTF_LOG1_MAC("exactMatchManagerEntry_updateDbDeleteStatistics: bad parameter [%d]) \n", status);
    }
}

static void exactMatchManagerEntry_updateDbStatistics(
    IN  UTF_PRV_CPSS_EXACT_MATCH_API_STATISTICS_ENT         statisticType,
    IN  GT_STATUS                                           status,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC        *statisticsPtr
)
{
    switch (statisticType)
    {
        case UTF_PRV_CPSS_API_STATISTICS_ADD_E:
            exactMatchManagerEntry_updateDbAddStatistics(status, statisticsPtr);
            break;
        case UTF_PRV_CPSS_API_STATISTICS_DELETE_E:
            exactMatchManagerEntry_updateDbDeleteStatistics(status, statisticsPtr);
            break;
        case UTF_PRV_CPSS_API_STATISTICS_UPDATE_E:
            exactMatchManagerEntry_updateDbUpdateStatistics(status, statisticsPtr);
            break;
        default:
            break;
    }
}

static void bad_param_check_exactMatchManagerEntryDelete_tti(
    IN GT_U32   exactMatchManagerId
)
{
    GT_STATUS   st=GT_OK;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC              entry;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC   exactMatchEntryAddInfo;

    /* set valid key parameters */
    entry.exactMatchEntry.key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
    entry.exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
    entry.expandedActionIndex = 7;
    entry.exactMatchUserDefined = 7;

    /* build 'valid' entry : tti */
    entry.exactMatchActionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
    prvSetExactMatchActionDefaultValues(&entry.exactMatchAction,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

    exactMatchEntryAddInfo.rehashEnable    = GT_FALSE;

    PRV_UTF_LOG0_MAC("check that info for the entry is valid \n");
    /* check that info for the entry is valid */
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                 "exactMatchManagerEntryAdd_tti: unexpected GT_OK [%d]",
                                 exactMatchManagerId);

    PRV_UTF_LOG0_MAC("remove the entry \n");
    /* remove the entry */
    EM_MANAGER_ENTRY_DELETE_WITH_STATISTICS(exactMatchManagerId,&entry,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                 "cpssDxChExactMatchManagerEntryDelete: unexpected GT_OK [%d]",
                                 exactMatchManagerId);

    /**************************************************************/
    /* check illegal key values for the delete                    */
    /**************************************************************/
    entry.exactMatchEntry.key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E;
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChExactMatchManagerEntryDelete:keySize expected to GT_BAD_PARAM on value [%d]",
            entry.exactMatchEntry.key.keySize);
    entry.exactMatchEntry.key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;

    entry.exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChExactMatchManagerEntryDelete:lookupNum expected to GT_BAD_PARAM on value [%d]",
            entry.exactMatchEntry.lookupNum);
    entry.exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;

    entry.expandedActionIndex=8;
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChExactMatchManagerEntryDelete:expandedActionIndex expected to GT_BAD_PARAM on value [%d]",
            entry.expandedActionIndex);
    entry.expandedActionIndex=16;
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChExactMatchManagerEntryDelete: expandedActionIndex expected to GT_BAD_PARAM on value [%d]",
            entry.expandedActionIndex);
    entry.expandedActionIndex=7;

    entry.exactMatchUserDefined=BIT_16;
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChExactMatchManagerEntryDelete: expected to GT_BAD_PARAM on value [%d]",
            entry.exactMatchUserDefined);
    entry.exactMatchUserDefined=7;
}

static void bad_param_check_exactMatchManagerEntryDelete_pcl(
    IN GT_U32   exactMatchManagerId
)
{
    GT_STATUS   st=GT_OK;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC              entry;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC   exactMatchEntryAddInfo;

    /* set valid key parameters */
    entry.exactMatchEntry.key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
    entry.exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
    entry.expandedActionIndex = 15;
    entry.exactMatchUserDefined = 15;

    /* build 'valid' entry : pcl */
    entry.exactMatchActionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
    prvSetExactMatchActionDefaultValues(&entry.exactMatchAction,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

    exactMatchEntryAddInfo.rehashEnable    = GT_FALSE;

    PRV_UTF_LOG0_MAC("check that info for the entry is valid \n");
    /* check that info for the entry is valid */
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                 "exactMatchManagerEntryAdd_tti: unexpected GT_OK [%d]",
                                 exactMatchManagerId);

    PRV_UTF_LOG0_MAC("remove the entry \n");
    /* remove the entry */
    EM_MANAGER_ENTRY_DELETE_WITH_STATISTICS(exactMatchManagerId,&entry,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                 "cpssDxChExactMatchManagerEntryDelete: unexpected GT_OK [%d]",
                                 exactMatchManagerId);

    /**************************************************************/
    /* check illegal key values for the delete                    */
    /**************************************************************/
    entry.exactMatchEntry.key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E;
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChExactMatchManagerEntryDelete:keySize expected to GT_BAD_PARAM on value [%d]",
            entry.exactMatchEntry.key.keySize);
    entry.exactMatchEntry.key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;

    entry.exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChExactMatchManagerEntryDelete:lookupNum expected to GT_BAD_PARAM on value [%d]",
            entry.exactMatchEntry.lookupNum);
    entry.exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;

    entry.expandedActionIndex=7;
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChExactMatchManagerEntryDelete:expandedActionIndex expected to GT_BAD_PARAM on value [%d]",
            entry.expandedActionIndex);
    entry.expandedActionIndex=16;
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChExactMatchManagerEntryDelete: expandedActionIndex expected to GT_BAD_PARAM on value [%d]",
            entry.expandedActionIndex);
    entry.expandedActionIndex=15;

    entry.exactMatchUserDefined=BIT_16;
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChExactMatchManagerEntryDelete: expected to GT_BAD_PARAM on value [%d]",
            entry.exactMatchUserDefined);
    entry.exactMatchUserDefined=7;
}

static void bad_param_check_exactMatchManagerEntryDelete_epcl(
    IN GT_U32   exactMatchManagerId
)
{
    GT_STATUS   st=GT_OK;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC              entry;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC   exactMatchEntryAddInfo;

    /* set valid key parameters */
    entry.exactMatchEntry.key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
    entry.exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
    entry.expandedActionIndex = 15;
    entry.exactMatchUserDefined = 15;

    /* build 'valid' entry : pcl */
    entry.exactMatchActionType = CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E;
    prvSetExactMatchActionDefaultValues(&entry.exactMatchAction,CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E);

    exactMatchEntryAddInfo.rehashEnable    = GT_FALSE;

    PRV_UTF_LOG0_MAC("check that info for the entry is valid \n");
    /* check that info for the entry is valid */
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                 "exactMatchManagerEntryAdd_tti: unexpected GT_OK [%d]",
                                 exactMatchManagerId);

    PRV_UTF_LOG0_MAC("remove the entry \n");
    /* remove the entry */
    EM_MANAGER_ENTRY_DELETE_WITH_STATISTICS(exactMatchManagerId,&entry,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                 "cpssDxChExactMatchManagerEntryDelete: unexpected GT_OK [%d]",
                                 exactMatchManagerId);

    /**************************************************************/
    /* check illegal key values for the delete                    */
    /**************************************************************/
    entry.exactMatchEntry.key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E;
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChExactMatchManagerEntryDelete:keySize expected to GT_BAD_PARAM on value [%d]",
            entry.exactMatchEntry.key.keySize);
    entry.exactMatchEntry.key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;

    entry.exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChExactMatchManagerEntryDelete:lookupNum expected to GT_BAD_PARAM on value [%d]",
            entry.exactMatchEntry.lookupNum);
    entry.exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;

    entry.expandedActionIndex=7;
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChExactMatchManagerEntryDelete:expandedActionIndex expected to GT_BAD_PARAM on value [%d]",
            entry.expandedActionIndex);
    entry.expandedActionIndex=16;
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChExactMatchManagerEntryDelete: expandedActionIndex expected to GT_BAD_PARAM on value [%d]",
            entry.expandedActionIndex);
    entry.expandedActionIndex=15;

    entry.exactMatchUserDefined=BIT_16;
    EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChExactMatchManagerEntryDelete: expected to GT_BAD_PARAM on value [%d]",
            entry.exactMatchUserDefined);
    entry.exactMatchUserDefined=7;
}

/*
GT_STATUS cpssDxChExactMatchManagerEntryDelete
(
    IN GT_U32                                           exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC          *entryPtr
);
*/
static void internal_cpssDxChExactMatchManagerEntryDeleteUT(void)
{
    GT_STATUS   st=GT_OK;
    GT_U32      ii;
    GT_U32      exactMatchManagerId = TESTED_EXACT_MATCH_MANAGER_ID_CNS;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC             entry;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               exactMatchEntryType;

    cpssOsMemSet(&entry ,0,sizeof(entry));

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        EM_MANAGER_ENTRY_DELETE_WITH_STATISTICS(ii,&entry,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChExactMatchManagerEntryDelete: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    /* check out of range managers ... expect 'GT_OUT_OF_RANGE' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        EM_MANAGER_ENTRY_DELETE_WITH_STATISTICS(ii,&entry,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
            "cpssDxChExactMatchManagerEntryDelete: expected to GT_OUT_OF_RANGE on manager [%d]",
            ii);
    }

    /* create manager with device */
    st = createExactMatchManagerWithDevice(exactMatchManagerId,EXACT_MATCH_SIZE_FULL,prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "createExactMatchManagerWithDevice: expected to GT_OK on manager [%d]",
            exactMatchManagerId);
    if (st == GT_OK)
    {
        /* check invalid parameters */
        for(ii = 0 ; ii < UT_EXACT_MATCH_ENTRY_TYPE_VALID_NUM; ii++)
        {
            exactMatchEntryType = exactMatchEntryType_valid_arr[ii % UT_EXACT_MATCH_ENTRY_TYPE_VALID_NUM];

            switch(exactMatchEntryType)
            {
                case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
                    PRV_UTF_LOG0_MAC("check invalid parameters : tti \n");
                    bad_param_check_exactMatchManagerEntryDelete_tti(exactMatchManagerId);
                    break;
                case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
                    PRV_UTF_LOG0_MAC("check invalid parameters : pcl \n");
                    bad_param_check_exactMatchManagerEntryDelete_pcl(exactMatchManagerId);
                    break;
                case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
                    PRV_UTF_LOG0_MAC("check invalid parameters : epcl \n");
                    bad_param_check_exactMatchManagerEntryDelete_epcl(exactMatchManagerId);
                    break;
                default:
                    break;
            }
        }
    }
    else
    {
        PRV_UTF_LOG0_MAC("Ended - Test fail , could not create manager - createExactMatchManagerWithDevice fail \n");
    }

    /* NULL pointer */
    EM_MANAGER_ENTRY_DELETE_WITH_STATISTICS(exactMatchManagerId,NULL,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChExactMatchManagerEntryDelete: expected to GT_BAD_PTR on manager [%d]",
        exactMatchManagerId);

}
/*
GT_STATUS cpssDxChExactMatchManagerEntryDelete
(
    IN GT_U32                                           exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC          *entryPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerEntryDelete)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    internal_cpssDxChExactMatchManagerEntryDeleteUT();

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}

/**
GT_STATUS cpssDxChExactMatchManagerEntryGet(
    IN GT_U32                                           exactMatchManagerId,
    INOUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC       *entryPtr
);
*/
static void internal_cpssDxChExactMatchManagerEntryGetUT(void)
{
    GT_STATUS   st=GT_OK;
    GT_U32      ii;
    GT_U32      exactMatchManagerId = TESTED_EXACT_MATCH_MANAGER_ID_CNS;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC     entry;

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        st = cpssDxChExactMatchManagerEntryGet(ii,&entry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChExactMatchManagerEntryGet: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    /* check out of range managers ... expect 'GT_OUT_OF_RANGE' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        st = cpssDxChExactMatchManagerEntryGet(ii,&entry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
            "cpssDxChExactMatchManagerEntryGet: expected to GT_OUT_OF_RANGE on manager [%d]",
            ii);
    }

    /* create manager with device */
    st = createExactMatchManagerWithDevice(exactMatchManagerId,EXACT_MATCH_SIZE_FULL,prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "createExactMatchManagerWithDevice: expected to GT_OK on manager [%d]",
        exactMatchManagerId);

    /* NULL pointer */
    st = cpssDxChExactMatchManagerEntryGet(exactMatchManagerId,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChExactMatchManagerEntryGet: expected to GT_BAD_PTR on manager [%d]",
        exactMatchManagerId);
}

/**
GT_STATUS cpssDxChExactMatchManagerEntryGet (
    IN GT_U32                                           exactMatchManagerId,
    INOUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC       *entryPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerEntryGet)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    internal_cpssDxChExactMatchManagerEntryGetUT();

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}

/**
GT_STATUS cpssDxChExactMatchManagerEntryGetNext
(
    IN  GT_U32                                          exactMatchManagerId,
    IN  GT_BOOL                                         getFirst,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC         *entryPtr
);
*/
static void internal_cpssDxChExactMatchManagerEntryGetNextUT(void)
{
    GT_STATUS   st=GT_OK;
    GT_U32      ii;
    GT_U32      exactMatchManagerId = TESTED_EXACT_MATCH_MANAGER_ID_CNS;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC              entry;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC   exactMatchEntryAddInfo;

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        st = cpssDxChExactMatchManagerEntryGetNext(ii,GT_TRUE,&entry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChExactMatchManagerEntryGetNext: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    /* check out of range managers ... expect 'GT_OUT_OF_RANGE' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        st = cpssDxChExactMatchManagerEntryGetNext(ii,GT_TRUE,&entry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
            "cpssDxChExactMatchManagerEntryGetNext: expected to GT_OUT_OF_RANGE on manager [%d]",
            ii);
    }

    /* create manager with device */
    st = createExactMatchManagerWithDevice(exactMatchManagerId,EXACT_MATCH_SIZE_FULL,prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "createExactMatchManagerWithDevice: expected to GT_OK on manager [%d]",
        exactMatchManagerId);

    if (st == GT_OK)
    {
        /* build 'valid' tti entry */

        /* set valid key parameters */
        entry.exactMatchEntry.key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
        entry.exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        entry.expandedActionIndex = 2;
        entry.exactMatchUserDefined = 2;

        /* build 'valid' entry : tti */
        entry.exactMatchActionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
        prvSetExactMatchActionDefaultValues(&entry.exactMatchAction,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        exactMatchEntryAddInfo.rehashEnable    = GT_FALSE;

        PRV_UTF_LOG0_MAC("check that info for the TTI entry is valid \n");

        /* check that info for the entry is valid */
        EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
            "cpssDxChExactMatchManagerEntryAdd: unexpected GT_OK [%d]",
            exactMatchManagerId);

        /* calling 'next' without calling 'first' (valid use according to SRS document) */
        st = cpssDxChExactMatchManagerEntryGetNext(exactMatchManagerId,GT_FALSE,&entry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChExactMatchManagerEntryGetNext: expected to GT_OK on manager [%d]",
            exactMatchManagerId);

        /* calling 'next' -- no more */
        st = cpssDxChExactMatchManagerEntryGetNext(exactMatchManagerId,GT_FALSE,&entry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, st,
            "cpssDxChExactMatchManagerEntryGetNext: expected to GT_NO_MORE on manager [%d]",
            exactMatchManagerId);

        /* calling 'first' */
        st = cpssDxChExactMatchManagerEntryGetNext(exactMatchManagerId,GT_TRUE,&entry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChExactMatchManagerEntryGetNext: expected to GT_OK on manager [%d]",
            exactMatchManagerId);

        /* calling 'next' -- no more */
        st = cpssDxChExactMatchManagerEntryGetNext(exactMatchManagerId,GT_FALSE,&entry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, st,
            "cpssDxChExactMatchManagerEntryGetNext: expected to GT_NO_MORE on manager [%d]",
            exactMatchManagerId);

        /* adding another entry */
        /* build 'valid' pcl entry */

        /* set valid key parameters */
        entry.exactMatchEntry.key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
        entry.exactMatchEntry.lookupNum=CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
        entry.expandedActionIndex = 9;
        entry.exactMatchUserDefined = 9;

        /* build 'valid' entry : pcl */
        entry.exactMatchActionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
        prvSetExactMatchActionDefaultValues(&entry.exactMatchAction,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

        exactMatchEntryAddInfo.rehashEnable    = GT_FALSE;

        PRV_UTF_LOG0_MAC("check that info for the PCL entry is valid \n");

        /* check that info for the entry is valid */
        EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,&exactMatchEntryAddInfo,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK,st,
            "cpssDxChExactMatchManagerEntryAdd: unexpected GT_OK [%d]",
            exactMatchManagerId);

         /* calling 'first' */
        st = cpssDxChExactMatchManagerEntryGetNext(exactMatchManagerId,GT_TRUE,&entry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChExactMatchManagerEntryGetNext: expected to GT_OK on manager [%d]",
            exactMatchManagerId);

        /* calling 'next' -- gt_ok */
        st = cpssDxChExactMatchManagerEntryGetNext(exactMatchManagerId,GT_FALSE,&entry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChExactMatchManagerEntryGetNext: expected to GT_OK on manager [%d]",
            exactMatchManagerId);

        /* calling 'next' -- no more */
        st = cpssDxChExactMatchManagerEntryGetNext(exactMatchManagerId,GT_FALSE,&entry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, st,
            "cpssDxChExactMatchManagerEntryGetNext: expected to GT_NO_MORE on manager [%d]",
            exactMatchManagerId);
    }

    /* NULL pointer */
    st = cpssDxChExactMatchManagerEntryGetNext(exactMatchManagerId,GT_TRUE,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChExactMatchManagerEntryGetNext: expected to GT_BAD_PTR on manager [%d]",
        exactMatchManagerId);
}

/**
GT_STATUS cpssDxChExactMatchManagerEntryGetNext (
    IN  GT_U32                                          exactMatchManagerId,
    IN  GT_BOOL                                         getFirst,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC         *entryPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerEntryGetNext)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    internal_cpssDxChExactMatchManagerEntryGetNextUT();

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}

extern GT_STATUS prvTgfExactMatchEntryIndexFind_MultiHash
(
    IN  PRV_TGF_EXACT_MATCH_ENTRY_STC *entryPtr,
    OUT GT_U32                        *indexPtr
);

/* function to check that the entry exists in the HW */
static void exactMatchManagerHwEntryCheck(
    IN CPSS_DXCH_EXACT_MATCH_ENTRY_STC  *entryPtr
)
{
    GT_STATUS   st;
    PRV_TGF_EXACT_MATCH_ENTRY_STC   tgf_entry;
    GT_U32  hwIndex;

    cpssOsMemSet(&tgf_entry ,0, sizeof(PRV_TGF_EXACT_MATCH_ENTRY_STC));

    prvTgfConvertDxChExactMatchEntryToGenericExactMatchEntry(entryPtr,&tgf_entry);

    st = prvTgfExactMatchEntryIndexFind_MultiHash(&tgf_entry,&hwIndex);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,"prvTgfExactMatchEntryIndexFind_MultiHash keySize=[%d], lookupNum[%d]",
        entryPtr->key.keySize,entryPtr->lookupNum);
}

/**
GT_STATUS cpssDxChExactMatchManagerEntryAdd
(
    IN GT_U32                                               exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC   *paramsPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerEntryAddCuckooCapacityIncremental)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    check_hw_hold_entry = GT_TRUE;/* very slow mode ! */

    internal_cpssDxChExactMatchManagerCuckooEntryAddUT(GT_FALSE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E);

    CLEANUP_ALL_MANAGERS;

    internal_cpssDxChExactMatchManagerCuckooEntryAddUT(GT_FALSE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E);

    CLEANUP_ALL_MANAGERS;

    internal_cpssDxChExactMatchManagerCuckooEntryAddUT(GT_FALSE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E);

    CLEANUP_ALL_MANAGERS;

    internal_cpssDxChExactMatchManagerCuckooEntryAddUT(GT_FALSE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E);

    CLEANUP_ALL_MANAGERS;

    internal_cpssDxChExactMatchManagerCuckooEntryAddUT(GT_FALSE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E); /* random key */

    CLEANUP_ALL_MANAGERS;

    check_hw_hold_entry = GT_FALSE;

    RESTORE_OS_MALLOC_AND_OS_FREE;

}

UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerEntryAddCuckooCapacityRandom)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    check_hw_hold_entry = GT_TRUE;/* very slow mode ! */

    internal_cpssDxChExactMatchManagerCuckooEntryAddUT(GT_TRUE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E);

    CLEANUP_ALL_MANAGERS;

    internal_cpssDxChExactMatchManagerCuckooEntryAddUT(GT_TRUE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E);

    CLEANUP_ALL_MANAGERS;

    internal_cpssDxChExactMatchManagerCuckooEntryAddUT(GT_TRUE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E);

    CLEANUP_ALL_MANAGERS;

    internal_cpssDxChExactMatchManagerCuckooEntryAddUT(GT_TRUE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E);

    CLEANUP_ALL_MANAGERS;

    internal_cpssDxChExactMatchManagerCuckooEntryAddUT(GT_TRUE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E); /* random key */

    CLEANUP_ALL_MANAGERS;

    check_hw_hold_entry = GT_FALSE;

    RESTORE_OS_MALLOC_AND_OS_FREE;

}
/* test to check bug CPSS-12426: Cuckoo does not change EMM index for second 33 byte entry after adding 19 byte entry.
   Description: in Hash of 4 banks when key size is 33/47 there is only one option to add the enrty.
                If this location is occupied need to try to move all the other entries in this location.
   The bug: The check for depth is before we try to move all the other entries.                                                                                                                                         .
   Test: In the test we create keys with the same pattern with different size (padding with zeros).
        description below is for hash of 4 banks; in hash bigger then 4 all entries should be added.
        1. add entry size 5B (4 free hash options)
        2. add entry size 19B (1 free hash option)
        3. add entry size 33B (no free hash options, need to move entry 5B)
        4. add entry size 47B (no free hash options, can't add this entry).
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerEntryAddCuckooSameHash)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    check_hw_hold_entry = GT_TRUE;/* very slow mode ! */

    internal_cpssDxChExactMatchManagerCuckooEntryAddSingleUT(CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E);
    internal_cpssDxChExactMatchManagerCuckooEntryAddSingleUT(CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E);
    internal_cpssDxChExactMatchManagerCuckooEntryAddSingleUT(CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E);
    internal_cpssDxChExactMatchManagerCuckooEntryAddSingleUT(CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E);

    CLEANUP_ALL_MANAGERS;

    check_hw_hold_entry = GT_FALSE;

    RESTORE_OS_MALLOC_AND_OS_FREE;

}

/* check bounders of numeric field , and high values (31,32 bits value)  :
expected to fail */
static void FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchExpandedActionUpdate
(
    IN GT_U32                                                               exactMatchManagerId,
    IN GT_U32                                                               expandedActionIndex,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC         *paramsPtr,
    IN GT_U32                                                               *fieldPtr,
    IN GT_U32                                                               testedField_Value
)
{
    GT_STATUS   st=GT_OK;
    GT_U32      origValue = *fieldPtr;

    PRV_UTF_LOG0_MAC("check field with invalid field value\n");

    *fieldPtr = testedField_Value;
    EM_MANAGER_EXPANDED_ACTION_ENTRY_UPDATE(exactMatchManagerId, expandedActionIndex, paramsPtr, st);
    /* restore orig value */
    *fieldPtr = origValue;
    if (st != GT_OUT_OF_RANGE)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChExactMatchManagerExpandedActionUpdate: [%d] expected to GT_BAD_PARAM on value [%d]",
            *fieldPtr,
            testedField_Value);
    }
}
/**
GT_STATUS cpssDxChExactMatchManagerExpandedActionUpdate
(
    IN  GT_U32                                                          exactMatchManagerId,
    IN  GT_U32                                                          expandedActionIndex,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC    *paramsPtr
);
*/
static void internal_cpssDxChExactMatchManagerExpandedActionUpdateUT(void)
{
    GT_STATUS   st=GT_OK;
    GT_U32      ii;
    GT_U32      exactMatchManagerId = TESTED_EXACT_MATCH_MANAGER_ID_CNS;
    GT_U32      maxTotalEntries = EXACT_MATCH_SIZE_FULL;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      testResultArray[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNum=0, errorNumItr=0;
    GT_U32      expandedActionIndex=0;
    CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC    expandedActionParams;
    CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC    expandedActionParamsGet;
    CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC    expandedActionParamsGet2;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT                expandedActionOriginData,expandedActionOriginDataGet;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                                actionData,actionDataGet;


    cpssOsMemSet(&actionData ,0,sizeof(actionData));
    cpssOsMemSet(&actionDataGet ,0,sizeof(actionDataGet));
    cpssOsMemSet(&expandedActionParams ,0,sizeof(expandedActionParams));
    cpssOsMemSet(&expandedActionParamsGet ,0,sizeof(expandedActionParamsGet));
    cpssOsMemSet(&expandedActionParamsGet2 ,0,sizeof(expandedActionParamsGet2));
    cpssOsMemSet(&expandedActionOriginData ,0,sizeof(expandedActionOriginData));
    cpssOsMemSet(&expandedActionOriginDataGet ,0,sizeof(expandedActionOriginDataGet));
    cpssOsMemSet(testResultArray,0,sizeof(testResultArray));

    PRV_UTF_LOG0_MAC("Check non-initialized managers \n");

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        EM_MANAGER_EXPANDED_ACTION_ENTRY_UPDATE(ii,expandedActionIndex, &expandedActionParams,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChExactMatchManagerExpandedActionUpdate: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    PRV_UTF_LOG0_MAC("Check check out of range managers \n");
    /* check out of range managers ... expect 'GT_OUT_OF_RANGE' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        EM_MANAGER_EXPANDED_ACTION_ENTRY_UPDATE(ii,expandedActionIndex, &expandedActionParams,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
            "cpssDxChExactMatchManagerExpandedActionUpdate: expected to GT_OUT_OF_RANGE on manager [%d]",
            ii);
    }

    if(run_device_less == GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("create manager with device \n");
    }
    else
    {
        PRV_UTF_LOG0_MAC("create manager without device \n");
    }

    /* create manager with device */
    st = createExactMatchManagerWithDevice(exactMatchManagerId,maxTotalEntries,prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "createExactMatchManagerWithDevice: expected to GT_OK on manager [%d]",
                exactMatchManagerId);
    if (st==GT_OK)
    {
        PRV_UTF_LOG0_MAC("start - update the expander table\n");

        /* check valid parameters
           half array is configured in the create with TTI configuration and half with PCL configuration*/
        cpssOsBzero((GT_VOID*) &actionData, sizeof(actionData));
        cpssOsBzero((GT_VOID*) &actionDataGet, sizeof(actionDataGet));

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionActionStop=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionBridgeBypass=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionCommand=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionFlowId=GT_TRUE;
        expandedActionOriginData.ttiExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E);

        /* configure half array with TTI configuration and half with PCL configuration*/
        expandedActionParams.exactMatchExpandedEntryValid=GT_TRUE;
        expandedActionParams.expandedActionType=CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
        cpssOsMemCpy(&expandedActionParams.expandedAction,&actionData,sizeof(actionData));
        cpssOsMemCpy(&expandedActionParams.expandedActionOrigin.ttiExpandedActionOrigin,&expandedActionOriginData,sizeof(expandedActionOriginData));

        for (expandedActionIndex=0;expandedActionIndex<CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS/2;expandedActionIndex++)
        {
            /* change one of the fields in the action that have an overwrite value = GT_TRUE*/
            /* get the old param configured and change it  */
            EM_MANAGER_EXPANDED_ACTION_ENTRY_GET(exactMatchManagerId,expandedActionIndex, &expandedActionParamsGet,st);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                        "cpssDxChExactMatchManagerExpandedActionGet: expected to GT_OK on manager [%d]",
                                        exactMatchManagerId);
            if (expandedActionParamsGet.exactMatchExpandedEntryValid==GT_TRUE)
            {
                /* change one of the fields in the action that have an overwrite value = GT_TRUE*/
                if(expandedActionParamsGet.expandedAction.ttiAction.actionStop==GT_TRUE)
                    expandedActionParams.expandedAction.ttiAction.actionStop = GT_FALSE;
                else
                    expandedActionParams.expandedAction.ttiAction.actionStop=GT_TRUE;

                EM_MANAGER_EXPANDED_ACTION_ENTRY_UPDATE(exactMatchManagerId,expandedActionIndex, &expandedActionParams,st);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                            "cpssDxChExactMatchManagerExpandedActionUpdate: expected to GT_BAD_PARAM on manager [%d]",
                                            exactMatchManagerId);

                if (st == GT_OK)
                {
                    EM_MANAGER_EXPANDED_ACTION_ENTRY_GET(exactMatchManagerId, expandedActionIndex, &expandedActionParamsGet2, st);
                     /* check value was updated */
                    if (expandedActionParamsGet.expandedAction.ttiAction.actionStop==expandedActionParamsGet2.expandedAction.ttiAction.actionStop)
                    {
                        st = GT_BAD_VALUE;
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                                    "cpssDxChExactMatchManagerExpandedActionGet: expected to GT_BAD_VALUE on manager [%d]",
                                                    exactMatchManagerId);
                    }
                }
            }
            else
            {
                 st = GT_BAD_VALUE;
                /* error - entries set at create of manager shoould be valid */
                 UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                            "cpssDxChExactMatchManagerExpandedActionGet: expected valid entry on manager [%d], expandedActionIndex[%d]",
                                            exactMatchManagerId,expandedActionIndex);
            }
        }

        cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
        cpssOsMemSet(&expandedActionOriginDataGet,0, sizeof(expandedActionOriginDataGet));

        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup=GT_TRUE;
        expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;

        prvSetExactMatchActionDefaultValues(&actionData,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E);

        /* configure half array with TTI configuration and half with PCL configuration*/
        expandedActionParams.exactMatchExpandedEntryValid=GT_TRUE;
        expandedActionParams.expandedActionType=CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
        cpssOsMemCpy(&expandedActionParams.expandedAction,&actionData,sizeof(actionData));
        cpssOsMemCpy(&expandedActionParams.expandedActionOrigin.pclExpandedActionOrigin,&expandedActionOriginData,sizeof(expandedActionOriginData));

        for (;expandedActionIndex<CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS;expandedActionIndex++)
        {
            /* change one of the fields in the action that have an overwrite value = GT_TRUE*/
            /* get the old param configured and change it  */
            EM_MANAGER_EXPANDED_ACTION_ENTRY_GET(exactMatchManagerId,expandedActionIndex, &expandedActionParamsGet,st);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                        "cpssDxChExactMatchManagerExpandedActionGet: expected to GT_OK on manager [%d]",
                                        exactMatchManagerId);
            if (expandedActionParamsGet.exactMatchExpandedEntryValid==GT_TRUE)
            {
                /* change one of the fields in the action that have an overwrite value = GT_TRUE*/
                if(expandedActionParamsGet.expandedAction.pclAction.actionStop==GT_TRUE)
                    expandedActionParams.expandedAction.pclAction.actionStop = GT_FALSE;
                else
                    expandedActionParams.expandedAction.pclAction.actionStop=GT_TRUE;

                EM_MANAGER_EXPANDED_ACTION_ENTRY_UPDATE(exactMatchManagerId,expandedActionIndex, &expandedActionParams,st);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                            "cpssDxChExactMatchManagerExpandedActionUpdate: expected to GT_BAD_PARAM on manager [%d]",
                                            exactMatchManagerId);

                if (st == GT_OK)
                {
                    EM_MANAGER_EXPANDED_ACTION_ENTRY_GET(exactMatchManagerId, expandedActionIndex, &expandedActionParamsGet2, st);
                     /* check value was updated */
                    if (expandedActionParamsGet.expandedAction.pclAction.actionStop==expandedActionParamsGet2.expandedAction.pclAction.actionStop)
                    {
                        st = GT_BAD_VALUE;
                        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                                    "cpssDxChExactMatchManagerExpandedActionGet: expected to GT_BAD_VALUE on manager [%d]",
                                                    exactMatchManagerId);
                    }
                }
            }
            else
            {
                 st = GT_BAD_VALUE;
                /* error - entries set at create of manager shoould be valid */
                 UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                            "cpssDxChExactMatchManagerExpandedActionGet: expected valid entry on manager [%d], expandedActionIndex[%d]",
                                            exactMatchManagerId,expandedActionIndex);
            }
        }
    }
    else
    {
        PRV_UTF_LOG0_MAC("Ended - Test fail , could not create manager - createExactMatchManagerWithDevice fail \n");
    }

    /* Verify DB */
    cpssOsMemSet(testResultArray,0,sizeof(testResultArray));
    errorNum=0;
    st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                errorNumItr,
                testResultArray[errorNumItr],
                debugErrorArr[testResultArray[errorNumItr]].errString);
    }

    PRV_UTF_LOG0_MAC("check invalid parameters \n");
    /* check invalid parameters */
    expandedActionIndex=4;/* valid 0-15 */
    FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChExactMatchExpandedActionUpdate(exactMatchManagerId,
                                                                             expandedActionIndex,
                                                                             &expandedActionParams,
                                                                             (GT_U32 *)&expandedActionParams.expandedActionType,
                                                                             CPSS_DXCH_EXACT_MATCH_ACTION_LAST_E);
    expandedActionIndex=16;
    EM_MANAGER_EXPANDED_ACTION_ENTRY_UPDATE(exactMatchManagerId,expandedActionIndex,&expandedActionParams,st);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st,
        "cpssDxChExactMatchManagerExpandedActionUpdate: expected to GT_BAD_PARAM on manager [%d] and expandedActionIndex [%d]",
        exactMatchManagerId,expandedActionIndex);

    PRV_UTF_LOG0_MAC("check NULL pointers \n");
    /* NULL pointer */
    EM_MANAGER_EXPANDED_ACTION_ENTRY_UPDATE(exactMatchManagerId,expandedActionIndex,NULL,st);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChExactMatchManagerExpandedActionUpdate: expected to GT_BAD_PTR on manager [%d]",
        exactMatchManagerId);

    /* Verify DB */
    st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
    for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
    {
        PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                errorNumItr,
                testResultArray[errorNumItr],
                debugErrorArr[testResultArray[errorNumItr]].errString);
    }

    PRV_UTF_LOG0_MAC("Ended \n");
    return;
}
/**
GT_STATUS cpssDxChExactMatchManagerExpandedActionUpdate (
    IN  GT_U32                                                          exactMatchManagerId,
    IN  GT_U32                                                          expandedActionIndex,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC    *paramsPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerExpandedActionUpdate)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    check_hw_hold_entry = GT_TRUE;/* very slow mode ! */

    internal_cpssDxChExactMatchManagerExpandedActionUpdateUT();

    check_hw_hold_entry = GT_FALSE;

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}

/**
GT_STATUS cpssDxChExactMatchManagerExpandedActionGet(
    IN  GT_U32                                                          exactMatchManagerId,
    IN  GT_U32                                                          expandedActionIndex,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC    *paramsPtr
);
*/
static void internal_cpssDxChExactMatchManagerExpandedActionGetUT(void)
{
    GT_STATUS   st=GT_OK;
    GT_U32      ii;
    GT_U32      exactMatchManagerId = TESTED_EXACT_MATCH_MANAGER_ID_CNS;
    GT_U32      expandedActionIndex=0;
     CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC    expandedActionParams;

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        st = cpssDxChExactMatchManagerExpandedActionGet(ii,expandedActionIndex,&expandedActionParams);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChExactMatchManagerExpandedActionGet: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    /* check out of range managers ... expect 'GT_OUT_OF_RANGE' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        st = cpssDxChExactMatchManagerExpandedActionGet(ii,expandedActionIndex,&expandedActionParams);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
            "cpssDxChExactMatchManagerExpandedActionGet: expected to GT_OUT_OF_RANGE on manager [%d]",
            ii);
    }

    /* create manager with device */
    st = createExactMatchManagerWithDevice(exactMatchManagerId,EXACT_MATCH_SIZE_FULL,prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
        "createExactMatchManagerWithDevice: expected to GT_OK on manager [%d]",
        exactMatchManagerId);

    /* invalid index */
    expandedActionIndex=16;
    st = cpssDxChExactMatchManagerExpandedActionGet(exactMatchManagerId,expandedActionIndex,&expandedActionParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
        "cpssDxChExactMatchManagerExpandedActionGet: expected to GT_BAD_PARAM on manager [%d]",
        exactMatchManagerId);

    /* NULL pointer */
    st = cpssDxChExactMatchManagerExpandedActionGet(exactMatchManagerId,expandedActionIndex,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
        "cpssDxChExactMatchManagerExpandedActionGet: expected to GT_BAD_PTR on manager [%d]",
        exactMatchManagerId);
}

/**
GT_STATUS cpssDxChExactMatchManagerExpandedActionGet(
    IN  GT_U32                                                          exactMatchManagerId,
    IN  GT_U32                                                          expandedActionIndex,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC    *paramsPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerExpandedActionGet)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    internal_cpssDxChExactMatchManagerExpandedActionGetUT();

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}


static void exactMatchManagerEntry_incrementDbCounter(
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC      *lookupInfoPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC       *entryPtr,
    IN GT_BOOL                                       entryAdded,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC   *countersPtr
)
{
#define EM_UPDATE_COUNTER_VALUE(val) \
    val = (entryAdded) ? val + 1 : val - 1

    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT   clientType;
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT keySize;

    keySize = entryPtr->exactMatchEntry.key.keySize;
    clientType = lookupInfoPtr->lookupsArray[entryPtr->exactMatchEntry.lookupNum].lookupClient;

    switch (clientType)
    {
    case CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E:
        if (entryPtr->exactMatchActionType!=CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E)
        {
            /* illegal configuration */
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_STATE,"exactMatchManagerEntry_incrementDbCounter ");
        }
        EM_UPDATE_COUNTER_VALUE(countersPtr->ttiClientKeySizeEntriesArray[keySize]);
        break;
    case CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E:
        if (entryPtr->exactMatchActionType!=CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E)
        {
            /* illegal configuration */
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_STATE,"exactMatchManagerEntry_incrementDbCounter ");
        }
        EM_UPDATE_COUNTER_VALUE(countersPtr->ipcl0ClientKeySizeEntriesArray[keySize]);
        break;
    case CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_1_E:
        if (entryPtr->exactMatchActionType!=CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E)
        {
            /* illegal configuration */
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_STATE,"exactMatchManagerEntry_incrementDbCounter ");
        }
        EM_UPDATE_COUNTER_VALUE(countersPtr->ipcl1ClientKeySizeEntriesArray[keySize]);
        break;
    case CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_2_E:
        if (entryPtr->exactMatchActionType!=CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E)
        {
            /* illegal configuration */
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_STATE,"exactMatchManagerEntry_incrementDbCounter ");
        }
        EM_UPDATE_COUNTER_VALUE(countersPtr->ipcl2ClientKeySizeEntriesArray[keySize]);
        break;
    case CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E:
        if (entryPtr->exactMatchActionType!=CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E)
        {
            /* illegal configuration */
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_STATE,"exactMatchManagerEntry_incrementDbCounter");
        }
       EM_UPDATE_COUNTER_VALUE(countersPtr->epclClientKeySizeEntriesArray[keySize]);
        break;
    default:
        /* illegal configuration */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_BAD_STATE,"exactMatchManagerEntry_incrementDbCounter");
    }

   /* each entry hold diferrent number of indexes
        5  Bytes key size = 1 index
        19 Bytes key size = 2 index
        33 Bytes key size = 3 index
        47 Bytes key size = 4 index */
    if (entryAdded)
    {
        countersPtr->usedEntriesIndexes += (keySize+1);
        countersPtr->freeEntriesIndexes -= (keySize+1);
    }
    else
    {
        countersPtr->usedEntriesIndexes -= (keySize+1);
        countersPtr->freeEntriesIndexes += (keySize+1);
    }
}

/**
GT_STATUS cpssDxChExactMatchManagerCountersGet
(
    IN  GT_U32                                      exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC  *countersPtr
);
*/
static void internal_cpssDxChExactMatchManagerCountersGetUT(void)
{
    GT_STATUS   st=GT_OK;
    GT_U32      ii, bankId;
    GT_U32      numEntriesRemoved;/* total number of entries removed from the DB */
    GT_U32      numEntriesAdded, banksTotalEntries;  /* total number of entries added to the DB */
    GT_U32      numIndexesAdded;/* number of entries added to each bank;
                                   if one entry is added with keysize=47 then:
                                   numEntriesAdded=1 and numIndexesAdded=4 */
    GT_BOOL     entryAdded;/* indication that the entry was added to the DB */
    GT_BOOL     getFirst;  /* indication to get first entry from the DB */
    GT_U32      exactMatchManagerId = TESTED_EXACT_MATCH_MANAGER_ID_CNS;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                 entry;
    GT_U32                                                  maxTotalEntries;
    CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC              counters;
    CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC              countersGet;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT                   exactMatchEntryType;
    UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC    outputResults;

    CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC           capacityInfoGet;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC   entryAttrInfoGet;
    CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC             lookupInfoGet;
    CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC              agingInfoGet;

    cpssOsMemSet(&entry , 0, sizeof(entry));
    cpssOsMemSet(&counters, 0, sizeof(counters));
    cpssOsMemSet(&countersGet, 0, sizeof(countersGet));

    outputResults.entryPtr          = &entry;
    outputResults.entryAddedPtr     = &entryAdded;

    PRV_UTF_LOG0_MAC("Check non-initialized managers \n");

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        st = cpssDxChExactMatchManagerCountersGet(ii, &countersGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChExactMatchManagerCountersGet: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    PRV_UTF_LOG0_MAC("Check out of range managers \n");
    /* check out of range managers ... expect 'GT_OUT_OF_RANGE' */
    for(/*continue*/; ii < 100 ; ii+= 7)
    {
        st = cpssDxChExactMatchManagerCountersGet(ii, &countersGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
            "cpssDxChExactMatchManagerCountersGet: expected to GT_OUT_OF_RANGE on manager [%d]",
            ii);
    }

    numEntriesAdded = 0;
    numIndexesAdded = 0;
    maxTotalEntries = EXACT_MATCH_SIZE_FULL;

    if(em_debug_limited_entries)
    {
        maxTotalEntries = em_debug_limited_entries;
    }

    counters.freeEntriesIndexes = maxTotalEntries;

    if(run_device_less == GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("create manager with device \n");
    }
    else
    {
        PRV_UTF_LOG0_MAC("create manager without device \n");
    }

    /* create manager with device */
    st = createExactMatchManagerWithDevice(exactMatchManagerId,maxTotalEntries,prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "createExactMatchManagerWithDevice: expected to GT_OK on manager [%d]",
                exactMatchManagerId);

    if (st==GT_OK)
    {

        /* Retrieve data */
        st = cpssDxChExactMatchManagerConfigGet(exactMatchManagerId,
             &capacityInfoGet,&lookupInfoGet,&entryAttrInfoGet,&agingInfoGet);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChExactMatchManagerConfigGet ");


        PRV_UTF_LOG1_MAC("start - add [%d] entries \n",maxTotalEntries);
        for(ii = 0 ; ii < maxTotalEntries;/*update according to entry size*/)
        {
            entryAdded = GT_FALSE;

            exactMatchEntryType = exactMatchEntryType_valid_arr[ii % UT_EXACT_MATCH_ENTRY_TYPE_VALID_NUM];
            switch(exactMatchEntryType)
            {
                case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
                    exactMatchManagerEntryAdd_tti(exactMatchManagerId, ii, &outputResults);
                    break;
                case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
                    exactMatchManagerEntryAdd_pcl(exactMatchManagerId, ii, &outputResults);
                    break;
                case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
                    exactMatchManagerEntryAdd_epcl(exactMatchManagerId, ii, &outputResults);
                    break;
                default:
                    break;
            }
            if(entryAdded)
            {
                numEntriesAdded++;
                numIndexesAdded+=(outputResults.entryPtr->exactMatchEntry.key.keySize+1);
                /* increment counters */
                exactMatchManagerEntry_incrementDbCounter(&lookupInfoGet,&entry, GT_TRUE, &counters);
            }

            if(add_device_in_runtime == GT_TRUE &&
               (ii == (maxTotalEntries/2)))
            {
                CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[1];
                GT_U32                                                   numOfPairs=1;

                /* now in the middle of the insertions we add the device. */
                PRV_UTF_LOG0_MAC("now in the middle of the insertions we add the device \n");

                pairListArr[0].devNum = prvTgfDevNum;
                pairListArr[0].portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

                /* add device to it */
                st = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChExactMatchManagerDevListAdd: expected to GT_OK on manager [%d]",
                    exactMatchManagerId);
            }
            if(entryAdded)
            {
                ii+=(outputResults.entryPtr->exactMatchEntry.key.keySize+1);
            }
            else
            {
                ii++;
            }
        }

        /* get counters */
        st = cpssDxChExactMatchManagerCountersGet(exactMatchManagerId, &countersGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChExactMatchManagerCountersGet: expected to GT_OK on manager [%d]",
            ii);

        UTF_VERIFY_EQUAL1_STRING_MAC(0,cpssOsMemCmp(&counters.ttiClientKeySizeEntriesArray,
                                                    &countersGet.ttiClientKeySizeEntriesArray,
                                                    sizeof(counters.ttiClientKeySizeEntriesArray)),
            "cpssDxChExactMatchManagerCountersGet: ttiClientKeySizeEntriesArray : "
            "expected 'get' entry to match 'set' entry in manager [%d]",
            exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(0,cpssOsMemCmp(&counters.ipcl0ClientKeySizeEntriesArray,
                                                &countersGet.ipcl0ClientKeySizeEntriesArray,
                                                sizeof(counters.ipcl0ClientKeySizeEntriesArray)),
        "cpssDxChExactMatchManagerCountersGet: ipcl0ClientKeySizeEntriesArray : "
        "expected 'get' entry to match 'set' entry in manager [%d]",
        exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(0,cpssOsMemCmp(&counters.ipcl1ClientKeySizeEntriesArray,
                                                    &countersGet.ipcl1ClientKeySizeEntriesArray,
                                                    sizeof(counters.ipcl1ClientKeySizeEntriesArray)),
            "cpssDxChExactMatchManagerCountersGet: ipcl1ClientKeySizeEntriesArray : "
            "expected 'get' entry to match 'set' entry in manager [%d]",
            exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(0,cpssOsMemCmp(&counters.ipcl2ClientKeySizeEntriesArray,
                                                &countersGet.ipcl2ClientKeySizeEntriesArray,
                                                sizeof(counters.ipcl2ClientKeySizeEntriesArray)),
        "cpssDxChExactMatchManagerCountersGet: ipcl2ClientKeySizeEntriesArray : "
        "expected 'get' entry to match 'set' entry in manager [%d]",
        exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(0,cpssOsMemCmp(&counters.epclClientKeySizeEntriesArray,
                                                &countersGet.epclClientKeySizeEntriesArray,
                                                sizeof(counters.epclClientKeySizeEntriesArray)),
        "cpssDxChExactMatchManagerCountersGet: epclClientKeySizeEntriesArray : "
        "expected 'get' entry to match 'set' entry in manager [%d]",
        exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(0,cpssOsMemCmp(&counters.epclClientKeySizeEntriesArray,
                                                &countersGet.epclClientKeySizeEntriesArray,
                                                sizeof(counters.epclClientKeySizeEntriesArray)),
        "cpssDxChExactMatchManagerCountersGet: epclClientKeySizeEntriesArray : "
        "expected 'get' entry to match 'set' entry in manager [%d]",
        exactMatchManagerId);

        if(counters.usedEntriesIndexes != countersGet.usedEntriesIndexes)
        {
             UTF_VERIFY_EQUAL1_STRING_MAC(0,1,"cpssDxChExactMatchManagerCountersGet: usedEntriesIndexes : "
                                            "expected 'get' entry to match 'set' entry in manager [%d]",
                                            exactMatchManagerId);
        }
        if(counters.freeEntriesIndexes != countersGet.freeEntriesIndexes)
        {
             UTF_VERIFY_EQUAL1_STRING_MAC(0,1,"cpssDxChExactMatchManagerCountersGet: freeEntriesIndexes : "
                                            "expected 'get' entry to match 'set' entry in manager [%d]",
                                            exactMatchManagerId);
        }

        banksTotalEntries = 0;
        for (bankId = 0; bankId < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_NUM_BANKS_CNS; bankId++)
        {
            banksTotalEntries += countersGet.bankCounters[bankId];
        }

        UTF_VERIFY_EQUAL2_STRING_MAC(numIndexesAdded, banksTotalEntries,
            "cpssDxChExactMatchManagerCountersGet: expected equal number of added indexes[%d] banks entries[%d]",
            numIndexesAdded, banksTotalEntries);

        PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
            maxTotalEntries,numIndexesAdded,
                (100*numIndexesAdded)/maxTotalEntries);

        if(em_debug_limited_entries)
        {
            cpssDxChExactMatchRulesDump(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,0,(EXACT_MATCH_SIZE_FULL-1));
        }

        numEntriesRemoved = 0;
        getFirst = GT_TRUE;

        PRV_UTF_LOG1_MAC("start - remove the [%d] entries \n",numEntriesAdded);
        if(check_hw_hold_entry == GT_TRUE)
        {
            PRV_UTF_LOG0_MAC("(slow mode) for each entry check that HW hold the entry in proper index according to it's index \n");
        }

        ii = 0;

        /* delete all entries from the DB (one by one) */
        while(GT_OK == cpssDxChExactMatchManagerEntryGetNext(exactMatchManagerId,getFirst,&entry))
        {
            getFirst = GT_FALSE;

            if(check_hw_hold_entry == GT_TRUE)
            {
                /* just before we going to remove the entry ... lets check that it exists in the HW ! */
                exactMatchManagerHwEntryCheck(&entry.exactMatchEntry);
            }

            EM_MANAGER_ENTRY_DELETE_WITH_STATISTICS(exactMatchManagerId, &entry, st);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChExactMatchManagerEntryDelete: expected to GT_OK on manager [%d]",
                exactMatchManagerId);

            numEntriesRemoved++;
            /* decrement counters */
            exactMatchManagerEntry_incrementDbCounter(&lookupInfoGet, &entry, GT_FALSE, &counters);


            if((numEntriesRemoved % 1000) == 0)
            {
                PRV_UTF_LOG1_MAC("removed [%d] entries \n",numEntriesRemoved);
            }
            ii++;
        }

        /* get counters */
        st = cpssDxChExactMatchManagerCountersGet(exactMatchManagerId, &countersGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChExactMatchManagerCountersGet: expected to GT_OK on manager [%d]",
            ii);

        UTF_VERIFY_EQUAL1_STRING_MAC(0,cpssOsMemCmp(&counters.ttiClientKeySizeEntriesArray,
                                                    &countersGet.ttiClientKeySizeEntriesArray,
                                                    sizeof(counters.ttiClientKeySizeEntriesArray)),
            "cpssDxChExactMatchManagerCountersGet: ttiClientKeySizeEntriesArray : "
            "expected 'get' entry to match 'set' entry in manager [%d]",
            exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(0,cpssOsMemCmp(&counters.ipcl0ClientKeySizeEntriesArray,
                                                &countersGet.ipcl0ClientKeySizeEntriesArray,
                                                sizeof(counters.ipcl0ClientKeySizeEntriesArray)),
        "cpssDxChExactMatchManagerCountersGet: ipcl0ClientKeySizeEntriesArray : "
        "expected 'get' entry to match 'set' entry in manager [%d]",
        exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(0,cpssOsMemCmp(&counters.ipcl1ClientKeySizeEntriesArray,
                                                    &countersGet.ipcl1ClientKeySizeEntriesArray,
                                                    sizeof(counters.ipcl1ClientKeySizeEntriesArray)),
            "cpssDxChExactMatchManagerCountersGet: ipcl1ClientKeySizeEntriesArray : "
            "expected 'get' entry to match 'set' entry in manager [%d]",
            exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(0,cpssOsMemCmp(&counters.ipcl2ClientKeySizeEntriesArray,
                                                &countersGet.ipcl2ClientKeySizeEntriesArray,
                                                sizeof(counters.ipcl2ClientKeySizeEntriesArray)),
        "cpssDxChExactMatchManagerCountersGet: ipcl2ClientKeySizeEntriesArray : "
        "expected 'get' entry to match 'set' entry in manager [%d]",
        exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(0,cpssOsMemCmp(&counters.epclClientKeySizeEntriesArray,
                                                &countersGet.epclClientKeySizeEntriesArray,
                                                sizeof(counters.epclClientKeySizeEntriesArray)),
        "cpssDxChExactMatchManagerCountersGet: epclClientKeySizeEntriesArray : "
        "expected 'get' entry to match 'set' entry in manager [%d]",
        exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(0,cpssOsMemCmp(&counters.epclClientKeySizeEntriesArray,
                                                &countersGet.epclClientKeySizeEntriesArray,
                                                sizeof(counters.epclClientKeySizeEntriesArray)),
        "cpssDxChExactMatchManagerCountersGet: epclClientKeySizeEntriesArray : "
        "expected 'get' entry to match 'set' entry in manager [%d]",
        exactMatchManagerId);

        if(counters.usedEntriesIndexes != countersGet.usedEntriesIndexes)
        {
             UTF_VERIFY_EQUAL1_STRING_MAC(0,1,"cpssDxChExactMatchManagerCountersGet: usedEntriesIndexes : "
                                            "expected 'get' entry to match 'set' entry in manager [%d]",
                                            exactMatchManagerId);
        }
        if(counters.freeEntriesIndexes != countersGet.freeEntriesIndexes)
        {
             UTF_VERIFY_EQUAL1_STRING_MAC(0,1,"cpssDxChExactMatchManagerCountersGet: freeEntriesIndexes : "
                                            "expected 'get' entry to match 'set' entry in manager [%d]",
                                            exactMatchManagerId);
        }

        banksTotalEntries = 0;
        for (bankId = 0; bankId < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_NUM_BANKS_CNS; bankId++)
        {
            banksTotalEntries +=countersGet.bankCounters[bankId];
        }

        UTF_VERIFY_EQUAL1_STRING_MAC(0, banksTotalEntries,
            "cpssDxChExactMatchManagerCountersGet: not expected entries[%d] in banks after delete",
            banksTotalEntries);

        PRV_UTF_LOG2_MAC("ended - removed [%d] entries (out of expected [%d]) \n",numEntriesRemoved, numEntriesAdded);

        UTF_VERIFY_EQUAL2_STRING_MAC(numEntriesAdded, numEntriesRemoved,
            "expected number of removed entries [%d] to be the same as the added entries[%d]",
            numEntriesRemoved,numEntriesAdded);


        PRV_UTF_LOG0_MAC("check NULL pointers \n");
        /* NULL pointer */
        st = cpssDxChExactMatchManagerCountersGet(exactMatchManagerId, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
            "cpssDxChExactMatchManagerCountersGet: expected to GT_BAD_PTR on manager [%d]",
            exactMatchManagerId);


        PRV_UTF_LOG0_MAC("Ended \n");
    }
    else
    {
        PRV_UTF_LOG0_MAC("Ended - Test fail , could not create manager - createExactMatchManagerWithDevice fail \n");
    }
    return;
}
/**
GT_STATUS cpssDxChExactMatchManagerCountersGet
(
    IN  GT_U32                                      exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC  *countersPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerCountersGet)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    check_hw_hold_entry = GT_TRUE;/* very slow mode ! */

    internal_cpssDxChExactMatchManagerCountersGetUT();

    check_hw_hold_entry = GT_FALSE;

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}

/**
GT_STATUS cpssDxChExactMatchManagerStatisticsGet
(
    IN GT_U32                                           exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC    *statisticsPtr
);
*/
static void internal_cpssDxChExactMatchManagerStatisticsGetUT(void)
{
    GT_STATUS   st=GT_OK;
    GT_U32      ii;
    GT_U32      numEntriesRemoved;/* total number of entries removed from the DB */
    GT_U32      numEntriesAdded;  /* total number of entries added to the DB */
    GT_U32      numIndexesAdded;  /* number of entries added to each bank;
                                     if one entry is added with keysize=47 then:
                                     numEntriesAdded=1 and numIndexesAdded=4 */
    GT_BOOL     entryAdded;/* indication that the entry was added to the DB */
    GT_BOOL     getFirst;  /* indication to get first entry from the DB */
    GT_U32      exactMatchManagerId = TESTED_EXACT_MATCH_MANAGER_ID_CNS;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                 entry;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC      params;
    GT_U32                                                  maxTotalEntries;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT                   exactMatchEntryType;
    CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC            statisticsGet;
    UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC    outputResults;

    cpssOsMemSet(&entry ,0,sizeof(entry));
    cpssOsMemSet(&statistics_global,0,sizeof(statistics_global));
    cpssOsMemSet(&statisticsGet,0,sizeof(statisticsGet));

    outputResults.entryPtr          = &entry;
    outputResults.entryAddedPtr     = &entryAdded;

    PRV_UTF_LOG0_MAC("Check non-initialized managers \n");

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        st = cpssDxChExactMatchManagerStatisticsGet(ii, &statisticsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChExactMatchManagerStatisticsGet: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    PRV_UTF_LOG0_MAC("Check check out of range managers \n");
    /* check out of range managers ... expect 'GT_OUT_OF_RANGE' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        st = cpssDxChExactMatchManagerStatisticsGet(ii, &statisticsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
            "cpssDxChExactMatchManagerStatisticsGet: expected to GT_OUT_OF_RANGE on manager [%d]",
            ii);
    }

    numEntriesAdded = 0;
    numIndexesAdded = 0;
    maxTotalEntries = EXACT_MATCH_SIZE_FULL;

    if(em_debug_limited_entries)
    {
        maxTotalEntries = em_debug_limited_entries;
    }

    if(run_device_less == GT_FALSE)
    {
        PRV_UTF_LOG0_MAC("create manager with device \n");
    }
    else
    {
        PRV_UTF_LOG0_MAC("create manager without device \n");
    }
    /* create manager with device */
    st = createExactMatchManagerWithDevice(exactMatchManagerId,maxTotalEntries,prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "createExactMatchManagerWithDevice: expected to GT_OK on manager [%d]",
                exactMatchManagerId);

    if (st==GT_OK)
    {
        st = cpssDxChExactMatchManagerStatisticsClear(exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChExactMatchManagerStatisticsClear: expected to GT_OK on manager [%d]",
            exactMatchManagerId);

        PRV_UTF_LOG1_MAC("start - add [%d] entries \n",maxTotalEntries);
        for(ii = 0 ; ii < maxTotalEntries; /*update according to entry size*/)
        {
            entryAdded = GT_FALSE;

            exactMatchEntryType = exactMatchEntryType_valid_arr[ii % UT_EXACT_MATCH_ENTRY_TYPE_VALID_NUM];
            switch(exactMatchEntryType)
            {
                case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
                    exactMatchManagerEntryAdd_tti(exactMatchManagerId, ii, &outputResults);
                    break;
                case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
                    exactMatchManagerEntryAdd_pcl(exactMatchManagerId, ii, &outputResults);
                    break;
                case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
                    exactMatchManagerEntryAdd_epcl(exactMatchManagerId, ii, &outputResults);
                    break;
                default:
                    break;
            }

            if(entryAdded)
            {
                numEntriesAdded++;
                numIndexesAdded+=(outputResults.entryPtr->exactMatchEntry.key.keySize+1);
            }

            if(add_device_in_runtime == GT_TRUE &&
               (ii == (maxTotalEntries/2)))
            {
                CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[1];
                GT_U32                                                   numOfPairs=1;

                /* now in the middle of the insertions we add the device. */
                PRV_UTF_LOG0_MAC("now in the middle of the insertions we add the device \n");

                pairListArr[0].devNum = prvTgfDevNum;
                pairListArr[0].portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

                /* add device to it */
                st = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChExactMatchManagerDevListAdd: expected to GT_OK on manager [%d]",
                    exactMatchManagerId);
            }

            if(entryAdded)
            {
                ii+=(outputResults.entryPtr->exactMatchEntry.key.keySize+1);
            }
            else
            {
                ii++;
            }
        }

        PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
            maxTotalEntries,numIndexesAdded,
                (100*numIndexesAdded)/maxTotalEntries);

        if(em_debug_limited_entries)
        {
            cpssDxChExactMatchRulesDump(prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,0,(EXACT_MATCH_SIZE_FULL-1));
        }

        numEntriesRemoved = 0;
        getFirst = GT_TRUE;

        PRV_UTF_LOG1_MAC("start - remove the [%d] entries \n",numEntriesAdded);
        if(check_hw_hold_entry == GT_TRUE)
        {
            PRV_UTF_LOG0_MAC("(slow mode) for each entry check that HW hold the entry in proper index according to it's index \n");
        }

        /* remove all entries from the DB (one by one) */
        while(GT_OK == cpssDxChExactMatchManagerEntryGetNext(exactMatchManagerId,getFirst,&entry))
        {
            getFirst = GT_FALSE;

            if(check_hw_hold_entry == GT_TRUE)
            {
                /* just before we going to remove the entry ... lets check that it exists in the HW ! */
                 exactMatchManagerHwEntryCheck(&entry.exactMatchEntry);
            }

            EM_MANAGER_ENTRY_DELETE_WITH_STATISTICS(exactMatchManagerId,&entry,st);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChExactMatchManagerEntryDelete: expected to GT_OK on manager [%d]",
                exactMatchManagerId);
            numEntriesRemoved++;

            if((numEntriesRemoved % 1000) == 0)
            {
                PRV_UTF_LOG1_MAC("removed [%d] entries \n",numEntriesRemoved);
            }
        }

        st = cpssDxChExactMatchManagerStatisticsGet(exactMatchManagerId, &statisticsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChExactMatchManagerStatisticsGet: expected to GT_OK on manager [%d]",
            exactMatchManagerId);

        UTF_VERIFY_EQUAL1_STRING_MAC(0, cpssOsMemCmp(&statisticsGet, &statistics_global, sizeof(statisticsGet)),
                                     "cpssDxChExactMatchManagerStatisticsGet: expected 'get' entry to match 'set' entry in manager [%d]",
                                     exactMatchManagerId);

        PRV_UTF_LOG2_MAC("ended - removed [%d] entries (out of expected [%d]) \n",numEntriesRemoved,numEntriesAdded);

        UTF_VERIFY_EQUAL2_STRING_MAC(numEntriesAdded, numEntriesRemoved,
            "expected number of removed entries [%d] to be the same as the added entries[%d]",
            numEntriesRemoved,numEntriesAdded);

        cpssOsMemSet(&statistics_global,0,sizeof(statistics_global));
        cpssOsMemSet(&statisticsGet,0,sizeof(statisticsGet));

        st = cpssDxChExactMatchManagerStatisticsClear(exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChExactMatchManagerStatisticsClear: expected to GT_OK on manager [%d]",
            exactMatchManagerId);

         /* check invalid parameters */
        for(ii = 0 ; ii < UT_EXACT_MATCH_ENTRY_TYPE_VALID_NUM; ii++)
        {
            exactMatchEntryType = exactMatchEntryType_valid_arr[ii % UT_EXACT_MATCH_ENTRY_TYPE_VALID_NUM];
            switch(exactMatchEntryType)
            {
                case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
                    PRV_UTF_LOG0_MAC("check invalid parameters : tti \n");
                    bad_param_check_exactMatchManagerEntryUpdate_tti(exactMatchManagerId);
                    break;
                case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
                    PRV_UTF_LOG0_MAC("check invalid parameters : pcl \n");
                    bad_param_check_exactMatchManagerEntryUpdate_pcl(exactMatchManagerId);
                    break;
                case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
                    PRV_UTF_LOG0_MAC("check invalid parameters : epcl \n");
                    bad_param_check_exactMatchManagerEntryUpdate_epcl(exactMatchManagerId);
                    break;
                default:
                    break;
            }

        }

        PRV_UTF_LOG0_MAC("check NULL pointers \n");
        /* NULL pointer */
        EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,NULL,&params,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
            "cpssDxChExactMatchManagerEntryAdd: expected to GT_BAD_PTR on manager [%d]",
            exactMatchManagerId);

        EM_MANAGER_ENTRY_ADD_WITH_STATISTICS(exactMatchManagerId,&entry,NULL,st);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
            "cpssDxChExactMatchManagerEntryAdd: expected to GT_BAD_PTR on manager [%d]",
            exactMatchManagerId);

        st = cpssDxChExactMatchManagerStatisticsGet(exactMatchManagerId, &statisticsGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChExactMatchManagerStatisticsGet: expected to GT_OK on manager [%d]",
            exactMatchManagerId);

        UTF_VERIFY_EQUAL1_STRING_MAC(0, cpssOsMemCmp(&statisticsGet, &statistics_global, sizeof(statisticsGet)),
            "cpssDxChExactMatchManagerStatisticsGet: expected 'get' entry to match 'set' entry in manager [%d]",
            exactMatchManagerId);

        PRV_UTF_LOG0_MAC("Ended \n");
    }
    else
    {
        PRV_UTF_LOG0_MAC("Ended - Test fail , could not create manager - createExactMatchManagerWithDevice fail \n");
    }
    return;
}
/**
GT_STATUS cpssDxChExactMatchManagerStatisticsGet
(
    IN GT_U32                                           exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC    *statisticsPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerStatisticsGet)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    check_hw_hold_entry = GT_TRUE;/* very slow mode ! */

    internal_cpssDxChExactMatchManagerStatisticsGetUT();

    check_hw_hold_entry = GT_FALSE;

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}

/**
GT_STATUS cpssDxChExactMatchManagerDatabaseCheck
(
    IN  GT_U32                                                  exactMatchManagerId,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC             *checksPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      resultArray[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E],
    OUT GT_U32                                                  *errorNumberPtr
);
*/
static GT_VOID internal_cpssDxChExactMatchManagerDatabaseCheckUT(GT_VOID)
{
    GT_STATUS                                               st;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      testResultArray[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E];
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      expResultArray[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNum, errorNumItr, expErrorNum;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT                   exactMatchEntryType;
    GT_U32                                                  ii = 0, i;
    GT_BOOL                                                 entryAdded;
    GT_U32                                                  exactMatchManagerId    = 1;
    GT_U32                                                  numEntriesAdded = 0;
    GT_U32                                                  numIndexesAdded = 0;  /* number of entries added to each bank;
                                                                                    if one entry is added with keysize=47 then:
                                                                                    numEntriesAdded=1 and numIndexesAdded=4 */

    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                 entry;
    UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC    outputResults;
    GT_U32                                                  maxTotalEntriesArr[3];

    maxTotalEntriesArr[0] = (EXACT_MATCH_SIZE_FULL * 20)/100;
    maxTotalEntriesArr[1] = (EXACT_MATCH_SIZE_FULL * 50)/100;
    maxTotalEntriesArr[2] = (EXACT_MATCH_SIZE_FULL * 80)/100;

    outputResults.entryPtr                          = &entry;
    outputResults.entryAddedPtr                     = &entryAdded;

    /* create manager with device */
    st = createExactMatchManagerWithDevice(exactMatchManagerId,EXACT_MATCH_SIZE_FULL,prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "createExactMatchManagerWithDevice: expected to GT_OK on manager [%d]",
                exactMatchManagerId);
    if (st==GT_OK)
    {

        /* Basic API IN/OUT parameter Validation */
        st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, NULL, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, NULL, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        st = cpssDxChExactMatchManagerDatabaseCheck(CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS+1, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, st);

        for(i=0; i<3; i++)
        {
            /* Utilize Exact Match by 20% , 50%, 80% - and verify the tests */
            PRV_UTF_LOG1_MAC("Start - add [%d] entries\n", maxTotalEntriesArr[i]);
            for(; ii < EXACT_MATCH_SIZE_FULL; /*update according to entry size*/)
            {
                entryAdded = GT_FALSE;

                exactMatchEntryType = exactMatchEntryType_valid_arr[ii % UT_EXACT_MATCH_ENTRY_TYPE_VALID_NUM];

                switch(exactMatchEntryType)
                {
                    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
                        exactMatchManagerEntryAdd_tti(exactMatchManagerId, ii, &outputResults);
                        break;
                    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
                        exactMatchManagerEntryAdd_pcl(exactMatchManagerId, ii, &outputResults);
                        break;
                    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
                        exactMatchManagerEntryAdd_epcl(exactMatchManagerId, ii, &outputResults);
                        break;
                    default:
                        break;
                }
                if(entryAdded)
                {
                    numEntriesAdded++;
                    numIndexesAdded+=(outputResults.entryPtr->exactMatchEntry.key.keySize+1);
                }
                if((numEntriesAdded % 100) == 0)
                {
                    PRV_UTF_LOG1_MAC("Added [%d] entries \n", numEntriesAdded);
                }
                /* since the indexes are incremented according to the keySize we can not be sure we will
                   get to a case of numIndexesAdded == maxTotalEntriesArr[i] */
                if(numIndexesAdded >= maxTotalEntriesArr[i])
                {
                    break;
                }

                if(entryAdded)
                {
                    ii+=(outputResults.entryPtr->exactMatchEntry.key.keySize+1);
                }
                else
                {
                    ii++;
                }
            }
            PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
                    maxTotalEntriesArr[i], numIndexesAdded, (100*numIndexesAdded)/maxTotalEntriesArr[i]);

            cpssOsMemSet(testResultArray, 0, sizeof(testResultArray));
            st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChExactMatchManagerDatabaseCheck: expected to GT_OK on manager [%d]",
                    exactMatchManagerId);
            UTF_VERIFY_EQUAL2_STRING_MAC(errorNum, 0,
                    "cpssDxChExactMatchManagerDatabaseCheck: expected testResult SUCCESS, got [%d]errors on manager [%d]",
                    errorNum, exactMatchManagerId);
            for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
            {
                PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                                 errorNumItr,
                                 testResultArray[errorNumItr],
                                 debugErrorArr[testResultArray[errorNumItr]].errString);
            }
        }
        /* DEBUG API - prvCpssDxChExactMatchManagerDebugSelfTest */
        expErrorNum = 1;
        cpssOsMemSet(expResultArray, 0, sizeof(expResultArray));
        for(ii=CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E; ii<=CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E; ii++)
        {

            if (ii == CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E)
            {
                expErrorNum = ii;
                expResultArray[0] = CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E;
            }
            else
            {
                expResultArray[0]  = ii;
                expResultArray[ii] = ii;    /* Preparing array for full run */
            }

            st = prvCpssDxChExactMatchManagerDebugSelfTest(exactMatchManagerId, expResultArray, expErrorNum, GT_TRUE/* Inject Error */);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                        "prvCpssDxChExactMatchManagerDebugSelfTest: expected to GT_OK on manager [%d]",
                        exactMatchManagerId);
            cpssOsMemSet(testResultArray, 0, sizeof(testResultArray));
            st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
            if (ii==21)
            {
                /* CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_CLIENT_INVALID_E
                   cause GT_BAD_PARAM to be retuened due to  mismatch of actionType and clientType - this is as expected */
               UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                    "cpssDxChExactMatchManagerDatabaseCheck: expected to GT_BAD_PARAM on manager [%d]", exactMatchManagerId);
            }
            else{
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "cpssDxChExactMatchManagerDatabaseCheck: expected to GT_OK on manager [%d]", exactMatchManagerId);
            }

            if( ii == CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E /*13*/ &&
                debugErrorArr[ii].expErrArrSize != errorNum &&
                errorNum == 5)
            {
                /* NOTE: we get here in Phoenix if running at mainUT :
                    shell-execute prvUtfSkipLongTestsFlagSet 1
                    shell-execute utfTestsTypeRun 1,0,1,2,4,5,0

                    but we not get here if running just after cpssInitSystem.

                    we can get here also after cpssInitSystem , if calling 24 times to osRand() !!!
                    before calling the test
                */

                /* see in the error LOG : for the first 2 errors :
                    ERROR GT_BAD_PTR in function: internal_cpssDxChExactMatchManagerDatabaseCheck, file cpssdxchexactmatchmanager.c, line[8230]. errorNumPtr = 0
                    ERROR GT_BAD_PTR in function: internal_cpssDxChExactMatchManagerDatabaseCheck, file cpssdxchexactmatchmanager.c, line[8229]. resultArray = 0
                    ERROR GT_BAD_PTR in function: internal_cpssDxChExactMatchManagerDatabaseCheck, file cpssdxchexactmatchmanager.c, line[8231]. checksPtr = 0
                    ERROR GT_BAD_PARAM in function: internal_cpssDxChExactMatchManagerDatabaseCheck, file cpssdxchexactmatchmanager.c, line[8226]. exactMatchManagerId [33] >= [32] (out of range)
                    ERROR GT_FULL in function: mainLogicEntryAdd, file cpssDxChExactMatchManager.c, line[2977]. the entry not exists and not able to add it (before trying 'Cuckoo')
                    ERROR 0x0000ffff in function: prvCpssDxChExactMatchManagerDbCheck_counters, file prvcpssdxchbrgfdbmanager_db.c, line[3515]. Debug Error number [1] of Type[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E]
                    ERROR 0x0000ffff in function: prvCpssDxChExactMatchManagerDbCheck_counters, file prvcpssdxchbrgfdbmanager_db.c, line[3519]. mismatch for bankNum[3] : bankPopulation[2047] != db.bankPopulation[2048]
                    ERROR 0x0000ffff in function: prvCpssDxChExactMatchManagerDbCheck_indexPointer, file prvcpssdxchbrgfdbmanager_db.c, line[3823]. Debug Error number [2] of Type[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E]
                    ERROR 0x0000ffff in function: prvCpssDxChExactMatchManagerDbCheck_indexPointer, file prvcpssdxchbrgfdbmanager_db.c, line[3827]. mismatch for dbIndex[0] : fdbManagerPtr->entryPoolPtr[dbIndex].hwIndex[22772] != hwIndex[22771]
                */

                /* the invalid hwIndex +1 cause 3 more error places ! :
                    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E ,
                    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E ,
                    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E
                */

                /* see in the error LOG : for the 3 next errors : *
                    ERROR GT_BAD_STATE in function: prvCpssDxChExactMatchManagerDbAgeBinEntryGetNext, file prvCpssDxChExactMatchManager_db.c, line[3172].
                    ERROR 0x0000ffff in function: prvCpssDxChExactMatchManagerDbCheck_agingBin, file prvCpssDxChExactMatchManager_db.c, line[3907]. Debug Error number [3] of Type[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E]
                    ERROR 0x0000ffff in function: prvCpssDxChExactMatchManagerDbCheck_agingBin, file prvCpssDxChExactMatchManager_db.c, line[3913]. Debug Error number [4] of Type[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E]
                    ERROR 0x0000ffff in function: prvCpssDxChExactMatchManagerDbCheck_agingBin, file prvCpssDxChExactMatchManager_db.c, line[3916]. mismatch for totalAgeBinCnt[26213] != totalPopulation[26214]
                    ERROR 0x0000ffff in function: prvCpssDxChExactMatchManagerDbCheck_agingBin, file prvCpssDxChExactMatchManager_db.c, line[3928]. Debug Error number [5] of Type[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E]
                */

                errorNum = 2;/* make the test happy ! */
            }

            UTF_VERIFY_EQUAL1_STRING_MAC(debugErrorArr[ii].expErrArrSize, errorNum,
                "cpssDxChExactMatchManagerDatabaseCheck: failed for [%s]",debugErrorArr[ii].errString);

            for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
            {
                UTF_VERIFY_EQUAL3_STRING_MAC(debugErrorArr[ii].expErrArr[errorNumItr], testResultArray[errorNumItr],
                        "Expected error is not matching for debug Test[%s]: [Err_No-%d]-[%s]",
                        debugErrorArr[ii].errString,
                        testResultArray[errorNumItr],
                        debugErrorArr[testResultArray[errorNumItr]].errString);
            }
            st = prvCpssDxChExactMatchManagerDebugSelfTest(exactMatchManagerId, expResultArray, expErrorNum, GT_FALSE/* Restore Error */);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                    "prvCpssDxChExactMatchManagerDebugSelfTest: expected to GT_OK on manager [%d]",
                    exactMatchManagerId);
        }
        CLEANUP_MANAGER(exactMatchManagerId)
    }
    else
    {
        PRV_UTF_LOG0_MAC("Ended - Test fail , could not create manager - createExactMatchManagerWithDevice fail \n");
    }
    return;
}

/**
GT_STATUS cpssDxChExactMatchManagerDatabaseCheck
(
    IN  GT_U32                                                  exactMatchManagerId,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC             *checksPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      resultArray[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E],
    OUT GT_U32                                                  *errorNumberPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerDatabaseCheck)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    internal_cpssDxChExactMatchManagerDatabaseCheckUT();

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}

/* run the test of 'cpssDxChExactMatchManagerEntryAdd' , but without device bound to the manager */
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerEntryAdd_run_device_less)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    run_device_less = GT_TRUE;

    internal_cpssDxChExactMatchManagerEntryAddUT();

    run_device_less = GT_FALSE;

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}
/* run the test of 'cpssDxChExactMatchManagerEntryAdd' , but the device added to the
   manager after 1/2 table already full */
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerEntryAdd_add_device_in_runtime)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    run_device_less = GT_TRUE;
    add_device_in_runtime = GT_TRUE;

    internal_cpssDxChExactMatchManagerEntryAddUT();

    run_device_less = GT_FALSE;
    add_device_in_runtime = GT_FALSE;

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}

/* run the test of 'cpssDxChExactMatchManagerEntryUp-date' , but without device bound to the manager */
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerEntryUpdate_run_device_less)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    run_device_less = GT_TRUE;

    internal_cpssDxChExactMatchManagerEntryUpdateUT();

    run_device_less = GT_FALSE;

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}

/* run the test of 'cpssDxChExactMatchManagerEntryUpdate' , but the device added to the
   manager after 1/2 table already full */
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerEntryUpdate_add_device_in_runtime)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    run_device_less = GT_TRUE;
    add_device_in_runtime = GT_TRUE;

    internal_cpssDxChExactMatchManagerEntryUpdateUT();

    run_device_less = GT_FALSE;
    add_device_in_runtime = GT_FALSE;

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}

GT_VOID internal_cpssDxChExactMatchManagerCuckooEntryAddPerformanceUT
(
    IN GT_BOOL                                             isRand,
    IN CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT                  keySize,
    IN GT_BOOL                                             doPrint,
    IN GT_BOOL                                             isLongTest,
    IN GT_U32                                              seed
)
{
    GT_STATUS                                               st=GT_OK;
    GT_U32                                                  ii;
    GT_U32                                                  numEntriesAdded=0;  /* total number of entries added to the DB */
    GT_U32                                                  numIndexesAdded;    /* number of entries added to each bank;
                                                                                if one entry is added with keysize=47 then:
                                                                                numEntriesAdded=1 and numIndexesAdded=4 */
    GT_U32                                                  numEntriesMissed=0;
    GT_BOOL                                                 entryAdded;/* indication that the entry was added to the DB */
    GT_U32                                                  exactMatchManagerId = TESTED_EXACT_MATCH_MANAGER_ID_CNS;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                 entry;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC      params;
    GT_U32                                                  maxTotalEntries;
    UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC    outputResults;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      testResultArray[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNum=0, errorNumItr=0,size=0;
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT                      currKeySize;
    GT_U32                                                  firstMissStatus = 0;
    GT_U32                                                  firstMissCapacity = 0;

    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of init */
    GT_U32  secondsStartEntry, secondsEndEntry,
            nanoSecondsStartEntry, nanoSecondsEndEntry,
            secondsEntry, nanoSecEntry; /* time of init */
    GT_U32  j,secondsEntryMax=0, nanoSecEntryMax=0;

    /* prepare fixed random array */
    if (isRand == GT_TRUE && isLongTest == GT_FALSE)
    {
        for (ii=0; ii<131072; ii++)
            {
                for (j=0; j<47; j++) {
                    randomArr[ii].data[j] = cpssOsRand() & 0xFF;
                }
            }
    }

    /* prepare random array according to seed */
    if (isLongTest)
    {
        /* set specific seed for random generator */
        cpssOsSrand(seed);
        for (ii=0; ii<131072; ii++)
        {
            for (j=0; j<47; j++) {
                randomArr[ii].data[j] = cpssOsRand() & 0xFF;
            }
        }
    }
    cpssOsMemSet(&entry ,0,sizeof(entry));
    cpssOsMemSet(&entry.exactMatchEntry.key.pattern ,0,sizeof(entry.exactMatchEntry.key.pattern));
    cpssOsMemSet(&params,0,sizeof(params));
    cpssOsMemSet(testResultArray,0,sizeof(testResultArray));

    outputResults.entryPtr          = &entry;
    outputResults.entryAddedPtr     = &entryAdded;

    numEntriesAdded = 0;
    numIndexesAdded = 0;
    maxTotalEntries = EXACT_MATCH_SIZE_FULL;

    PRV_UTF_LOG2_MAC("create manager with device: numOfHwIndexes [%ld] numOfHashes [%d]\n",EXACT_MATCH_SIZE_FULL,EXACT_MATCH_MAX_BANKS);

    /* create manager with device */
    st = createExactMatchManagerWithDevice(exactMatchManagerId,maxTotalEntries,prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "createExactMatchManagerWithDevice: expected to GT_OK on manager [%d]",
                exactMatchManagerId);

    if (st==GT_OK)
    {
        switch (keySize)
        {
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
            size = 1;
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [5B]\n", maxTotalEntries/size);
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
            size = 2;
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [19B]\n", maxTotalEntries/size);
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
            size = 3;
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [33B]\n", maxTotalEntries/size);
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
            size = 4;
            PRV_UTF_LOG1_MAC("start - add [%d] entries for keySize [47B]\n", maxTotalEntries/size);
            break;
        default:
            break;
        }

        currKeySize = keySize;

        START_TIME(st);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,st,"cpssOsTimeRT: expected GT_OK");

        for(ii = 1 ; ii <= maxTotalEntries/size; ii++)
        {
            entryAdded = GT_FALSE;
            cpssOsTimeRT(&secondsStartEntry,&nanoSecondsStartEntry);
            exactMatchManagerEntryAddCuckoo_pcl(exactMatchManagerId, ii, &outputResults,isRand,currKeySize,GT_TRUE,GT_FALSE);
            CALC_DIFF(st);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,st,"CALC_DIFF: expected GT_OK");
            if ((secondsEntry > 0) && (secondsEntryMax < secondsEntry))
            {
                secondsEntryMax = secondsEntry;
                nanoSecEntryMax = nanoSecEntry;
            }
            else
            {
                nanoSecEntryMax = nanoSecEntry;
            }
            if ((ii%1024) == 0)
            {
                if (doPrint == GT_TRUE)
                {
                    cpssOsPrintf("%dk ",ii/1024);
                }
            }
            if(entryAdded)
            {
                numEntriesAdded++;
                numIndexesAdded+=(outputResults.entryPtr->exactMatchEntry.key.keySize+1);
            }
            else
            {
                if (outputResults.rc == GT_ALREADY_EXIST)
                    continue;

                numEntriesMissed++;
                if (outputResults.rc == GT_FULL)
                {
                    if (firstMissStatus == 0)
                    {
                        firstMissStatus = 1;
                        firstMissCapacity = (100*numIndexesAdded)/maxTotalEntries;

                    }
                }
            }
        }
        HOW_MUCH_TIME(numIndexesAdded,maxTotalEntries,GT_TRUE,st);

        PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
            maxTotalEntries,numIndexesAdded,
                (100*numIndexesAdded)/maxTotalEntries);

        if (firstMissCapacity != 0)
        {
            cpssOsPrintf("\nCuckoo 'ADD' first miss capacity %d%% num entries missed %d\n", firstMissCapacity,numEntriesMissed);
        }
        else
        {
            cpssOsPrintf("\nCuckoo 'ADD' NO misses capacity %d%%\n", (100*numIndexesAdded)/maxTotalEntries);
        }

        cpssOsPrintf("Entry Max time: %d sec., %d nanosec.\n",secondsEntryMax,nanoSecEntryMax);
        prvCpssDxChCuckooDbPrintStatistics(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);

        if (isLongTest)
        {
            if (firstMissCapacity != 0)
            {
                randomResArray[seed].firstMiss.noFirstMiss = GT_FALSE;
                randomResArray[seed].firstMiss.capacity = firstMissCapacity;
                randomResArray[seed].firstMiss.numEntriesMissed = numEntriesMissed;
            }
            else
            {
                randomResArray[seed].firstMiss.noFirstMiss = GT_TRUE;
            }
            randomResArray[seed].utilization.capacity = (100*numIndexesAdded)/maxTotalEntries;
            randomResArray[seed].utilization.seconds = seconds;
            randomResArray[seed].utilization.nanoSec = nanoSec;
            randomResArray[seed].utilization.succeededIndexes = numIndexesAdded;
            randomResArray[seed].utilization.secondsEntryMax = secondsEntryMax;
            randomResArray[seed].utilization.nanoSecEntryMax = nanoSecEntryMax;
        }

        /* Verify DB */
        st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                    errorNumItr,
                    testResultArray[errorNumItr],
                    debugErrorArr[testResultArray[errorNumItr]].errString);
        }

        PRV_UTF_LOG0_MAC("Ended \n\n");
    }
    else
    {
        PRV_UTF_LOG0_MAC("Ended - Test fail , could not create manager - createExactMatchManagerWithDevice fail \n");
    }

    /* restore */
    CLEANUP_ALL_MANAGERS;

    return;
}

GT_VOID internal_cpssDxChExactMatchManagerCuckooEntryAddPerformanceAverageUT
(
    IN CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT                   keySize,
    IN GT_BOOL                                              doPrint
)
{
    GT_U32 i,counterFirstMiss = 0;
    GT_FLOAT64 time,entryTime;
    GT_U32 NUM_ITERATIONS = 100;
    GT_CHAR str[10];

    switch (keySize)
    {
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
        cpssOsMemCpy(str,"[5B]",sizeof("[5B]"));
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
        cpssOsMemCpy(str,"[19B]",sizeof("[19B]"));
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
        cpssOsMemCpy(str,"[33B]",sizeof("[33B]"));
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
        cpssOsMemCpy(str,"[47B]",sizeof("[47B]"));
        break;
    default:
        break;
    }

    for (i=1; i<=NUM_ITERATIONS; i++)
    {
        cpssOsPrintf("ITERATION %d keySize %s:\n\n",i,str);
        internal_cpssDxChExactMatchManagerCuckooEntryAddPerformanceUT(GT_TRUE,keySize,GT_TRUE,GT_TRUE,i);
    }

    /* location 0 will hold the average result */
    cpssOsMemSet(&randomResArray[0],0,sizeof(randomResArray[0]));

    for (i=1; i<=NUM_ITERATIONS; i++)
    {
        if (randomResArray[i].firstMiss.noFirstMiss == GT_FALSE)
        {
            randomResArray[0].firstMiss.capacity += randomResArray[i].firstMiss.capacity;
            randomResArray[0].firstMiss.numEntriesMissed += randomResArray[i].firstMiss.numEntriesMissed;
            counterFirstMiss++;
        }
        randomResArray[0].utilization.capacity += randomResArray[i].utilization.capacity;
        randomResArray[0].utilization.seconds += randomResArray[i].utilization.seconds;
        randomResArray[0].utilization.nanoSec += randomResArray[i].utilization.nanoSec;
        if (randomResArray[0].utilization.nanoSec >= 1000000000)
        {
            randomResArray[0].utilization.nanoSec -= 1000000000;
            randomResArray[0].utilization.seconds++;
        }
        randomResArray[0].utilization.succeededIndexes += randomResArray[i].utilization.succeededIndexes;
        randomResArray[0].utilization.secondsEntryMax += randomResArray[i].utilization.secondsEntryMax;
        randomResArray[0].utilization.nanoSecEntryMax += randomResArray[i].utilization.nanoSecEntryMax;
        if (randomResArray[0].utilization.nanoSecEntryMax >= 1000000000)
        {
            randomResArray[0].utilization.nanoSecEntryMax -= 1000000000;
            randomResArray[0].utilization.secondsEntryMax++;
        }
    }
    time = randomResArray[0].utilization.seconds + randomResArray[0].utilization.nanoSec*0.000000001;
    time = time/NUM_ITERATIONS;
    entryTime = randomResArray[0].utilization.secondsEntryMax + randomResArray[0].utilization.nanoSecEntryMax*0.000000001;
    entryTime = entryTime/NUM_ITERATIONS;

    /* save data */
    randomFinalResArray[keySize].firstMiss.capacity = randomResArray[0].firstMiss.capacity/counterFirstMiss;
    randomFinalResArray[keySize].firstMiss.numEntriesMissed = randomResArray[0].firstMiss.numEntriesMissed/counterFirstMiss;
    randomFinalResArray[keySize].utilization.capacity = randomResArray[0].utilization.capacity/NUM_ITERATIONS;
    randomFinalResArray[keySize].time = time;
    randomFinalResArray[keySize].utilization.succeededIndexes = randomResArray[0].utilization.succeededIndexes/NUM_ITERATIONS;
    randomFinalResArray[keySize].entryTime = entryTime;

    if (doPrint == GT_TRUE)
    {
        cpssOsPrintf("Average results for %d random iterations keySize %s:\n",NUM_ITERATIONS,str);
        cpssOsPrintf("----------------------------------------------------\n\n");
        cpssOsPrintf("first miss capacity %d%% num entries missed %d\n",
                     randomResArray[0].firstMiss.capacity/counterFirstMiss,randomResArray[0].firstMiss.numEntriesMissed/counterFirstMiss);
        cpssOsPrintf("utilization: %d%% %10.10f sec.\n",randomResArray[0].utilization.capacity/NUM_ITERATIONS,time);

        cpssOsPrintf("out of [%d] entries , succeeded [%d] entries\n",
                     EXACT_MATCH_SIZE_FULL,randomResArray[0].utilization.succeededIndexes/NUM_ITERATIONS);

        cpssOsPrintf("Entry Max time: %10.10f sec.\n",entryTime);
    }
    return;

}

/* this is a very very long test; run it only manualy */
GT_VOID internal_cpssDxChExactMatchManagerCuckooEntryAddPerformanceAverageAllUT
(
    GT_VOID
)
{
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT key;
    GT_CHAR str[10];

    BIND_MALLOC_AND_FREE;

    for (key=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E; key<CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E; key++)
    {
        internal_cpssDxChExactMatchManagerCuckooEntryAddPerformanceAverageUT(key,GT_TRUE);
    }

    for (key=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E; key<CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E; key++)
    {

        switch (key)
        {
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
            cpssOsMemCpy(str,"[5B]",sizeof("[5B]"));
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
            cpssOsMemCpy(str,"[19B]",sizeof("[19B]"));
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
            cpssOsMemCpy(str,"[33B]",sizeof("[33B]"));
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
            cpssOsMemCpy(str,"[47B]",sizeof("[47B]"));
            break;
        default:
            break;
        }

        cpssOsPrintf("Average results for 100 random iterations keySize %s:\n",str);
        cpssOsPrintf("-----------------------------------------------------\n\n");
        cpssOsPrintf("first miss capacity %d%% num entries missed %d\n",
                     randomFinalResArray[key].firstMiss.capacity,randomFinalResArray[key].firstMiss.numEntriesMissed);
        cpssOsPrintf("utilization: %d%% %10.10f sec.\n",randomFinalResArray[key].utilization.capacity,randomFinalResArray[key].time);

        cpssOsPrintf("out of [%d] entries , succeeded [%d] entries\n",
                     EXACT_MATCH_SIZE_FULL,randomFinalResArray[key].utilization.succeededIndexes);

        cpssOsPrintf("Entry Max time: %10.10f sec.\n\n",randomFinalResArray[key].entryTime);
    }

    RESTORE_OS_MALLOC_AND_OS_FREE;

    return;

}

/* run this test manualy
   Test purppose to check failures distribution for ipv6 key 47B MHT2 */
GT_VOID internal_cpssDxChCuckooEntryAddIpv6EntryIncrementalUT
(
    GT_BOOL         isRand
)
{
    GT_STATUS rc;
    GT_U32 exactMatchManagerId = 1;
    GT_U32 db_banks = 8; /* 8 hash for MHT2 with key size 47B */
    GT_U32 db_lines = 16*1024; /* total 128K = 8*16K */
    GT_U32 size = 4; /* key 47B */
    GT_U32 index,depth,sizeInBytes = 47;
    GT_U8 key[PRV_CPSS_DXCH_CUCKOO_MAX_KEY_SIZE_CNS];
    PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC entryInfo;
    GT_U32 failed=0, numOfOperations,counter=0;
    GT_U32 count_banks = 0,first_miss=0,i,j;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    PRV_CPSS_DXCH_CUCKOO_DB_LOG_ARRAY_STC *cuckooDbLogPtr;

    cpssOsMemSet(&entryInfo,0,sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_INFO_STC));
    cpssOsMemSet(key,0,sizeof(key));

    BIND_MALLOC_AND_FREE;

    PRV_UTF_LOG2_MAC("create manager with device: numOfHwIndexes [%ld] numOfHashes [%d]\n",8,128*1024);

    /* the test should check MHT2; however EMM doesn't support it so i change manager capacity manually */
    /* create manager with device */
    rc = createExactMatchManagerWithDevice(exactMatchManagerId,32*1024,prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
    if (rc != GT_OK)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "createExactMatchManagerWithDevice: expected to be GT_OK on manager [%d]",exactMatchManagerId);
        goto exit_cleanly_lbl;
    }

    /* delete cuckoo DB; the cuckoo was created with 32K according to manager capacity attributes.
       We will delete it and create cuckoo manager with 128K and 8 hashes */
    rc = prvCpssDxChCuckooDbDelete(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);
    if (rc != GT_OK)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssDxChCuckooDbDelete: expected to be GT_OK on EM client and manager [%d]",exactMatchManagerId);
        goto exit_cleanly_lbl;
    }

    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes = 8;
    exactMatchManagerPtr->cpssHashParams.size = PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_128K_E;

    /* initialize cuckoo DB with 128K and 8 hashes */
    rc = prvCpssDxChCuckooDbInit(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);
    if (rc != GT_OK)
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssDxChCuckooDbInit: expected to be GT_OK on EM client and manager [%d]",exactMatchManagerId);
        goto exit_cleanly_lbl;
    }

    /* prepare fixed random array */
    if (isRand == GT_TRUE)
    {
        for (i=0; i<32768; i++)
        {
            for (j=0; j<47; j++) {
                randomArr[i].data[j] = cpssOsRand() & 0xFF;
            }
        }
    }

    for (index = 1; index <= db_lines*db_banks/size; index++)
    {
        if (isRand == GT_FALSE)
        {
            /* build ipv6 key */

            key[0] = 0x21;      /* PCLID 2B */
            key[1] = 0x43;
            key[2] = 0x55;      /* UDB 1B */
            key[3] = 0x11;      /* VRFID 1B */
            key[4] = 0x11;
            key[5] = 0xee;      /* Ipv6 DIP 16B */
            key[6] = 0xff;
            key[7] = 0xdd;
            key[8] = 0xcc;
            key[9] = index & 0xFF;
            key[10] = (index >> 8) & 0xFF;
            key[11] = (index >> 16) & 0xFF;
            key[12] = (index >> 24) & 0xFF;
            key[13] = 0;
            key[14] = 0;
            key[15] = 0;
            key[16] = 0;
            key[17] = 0;
            key[18] = 0;
            key[19] = 0x22;
            key[20] = 0x11;
            key[21] = 0x12;      /* Ipv6 SIP 16B */
            key[22] = 0x32;
            key[23] = 0x45;
            key[24] = 0x67;
            key[25] = index & 0xFF;
            key[26] = index & 0xFF;
            key[27] = (index >> 8) & 0xFF;
            key[28] = 0;
            key[29] = (index >> 16) & 0xFF;
            key[30] = 0;
            key[31] = (index >> 16) & 0xFF;
            key[32] = (index >> 24) & 0xFF;
            key[33] = 0;
            key[34] = 0;
            key[35] = 0x45;
            key[36] = 0x65;
            key[37] = (index > 1024*16) ? 0x11 : 0x6; /* IP PROTOCOL 1B */
            key[38] = index & 0xFF;         /* UDP Source Port 2B*/
            key[39] = (index >> 8) & 0xFF;
            key[40] = index & 0xFF;         /* UDP Dest Port 2B*/
            key[41] = (index >> 8) & 0xFF;
            key[42] = 0;
            key[43] = 0;
            key[44] = 0;
            key[45] = 0;
            key[46] = 0;
        }
        else
        {
            for (i=0; i<47; i++)
                key[i]=randomArr[index].data[i];
        }

        /* get cuckoo max depth definition according to key size in bytes */
        rc = prvCpssDxChCuckooDbMaxDepthGet(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId,sizeInBytes,&depth);
        if (rc != GT_OK)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssDxChCuckooDbInit: expected to be GT_OK on EM client and manager [%d]",exactMatchManagerId);
            goto exit_cleanly_lbl;
        }

        /* prepare entryInfo */
        entryInfo.bank = PRV_CPSS_DXCH_CUCKOO_INVALID_BANK_CNS;
        entryInfo.sizeInBytes = sizeInBytes;
        cpssOsMemCpy(entryInfo.data,key,sizeof(GT_U8)*sizeInBytes);
        entryInfo.depth = depth;
        entryInfo.line = 0;

        /* Rehash Exact Match entry */
        rc = prvCpssDxChCuckooDbRehashEntry(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId,&entryInfo);

        /* Free place found in DB */
        if (rc == GT_OK)
        {
            /* perform all the log operations in cuckoo DB */
            rc = prvCpssDxChCuckooRelocateEntriesDebug(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);
            if (rc != GT_OK)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssDxChCuckooRelocateEntriesDebug: expected to be GT_OK on EM client and manager [%d]",exactMatchManagerId);
                goto exit_cleanly_lbl;
            }

            /* delete log operations */
            rc = prvCpssDxChCuckooDbLogArrayDelete(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);
            if (rc != GT_OK)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssDxChCuckooDbLogArrayDelete: expected to be GT_OK on EM client and manager [%d]",exactMatchManagerId);
                goto exit_cleanly_lbl;
            }
            count_banks +=4;
            counter +=4;
        }
        else
        {
            failed++;
            first_miss = count_banks;
            if (failed == 1)
            {
                cpssOsPrintf("First index Failure: capacity %d\n",(count_banks*100)/(db_banks*db_lines));
            }

            /* get pointer to cuckoo log operations */
            rc = prvCpssDxChCuckooDbLogPtrGet(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId,&numOfOperations,&cuckooDbLogPtr);
            if (rc != GT_OK)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssDxChCuckooDbLogPtrGet: expected to be GT_OK on EM client and manager [%d]",exactMatchManagerId);
                goto exit_cleanly_lbl;

            }
            if (numOfOperations)
            {
                /* delete log operations */
                rc = prvCpssDxChCuckooDbLogArrayDelete(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);
                if (rc != GT_OK)
                {
                    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvCpssDxChCuckooDbLogArrayDelete: expected to be GT_OK on EM client and manager [%d]",exactMatchManagerId);
                    goto exit_cleanly_lbl;
                }
            }
        }

        if (counter >= 1310 && first_miss != 0)
        {
            cpssOsPrintf("capacity %d total failed indexes %d\n",(count_banks*100)/(db_banks*db_lines),failed);
            counter = 0;
        }

    }
    cpssOsPrintf("Final Results: capacity %d total failed indexes %d or entries %d\n",(count_banks*100)/(db_banks*db_lines),failed,failed*4);
    prvCpssDxChCuckooDbPrintStatistics(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;

    return;

exit_cleanly_lbl:

CLEANUP_ALL_MANAGERS;

RESTORE_OS_MALLOC_AND_OS_FREE;

}

/* run cpssDxChExactMatchManagerEntryAdd API to check
   performance and capacity while adding incremental keys
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerEntryAddCuckooCapacityIncrementalPreformance)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    internal_cpssDxChExactMatchManagerCuckooEntryAddPerformanceUT(GT_FALSE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E,GT_FALSE,GT_FALSE,0);

    internal_cpssDxChExactMatchManagerCuckooEntryAddPerformanceUT(GT_FALSE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E,GT_FALSE,GT_FALSE,0);

    internal_cpssDxChExactMatchManagerCuckooEntryAddPerformanceUT(GT_FALSE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E,GT_FALSE,GT_FALSE,0);

    internal_cpssDxChExactMatchManagerCuckooEntryAddPerformanceUT(GT_FALSE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E,GT_TRUE,GT_FALSE,0);

    RESTORE_OS_MALLOC_AND_OS_FREE;

}

/* run cpssDxChExactMatchManagerEntryAdd API to check
   performance and capacity while adding random keys
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerEntryAddCuckooCapacityRandomPreformance)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    internal_cpssDxChExactMatchManagerCuckooEntryAddPerformanceUT(GT_TRUE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E,GT_FALSE,GT_FALSE,0);

    internal_cpssDxChExactMatchManagerCuckooEntryAddPerformanceUT(GT_TRUE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E,GT_FALSE,GT_FALSE,0);

    internal_cpssDxChExactMatchManagerCuckooEntryAddPerformanceUT(GT_TRUE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E,GT_FALSE,GT_FALSE,0);

    internal_cpssDxChExactMatchManagerCuckooEntryAddPerformanceUT(GT_TRUE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E,GT_FALSE,GT_FALSE,0);

    RESTORE_OS_MALLOC_AND_OS_FREE;

}

/**
GT_STATUS cpssDxChExactMatchManagerEntryAdd
(
    IN GT_U32                                               exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC              *entryPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC   *paramsPtr
);
GT_STATUS cpssDxChExactMatchManagerDelete
(
    IN GT_U32 exactMatchManagerId
);
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerEntryAddDeleteAddCuckooCapacityIncremental)
{
    /* this is a very long test when number of hashes is bigger then 4 */
    if (EXACT_MATCH_MAX_BANKS > 4)
    {
         /* skip this test when 'prvUtfSkipLongTestsFlagSet' */
        PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);
    }

    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    internal_cpssDxChExactMatchManagerCuckooEntryAddDeleteAddUT(GT_FALSE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E);

    CLEANUP_ALL_MANAGERS;

    internal_cpssDxChExactMatchManagerCuckooEntryAddDeleteAddUT(GT_FALSE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E);

    CLEANUP_ALL_MANAGERS;

    internal_cpssDxChExactMatchManagerCuckooEntryAddDeleteAddUT(GT_FALSE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E);

    CLEANUP_ALL_MANAGERS;

    internal_cpssDxChExactMatchManagerCuckooEntryAddDeleteAddUT(GT_FALSE,CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E);

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;

}


/**
GT_STATUS cpssDxChExactMatchManagerEntryRewrite
(

    IN  GT_U32 exactMatchManagerId,
    IN  GT_U32 entriesIndexesArray[],
    IN  GT_U32 entriesIndexesNum
)
*/
static GT_VOID internal_cpssDxChExactMatchManagerEntryRewriteUT()
{
    GT_STATUS                                               st;
    GT_U32                                                  ii;
    GT_U32                                                  exactMatchManagerId = TESTED_EXACT_MATCH_MANAGER_ID_CNS;
    GT_U32                                                  entriesIndexesArray[512];
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC            statistics;

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        st = cpssDxChExactMatchManagerEntryRewrite(ii, entriesIndexesArray, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChExactMatchManagerEntryRewrite: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    /* check out of range managers ... expect 'GT_OUT_OF_RANGE' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        st = cpssDxChExactMatchManagerEntryRewrite(ii, entriesIndexesArray, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
            "cpssDxChExactMatchManagerEntryRewrite: expected to GT_BAD_PARAM on manager [%d]",
            ii);
    }

    /* create single valid manager */
    st = createExactMatchManagerWithDevice(exactMatchManagerId,EXACT_MATCH_SIZE_FULL,prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
     UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "createExactMatchManagerWithDevice: expected to GT_OK on manager [%d]",
                exactMatchManagerId);
     if (st==GT_OK)
     {
        exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

        /* Check for NULL pointer */
        st = cpssDxChExactMatchManagerEntryRewrite(exactMatchManagerId, NULL, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
            "cpssDxChExactMatchManagerEntryRewrite: expected to GT_BAD_PTR on manager [%d]",
            exactMatchManagerId);

        st = cpssDxChExactMatchManagerStatisticsGet(exactMatchManagerId, &statistics);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,st, "cpssDxChExactMatchManagerStatisticsGet: GT_OK");
        UTF_VERIFY_EQUAL1_STRING_MAC(0,statistics.entriesRewriteOk,
            "cpssDxChExactMatchManagerStatisticsGet: rewrite OK - invalid counter", exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(1,statistics.entriesRewriteErrorInputInvalid,
            "cpssDxChExactMatchManagerStatisticsGet: rewrite Error input invalid - invalid counter", exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(0,statistics.entriesRewriteTotalRewrite,
            "cpssDxChExactMatchManagerStatisticsGet: rewrite total count - invalid counter", exactMatchManagerId);

        /* Fill with random hwIndex */
        for(ii=0; ii<5; ii++)
        {
            /* hw index range 20 bits */
            entriesIndexesArray[ii] = cpssOsRand() | (1 << 21);
        }

        /* Check for invalid Exact Match hwIndex */
        st = cpssDxChExactMatchManagerEntryRewrite(exactMatchManagerId, entriesIndexesArray, 5);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
            "cpssDxChExactMatchManagerEntryRewrite: expected to GT_OK on manager [%d]",
            exactMatchManagerId);

        st = cpssDxChExactMatchManagerStatisticsGet(exactMatchManagerId, &statistics);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,st, "cpssDxChExactMatchManagerStatisticsGet: GT_OK");
        UTF_VERIFY_EQUAL1_STRING_MAC(0,statistics.entriesRewriteOk,
            "cpssDxChExactMatchManagerStatisticsGet: rewrite OK - invalid counter", exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(2,statistics.entriesRewriteErrorInputInvalid,
            "cpssDxChExactMatchManagerStatisticsGet: rewrite Error input invalid - invalid counter", exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(0,statistics.entriesRewriteTotalRewrite,
            "cpssDxChExactMatchManagerStatisticsGet: rewrite total count - invalid counter", exactMatchManagerId);

        /* Fill with random hwIndex */
        for(ii=0; ii<512; ii++)
        {
            entriesIndexesArray[ii] = cpssOsRand() % exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes;
        }

        /* since non of those indexes were defined the rewrite will do invalidate to all indexes */
        st = cpssDxChExactMatchManagerEntryRewrite(exactMatchManagerId, entriesIndexesArray, 512);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
            "cpssDxChExactMatchManagerEntryRewrite: expected to GT_OK on manager [%d]",
            exactMatchManagerId);

        st = cpssDxChExactMatchManagerStatisticsGet(exactMatchManagerId, &statistics);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,st, "cpssDxChExactMatchManagerStatisticsGet: GT_OK");
        UTF_VERIFY_EQUAL1_STRING_MAC(1,statistics.entriesRewriteOk,
            "cpssDxChExactMatchManagerStatisticsGet: rewrite OK - invalid counter", exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(2,statistics.entriesRewriteErrorInputInvalid,
            "cpssDxChExactMatchManagerStatisticsGet: rewrite Error input invalid - invalid counter", exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(512,statistics.entriesRewriteTotalRewrite,
            "cpssDxChExactMatchManagerStatisticsGet: rewrite total count - invalid counter", exactMatchManagerId);
     }
     else
     {
        PRV_UTF_LOG0_MAC("Ended - Test fail , could not create manager - createExactMatchManagerWithDevice fail \n");
     }
}

/**
GT_STATUS cpssDxChExactMatchManagerEntryRewrite
(

    IN  GT_U32 exactMatchManagerId,
    IN  GT_U32 entriesIndexesArray[],
    IN  GT_U32 entriesIndexesNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerEntryRewrite)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    internal_cpssDxChExactMatchManagerEntryRewriteUT();

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}


typedef GT_STATUS DXCH_DATA_INTEGRITY_EVENT_CB_FUNC
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC     *eventPtr
);
extern DXCH_DATA_INTEGRITY_EVENT_CB_FUNC        *dxChDataIntegrityEventIncrementFunc;
static DXCH_DATA_INTEGRITY_EVENT_CB_FUNC        *savedDataIntegrityHandler;
#define EM_MANAGER_SER_INDEX_ARRAY_SIZE         10
static GT_U32                                   indexesArray[EM_MANAGER_SER_INDEX_ARRAY_SIZE];
static GT_U32                                   indexesNum;
static CPSS_DXCH_LOCATION_SPECIFIC_INFO_STC     errorLocationInfo;
GT_VOID prvCpssDxChExactMatchManagerEntryRewrite_SER_errorInject
(
    IN GT_BOOL    status
)
{
    GT_STATUS                               rc;
    CPSS_EVENT_MASK_SET_ENT                 mask;

    mask = (status == GT_TRUE)?CPSS_EVENT_UNMASK_E:CPSS_EVENT_MASK_E;

    /* AUTODOC: Unmask/Mask interrupts from the DFX */
    rc = cpssDxChDiagDataIntegrityEventMaskSet(prvTgfDevNum,
                                               &errorLocationInfo,
                                               CPSS_DIAG_DATA_INTEGRITY_ERROR_CAUSE_TYPE_PARITY_E,
                                               mask);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: enable/disabe error injection to Exact Match table */
    rc = cpssDxChDiagDataIntegrityErrorInjectionConfigSet(prvTgfDevNum,
                                                          &errorLocationInfo,
                                                          CPSS_DIAG_DATA_INTEGRITY_ERROR_INJECT_MODE_MULTIPLE_E,
                                                          status);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
}

/**
* @internal hwInfoEventErrorHandlerFunc function
* @endinternal
*
* @brief   Callback called in case of data integrity error.
*          Store HW related info from event to global structure.
* @param[in] dev                      - the device.
* @param[in] eventPtr                 - filled structure which describes data integrity error event
*/
static GT_STATUS hwInfoEventErrorHandlerFunc
(
    IN GT_U8                                     dev,
    IN CPSS_DXCH_DIAG_DATA_INTEGRITY_EVENT_STC  *eventPtr

)
{
    GT_U32                                              ii;
    CPSS_NULL_PTR_CHECK_MAC(eventPtr);
    if(dev >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        /* check device out of range */
        return GT_BAD_PARAM;
    }
    if(indexesNum > EM_MANAGER_SER_INDEX_ARRAY_SIZE-1)
    {
        PRV_UTF_LOG0_MAC("indexesArray : index out of range");
        return GT_BAD_PARAM;
    }
    CPSS_ZERO_LEVEL_API_LOCK_MAC
    /* Add the HW indexes to the index array - will be processed by the correction API */
    for(ii=0; ii<indexesNum; ii++)
    {
        if(indexesArray[ii] == eventPtr->location.hwEntryInfo.hwTableEntryIndex)
        {
            CPSS_ZERO_LEVEL_API_UNLOCK_MAC
            return GT_OK;
        }
    }
    indexesArray[indexesNum] = eventPtr->location.hwEntryInfo.hwTableEntryIndex;
    indexesNum+=1;
    CPSS_ZERO_LEVEL_API_UNLOCK_MAC

    PRV_UTF_LOG3_MAC("Interrupt received for HW table index : 0x%x - portGroup 0x%x, indexesArray[%d]\n",
            eventPtr->location.hwEntryInfo.hwTableEntryIndex,
            eventPtr->location.portGroupsBmp,(indexesNum-1));

    return GT_OK;
}

void internal_cpssDxChExactMatchManagerEntryRewrite_SER(void)
{
    GT_STATUS                                                   rc;
    GT_U32                                                      exactMatchManagerId = 20;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC      *exactMatchManagerPtr;
    GT_U32                                                      errorNum, errorNumItr;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT          testResultArray[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                      ii;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                     entry;
    GT_U32                                                      maxTotalEntries = EXACT_MATCH_SIZE_FULL;
    GT_U32                                                      numEntriesAdded = 0;
    GT_U32                                                      numIndexesAdded = 0;  /* number of entries added to each bank;
                                                                                        if one entry is added with keysize=47 then:
                                                                                        numEntriesAdded=1 and numIndexesAdded=4 */
    GT_BOOL                                                     entryAdded;
    UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC        outputResults;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT                       exactMatchEntryType;
    GT_U32                                                      entriesToAdd = 5;
    CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC                statistics;
    GT_BOOL                                                     getFirst=GT_TRUE;

    cpssOsMemSet(&errorLocationInfo,0,sizeof(errorLocationInfo));
    indexesNum=0;
    cpssOsMemSet(indexesArray,0,sizeof(indexesArray));

    errorLocationInfo.portGroupsBmp                                                   = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    errorLocationInfo.type                                                            = CPSS_DXCH_LOCATION_LOGICAL_INDEX_INFO_TYPE;
    errorLocationInfo.info.logicalEntryInfo.numOfLogicalTables                        = 1;
    errorLocationInfo.info.logicalEntryInfo.logicaTableInfo[0].logicalTableType       = CPSS_DXCH_LOGICAL_TABLE_EXACT_MATCH_E;
    errorLocationInfo.info.logicalEntryInfo.logicaTableInfo[0].logicalTableEntryIndex = 0;
    errorLocationInfo.info.logicalEntryInfo.logicaTableInfo[0].numEntries             = 1;
    savedDataIntegrityHandler                                                         = dxChDataIntegrityEventIncrementFunc;
    dxChDataIntegrityEventIncrementFunc                                               = hwInfoEventErrorHandlerFunc;

    /* create manager with device */
    rc = createExactMatchManagerWithDevice(exactMatchManagerId,maxTotalEntries,prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "createExactMatchManagerWithDevice: expected to GT_OK on manager [%d]",
                exactMatchManagerId);
    if (rc==GT_OK)
    {
        outputResults.entryPtr                          = &entry;
        outputResults.entryAddedPtr                     = &entryAdded;

        exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);
        PRV_UTF_LOG1_MAC("start - add [%d] entries \n", entriesToAdd);

        for(ii = 0 ; ii < entriesToAdd; /*update according to entry size*/)
        {
            entryAdded = GT_FALSE;

            exactMatchEntryType = exactMatchEntryType_valid_arr[ii % UT_EXACT_MATCH_ENTRY_TYPE_VALID_NUM];

            /* Inject error for Exact Match table */
            prvCpssDxChExactMatchManagerEntryRewrite_SER_errorInject(GT_TRUE);

            switch(exactMatchEntryType)
            {
                case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
                    exactMatchManagerEntryAdd_tti(exactMatchManagerId, ii, &outputResults);
                    break;
                case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
                    exactMatchManagerEntryAdd_pcl(exactMatchManagerId, ii, &outputResults);
                    break;
                case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
                    exactMatchManagerEntryAdd_epcl(exactMatchManagerId, ii, &outputResults);
                    break;
                default:
                    break;
            }

            /* numIndexesAdded - also indicate number of events expected
               In case of keySize=19Bytes 2 entries written, so 2 events are expected */
            if(entryAdded)
            {
                numEntriesAdded++;
                numIndexesAdded+=(outputResults.entryPtr->exactMatchEntry.key.keySize+1);
                ii+=(outputResults.entryPtr->exactMatchEntry.key.keySize+1);
            }
            else
            {
                ii++;
            }
        }

        PRV_UTF_LOG3_MAC("ended - out of [%d] entries , succeeded [%d] entries (%d%%)\n",
            entriesToAdd, numIndexesAdded, (100*numIndexesAdded)/entriesToAdd);

        /* Backup the current manager content */
        rc = prvCpssDxChExactMatchManagerBackUp(exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvCpssDxChExactMatchManagerBackUp: GT_OK [%d]", exactMatchManagerId);

    #ifdef ASIC_SIMULATION
        /* Collect hwIndex(Valid) & corrupt the HW data not DB */
        for(indexesNum=0; indexesNum<numIndexesAdded; indexesNum++)
        {
            indexesArray[indexesNum] = exactMatchManagerPtr->entryPoolPtr[indexesNum].hwIndex;
            if (exactMatchManagerPtr->entryPoolPtr[indexesNum].isFirst)
            {
                PRV_UTF_LOG3_MAC("WM DB Index      :  %d - Entry Key: size=(%d) , type=(%d)\n",
                                 indexesArray[indexesNum],
                                 exactMatchManagerPtr->entryPoolPtr[indexesNum].hwExactMatchKeySize+1,
                                 exactMatchManagerPtr->entryPoolPtr[indexesNum].hwExactMatchEntryType);
            }
            else
            {
                PRV_UTF_LOG1_MAC("WM DB Index      :  %d \n",
                                 indexesArray[indexesNum]);
            }
        }

        rc = prvCpssDxChExactMatchManagerInjectHwCorruption(exactMatchManagerId, indexesArray, indexesNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvCpssDxChExactMatchManagerInjectHwCorruption: GT_OK [%d]", exactMatchManagerId);
    #else
        /* avoid warnings */
        exactMatchManagerPtr=exactMatchManagerPtr;
    #endif

        /* Verify DB - Should detect HW/SW mismatch error */
        rc = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        UTF_VERIFY_EQUAL0_PARAM_MAC(1, errorNum);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
                UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E,
                        testResultArray[errorNumItr],
                        "Expected error is not matching");
        }

         /* read all entries from the DB (one by one) and then from HW to invoke interupt */
        while(GT_OK == cpssDxChExactMatchManagerEntryGetNext(exactMatchManagerId,getFirst,&entry))/* get from DB */
        {
            getFirst = GT_FALSE;

            rc = prvCpssDxChExactMatchManagerEntryGetFromHw(exactMatchManagerId, &entry);/* get from HW */
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                         "prvCpssDxChExactMatchManagerEntryGetFromHw: expected to GT_OK on manager [%d]",
                                         exactMatchManagerId);
        }

       /* Waiting for the events to come and populate the index array */
        cpssOsTimerWkAfter(2000);

        /* AUTODOC: DISABLE : error injection to Exact Match table */
        /* DISABLE error injection */
        prvCpssDxChExactMatchManagerEntryRewrite_SER_errorInject(GT_FALSE);
        dxChDataIntegrityEventIncrementFunc = savedDataIntegrityHandler;

        rc = cpssDxChExactMatchManagerEntryRewrite(exactMatchManagerId, indexesArray, indexesNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChExactMatchManagerEntryRewrite: GT_OK [%d]", exactMatchManagerId);

        rc = cpssDxChExactMatchManagerStatisticsGet(exactMatchManagerId, &statistics);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "cpssDxChExactMatchManagerStatisticsGet: GT_OK [%d]", exactMatchManagerId);

        UTF_VERIFY_EQUAL1_STRING_MAC(1,statistics.entriesRewriteOk,
            "cpssDxChExactMatchManagerStatisticsGet: rewrite OK - invalid counter", exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(0,statistics.entriesRewriteErrorInputInvalid,
            "cpssDxChExactMatchManagerStatisticsGet: rewrite Error input invalid - invalid counter", exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(numIndexesAdded,statistics.entriesRewriteTotalRewrite,
            "cpssDxChExactMatchManagerStatisticsGet: rewrite total count - invalid counter", exactMatchManagerId);

        /* Verify Exact Match manager entries before and after rewrite entries */
        rc = prvCpssDxChExactMatchManagerBackUpInstanceVerify(exactMatchManagerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvCpssDxChExactMatchManagerBackUpInstanceVerify: GT_OK [%d]", exactMatchManagerId);

        /* Verify DB */
        rc = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
        UTF_VERIFY_EQUAL0_PARAM_MAC(0, errorNum);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                    errorNumItr,
                    testResultArray[errorNumItr],
                    debugErrorArr[testResultArray[errorNumItr]].errString);
        }
    }
    else
    {
        PRV_UTF_LOG0_MAC("Ended - Test fail , could not create manager - createExactMatchManagerWithDevice fail \n");
    }
}

UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerEntryRewrite_SER)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /* SER Test case if based on DI, and its not supported in case of AC5P and AC5X) */
    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        SKIP_TEST_MAC
    }

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    internal_cpssDxChExactMatchManagerEntryRewrite_SER();

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}

/**
GT_STATUS cpssDxChExactMacthManagerAgingScan
(
    IN  GT_U32                                                  exactMacthManagerId,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_SCAN_PARAMS_STC     *paramsPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC    entriesAgedoutArray[],
    OUT GT_U32                                                  *entriesAgedoutNumPtr
)
*/
static GT_VOID internal_cpssDxChExactMatchManagerAgingScanUT(GT_VOID)
{
    GT_STATUS                                               st;
    CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_SCAN_PARAMS_STC     scanParam;
    GT_U32                                                  exactMatchManagerId;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC    entriesAgedoutArray;
    GT_U32                                                  entriesAgedoutNum;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC   pairListArr[2];
    GT_U32                                                  numOfPairs = 1;


    pairListArr[0].devNum = prvTgfDevNum;
    pairListArr[0].portGroupsBmp = 0x1;

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(exactMatchManagerId = 0 ; exactMatchManagerId < 2; exactMatchManagerId++)
    {
        st = cpssDxChExactMatchManagerAgingScan(exactMatchManagerId, &scanParam, &entriesAgedoutArray, &entriesAgedoutNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_INITIALIZED, st);


        st = createExactMatchManager(exactMatchManagerId, EXACT_MATCH_SIZE_FULL, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "createExactMatchManager: expected to GT_OK on manager [%d]",
                                     exactMatchManagerId);

        st = cpssDxChExactMatchManagerAgingScan(exactMatchManagerId, &scanParam, NULL, &entriesAgedoutNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssDxChExactMatchManagerAgingScan(exactMatchManagerId, &scanParam, &entriesAgedoutArray, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssDxChExactMatchManagerAgingScan(exactMatchManagerId, &scanParam, NULL, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssDxChExactMatchManagerAgingScan(exactMatchManagerId, NULL, &entriesAgedoutArray, &entriesAgedoutNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /* add device to it */
        st = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId,pairListArr,numOfPairs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChExactMatchManagerDevListAdd: expected to GT_OK on manager [%d]",
                exactMatchManagerId);

        st = cpssDxChExactMatchManagerAgingScan(exactMatchManagerId, &scanParam, NULL, &entriesAgedoutNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssDxChExactMatchManagerAgingScan(exactMatchManagerId, &scanParam, &entriesAgedoutArray, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssDxChExactMatchManagerAgingScan(exactMatchManagerId, &scanParam, NULL, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = cpssDxChExactMatchManagerAgingScan(exactMatchManagerId, NULL, &entriesAgedoutArray, &entriesAgedoutNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        st = cpssDxChExactMatchManagerDevListRemove(exactMatchManagerId,pairListArr, numOfPairs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "cpssDxChExactMatchManagerDevListRemove: expected to GT_OK on manager [%d]",
                exactMatchManagerId);
    }

    st = cpssDxChExactMatchManagerAgingScan(CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS+1, &scanParam, &entriesAgedoutArray, &entriesAgedoutNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, st);
}

/**
GT_STATUS cpssDxChExactMatchManagerAgingScan
(
    IN  GT_U32                                                  exactMatchManagerId,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_SCAN_PARAMS_STC     *paramsPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC    entriesAgedoutArray[],
    OUT GT_U32                                                  *entriesAgedoutNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerAgingScan)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    internal_cpssDxChExactMatchManagerAgingScanUT();

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}

/**
GT_STATUS cpssDxChExactMatchManagerDeleteScan
(
    IN  GT_U32                                                  exactMatchManagerId,
    IN  GT_BOOL                                                 exactMatchScanStart,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_DELETE_SCAN_PARAMS_STC    *paramsPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC    entriesDeletedArray[],
    OUT GT_U32                                                  *entriesDeletedNumPtr
)
*/
static GT_VOID internal_cpssDxChExactMatchManagerDeleteScanUT(GT_VOID)
{
    GT_STATUS                                               st;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DELETE_SCAN_PARAMS_STC    scanParam;
    GT_U32                                                  exactMatchManagerId;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC    entriesDeleteArray[2];
    GT_U32                                                  entriesDeleteNum;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC   pairListArr[2];
    GT_U32                                                  numOfPairs = 1;
    GT_BOOL                                                 scanStart = GT_TRUE;
    GT_U32                                                  errorNum;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      testResultArray[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  errorNumItr;
    GT_U32                                                  ii=0;

    pairListArr[0].devNum = prvTgfDevNum;
    pairListArr[0].portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* since no manager is valid ... expect 'GT_NOT_INITIALIZED' */
    for(ii = 0 ; ii < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)
    {
        st = cpssDxChExactMatchManagerDeleteScan(ii, scanStart, &scanParam, entriesDeleteArray, &entriesDeleteNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
            "cpssDxChExactMatchManagerDeleteScan: expected to GT_NOT_INITIALIZED on manager [%d]",
            ii);
    }

    /* check out of range managers ... expect 'GT_OUT_OF_RANGE' */
    for(/*continue*/ ; ii < 100 ; ii+= 7)
    {
        st = cpssDxChExactMatchManagerDeleteScan(ii, scanStart, &scanParam, entriesDeleteArray, &entriesDeleteNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
            "cpssDxChExactMatchManagerDeleteScan: expected to GT_OUT_OF_RANGE on manager [%d]",
            ii);
    }

    for(exactMatchManagerId = 0 ; exactMatchManagerId < 2; exactMatchManagerId++)
    {
        /* create valid manager */
        st = createExactMatchManager(exactMatchManagerId, EXACT_MATCH_SIZE_FULL, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "createExactMatchManager: expected to GT_OK on manager [%d]",
                                     exactMatchManagerId);

        st = cpssDxChExactMatchManagerDeleteScan(exactMatchManagerId, scanStart, &scanParam, NULL, &entriesDeleteNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, st,
                                     "cpssDxChExactMatchManagerDeleteScan: expected to GT_OK on manager [%d]",
                                     exactMatchManagerId);

        st = cpssDxChExactMatchManagerDeleteScan(exactMatchManagerId, scanStart, &scanParam, entriesDeleteArray, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, st,
                                     "cpssDxChExactMatchManagerDeleteScan: expected to GT_OK on manager [%d]",
                                     exactMatchManagerId);

        st = cpssDxChExactMatchManagerDeleteScan(exactMatchManagerId, scanStart, &scanParam, NULL, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, st,
                                     "cpssDxChExactMatchManagerDeleteScan: expected to GT_OK on manager [%d]",
                                     exactMatchManagerId);

        st = cpssDxChExactMatchManagerDeleteScan(exactMatchManagerId, scanStart, NULL, entriesDeleteArray, &entriesDeleteNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "cpssDxChExactMatchManagerDeleteScan: expected to GT_OK on manager [%d]",
                                     exactMatchManagerId);

        /* add device to it */
        st = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId, pairListArr ,numOfPairs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChExactMatchManagerDevListAdd: expected to GT_OK on manager [%d]",
                                     exactMatchManagerId);

        st = cpssDxChExactMatchManagerDeleteScan(exactMatchManagerId, scanStart, &scanParam, NULL, &entriesDeleteNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, st,
                                     "cpssDxChExactMatchManagerDeleteScan: expected to GT_OK on manager [%d]",
                                     exactMatchManagerId);

        st = cpssDxChExactMatchManagerDeleteScan(exactMatchManagerId, scanStart, &scanParam, entriesDeleteArray, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, st,
                                     "cpssDxChExactMatchManagerDeleteScan: expected to GT_OK on manager [%d]",
                                     exactMatchManagerId);

        st = cpssDxChExactMatchManagerDeleteScan(exactMatchManagerId, scanStart, &scanParam, NULL, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_MORE, st,
                                     "cpssDxChExactMatchManagerDeleteScan: expected to GT_OK on manager [%d]",
                                     exactMatchManagerId);

        st = cpssDxChExactMatchManagerDeleteScan(exactMatchManagerId, scanStart, NULL, entriesDeleteArray, &entriesDeleteNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "cpssDxChExactMatchManagerDeleteScan: expected to GT_OK on manager [%d]",
                                     exactMatchManagerId);

        /* Verify DB */
        st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                    errorNumItr,
                    testResultArray[errorNumItr],
                    debugErrorArr[testResultArray[errorNumItr]].errString);
        }

        st = cpssDxChExactMatchManagerDevListRemove(exactMatchManagerId,pairListArr,numOfPairs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChExactMatchManagerDevListRemove: expected to GT_OK on manager [%d]",
                                     exactMatchManagerId);
        scanStart = GT_FALSE;
    }

    st = cpssDxChExactMatchManagerDeleteScan(CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS+1, scanStart, &scanParam, entriesDeleteArray, &entriesDeleteNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
                                 "cpssDxChExactMatchManagerDevListRemove: expected to GT_OK on manager [%d]",
                                 exactMatchManagerId);
}

/**
GT_STATUS cpssDxChExactMatchManagerDeleteScan
(
    IN  GT_U32                                                exactMatchManagerId,
    IN  GT_BOOL                                               scanStart,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_DELETE_SCAN_PARAMS_STC  *paramsPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC  entriesDeletedArray[],
    OUT GT_U32                                                *entriesDeletedNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerDeleteScan)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    internal_cpssDxChExactMatchManagerDeleteScanUT();

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}
void internal_cpssDxChExactMatchManagerDeleteScan_with_filter(void)
{
    GT_STATUS                                               st;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                 entry;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DELETE_SCAN_PARAMS_STC    deleteScanParam;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_EVENT_STC    entriesDeleteArray[2];
    GT_U32                                                  entriesDeleteNum;
    CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC            statistics;
    GT_U32                                                  exactMatchManagerId = 11;
    GT_U32                                                  errorNum, errorNumItr;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      testResultArray[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E];
    GT_U32                                                  rand32Bits;
    UTF_PRV_CPSS_EXACT_MATCH_API_OUTPUT_RESULTS_INFO_STC    outputResults;
    GT_BOOL                                                 entryAdded;/* indication that the entry was added to the DB */

    cpssOsMemSet(&entry ,0,sizeof(entry));
    outputResults.entryPtr          = &entry;
    outputResults.entryAddedPtr     = &entryAdded;

    /* create manager with device */
    st = createExactMatchManagerWithDevice(exactMatchManagerId,EXACT_MATCH_SIZE_FULL,prvTgfDevNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                "createExactMatchManagerWithDevice: expected to GT_OK on manager [%d]",
                exactMatchManagerId);
    if (st==GT_OK)
    {
        /* add 4 exact match entries to be scanned */

        rand32Bits=1234567890;/* random number */

        /* create a TTI entry with 33Byte key size - first lookup -  UserDefined = rand32Bits & 0xFFFF;--> 722 */
        specificExactMatchManagerEntryAdd(exactMatchManagerId,rand32Bits,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E,&outputResults);
        /* create a PCL entry with 47Byte key size - second lookup UserDefined = rand32Bits & 0xFFFF;--> 723*/
        specificExactMatchManagerEntryAdd(exactMatchManagerId,rand32Bits+1,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E,&outputResults);
        /* create a TTI entry with 5Byte key size - first lookup UserDefined = rand32Bits & 0xFFFF;--> 724*/
        specificExactMatchManagerEntryAdd(exactMatchManagerId,rand32Bits+2,CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E,&outputResults);
        /* create a PCL entry with 19Byte key size - second lookup UserDefined = rand32Bits & 0xFFFF;--> 725*/
        specificExactMatchManagerEntryAdd(exactMatchManagerId,rand32Bits+3,CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E,&outputResults);

        /* Verify DB */
        st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                    errorNumItr,
                    testResultArray[errorNumItr],
                    debugErrorArr[testResultArray[errorNumItr]].errString);
        }
        st = cpssDxChExactMatchManagerStatisticsGet(exactMatchManagerId, &statistics);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,st, "cpssDxChExactMatchManagerStatisticsGet: GT_OK");
        UTF_VERIFY_EQUAL0_STRING_MAC(4, statistics.entryAddOkRehashingStage0, "ERROR: cpssDxChExactMatchManagerStatisticsGet");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, statistics.entryAddErrorInputInvalid, "ERROR: cpssDxChExactMatchManagerStatisticsGet");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, statistics.entryAddErrorTableFull,    "ERROR: cpssDxChExactMatchManagerStatisticsGet");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, statistics.entryAddErrorBadState,     "ERROR: cpssDxChExactMatchManagerStatisticsGet");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, statistics.entryAddErrorEntryExist,   "ERROR: cpssDxChExactMatchManagerStatisticsGet");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, statistics.entryAddErrorReplayEntryNotFound,"ERROR: cpssDxChExactMatchManagerStatisticsGet");

        /* this was the first entry added */
        cpssOsMemSet(&deleteScanParam, 0, sizeof(deleteScanParam));
        deleteScanParam.deleteClientEntries[CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E]      = GT_TRUE;
        deleteScanParam.deleteKeySizeEntries[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E]   = GT_TRUE;
        deleteScanParam.deleteLookupEntries[CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E]    = GT_TRUE;
        deleteScanParam.userDefinedPattern                                           = 722;
        deleteScanParam.userDefinedMask                                              = 0xFFFF;
        cpssOsMemSet(deleteScanParam.mask,0xFF,sizeof(deleteScanParam.mask));
        cpssOsMemSet(deleteScanParam.pattern,0,sizeof(deleteScanParam.pattern));
        deleteScanParam.pattern[0]=(GT_U8)(rand32Bits >> 24);
        deleteScanParam.pattern[1]=(GT_U8)(rand32Bits >> 16);
        deleteScanParam.pattern[2]=(GT_U8)(rand32Bits >>  8);
        deleteScanParam.pattern[3]=(GT_U8)(rand32Bits >>  0);

        deleteScanParam.pattern[29]=(GT_U8)(rand32Bits >> 24);
        deleteScanParam.pattern[30]=(GT_U8)(rand32Bits >> 16);
        deleteScanParam.pattern[31]=(GT_U8)(rand32Bits >>  8);
        deleteScanParam.pattern[32]=(GT_U8)(rand32Bits >>  0);

        st = cpssDxChExactMatchManagerDeleteScan(exactMatchManagerId, GT_TRUE, &deleteScanParam, entriesDeleteArray, &entriesDeleteNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, st, "cpssDxChExactMatchManagerDeleteScan: GT_OK");

        /* Verify DB */
        st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                    errorNumItr,
                    testResultArray[errorNumItr],
                    debugErrorArr[testResultArray[errorNumItr]].errString);
        }

        st = cpssDxChExactMatchManagerStatisticsGet(exactMatchManagerId, &statistics);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,st, "cpssDxChExactMatchManagerStatisticsGet: GT_OK");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, statistics.scanDeleteOk,                    "ERROR: cpssDxChExactMatchManagerStatisticsGet");
        UTF_VERIFY_EQUAL0_STRING_MAC(1, statistics.scanDeleteOkNoMore,              "ERROR: cpssDxChExactMatchManagerStatisticsGet");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, statistics.scanDeleteErrorInputInvalid,     "ERROR: cpssDxChExactMatchManagerStatisticsGet");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, statistics.scanDeleteErrorInputInvalid,     "ERROR: cpssDxChExactMatchManagerStatisticsGet");
        UTF_VERIFY_EQUAL0_STRING_MAC(1, statistics.scanDeleteTotalDeletedEntries,   "ERROR: cpssDxChExactMatchManagerStatisticsGet");

        /* this was the last entry added */
        cpssOsMemSet(&deleteScanParam, 0, sizeof(deleteScanParam));
        deleteScanParam.deleteClientEntries[CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E]      = GT_TRUE;
        deleteScanParam.deleteKeySizeEntries[CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E]   = GT_TRUE;
        deleteScanParam.deleteLookupEntries[CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E]   = GT_TRUE;
        deleteScanParam.userDefinedPattern                                           = 725;
        deleteScanParam.userDefinedMask                                              = 0xFFFF;
        cpssOsMemSet(deleteScanParam.mask,0xFF,sizeof(deleteScanParam.mask));
        cpssOsMemSet(deleteScanParam.pattern,0,sizeof(deleteScanParam.pattern));
        deleteScanParam.pattern[0]=(GT_U8)((rand32Bits+3) >> 24);
        deleteScanParam.pattern[1]=(GT_U8)((rand32Bits+3) >> 16);
        deleteScanParam.pattern[2]=(GT_U8)((rand32Bits+3) >>  8);
        deleteScanParam.pattern[3]=(GT_U8)((rand32Bits+3) >>  0);

        deleteScanParam.pattern[15]=(GT_U8)((rand32Bits+3) >> 24);
        deleteScanParam.pattern[16]=(GT_U8)((rand32Bits+3) >> 16);
        deleteScanParam.pattern[17]=(GT_U8)((rand32Bits+3) >>  8);
        deleteScanParam.pattern[18]=(GT_U8)((rand32Bits+3) >>  0);

        st = cpssDxChExactMatchManagerDeleteScan(exactMatchManagerId, GT_TRUE, &deleteScanParam, entriesDeleteArray, &entriesDeleteNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, st, "cpssDxChExactMatchManagerDeleteScan: GT_OK");

        /* Verify DB */
        st = cpssDxChExactMatchManagerDatabaseCheck(exactMatchManagerId, &dbCheckParam, testResultArray, &errorNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(errorNum, 0);
        for(errorNumItr = 0; errorNumItr<errorNum; errorNumItr++)
        {
            PRV_UTF_LOG3_MAC("testResult Error[Itr - %d] -[Err_No - %d]-[%s]\n",
                    errorNumItr,
                    testResultArray[errorNumItr],
                    debugErrorArr[testResultArray[errorNumItr]].errString);
        }

        st = cpssDxChExactMatchManagerStatisticsGet(exactMatchManagerId, &statistics);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,st, "cpssDxChExactMatchManagerStatisticsGet: GT_OK");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, statistics.scanDeleteOk,                    "ERROR: cpssDxChExactMatchManagerStatisticsGet");
        UTF_VERIFY_EQUAL0_STRING_MAC(2, statistics.scanDeleteOkNoMore,              "ERROR: cpssDxChExactMatchManagerStatisticsGet");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, statistics.scanDeleteErrorInputInvalid,     "ERROR: cpssDxChExactMatchManagerStatisticsGet");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, statistics.scanDeleteErrorInputInvalid,     "ERROR: cpssDxChExactMatchManagerStatisticsGet");
        UTF_VERIFY_EQUAL0_STRING_MAC(2, statistics.scanDeleteTotalDeletedEntries,   "ERROR: cpssDxChExactMatchManagerStatisticsGet");
    }
    else
    {
        PRV_UTF_LOG0_MAC("Ended - Test fail , could not create manager - cpssDxChExactMatchManagerDeleteScan_with_filter fail \n");
    }
    return;
}

UTF_TEST_CASE_MAC(cpssDxChExactMatchManagerDeleteScan_with_filter)
{
    /* this is device less API , check if we allow it to run on current device */
    DEVICE_LESS_API_SKIP_NON_APPLICABLE_DEVICE;

    /********************************************************/
    /* bug in 'osMalloc'  !!! see JIRA : CPSS-9233 :        */
    /* osMalloc return NULL size in range 0x10000 - 0x200000*/
    /* after 206 iterations - although first 205 freed      */
    /********************************************************/
    BIND_MALLOC_AND_FREE;

    internal_cpssDxChExactMatchManagerDeleteScan_with_filter();

    CLEANUP_ALL_MANAGERS;

    RESTORE_OS_MALLOC_AND_OS_FREE;
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChExactMatchManagerUt suit
   !!! the Exact Match manager tests guaranty to cleanup configurations !!!
*/
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChExactMatchManager)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerDevListAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerDevListRemove)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerEntryAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerEntryDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerEntryGetNext)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerEntryUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerExpandedActionUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerExpandedActionGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerDevListGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerCountersGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerStatisticsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerDatabaseCheck)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerEntryRewrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerAgingScan)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerDeleteScan)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerEntryAddCuckooCapacityIncremental)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerEntryAddCuckooCapacityRandom)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerEntryAddCuckooCapacityIncrementalPreformance)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerEntryAddCuckooCapacityRandomPreformance)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerEntryAddDeleteAddCuckooCapacityIncremental)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerEntryAddCuckooSameHash)

    /* extra permutations of : cpssDxChExactMatchManagerEntryAdd test */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerEntryAdd_run_device_less)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerEntryAdd_add_device_in_runtime)

    /* extra permutations of cpssDxChExactMatchManagerEntryUpdate test */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerEntryUpdate_run_device_less)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerEntryUpdate_add_device_in_runtime)

    /* Test for SER functionality */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerEntryRewrite_SER)

    /* extra permutations of cpssDxChExactMatchManagerDeleteScan test */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChExactMatchManagerDeleteScan_with_filter)

UTF_SUIT_END_TESTS_MAC(cpssDxChExactMatchManager)

