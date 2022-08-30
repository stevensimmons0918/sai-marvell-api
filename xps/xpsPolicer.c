// xpsPolicer.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpsAllocator.h"
#include "xpsInternal.h"
#include "xpsPolicer.h"
#include "xpsInit.h"
#include "xpsInterface.h"
#include "xpsPort.h"
#include "xpsLock.h"
#include "cpssHalUtil.h"
#include "cpssHalCopp.h"
#include "cpssHalQos.h"
#include "cpssHalMirror.h"

/* IPLR-1 with 2K entries used for ingress port policer */
#define XPS_INGRESS_PORT_POLICER_INDEX_MAX   2048
#define XPS_INGRESS_PORT_POLICER_INDEX_START 0

/* IPLR-2 with 1K entries used for ingress flow policer */
#define XPS_INGRESS_FLOW_POLICER_INDEX_MAX   1023
#define XPS_INGRESS_FLOW_POLICER_INDEX_START 0

/* EPLR 1K entries - 512 each for egress port & flow policer */
#define XPS_EGRESS_PORT_POLICER_INDEX_MAX    512
#define XPS_EGRESS_PORT_POLICER_INDEX_START  0

#define XPS_EGRESS_FLOW_POLICER_INDEX_MAX    512
#define XPS_EGRESS_FLOW_POLICER_INDEX_START  512

/* CPU code rate limit index 2 to 255 */
/* 0 (FWD_ALL) and 1 (DROP_ALL) is reserved */
#define XPS_COPP_POLICER_INDEX_MAX           255
#define XPS_COPP_POLICER_INDEX_START         2

#ifdef __cplusplus
extern "C" {
#endif

XP_STATUS xpsPolicerInitScope(xpsScope_t scopeId)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    GT_STATUS cpss_status = GT_OK;
    xpsDevice_t  xpsDevId = 0;

    XPS_FUNC_ENTRY_LOG();

    xpsRetVal = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT,
                                            XPS_ALLOCATOR_INGRESS_PORT_POLICER_ENTRY, XPS_INGRESS_PORT_POLICER_INDEX_MAX,
                                            XPS_INGRESS_PORT_POLICER_INDEX_START);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to initialize SAI Policer ID allocator\n");
        return xpsRetVal;
    }

    xpsRetVal = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT,
                                            XPS_ALLOCATOR_EGRESS_PORT_POLICER_ENTRY, XPS_EGRESS_PORT_POLICER_INDEX_MAX,
                                            XPS_EGRESS_PORT_POLICER_INDEX_START);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to initialize SAI Policer ID allocator\n");
        return xpsRetVal;
    }

    xpsRetVal = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT,
                                            XPS_ALLOCATOR_INGRESS_FLOW_POLICER_ENTRY, XPS_INGRESS_FLOW_POLICER_INDEX_MAX,
                                            XPS_INGRESS_FLOW_POLICER_INDEX_START);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to initialize SAI Policer ID allocator\n");
        return xpsRetVal;
    }

    xpsRetVal = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT,
                                            XPS_ALLOCATOR_EGRESS_FLOW_POLICER_ENTRY, XPS_EGRESS_FLOW_POLICER_INDEX_MAX,
                                            XPS_EGRESS_FLOW_POLICER_INDEX_START);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to initialize SAI Policer ID allocator\n");
        return xpsRetVal;
    }

    xpsRetVal = xpsAllocatorInitIdAllocator(XP_SCOPE_DEFAULT,
                                            XPS_ALLOCATOR_COPP_POLICER_ENTRY, XPS_COPP_POLICER_INDEX_MAX,
                                            XPS_COPP_POLICER_INDEX_START);
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to initialize SAI Policer ID allocator\n");
        return xpsRetVal;
    }

    /* Initializing policer engine*/
    cpss_status = cpssHalPolicerInit(xpsDevId);
    if (cpss_status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to initialize policer engine, cpss_status: %d\n",
              cpss_status);
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }

    XPS_FUNC_EXIT_LOG();

    return xpsRetVal;
}

XP_STATUS xpsPolicerIndexAllocate(xpsPolicerType_e policerType,
                                  uint32_t *pPolicerIndex)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;

    switch (policerType)
    {
        case XP_COPP_POLICER:
            xpsRetVal = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT,
                                               XPS_ALLOCATOR_COPP_POLICER_ENTRY, pPolicerIndex);
            break;
        case XP_INGRESS_PORT_POLICER:
            /* for ingress port policer, policer entry index is portNum + stormType */
            xpsRetVal = xpsAllocatorAllocateWithId(XP_SCOPE_DEFAULT,
                                                   XPS_ALLOCATOR_INGRESS_PORT_POLICER_ENTRY, *pPolicerIndex);
            break;
        case XP_EGRESS_PORT_POLICER:
            /* for egress port policer, policer entry index is portNum */
            xpsRetVal = xpsAllocatorAllocateWithId(XP_SCOPE_DEFAULT,
                                                   XPS_ALLOCATOR_EGRESS_PORT_POLICER_ENTRY, *pPolicerIndex);
            break;
        case XP_INGRESS_FLOW_POLICER:
            xpsRetVal = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT,
                                               XPS_ALLOCATOR_INGRESS_FLOW_POLICER_ENTRY, pPolicerIndex);
            break;
        case XP_EGRESS_FLOW_POLICER:
            xpsRetVal = xpsAllocatorAllocateId(XP_SCOPE_DEFAULT,
                                               XPS_ALLOCATOR_EGRESS_FLOW_POLICER_ENTRY, pPolicerIndex);
            break;
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid policer type %d\n",
                  policerType);
            return XP_ERR_INVALID_INPUT;

    }
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to allocate Policer Id |Error: %d\n",
              xpsRetVal);
        return xpsRetVal;
    }

    return xpsRetVal;
}

XP_STATUS xpsPolicerIndexRelease(xpsPolicerType_e policerType,
                                 uint32_t pPolicerIndex)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;

    switch (policerType)
    {
        case XP_COPP_POLICER:
            xpsRetVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT,
                                              XPS_ALLOCATOR_COPP_POLICER_ENTRY, pPolicerIndex);
            break;
        case XP_INGRESS_PORT_POLICER:
            xpsRetVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT,
                                              XPS_ALLOCATOR_INGRESS_PORT_POLICER_ENTRY, pPolicerIndex);
            break;
        case XP_EGRESS_PORT_POLICER:
            xpsRetVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT,
                                              XPS_ALLOCATOR_EGRESS_PORT_POLICER_ENTRY, pPolicerIndex);
            break;
        case XP_INGRESS_FLOW_POLICER:
            xpsRetVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT,
                                              XPS_ALLOCATOR_INGRESS_FLOW_POLICER_ENTRY, pPolicerIndex);
            break;
        case XP_EGRESS_FLOW_POLICER:
            xpsRetVal = xpsAllocatorReleaseId(XP_SCOPE_DEFAULT,
                                              XPS_ALLOCATOR_EGRESS_FLOW_POLICER_ENTRY, pPolicerIndex);
            break;
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid policer type %d\n",
                  policerType);
            return XP_ERR_INVALID_INPUT;

    }
    if (xpsRetVal != XP_NO_ERR)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to release Policer Id |Error: %d\n",
              xpsRetVal);
        return xpsRetVal;
    }

    return xpsRetVal;
}

XP_STATUS xpsPolicerPortPolicerIndexGet(xpsDevice_t devId, uint32_t portId,
                                        xpsPolicerType_e policerType, xpsPolicerStormType_e stormType,
                                        uint32_t *pPolicerIndex)
{
    XP_STATUS xpsRetVal = XP_NO_ERR;
    uint32_t portNum;

    portNum = xpsGlobalPortToPortnum(devId, portId);
    if (portNum == 0xffff)
    {
        return xpsConvertCpssStatusToXPStatus(GT_FAIL);
    }

    switch (policerType)
    {
        case XP_INGRESS_PORT_POLICER:
            /* for ingress port policer, policer entry index is portNum + stormType */
            *pPolicerIndex = (portNum << 2) | stormType;
            break;
        case XP_EGRESS_PORT_POLICER:
            /* for egress port policer, policer entry index is portNum */
            *pPolicerIndex = portNum;
            break;
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid policer type %d\n",
                  policerType);
            return XP_ERR_INVALID_INPUT;
    }

    return xpsRetVal;
}

XP_STATUS xpsPolicerStormTypeEnable(xpsDevice_t devId, uint32_t portId,
                                    xpsPolicerStormType_e stormType, bool enable)
{
    GT_STATUS status = GT_OK;
    XP_STATUS xpsRetVal = XP_NO_ERR;
    uint32_t stormIndex = (enable) ? stormType : 0;

    switch (stormType)
    {
        case XPS_UNKNOWN_STORM_CONTROL:
            status = cpssHalQosPolicerPortStormTypeIndexSet(devId, portId,
                                                            CPSS_DXCH_POLICER_STAGE_INGRESS_0_E,
                                                            CPSS_DXCH_POLICER_STORM_TYPE_UC_UNKNOWN_E,
                                                            stormIndex);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Error: Failed to set policer storm type %d, Index %d, cpss_status: %d\n",
                      stormType, stormIndex, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            status = cpssHalQosPolicerPortStormTypeIndexSet(devId, portId,
                                                            CPSS_DXCH_POLICER_STAGE_INGRESS_0_E,
                                                            CPSS_DXCH_POLICER_STORM_TYPE_MC_UNREGISTERED_E,
                                                            stormIndex);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Error: Failed to set policer storm type %d, Index %d, cpss_status: %d\n",
                      stormType, stormIndex, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_MC_STORM_CONTROL:
            status = cpssHalQosPolicerPortStormTypeIndexSet(devId, portId,
                                                            CPSS_DXCH_POLICER_STAGE_INGRESS_0_E,
                                                            CPSS_DXCH_POLICER_STORM_TYPE_MC_REGISTERED_E,
                                                            stormIndex);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Error: Failed to set policer storm type %d, Index %d, cpss_status: %d\n",
                      stormType, stormIndex, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_BC_STORM_CONTROL:
            status = cpssHalQosPolicerPortStormTypeIndexSet(devId, portId,
                                                            CPSS_DXCH_POLICER_STAGE_INGRESS_0_E,
                                                            CPSS_DXCH_POLICER_STORM_TYPE_BC_E,
                                                            stormIndex);
            if (status != GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Error: Failed to set policer storm type %d, Index %d, cpss_status: %d\n",
                      stormType, stormIndex, status);
                return xpsConvertCpssStatusToXPStatus(status);
            }
            break;
        case XPS_ALL_STORM_CONTROL:
            /* do nothing */
            /* by default, all storm types are mapped to index zero */
            break;
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid storm type %d\n",
                  stormType);
            return XP_ERR_INVALID_INPUT;
    }
    return xpsRetVal;
}

XP_STATUS xpsPolicerInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    return xpsPolicerInitScope(XP_SCOPE_DEFAULT);
}

XP_STATUS xpsPolicerDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS cpss_status = GT_OK;
    xpsDevice_t  xpsDevId = 0;

    cpss_status = cpssHalPolicerDeInit(xpsDevId);
    if (cpss_status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to DeInitialize policer engine, cpss_status: %d\n",
              cpss_status);
        return xpsConvertCpssStatusToXPStatus(cpss_status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    return xpsPolicerDeInitScope(XP_SCOPE_DEFAULT);
}

XP_STATUS xpsPolicerAddDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerEnablePortPolicing(xpsInterfaceId_t portIntfId,
                                       uint16_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerEnablePortPolicingScope(xpsScope_t scopeId,
                                            xpsInterfaceId_t portIntfId, uint16_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/* Get Egress Policer Flow counter index. We will use standard mode for billing counting. */
uint32_t xpsPolicerGetEgressFlowCounterIndex(uint32_t index)
{
    uint32_t egressPolicerMaxFlowId = CPSSHAL_ERSPAN_EPLR_FLOW_ID_BASE +
                                      CPSSHAL_ERSPAN_EPLR_FLOW_ID_MAX;
    uint32_t mirrorPolicerMinAvailableId = XPS_EGRESS_FLOW_POLICER_INDEX_START +
                                           XP_MIRROR_MAX_USER_SESSION + 1;

    if (index <  mirrorPolicerMinAvailableId)
    {
        return index;
    }

    return (egressPolicerMaxFlowId + index - mirrorPolicerMinAvailableId);
}


XP_STATUS xpsPolicerAddEntry(xpsDevice_t devId, xpsPolicerType_e policerType,
                             uint32_t index, uint32_t counterIndex, xpsPolicerEntry_t *pEntry)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS status = GT_OK;
    CPSS_DXCH3_POLICER_METERING_ENTRY_STC entryPtr;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT         stage;
    memset(&entryPtr, 0, sizeof(entryPtr));

    switch (policerType)
    {
        case XP_INGRESS_PORT_POLICER:
            stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
            break;
        case XP_INGRESS_FLOW_POLICER:
            stage = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
            break;
        case XP_EGRESS_PORT_POLICER:
        case XP_EGRESS_FLOW_POLICER:
        case XP_COPP_POLICER:
            stage = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
            break;
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid policer type %d\n",
                  policerType);
            return XP_ERR_INVALID_INPUT;
            break;
    }

    uint32_t mru = 0;
    status = cpssHalPolicerMruGet(devId, stage, &mru);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to get policer mru, status: %d\n", status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    if (pEntry->isPacketBasedPolicing)
    {
        entryPtr.byteOrPacketCountingMode =
            CPSS_DXCH3_POLICER_METER_RESOLUTION_PACKETS_E;
    }
    else
    {
        entryPtr.byteOrPacketCountingMode = CPSS_DXCH3_POLICER_METER_RESOLUTION_BYTES_E;
    }

    if (pEntry->mode == XPS_POLICER_MODE_SRTC_E)
    {
        entryPtr.meterMode = CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E;
        entryPtr.tokenBucketParams.srTcmParams.cir = pEntry->cir;
        entryPtr.tokenBucketParams.srTcmParams.cbs = mru + pEntry->cbs;
        entryPtr.tokenBucketParams.srTcmParams.ebs = mru + pEntry->pbs;
    }
    else if (pEntry->mode == XPS_POLICER_MODE_STORM_CONTROL)
    {
        entryPtr.meterMode = CPSS_DXCH3_POLICER_METER_MODE_SR_TCM_E;
        entryPtr.tokenBucketParams.srTcmParams.cir = pEntry->cir;
        entryPtr.tokenBucketParams.srTcmParams.cbs = mru + pEntry->cbs;
        entryPtr.tokenBucketParams.srTcmParams.ebs = mru;
    }
    else if (pEntry->mode == XPS_POLICER_MODE_TRTC_E)
    {
        entryPtr.meterMode = CPSS_DXCH3_POLICER_METER_MODE_TR_TCM_E;
        entryPtr.tokenBucketParams.trTcmParams.cir = pEntry->cir;
        entryPtr.tokenBucketParams.trTcmParams.cbs = mru + pEntry->cbs;
        entryPtr.tokenBucketParams.trTcmParams.pir = pEntry->pir;
        entryPtr.tokenBucketParams.trTcmParams.pbs = mru + pEntry->pbs;
    }

    if (pEntry->colorAware)
    {
        entryPtr.meterColorMode = CPSS_POLICER_COLOR_AWARE_E;
    }
    else
    {
        entryPtr.meterColorMode = CPSS_POLICER_COLOR_BLIND_E;
    }

    if (pEntry->dropGreen)
    {
        entryPtr.greenPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E;
    }
    else
    {
        entryPtr.greenPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
    }

    if (pEntry->dropYellow)
    {
        entryPtr.yellowPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E;
    }
    else
    {
        entryPtr.yellowPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
    }

    if (pEntry->dropRed)
    {
        entryPtr.redPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_DROP_E;
    }
    else
    {
        entryPtr.redPcktCmd = CPSS_DXCH3_POLICER_NON_CONFORM_CMD_NO_CHANGE_E;
    }

    entryPtr.modifyUp = CPSS_DXCH_POLICER_MODIFY_UP_DISABLE_E;
    entryPtr.modifyDp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    entryPtr.modifyDscp = CPSS_DXCH_POLICER_MODIFY_DSCP_DISABLE_E;
    entryPtr.modifyExp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    entryPtr.modifyTc = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    entryPtr.packetSizeMode = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
    entryPtr.mngCounterSet = CPSS_DXCH3_POLICER_MNG_CNTR_DISABLED_E;
    if (policerType == XP_EGRESS_FLOW_POLICER)
    {
        entryPtr.countingEntryIndex = xpsPolicerGetEgressFlowCounterIndex(counterIndex);
    }
    else
    {
        entryPtr.countingEntryIndex = counterIndex;
    }

    status = cpssHalQosAddPolicerEntry(devId, stage, index, &entryPtr);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Error: Failed to add policer entry, cpss_status: %d\n", status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerRemoveEntry(xpsDevice_t devId, xpsPolicerType_e client,
                                uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;
    GT_STATUS status = GT_OK;

    /* Do need to clear the entry in HW.
     * Policer entry will be overwritten when enabled again. */

    switch (client)
    {
        case XP_INGRESS_PORT_POLICER:
            stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
            break;
        case XP_INGRESS_FLOW_POLICER:
            stage = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
            break;
        case XP_EGRESS_PORT_POLICER:
        case XP_EGRESS_FLOW_POLICER:
        case XP_COPP_POLICER:
            stage = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
            break;
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR, "Invalid policer type %d\n",
                  client);
            return XP_ERR_INVALID_INPUT;
            break;
    }

    status = cpssHalPolicerCounterEntryClear(devId,
                                             stage,
                                             index);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalPolicerCounterEntryClear fail:%d", status);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerGetEntry(xpsDevice_t devId, xpsPolicerType_e client,
                             uint32_t index, xpsPolicerEntry_t *pEntry)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS status;
    memset(pEntry, 0, sizeof(xpPolicerEntry_t));

    if (client == XP_COPP_POLICER)
    {
        /* Supports only PPS mode */
        status = cpssHalCoppGetCpuRateLimit(devId, index, &(pEntry->cir));
        if (GT_OK != status)
        {
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerGetPolicerCounterEntry(xpsDevice_t devId,
                                           xpsPolicerType_e policerType, uint32_t index, xpsPolicerCounterEntry_t *entry)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS                        status = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;

    if (!entry)
    {
        return XP_ERR_NULL_POINTER;
    }

    switch (policerType)
    {
        case XP_INGRESS_PORT_POLICER:
            stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
            break;
        case XP_INGRESS_FLOW_POLICER:
            stage = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
            break;
        case XP_EGRESS_FLOW_POLICER:
            stage = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
            index = xpsPolicerGetEgressFlowCounterIndex(index);
            break;
        default:
            stage = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
            break;
    }

    /* TODO need to check for cpuCodeRateLimiter counters */

    if (policerType != XP_COPP_POLICER)
    {
        status = cpssHalPolicerCounterEntryGet(devId, stage, index, &entry->greenEvent,
                                               &entry->yellowEvent, &entry->redEvent);
        if (status != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Fetch policer counter information failed, Index(%u)", index);
            return xpsConvertCpssStatusToXPStatus(status);
        }
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerClearPolicerCounterEntry(xpsDevice_t devId,
                                             xpsPolicerType_e policerType, uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS                        status = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;

    switch (policerType)
    {
        case XP_INGRESS_PORT_POLICER:
            stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
            break;
        case XP_INGRESS_FLOW_POLICER:
            stage = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
            break;
        case XP_EGRESS_FLOW_POLICER:
            stage = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
            index = xpsPolicerGetEgressFlowCounterIndex(index);
            break;
        default:
            stage = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
            break;
    }

    status = cpssHalPolicerCounterEntryClear(devId, stage, index);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Clear policer counter failed, Index(%u)", index);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerEPortPolicingEnable(xpsDevice_t devId, uint32_t ePort,
                                        xpsPolicerType_e policerType, uint32_t index, bool enable)
{
    XPS_FUNC_ENTRY_LOG();

    GT_STATUS                        status = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;

    switch (policerType)
    {
        case XP_INGRESS_PORT_POLICER:
            stage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
            break;
        case XP_INGRESS_FLOW_POLICER:
            stage = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
            break;
        case XP_EGRESS_PORT_POLICER:
        case XP_EGRESS_FLOW_POLICER:
            stage = CPSS_DXCH_POLICER_STAGE_EGRESS_E;
            break;
        default:
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "policer type not valid\n");
            return XP_ERR_INVALID_VALUE;
    }

    status = cpssHalPolicerEPortTriggerEntrySet(devId, stage, ePort, index, enable);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Set Eport policer entry failed, Index(%u)", index);
        return xpsConvertCpssStatusToXPStatus(status);
    }

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerAddPortPolicingEntry(xpsInterfaceId_t portIntfId,
                                         xpsPolicerType_e client, xpsPolicerEntry_t *pEntry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerAddPortPolicingEntryScope(xpsScope_t scopeId,
                                              xpsInterfaceId_t portIntfId, xpsPolicerType_e client,
                                              xpsPolicerEntry_t *pEntry)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerRemovePortPolicingEntry(xpsInterfaceId_t portIntfId,
                                            xpsPolicerType_e client)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerRemovePortPolicingEntryScope(xpsScope_t scopeId,
                                                 xpsInterfaceId_t portIntfId, xpsPolicerType_e client)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerSetAttribute(xpsDevice_t devId, xpsPolicerType_e client,
                                 uint32_t index, xpPolicerField_t field, void *data)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerGetAttribute(xpsDevice_t devId,  xpsPolicerType_e client,
                                 uint32_t index, xpPolicerField_t field, void *data)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerSetResultByType(xpsDevice_t devId, xpsPolicerType_e client,
                                    uint32_t index, xpPolicingResultType resultType,
                                    xpPolicerResult_t *redPolResult, xpPolicerResult_t *yellowPolResult,
                                    xpPolicerResult_t *greenPolResult)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerGetResultByType(xpsDevice_t devId, xpsPolicerType_e client,
                                    uint32_t index, xpPolicingResultType resultType,
                                    xpPolicerResult_t *redPolResult, xpPolicerResult_t *yellowPolResult,
                                    xpPolicerResult_t *greenPolResult)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerSetResultByColor(xpsDevice_t devId,
                                     xpsPolicerType_e client,
                                     uint32_t index, xpPolicingResultType resultType, xpPolicingResultColor color,
                                     xpPolicerResult_t *polResult)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerSetPolicerStandard(xpDevice_t devId,
                                       xpPolicerStandard_e polStandard)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerGetPolicerStandard(xpDevice_t devId,
                                       xpPolicerStandard_e* polStandard)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerEnablePacketBasedPolicing(xpDevice_t devId,
                                              xpsPolicerType_e client, uint32_t index, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerIsPacketBasedPolicingEnabled(xpDevice_t devId,
                                                 xpsPolicerType_e client, uint32_t index, uint32_t* enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerEnableByteBasedPolicing(xpDevice_t devId,
                                            xpsPolicerType_e client, uint32_t index, uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsPolicerIsByteBasedPolicingEnabled(xpDevice_t devId,
                                               xpsPolicerType_e client, uint32_t index, uint32_t* enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif
