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
* @file prvCpssDxChExactMatchManager_utils.h
*
* @brief Exact Match manager support.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChExactMatchManager_utils_h
#define __prvCpssDxChExactMatchManager_utils_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/exactMatchManager/cpssDxChExactMatchManager.h>

/* global variables macros */

#define PRV_SHARED_EMM_DIR_EMM_DB_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxchEmMgrDir.emMgrDbSrc._var)

/* get pointer to the Exact Match manager */
#define EM_MANAGER_GET_MAC(_exactMatchManagerId)  PRV_SHARED_EMM_DIR_EMM_DB_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDbArr)[_exactMatchManagerId]

/* get Exact Match manager Id from Excat Match pointer */
#define EM_MANAGER_ID_GET_MAC(_exactMatchManagerPtr,_exactMatchManagerId)       \
{                                                                               \
    _exactMatchManagerId = CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS;  \
    for (ii=0; ii<CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS; ii++)     \
    {                                                                           \
        if (PRV_SHARED_EMM_DIR_EMM_DB_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDbArr)[ii] == exactMatchManagerPtr)      \
        {                                                                       \
            _exactMatchManagerId =  ii;                                         \
            break;                                                              \
        }                                                                       \
    }                                                                           \
}

/* macro to check and return error when exactMatchManagerId is not valid */
#define EM_MANAGER_ID_CHECK(_exactMatchManagerId)                   \
    if((_exactMatchManagerId) >= CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS)     \
    {                                                               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,                 \
            "exactMatchManagerId [%d] >= [%d] (out of range)",      \
            (_exactMatchManagerId),                                 \
            CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS);    \
    }                                                               \
    else if(NULL == PRV_SHARED_EMM_DIR_EMM_DB_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDbArr)[_exactMatchManagerId])      \
    {                                                               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,           \
            "exactMatchManagerId [%d] was not initialized ",        \
            (_exactMatchManagerId));                                \
    }

#define EM_MANAGER_AGE_BIN_ID_CHECK(_exactMatchAgeBinId)            \
    if((_exactMatchAgeBinId) >= PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_AGE_BIN_ID_CNS)  \
    {                                                               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,                 \
            "exactMatchAgeBinId [%d] >= [%d] (out of range)",       \
            (_exactMatchAgeBinId),                                  \
            PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_AGE_BIN_ID_CNS);  \
    }

#define EM_MANAGER_HW_INDEX_CHECK(_hwIndex, _exactMatchManagerPtr)                  \
    if((_hwIndex) >= _exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes) \
    {                                                                               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,                                 \
            "hwIndex [%d] >= [%d] (out of range)",                                  \
            (_hwIndex),                                                             \
            _exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);         \
    }

#define EM_MUTEX_OF_CPSS_DEV   0/*PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS*/

/*Lock the access to per device data base in order to avoid corruption*/
/* this macro do:
    1. lock
    2. check the device exists and is DXCH device --> return ERROR if not valid
*/
#define EM_LOCK_DEV_NUM(_devNum)    \
        PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(_devNum,EM_MUTEX_OF_CPSS_DEV)

/*Unlock the access to per device data base */
#define EM_UNLOCK_DEV_NUM(_devNum)    \
            CPSS_API_UNLOCK_MAC(_devNum,EM_MUTEX_OF_CPSS_DEV);

/*Lock the access to per device data base in order to avoid corruption*/
/* this macro do:
    1. lock
    2. check the device exists and is DXCH device -->
        a. set isValid = 0 if not valid
        a. set isValid = 1 if     valid
*/
#define EM_LOCK_DEV_NUM_SET_IS_VALID(_devNum,isValid)    \
        CPSS_API_LOCK_MAC(_devNum,EM_MUTEX_OF_CPSS_DEV); \
        isValid = (PRV_CPSS_IS_DEV_EXISTS_MAC(_devNum) && PRV_CPSS_DXCH_FAMILY_CHECK_MAC(_devNum)) ? 1 : 0



/* macro to check that x > y is GT_BAD_PARAM */
#define EM_CHECK_X_NOT_MORE_THAN_Y_MAC(x,y)             \
    if((GT_U32)x > (GT_U32)y)                           \
    {                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,     \
            "[%s][%d] > [%s][%d] ",                     \
            #x,x,#y,y);                                 \
    }

/* macro to check that x >= y is GT_BAD_PARAM */
#define EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(x,y)    \
    if((GT_U32)x >= (GT_U32)y)                          \
    {                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,     \
            "[%s][%d] >= [%s][%d] ",                    \
            #x,x,#y,y);                                 \
    }

/* macro to check that x != y is GT_BAD_STATE */
#define EM_CHECK_X_EQUAL_Y_MAC(x,y)                     \
    if((GT_U32)x != (GT_U32)y)                          \
    {                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,     \
            "[%s][%d] != [%s][%d] (expected equal)",    \
            #x,x,#y,y);                                 \
    }


/* macro to check that x == 0 is GT_BAD_STATE */
#define EM_CHECK_X_NOT_ZERO_MAC(x)                      \
    if((x) == 0)                                        \
    {                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,     \
            "[%s] must not be ZERO",                    \
            #x);                                        \
    }


/* macro for 'bookmark' of not implemented values */
#define EM_VALUE_FOR_PARAM_NOT_IMPLEMENTED_MAC(param)    \
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,    \
        "[%s] not implemented for value [%d] ",          \
        #param,                                          \
        param)

/* set 'dev' bit in the bmp */
#define EM_DEV_BMP_SET_MAC(devsBitmap, devNum)   \
    EM_BMP_SET_MAC(devsBitmap, devNum)

/* clear 'dev' bit in the bmp */
#define EM_DEV_BMP_CLEAR_MAC(devsBitmap, devNum)   \
    EM_BMP_CLEAR_MAC(devsBitmap, devNum)

/* is 'dev' bit set in bmp ? */
#define EM_IS_DEV_BMP_SET_MAC(devsBitmap, devNum)   \
    EM_IS_BMP_SET_MAC(devsBitmap, devNum)

/* handle PROFILE_ID_MAPPING_ENTRIES bitmap */
/* set 'profileId mapping' bit in the bmp */
#define EM_PROFILE_ID_MAPPING_BMP_SET_MAC(_profileIdMappingBitmap, _entryIndex)   \
    EM_BMP_SET_MAC(_profileIdMappingBitmap, _entryIndex)

/* clear 'profileId mapping' bit in the bmp */
#define EM_PROFILE_ID_MAPPING_BMP_CLEAR_MAC(_profileIdMappingBitmap, _entryIndex)   \
    EM_BMP_CLEAR_MAC(_profileIdMappingBitmap, _entryIndex)

/* is 'profileId mapping' bit set in bmp ? */
#define EM_IS_PROFILE_ID_MAPPING_BMP_SET_MAC(_profileIdMappingBitmap, _entryIndex)   \
    EM_IS_BMP_SET_MAC(_profileIdMappingBitmap, _entryIndex)

/* set bit in the bmp */
#define EM_BMP_SET_MAC(bmp, index)   \
    (bmp)[(index)>>5] |= 1 << ((index) & 0x1f)

/* clear bit in the bmp */
#define EM_BMP_CLEAR_MAC(bmp, index)   \
    (bmp)[(index)>>5] &= ~(1 << ((index) & 0x1f))

/* is bit set in bmp ? */
#define EM_IS_BMP_SET_MAC(bmp, index)   \
    (((bmp)[(index)>>5] & (1 << ((index) & 0x1f))) ? 1 : 0)

/* check if 2 BMPs hold any overlapping info */
/* both must be of the same size             */
/* bmp1 must be 'array' with valid 'sizeof(bmp1)'   */
#define EM_ARE_2_BMPS_OVERLAP_MAC(bmp1,bmp2,areOverlap)\
    {                                                  \
        GT_U32  _ii;                                   \
        GT_U32  _iiMax = sizeof(bmp1) / 4;             \
        areOverlap = 0;                                \
        for(_ii = 0 ; _ii < _iiMax ; _ii++)            \
        {                                              \
            if(bmp1[_ii] & bmp2[_ii])                  \
            {                                          \
                areOverlap = 1;                        \
                break;                                 \
            }                                          \
        }                                              \
    }

/*
 * Macro to increment the age-bin counter by 1
 * and update the age-bin usage matrix if required
 *     level1UsageBitmap = 1 bit indicates 1  age-bin
 *     level2UsageBitmap = 1 bit indicates 32 age-bin(or 32 bits of level1)
 */
#define EM_INCREASE_AGE_BIN_COUNTER_MAC(_exactMatchManagerPtr, _ageBinIndex)                                                                \
{                                                                                                                                           \
    _exactMatchManagerPtr->agingBinInfo.ageBinListPtr[_ageBinIndex].totalUsedEntriesCnt += 1;                                               \
    if(_exactMatchManagerPtr->agingBinInfo.ageBinListPtr[_ageBinIndex].totalUsedEntriesCnt == _exactMatchManagerPtr->capacityInfo.maxEntriesPerAgingScan) \
    {                                                                                                                                       \
        _exactMatchManagerPtr->agingBinInfo.level1UsageBitmapPtr[_ageBinIndex>>5] |= (1 << (_ageBinIndex & 0x1F));                          \
        if(_exactMatchManagerPtr->agingBinInfo.level1UsageBitmapPtr[_ageBinIndex>>5] == 0xFFFFFFFF)                                         \
        {                                                                                                                                   \
            _exactMatchManagerPtr->agingBinInfo.level2UsageBitmap |= ( 1 << (_ageBinIndex >> 5));                                           \
        }                                                                                                                                   \
    }                                                                                                                                       \
}

/*
 * Macro to decrement the age-bin counter by 1
 * and update the age-bin usage matrix if required
 *     level1UsageBitmap = 1 bit indicates 1  age-bin
 *     level2UsageBitmap = 1 bit indicates 32 age-bin(or 32 bits of level1)
 */
#define EM_DECREASE_AGE_BIN_COUNTER_MAC(_exactMatchManagerPtr, _ageBinIndex)                                                                \
{                                                                                                                                           \
    _exactMatchManagerPtr->agingBinInfo.ageBinListPtr[_ageBinIndex].totalUsedEntriesCnt -= 1;                                               \
    if(_exactMatchManagerPtr->agingBinInfo.ageBinListPtr[_ageBinIndex].totalUsedEntriesCnt == (_exactMatchManagerPtr->capacityInfo.maxEntriesPerAgingScan -1)) \
    {                                                                                                                                       \
        _exactMatchManagerPtr->agingBinInfo.level1UsageBitmapPtr[_ageBinIndex>>5] &= ~(1 << (_ageBinIndex & 0x1F));                         \
        _exactMatchManagerPtr->agingBinInfo.level2UsageBitmap &= ~( 1 << (_ageBinIndex >> 5));                                              \
    }                                                                                                                                       \
}

/* macro to convert Exact Match key enum to size in bytes */
#define EM_CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(_keyEnum,_sizeInBytes)                \
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
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(_keyEnum);                            \
    }

/* macro to convert Exact Match key size in bytes to enum */
#define EM_CONV_SIZE_IN_BYTES_TO_KEY_ENUM_MAC(_sizeInBytes,_keyEnum)                \
    switch (_sizeInBytes)                                                           \
    {                                                                               \
    case 5:                                                                         \
        _keyEnum = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;                             \
        break;                                                                      \
    case 19:                                                                        \
        _keyEnum = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;                            \
        break;                                                                      \
    case 33:                                                                        \
        _keyEnum = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;                            \
        break;                                                                      \
    case 47:                                                                        \
        _keyEnum = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;                            \
        break;                                                                      \
    default:                                                                        \
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(_sizeInBytes);                        \
    }

/* macro to convert key size in bytes to bank step */
#define EM_CONV_KEY_SIZE_IN_BYTES_TO_BANK_STEP_MAC(_sizeInBytes,_bankStep)          \
    switch (_sizeInBytes)                                                           \
    {                                                                               \
    case 5:                                                                         \
        _bankStep = 1;                                                              \
        break;                                                                      \
    case 19:                                                                        \
        _bankStep = 2;                                                              \
        break;                                                                      \
    case 33:                                                                        \
        _bankStep = 3;                                                              \
        break;                                                                      \
    case 47:                                                                        \
        _bankStep = 4;                                                              \
        break;                                                                      \
    default:                                                                        \
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(_sizeInBytes);                        \
    }

/* macro to convert from {bank,line} to hw index */
#define EM_HW_INDEX_GET_MAC(_numOfHashes,_bank,_line,_hwIndex)                      \
{                                                                                   \
    GT_U32 _shift=0;                                                                \
    switch (_numOfHashes)                                                           \
    {                                                                               \
    case 4:                                                                         \
        _shift = 2;                                                                 \
        break;                                                                      \
    case 8:                                                                         \
        _shift = 3;                                                                 \
        break;                                                                      \
    case 16:                                                                        \
        _shift = 4;                                                                 \
        break;                                                                      \
    default:                                                                        \
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(_numOfHashes);                        \
    }                                                                               \
    _hwIndex =  (_line <<_shift) | _bank;                                           \
}

/* macro to convert from hw index to line */
#define EM_LINE_FROM_HW_INDEX_GET_MAC(_numOfHashes,_hwIndex,_line)                  \
{                                                                                   \
    GT_U32 _shift=0;                                                                \
    switch (_numOfHashes)                                                           \
    {                                                                               \
    case 4:                                                                         \
        _shift = 2;                                                                 \
        break;                                                                      \
    case 8:                                                                         \
        _shift = 3;                                                                 \
        break;                                                                      \
    case 16:                                                                        \
        _shift = 4;                                                                 \
        break;                                                                      \
    default:                                                                        \
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(_numOfHashes);                        \
    }                                                                               \
    _line = (_hwIndex >> _shift);                                                   \
}

/* macro to convert from hw index to bank */
#define EM_BANK_FROM_HW_INDEX_GET_MAC(_numOfHashes,_hwIndex,_bank)                  \
{                                                                                   \
    GT_U32 _mask=0;                                                                 \
    switch (_numOfHashes)                                                           \
    {                                                                               \
    case 4:                                                                         \
        _mask = 3;                                                                  \
        break;                                                                      \
    case 8:                                                                         \
        _mask = 7;                                                                  \
        break;                                                                      \
    case 16:                                                                        \
        _mask = 15;                                                                 \
        break;                                                                      \
    default:                                                                        \
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(_numOfHashes);                        \
    }                                                                               \
    _bank = (_hwIndex & _mask);                                                     \
}

/*
 * @struct PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_BANK_INFO_STC
 *
 * @brief internal Exact Match manager DB (database) 'per bank' info
 *
*/
typedef struct{
    /** number of entries in the bank (counter) */
    GT_U32              bankPopulation;
}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_BANK_INFO_STC;

/*
 * @struct PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_TYPE_COUNTERS_STC
 *
 * @brief internal Exact Match manager DB (database) 'per entry type' counters
 *
*/
typedef struct{
    /** number of entries of the entry type */
    GT_U32              entryTypePopulation;
}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_TYPE_COUNTERS_STC;


/* statistics about the 'entry add'  API cpssDxChExactMatchManagerEntryAdd(...) */
typedef enum{
     PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_INPUT_INVALID_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_BAD_STATE_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_ENTRY_EXIST_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TABLE_FULL_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_HW_UPDATE_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_REPLAY_ENTRY_NOT_FOUND_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_OTHER_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_0_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_1_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_2_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_3_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_4_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_5_AND_ABOVE_E

    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS___LAST___E
}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ENT;

/* statistics about the 'entry delete'  API cpssDxChExactMatchManagerEntryDelete(...) */
typedef enum{
     PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_INPUT_INVALID_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_BAD_STATE_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_ENTRY_NOT_FOUND_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_HW_UPDATE_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_OK_E

    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS___LAST___E
}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_ENT;

/* statistics about the 'entry update'  API cpssDxChExactMatchManagerEntryUpdate(...) */
typedef enum{
     PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_INPUT_INVALID_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_BAD_STATE_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_NOT_FOUND_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_NOT_ALLOWED_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_HW_UPDATE_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_OK_E

    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS___LAST___E
}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_ENT;

/* statistics about the 'Delete Scan'  API cpssDxChExactMatchManagerDeleteScan(...) */
typedef enum{
     PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_DELETE_API_STATISTICS_OK_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_DELETE_API_STATISTICS_OK_NO_MORE_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_DELETE_API_STATISTICS_ERROR_INPUT_INVALID_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_DELETE_API_STATISTICS_TOTAL_DELETED_E

    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_DELETE_API_STATISTICS___LAST___E
}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_DELETE_API_STATISTICS_ENT;

/* statistics about the 'Age Scan'  API cpssDxChExactMatchManagerAgingScan(...) */
typedef enum{
     PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_AGE_API_STATISTICS_OK_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_AGE_API_STATISTICS_ERROR_INPUT_INVALID_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_DELETED_E

    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_AGE_API_STATISTICS___LAST___E
}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_AGE_API_STATISTICS_ENT;


/* statistics about the 'Entries Rewrite'  API cpssDxChExactMatchManagerEntryRewrite(...) */
typedef enum{
     PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRIES_REWRITE_API_STATISTICS_OK_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRIES_REWRITE_API_STATISTICS_ERROR_INPUT_INVALID_E
    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRIES_REWRITE_API_STATISTICS_TOTAL_REWRITE_E

    ,PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRIES_REWRITE_API_STATISTICS___LAST___E
}PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRIES_REWRITE_API_STATISTICS_ENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChExactMatchManager_utils_h */


