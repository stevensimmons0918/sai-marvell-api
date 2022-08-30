/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssGenPhyLog.c
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
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/phy/cpssGenPhySmi.h>
#include <cpss/generic/phy/cpssGenPhyVct.h>
#include <cpss/generic/phy/private/prvCpssGenPhyLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_PHY_ERRATA_WA_ENT[]  =
{
    "CPSS_PHY_ERRATA_WA_88E1240_SGMII_LOCKUP_E",
    "CPSS_PHY_ERRATA_WA_88E1340_BGA_INIT_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_PHY_ERRATA_WA_ENT);
const char * const prvCpssLogEnum_CPSS_VCT_ACTION_ENT[]  =
{
    "CPSS_VCT_START_E",
    "CPSS_VCT_GET_RES_E",
    "CPSS_VCT_ABORT_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_VCT_ACTION_ENT);
const char * const prvCpssLogEnum_CPSS_VCT_CABLE_LEN_ENT[]  =
{
    "CPSS_VCT_CABLE_LESS_10M_E",
    "CPSS_VCT_CABLE_GREATER_10M_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_VCT_CABLE_LEN_ENT);
const char * const prvCpssLogEnum_CPSS_VCT_DOWNSHIFT_STATUS_ENT[]  =
{
    "CPSS_VCT_NO_DOWNSHIFT_E",
    "CPSS_VCT_DOWNSHIFT_E",
    "CPSS_VCT_NOT_APPLICABLE_DOWNSHIFT_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_VCT_DOWNSHIFT_STATUS_ENT);
const char * const prvCpssLogEnum_CPSS_VCT_NORMAL_CABLE_LEN_ENT[]  =
{
    "CPSS_VCT_LESS_THAN_50M_E",
    "CPSS_VCT_50M_80M_E",
    "CPSS_VCT_80M_110M_E",
    "CPSS_VCT_110M_140M_E",
    "CPSS_VCT_MORE_THAN_140_E",
    "CPSS_VCT_UNKNOWN_LEN_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_VCT_NORMAL_CABLE_LEN_ENT);
const char * const prvCpssLogEnum_CPSS_VCT_PHY_TYPES_ENT[]  =
{
    "CPSS_VCT_PHY_100M_E",
    "CPSS_VCT_PHY_1000M_E",
    "CPSS_VCT_PHY_10000M_E",
    "CPSS_VCT_PHY_1000M_B_E",
    "CPSS_VCT_PHY_1000M_MP_E",
    "CPSS_VCT_PHY_1000M_MP_NO_FIBER_E",
    "CPSS_VCT_PHY_1000M_MP_NO_FIBER_NG_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_VCT_PHY_TYPES_ENT);
const char * const prvCpssLogEnum_CPSS_VCT_PAIR_SWAP_ENT[]  =
{
    "CPSS_VCT_CABLE_STRAIGHT_E",
    "CPSS_VCT_CABLE_CROSSOVER_E",
    "CPSS_VCT_NOT_APPLICABLE_SWAP_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_VCT_PAIR_SWAP_ENT);
const char * const prvCpssLogEnum_CPSS_VCT_POLARITY_SWAP_ENT[]  =
{
    "CPSS_VCT_POSITIVE_POLARITY_E",
    "CPSS_VCT_NEGATIVE_POLARITY_E",
    "CPSS_VCT_NOT_APPLICABLE_POLARITY_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_VCT_POLARITY_SWAP_ENT);
const char * const prvCpssLogEnum_CPSS_VCT_TEST_STATUS_ENT[]  =
{
    "CPSS_VCT_TEST_FAIL_E",
    "CPSS_VCT_NORMAL_CABLE_E",
    "CPSS_VCT_OPEN_CABLE_E",
    "CPSS_VCT_SHORT_CABLE_E",
    "CPSS_VCT_IMPEDANCE_MISMATCH_E",
    "CPSS_VCT_SHORT_WITH_PAIR0_E",
    "CPSS_VCT_SHORT_WITH_PAIR1_E",
    "CPSS_VCT_SHORT_WITH_PAIR2_E",
    "CPSS_VCT_SHORT_WITH_PAIR3_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_VCT_TEST_STATUS_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_VCT_CABLE_EXTENDED_STATUS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_VCT_CABLE_EXTENDED_STATUS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, vctExtendedCableStatus, CPSS_VCT_EXTENDED_STATUS_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, accurateCableLen, CPSS_VCT_ACCURATE_CABLE_LEN_STC);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, twoPairDownShift, CPSS_VCT_DOWNSHIFT_STATUS_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_VCT_CABLE_STATUS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_VCT_CABLE_STATUS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, cableStatus, CPSS_VCT_MDI_PAIR_NUM_CNS, CPSS_VCT_STATUS_STC);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, normalCableLen, CPSS_VCT_NORMAL_CABLE_LEN_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, phyType, CPSS_VCT_PHY_TYPES_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_VCT_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_VCT_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, length, CPSS_VCT_CABLE_LEN_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_VCT_ACCURATE_CABLE_LEN_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_VCT_ACCURATE_CABLE_LEN_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, isValid, CPSS_VCT_MDI_PAIR_NUM_CNS, GT_BOOL);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, cableLen, CPSS_VCT_MDI_PAIR_NUM_CNS, GT_U16);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_VCT_EXTENDED_STATUS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_VCT_EXTENDED_STATUS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isValid);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, pairSwap, CPSS_VCT_CHANNEL_PAIR_NUM_CNS, CPSS_VCT_PAIR_SWAP_ENT);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, pairPolarity, CPSS_VCT_MDI_PAIR_NUM_CNS, CPSS_VCT_POLARITY_SWAP_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, pairSkew, CPSS_VCT_PAIR_SKEW_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_VCT_STATUS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_VCT_STATUS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, testStatus, CPSS_VCT_TEST_STATUS_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, errCableLen);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_VCT_PAIR_SKEW_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_VCT_PAIR_SKEW_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, isValid);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, skew, CPSS_VCT_MDI_PAIR_NUM_CNS, GT_U32);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_VCT_PAIR_SWAP_ENT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_FIELD_VAL_PTR_MAC(CPSS_VCT_PAIR_SWAP_ENT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *valPtr, CPSS_VCT_PAIR_SWAP_ENT);
}
void prvCpssLogParamFuncStc_CPSS_VCT_POLARITY_SWAP_ENT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_FIELD_VAL_PTR_MAC(CPSS_VCT_POLARITY_SWAP_ENT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *valPtr, CPSS_VCT_POLARITY_SWAP_ENT);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_PHY_ERRATA_WA_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_PHY_ERRATA_WA_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_PHY_ERRATA_WA_ENT);
}
void prvCpssLogParamFunc_CPSS_VCT_ACTION_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_VCT_ACTION_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_VCT_ACTION_ENT);
}
void prvCpssLogParamFunc_CPSS_VCT_CABLE_EXTENDED_STATUS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_VCT_CABLE_EXTENDED_STATUS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_VCT_CABLE_EXTENDED_STATUS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_VCT_CABLE_STATUS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_VCT_CABLE_STATUS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_VCT_CABLE_STATUS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_VCT_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_VCT_CONFIG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_VCT_CONFIG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}

