//  xpsAcm.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

/**
 * \xpsAcm.c
 *
 * \brief
 *
 * \DESCRIPTION
 *
 *
 */

#include "xpsAcm.h"
#include "xpsLock.h"
#include "xpsCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* \var xpsAcmInit
* \public
* \brief xpsAcmInit routine for the ACM manager
*
* \return XP_STATUS
*/
XP_STATUS xpsAcmInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcmInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

/**
* \var xpsAcmDeInit
* \public
* \brief uninitialize routine for the ACM manager.
*
* \return XP_STATUS
*/
XP_STATUS xpsAcmDeInit(void)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcmDeInitScope(xpsScope_t scopeId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcmAddDevice(xpsDevice_t devId, xpsInitType_t initType)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcmRemoveDevice(xpsDevice_t devId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcmGetCounterValue(xpsDevice_t devId, xpAcmClient_e client,
                                uint32_t countIndex, uint64_t *nPkts, uint64_t *nBytes)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcmGetSeCounterValue(xpsDevice_t devId, xpAcmClient_e client,
                                  uint32_t seVal, uint32_t countIndex, uint64_t *nPkts, uint64_t *nBytes)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcmPrintCounterValue(xpsDevice_t devId, xpAcmClient_e client,
                                  uint32_t countIndex, int printZeros)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcmPrintAllCounterValue(xpsDevice_t devId, xpAcmClient_e client,
                                     int printZeros)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcmGetSamplingConfig(xpsDevice_t devId, xpAcmClient_e client,
                                  uint32_t index, uint32_t *nSample, uint32_t *mBase, uint32_t *mExpo)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcmSetSamplingConfig(xpsDevice_t devId, xpAcmClient_e client,
                                  uint32_t index, uint32_t nSample, uint32_t mBase, uint32_t mExpo)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcmGetSamplingState(xpsDevice_t devId,  xpAcmClient_e client,
                                 uint32_t index, uint64_t *totalCnt, uint32_t *interEventCnt,
                                 uint32_t *interSampleStart)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcmSetSamplingState(xpsDevice_t devId,  xpAcmClient_e client,
                                 uint32_t index, uint64_t totalCnt,
                                 uint32_t interEventCnt, uint32_t interSampleStart)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcmCntSetGlobalConfigBucketization(xpsDevice_t devId,
                                                uint8_t enable, int startRange, int endRange,
                                                uint8_t numBkts, uint8_t granularity, uint8_t addAddr, uint8_t bktUseAddr)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcmCntSetGlobalConfigModePol(xpsDevice_t devId,
                                          uint8_t refreshEnable, uint8_t unitTime, uint8_t refrTimeGranularity,
                                          uint8_t updateWeight, uint8_t billingCntrEnable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcmPrintSamplingConfig(xpsDevice_t devId, xpAcmClient_e client,
                                    uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcmPrintSamplingState(xpsDevice_t devId, xpAcmClient_e client,
                                   uint32_t index)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAllocateAcm(xpAcmClient_e client, xpAcmType_t type, xpAcm_t *acmId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsReleaseAcm(xpAcmClient_e client, xpAcmType_t type, xpAcm_t acmId)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsSetCountMode(xpsDevice_t devId, xpAcmClient_e client,
                          xpAcmType_t type, int bankMode,
                          int clearOnRead, int wrapAround, int countOffset)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcmSetDropReasonCodeCountingEnable(xpsDevice_t devId,
                                                uint32_t enable)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcmSetLaneIndexBaseSel(xpDevice_t devId, xpAcmClient_e client,
                                    xpAcmType_t type, uint32_t baseSel)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

XP_STATUS xpsAcmGetLaneIndexBaseSel(xpDevice_t devId, xpAcmClient_e client,
                                    xpAcmType_t type, uint32_t *baseSel)
{
    XPS_FUNC_ENTRY_LOG();

    XPS_FUNC_EXIT_LOG();

    return XP_NO_ERR;
}

#ifdef __cplusplus
}
#endif

