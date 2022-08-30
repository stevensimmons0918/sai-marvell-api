/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChPpuLog.c
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

/* disable deprecation warnings (if one) */
#ifdef __GNUC__
#if  (__GNUC__*100+__GNUC_MINOR__) >= 406
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#endif

#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/ppu/cpssDxChPpuTypes.h>
#include <cpss/dxCh/dxChxGen/ppu/private/prvCpssDxChPpuLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, protWinEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, protWinStartOffset);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, protWinEndOffset);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, setDescBits, CPSS_DXCH_PPU_DAU_DESC_BYTE_SET_MAX_CNS, CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_BIT_SEL_PER_BYTE_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PPU_DEBUG_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PPU_DEBUG_COUNTERS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, counterIn);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, counterOut);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, counterKstg0);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, counterKstg1);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, counterKstg2);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, counterDau);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, counterLoopback);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PPU_ERROR_PROFILE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PPU_ERROR_PROFILE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, errProfileField, CPSS_DXCH_PPU_ERROR_PROFILE_FIELDS_MAX_CNS, CPSS_DXCH_PPU_ERROR_PROFILE_FIELD_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PPU_GLOBAL_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PPU_GLOBAL_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxLoopBack);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, errProfMaxLoopBack);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, errProfSer);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, errProfOffsetOor);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, debugCounterEnable);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, rotActionEntry, CPSS_DXCH_PPU_KSTG_ROTS_MAX_CNS, CPSS_DXCH_PPU_KSTG_ROT_ACTION_ENTRY_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, setNextState);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, setConstNextShift);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, setLoopBack);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, counterSet);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, counterSetVal);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, counterOper);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, byteSelOffset, CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_BYTE_SELECT_MAX_CNS, GT_U8);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, keyLsb);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, keyMsb);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, maskLsb);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, maskMsb);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isValid);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PPU_PROFILE_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PPU_PROFILE_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ppuEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ppuState);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, spBusDefaultProfile);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, anchorType);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, offset);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PPU_SP_BUS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PPU_SP_BUS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, data, CPSS_DXCH_PPU_SP_BUS_BYTES_MAX_CNS, GT_U8);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_BIT_SEL_PER_BYTE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_BIT_SEL_PER_BYTE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, spByteWriteEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, spByteNumBits);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, spByteSrcOffset);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, spByteTargetOffset);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PPU_ERROR_PROFILE_FIELD_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PPU_ERROR_PROFILE_FIELD_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, writeEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numBits);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, errorDataField);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, targetOffset);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_PPU_KSTG_ROT_ACTION_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_PPU_KSTG_ROT_ACTION_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srcRegSel);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, setCmd4Byte);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srcOffset);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srcNumValBits);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, shiftLeftRightSel);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, shiftNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, setBitsNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, setBitsVal);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, setBitsOffset);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, addSubConst);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, addSubConstSel);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cmpVal);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cmpFalseValLd);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cmpCond);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, cmpMask);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cmpTrueValLd);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, func);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, funcSecondOperand);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, target);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ldSpBusNumBytes);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, ldSpBusOffset);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, interrupt);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, interruptIndex);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PPU_DEBUG_COUNTERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PPU_DEBUG_COUNTERS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PPU_DEBUG_COUNTERS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PPU_ERROR_PROFILE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PPU_ERROR_PROFILE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PPU_ERROR_PROFILE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PPU_GLOBAL_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PPU_GLOBAL_CONFIG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PPU_GLOBAL_CONFIG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PPU_PROFILE_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PPU_PROFILE_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PPU_PROFILE_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_PPU_SP_BUS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PPU_SP_BUS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_PPU_SP_BUS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC_PTR_protWinPtr = {
     "protWinPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC_PTR_dauProfilePtr = {
     "dauProfilePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PPU_ERROR_PROFILE_STC_PTR_errorProfilePtr = {
     "errorProfilePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PPU_ERROR_PROFILE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PPU_GLOBAL_CONFIG_STC_PTR_ppuGlobalConfigPtr = {
     "ppuGlobalConfigPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PPU_GLOBAL_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC_PTR_ppuActionEntryPtr = {
     "ppuActionEntryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC_PTR_keygenProfilePtr = {
     "keygenProfilePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC_PTR_tcamEntryPtr = {
     "tcamEntryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PPU_PROFILE_ENTRY_STC_PTR_ppuProfilePtr = {
     "ppuProfilePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PPU_PROFILE_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PPU_SP_BUS_STC_PTR_spBusProfilePtr = {
     "spBusProfilePtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PPU_SP_BUS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_kstgNum = {
     "kstgNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_maxLoopback = {
     "maxLoopback", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ttiRuleIndex = {
     "ttiRuleIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC_PTR_protWinPtr = {
     "protWinPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC_PTR_dauProfilePtr = {
     "dauProfilePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PPU_DEBUG_COUNTERS_STC_PTR_dbgCountersPtr = {
     "dbgCountersPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PPU_DEBUG_COUNTERS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PPU_ERROR_PROFILE_STC_PTR_errorProfilePtr = {
     "errorProfilePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PPU_ERROR_PROFILE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PPU_GLOBAL_CONFIG_STC_PTR_ppuGlobalConfigPtr = {
     "ppuGlobalConfigPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PPU_GLOBAL_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC_PTR_ppuActionEntryPtr = {
     "ppuActionEntryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC_PTR_keygenProfilePtr = {
     "keygenProfilePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC_PTR_tcamEntryPtr = {
     "tcamEntryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PPU_PROFILE_ENTRY_STC_PTR_ppuProfilePtr = {
     "ppuProfilePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PPU_PROFILE_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PPU_SP_BUS_STC_PTR_spBusProfilePtr = {
     "spBusProfilePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_PPU_SP_BUS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxLoopbackPtr = {
     "maxLoopbackPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuGlobalConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_PPU_GLOBAL_CONFIG_STC_PTR_ppuGlobalConfigPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuSrcPortProfileIndexSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_srcPortNum,
    &DX_IN_GT_U32_profileNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuSrcPortProfileIndexGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_srcPortNum,
    &DX_OUT_GT_U32_PTR_profileNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuDauProtectedWindowSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC_PTR_protWinPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuDauProtectedWindowGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC_PTR_protWinPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuActionTableEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_kstgNum,
    &DX_IN_GT_U32_index,
    &DX_IN_CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC_PTR_ppuActionEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuActionTableEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_kstgNum,
    &DX_IN_GT_U32_index,
    &DX_OUT_CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC_PTR_ppuActionEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuKstgKeyGenProfileSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_kstgNum,
    &DX_IN_GT_U32_profileNum,
    &DX_IN_CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC_PTR_keygenProfilePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuKstgTcamEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_kstgNum,
    &DX_IN_GT_U32_profileNum,
    &DX_IN_CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC_PTR_tcamEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuKstgKeyGenProfileGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_kstgNum,
    &DX_IN_GT_U32_profileNum,
    &DX_OUT_CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC_PTR_keygenProfilePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuKstgTcamEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_kstgNum,
    &DX_IN_GT_U32_profileNum,
    &DX_OUT_CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC_PTR_tcamEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuMaxLoopbackSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_maxLoopback
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuDauProfileEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_profileIndex,
    &DX_IN_CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC_PTR_dauProfilePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuDauProfileEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_profileIndex,
    &DX_OUT_CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC_PTR_dauProfilePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuErrorProfileSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_profileNum,
    &DX_IN_CPSS_DXCH_PPU_ERROR_PROFILE_STC_PTR_errorProfilePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuProfileSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_profileNum,
    &DX_IN_CPSS_DXCH_PPU_PROFILE_ENTRY_STC_PTR_ppuProfilePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuSpBusDefaultProfileSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_profileNum,
    &DX_IN_CPSS_DXCH_PPU_SP_BUS_STC_PTR_spBusProfilePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuErrorProfileGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_profileNum,
    &DX_OUT_CPSS_DXCH_PPU_ERROR_PROFILE_STC_PTR_errorProfilePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuProfileGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_profileNum,
    &DX_OUT_CPSS_DXCH_PPU_PROFILE_ENTRY_STC_PTR_ppuProfilePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuSpBusDefaultProfileGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_profileNum,
    &DX_OUT_CPSS_DXCH_PPU_SP_BUS_STC_PTR_spBusProfilePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuTtiActionProfileIndexSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_ttiRuleIndex,
    &DX_IN_GT_U32_profileNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuTtiActionProfileIndexGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_ttiRuleIndex,
    &DX_OUT_GT_U32_PTR_profileNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuDebugCountersGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_PPU_DEBUG_COUNTERS_STC_PTR_dbgCountersPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuGlobalConfigGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_CPSS_DXCH_PPU_GLOBAL_CONFIG_STC_PTR_ppuGlobalConfigPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChPpuMaxLoopbackGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_U32_PTR_maxLoopbackPtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChPpuLogLibDb[] = {
    {"cpssDxChPpuEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChPpuEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChPpuMaxLoopbackSet", 2, cpssDxChPpuMaxLoopbackSet_PARAMS, NULL},
    {"cpssDxChPpuMaxLoopbackGet", 2, cpssDxChPpuMaxLoopbackGet_PARAMS, NULL},
    {"cpssDxChPpuGlobalConfigSet", 2, cpssDxChPpuGlobalConfigSet_PARAMS, NULL},
    {"cpssDxChPpuGlobalConfigGet", 2, cpssDxChPpuGlobalConfigGet_PARAMS, NULL},
    {"cpssDxChPpuTtiActionProfileIndexSet", 3, cpssDxChPpuTtiActionProfileIndexSet_PARAMS, NULL},
    {"cpssDxChPpuTtiActionProfileIndexGet", 3, cpssDxChPpuTtiActionProfileIndexGet_PARAMS, NULL},
    {"cpssDxChPpuSrcPortProfileIndexSet", 3, cpssDxChPpuSrcPortProfileIndexSet_PARAMS, NULL},
    {"cpssDxChPpuSrcPortProfileIndexGet", 3, cpssDxChPpuSrcPortProfileIndexGet_PARAMS, NULL},
    {"cpssDxChPpuProfileSet", 3, cpssDxChPpuProfileSet_PARAMS, NULL},
    {"cpssDxChPpuProfileGet", 3, cpssDxChPpuProfileGet_PARAMS, NULL},
    {"cpssDxChPpuSpBusDefaultProfileSet", 3, cpssDxChPpuSpBusDefaultProfileSet_PARAMS, NULL},
    {"cpssDxChPpuSpBusDefaultProfileGet", 3, cpssDxChPpuSpBusDefaultProfileGet_PARAMS, NULL},
    {"cpssDxChPpuKstgKeyGenProfileSet", 4, cpssDxChPpuKstgKeyGenProfileSet_PARAMS, NULL},
    {"cpssDxChPpuKstgKeyGenProfileGet", 4, cpssDxChPpuKstgKeyGenProfileGet_PARAMS, NULL},
    {"cpssDxChPpuKstgTcamEntrySet", 4, cpssDxChPpuKstgTcamEntrySet_PARAMS, NULL},
    {"cpssDxChPpuKstgTcamEntryGet", 4, cpssDxChPpuKstgTcamEntryGet_PARAMS, NULL},
    {"cpssDxChPpuActionTableEntrySet", 4, cpssDxChPpuActionTableEntrySet_PARAMS, NULL},
    {"cpssDxChPpuActionTableEntryGet", 4, cpssDxChPpuActionTableEntryGet_PARAMS, NULL},
    {"cpssDxChPpuDauProfileEntrySet", 3, cpssDxChPpuDauProfileEntrySet_PARAMS, NULL},
    {"cpssDxChPpuDauProfileEntryGet", 3, cpssDxChPpuDauProfileEntryGet_PARAMS, NULL},
    {"cpssDxChPpuDauProtectedWindowSet", 3, cpssDxChPpuDauProtectedWindowSet_PARAMS, NULL},
    {"cpssDxChPpuDauProtectedWindowGet", 3, cpssDxChPpuDauProtectedWindowGet_PARAMS, NULL},
    {"cpssDxChPpuErrorProfileSet", 3, cpssDxChPpuErrorProfileSet_PARAMS, NULL},
    {"cpssDxChPpuErrorProfileGet", 3, cpssDxChPpuErrorProfileGet_PARAMS, NULL},
    {"cpssDxChPpuDebugCountersGet", 2, cpssDxChPpuDebugCountersGet_PARAMS, NULL},
    {"cpssDxChPpuDebugCountersClear", 1, prvCpssLogGenDevNum_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_PPU(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChPpuLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChPpuLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

