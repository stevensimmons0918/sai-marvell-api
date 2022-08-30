/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChFabricLog.c
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
#include <cpss/dxCh/dxChxGen/fabric/cpssDxChFabricHGLink.h>
#include <cpss/dxCh/dxChxGen/fabric/private/prvCpssDxChFabricLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT[]  =
{
    "CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_UC_E",
    "CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_MC_E",
    "CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_BC_E",
    "CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_OTHER_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT[]  =
{
    "CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_20B_E",
    "CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_24B_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT[]  =
{
    "CPSS_DXCH_FABRIC_HGL_DIRECTION_RX_E",
    "CPSS_DXCH_FABRIC_HGL_DIRECTION_TX_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT[]  =
{
    "CPSS_DXCH_FABRIC_HGL_ECC_ECC7_E",
    "CPSS_DXCH_FABRIC_HGL_ECC_CRC8_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, etherType);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, prioVlanId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cpId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, version);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, macLowByte);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_32_HEX_MAC(valPtr, etherType);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, pfcOpcode);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, up);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, qosProfile);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, trgDev);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, trgPort);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_FABRIC_HGL_RX_DSA_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_FABRIC_HGL_RX_DSA_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, vid);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, srcId);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_FABRIC_HGL_RX_ERROR_CNTRS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_FABRIC_HGL_RX_ERROR_CNTRS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, badLengthCells);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, badHeaderCells);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, reformatErrorCells);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_FABRIC_HGL_RX_DSA_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_FABRIC_HGL_RX_DSA_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_FABRIC_HGL_RX_DSA_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT_cellType = {
     "cellType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT_type = {
     "type", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT_cellDirection = {
     "cellDirection", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT_eccType = {
     "eccType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC_PTR_e2eParamsPtr = {
     "e2eParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC_PTR_llParamsPtr = {
     "llParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_FABRIC_HGL_RX_DSA_STC_PTR_dsaParamsPtr = {
     "dsaParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_FABRIC_HGL_RX_DSA_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_idleCntEnable = {
     "idleCntEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_maxIdleCnt = {
     "maxIdleCnt", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT_PTR_cellTypePtr = {
     "cellTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT_PTR_typePtr = {
     "typePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT_PTR_eccTypePtr = {
     "eccTypePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC_PTR_e2eParamsPtr = {
     "e2eParamsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC_PTR_llParamsPtr = {
     "llParamsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_FABRIC_HGL_RX_DSA_STC_PTR_dsaParamsPtr = {
     "dsaParamsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_FABRIC_HGL_RX_DSA_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_FABRIC_HGL_RX_ERROR_CNTRS_STC_PTR_rxErrorsCntrsPtr = {
     "rxErrorsCntrsPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_FABRIC_HGL_RX_ERROR_CNTRS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_idleCntEnablePtr = {
     "idleCntEnablePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_laneSyncOkArr = {
     "laneSyncOkArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_cellCntrPtr = {
     "cellCntrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_maxIdleCntPtr = {
     "maxIdleCntPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFabricHGLinkDescriptorTypeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT_type
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFabricHGLinkCellsCntrTypeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT_cellDirection,
    &DX_IN_CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT_cellType
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFabricHGLinkCellsCntrTypeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT_cellDirection,
    &DX_OUT_CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT_PTR_cellTypePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFabricHGLinkCellsCntrGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT_cellDirection,
    &DX_OUT_GT_U32_PTR_cellCntrPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFabricHGLinkConfigEccTypeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT_eccType
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFabricHGLinkFcRxE2eParamsSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC_PTR_e2eParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFabricHGLinkRxLinkLayerFcParamsSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC_PTR_llParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFabricHGLinkRxDsaParamsSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_CPSS_DXCH_FABRIC_HGL_RX_DSA_STC_PTR_dsaParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFabricHGLinkPcsMaxIdleCntEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_BOOL_idleCntEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFabricHGLinkFcMacSaSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_ETHERADDR_PTR_macPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFabricHGLinkPcsMaxIdleCntSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_IN_GT_U32_maxIdleCnt
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFabricHGLinkDescriptorTypeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT_PTR_typePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFabricHGLinkConfigEccTypeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT_PTR_eccTypePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFabricHGLinkFcRxE2eParamsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC_PTR_e2eParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFabricHGLinkRxLinkLayerFcParamsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC_PTR_llParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFabricHGLinkRxDsaParamsGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_FABRIC_HGL_RX_DSA_STC_PTR_dsaParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFabricHGLinkRxErrorCntrGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_CPSS_DXCH_FABRIC_HGL_RX_ERROR_CNTRS_STC_PTR_rxErrorsCntrsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_BOOL_PTR_idleCntEnablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFabricHGLinkFcMacSaGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_ETHERADDR_PTR_macPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFabricHGLinkPcsMaxIdleCntGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PHYSICAL_PORT_NUM_portNum,
    &DX_OUT_GT_U32_PTR_maxIdleCntPtr
};
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChFabricHGLinkPcsRxStatusGet_PARAMS[];


/********* lib API DB *********/

extern void cpssDxChFabricHGLinkPcsRxStatusGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChFabricHGLinkRxErrorCntrGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChFabricLogLibDb[] = {
    {"cpssDxChFabricHGLinkPcsLoopbackEnableSet", 3, prvCpssLogGenDevNumPortNumEnable_PARAMS, NULL},
    {"cpssDxChFabricHGLinkPcsLoopbackEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr_PARAMS, NULL},
    {"cpssDxChFabricHGLinkPcsMaxIdleCntEnableSet", 3, cpssDxChFabricHGLinkPcsMaxIdleCntEnableSet_PARAMS, NULL},
    {"cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet", 3, cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet_PARAMS, NULL},
    {"cpssDxChFabricHGLinkPcsMaxIdleCntSet", 3, cpssDxChFabricHGLinkPcsMaxIdleCntSet_PARAMS, NULL},
    {"cpssDxChFabricHGLinkPcsMaxIdleCntGet", 3, cpssDxChFabricHGLinkPcsMaxIdleCntGet_PARAMS, NULL},
    {"cpssDxChFabricHGLinkPcsRxStatusGet", 3, cpssDxChFabricHGLinkPcsRxStatusGet_PARAMS, cpssDxChFabricHGLinkPcsRxStatusGet_preLogic},
    {"cpssDxChFabricHGLinkRxErrorCntrGet", 3, cpssDxChFabricHGLinkRxErrorCntrGet_PARAMS, cpssDxChFabricHGLinkRxErrorCntrGet_preLogic},
    {"cpssDxChFabricHGLinkConfigEccTypeSet", 3, cpssDxChFabricHGLinkConfigEccTypeSet_PARAMS, NULL},
    {"cpssDxChFabricHGLinkConfigEccTypeGet", 3, cpssDxChFabricHGLinkConfigEccTypeGet_PARAMS, NULL},
    {"cpssDxChFabricHGLinkRxCrcCheckEnableSet", 3, prvCpssLogGenDevNumPortNumEnable_PARAMS, NULL},
    {"cpssDxChFabricHGLinkRxCrcCheckEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr_PARAMS, NULL},
    {"cpssDxChFabricHGLinkLbiEnableSet", 3, prvCpssLogGenDevNumPortNumEnable_PARAMS, NULL},
    {"cpssDxChFabricHGLinkLbiEnableGet", 3, prvCpssLogGenDevNumPortNumEnablePtr_PARAMS, NULL},
    {"cpssDxChFabricHGLinkCellsCntrTypeSet", 4, cpssDxChFabricHGLinkCellsCntrTypeSet_PARAMS, NULL},
    {"cpssDxChFabricHGLinkCellsCntrTypeGet", 4, cpssDxChFabricHGLinkCellsCntrTypeGet_PARAMS, NULL},
    {"cpssDxChFabricHGLinkCellsCntrGet", 4, cpssDxChFabricHGLinkCellsCntrGet_PARAMS, NULL},
    {"cpssDxChFabricHGLinkFcMacSaSet", 3, cpssDxChFabricHGLinkFcMacSaSet_PARAMS, NULL},
    {"cpssDxChFabricHGLinkFcMacSaGet", 3, cpssDxChFabricHGLinkFcMacSaGet_PARAMS, NULL},
    {"cpssDxChFabricHGLinkRxDsaParamsSet", 3, cpssDxChFabricHGLinkRxDsaParamsSet_PARAMS, NULL},
    {"cpssDxChFabricHGLinkRxDsaParamsGet", 3, cpssDxChFabricHGLinkRxDsaParamsGet_PARAMS, NULL},
    {"cpssDxChFabricHGLinkFcRxE2eParamsSet", 3, cpssDxChFabricHGLinkFcRxE2eParamsSet_PARAMS, NULL},
    {"cpssDxChFabricHGLinkFcRxE2eParamsGet", 3, cpssDxChFabricHGLinkFcRxE2eParamsGet_PARAMS, NULL},
    {"cpssDxChFabricHGLinkRxLinkLayerFcParamsSet", 3, cpssDxChFabricHGLinkRxLinkLayerFcParamsSet_PARAMS, NULL},
    {"cpssDxChFabricHGLinkRxLinkLayerFcParamsGet", 3, cpssDxChFabricHGLinkRxLinkLayerFcParamsGet_PARAMS, NULL},
    {"cpssDxChFabricHGLinkDescriptorTypeSet", 3, cpssDxChFabricHGLinkDescriptorTypeSet_PARAMS, NULL},
    {"cpssDxChFabricHGLinkDescriptorTypeGet", 3, cpssDxChFabricHGLinkDescriptorTypeGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_FABRIC(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChFabricLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChFabricLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

