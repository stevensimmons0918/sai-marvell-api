/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/*******************************************************************************
* @file cpssHalQos.c
*
* @brief Private API implementation for CPSS Qos feature which can be used in
*        XPS layer.
*
* @version   01
*******************************************************************************/

#include "cpssHalQos.h"
#include "cpssHalCounter.h"
#include "xpsCommon.h"
#include "cpssHalDevice.h"
#include "cpssDxChPortTx.h"
#include "cpssDxChPortBufMg.h"
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>
#include "cpssDxChPortMapping.h"
#include "cpssDxChPortPfc.h"
#include "cpssDxChCos.h"
#include "cpssDxChCscd.h"
#include "cpssHalUtil.h"
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChRegs.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqPfcc.h>
#include "cpssHalCopp.h"
#include "cpssHalMirror.h"
#include "cpssHalSys.h"
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define CONVERT_TO_64BIT_INT(cnt) ((uint64_t)cnt.l[1]<<32)|cnt.l[0]
#else
#define CONVERT_TO_64BIT_INT(cnt) ((uint64_t)cnt.l[0]<<32)|cnt.l[1]
#endif

#define SAI2CPSS_PORT_CONVERT(_saiDevNum,_saiPortNumber)\
do\
{\
    GT_U32 origSaiDevNum = _saiDevNum;\
    cpssHalSetDeviceSwitchId(_saiDevNum);\
    _saiDevNum = xpsGlobalIdToDevId(_saiDevNum, _saiPortNumber);\
    _saiPortNumber = xpsGlobalPortToPortnum(origSaiDevNum, _saiPortNumber);\
}\
while(0);\

#define MAX_POOL_NUM 2

#define CPSSHAL_TD_POOL_LIMIT_WA_RESERVED_POOL_INDX 0

GT_32 queueStatCncBlockNum_g = 0;
GT_32 queueWatermarkCncBlockNum_g = 0;

/*In Falcon architecture there is no ingress/egress pools ,same pool  is function as ingress and egress.
In order to satisfy SAI model the structure below overshadow this .
The HW should contain the smalles value between egress and ingress*/
typedef struct
{

    GT_U32  ingressAvaileble;
    GT_U32  ingressLimit;
    GT_U32  egressAvaileble;
    GT_U32  egressLimit;
} CPSS_HAL_VIRT_POOL_STC;

CPSS_HAL_VIRT_POOL_STC virtualPoolDb[MAX_POOL_NUM]=
{
    {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}, {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}
};

GT_STATUS cpssHalQosBindReasonCodeToCpuQueue(uint32_t devId,
                                             CPSS_NET_RX_CPU_CODE_ENT reasonCode, uint32_t queueNum)
{
    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC entryInfo;
    GT_STATUS status;
    GT_U32    cpssDevNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        /* Read the cpu code table entry */
        status = cpssDxChNetIfCpuCodeTableGet(cpssDevNum, reasonCode, &entryInfo);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to read cpu code table entry, "
                  "rc:%d, dev:%d, RC:%d\n", status, cpssDevNum, reasonCode);
            return status;
        }

        /* Modify the entry */
        entryInfo.tc = (GT_U8)queueNum;

        /* Write the cpu code table entry back */
        status = cpssDxChNetIfCpuCodeTableSet(cpssDevNum, reasonCode, &entryInfo);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to write cpu code table entry, "
                  "rc:%d, dev:%d, RC:%\n", status, cpssDevNum, reasonCode);
            return status;
        }
    }

    return GT_OK;
}

GT_STATUS cpssHalQosGetReasonCodeToCpuQueue(uint32_t devId,
                                            CPSS_NET_RX_CPU_CODE_ENT reasonCode, uint32_t *queueNum)
{
    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC entryInfo;
    GT_STATUS status;

    /* Read the cpu code table entry for given reason code */
    status = cpssDxChNetIfCpuCodeTableGet(devId, reasonCode, &entryInfo);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to read cpu code table entry, "
              "rc:%d, dev:%d, RC:%d\n", status, devId, reasonCode);
        return status;
    }

    *queueNum = entryInfo.tc;

    return GT_OK;
}

/**
* @internal cpssHalPortTxSharedPoolLimitsSet function
* @endinternal
*
* @brief   Set maximal descriptors and buffers limits for shared pool.
*
* @param[in] devNum                   - physical device number
* @param[in] poolNum                  - Shared pool number.
*                                       Falcon :Range 0..1
* @param[in] maxBufNum                - The number of buffers allocated for a shared pool.
*                                       Falcon:Range   0..0xFFFFF
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - on out of range value
**/

GT_STATUS cpssHalPortTxSharedPoolLimitsSet
(
    uint32_t devId,
    uint32_t poolNum,
    uint32_t maxBufNum,
    GT_BOOL  isIngress
)
{
    GT_STATUS status;
    GT_U32 maxDescNum=0;
    GT_U8 cpssDevNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        if (poolNum>=MAX_POOL_NUM)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Pool num is too big %d "
                  "should be smaller then %d\n", poolNum, MAX_POOL_NUM);
            return GT_OUT_OF_RANGE;
        }
        if (GT_TRUE == isIngress)
        {
            virtualPoolDb[poolNum].ingressLimit= maxBufNum;
        }
        else
        {
            virtualPoolDb[poolNum].egressLimit= maxBufNum;
        }
        maxBufNum = MIN(virtualPoolDb[poolNum].ingressLimit,
                        virtualPoolDb[poolNum].egressLimit);

        if (poolNum == CPSSHAL_TD_POOL_LIMIT_WA_RESERVED_POOL_INDX)
        {
            /* Default Pool WA- CPSS-11271
             * TD limit for reserved pool is taken care by CPSS
             * pool-0 = 2*PB - pool-1 */
            continue;
        }

        status = cpssDxChPortTxSharedPoolLimitsSet(cpssDevNum, poolNum, maxBufNum,
                                                   maxDescNum);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set shared pool limit set, "
                  "status:%d, cpssdev:%d, poolNum:%d, maxBufNum:%d\n", status, cpssDevNum,
                  poolNum, maxBufNum);
            return status;
        }
    }
    return GT_OK;
}

/**
* @internal  cpssHalPortTxTailDropGlobalParamsSet function
* @endinternal
*
* @brief  Set amount of available buffers for dynamic buffers allocation
*
* @param[in] devNum                   - Device number
* @param[in] resourceMode             - Defines which free buffers resource is used to calculate
*                                       the dynamic buffer limit for the Port/Queue limits(global
*                                       or pool)
* @param[in]  globalAvailableBuffers   - Global amount of available buffers for dynamic buffers allocation
* @param[in]  pool0AvailableBuffers    - Pool 0 amount of available buffers for dynamic buffers allocation
* @param[in]  pool1AvailableBuffers    - Pool 1 amount of available buffers for dynamic buffers allocation
*
* @retval GT_OK                       - on success
* @retval GT_FAIL                     - on error
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - on invalid input parameters value
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
* @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/

GT_STATUS cpssHalPortTxTailDropGlobalParamsSet(uint32_t devId,
                                               CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT resourcemode,
                                               uint32_t globalAvailableBuffers, uint32_t pool0AvailableBuffers,
                                               uint32_t pool1AvailableBuffers)
{
    GT_STATUS status;
    GT_U8 cpssDevNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        status = cpssDxChPortTxTailDropGlobalParamsSet(cpssDevNum, resourcemode,
                                                       globalAvailableBuffers, pool0AvailableBuffers, pool1AvailableBuffers);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set tail drop global params, "
                  "status:%d, cpssdev:%d, globalavailablebuffers:%d, globalAvailableBuffers:%d, pool1AvailableBuffers:%d\n",
                  status, cpssDevNum, globalAvailableBuffers, pool0AvailableBuffers,
                  pool1AvailableBuffers);
            return status;
        }
    }
    return GT_OK;
}

GT_STATUS cpssHalPortTxTailDropConfigureAvaileblePool
(
    uint32_t devId,
    uint32_t poolId,
    uint32_t poolAvailableBuffers,
    GT_BOOL  isIngress
)
{
    GT_STATUS status;
    GT_U8 cpssDevNum = 0;
    GT_U32 globalAvailableBuffers;
    GT_U32 pool0AvailableBuffers, pool1AvailableBuffers;
    CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_ENT resourcemode;


    if (poolId>=MAX_POOL_NUM)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Pool num is too big %d "
              "should be smaller then %d\n", poolId, MAX_POOL_NUM);
        return GT_OUT_OF_RANGE;
    }
    if (GT_TRUE == isIngress)
    {
        virtualPoolDb[poolId].ingressAvaileble= poolAvailableBuffers;
    }
    else
    {
        virtualPoolDb[poolId].egressAvaileble= poolAvailableBuffers;
    }

    poolAvailableBuffers = MIN(virtualPoolDb[poolId].ingressAvaileble,
                               virtualPoolDb[poolId].egressAvaileble);

    status = cpssDxChPortTxTailDropGlobalParamsGet(cpssDevNum, &resourcemode,
                                                   &globalAvailableBuffers, &pool0AvailableBuffers, &pool1AvailableBuffers);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set tail drop global params, "
              "status:%d, cpssdev:%d, globalavailablebuffers:%d, globalAvailableBuffers:%d, pool1AvailableBuffers:%d\n",
              status, cpssDevNum, globalAvailableBuffers, pool0AvailableBuffers,
              pool1AvailableBuffers);
        return status;
    }

    switch (poolId)
    {
        case 0:
            pool0AvailableBuffers = poolAvailableBuffers;
            break;
        case 1:
            pool1AvailableBuffers = poolAvailableBuffers;
            break;
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "unsupported pool id :%d\n",
                  poolId);
            break;
    }

    resourcemode = CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_POOL_E;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        status = cpssDxChPortTxTailDropGlobalParamsSet(cpssDevNum, resourcemode,
                                                       globalAvailableBuffers, pool0AvailableBuffers, pool1AvailableBuffers);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set tail drop global params, "
                  "status:%d, cpssdev:%d, globalavailablebuffers:%d, globalAvailableBuffers:%d, pool1AvailableBuffers:%d\n",
                  status, cpssDevNum, globalAvailableBuffers, pool0AvailableBuffers,
                  pool1AvailableBuffers);
            return status;
        }
    }
    return GT_OK;
}

/**
* @internal cpssHalPortTx4TcTailDropProfileSet function
* @endinternal
*
* @brief   Set tail drop profiles limits for particular TC.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                       Drop Parameters (0..7).For Falcon only (0..15)
* @param[in] tailDropProfileParamsPtr -
*                                      the Drop Profile Parameters to associate
*                                      with the Traffic Class in this Profile set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
**/

GT_STATUS cpssHalPortTx4TcTailDropProfileSet(uint32_t devId,
                                             CPSS_PORT_TX_DROP_PROFILE_SET_ENT profileSet,
                                             uint8_t trafficClass,
                                             CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS_STC *tailDropProfileParamsPtr)
{
    GT_STATUS status;
    GT_U8 cpssDevNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        status = cpssDxChPortTx4TcTailDropProfileSet(cpssDevNum, profileSet,
                                                     trafficClass, tailDropProfileParamsPtr);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set port tx tc tail drop profile set, "
                  "status:%d, cpssdev:%d, profileset:%d, trafficClass:%d\n",
                  status, cpssDevNum, profileSet, trafficClass);
            return status;
        }
    }
    return GT_OK;
}

/**
* @internal cpssHalPortPfcGlobalTcThresholdSet function
* @endinternal
*
* @brief   Set global TC enable and threshold
*  When enable, once Global TC counter is above the calculated threshold,
*  PFC OFF message is sent to all ports, for the specific TC.
*
* @param[in] devNum             - device number.
* @param[in] tc                        - Traffic class[0..7]
* @param[in] enable               - Global TC PFC enable option.
* @param[in] thresholdCfgPtr - (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             -wrong input parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
**/

GT_STATUS cpssHalPortPfcGlobalTcThresholdSet(uint32_t devId, uint8_t tc,
                                             GT_BOOL enable, CPSS_DXCH_PFC_THRESHOLD_STC *thresholdCfgPtr,
                                             CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr)
{
    GT_STATUS status;
    GT_U8 cpssDevNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        status = cpssDxChPortPfcGlobalTcThresholdSet(cpssDevNum, tc, enable,
                                                     thresholdCfgPtr, hysteresisCfgPtr);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set pfc Global tc Threshold Set, "
                  "status:%d, dev:%d, tc:%d, enable:%d\n", status, cpssDevNum, tc, enable);
            return status;
        }
    }
    return GT_OK;
}

/**
* @internal cpssHalPortPfcPortThresholdSet function
* @endinternal
*
* @brief   Sets  PFC OFF port limit. When Flow Control is enabled, PFC-OFF packet sent
*         when the number of buffers for this port is more than thresholdCfg limit.
*
* @param[in] devNum                   -       physical device number
* @param[in] portNum                        - physical port number
* @param[in] enable                -          Global port PFC enable option.
* @param[in] thresholdCfgPtr -        (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
**/

GT_STATUS cpssHalPortPfcPortThresholdSet(uint8_t  devId,
                                         GT_PHYSICAL_PORT_NUM portNum, GT_BOOL enable,
                                         CPSS_DXCH_PFC_THRESHOLD_STC *thresholdCfgPtr,
                                         CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr)
{
    GT_STATUS status = GT_OK;
    GT_U8     cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    status = cpssDxChPortPfcPortThresholdSet(cpssDevNum, cpssPortNum, enable,
                                             thresholdCfgPtr, hysteresisCfgPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set pfc port  Threshold Set, "
              "status:%d, dev:%d, portNum:%d, enable:%d\n", status, cpssDevNum, cpssPortNum,
              enable);
        return status;
    }
    return GT_OK;
}

/**
* @internal cpssHalPortXonLimitSet function
* @endinternal
*
* @brief   Sets X-ON port limit. When Flow Control is enabled, X-ON packet sent
*         when the number of buffers for this port is less than xonLimit.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] xonLimit                 - X-ON limit in resolution of 2 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Since in CH3 the HW resolution is 8, in case the xonLimit input
*       parameter is not a multiple of 8 we round it UP to the nearest one.
*       For Bobcat2 buffer resolution in HW is 1.
**/

GT_STATUS cpssHalPortXonLimitSet(uint8_t devId,
                                 CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet, uint32_t xonLimit)
{
#if 0
    GT_STATUS status;
#endif
    GT_U8 cpssDevNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
#if 0
        /* Commented for now as this api is yet to be supported in cpss ref:CPSS_TBD_BOOKMARK_FALCON */
        status = cpssDxChPortXonLimitSet(cpssDevNum, profileSet, xonLimit);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set port xon limit set, "
                  "status:%d, dev:%d, xonLimit%d\n", status, cpssDevNum, xonLimit);
            return status;
        }
#endif

    }
    return GT_OK;
}

/**
* @internal cpssHalPortXoffLimitSet function
* @endinternal
*
* @brief   Sets X-OFF port limit. When Flow Control is enabled, X-OFF packet sent
*         when the number of buffers for this port is less than xoffLimit.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] xoffLimit                - X-OFF limit in resolution of 2 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Since in CH3 the HW resolution is 8, in case the xoffLimit input
*       parameter is not a multiple of 8 we round it UP to the nearest one.
*       For Bobcat2 buffer resolution in HW is 1.
**/

GT_STATUS cpssHalPortXoffLimitSet(uint8_t devId,
                                  CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet, uint32_t xoffLimit)
{
#if 0
    GT_STATUS status;
#endif
    GT_U8 cpssDevNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
#if 0
        /* Commented for now as this api is yet to be supported in cpss ref:CPSS_TBD_BOOKMARK_FALCON */
        status = cpssDxChPortXoffLimitSet(cpssDevNum, profileSet, xoffLimit);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set port xoff limit set, "
                  "status:%d, dev:%d, xoffLimit:%d\n", status, cpssDevNum, xoffLimit);
            return status;
        }
#endif
    }
    return GT_OK;
}

/**
* @internal cpssHalPortRxBufLimitSet function
* @endinternal
*
* @brief   Sets receive buffer limit threshold for the specified port.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] rxBufLimit               - buffer limit threshold in resolution of 4 buffers.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Since in CH3 and above, the HW resolution is 16, in case the rxBufLimit
*       input parameter is not a multiple of 16 we round it UP to the nearest one.
*       For Bobcat2 buffer resolution in HW is 1.
**/

GT_STATUS cpssHalPortRxBufLimitSet(uint8_t devId,
                                   CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet, uint32_t rxBufLimit)
{
#if 0
    GT_STATUS status;
#endif
    GT_U8 cpssDevNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
#if 0
        /* Commented for now as this api is yet to be supported in cpss ref:CPSS_TBD_BOOKMARK_FALCON */
        status = cpssDxChPortRxBufLimitSet(cpssDevNum, profileSet, rxBufLimit);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set port Rx Buf limit set, "
                  "status:%d, dev:%d, rxBufLimit:%d\n", status, cpssDevNum,  rxBufLimit);
            return status;
        }
#endif
    }
    return GT_OK;
}

/**
* @internal cpssHalPortRxFcProfileSet function
* @endinternal
*
* @brief   Bind a port to a flow control profile.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
**/

GT_STATUS cpssHalPortRxFcProfileSet(uint8_t devId, GT_PHYSICAL_PORT_NUM portNum,
                                    CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet)
{
#if 0
    GT_STATUS status = GT_OK;
#endif
    GT_U8     cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }
#if 0
    /* Commented for now as this api is yet to be supported in cpss ref:CPSS_TBD_BOOKMARK_FALCON */
    status = cpssDxChPortRxFcProfileSet(cpssDevNum, cpssPortNum, profileSet);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set Port Rx Fc Profile Set, "
              "status:%d, dev:%d, portNum:%d\n", status, cpssDevNum, cpssPortNum);
        return status;
    }
#endif
    return GT_OK;
}

GT_STATUS cpssHalGetNumberOfQueuesPerPort(uint8_t devId, uint32_t port,
                                          uint32_t* numQs)
{
    /* TODO: HACK: Need to find cpss api to get number of queues per port */
    *numQs = 16;

    return GT_OK;
}

/**
* @internal cpssHalCncQueueStatusLimitSet function
* @endinternal
*
* @brief   Set the limit of queue buffers consumption for triggering queue statistics counting.
*
* @param[in] devId            - device number.
* @param[in] profileSet        - profile set.
* @param[in] tcQueue           - traffic class queue (0..15)
* @param[in] queueLimit        - queue threshold limit.(APPLICABLE RANGES : 0 - 0xfffff)
*
* @retval GT_OK                - on success
* @retval GT_BAD_PTR           - on NULL pointer
* @retval GT_BAD_PARAM         - on wrong device number or profile or Tc.
* @retval GT_HW_ERROR          - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE  - on limit out of range
*/
GT_STATUS cpssHalCncQueueStatusLimitSet
(
    IN GT_U8                               devId,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN GT_U8                               tcQueue,
    IN GT_U32                              queueLimit
)
{
    GT_STATUS status;
    GT_U8 cpssDevNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        status = cpssDxChCncQueueStatusLimitSet(cpssDevNum, profileSet, tcQueue,
                                                queueLimit);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set queue threshold limit, "
                  "status:%d, cpssDevNum:%d, tcQueue:%d queueLimit:%d\n", status, cpssDevNum,
                  tcQueue, queueLimit);
            return status;
        }
    }
    return GT_OK;
}



/**
* @internal cpssHalPortTxBindPortToDpSet function
* @endinternal
*
* @brief   Bind a Physical Port to a specific Drop Profile Set.
*
* @param[in] devId                    - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssHalPortTxBindPortToDpSet
(
    IN  GT_U8                               devId,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet
)
{
    GT_STATUS status;

    SAI2CPSS_PORT_CONVERT(devId, portNum);

    {
        status = cpssDxChPortTxBindPortToDpSet(devId, portNum, profileSet);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set bind port to profile set, "
                  "status:%d, cpssDevNum:%d, portNum:%d profileSet:%d\n",
                  status, devId, portNum, profileSet);
            return status;
        }
    }
    return GT_OK;
}
/**
* @internal cpssHalPortTxBindPortToDpGet function
* @endinternal
*
* @brief   Bind a Physical Port to a specific Drop Profile Set.
*
* @param[in] devId                    - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssHalPortTxBindPortToDpGet
(
    IN  GT_U8                               devId,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   *profileSetPtr
)
{
    GT_STATUS status;


    SAI2CPSS_PORT_CONVERT(devId, portNum);

    status = cpssDxChPortTxBindPortToDpGet(devId, portNum, profileSetPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get bind port to profile set, "
              "status:%d, cpssDevNum:%d, portNum:%d profileSet:%d\n",
              status, devId, portNum);
        return status;
    }

    return GT_OK;
}


/**
* @internal cpssHalPortPfcCounterGet function
* @endinternal
*
* @brief   Get PFC counter value and watermark per PFC counter and traffic class.
*
* @param[in] devId                   - device number
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] pfcCounterNum            - PFC counter (APPLICABLE RANGES: 0..127)
* @param[out] pfcCounterValuePtr       - (pointer to) PFC counter value
* @param[out] pfcMaxValuePtr           - (pointer to) PFC watermark counter value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - on wrong pfcCounterNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
*@note    In SIP6 pfcCounterNum is used as physical port number.
*@note    In case tcQueue equal 0xFF the counter value refer to all TC's consumed by port.
*/
GT_STATUS cpssHalPortPfcCounterGet
(
    IN  GT_U8   devId,
    IN  GT_U8   tcQueue,
    IN  GT_U32  pfcCounterNum,
    OUT GT_U32  *pfcCounterValuePtr,
    OUT GT_U32  *pfcMaxValuePtr
)
{
    GT_STATUS status;
    GT_U8 cpssDevNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        status = cpssDxChPortPfcCounterGet(cpssDevNum, tcQueue, pfcCounterNum,
                                           pfcCounterValuePtr);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to get PFC counter value"
                  " per PFC counter and traffic class, status:%d, cpssDevNum:%d, tcQueue:%d"
                  " pfcCounterNum:%d\n", status, cpssDevNum, tcQueue, pfcCounterNum);
            return status;
        }

        /* HW TODO: HW may support pool watermark in the future, when we should modify here */
        *pfcMaxValuePtr = *pfcCounterValuePtr;
    }
    return GT_OK;
}

/**
* @internal cpssHalPortTx4TcTailDropProfileGet function
* @endinternal
*
* @brief   Get tail drop profiles limits for particular TC.
*
* @note   APPLICABLE DEVICES:      xCat3;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                                           Drop Parameters (0..7).For Falcon only (0..15)
*
* @param[out] tailDropProfileParamsPtr - Pointer to
*                                      the Drop Profile Parameters to associate
*                                      with the Traffic Class in this Profile.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalPortTx4TcTailDropProfileGet(uint32_t devId,
                                             CPSS_PORT_TX_DROP_PROFILE_SET_ENT profileSet,
                                             uint8_t trafficClass,
                                             CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS_STC *tailDropProfileParamsPtr)
{
    GT_STATUS status;
    GT_U8 cpssDevNum=0;

    status = cpssDxChPortTx4TcTailDropProfileGet(cpssDevNum, profileSet,
                                                 trafficClass, tailDropProfileParamsPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get port tx tc tail drop profile set, "
              "status:%d, cpssdev:%d, profileset:%d, trafficClass:%d\n",
              status, cpssDevNum, profileSet, trafficClass);
        return status;
    }

    return GT_OK;
}
/**
* @internal cpssHalPortPfcPortTcThresholdSet function
* @endinternal
*
* @brief   Sets  PFC OFF port/tc limit. When Flow Control is enabled, PFC-OFF packet sent
*         when the number of buffers for this port/TC  is more than thresholdCfg limit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -physical device number
* @param[in] portNum                    physical port number
* @param[in] tc                        -      Traffic class[0..7]
* @param[in] enable                       port/tc PFC enable option.
* @param[in] thresholdCfgPtr - (pointer to) threshold struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssHalPortPfcPortTcThresholdSet
(
    IN  GT_U8                              devNum,
    IN  GT_PHYSICAL_PORT_NUM               portNum,
    IN  GT_U8                              tc,
    IN  CPSS_DXCH_PFC_THRESHOLD_STC        *thresholdCfgPtr
)
{
    GT_STATUS status;
    CPSS_DXCH_PFC_THRESHOLD_STC currentThresholdCfg;
    GT_BOOL enable;

    SAI2CPSS_PORT_CONVERT(devNum, portNum);

    status = cpssDxChPortPfcPortTcThresholdGet(devNum, portNum, tc, &enable,
                                               &currentThresholdCfg);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to get  port PFC, "
              "status:%d, cpssdev:%d, port:%d, trafficClass:%d\n",
              status, devNum, portNum, tc);
        return status;
    }

    if ((currentThresholdCfg.alfa!=thresholdCfgPtr->alfa)||
        (currentThresholdCfg.guaranteedThreshold!=thresholdCfgPtr->guaranteedThreshold))
    {
        status = cpssDxChPortPfcPortTcThresholdSet(devNum, portNum, tc, enable,
                                                   thresholdCfgPtr);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to set  port PFC, "
                  "status:%d, cpssdev:%d, port:%d, trafficClass:%d\n",
                  status, devNum, portNum, tc);
            return status;
        }
    }

    return GT_OK;
}
/**
* @internal internal_cpssDxChPortPfcPortThresholdSet function
* @endinternal
*
* @brief   Sets  PFC OFF port limit. When Flow Control is enabled, PFC-OFF packet sent
*         when the number of buffers for this port is more than thresholdCfg limit.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3; Lion;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -       physical device number
* @param[in] portNum                        - physical port number
* @param[in] enable                -          Global port PFC enable option.
* @param[in] thresholdCfgPtr -        (pointer to) threshold struct
* @param[in] hysteresisCfgPtr - (pointer to) hysteresis struct
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssHalPortPfcPortPfcPortThresholdSet
(
    IN GT_U8                              devNum,
    IN GT_PHYSICAL_PORT_NUM               portNum,
    IN GT_BOOL                            enable,
    IN CPSS_DXCH_PFC_THRESHOLD_STC       *thresholdCfgPtr,
    IN CPSS_DXCH_PFC_HYSTERESIS_CONF_STC *hysteresisCfgPtr
)
{
    GT_STATUS status;

    SAI2CPSS_PORT_CONVERT(devNum, portNum);

    status = cpssDxChPortPfcPortThresholdSet(devNum, portNum, enable,
                                             thresholdCfgPtr, hysteresisCfgPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to set  port PFC, "
              "status:%d, cpssdev:%d, port:%d\n",
              status, devNum, portNum);
        return status;
    }

    return GT_OK;
}




/*
 * @internal cpssHalPortTxQueueTxEnableSet function
 * @endinternal
 *
 * @brief  Enable/Disable transmission from a Traffic Class queue
 *         on the specified port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tcQueue               - traffic class queue on this Physical Port
 * @param[in] enable                - GT_TRUE, enable transmission from the queue
 *                                     GT_FALSE, disable transmission from the queue
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortTxQueueTxEnableSet(uint32_t devId, uint32_t port,
                                        uint32_t tcQueue, GT_BOOL enable)
{
    GT_STATUS rc = GT_OK;
    GT_U8     cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    if (cpssHalDevPPFamilyGet(devId) != CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        return GT_OK;
    }
    cpssHalSetDeviceSwitchId(devId);

    /*
     * TODO: This API get port as CPU port num
     * For CPU Port Below 2 APIs will FAIL
     * Need to address in DeviceMgr
     *
     */

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    rc = cpssDxChPortTxQueueTxEnableSet(cpssDevNum, cpssPortNum, tcQueue, enable);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortTxQueueTxEnableSet dev %d port %d queue %d failed(%d)",
              cpssDevNum, cpssPortNum, tcQueue, rc);
        return rc;
    }

    return rc;
}

/**
 * @internal cpssHalPortTxQueueSchedulerTypeSet function
 * @endinternal
 *
 * @brief  Set Traffic Class Queue scheduling type -SP/DWRR on
 *         specificed port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tcQueue               - traffic class queue on this Physical Port
 * @param[in] arbGroup              - scheduling arbitration group:
 *                                     1) Strict Priority
 *                                     2) WRR Group - 0 (DWRR)
 *
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortTxQueueSchedulerTypeSet(uint32_t devId, uint32_t port,
                                             uint32_t tcQueue, CPSS_PORT_TX_Q_ARB_GROUP_ENT arbGroup)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;
    CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT profile;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    rc = cpssDxChPortTxBindPortToSchedulerProfileGet(cpssDevNum, cpssPortNum,
                                                     &profile);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortTxBindPortToSchedulerProfileGet dev %d port %d failed(%d)",
              cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    rc = cpssDxChPortTxQArbGroupSet(cpssDevNum, tcQueue, arbGroup, profile);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortTxQArbGroupSet dev %d queue %d failed(%d)",
              cpssDevNum, tcQueue, rc);
        return rc;
    }

    return rc;
}

/**
 * @internal cpssHalPortTxQueueWRRWeightSet function
 * @endinternal
 *
 * @brief  Set Traffic Class Queue DWRR weight on
 *         specificed port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tcQueue               - traffic class queue on this Physical Port
 * @param[in] weight                - wrr weight
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortTxQueueWRRWeightSet(uint32_t devId, uint32_t port,
                                         uint32_t tcQueue, uint32_t weight)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;
    CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT profile;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    rc = cpssDxChPortTxBindPortToSchedulerProfileGet(cpssDevNum, cpssPortNum,
                                                     &profile);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortTxBindPortToSchedulerProfileGet dev %d port %d failed(%d)",
              cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    rc = cpssDxChPortTxQWrrProfileSet(cpssDevNum, tcQueue, weight, profile);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortTxQWrrProfileSet dev %d queue %d failed(%d)",
              cpssDevNum, tcQueue, rc);
        return rc;
    }

    return rc;
}

/**
 * @internal cpssHalBindPortToSchedulerProfileGet function
 * @endinternal
 *
 * @brief  Get scheduler profile of a port.
 *
 * @param[in]  devId                - switch Id
 * @param[in]  port                 - port number
 * @param[out] profile              - The Profile Set in which the scheduler's parameters are
 *                                    associated.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalBindPortToSchedulerProfileGet(uint32_t devId, uint32_t port,
                                               uint32_t *profile)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    rc = cpssDxChPortTxBindPortToSchedulerProfileGet(cpssDevNum, cpssPortNum,
                                                     profile);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortTxBindPortToSchedulerProfileGet dev %d port %d failed(%d)",
              cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    return rc;
}

/**
 * @internal cpssHalBindPortToSchedulerProfileSet function
 * @endinternal
 *
 * @brief  Bind a port to scheduler profile set.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] profileSet            - The Profile Set in which the scheduler's parameters are
 *                                    associated.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalBindPortToSchedulerProfileSet(uint32_t devId, uint32_t port,
                                               uint32_t profile)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    rc = cpssDxChPortTxBindPortToSchedulerProfileSet(cpssDevNum, cpssPortNum,
                                                     (CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT)profile);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortTxBindPortToSchedulerProfileSet dev %d port %d failed(%d)",
              cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    return rc;
}

/**
 * @internal cpssHalPortTxQMaxRateSet function
 * @endinternal
 *
 * @brief   Set MAX Token Bucket Shaper Profile on
 *          specific queue of specified port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tcQueue               - traffic class queue on this Physical Port
 * @param[in] burstSize             - burst size in units of 4K bytes
 * @param[in] maxRatePtr            - Requested Rate in Kbps
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortTxQMaxRateSet(uint32_t devId, uint32_t port,
                                   uint32_t tcQueue, uint32_t burstSize, uint32_t *maxRatePtr)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    rc = cpssDxChPortTxQShaperProfileSet(cpssDevNum, cpssPortNum, tcQueue,
                                         burstSize, maxRatePtr);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortTxQShaperProfileSet dev %d port %d failed(%d)",
              cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    return rc;
}

/**
 * @internal cpssHalPortTxQMinRateSet function
 * @endinternal
 *
 * @brief   Set MIN Token Bucket Shaper Profile on
 *          specific queue of specified port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tcQueue               - traffic class queue on this Physical Port
 * @param[in] burstSize             - burst size in units of 4K bytes
 * @param[in] minRatePtr            - Requested Rate in Kbps
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortTxQMinRateSet(uint32_t devId, uint32_t port,
                                   uint32_t tcQueue, uint32_t burstSize, uint32_t *minRatePtr)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    if (!PRV_CPSS_SIP_6_CHECK_MAC(cpssDevNum))
    {
        return rc;
    }

    rc = cpssDxChPortTxQMinimalRateSet(cpssDevNum, cpssPortNum, tcQueue, burstSize,
                                       minRatePtr);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortTxQMinimalRateSet dev %d port %d failed(%d)",
              cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    return rc;
}

/**
 * @internal cpssHalPortTxQMaxRateEnableSet function
 * @endinternal
 *
 * @brief   Enable/Disable MAX Token Bucket Shaper Profile on
 *          specific queue of specified port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tcQueue               - traffic class queue on this Physical Port
 * @param[in] enable                - GT_TRUE - enable, GT_FALSE - disable
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortTxQMaxRateEnableSet(uint32_t devId, uint32_t port,
                                         uint32_t tcQueue, GT_BOOL enable)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    rc = cpssDxChPortTxQShaperEnableSet(cpssDevNum, cpssPortNum, tcQueue, enable);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortTxQShaperEnableSet dev %d port %d failed(%d)",
              cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    return rc;
}

/**
 * @internal cpssHalPortTxQMinRateEnableSet function
 * @endinternal
 *
 * @brief   Enable/Disable MIN Token Bucket Shaper Profile on
 *          specific queue of specified port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tcQueue               - traffic class queue on this Physical Port
 * @param[in] enable                - GT_TRUE - enable, GT_FALSE - disable
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortTxQMinRateEnableSet(uint32_t devId, uint32_t port,
                                         uint32_t tcQueue, GT_BOOL enable)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    if (!PRV_CPSS_SIP_6_CHECK_MAC(cpssDevNum))
    {
        return rc;
    }

    rc = cpssDxChPortTxQMinimalRateEnableSet(cpssDevNum, cpssPortNum, tcQueue,
                                             enable);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortTxQMinimalRateEnableSet dev %d port %d failed(%d)",
              cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    return rc;
}

/**
 * @internal cpssHalPortShaperRateSet function
 * @endinternal
 *
 * @brief   Set Token Bucket Shaper Profile on specified port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] burstSize             - burst size in units of 4K bytes
 * @param[in] maxRatePtr            - Requested Rate in Kbps
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortShaperRateSet(uint32_t devId, uint32_t port,
                                   uint32_t burstSize, uint32_t *maxRatePtr)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    rc = cpssDxChPortTxShaperProfileSet(cpssDevNum, cpssPortNum, burstSize,
                                        maxRatePtr);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortTxShaperProfileSet dev %d port %d failed(%d)",
              cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    return rc;
}

/**
 * @internal cpssHalPortShaperEnableSet function
 * @endinternal
 *
 * @brief   Enable/Disable Token Bucket Shaper on specified port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] enable                - GT_TRUE - enable, GT_FALSE - disable
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortShaperEnableSet(uint32_t devId, uint32_t port,
                                     GT_BOOL enable)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    rc = cpssDxChPortTxShaperEnableSet(cpssDevNum, cpssPortNum, enable);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortTxShaperEnableSet dev %d port %d failed(%d)",
              cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    return rc;
}

/*** QoS Map APIs ***/

#define QOS_MAP_TABLE_NUM   12
#define MAX_L2_MAP_PER_TBL  16
#define MAX_L3_MAP_PER_TBL  64
#define MAX_QOS_MAP_PER_TBL (MAX_L2_MAP_PER_TBL + MAX_L3_MAP_PER_TBL)

/* 12 * (16+64) = 960. 960 QoS profiles will be used for QoS mapping Table.
 * 8 QoS profiles from 960 to 967 will be used for set default QoSprofile value for a port. */
#define PORT_DEFAULT_QOS_PROFILE_START   (MAX_QOS_MAP_PER_TBL * QOS_MAP_TABLE_NUM)
#define CUSTOM_QOS_PROFILE_START         (PORT_DEFAULT_QOS_PROFILE_START + 8)

/**
 * @internal cpssHalQoSDot1pToTcMapSet function
 * @endinternal
 *
 * @brief   set a L2 QoS map on specific QoS mapping table.
 *
 * @param[in] devId                 - switch Id
 * @param[in] idx                   - QoS mapping table index
 * @param[in] pcp                   - L2 user priority value
 * @param[in] dei                   - value of CFI/DEI bit
 * @param[in] tc                    - traffic class value to be mapped with pcp & dei.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSDot1pToTcMapSet(uint32_t devId, uint32_t idx, uint32_t pcp,
                                    uint32_t dei, uint32_t tc)
{
    GT_STATUS rc = GT_OK;
    GT_U8     cpssDevNum;
    GT_U32    qosProfileIdx = idx * MAX_QOS_MAP_PER_TBL + pcp +
                              (dei * CPSS_USER_PRIORITY_RANGE_CNS);
    CPSS_DXCH_COS_PROFILE_STC qosProfile;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        rc = cpssDxChCosUpCfiDeiToProfileMapSet(cpssDevNum, idx, 0, pcp, dei,
                                                qosProfileIdx);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChCosUpCfiDeiToProfileMapSet dev %d failed(%d)", cpssDevNum, rc);
            return rc;
        }

        rc = cpssDxChCosProfileEntryGet(cpssDevNum, qosProfileIdx, &qosProfile);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChCosProfileEntryGet dev %d failed(%d)", cpssDevNum, rc);
            return rc;
        }

        /* update tc value in QoS profile */

        qosProfile.trafficClass = tc;

        rc = cpssDxChCosProfileEntrySet(cpssDevNum, qosProfileIdx, &qosProfile);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChCosProfileEntrySet dev %d failed(%d)", cpssDevNum, rc);
            return rc;
        }
    }

    return rc;
}

/**
 * @internal cpssHalQoSDot1pToDpMapSet function
 * @endinternal
 *
 * @brief   set a L2 QoS map on specific QoS mapping table.
 *
 * @param[in] devId                 - switch Id
 * @param[in] idx                   - QoS mapping table index
 * @param[in] pcp                   - L2 user priority value
 * @param[in] dei                   - value of CFI/DEI bit
 * @param[in] dp                    - drop precedence value to be mapped with pcp & dei.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSDot1pToDpMapSet(uint32_t devId, uint32_t idx, uint32_t pcp,
                                    uint32_t dei, uint32_t dp)
{
    GT_STATUS rc = GT_OK;
    GT_U8     cpssDevNum;
    GT_U32    qosProfileIdx = idx * MAX_QOS_MAP_PER_TBL + pcp +
                              (dei * CPSS_USER_PRIORITY_RANGE_CNS);
    CPSS_DXCH_COS_PROFILE_STC qosProfile;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        rc = cpssDxChCosUpCfiDeiToProfileMapSet(cpssDevNum, idx, 0, pcp, dei,
                                                qosProfileIdx);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChCosUpCfiDeiToProfileMapSet dev %d failed(%d)", cpssDevNum, rc);
            return rc;
        }

        rc = cpssDxChCosProfileEntryGet(cpssDevNum, qosProfileIdx, &qosProfile);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChCosProfileEntryGet dev %d failed(%d)", cpssDevNum, rc);
            return rc;
        }

        /* update DP value in QoS profile */

        qosProfile.dropPrecedence = dp;

        rc = cpssDxChCosProfileEntrySet(cpssDevNum, qosProfileIdx, &qosProfile);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChCosProfileEntrySet dev %d failed(%d)", cpssDevNum, rc);
            return rc;
        }
    }

    return rc;
}

/**
 * @internal cpssHalQoSDscpToTcMapSet function
 * @endinternal
 *
 * @brief   set a L3 QoS map on specific QoS mapping table.
 *
 * @param[in] devId                 - switch Id
 * @param[in] idx                   - QoS mapping table index
 * @param[in] dscp                  - DSCP value of a IP packet
 * @param[in] tc                    - traffic class value to be mapped with pcp & dei.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSDscpToTcMapSet(uint32_t devId, uint32_t idx, uint32_t dscp,
                                   uint32_t tc)
{
    GT_STATUS rc = GT_OK;
    GT_U8     cpssDevNum;
    GT_U32    qosProfileIdx = idx * MAX_QOS_MAP_PER_TBL + MAX_L2_MAP_PER_TBL + dscp;
    CPSS_DXCH_COS_PROFILE_STC qosProfile;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        rc = cpssDxChCosDscpToProfileMapSet(cpssDevNum, idx, dscp, qosProfileIdx);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChCosDscpToProfileMapSet dev %d failed(%d)", cpssDevNum, rc);
            return rc;
        }

        rc = cpssDxChCosProfileEntryGet(cpssDevNum, qosProfileIdx, &qosProfile);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChCosProfileEntryGet dev %d failed(%d)", cpssDevNum, rc);
            return rc;
        }

        /* update tc value in QoS profile */

        qosProfile.trafficClass = tc;

        rc = cpssDxChCosProfileEntrySet(cpssDevNum, qosProfileIdx, &qosProfile);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChCosProfileEntrySet dev %d failed(%d)", cpssDevNum, rc);
            return rc;
        }
    }

    return rc;
}

/**
 * @internal cpssHalQoSDscpToDpMapSet function
 * @endinternal
 *
 * @brief   set a L3 QoS map on specific QoS mapping table.
 *
 * @param[in] devId                 - switch Id
 * @param[in] idx                   - QoS mapping table index
 * @param[in] dscp                  - DSCP value of a IP packet
 * @param[in] dp                    - drop precedence value to be mapped with pcp & dei.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSDscpToDpMapSet(uint32_t devId, uint32_t idx, uint32_t dscp,
                                   uint32_t dp)
{
    GT_STATUS rc = GT_OK;
    GT_U8     cpssDevNum;
    GT_U32    qosProfileIdx = idx * MAX_QOS_MAP_PER_TBL + MAX_L2_MAP_PER_TBL + dscp;
    CPSS_DXCH_COS_PROFILE_STC qosProfile;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        rc = cpssDxChCosDscpToProfileMapSet(cpssDevNum, idx, dscp, qosProfileIdx);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChCosDscpToProfileMapSet dev %d failed(%d)", cpssDevNum, rc);
            return rc;
        }

        rc = cpssDxChCosProfileEntryGet(cpssDevNum, qosProfileIdx, &qosProfile);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChCosProfileEntryGet dev %d failed(%d)", cpssDevNum, rc);
            return rc;
        }

        /* update DP value in QoS profile */

        qosProfile.dropPrecedence = dp;

        rc = cpssDxChCosProfileEntrySet(cpssDevNum, qosProfileIdx, &qosProfile);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChCosProfileEntrySet dev %d failed(%d)", cpssDevNum, rc);
            return rc;
        }
    }

    return rc;
}

/**
 * @internal cpssHalQoSPortIngressQosMappingTableIdxSet function
 * @endinternal
 *
 * @brief  Set ingress QoS mapping table index on
 *         specificed port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] idx                   - Ingress QoS mapping Table index
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSPortIngressQosMappingTableIdxSet(uint32_t devId,
                                                     uint32_t port, uint32_t idx)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    rc = cpssDxChCosPortTrustQosMappingTableIndexSet(cpssDevNum, cpssPortNum,
                                                     GT_FALSE, idx);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChCosPortTrustQosMappingTableIndexSet \
                    dev %d port %d failed(%d)", cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    return rc;
}

GT_STATUS cpssHalQosPortTrustQosMappingTableIndexGet(uint32_t devId,
                                                     uint32_t port, GT_BOOL *useUpAsIndexPtr, uint32_t *tableIdx)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    rc = cpssDxChCosPortTrustQosMappingTableIndexGet(cpssDevNum, cpssPortNum,
                                                     useUpAsIndexPtr, tableIdx);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChCosPortTrustQosMappingTableIndexSet \
                dev %d port %d failed(%d)", cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    return rc;


}


/**
 * @internal cpssHalPortQosTrustModeSet function
 * @endinternal
 *
 * @brief Configures port's QoS Trust Mode.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] l2trust               - true  - enable l2 Trust mode
 *                                    false - disable l2 trust mode
 * @param[in] l3trust               - true  - enable l3 Trust mode
 *                                    false - disable l3 trust mode
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortQosTrustModeSet(uint32_t devId, uint32_t port,
                                     GT_BOOL l2Trust, GT_BOOL l3Trust)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;
    CPSS_QOS_PORT_TRUST_MODE_ENT  portQosTrustMode = CPSS_QOS_PORT_NO_TRUST_E;
    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    if ((l2Trust == GT_TRUE) && (l3Trust == GT_TRUE))
    {
        portQosTrustMode = CPSS_QOS_PORT_TRUST_L2_L3_E;
    }
    else if (l2Trust == GT_TRUE)
    {
        portQosTrustMode = CPSS_QOS_PORT_TRUST_L2_E;
    }
    else if (l3Trust == GT_TRUE)
    {
        portQosTrustMode = CPSS_QOS_PORT_TRUST_L3_E;
    }
    else
    {
        portQosTrustMode = CPSS_QOS_PORT_NO_TRUST_E;
    }

    rc = cpssDxChCosPortQosTrustModeSet(cpssDevNum, cpssPortNum, portQosTrustMode);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChCosPortQosTrustModeSet \
                    dev %d port %d failed(%d)", cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    return rc;
}

/**
 * @internal cpssHalQoSPortEgressQosMappingTableIdxSet function
 * @endinternal
 *
 * @brief  Set egress QoS mapping table index on
 *         specificed port of specified device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] idx                   - Egress QoS mapping Table index
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSPortEgressQosMappingTableIdxSet(uint32_t devId,
                                                    uint32_t port, uint32_t idx)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    rc = cpssDxChCosPortEgressQosMappingTableIndexSet(cpssDevNum, cpssPortNum, idx);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChCosPortEgressQosMappingTableIndexSet \
                    dev %d port %d failed(%d)", cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    return rc;
}

/**
 * @internal cpssHalPortEgressQosTcDpMappingEnableSet function
 * @endinternal
 *
 * @brief Enable/disable Egress port (TC, DP) mapping to (UP,EXP,DSCP).
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] enable                - enable/disable Egress port {TC, DP} mapping.
 *                                    GT_TRUE  - enable mapping.
 *                                    GT_FALSE - disable mapping.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortEgressQosTcDpMappingEnableSet(uint32_t devId,
                                                   uint32_t port, GT_BOOL enable)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    rc = cpssDxChCosPortEgressQosTcDpMappingEnableSet(cpssDevNum, cpssPortNum,
                                                      enable);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChCosPortEgressQosTcDpMappingEnableSet \
                    dev %d port %d failed(%d)", cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    return rc;
}

/**
 * @internal cpssHalPortEgressQosDot1pMappingEnableSet function
 * @endinternal
 *
 * @brief Enable/disable Egress port Dot1p mapping.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] enable                - enable/disable Egress port dot1p mapping.
 *                                    GT_TRUE  - enable mapping.
 *                                    GT_FALSE - disable mapping.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortEgressQosDot1pMappingEnableSet(uint32_t devId,
                                                    uint32_t port, GT_BOOL enable)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    rc = cpssDxChCosPortEgressQosUpMappingEnableSet(cpssDevNum, cpssPortNum,
                                                    enable);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChCosPortEgressQosUpMappingEnableSet \
                    dev %d port %d failed(%d)", cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    return rc;
}

/**
 * @internal cpssHalPortEgressQosDscpMappingEnableSet function
 * @endinternal
 *
 * @brief Enable/disable Egress port DSCP.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] enable                - enable/disable Egress port DSCP mapping.
 *                                    GT_TRUE  - enable mapping.
 *                                    GT_FALSE - disable mapping.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortEgressQosDscpMappingEnableSet(uint32_t devId,
                                                   uint32_t port, GT_BOOL enable)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    rc = cpssDxChCosPortEgressQosDscpMappingEnableSet(cpssDevNum, cpssPortNum,
                                                      enable);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChCosPortEgressQosDscpMappingEnableSet \
                    dev %d port %d failed(%d)", cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    return rc;
}

/**
 * @internal cpssHalPortEgressQosDscpMappingEnableGet function
 * @endinternal
 *
 * @brief Get Enable/disable Egress port DSCP.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 *
 * @param[out] enable               - Egress port DSCP mapping status
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortEgressQosDscpMappingEnableGet(uint32_t devId,
                                                   uint32_t port, GT_BOOL *enable)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    if (enable == NULL)
    {
        return GT_BAD_PTR;
    }

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    rc = cpssDxChCosPortEgressQosDscpMappingEnableGet(cpssDevNum, cpssPortNum,
                                                      enable);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChCosPortEgressQosDscpMappingEnableGet \
                    dev %d port %d failed(%d)", cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    return rc;
}

/**
 * @internal cpssHalQoSTcDpToDot1pMapSet function
 * @endinternal
 *
 * @brief   set a tc,dp to dot1p QoS map on specific egress QoS mapping table.
 *
 * @param[in] devId                 - switch Id
 * @param[in] idx                   - Egress QoS mapping table index
 * @param[in] tc                    - traffic class
 * @param[in] dp                    - drop precedence
 * @param[in] dot1p                 - user priority value
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSTcDpToDot1pMapSet(uint32_t devId, uint32_t idx, uint32_t tc,
                                      uint32_t dp, uint32_t dot1p)
{
    GT_STATUS rc = GT_OK;
    GT_U8     cpssDevNum;
    GT_U32    up = 0;
    GT_U32    dscp = 0;
    GT_U32    exp  = 0;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        rc = cpssDxChCosEgressTcDp2UpExpDscpMappingEntryGet(cpssDevNum, idx, tc, dp,
                                                            &up, &exp, &dscp);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChCosEgressTcDp2UpExpDscpMappingEntryGet dev %d failed(%d)", cpssDevNum,
                  rc);
            return rc;
        }

        /* update dot1p value in QoS profile */

        rc = cpssDxChCosEgressTcDp2UpExpDscpMappingEntrySet(cpssDevNum, idx, tc, dp,
                                                            dot1p, exp, dscp);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChCosEgressTcDp2UpExpDscpMappingEntrySet dev %d failed(%d)", cpssDevNum,
                  rc);
            return rc;
        }
    }

    return rc;
}

/**
 * @internal cpssHalQoSTcDpToDscpMapSet function
 * @endinternal
 *
 * @brief   set a tc,dp to dscp QoS map on specific egress QoS mapping table.
 *
 * @param[in] devId                 - switch Id
 * @param[in] idx                   - Egress QoS mapping table index
 * @param[in] tc                    - traffic class
 * @param[in] dp                    - drop precedence
 * @param[in] dscp                  - DSCP value
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSTcDpToDscpMapSet(uint32_t devId, uint32_t idx, uint32_t tc,
                                     uint32_t dp, uint32_t dscp)
{
    GT_STATUS rc = GT_OK;
    GT_U8     cpssDevNum;
    GT_U32    dot1p = 0;
    GT_U32    dscp_old = 0;
    GT_U32    exp  = 0;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        rc = cpssDxChCosEgressTcDp2UpExpDscpMappingEntryGet(cpssDevNum, idx, tc, dp,
                                                            &dot1p, &exp, &dscp_old);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChCosEgressTcDp2UpExpDscpMappingEntryGet dev %d failed(%d)", cpssDevNum,
                  rc);
            return rc;
        }

        /* update dscp value in QoS profile */

        rc = cpssDxChCosEgressTcDp2UpExpDscpMappingEntrySet(cpssDevNum, idx, tc, dp,
                                                            dot1p, exp, dscp);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChCosEgressTcDp2UpExpDscpMappingEntrySet dev %d failed(%d)", cpssDevNum,
                  rc);
            return rc;
        }
    }

    return rc;
}

/**
 * @internal cpssHalPortPfcToQueueMapSet function
 * @endinternal
 *
 * @brief   Sets PFC TC to  queue map.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] pfcPrio               - PFC priority
 * @param[in] queue                 - queue number
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortPfcToQueueMapSet(uint32_t devId, uint32_t port,
                                      uint32_t pfcPrio, uint32_t queue)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    if (!PRV_CPSS_SIP_6_CHECK_MAC(cpssDevNum))
    {
        return GT_FAIL;
    }

    rc = cpssDxChPortPfcTcToQueueMapSet(cpssDevNum, cpssPortNum, pfcPrio, queue);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortPfcTcToQueueMapSet \
                    dev %d port %d failed(%d)", cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    return rc;
}

/**
 * @internal cpssHalQoSTcRemapTableSet function
 * @endinternal
 *
 * @brief   Sets mapping of TC to egress queue.
 *
 * @param[in] devId                 - switch Id
 * @param[in] tc                    - traffic class
 * @param[in] queue                 - queue number
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSTcRemapTableSet(uint32_t devId, uint32_t tc, uint32_t queue)
{
    GT_STATUS rc = GT_OK;
    GT_U8     cpssDevNum;
    CPSS_DP_LEVEL_ENT dp;
    CPSS_DXCH_NET_DSA_CMD_ENT  pktCmd;
    GT_BOOL isMultiDest;
    GT_U32  tgtPortTcProf;
    GT_U32  srcPortTcProf;

    CPSS_DXCH_CSCD_QOS_TC_DP_REMAP_INDEX_STC tcDpRemapping;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        if (!PRV_CPSS_SIP_6_CHECK_MAC(cpssDevNum))
        {
            return GT_FAIL;
        }

        for (dp = CPSS_DP_GREEN_E ; dp <= CPSS_DP_RED_E ; dp++)
        {
            tcDpRemapping.tc = tc;
            tcDpRemapping.dp = dp;

            for (pktCmd = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
                 pktCmd <= CPSS_DXCH_NET_DSA_CMD_FORWARD_E; pktCmd++)
            {
                tcDpRemapping.dsaTagCmd = pktCmd;

                for (isMultiDest = GT_FALSE; isMultiDest <= GT_TRUE; isMultiDest++)
                {
                    tcDpRemapping.packetIsMultiDestination = isMultiDest;

                    for (tgtPortTcProf = 0; tgtPortTcProf < BIT_2; tgtPortTcProf++)
                    {
                        tcDpRemapping.targetPortTcProfile = tgtPortTcProf;

                        for (srcPortTcProf = 0; srcPortTcProf < BIT_2; srcPortTcProf++)
                        {
                            tcDpRemapping.isStack = srcPortTcProf;

                            rc = cpssDxChCscdQosTcDpRemapTableSet(cpssDevNum, &tcDpRemapping, queue,
                                                                  dp, tc, CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E, CPSS_DP_FOR_RX_LOW_E, GT_FALSE);
                            if (rc != GT_OK)
                            {
                                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                                      "cpssDxChCscdQosTcDpRemapTableSet dev %d failed(%d)", cpssDevNum, rc);
                                return rc;
                            }
                        }
                    }
                }
            }
        }
    }

    return rc;
}

/**
 * @internal cpssHalQoSPortDefaultTcSet function
 * @endinternal
 *
 * @brief   Sets default tc value for a specific port.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tc                    - traffic class
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSPortDefaultTcSet(uint32_t devId, uint32_t port, uint32_t tc)
{
    GT_STATUS rc = GT_OK;
    GT_U8     cpssDevNum;
    GT_U32    qosProfileIdx = PORT_DEFAULT_QOS_PROFILE_START + tc;

    CPSS_DXCH_COS_PROFILE_STC qosProfile;
    CPSS_QOS_ENTRY_STC        qosCfgEntry;
    GT_PHYSICAL_PORT_NUM      cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    rc = cpssDxChCosProfileEntryGet(cpssDevNum, qosProfileIdx, &qosProfile);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChCosProfileEntryGet dev %d failed(%d)", cpssDevNum, rc);
        return rc;
    }

    /* update TC value in QoS profile */

    qosProfile.trafficClass = tc;

    rc = cpssDxChCosProfileEntrySet(cpssDevNum, qosProfileIdx, &qosProfile);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChCosProfileEntrySet dev %d failed(%d)", cpssDevNum, rc);
        return rc;
    }

    /* Set default QoS profile for the port */

    rc = cpssDxChCosPortQosConfigGet(cpssDevNum, cpssPortNum, &qosCfgEntry);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "cpssDxChCosPortQosConfigGet \
                    dev %d port %d failed(%d)", cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    /* update QoS profile index */

    qosCfgEntry.qosProfileId = qosProfileIdx;

    rc = cpssDxChCosPortQosConfigSet(cpssDevNum, cpssPortNum, &qosCfgEntry);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "cpssDxChCosPortQosConfigSet \
                    dev %d port %d failed(%d)", cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    return rc;
}

/**
 * @internal cpssHalQoSPortDefaultTcGet function
 * @endinternal
 *
 * @brief   Get default tc value of a specific port.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 *
 * @param[out] tc                   - traffic class
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQoSPortDefaultTcGet(uint32_t devId, uint32_t port,
                                     uint32_t *tc)
{
    GT_STATUS rc = GT_OK;
    GT_U8     cpssDevNum;

    CPSS_DXCH_COS_PROFILE_STC qosProfile;
    CPSS_QOS_ENTRY_STC        qosCfgEntry;
    GT_PHYSICAL_PORT_NUM      cpssPortNum;

    if (tc == NULL)
    {
        return GT_BAD_PTR;
    }

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    rc = cpssDxChCosPortQosConfigGet(cpssDevNum, cpssPortNum, &qosCfgEntry);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "cpssDxChCosPortQosConfigGet \
                    dev %d port %d failed(%d)", cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    rc = cpssDxChCosProfileEntryGet(cpssDevNum, qosCfgEntry.qosProfileId,
                                    &qosProfile);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChCosProfileEntryGet dev %d failed(%d)", cpssDevNum, rc);
        return rc;
    }

    *tc = qosProfile.trafficClass;

    return rc;
}

/*** PFC APIs ***/

/**
 * @internal cpssHalPortPfcTcEnableSet function
 * @endinternal
 *
 * @brief   Sets PFC enable for specific port.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tc                    - traffic class
 * @param[in] enable                - port PFC enable option
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortPfcTcEnableSet(uint32_t devId, uint32_t port, uint32_t tc,
                                    GT_BOOL enable)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    CPSS_DXCH_PFC_THRESHOLD_STC threshold;
    GT_BOOL pfcEnable = GT_FALSE;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    if (!PRV_CPSS_SIP_6_CHECK_MAC(cpssDevNum))
    {
        /* TODO for Aldrin */
        return rc;
    }

    rc = cpssDxChPortPfcPortTcThresholdGet(cpssDevNum, cpssPortNum, tc, &pfcEnable,
                                           &threshold);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortPfcPortTcThresholdGet \
                    dev %d port %d failed(%d)", cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    if (pfcEnable == enable)
    {
        return rc;
    }

    rc = cpssDxChPortPfcPortTcThresholdSet(cpssDevNum, cpssPortNum, tc, enable,
                                           &threshold);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortPfcPortTcThresholdSet \
                    dev %d port %d failed(%d)", cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    return rc;
}

/**
 * @internal cpssHalPortPfcTcEnableGet function
 * @endinternal
 *
 * @brief   Get PFC enable for specific port.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tc                    - traffic class
 * @param[out] enable               - port PFC enable option
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortPfcTcEnableGet(uint32_t devId, uint32_t port, uint32_t tc,
                                    GT_BOOL *enable)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    CPSS_DXCH_PFC_THRESHOLD_STC threshold;
    GT_BOOL pfcEnable = GT_FALSE;

    if (enable == NULL)
    {
        return GT_BAD_PTR;
    }

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    if (!PRV_CPSS_SIP_6_CHECK_MAC(cpssDevNum))
    {
        *enable = GT_FALSE;
        return rc;
    }

    rc = cpssDxChPortPfcPortTcThresholdGet(cpssDevNum, cpssPortNum, tc, &pfcEnable,
                                           &threshold);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortPfcPortTcThresholdGet "
              "dev %d port %d failed(%d)", cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    *enable = pfcEnable;

    return rc;
}

/**
 * @internal cpssHalPortPfcTcResponseEnableSet function
 * @endinternal
 *
 * @brief   Enable /Disable PFC response per port.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] tc                    - traffic class
 * @param[in] enable                - port PFC response enable option
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPortPfcTcResponseEnableSet(uint32_t devId, uint32_t port,
                                            uint32_t tc, GT_BOOL enable)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    cpssHalSetDeviceSwitchId(devId);

    cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    if (!PRV_CPSS_SIP_6_CHECK_MAC(cpssDevNum))
    {
        /* TODO for Aldrin */
        return rc;
    }

    rc = cpssDxChPortPfcResponceEnableSet(cpssDevNum, cpssPortNum, tc, enable);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChPortPfcResponceEnableSet \
                    dev %d port %d failed(%d)", cpssDevNum, cpssPortNum, rc);
        return rc;
    }

    return rc;
}

/**
* @internal cpssHalPortFlowControlModeSet function
* @endinternal
*
* @brief   Sets Flow Control mode on given port.
*
* @note   APPLICABLE DEVICES:      Lion; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; xCat2.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port number.
* @param[in] fcMode                   - flow control mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note For Lion and above:
*       This function also configures insertion of DSA tag for PFC frames.
*
*/
GT_STATUS cpssHalPortFlowControlModeSet
(
    IN  GT_U8     devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_DXCH_PORT_FC_MODE_ENT fcMode
)
{
    GT_STATUS status;

    CPSS_PP_FAMILY_TYPE_ENT devType = cpssHalDevPPFamilyGet(devNum);
    if (devType != CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        return GT_OK;
    }
    SAI2CPSS_PORT_CONVERT(devNum, portNum);

    status = cpssDxChPortFlowControlModeSet(devNum, portNum, fcMode);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to set fc mode  "
              "status:%d, cpssdev:%d, port:%d, fcMode:%d\n",
              status, devNum, portNum, fcMode);
        return status;
    }

    return GT_OK;
}

static GT_STATUS cpssHalGetDevNumAndPortNum(uint32_t devId, uint32_t port,
                                            GT_U8 *cpssDevNum, GT_PHYSICAL_PORT_NUM *cpssPortNum)
{
    GT_STATUS            rc = GT_OK;

    cpssHalSetDeviceSwitchId(devId);

    *cpssDevNum = xpsGlobalIdToDevId(devId, port);
    if (*cpssDevNum == 0xff)
    {
        return GT_FAIL;
    }

    *cpssPortNum = xpsGlobalPortToPortnum(devId, port);
    if (*cpssPortNum == 0xffff)
    {
        return GT_FAIL;
    }

    return rc;
}

/**
* @internal cpssHalPortTx4TcTailDropWredProfileGet function
* @endinternal
*
* @brief   Get tail drop WRED profile  parameters  for particular TC.
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                      Drop Parameters (0..15).
* @param[out] tailDropWredProfileParamsPtr -
*                                  (pointer to)the Drop Profile WRED Parameters to associate
*                                      with the Traffic Class in this Profile set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalPortTx4TcTailDropWredProfileGet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN    GT_U8                                   trafficClass,
    OUT   CPSS_PORT_TX_Q_TAIL_DROP_WRED_TC_DP_PROF_PARAMS_STC
    *tailDropWredProfileParamsPtr
)
{
    GT_STATUS status;

    status = cpssDxChPortTx4TcTailDropWredProfileGet(devNum, profileSet,
                                                     trafficClass, tailDropWredProfileParamsPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to get wred profile  "
              "status:%d, cpssdev:%d, profile:%d\n",
              status, devNum, profileSet);
        return status;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortTxTailDropWredProfileGet function
* @endinternal
*
* @brief   Get tail drop port WRED profile  parameters .
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
*
* @param[out] tailDropWredProfileParamsPtr -
*                                      the Drop Profile WRED Parameters to associate
*                                      with the port  in this Profile set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalPortTxTailDropWredProfileGet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    OUT   CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC     *tailDropWredProfileParamsPtr
)
{
    GT_STATUS status;

    status = cpssDxChPortTxTailDropWredProfileGet(devNum, profileSet,
                                                  tailDropWredProfileParamsPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to get wred profile  "
              "status:%d, cpssdev:%d, profile:%d\n",
              status, devNum, profileSet);
        return status;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortTxTailDropProfileGet function
* @endinternal
*
* @brief Get port dynamic limit alpha and set maximal port's limits of buffers
*       and descriptors.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                       Class Drop Parameters is associated
*
* @param[out] portAlphaPtr            - Pointer to ratio of the free buffers
*                                       used for the port thresholds
*                                   (APPLICABLE DEVICES: Aldrin2; Falcon).
* @param[out] portMaxBuffLimitPtr     - Pointer to maximal number of buffers for a port
* @param[out] portMaxDescrLimitPtr    - Pointer to maximal number of descriptors for a port
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalPortTxTailDropProfileGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT    profileSet,
    OUT CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT *portAlphaPtr
)
{
    GT_STATUS status;
    GT_U32    dummy;

    status = cpssDxChPortTxTailDropProfileGet(devNum, profileSet, portAlphaPtr,
                                              &dummy, &dummy);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to get wred profile  "
              "status:%d, cpssdev:%d, profile:%d\n",
              status, devNum, profileSet);
        return status;
    }

    return GT_OK;
}


/**
* @internal cpssHalPortTxTailDropProfileSet function
* @endinternal
*
* @brief Configures port dynamic limit alpha and set maximal port's limits of
*       buffers and descriptors.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                       Class Drop Parameters is associated
* @param[in] portAlpha                - ratio of the free buffers used for the port
*                                       thresholds (APPLICABLE DEVICES: Aldrin2; Falcon).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalPortTxTailDropProfileSet
(
    IN  GT_U8                                devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT    profileSet,
    IN  CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT portAlpha
)
{
    GT_STATUS status;
    GT_U32    dummy=0;

    status = cpssDxChPortTxTailDropProfileSet(devNum, profileSet, portAlpha, dummy,
                                              dummy);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to set td profile  "
              "status:%d, cpssdev:%d, profile:%d\n",
              status, devNum, profileSet);
        return status;
    }

    return GT_OK;
}


/**
* @internal cpssHalPortTxTailDropWredProfileSet function
* @endinternal
*
* @brief   Set tail drop port WRED profile  parameters .
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
*
* @param[in] tailDropWredProfileParamsPtr -
*                                      the Drop Profile WRED Parameters to associate
*                                      with the Traffic Class in this Profile set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalPortTxTailDropWredProfileSet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN    CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC     *tailDropWredProfileParamsPtr
)
{
    GT_STATUS status;

    status = cpssDxChPortTxTailDropWredProfileSet(devNum, profileSet,
                                                  tailDropWredProfileParamsPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set port wred profile  "
              "status:%d, cpssdev:%d, profile:%d\n",
              status, devNum, profileSet);
        return status;
    }

    return GT_OK;
}

/*
 * @internal cpssHalQosAddPolicerEntry function
 * @endinternal
 *
 * @brief  Add a policer entry to the device.
 *
 * @param[in] devId                 - switch Id
 * @param[in] stage                 - policer stage - ingress-1,2 & egress
 * @param[in] policer index         - policer index
 * @param[in] entryPtr              - pointer to the metering policer entry to be set
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQosAddPolicerEntry(uint32_t devId,
                                    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
                                    uint32_t policerIndex, CPSS_DXCH3_POLICER_METERING_ENTRY_STC *entryPtr)
{
    GT_STATUS                              rc = GT_OK;
    GT_U8                                  cpssDevNum = 0;
    CPSS_DXCH3_POLICER_METER_TB_PARAMS_UNT tbParams;
    CPSS_DXCH3_POLICER_BILLING_ENTRY_STC   billingCntr;

    if (entryPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    /* In CPSS, CIR/PIR is configured in units of Kbps.
     * In Packet resolution mode, HW calulates packet size = 1 byte.
     * So, X pps = 8*X bps = 8*X/1000 kbps */
    if (entryPtr->byteOrPacketCountingMode ==
        CPSS_DXCH3_POLICER_METER_RESOLUTION_PACKETS_E)
    {
        if (entryPtr->meterMode == CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E)
        {
            entryPtr->tokenBucketParams.srTcmParams.cir =
                (entryPtr->tokenBucketParams.srTcmParams.cir * XP_BITS_IN_BYTE) /
                XP_KBPS_TO_BITS_FACTOR;
        }
        else
        {
            entryPtr->tokenBucketParams.trTcmParams.cir =
                (entryPtr->tokenBucketParams.trTcmParams.cir * XP_BITS_IN_BYTE) /
                XP_KBPS_TO_BITS_FACTOR;
            entryPtr->tokenBucketParams.trTcmParams.pir =
                (entryPtr->tokenBucketParams.trTcmParams.pir * XP_BITS_IN_BYTE) /
                XP_KBPS_TO_BITS_FACTOR;
        }
    }

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        /* Billing params cannot be modified under traffic on the fly. Hence
           set configure it before associating to policer index .*/
        cpssOsMemSet(&billingCntr, 0,
                     sizeof(CPSS_DXCH3_POLICER_BILLING_ENTRY_STC));

        /* TODO: Delete/re-add of same policer is not reflecting in ASIC as expected.*/
        if (stage == CPSS_DXCH_POLICER_STAGE_EGRESS_E &&
            (policerIndex >= CPSSHAL_ERSPAN_EPLR_FLOW_ID_BASE &&
             policerIndex <= (CPSSHAL_ERSPAN_EPLR_FLOW_ID_BASE +
                              XP_MIRROR_MAX_USER_SESSION)))
        {
            billingCntr.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
            billingCntr.lmCntrCaptureMode =
                CPSS_DXCH3_POLICER_LM_CNTR_CAPTURE_MODE_INSERT_E;
            billingCntr.billingCntrAllEnable = GT_TRUE;
        }

        if (entryPtr->byteOrPacketCountingMode ==
            CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E)
        {
            billingCntr.billingCntrMode = CPSS_DXCH3_POLICER_BILLING_CNTR_1_BYTE_E;
        }
        else
        {
            billingCntr.billingCntrMode = CPSS_DXCH3_POLICER_BILLING_CNTR_PACKET_E;
        }

        billingCntr.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;

        rc = cpssDxCh3PolicerBillingEntrySet(cpssDevNum, stage,
                                             entryPtr->countingEntryIndex, &billingCntr);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed cpssDxCh3PolicerBillingEntrySet, "
                  "rc:%d, dev:%d, index:%d\n", rc, cpssDevNum, policerIndex);
            return rc;
        }

        rc = cpssDxCh3PolicerMeteringEntrySet(cpssDevNum, stage, policerIndex, entryPtr,
                                              &tbParams);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed cpssDxCh3PolicerMeteringEntrySet, "
                  "rc:%d, dev:%d, index:%d\n", rc, cpssDevNum, policerIndex);
            return rc;
        }
    }
    return rc;
}

/*
 * @internal cpssHalPolicerBillingEntrySet function
 * @endinternal
 *
 * @brief   Sets Policer Billing Counters.
 *
 * @param[in] devId                 - switch Id
 * @param[in] stage                 - policer stage - ingress-1,2 & egress
 * @param[in] entryIndex            - policer index
 * @param[in] billingCntrPtr        - pointer to the Billing Counters Entry.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalPolicerBillingEntrySet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT      stage,
    IN  GT_U32                                entryIndex,
    IN  CPSS_DXCH3_POLICER_BILLING_ENTRY_STC *billingCntrPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     cpssDevNum = 0;

    if (billingCntrPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    XPS_DEVICES_PER_SWITCH_ITER(devNum, cpssDevNum)
    {
        rc = cpssDxCh3PolicerBillingEntrySet(cpssDevNum, stage,
                                             entryIndex, billingCntrPtr);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed cpssDxCh3PolicerBillingEntrySet, "
                  "rc:%d, dev:%d, index:%d\n", rc, cpssDevNum, entryIndex);
            return rc;
        }
    }

    return rc;
}

/**
 * @internal cpssHalQosPolicerPortStormTypeIndexSet function
 * @endinternal
 *
 * @brief  Bind a port to storm type.
 *
 * @param[in] devId                 - switch Id
 * @param[in] port                  - port number
 * @param[in] stage                 - Policer Stage type: Ingress #0, Ingress #1.
 * @param[in] stormType             - storm type
 * @param[in] index                 - policer  (APPLICABLE RANGES: 0..3)
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - wrong devNum
 * @retval GT_BAD_PTR               - on bad pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 * @retval GT_FAIL                  - otherwise
 */
GT_STATUS cpssHalQosPolicerPortStormTypeIndexSet(uint32_t devId,
                                                 uint32_t portNum, CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
                                                 CPSS_DXCH_POLICER_STORM_TYPE_ENT stormType, uint32_t stormIndex)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    rc = cpssHalGetDevNumAndPortNum(devId, portNum, &cpssDevNum, &cpssPortNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChPolicerPortStormTypeIndexSet(cpssDevNum, stage, cpssPortNum,
                                              stormType, stormIndex);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed cpssDxChPolicerPortStormTypeIndexSet, "
              "rc:%d, dev:%d, portNum:%d stormType:%d, stormIndex:%d\n", rc, cpssDevNum,
              cpssPortNum, stormType, stormIndex);
        return rc;
    }
    return rc;
}

/**
 * @internal cpssHalPolicerCounterEntryGet function
 * @endinternal
 *
 * @brief  Retrieve Policer Counters for the given index.
 *
 * @param[in] devId                 - switch Id
 * @param[in] stage                 - Policer stage type.
 * @param[in] policerIndex          - policer Billing Counters Entry index
 * @param[out] greenCntr            - Green pkt counter.
 * @param[out] yellowCntr           - yellow pkt counter.
 * @param[out] redCntr              - Red pkt counter.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
 * @retval GT_HW_ERROR              - on hardware error.
 * @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
 * @retval GT_BAD_STATE             - on counter entry type mismatch.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */

GT_STATUS cpssHalPolicerCounterEntryGet(uint32_t devId,
                                        CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage, uint32_t policerIndex,
                                        uint64_t *greenCntr, uint64_t *yellowCntr, uint64_t *redCntr)
{
    GT_STATUS                            rc = GT_OK;
    GT_U8                                cpssDevNum;
    CPSS_DXCH3_POLICER_BILLING_ENTRY_STC billingCntr;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        cpssOsMemSet(&billingCntr, 0,
                     sizeof(CPSS_DXCH3_POLICER_BILLING_ENTRY_STC));
        rc = cpssDxCh3PolicerBillingEntryGet(cpssDevNum, stage, policerIndex, GT_TRUE,
                                             &billingCntr);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxCh3PolicerBillingEntryGet dev %d stage %d index %d failed(%d)",
                  devId, stage, policerIndex, rc);
            return rc;
        }

        if (greenCntr)
        {
            *greenCntr += CONVERT_TO_64BIT_INT(billingCntr.greenCntr);
        }
        if (yellowCntr)
        {
            *yellowCntr += CONVERT_TO_64BIT_INT(billingCntr.yellowCntr);
        }
        if (redCntr)
        {
            *redCntr += CONVERT_TO_64BIT_INT(billingCntr.redCntr);
        }
    }

    return rc;
}

/**
 * @internal cpssHalPolicerCounterEntryClear function
 * @endinternal
 *
 * @brief  Clear Policer Counters for the given index.
 *
 * @param[in] devId                 - switch Id
 * @param[in] stage                 - Policer stage type.
 * @param[in] policerIndex          - policer Billing Counters Entry index
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
 * @retval GT_HW_ERROR              - on hardware error.
 * @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
 * @retval GT_BAD_STATE             - on counter entry type mismatch.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */

GT_STATUS cpssHalPolicerCounterEntryClear(uint32_t devId,
                                          CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
                                          uint32_t policerIndex)
{
    GT_STATUS                            rc = GT_OK;
    GT_U8                                cpssDevNum;
    CPSS_DXCH3_POLICER_BILLING_ENTRY_STC billingCntr;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        cpssOsMemSet(&billingCntr, 0,
                     sizeof(CPSS_DXCH3_POLICER_BILLING_ENTRY_STC));
        rc = cpssDxCh3PolicerBillingEntryGet(cpssDevNum, stage, policerIndex, GT_TRUE,
                                             &billingCntr);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxCh3PolicerBillingEntryGet dev %d stage %d index %d failed(%d)",
                  devId, stage, policerIndex, rc);
            return rc;
        }
    }

    return rc;
}

/**
 * @internal cpssHalPolicerEPortTriggerEntrySet function
 * @endinternal
 *
 * @brief   Set Policer trigger entry.
 *
 * @param[in] devId                 - switch Id
 * @param[in] stage                 - Policer stage type.
 * @param[in] ePort                 - ePort i.e polcier trigger table index.
 * @param[in] policerIndex          - eport policer trigger table index
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
 * @retval GT_HW_ERROR              - on hardware error.
 * @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
 * @retval GT_BAD_STATE             - on counter entry type mismatch.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */

GT_STATUS cpssHalPolicerEPortTriggerEntrySet(uint32_t devId,
                                             CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
                                             uint32_t ePort, uint32_t policerIndex, bool enable)
{
    GT_STATUS                            rc = GT_OK;
    GT_U8                                cpssDevNum;
    CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC  entry;

    cpssOsMemSet(&entry, 0,
                 sizeof(CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC));

    if (enable)
    {
        entry.meteringEnable = GT_TRUE;
        entry.countingEnable = GT_TRUE;
        entry.policerIndex = policerIndex;
    }
    else
    {
        entry.meteringEnable = GT_FALSE;
        entry.countingEnable = GT_FALSE;
        entry.policerIndex = 0;
    }
    entry.ucKnownEnable = GT_TRUE;
    entry.ucKnownOffset = 0;
    entry.ucUnknownEnable= GT_TRUE;
    entry.ucUnknownOffset = 0;
    entry.mcRegisteredEnable = GT_TRUE;
    entry.mcRegisteredOffset = 0;
    entry.mcUnregisteredEnable = GT_TRUE;
    entry.mcUnregisteredOffset = 0;
    entry.bcEnable = GT_TRUE;
    entry.bcOffset = 0;
    entry.tcpSynEnable = GT_TRUE;
    entry.tcpSynOffset = 0;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        rc = cpssDxChPolicerTriggerEntrySet(cpssDevNum, stage, ePort, &entry);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalPolicerEPortTriggerEntrySet dev %d stage %d index %d failed(%d)",
                  devId, stage, policerIndex, rc);
            return rc;
        }
    }

    return rc;
}

/**
* @internal cpssDxChPortTx4TcTailDropEcnMarkingProfileSet function
* @endinternal
*
* @brief   Set tail drop ECN  profile parameters  for particular TC.
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                      Drop Parameters (0..15).
* @param[in] tailDropEcnProfileParamsPtr -
*                                      the Drop Profile ECN Parameters to associate
*                                      with the Traffic Class in this Profile set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalPortTx4TcTailDropEcnMarkingProfileSet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN    GT_U8                                   trafficClass,
    IN    CPSS_PORT_QUEUE_ECN_PARAMS_STC          *tailDropEcnProfileParamsPtr
)
{
    GT_STATUS status;

    status = cpssDxChPortTx4TcTailDropEcnMarkingProfileSet(devNum, profileSet,
                                                           trafficClass, tailDropEcnProfileParamsPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set port ecn profile  "
              "status:%d, cpssdev:%d, profile:%d\n",
              status, devNum, profileSet);
        return status;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortTx4TcTailDropEcnMarkingProfileGet function
* @endinternal
*
* @brief   Get tail drop ECN  profile parameters  for particular TC.
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:   xCat3;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                      Drop Parameters (0..15).
* @param[out] tailDropEcnProfileParamsPtr -(pointer to)
*                                      the Drop Profile ECN Parameters to associate
*                                      with the Traffic Class in this Profile set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalPortTx4TcTailDropEcnMarkingProfileGet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN    GT_U8                                   trafficClass,
    OUT   CPSS_PORT_QUEUE_ECN_PARAMS_STC          *tailDropEcnProfileParamsPtr
)
{
    GT_STATUS status;

    status = cpssDxChPortTx4TcTailDropEcnMarkingProfileGet(devNum, profileSet,
                                                           trafficClass, tailDropEcnProfileParamsPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get port ecn profile  "
              "status:%d, cpssdev:%d, profile:%d\n",
              status, devNum, profileSet);
        return status;
    }

    return GT_OK;
}

/**
* @internal  cpssHalPortTxTcMapToSharedPoolSet function
* @endinternal
*
* @brief  Set tail drop mapping between TC to pool
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum       - Device number
* @param[in] tc           - Traffic class [0..7]
* @param[in] poolNum      - Pool id[0..1]
*
* @retval GT_OK                       - on success
* @retval GT_FAIL                     - on error
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - on invalid input parameters value
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
* @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS cpssHalPortTxTcMapToSharedPoolSet
(
    IN  GT_U8                               devNum,
    IN  GT_U8                               tc,
    IN  GT_U32                              poolNum
)
{
    GT_STATUS status;

    status = cpssDxChPortTxTcMapToSharedPoolSet(devNum, tc, poolNum);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to bind tc to pool  "
              "status:%d, cpssdev:%d, pool:%d tc %d\n",
              status, devNum, tc);
        return status;
    }

    return GT_OK;
}

/**
* @internal  cpssHalPortTxTcMapToSharedPoolGet function
* @endinternal
*
* @brief  Get tail drop mapping between TC to pool
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3;  Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number
* @param[in] tc                       - traffic class [0..7]
* @param[out]  poolNumPtr             - (Pointer to) Pool id[0..1]
*
* @retval GT_OK                       - on success
* @retval GT_FAIL                     - on error
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - on invalid input parameters value
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
* @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS cpssHalPortTxTcMapToSharedPoolGet
(
    IN  GT_U8                               devNum,
    IN  GT_U8                               tc,
    OUT GT_U32                              *poolNumPtr
)
{
    GT_STATUS status;

    status = cpssDxChPortTxTcMapToSharedPoolGet(devNum, tc, poolNumPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get binding for tc to pool  "
              "status:%d, cpssdev:%d, pool:%d tc %d\n",
              status, devNum, tc);
        return status;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortTx4TcBufNumberGet function
* @endinternal
*
* @brief   Gets the current number of buffers allocated on specified port
*         for specified Traffic Class queues.
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] trafClass                - trafiic class (0..7)
*
* @param[out] numPtr                   - (pointer to) the number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Note: Do not call this API under traffic for Lion devices
*
*/
GT_STATUS cpssHalPortTx4TcBufNumberGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U8       trafClass,
    OUT GT_U32      *numPtr
)
{
    GT_STATUS status;
    GT_U8     cpssDevId;
    GT_U32    cpssPortNum;

    /* covert global num to CPSS num */
    status = cpssHalGetDevNumAndPortNum(devNum, portNum, &cpssDevId, &cpssPortNum);
    if (status != GT_OK)
    {
        return status;
    }
    status = cpssDxChPortTx4TcBufNumberGet(cpssDevId, cpssPortNum, trafClass,
                                           numPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get buffer num  for port %d tc %d  "
              "status:%d, cpssdev:%d\n",
              portNum, trafClass, status, cpssDevId);

        return status;
    }

    return GT_OK;
}

/**
* @internal cpssHalCncQueueStatisticInit function
* @endinternal
*
* @brief   Initialise egress queue pass drop cnc client and Queue Statistic CNC Client.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalCncQueueStatisticInit
(
    IN  GT_U8 devNum
)
{
    GT_STATUS rc;
    GT_U32    i;

    CPSS_PP_FAMILY_TYPE_ENT devType = cpssHalDevPPFamilyGet(devNum);

    if (devType == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        /* Set Egress queue client counting mode to tail drop reduced */
        rc = cpssDxChCncEgressQueueClientModeSet(devNum,
                                                 CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Egress queue client mode set failed rc:%d", rc);
            return rc;
        }

        int numPorts = 0;
        int numCPUPorts = 0;
        cpssHalGetMaxGlobalPorts(&numPorts);
        cpssHalGetMaxCPUPorts(devNum, &numCPUPorts);

        numPorts += numCPUPorts;
        /* Each CNC block supports 1024/2 = 512 queues i.e 64 ports */
        /* Each queue will be mapped to two blocks for packet and byte counters. *
         * So, totally 2 * QUEUE_STAT_CNC_BLOCK_NUM blocks will be used for queue stats. */
        queueStatCncBlockNum_g = ((numPorts%64) ? (numPorts/64)+1 : numPorts/64);


        /*Enable  queue statistic counter */

        for (i=0; i<queueStatCncBlockNum_g; i++)
        {
            /* mapping each queue to two blocks. one for pkt counter(format_3) and another for byte counter(format_4) */

            rc = cpssHalCncBlockClientEnableAndBindSet(devNum,
                                                       QUEUE_STAT_CNC_BLOCK_FIRST+i,
                                                       QUEUE_STAT_CLIENT, GT_TRUE, i, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_3_E);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to enable block client rc %d\n", rc);
                return rc;
            }

            rc = cpssHalCncBlockClientEnableAndBindSet(devNum,
                                                       QUEUE_STAT_CNC_BLOCK_FIRST + queueStatCncBlockNum_g + i,
                                                       QUEUE_STAT_CLIENT, GT_TRUE, i, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to enable block client rc %d\n", rc);
                return rc;
            }
        }

        /* The initialization of Queue Statistics Client CNC block  */

        /* Each CNC block supports 1024 queues i.e 128 ports */
        queueWatermarkCncBlockNum_g = (numPorts + 127) / 128;   /* align to 128*/
        for (i=0; i<queueWatermarkCncBlockNum_g; i++)
        {
            /* enable block's client, and set to Mode 5 */
            rc = cpssHalCncBlockClientEnableAndBindSet(devNum,
                                                       QUEUE_STAT_WATERMARK_CNC_BLOCK_START + i,
                                                       CPSS_DXCH_CNC_CLIENT_QUEUE_STAT_E, GT_TRUE, i,
                                                       CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_5_E);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to enable block client rc %d\n", rc);
                return rc;
            }
        }

    }

    return GT_OK;
}



GT_STATUS cpssHalCncQueueStatGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            port,
    IN  GT_U32                            queue,
    OUT uint64_t                          *passedBytePtr,
    OUT uint64_t                          *passedPcktPtr,
    OUT uint64_t                          *droppedBytePtr,
    OUT uint64_t                          *droppedPcktPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 block;
    CPSS_DXCH_CNC_COUNTER_FORMAT_ENT format;
    GT_U8                cpssDevId;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    CPSS_PP_FAMILY_TYPE_ENT devType = cpssHalDevPPFamilyGet(devNum);
    /* covert global num to CPSS num */
    rc = cpssHalGetDevNumAndPortNum(devNum, port, &cpssDevId, &cpssPortNum);
    if (rc != GT_OK)
    {
        return rc;
    }


    if ((passedBytePtr == NULL) || (passedPcktPtr == NULL) ||
        (droppedBytePtr == NULL) || (droppedPcktPtr == NULL))
    {
        return GT_BAD_PTR;
    }

    if (devType == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        block = QUEUE_STAT_CNC_BLOCK_FIRST;
        format = CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_3_E;
        rc = cpssHalCncQueueCounterGet(cpssDevId, cpssPortNum, queue, block, format,
                                       NULL, passedPcktPtr, NULL, NULL);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = cpssHalCncQueueCounterGet(cpssDevId, cpssPortNum, queue, block, format,
                                       NULL, NULL, NULL, droppedPcktPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        block = QUEUE_STAT_CNC_BLOCK_FIRST + queueStatCncBlockNum_g;
        format = CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E;
        rc = cpssHalCncQueueCounterGet(cpssDevId, cpssPortNum, queue, block, format,
                                       passedBytePtr, NULL, NULL, NULL);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = cpssHalCncQueueCounterGet(cpssDevId, cpssPortNum, queue, block, format,
                                       NULL, NULL, droppedBytePtr, NULL);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else if (devType == CPSS_PP_FAMILY_DXCH_AC3X_E)
    {
        *passedBytePtr = 0;
        *droppedBytePtr = 0;
        *passedPcktPtr = 0;
        *droppedPcktPtr = 0;
    }

    return rc;
}

GT_STATUS cpssHalCncQueueCounterGet
(
    IN  GT_U8                             cpssDevId,
    IN  GT_PHYSICAL_PORT_NUM              cpssPortNum,
    IN  GT_U32                            queue,
    IN  GT_U32                            firstBlock,
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    OUT uint64_t                          *totalPassedBytePtr,
    OUT uint64_t                          *totalPassedPcktPtr,
    OUT uint64_t                          *totalDroppedBytePtr,
    OUT uint64_t                          *totalDroppedPcktPtr
)
{
    GT_STATUS   status;
    GT_U32      queueBase = 0, blockInternalIndex;

    CPSS_DXCH_CNC_COUNTER_STC  cnt;

    status = cpssDxChCncPortQueueGroupBaseGet(cpssDevId, cpssPortNum,
                                              CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E, &queueBase);
    if (GT_OK != status)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              " cpssDxChCncPortQueueGroupBaseGet failed with err : %d on device :%d\n ",
              status, cpssDevId);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    blockInternalIndex = queueBase>>9;

    cpssOsMemSet(&cnt, 0, sizeof(CPSS_DXCH_CNC_COUNTER_STC));

    if (totalPassedBytePtr||totalPassedPcktPtr)
    {

        status = cpssHalCncCounterGet(cpssDevId, firstBlock+blockInternalIndex,
                                      ((queueBase&0x1FF)+queue)<<1,
                                      format, totalPassedPcktPtr, totalPassedBytePtr, NULL);
        if (GT_OK != status)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssHalCncCounterGet failed with err : %d on device :%d\n ", status,
                  cpssDevId);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    if (totalDroppedBytePtr||totalDroppedPcktPtr)
    {
        status = cpssHalCncCounterGet(cpssDevId, firstBlock+blockInternalIndex,
                                      (((queueBase&0x1FF)+queue)<<1)+1,
                                      format, totalDroppedPcktPtr, totalDroppedBytePtr, NULL);
        if (GT_OK != status)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssHalCncCounterGet failed with err : %d on device :%d\n ", status,
                  cpssDevId);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    return GT_OK;
}

/**
* @internal cpssHalCncQueueWatermarkGet function
* @endinternal
*
* @brief   Gets the maximum number of buffers occupied by queue
*
* @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] port                     - physical or CPU port number
* @param[in] queue                    - port's queue (0..7)
*
* @param[out] watermark               - (pointer to) the maximum number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssHalCncQueueWatermarkGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            port,
    IN  GT_U32                            queue,
    OUT uint64_t                          *watermark
)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevId;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    /* covert global num to CPSS num */
    rc = cpssHalGetDevNumAndPortNum(devNum, port, &cpssDevId, &cpssPortNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    CPSS_PP_FAMILY_TYPE_ENT devType = cpssHalDevPPFamilyGet(devNum);

    if ((watermark == NULL))
    {
        return GT_BAD_PTR;
    }

    if (devType == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        GT_STATUS   status;
        GT_U32      queueBase = 0, blockInternalIndex;

        status = cpssDxChCncPortQueueGroupBaseGet(cpssDevId, cpssPortNum,
                                                  CPSS_DXCH_CNC_CLIENT_QUEUE_STAT_E, &queueBase);
        if (GT_OK != status)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssDxChCncPortQueueGroupBaseGet failed with err : %d on device :%d\n ",
                  status, cpssDevId);
            return xpsConvertCpssStatusToXPStatus(status);
        }

        blockInternalIndex =
            queueBase>>10;  /* 1024 queues uses one Queue Statistics CNC block */


        status = cpssHalCncCounterGet(cpssDevId,
                                      QUEUE_STAT_WATERMARK_CNC_BLOCK_START + blockInternalIndex,
                                      (queueBase&0x3FF) + queue,
                                      CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_5_E, NULL, NULL, watermark);
        if (GT_OK != status)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  " cpssHalCncCounterGet failed with err : %d on device :%d\n ", status,
                  cpssDevId);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }
    else
    {
        *watermark = 0;
    }

    return rc;
}

/**
* @internal cpssDxChPortPfcPoolHeadroomCountersGet
* @endinternal
*
* @brief  Get headroom current and maximal value for specific pool.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]   devNum                   -physical device number
* @param[in]   poolId                        Pool index[0..1]
* @param[out]  currentValPtr           (pointer to)Current  headroom size
* @param[out] maxPeakValPtr        (pointer to)Maximal headroom size

* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssHalPoolHeadroomCountersGet
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       poolId,
    IN  GT_BOOL                     extendedPool,
    OUT GT_U32                      *currentValPtr,
    OUT GT_U32                      *maxPeakValPtr
)
{
    GT_STATUS rc;

    if (GT_TRUE==extendedPool)
    {
        poolId+=SIP6_SHARED_REGULAR_POOLS_NUM_CNS;
    }

    if (poolId >= SIP6_SHARED_TOTAL_POOLS_NUM_CNS)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Pool id is too big  %d \n",
              poolId);

        return GT_OUT_OF_RANGE;
    }

    if (currentValPtr==NULL||maxPeakValPtr==NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer error\n");

        return GT_BAD_PTR;
    }

    rc = prvCpssFalconTxqPfccHeadroomCounterGet(devNum,
                                                PRV_CPSS_DXCH_FALCON_TXQ_PFCC_MASTER_TILE_MAC,
                                                0/*don't care*/, poolId, PRV_CPSS_DXCH_TXQ_SIP_6_PFCC_HEADROOM_TYPE_POOL,
                                                currentValPtr, maxPeakValPtr);

    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get  pool headroom counter   ");

        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssHalPoolCountersGet function
* @endinternal
*
* @brief   Get buffer pool current occupancy and maximum watermark
*
* @param[in] devNum                  - device number
* @param[in] poolId                  - buffer pool [0..1]
* @param[in] extendedPool            - is extended buffer pool
* @param[out] currentValPtr          - (pointer to) current buffer pool occupancy
* @param[out] maxValPtr              - (pointer to) buffer pool watermark occupancy
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - on wrong pfcCounterNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssHalPoolCountersGet
(
    IN  GT_U8                       devNum,
    IN  GT_U8                       poolId,
    IN  GT_BOOL                     extendedPool,
    OUT GT_U32                      *currentValPtr,
    OUT GT_U32                      *maxValPtr
)
{
    GT_STATUS rc;

    if (GT_TRUE==extendedPool)
    {
        poolId+=SIP6_SHARED_REGULAR_POOLS_NUM_CNS;
    }

    if (poolId >= SIP6_SHARED_TOTAL_POOLS_NUM_CNS)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Pool id is too big  %d \n",
              poolId);

        return GT_OUT_OF_RANGE;
    }

    if (currentValPtr==NULL || maxValPtr==NULL)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Null pointer error\n");

        return GT_BAD_PTR;
    }

    rc = prvCpssSip6TxqPoolCounterGet(devNum, poolId, currentValPtr);

    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get  pool headroom counter   ");

        return rc;
    }

    /* HW TODO: HW may support pool watermark in the future, when we should modify here */
    *maxValPtr = *currentValPtr;

    return GT_OK;
}


/**
* @internal cpssDxChPortPfcPortTcHeadroomCounterGet
* @endinternal
*
* @brief  Get current headroom size for specific port/TC
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] tc                       - Traffic class[0..7]
* @param[out] ctrValPtr               - (pointer to)Headroom size
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssHalPortTcHeadroomCounterGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U8       tc,
    OUT GT_U32      *ctrValPtr
)
{
    GT_STATUS rc;
    GT_U8                cpssDevId;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    /* covert global num to CPSS num */
    rc = cpssHalGetDevNumAndPortNum(devNum, portNum, &cpssDevId, &cpssPortNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChPortPfcPortTcHeadroomCounterGet(cpssDevId, cpssPortNum, tc,
                                                 ctrValPtr);

    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get  port/tc  headroom counter   \n");

        return rc;
    }

    return GT_OK;
}

/**
* @internal  cpssHalPortTxTcMapToSharedPoolSet function
* @endinternal
*
* @brief  Set tail drop mapping between TC to pool
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum       - Device number
* @param[in] tc           - Traffic class [0..7]
* @param[in] poolNum      - Pool id[0..1]
*
* @retval GT_OK                       - on success
* @retval GT_FAIL                     - on error
* @retval GT_HW_ERROR                 - on hardware error
* @retval GT_BAD_PARAM                - on invalid input parameters value
* @retval GT_NOT_APPLICABLE_DEVICE    - on not applicable device
* @retval GT_BAD_PTR                  - one of the parameters is NULL pointer
**/
GT_STATUS cpssHalPortTxTcMapToSharedEgressPoolSet
(
    IN  GT_U8                               devNum,
    IN  GT_U8                               tc,
    IN  GT_U32                              poolNum
)
{
    GT_STATUS status;

    status = prvCpssFalconTxqPfccMapTcToExtendedPoolSet(devNum, tc, poolNum);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to bind tc to pool  "
              "status:%d, cpssdev:%d, pool:%d tc %d\n",
              status, devNum, tc);
        return status;
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortEcnMarkingEnableSet function
* @endinternal
*
* @brief   Enable/Disable ECN marking.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - type of IP stack
* @param[in] enable                   - GT_TRUE: ECN marking enable
*                                      GT_FALSE: ECN marking disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or protocolStack parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
XP_STATUS cpssHalEcnMarkingEnableSet
(
    IN GT_U8                               devNum,
    IN GT_BOOL                             enable
)
{
    GT_STATUS status;

    status = cpssDxChPortEcnMarkingEnableSet(devNum, CPSS_IP_PROTOCOL_IPV4V6_E,
                                             enable);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Failed to enable ECN  "
              "status:%d, cpssdev:%d\n",
              status, devNum);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    return GT_OK;
}

/**
 * @internal cpssHalPolicerInit function
 * @endinternal
 *
 * @brief  Policer Init.
 *
 * @param[in] devId                 - switch Id
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_TIMEOUT               - on time out.
 * @retval GT_HW_ERROR              - on hardware error.
 * @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
 * @retval GT_BAD_STATE             - on counter entry type mismatch.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssHalPolicerInit(uint32_t devId)
{
    GT_STATUS rc = GT_OK;
    GT_U32    cpssDevNum;
    GT_U32    portNum = 0;
    int       maxTotalPorts = 0;
    GT_U32    mru = 0;
    GT_BOOL   enablePortCount;
    XP_DEV_TYPE_T devType;
    CPSS_DXCH_POLICER_STAGE_METER_MODE_ENT mode;
    CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_ENT type;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;
    CPSS_DXCH_POLICER_STORM_TYPE_ENT stormType;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    cpssHalGetMaxGlobalPorts(&maxTotalPorts);
    cpssHalGetDeviceType(devId, &devType);
    mru = cpssHalSys_param_mru(devType);

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        if ((cpssHalDevPPFamilyGet(cpssDevNum) != CPSS_PP_FAMILY_DXCH_FALCON_E) &&
            (cpssHalDevPPFamilyGet(cpssDevNum) != CPSS_PP_FAMILY_DXCH_AC5X_E) &&
            (cpssHalDevPPFamilyGet(cpssDevNum) != CPSS_PP_FAMILY_DXCH_ALDRIN2_E) &&
            (cpssHalDevPPFamilyGet(cpssDevNum) != CPSS_PP_FAMILY_DXCH_AC3X_E) )
        {
            /* TBD */
            return rc;
        }

        for (stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
             stage <= CPSS_DXCH_POLICER_STAGE_EGRESS_E; stage++)
        {

            /* Set the policer global stage mode: IPLR0 uses port based metering and
             * IPLR1 and EPLR uses flow based metering
             * Set the metering address select mode for Stage 0 which is port based
             * metering to compressed address mode(index = portNum << 2 | stormType)
             */

            if (stage == CPSS_DXCH_POLICER_STAGE_INGRESS_0_E)
            {
                mode = CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E;
                type = CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_COMPRESSED_E;
                enablePortCount = GT_TRUE;
            }
            else
            {
                mode = CPSS_DXCH_POLICER_STAGE_METER_MODE_FLOW_E;
                type = CPSS_DXCH_POLICER_PORT_MODE_ADDR_SELECT_TYPE_FULL_E;
                enablePortCount = GT_FALSE;
            }

            /* Enable metering engine for the device */
            rc = cpssDxCh3PolicerMeteringEnableSet(cpssDevNum, stage, GT_TRUE);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxCh3PolicerMeteringEnableSet rc %d, stage %d\n",
                      rc, stage);
                return rc;
            }

            rc = cpssDxChPolicerStageMeterModeSet(cpssDevNum, stage, mode);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPolicerStageMeterModeSet rc %d, stage %d mode %d\n",
                      rc, stage, mode);
                return rc;
            }

            rc = cpssDxChPolicerPortModeAddressSelectSet(cpssDevNum, stage, type);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPolicerPortModeAddressSelectSet rc %d, stage %d\n",
                      rc, stage);
                return rc;
            }

            /* Sets policer global stage counting mode: IPLR0/1 uses Billing(per color counting) which is triggered by metering entry
             * for stage 0 and policy action for stage 1
             */
            rc = cpssDxChPolicerCountingModeSet(cpssDevNum, stage,
                                                CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPolicerCountingModeSet rc %d, stage %d\n",
                      rc, stage);
                return rc;
            }

            /* Packet size mode based on which metering is done */
            rc = cpssDxCh3PolicerPacketSizeModeSet(cpssDevNum, stage,
                                                   CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxCh3PolicerPacketSizeModeSet rc %d, stage %d\n",
                      rc, stage);
                return rc;
            }

            /* Enable counting when port based metering is triggered for the packet */
            rc = cpssDxChPolicerCountingTriggerByPortEnableSet(cpssDevNum, stage,
                                                               enablePortCount);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPolicerCountingTriggerByPortEnableSet rc %d, stage %d\n",
                      rc, stage);
                return rc;
            }

            /* Write back cache flush for policer counting entries for both policer stages */
            rc = cpssDxChPolicerCountingWriteBackCacheFlush(cpssDevNum, stage);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPolicerCountingWriteBackCacheFlush rc %d, stage %d\n",
                      rc, stage);
                return rc;
            }

            /* Set MRU value */
            if (cpssHalDevPPFamilyGet(cpssDevNum) != CPSS_PP_FAMILY_DXCH_AC5X_E)
            {
                rc = cpssDxCh3PolicerMruSet(cpssDevNum, stage, mru);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxCh3PolicerMruSet rc %d, stage %d mru %d\n",
                          rc, stage, mru);
                    return rc;
                }
            }

        }

        /* Falcon has flexible configuration for Policers memory (4K) which is shared b/w PLR 0,1,2 (ingress and egress).
         * 2K - Ingress Stage-1 (port)
         * 1K - Ingress stage-2 (flow)
         * 1K - Egress stage
         */

        if (cpssHalDevPPFamilyGet(cpssDevNum) != CPSS_PP_FAMILY_DXCH_AC5X_E)
        {
            /* This cpss api is not required for AC5X */
            if (!IS_DEVICE_FUJITSU_SMALL(devType))
            {
                rc = cpssDxChPolicerMemorySizeModeSet(cpssDevNum,
                                                      CPSS_DXCH_POLICER_MEMORY_FLEX_MODE_E,
                                                      896,
                                                      1024);
            }
            else
            {
                rc = cpssDxChPolicerMemorySizeModeSet(cpssDevNum,
                                                      CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_0_E,
                                                      1792,
                                                      256);
            }
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPolicerMemorySizeModeSet rc %d\n", rc);
                return rc;
            }
        }
        /* By Default, all storm types mapped to index 0 */
        XPS_GLOBAL_PORT_ITER(portNum, maxTotalPorts)
        {
            cpssPortNum = xpsGlobalPortToPortnum(cpssDevNum, portNum);
            for (stormType = CPSS_DXCH_POLICER_STORM_TYPE_UC_KNOWN_E;
                 stormType <= CPSS_DXCH_POLICER_STORM_TYPE_TCP_SYN_E; stormType++)
            {
                rc = cpssDxChPolicerPortStormTypeIndexSet(cpssDevNum,
                                                          CPSS_DXCH_POLICER_STAGE_INGRESS_0_E, cpssPortNum,
                                                          stormType, 0);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "Failed cpssDxChPolicerPortStormTypeIndexSet, "
                          "rc:%d, dev:%d, cpssPortNum:%d stormType:%d, stormIndex:%d\n", rc, cpssDevNum,
                          cpssPortNum, stormType, 0);
                    return rc;
                }
            }
        }

        /* Enbale Eport based trigger for Egress stage. Used by mirror policer */
        rc = cpssDxChPolicerEAttributesMeteringModeSet(cpssDevNum,
                                                       CPSS_DXCH_POLICER_STAGE_EGRESS_E,
                                                       CPSS_DXCH_POLICER_E_ATTR_METER_MODE_EPORT_ENABLED_E);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChPolicerMemorySizeModeSet rc %d\n", rc);
            return rc;
        }
        if (cpssHalDevPPFamilyGet(cpssDevNum) != CPSS_PP_FAMILY_DXCH_ALDRIN2_E &&
            cpssHalDevPPFamilyGet(cpssDevNum) != CPSS_PP_FAMILY_DXCH_AC3X_E)
        { 
            /* Enable metering for TO_ANALYZER pkts. */
            rc = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet(cpssDevNum,
                                                                         GT_TRUE);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                   "cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet rc %d\n", rc);
                return rc;
            }
        
            /* Set Policer Drop type to HARD. To drop TO_ANALYZER pkts by metering. */
            rc = cpssDxCh3PolicerDropTypeSet(cpssDevNum, CPSS_DXCH_POLICER_STAGE_EGRESS_E,
                                         CPSS_DROP_MODE_HARD_E);
            if (rc != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                    "cpssDxCh3PolicerDropTypeSet rc %d\n", rc);
                return rc;
            }
        }
    }

    return rc;
}

/**
 * @internal cpssHalPolicerDeInit function
 * @endinternal
 *
 * @brief  Policer De-Init.
 *
 * @param[in] devId                 - switch Id
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_TIMEOUT               - on time out.
 * @retval GT_HW_ERROR              - on hardware error.
 * @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
 * @retval GT_BAD_STATE             - on counter entry type mismatch.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssHalPolicerDeInit(uint32_t devId)
{
    /* nothing to be done */
    return GT_OK;
}

/**
 * @internal cpssHalPolicerPortMeteringEnableSet function
 * @endinternal
 *
 * @brief  Enable/disable port metering trigger for pkts arriving on this port.
 *
 * @param[in] devId                 - switch Id
 * @param[in] stage                 - Policer stage type
 * @param[in] portNum               - port number
 * @param[in] enable                - enable/disable port based metering
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
 * @retval GT_HW_ERROR              - on hardware error.
 * @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
 * @retval GT_BAD_STATE             - on counter entry type mismatch.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssHalPolicerPortMeteringEnableSet(uint32_t devId, uint32_t stage,
                                              uint32_t portNum, GT_BOOL enable)
{
    GT_STATUS            rc = GT_OK;
    GT_U8                cpssDevNum;
    GT_PHYSICAL_PORT_NUM cpssPortNum;

    rc = cpssHalGetDevNumAndPortNum(devId, portNum, &cpssDevNum, &cpssPortNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxCh3PolicerPortMeteringEnableSet(cpssDevNum, stage, cpssPortNum,
                                               enable);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxCh3PolicerPortMeteringEnableSet rc %d, stage %d port %d\n",
              rc, stage, cpssPortNum);
        return rc;
    }

    return rc;
}

/**
 * @internal cpssHalPolicerMruGet function
 * @endinternal
 *
 * @brief   Gets the Policer Maximum Receive Unit size.
 *
 * @param[in] devId                 - switch Id
 * @param[in] stage                 - Policer stage type
 * @param[out] mru                  - MRU value in bytes
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_TIMEOUT               - on time out of IPLR Table indirect access.
 * @retval GT_HW_ERROR              - on hardware error.
 * @retval GT_BAD_PARAM             - on wrong devNum, stage or entryIndex.
 * @retval GT_BAD_STATE             - on counter entry type mismatch.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssHalPolicerMruGet(uint32_t devId, uint32_t stage, uint32_t *mru)
{
    GT_STATUS rc = GT_OK;

    if (!mru)
    {
        return GT_BAD_PTR;
    }

    CPSS_PP_FAMILY_TYPE_ENT devFamily = cpssHalDevPPFamilyGet(devId);
    if (devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
    {
        /* cpssDxCh3PolicerMruGet is not applicable for AC5X so returning */
        return rc;
    }

    rc = cpssDxCh3PolicerMruGet(devId, stage, mru);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxCh3PolicerMruGet rc %d, stage %d\n",
              rc, stage);
        return rc;
    }

    return rc;
}

/**
* @internal cpssHalPortPfcTcResourceModeSet
* @endinternal
*
* @brief  Configure mode that count occupied buffers (Packet buffer occupied or Pool occupied).
*              In case Pool mode is selected ,headroom subtraction  can be enabled by settin mode to
*              CPSS_PORT_TX_PFC_RESOURCE_MODE_POOL_WITH_HEADROOM_SUBTRACTION_E
*
* @note   APPLICABLE DEVICES:      Falcon.
*
* @param[in] devId                 - switch Id
* @param[in] tc                    - Traffic class [0..7]
* @param[in] mode                  - resource mode (PB/Pool/Pool with headroom subtraction)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssHalPortPfcTcResourceModeSet(uint32_t devId, uint32_t tc,
                                          CPSS_PORT_TX_PFC_RESOURCE_MODE_ENT mode)
{
    GT_STATUS status;

    GT_U8     cpssDevNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        if (cpssHalDevPPFamilyGet(cpssDevNum) == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            status = cpssDxChPortPfcTcResourceModeSet(cpssDevNum, tc, mode);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to set pfc resource mode  "
                      "status:%d, cpssdev:%d, mode:%d\n",
                      status, cpssDevNum, mode);
                return status;
            }
        }
    }

    return GT_OK;
}

/**
* @internal cpssHalPortPfcDbaTcAvailableBuffersSet
* @endinternal
*
* @brief   Set amount of buffers available for dynamic allocation for PFC.
*
* @param[in] devId                 - switch Id
* @param[in] tc                    - TC
* @param[in] availableBuffers      - Available buffers for DBA
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssHalPortPfcDbaTcAvailableBuffersSet(uint32_t devId, uint32_t tc,
                                                 uint32_t availableBuffers)
{
    GT_STATUS status;
    GT_U8 cpssDevNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        status = cpssDxChPortPfcDbaTcAvailableBuffersSet(cpssDevNum,
                                                         CPSS_DATA_PATH_UNAWARE_MODE_CNS,
                                                         CPSS_PORT_TX_PFC_AVAILABLE_BUFF_CONF_MODE_GLOBAL_TC_E, tc, availableBuffers);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to set pfc dba available buffers "
                  "status:%d, cpssdev:%d, buff:%d, tc: %d\n",
                  status, cpssDevNum, availableBuffers, tc);
            return status;
        }
    }

    return GT_OK;
}

GT_STATUS cpssHalQosMapInit(uint32_t devId)
{
    GT_STATUS rc = GT_OK;
    GT_U8     cpssDevNum;
    GT_U32    qosProfileIdx = 0;
    CPSS_DXCH_COS_PROFILE_STC qosProfile;
    GT_U32 idx = 0;
    GT_U32 dscpIdx = 0;

    XPS_DEVICES_PER_SWITCH_ITER(devId, cpssDevNum)
    {
        for (idx = 0; idx < QOS_MAP_TABLE_NUM; idx++)
        {
            for (dscpIdx = 0; dscpIdx < MAX_L3_MAP_PER_TBL; dscpIdx++)
            {
                qosProfileIdx = idx * MAX_QOS_MAP_PER_TBL + MAX_L2_MAP_PER_TBL + dscpIdx;
                rc = cpssDxChCosDscpToProfileMapSet(cpssDevNum, idx, dscpIdx, qosProfileIdx);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChCosDscpToProfileMapSet dev %d failed(%d)", cpssDevNum, rc);
                    return rc;
                }

                rc = cpssDxChCosProfileEntryGet(cpssDevNum, qosProfileIdx, &qosProfile);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChCosProfileEntryGet dev %d failed(%d)", cpssDevNum, rc);
                    return rc;
                }

                /* update dscp value in QoS profile */

                qosProfile.dscp = dscpIdx;

                rc = cpssDxChCosProfileEntrySet(cpssDevNum, qosProfileIdx, &qosProfile);
                if (rc != GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChCosProfileEntrySet dev %d failed(%d)", cpssDevNum, rc);
                    return rc;
                }
            }
        }
    }
    return GT_OK;
}

GT_STATUS cpssHalPolicerTriggerEntryGet(uint32_t devNum,
                                        CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage,
                                        GT_U32 ePort,
                                        CPSS_DXCH_POLICER_TRIGGER_ENTRY_STC *entryPtr)
{
    GT_STATUS rc = GT_OK;
    rc = cpssDxChPolicerTriggerEntryGet(devNum, stage, ePort, entryPtr);
    if (rc != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalPolicerEPortTriggerEntrySet stage %d ePort %d failed(%d)",
              stage, ePort, rc);
        return rc;
    }

    return rc;
}
