/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssGenPpTypesManualLog.c
*       Manually implemented CPSS Log type wrappers
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/common/port/cpssPortManager.h>

#ifndef CHX_FAMILY
void prvCpssLogParamFuncStc_GT_IPV6ADDR_PTR
(
    IN    CPSS_LOG_LIB_ENT                        contextLib,
    IN    CPSS_LOG_TYPE_ENT                       logType,
    IN    GT_CHAR_PTR                             namePtr,
    IN    void                                  * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_FIELD_VAL_PTR_MAC(GT_IPV6ADDR *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    prvCpssLogStcIpV6(contextLib,logType,namePtr, valPtr->arIP);
}

void prvCpssLogParamFuncStc_GT_IPADDR_PTR
(
    IN    CPSS_LOG_LIB_ENT                        contextLib,
    IN    CPSS_LOG_TYPE_ENT                       logType,
    IN    GT_CHAR_PTR                             namePtr,
    IN    void                                  * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_FIELD_VAL_PTR_MAC(GT_IPADDR *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    prvCpssLogStcIpV4(contextLib,logType,namePtr, valPtr->arIP);
}

void prvCpssLogParamFuncStc_GT_ETHERADDR_PTR
(
    IN    CPSS_LOG_LIB_ENT                        contextLib,
    IN    CPSS_LOG_TYPE_ENT                       logType,
    IN    GT_CHAR_PTR                             namePtr,
    IN    void                                  * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_FIELD_VAL_PTR_MAC(GT_ETHERADDR *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    prvCpssLogStcMac(contextLib, logType, namePtr, valPtr->arEther);
}

void prvCpssLogParamFuncStc_CPSS_PORTS_BMP_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PORTS_BMP_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    inOutParamInfoPtr->paramKey.paramKeyArr[2] = CPSS_MAX_PORTS_BMP_NUM_CNS;
    prvCpssLogArrayOfParamsHandle(contextLib,logType,"ports",(void*) valPtr->ports, sizeof(GT_U32), inOutParamInfoPtr,
                              prvCpssLogParamFuncStc_GT_U32_HEX_PTR);
    prvCpssLogStcLogEnd(contextLib, logType);
}

void prvCpssLogParamFuncStc_CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, loopbackType, CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT);
    switch (valPtr->loopbackType)
    {
        case CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E:
            PRV_CPSS_LOG_STC_BOOL_MAC((&valPtr->loopbackMode), macLoopbackEnable);
            break;
        case CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E:
            PRV_CPSS_LOG_STC_ENUM_MAC((&valPtr->loopbackMode), serdesLoopbackMode, CPSS_PORT_SERDES_LOOPBACK_MODE_ENT);
            break;
        case CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E:
            /* nothing to add here. the attributeType 'LAST' was logged before the 'switch' scope at
               PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, loopbackType, CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT);*/
            break;
    }
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, enableRegularTrafic);
    prvCpssLogStcLogEnd(contextLib, logType);
}

#endif
