/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDriverLog.c
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpssDriver/log/private/prvCpssDriverLog.h>
#include <cpssDriver/pp/config/generic/cpssDrvPpGenDump.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>


/********* enums *********/

const PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DRV_HW_RESOURCE_TYPE_ENT[]  =
{PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRV_HW_RESOURCE_SWITCHING_AND_MG0_CORE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRV_HW_RESOURCE_MG1_CORE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRV_HW_RESOURCE_MG2_CORE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRV_HW_RESOURCE_MG3_CORE_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRV_HW_RESOURCE_INTERNAL_PCI_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_DRV_HW_RESOURCE_DFX_CORE_E)
};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(CPSS_DRV_HW_RESOURCE_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DRV_HW_TRACE_TYPE_ENT[]  =
{
    "CPSS_DRV_HW_TRACE_TYPE_READ_E",
    "CPSS_DRV_HW_TRACE_TYPE_WRITE_E",
    "CPSS_DRV_HW_TRACE_TYPE_BOTH_E",
    "CPSS_DRV_HW_TRACE_TYPE_WRITE_DELAY_E",
    "CPSS_DRV_HW_TRACE_TYPE_ALL_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DRV_HW_TRACE_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_MEMORY_DUMP_TYPE_ENT[]  =
{
    "CPSS_MEMORY_DUMP_BYTE_E",
    "CPSS_MEMORY_DUMP_SHORT_E",
    "CPSS_MEMORY_DUMP_WORD_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_MEMORY_DUMP_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DRV_HW_ACCESS_OBJ_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DRV_HW_ACCESS_OBJ_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hwAccessRegisterReadFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hwAccessRegisterWriteFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hwAccessRegisterFieldReadFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hwAccessRegisterFieldWriteFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hwAccessRegisterBitMaskReadFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hwAccessRegisterBitMaskWriteFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hwAccessRamReadFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hwAccessRamWriteFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hwAccessRamWriteInReverseFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hwAccessVectorReadFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hwAccessVectorWriteFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hwAccessRegisterIsrReadFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, hwAccessRegisterIsrWriteFunc);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DRV_HW_ACCESS_OBJ_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DRV_HW_ACCESS_OBJ_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DRV_HW_ACCESS_OBJ_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DRV_HW_RESOURCE_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DRV_HW_RESOURCE_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAP_MAC(namePtr, paramVal, CPSS_DRV_HW_RESOURCE_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DRV_HW_TRACE_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DRV_HW_TRACE_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DRV_HW_TRACE_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_MEMORY_DUMP_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_MEMORY_DUMP_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_MEMORY_DUMP_TYPE_ENT);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC IN_CPSS_DRV_HW_ACCESS_OBJ_STC_PTR_hwAccessObjPtr = {
     "hwAccessObjPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DRV_HW_ACCESS_OBJ_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_CPSS_DRV_HW_RESOURCE_TYPE_ENT_resourceType = {
     "resourceType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DRV_HW_RESOURCE_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_CPSS_DRV_HW_TRACE_TYPE_ENT_traceType = {
     "traceType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DRV_HW_TRACE_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_CPSS_MEMORY_DUMP_TYPE_ENT_dumpType = {
     "dumpType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_MEMORY_DUMP_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_BOOL_bind = {
     "bind", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_HEX_PTR_addrArr = {
     "addrArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_HEX_PTR_data = {
     "data", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_HEX_PTR_dataArr = {
     "dataArr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_HEX_addr = {
     "addr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_HEX_fieldData = {
     "fieldData", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_HEX_mask = {
     "mask", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_HEX_regAddr = {
     "regAddr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_HEX_startAddr = {
     "startAddr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_HEX_value = {
     "value", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_arrLen = {
     "arrLen", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_dumpLength = {
     "dumpLength", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_fieldLength = {
     "fieldLength", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_fieldOffset = {
     "fieldOffset", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_length = {
     "length", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_mgNum = {
     "mgNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_portGroupId = {
     "portGroupId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_CPSS_INTERRUPT_SCAN_STC_PTR_PTR_treeRootPtrPtr = {
     "treeRootPtrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_PTR_MAC(CPSS_INTERRUPT_SCAN_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U32_HEX_PTR_data = {
     "data", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U32_HEX_PTR_dataArr = {
     "dataArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U32_HEX_PTR_fieldData = {
     "fieldData", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32_HEX)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U32_PTR_PTR_notAccessibleBeforeStartInitPtrPtr = {
     "notAccessibleBeforeStartInitPtrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U32_PTR_numOfElementsPtr = {
     "numOfElementsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U32_PTR_numOfInterruptRegistersNotAccessibleBeforeStartInitPtr = {
     "numOfInterruptRegistersNotAccessibleBeforeStartInitPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDrvHwAccessObjectBind_PARAMS[] =  {
    &IN_CPSS_DRV_HW_ACCESS_OBJ_STC_PTR_hwAccessObjPtr,
    &IN_GT_BOOL_bind
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDrvPpHwTraceEnable_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_CPSS_DRV_HW_TRACE_TYPE_ENT_traceType,
    &IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const prvCpssDrvHwPpMgSetRegField_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_mgNum,
    &IN_GT_U32_HEX_regAddr,
    &IN_GT_U32_fieldOffset,
    &IN_GT_U32_fieldLength,
    &IN_GT_U32_HEX_fieldData
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const prvCpssDrvHwPpMgGetRegField_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_mgNum,
    &IN_GT_U32_HEX_regAddr,
    &IN_GT_U32_fieldOffset,
    &IN_GT_U32_fieldLength,
    &OUT_GT_U32_HEX_PTR_fieldData
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const prvCpssDrvHwPpMgWriteReg_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_mgNum,
    &IN_GT_U32_HEX_regAddr,
    &IN_GT_U32_HEX_data
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const prvCpssDrvHwPpMgReadReg_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_mgNum,
    &IN_GT_U32_HEX_regAddr,
    &OUT_GT_U32_HEX_PTR_data
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDrvPpHwResourceWriteRegister_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_portGroupId,
    &IN_CPSS_DRV_HW_RESOURCE_TYPE_ENT_resourceType,
    &IN_GT_U32_HEX_regAddr,
    &IN_GT_U32_HEX_data
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDrvPpHwResourceReadRegister_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_portGroupId,
    &IN_CPSS_DRV_HW_RESOURCE_TYPE_ENT_resourceType,
    &IN_GT_U32_HEX_regAddr,
    &OUT_GT_U32_HEX_PTR_dataPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDrvPpHwRamInReverseWrite_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_portGroupId,
    &IN_GT_U32_HEX_addr,
    &IN_GT_U32_length,
    &IN_GT_U32_HEX_PTR_data
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const prvCpssDrvHwPpPortGroupReadRam_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_portGroupId,
    &IN_GT_U32_HEX_addr,
    &IN_GT_U32_length,
    &OUT_GT_U32_HEX_PTR_data
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const prvCpssDrvHwPpPortGroupWriteVec_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_portGroupId,
    &IN_GT_U32_HEX_PTR_addrArr,
    &IN_GT_U32_HEX_PTR_dataArr,
    &IN_GT_U32_arrLen
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const prvCpssDrvHwPpPortGroupReadVec_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_portGroupId,
    &IN_GT_U32_HEX_PTR_addrArr,
    &OUT_GT_U32_HEX_PTR_dataArr,
    &IN_GT_U32_arrLen
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDrvPpHwRegFieldSet_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_portGroupId,
    &IN_GT_U32_HEX_regAddr,
    &IN_GT_U32_fieldOffset,
    &IN_GT_U32_fieldLength,
    &IN_GT_U32_HEX_fieldData
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDrvPpHwRegFieldGet_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_portGroupId,
    &IN_GT_U32_HEX_regAddr,
    &IN_GT_U32_fieldOffset,
    &IN_GT_U32_fieldLength,
    &OUT_GT_U32_HEX_PTR_fieldData
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const prvCpssDrvHwPpPortGroupWriteInternalPciReg_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_portGroupId,
    &IN_GT_U32_HEX_regAddr,
    &IN_GT_U32_HEX_data
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDrvPpHwRegBitMaskWrite_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_portGroupId,
    &IN_GT_U32_HEX_regAddr,
    &IN_GT_U32_HEX_mask,
    &IN_GT_U32_HEX_value
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDrvPpHwRegBitMaskRead_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_portGroupId,
    &IN_GT_U32_HEX_regAddr,
    &IN_GT_U32_HEX_mask,
    &OUT_GT_U32_HEX_PTR_dataPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDrvPpHwRegisterWrite_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_portGroupId,
    &IN_GT_U32_HEX_regAddr,
    &IN_GT_U32_HEX_value
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDrvPpHwRegisterRead_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_portGroupId,
    &IN_GT_U32_HEX_regAddr,
    &OUT_GT_U32_HEX_PTR_data
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const prvCpssDrvHwPpWriteRamInReverse_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_HEX_addr,
    &IN_GT_U32_length,
    &IN_GT_U32_HEX_PTR_data
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDrvHwPpResetAndInitControllerSetRegField_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_HEX_regAddr,
    &IN_GT_U32_fieldOffset,
    &IN_GT_U32_fieldLength,
    &IN_GT_U32_HEX_fieldData
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDrvHwPpResetAndInitControllerGetRegField_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_HEX_regAddr,
    &IN_GT_U32_fieldOffset,
    &IN_GT_U32_fieldLength,
    &OUT_GT_U32_HEX_PTR_fieldData
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDrvHwPpResetAndInitControllerWriteReg_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_HEX_regAddr,
    &IN_GT_U32_HEX_data
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const prvCpssDrvHwPpWriteRegBitMask_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_HEX_regAddr,
    &IN_GT_U32_HEX_mask,
    &IN_GT_U32_HEX_value
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const prvCpssDrvHwPpReadRegBitMask_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_HEX_regAddr,
    &IN_GT_U32_HEX_mask,
    &OUT_GT_U32_HEX_PTR_dataPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const prvCpssDrvHwPpWriteRegister_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_HEX_regAddr,
    &IN_GT_U32_HEX_value
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDrvHwPpResetAndInitControllerReadReg_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_HEX_regAddr,
    &OUT_GT_U32_HEX_PTR_data
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssPpDumpMemory_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_HEX_startAddr,
    &IN_CPSS_MEMORY_DUMP_TYPE_ENT_dumpType,
    &IN_GT_U32_dumpLength
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDrvInterruptsTreeGet_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &OUT_GT_U32_PTR_numOfElementsPtr,
    &OUT_CPSS_INTERRUPT_SCAN_STC_PTR_PTR_treeRootPtrPtr,
    &OUT_GT_U32_PTR_numOfInterruptRegistersNotAccessibleBeforeStartInitPtr,
    &OUT_GT_U32_PTR_PTR_notAccessibleBeforeStartInitPtrPtr
};
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDrvPpHwRamRead_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDrvPpHwRamWrite_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDrvPpHwVectorRead_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDrvPpHwVectorWrite_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const prvCpssDrvHwPpReadRam_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const prvCpssDrvHwPpReadVec_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const prvCpssDrvHwPpWriteRam_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const prvCpssDrvHwPpWriteVec_PARAMS[];


/********* lib API DB *********/


#ifdef CHX_FAMILY
extern void cpssDrvPpHwRamRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDrvPpHwRamRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDrvPpHwVectorRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDrvPpHwVectorRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDrvPpHwRamRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void prvCpssDrvHwPpReadRam_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void prvCpssDrvHwPpReadRam_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void prvCpssDrvHwPpReadVec_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void prvCpssDrvHwPpReadVec_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void prvCpssDrvHwPpReadRam_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDrvPpHwRamRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDrvPpHwRamRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDrvPpHwRamRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDrvPpHwVectorRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDrvPpHwVectorRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssCommonDXCpssDriverLogLibDb[] = {
    {"cpssDrvPpHwRegisterRead", 4, cpssDrvPpHwRegisterRead_PARAMS, NULL},
    {"cpssDrvPpHwRegisterWrite", 4, cpssDrvPpHwRegisterWrite_PARAMS, NULL},
    {"cpssDrvPpHwRegFieldGet", 6, cpssDrvPpHwRegFieldGet_PARAMS, NULL},
    {"cpssDrvPpHwRegFieldSet", 6, cpssDrvPpHwRegFieldSet_PARAMS, NULL},
    {"cpssDrvPpHwRegBitMaskRead", 5, cpssDrvPpHwRegBitMaskRead_PARAMS, NULL},
    {"cpssDrvPpHwRegBitMaskWrite", 5, cpssDrvPpHwRegBitMaskWrite_PARAMS, NULL},
    {"cpssDrvPpHwRamRead", 5, cpssDrvPpHwRamRead_PARAMS, cpssDrvPpHwRamRead_preLogic},
    {"cpssDrvPpHwRamWrite", 5, cpssDrvPpHwRamWrite_PARAMS, cpssDrvPpHwRamRead_preLogic},
    {"cpssDrvPpHwVectorRead", 5, cpssDrvPpHwVectorRead_PARAMS, cpssDrvPpHwVectorRead_preLogic},
    {"cpssDrvPpHwVectorWrite", 5, cpssDrvPpHwVectorWrite_PARAMS, cpssDrvPpHwVectorRead_preLogic},
    {"cpssDrvPpHwRamInReverseWrite", 5, cpssDrvPpHwRamInReverseWrite_PARAMS, cpssDrvPpHwRamRead_preLogic},
    {"cpssDrvHwPpResetAndInitControllerReadReg", 3, cpssDrvHwPpResetAndInitControllerReadReg_PARAMS, NULL},
    {"cpssDrvHwPpResetAndInitControllerWriteReg", 3, cpssDrvHwPpResetAndInitControllerWriteReg_PARAMS, NULL},
    {"cpssDrvHwPpResetAndInitControllerGetRegField", 5, cpssDrvHwPpResetAndInitControllerGetRegField_PARAMS, NULL},
    {"cpssDrvHwPpResetAndInitControllerSetRegField", 5, cpssDrvHwPpResetAndInitControllerSetRegField_PARAMS, NULL},
    {"cpssDrvPpHwTraceEnable", 3, cpssDrvPpHwTraceEnable_PARAMS, NULL},
    {"cpssDrvHwAccessObjectBind", 2, cpssDrvHwAccessObjectBind_PARAMS, NULL},
    {"cpssDrvPpHwResourceReadRegister", 5, cpssDrvPpHwResourceReadRegister_PARAMS, NULL},
    {"cpssDrvPpHwResourceWriteRegister", 5, cpssDrvPpHwResourceWriteRegister_PARAMS, NULL},
    {"prvCpssDrvHwPpReadRegister", 3, cpssDrvHwPpResetAndInitControllerReadReg_PARAMS, NULL},
    {"prvCpssDrvHwPpWriteRegister", 3, prvCpssDrvHwPpWriteRegister_PARAMS, NULL},
    {"prvCpssDrvHwPpGetRegField", 5, cpssDrvHwPpResetAndInitControllerGetRegField_PARAMS, NULL},
    {"prvCpssDrvHwPpSetRegField", 5, cpssDrvHwPpResetAndInitControllerSetRegField_PARAMS, NULL},
    {"prvCpssDrvHwPpReadRegBitMask", 4, prvCpssDrvHwPpReadRegBitMask_PARAMS, NULL},
    {"prvCpssDrvHwPpWriteRegBitMask", 4, prvCpssDrvHwPpWriteRegBitMask_PARAMS, NULL},
    {"prvCpssDrvHwPpReadRam", 4, prvCpssDrvHwPpReadRam_PARAMS, prvCpssDrvHwPpReadRam_preLogic},
    {"prvCpssDrvHwPpWriteRam", 4, prvCpssDrvHwPpWriteRam_PARAMS, prvCpssDrvHwPpReadRam_preLogic},
    {"prvCpssDrvHwPpReadVec", 4, prvCpssDrvHwPpReadVec_PARAMS, prvCpssDrvHwPpReadVec_preLogic},
    {"prvCpssDrvHwPpWriteVec", 4, prvCpssDrvHwPpWriteVec_PARAMS, prvCpssDrvHwPpReadVec_preLogic},
    {"prvCpssDrvHwPpWriteRamInReverse", 4, prvCpssDrvHwPpWriteRamInReverse_PARAMS, prvCpssDrvHwPpReadRam_preLogic},
    {"prvCpssDrvHwPpReadInternalPciReg", 3, cpssDrvHwPpResetAndInitControllerReadReg_PARAMS, NULL},
    {"prvCpssDrvHwPpWriteInternalPciReg", 3, cpssDrvHwPpResetAndInitControllerWriteReg_PARAMS, NULL},
    {"prvCpssDrvHwPpResetAndInitControllerReadReg", 3, cpssDrvHwPpResetAndInitControllerReadReg_PARAMS, NULL},
    {"prvCpssDrvHwPpResetAndInitControllerGetRegField", 5, cpssDrvHwPpResetAndInitControllerGetRegField_PARAMS, NULL},
    {"prvCpssDrvHwPpResetAndInitControllerWriteReg", 3, cpssDrvHwPpResetAndInitControllerWriteReg_PARAMS, NULL},
    {"prvCpssDrvHwPpResetAndInitControllerSetRegField", 5, cpssDrvHwPpResetAndInitControllerSetRegField_PARAMS, NULL},
    {"prvCpssDrvHwPpPortGroupReadRegister", 4, cpssDrvPpHwRegisterRead_PARAMS, NULL},
    {"prvCpssDrvHwPpPortGroupWriteRegister", 4, cpssDrvPpHwRegisterWrite_PARAMS, NULL},
    {"prvCpssDrvHwPpPortGroupGetRegField", 6, cpssDrvPpHwRegFieldGet_PARAMS, NULL},
    {"prvCpssDrvHwPpPortGroupSetRegField", 6, cpssDrvPpHwRegFieldSet_PARAMS, NULL},
    {"prvCpssDrvHwPpPortGroupReadRegBitMask", 5, cpssDrvPpHwRegBitMaskRead_PARAMS, NULL},
    {"prvCpssDrvHwPpPortGroupWriteRegBitMask", 5, cpssDrvPpHwRegBitMaskWrite_PARAMS, NULL},
    {"prvCpssDrvHwPpPortGroupReadRam", 5, prvCpssDrvHwPpPortGroupReadRam_PARAMS, cpssDrvPpHwRamRead_preLogic},
    {"prvCpssDrvHwPpPortGroupWriteRam", 5, cpssDrvPpHwRamInReverseWrite_PARAMS, cpssDrvPpHwRamRead_preLogic},
    {"prvCpssDrvHwPpPortGroupWriteRamInReverse", 5, cpssDrvPpHwRamInReverseWrite_PARAMS, cpssDrvPpHwRamRead_preLogic},
    {"prvCpssDrvHwPpPortGroupReadVec", 5, prvCpssDrvHwPpPortGroupReadVec_PARAMS, cpssDrvPpHwVectorRead_preLogic},
    {"prvCpssDrvHwPpPortGroupWriteVec", 5, prvCpssDrvHwPpPortGroupWriteVec_PARAMS, cpssDrvPpHwVectorRead_preLogic},
    {"prvCpssDrvHwPpPortGroupReadInternalPciReg", 4, cpssDrvPpHwRegisterRead_PARAMS, NULL},
    {"prvCpssDrvHwPpPortGroupWriteInternalPciReg", 4, prvCpssDrvHwPpPortGroupWriteInternalPciReg_PARAMS, NULL},
    {"prvCpssDrvHwPpMgReadReg", 4, prvCpssDrvHwPpMgReadReg_PARAMS, NULL},
    {"prvCpssDrvHwPpMgWriteReg", 4, prvCpssDrvHwPpMgWriteReg_PARAMS, NULL},
    {"prvCpssDrvHwPpMgGetRegField", 6, prvCpssDrvHwPpMgGetRegField_PARAMS, NULL},
    {"prvCpssDrvHwPpMgSetRegField", 6, prvCpssDrvHwPpMgSetRegField_PARAMS, NULL},
    {"prvCpssDrvHwPpBar0ReadReg", 3, cpssDrvHwPpResetAndInitControllerReadReg_PARAMS, NULL},
    {"prvCpssDrvHwPpBar0WriteReg", 3, cpssDrvHwPpResetAndInitControllerWriteReg_PARAMS, NULL},
    {"cpssDrvInterruptsTreeGet", 5, cpssDrvInterruptsTreeGet_PARAMS, NULL},
    {"cpssPpDumpRegisters", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssPpDumpMemory", 4, cpssPpDumpMemory_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_APP_DRIVER_CALL(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssCommonDXCpssDriverLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssCommonDXCpssDriverLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}
#endif

#ifdef PX_FAMILY
extern void cpssDrvPpHwRamRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDrvPpHwRamRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDrvPpHwVectorRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDrvPpHwVectorRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDrvPpHwRamRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void prvCpssDrvHwPpReadRam_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void prvCpssDrvHwPpReadRam_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void prvCpssDrvHwPpReadVec_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void prvCpssDrvHwPpReadVec_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void prvCpssDrvHwPpReadRam_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDrvPpHwRamRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDrvPpHwRamRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDrvPpHwRamRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDrvPpHwVectorRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDrvPpHwVectorRead_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssCommonPXCpssDriverLogLibDb[] = {
    {"cpssDrvPpHwRegisterRead", 4, cpssDrvPpHwRegisterRead_PARAMS, NULL},
    {"cpssDrvPpHwRegisterWrite", 4, cpssDrvPpHwRegisterWrite_PARAMS, NULL},
    {"cpssDrvPpHwRegFieldGet", 6, cpssDrvPpHwRegFieldGet_PARAMS, NULL},
    {"cpssDrvPpHwRegFieldSet", 6, cpssDrvPpHwRegFieldSet_PARAMS, NULL},
    {"cpssDrvPpHwRegBitMaskRead", 5, cpssDrvPpHwRegBitMaskRead_PARAMS, NULL},
    {"cpssDrvPpHwRegBitMaskWrite", 5, cpssDrvPpHwRegBitMaskWrite_PARAMS, NULL},
    {"cpssDrvPpHwRamRead", 5, cpssDrvPpHwRamRead_PARAMS, cpssDrvPpHwRamRead_preLogic},
    {"cpssDrvPpHwRamWrite", 5, cpssDrvPpHwRamWrite_PARAMS, cpssDrvPpHwRamRead_preLogic},
    {"cpssDrvPpHwVectorRead", 5, cpssDrvPpHwVectorRead_PARAMS, cpssDrvPpHwVectorRead_preLogic},
    {"cpssDrvPpHwVectorWrite", 5, cpssDrvPpHwVectorWrite_PARAMS, cpssDrvPpHwVectorRead_preLogic},
    {"cpssDrvPpHwRamInReverseWrite", 5, cpssDrvPpHwRamInReverseWrite_PARAMS, cpssDrvPpHwRamRead_preLogic},
    {"cpssDrvHwPpResetAndInitControllerReadReg", 3, cpssDrvHwPpResetAndInitControllerReadReg_PARAMS, NULL},
    {"cpssDrvHwPpResetAndInitControllerWriteReg", 3, cpssDrvHwPpResetAndInitControllerWriteReg_PARAMS, NULL},
    {"cpssDrvHwPpResetAndInitControllerGetRegField", 5, cpssDrvHwPpResetAndInitControllerGetRegField_PARAMS, NULL},
    {"cpssDrvHwPpResetAndInitControllerSetRegField", 5, cpssDrvHwPpResetAndInitControllerSetRegField_PARAMS, NULL},
    {"cpssDrvPpHwTraceEnable", 3, cpssDrvPpHwTraceEnable_PARAMS, NULL},
    {"cpssDrvHwAccessObjectBind", 2, cpssDrvHwAccessObjectBind_PARAMS, NULL},
    {"cpssDrvPpHwResourceReadRegister", 5, cpssDrvPpHwResourceReadRegister_PARAMS, NULL},
    {"cpssDrvPpHwResourceWriteRegister", 5, cpssDrvPpHwResourceWriteRegister_PARAMS, NULL},
    {"prvCpssDrvHwPpReadRegister", 3, cpssDrvHwPpResetAndInitControllerReadReg_PARAMS, NULL},
    {"prvCpssDrvHwPpWriteRegister", 3, prvCpssDrvHwPpWriteRegister_PARAMS, NULL},
    {"prvCpssDrvHwPpGetRegField", 5, cpssDrvHwPpResetAndInitControllerGetRegField_PARAMS, NULL},
    {"prvCpssDrvHwPpSetRegField", 5, cpssDrvHwPpResetAndInitControllerSetRegField_PARAMS, NULL},
    {"prvCpssDrvHwPpReadRegBitMask", 4, prvCpssDrvHwPpReadRegBitMask_PARAMS, NULL},
    {"prvCpssDrvHwPpWriteRegBitMask", 4, prvCpssDrvHwPpWriteRegBitMask_PARAMS, NULL},
    {"prvCpssDrvHwPpReadRam", 4, prvCpssDrvHwPpReadRam_PARAMS, prvCpssDrvHwPpReadRam_preLogic},
    {"prvCpssDrvHwPpWriteRam", 4, prvCpssDrvHwPpWriteRam_PARAMS, prvCpssDrvHwPpReadRam_preLogic},
    {"prvCpssDrvHwPpReadVec", 4, prvCpssDrvHwPpReadVec_PARAMS, prvCpssDrvHwPpReadVec_preLogic},
    {"prvCpssDrvHwPpWriteVec", 4, prvCpssDrvHwPpWriteVec_PARAMS, prvCpssDrvHwPpReadVec_preLogic},
    {"prvCpssDrvHwPpWriteRamInReverse", 4, prvCpssDrvHwPpWriteRamInReverse_PARAMS, prvCpssDrvHwPpReadRam_preLogic},
    {"prvCpssDrvHwPpReadInternalPciReg", 3, cpssDrvHwPpResetAndInitControllerReadReg_PARAMS, NULL},
    {"prvCpssDrvHwPpWriteInternalPciReg", 3, cpssDrvHwPpResetAndInitControllerWriteReg_PARAMS, NULL},
    {"prvCpssDrvHwPpResetAndInitControllerReadReg", 3, cpssDrvHwPpResetAndInitControllerReadReg_PARAMS, NULL},
    {"prvCpssDrvHwPpResetAndInitControllerGetRegField", 5, cpssDrvHwPpResetAndInitControllerGetRegField_PARAMS, NULL},
    {"prvCpssDrvHwPpResetAndInitControllerWriteReg", 3, cpssDrvHwPpResetAndInitControllerWriteReg_PARAMS, NULL},
    {"prvCpssDrvHwPpResetAndInitControllerSetRegField", 5, cpssDrvHwPpResetAndInitControllerSetRegField_PARAMS, NULL},
    {"prvCpssDrvHwPpPortGroupReadRegister", 4, cpssDrvPpHwRegisterRead_PARAMS, NULL},
    {"prvCpssDrvHwPpPortGroupWriteRegister", 4, cpssDrvPpHwRegisterWrite_PARAMS, NULL},
    {"prvCpssDrvHwPpPortGroupGetRegField", 6, cpssDrvPpHwRegFieldGet_PARAMS, NULL},
    {"prvCpssDrvHwPpPortGroupSetRegField", 6, cpssDrvPpHwRegFieldSet_PARAMS, NULL},
    {"prvCpssDrvHwPpPortGroupReadRegBitMask", 5, cpssDrvPpHwRegBitMaskRead_PARAMS, NULL},
    {"prvCpssDrvHwPpPortGroupWriteRegBitMask", 5, cpssDrvPpHwRegBitMaskWrite_PARAMS, NULL},
    {"prvCpssDrvHwPpPortGroupReadRam", 5, prvCpssDrvHwPpPortGroupReadRam_PARAMS, cpssDrvPpHwRamRead_preLogic},
    {"prvCpssDrvHwPpPortGroupWriteRam", 5, cpssDrvPpHwRamInReverseWrite_PARAMS, cpssDrvPpHwRamRead_preLogic},
    {"prvCpssDrvHwPpPortGroupWriteRamInReverse", 5, cpssDrvPpHwRamInReverseWrite_PARAMS, cpssDrvPpHwRamRead_preLogic},
    {"prvCpssDrvHwPpPortGroupReadVec", 5, prvCpssDrvHwPpPortGroupReadVec_PARAMS, cpssDrvPpHwVectorRead_preLogic},
    {"prvCpssDrvHwPpPortGroupWriteVec", 5, prvCpssDrvHwPpPortGroupWriteVec_PARAMS, cpssDrvPpHwVectorRead_preLogic},
    {"prvCpssDrvHwPpPortGroupReadInternalPciReg", 4, cpssDrvPpHwRegisterRead_PARAMS, NULL},
    {"prvCpssDrvHwPpPortGroupWriteInternalPciReg", 4, prvCpssDrvHwPpPortGroupWriteInternalPciReg_PARAMS, NULL},
    {"prvCpssDrvHwPpMgReadReg", 4, prvCpssDrvHwPpMgReadReg_PARAMS, NULL},
    {"prvCpssDrvHwPpMgWriteReg", 4, prvCpssDrvHwPpMgWriteReg_PARAMS, NULL},
    {"prvCpssDrvHwPpMgGetRegField", 6, prvCpssDrvHwPpMgGetRegField_PARAMS, NULL},
    {"prvCpssDrvHwPpMgSetRegField", 6, prvCpssDrvHwPpMgSetRegField_PARAMS, NULL},
    {"prvCpssDrvHwPpBar0ReadReg", 3, cpssDrvHwPpResetAndInitControllerReadReg_PARAMS, NULL},
    {"prvCpssDrvHwPpBar0WriteReg", 3, cpssDrvHwPpResetAndInitControllerWriteReg_PARAMS, NULL},
    {"cpssDrvInterruptsTreeGet", 5, cpssDrvInterruptsTreeGet_PARAMS, NULL},
    {"cpssPpDumpRegisters", 1, prvCpssLogGenDevNum3_PARAMS, NULL},
    {"cpssPpDumpMemory", 4, cpssPpDumpMemory_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_PX_CPSS_LOG_LIB_APP_DRIVER_CALL(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssCommonPXCpssDriverLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssCommonPXCpssDriverLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}
#endif

