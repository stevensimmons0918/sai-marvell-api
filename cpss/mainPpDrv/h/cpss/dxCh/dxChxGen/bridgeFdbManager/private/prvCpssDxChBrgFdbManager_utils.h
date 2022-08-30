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
* @file prvCpssDxChBrgFdbManager_utils.h
*
* @brief FDB manager support.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChBrgFdbManager_utils_h
#define __prvCpssDxChBrgFdbManager_utils_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/bridgeFdbManager/cpssDxChBrgFdbManager.h>
#include <cpssCommon/private/prvCpssMisc.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTablesSip6.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>

extern const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip6FdbFdbTableFieldsFormat[SIP6_FDB_FDB_TABLE_FIELDS___LAST_VALUE___E];
extern const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5FdbFdbTableFieldsFormat[SIP5_10_FDB_FDB_TABLE_FIELDS___LAST_VALUE___E];
extern const PRV_CPSS_ENTRY_FORMAT_TABLE_STC prvCpssDxChSip5_20FdbFdbTableFieldsFormat[SIP5_10_FDB_FDB_TABLE_FIELDS___LAST_VALUE___E];

/*global variables macros*/


/* macro to set value to field of (FDB) FDB entry format in buffer */
/* based on : SIP6_FDB_ENTRY_FIELD_SET_MAC */
#define SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(_hwDataArr,_fieldName,_value)      \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                                    \
        prvCpssDxChSip6FdbFdbTableFieldsFormat,                            \
        _fieldName,                                                        \
        _value)


/* macro to get field value of (FDB) FDB entry format in buffer */
/* based on : SIP6_FDB_ENTRY_FIELD_GET_MAC */
#define SIP6_FDB_HW_ENTRY_FIELD_GET_MAC(_hwDataArr,_fieldName,_value)      \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                                    \
        prvCpssDxChSip6FdbFdbTableFieldsFormat,                            \
        _fieldName,                                                        \
        _value)

/* macro to get field value of (FDB) FDB entry format in buffer */
#define SIP5_20_FDB_HW_ENTRY_FIELD_GET_MAC(_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        prvCpssDxChSip5_20FdbFdbTableFieldsFormat,                  \
        _fieldName,                                                 \
        _value)

/* macro to set value to field of (FDB) FDB entry format in buffer */
#define SIP5_20_FDB_HW_ENTRY_FIELD_SET_MAC(_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        prvCpssDxChSip5_20FdbFdbTableFieldsFormat,                  \
        _fieldName,                                                 \
        _value)

/* macro to get field value of (FDB) FDB entry format in buffer */
#define SIP5_FDB_HW_ENTRY_FIELD_GET_MAC(_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        prvCpssDxChSip5FdbFdbTableFieldsFormat,                     \
        _fieldName,                                                 \
        _value)

/* macro to set value to field of (FDB) FDB entry format in buffer */
#define SIP5_FDB_HW_ENTRY_FIELD_SET_MAC(_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        prvCpssDxChSip5FdbFdbTableFieldsFormat,                     \
        _fieldName,                                                 \
        _value)

/* based on MAC_ADDR_SET_FIELD_BY_ID_MAC */
#define FDB_HW_MAC_ADDR_SET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,lowWord,highWord)   \
{                                                                             \
    GT_U32    _wordsArr[2];                                                   \
                                                                              \
    _wordsArr[0] =  lowWord;                                                  \
    _wordsArr[1] =  highWord;                                                 \
                                                                              \
    U_ANY_SET_FIELD_BY_ID_MAC(startEntryPtr,fieldsInfoArr,fieldId,_wordsArr); \
}

/* macro to set value to MAC ADDR field of (FDB) FDB entry format in buffer */
/* based on : SIP6_FDB_ENTRY_FIELD_MAC_ADDR_SET_MAC */
#define SIP6_FDB_HW_ENTRY_FIELD_MAC_ADDR_SET_MAC(_hwDataArr,lowWord,highWord)  \
    FDB_HW_MAC_ADDR_SET_FIELD_BY_ID_MAC(_hwDataArr,                    \
        prvCpssDxChSip6FdbFdbTableFieldsFormat,                             \
        SIP6_FDB_FDB_TABLE_FIELDS_MAC_ADDR_E,                               \
        lowWord,highWord)

/* macro to get value to MAC ADDR field of (FDB) FDB entry format in buffer */
/* based on : SIP6_FDB_ENTRY_FIELD_MAC_ADDR_GET_MAC */
#define SIP6_FDB_HW_ENTRY_FIELD_MAC_ADDR_GET_MAC(_hwDataArr,_macAddrBytes)  \
    MAC_ADDR_GET_FIELD_BY_ID_MAC(_hwDataArr,                                \
        prvCpssDxChSip6FdbFdbTableFieldsFormat,                             \
        SIP6_FDB_FDB_TABLE_FIELDS_MAC_ADDR_E,                               \
        _macAddrBytes)

/* macro to set value to MAC ADDR field of (FDB) FDB entry format in buffer */
#define SIP5_FDB_HW_ENTRY_FIELD_MAC_ADDR_SET_MAC(_hwDataArr,lowWord,highWord)   \
    FDB_HW_MAC_ADDR_SET_FIELD_BY_ID_MAC(_hwDataArr,                         \
        prvCpssDxChSip5FdbFdbTableFieldsFormat,                             \
        SIP5_FDB_FDB_TABLE_FIELDS_MAC_ADDR_E,                               \
        lowWord,highWord)

/* macro to get value to MAC ADDR field of (FDB) FDB entry format in buffer */
#define SIP5_FDB_HW_ENTRY_FIELD_MAC_ADDR_GET_MAC(_hwDataArr,_macAddrBytes)   \
    MAC_ADDR_GET_FIELD_BY_ID_MAC(_hwDataArr,                           \
        prvCpssDxChSip5FdbFdbTableFieldsFormat,                  \
        SIP5_FDB_FDB_TABLE_FIELDS_MAC_ADDR_E,                    \
        _macAddrBytes)

/* macro to set value to DIP field of (FDB) FDB entry format in buffer */
#define SIP6_FDB_HW_ENTRY_FIELD_DIP_ADDR_SET_MAC(_hwDataArr, hwValue)       \
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(_hwDataArr,                             \
        SIP6_FDB_FDB_TABLE_FIELDS_DIP_E,                                    \
        hwValue)

/* macro to set value to SIP field of (FDB) FDB entry format in buffer */
#define SIP6_FDB_HW_ENTRY_FIELD_SIP_ADDR_SET_MAC(_hwDataArr, hwValue)       \
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(_hwDataArr,                             \
        SIP6_FDB_FDB_TABLE_FIELDS_SIP_26_0_E,                               \
        U32_GET_FIELD_MAC(hwValue,0,27));                                   \
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(_hwDataArr,                             \
        SIP6_FDB_FDB_TABLE_FIELDS_SIP_30_27_E,                              \
        U32_GET_FIELD_MAC(hwValue,27,4));                                   \
    SIP6_FDB_HW_ENTRY_FIELD_SET_MAC(_hwDataArr,                             \
        SIP6_FDB_FDB_TABLE_FIELDS_SIP_31_E,                                 \
        U32_GET_FIELD_MAC(hwValue,31,1))

/* Converts packet command to hardware value */
#define PRV_CPSS_DXCH_FDB_CONVERT_ATTRIBUTE_MODIFY_TO_HW_VAL_MAC(_val, _cmd)       \
    switch (_cmd)                                                           \
    {                                                                       \
        case CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E: _val = 0; break; \
        case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:        _val = 1; break; \
        case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:       _val = 2; break; \
        default:                                                            \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_ONE_PARAM_FORMAT_MAC(_cmd)); \
    }

/* Converts  hardware value to packet command */
#define PRV_CPSS_DXCH_FDB_CONVERT_HW_VAL_TO_ATTRIBUTE_MODIFY_MAC(_cmd, _val)       \
    switch (_val)                                                           \
    {                                                                       \
        case 0: _cmd = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E; break; \
        case 1: _cmd = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;        break; \
        case 2: _cmd = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;       break; \
        default:                                                            \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_ONE_PARAM_FORMAT_MAC(_cmd)); \
    }

/* Converts  hardware value to packet command */
#define PRV_CPSS_DXCH_FDB_CONVERT_HW_VAL_TO_SW_CMD_MAC(_cmd, _val)      \
    switch (_val)                                                       \
    {                                                                   \
        case 0:                                                         \
            _cmd = CPSS_PACKET_CMD_FORWARD_E;                           \
            break;                                                      \
        case 1:                                                         \
            _cmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;                     \
            break;                                                      \
        case 2:                                                         \
            _cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;                       \
            break;                                                      \
        case 3:                                                         \
            _cmd = CPSS_PACKET_CMD_DROP_HARD_E;                         \
            break;                                                      \
        case 4:                                                         \
            _cmd = CPSS_PACKET_CMD_DROP_SOFT_E;                         \
            break;                                                      \
        default:                                                        \
            _cmd = (_val);                                              \
            break;                                                      \
    }

#define FDB_MANAGER_AGE_BIN_ID_CHECK(_ageBinId)                     \
    if((_ageBinId) >= PRV_CPSS_DXCH_BRG_FDB_MANAGER_MAX_AGE_BIN_ID_CNS)  \
    {                                                               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,                 \
            "ageBinId [%d] >= [%d] (out of range)",                 \
            (_ageBinId),                                            \
            PRV_CPSS_DXCH_BRG_FDB_MANAGER_MAX_AGE_BIN_ID_CNS);      \
    }

#define FDB_MANAGER_HW_INDEX_CHECK(_hwIndex, _fdbManagerPtr)        \
    if((_hwIndex) >= _fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes) \
    {                                                               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,                 \
            "hwIndex [%d] >= [%d] (out of range)",                  \
            (_hwIndex),                                             \
            _fdbManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes); \
    }

#define MUTEX_OF_CPSS_DEV   0/*PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS*/

/*Lock the access to per device data base in order to avoid corruption*/
/* this macro do:
    1. lock
    2. check the device exists and is DXCH device --> return ERROR if not valid
*/
#define LOCK_DEV_NUM(_devNum)    \
        PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(_devNum,MUTEX_OF_CPSS_DEV)

/*Unlock the access to per device data base */
#define UNLOCK_DEV_NUM(_devNum)    \
            CPSS_API_UNLOCK_NO_CHECKS_MAC(_devNum,MUTEX_OF_CPSS_DEV);

/*Lock the access to per device data base in order to avoid corruption*/
/* this macro do:
    1. lock
    2. check the device exists and is DXCH device -->
        a. set isValid = 0 if not valid
        a. set isValid = 1 if     valid
*/
#define LOCK_DEV_NUM_SET_IS_VALID(_devNum,isValid)    \
        CPSS_API_LOCK_MAC(_devNum,MUTEX_OF_CPSS_DEV);  \
        isValid = (PRV_CPSS_IS_DEV_EXISTS_MAC(_devNum) && PRV_CPSS_DXCH_FAMILY_CHECK_MAC(_devNum)) ? 1 : 0



/* macro to check that x > y is GT_BAD_PARAM */
#define CHECK_X_NOT_MORE_THAN_Y_MAC(x,y)                \
    if((GT_U32)x > (GT_U32)y)                           \
    {                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,     \
            "[%s][%d] > [%s][%d] ",                     \
            #x,x,#y,y);                                 \
    }

/* macro to check that x >= y is GT_BAD_PARAM */
#define CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(x,y)       \
    if((GT_U32)x >= (GT_U32)y)                          \
    {                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,     \
            "[%s][%d] >= [%s][%d] ",                    \
            #x,x,#y,y);                                 \
    }

/* macro to check that x != y is GT_BAD_STATE */
#define CHECK_X_EQUAL_Y_MAC(x,y)                        \
    if((GT_U32)x != (GT_U32)y)                          \
    {                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,     \
            "[%s][%d] != [%s][%d] (expected equal)",    \
            #x,x,#y,y);                                 \
    }


/* macro to check that x == 0 is GT_BAD_STATE */
#define CHECK_X_NOT_ZERO_MAC(x)                         \
    if((x) == 0)                                        \
    {                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,     \
            "[%s] must not be ZERO",                    \
            #x);                                        \
    }

/* before decrement do coherency check -- can't be ZERO because we are decrementing it */
#define DECREMENT_PARAM_MAC(param) \
    CHECK_X_NOT_ZERO_MAC(param);   \
    (param)--

/* macro for 'bookmark' of not implemented values */
#define VALUE_FOR_PARAM_NOT_IMPLEMENTED_MAC(param)    \
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,       \
        "[%s] not implemented for value [%d] ",             \
        #param,                                             \
        param)

/* set 'dev' bit in the bmp */
#define DEV_BMP_SET_MAC(devsBitmap, devNum)   \
    BMP_SET_MAC(devsBitmap, devNum)

/* clear 'dev' bit in the bmp */
#define DEV_BMP_CLEAR_MAC(devsBitmap, devNum)   \
    BMP_CLEAR_MAC(devsBitmap, devNum)

/* is 'dev' bit set in bmp ? */
#define IS_DEV_BMP_SET_MAC(devsBitmap, devNum)   \
    IS_BMP_SET_MAC(devsBitmap, devNum)

/* set bit in the bmp */
#define BMP_SET_MAC(bmp, index)   \
    (bmp)[(index)>>5] |= 1 << ((index) & 0x1f)

/* clear bit in the bmp */
#define BMP_CLEAR_MAC(bmp, index)   \
    (bmp)[(index)>>5] &= ~(1 << ((index) & 0x1f))

/* is bit set in bmp ? */
#define IS_BMP_SET_MAC(bmp, index)   \
    (((bmp)[(index)>>5] & (1 << ((index) & 0x1f))) ? 1 : 0)

/* check if 2 BMPs hold any overlapping info */
/* both must be of the same size             */
/* bmp1 must be 'array' with valid 'sizeof(bmp1)'   */
#define ARE_2_BMPS_OVERLAP_MAC(bmp1,bmp2,areOverlap)   \
    {                                                  \
        GT_U32  _ii;                                   \
        GT_U32  _iiMax = sizeof(bmp1) / 4;             \
        areOverlap = 0;                                \
        for(_ii = 0 ; _ii < _iiMax ; _ii++)             \
        {                                              \
            if(bmp1[_ii] & bmp2[_ii])                    \
            {                                          \
                areOverlap = 1;                        \
                break;                                 \
            }                                          \
        }                                              \
    }


/* Check the reset bit position of LSB in the bitmap
 * e.g bmp - 0001 1011 (pos = 2)
 *     bmp - 0010 0010 (pos = 0)
 **/
#define GET_LSB_BIT_RESET_POSITION_MAC(_bmp, _pos)    \
{                                                     \
    if(_bmp == 0xFFFFFFFF)                            \
    {                                                 \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,   \
            "(expected at least one bit reset)");     \
    }                                                 \
    for(_pos= 0; _pos<32; _pos++)                     \
    {                                                 \
        if((_bmp & (1 << _pos)) == 0)                 \
        {                                             \
            break;                                    \
        }                                             \
    }                                                 \
}

/*
 * Macro to increment the age-bin counter by 1
 * and update the age-bin usage matrix if required
 *     level1UsageBitmap = 1 bit indicates 1  age-bin
 *     level2UsageBitmap = 1 bit indicates 32 age-bin(or 32 bits of level1)
 */
#define INCREASE_AGE_BIN_COUNTER_MAC(_fdbManagerPtr, _ageBinIndex)                                                                          \
{                                                                                                                                           \
    _fdbManagerPtr->agingBinInfo.ageBinListPtr[_ageBinIndex].totalUsedEntriesCnt += 1;                                                      \
    if(_fdbManagerPtr->agingBinInfo.ageBinListPtr[_ageBinIndex].totalUsedEntriesCnt == _fdbManagerPtr->capacityInfo.maxEntriesPerAgingScan) \
    {                                                                                                                                       \
        _fdbManagerPtr->agingBinInfo.level1UsageBitmapPtr[_ageBinIndex>>5] |= (1 << (_ageBinIndex & 0x1F));                                 \
        if(_fdbManagerPtr->agingBinInfo.level1UsageBitmapPtr[_ageBinIndex>>5] == 0xFFFFFFFF)                                                \
        {                                                                                                                                   \
            _fdbManagerPtr->agingBinInfo.level2UsageBitmap |= ( 1 << (_ageBinIndex >> 5));                                                  \
        }                                                                                                                                   \
    }                                                                                                                                       \
}

/*
 * Macro to decrement the age-bin counter by 1
 * and update the age-bin usage matrix if required
 *     level1UsageBitmap = 1 bit indicates 1  age-bin
 *     level2UsageBitmap = 1 bit indicates 32 age-bin(or 32 bits of level1)
 */
#define DECREASE_AGE_BIN_COUNTER_MAC(_fdbManagerPtr, _ageBinIndex)                                                                          \
{                                                                                                                                           \
    _fdbManagerPtr->agingBinInfo.ageBinListPtr[_ageBinIndex].totalUsedEntriesCnt -= 1;                                                      \
    if(_fdbManagerPtr->agingBinInfo.ageBinListPtr[_ageBinIndex].totalUsedEntriesCnt == (_fdbManagerPtr->capacityInfo.maxEntriesPerAgingScan -1)) \
    {                                                                                                                                       \
        _fdbManagerPtr->agingBinInfo.level1UsageBitmapPtr[_ageBinIndex>>5] &= ~(1 << (_ageBinIndex & 0x1F));                                \
        _fdbManagerPtr->agingBinInfo.level2UsageBitmap &= ~( 1 << (_ageBinIndex >> 5));                                                     \
    }                                                                                                                                       \
}

/** Macro to determine if FDB manager shadow is created for SIP4 or not */
#define IS_FDB_MANAGER_FOR_SIP_4(_fdbManagerPtr) \
    (_fdbManagerPtr->entryAttrInfo.shadowType == CPSS_DXCH_BRG_FDB_MANAGER_SHADOW_TYPE_SIP4_E)


/* Type of check for FDB manager HW Free/Used list check with/without SP */
typedef enum{
     PRV_CPSS_DXCH_FDB_MANAGER_DB_CHECK_USED_LIST_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_DB_CHECK_FREE_LIST_SP_AS_USED_E
    ,PRV_CPSS_DXCH_FDB_MANAGER_DB_CHECK_FREE_LIST_SP_AS_FREE_E
}PRV_CPSS_DXCH_FDB_MANAGER_DB_CHECK_LIST_TYPE_ENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChBrgFdbManager_utils_h */


