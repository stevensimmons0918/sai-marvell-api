/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalMirror.c
*
* @brief Private API implementation which can be used in XPS layer.
*
* @version   01
********************************************************************************
*/


#include "cpssHalUtil.h"
#include "cpssHalDevice.h"
#include "cpssHalMirror.h"
#include "cpssHalL3.h"
#include "cpssHalQos.h"
#include "cpssDxChBrgGen.h"
#include "xpsCommon.h"
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOam.h>
#include <cpss/dxCh/dxChxGen/policer/private/prvCpssDxChPolicer.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include "xpsMtuProfile.h"

#define OAM_TLV_DATA_OFFSET_STC 4

/**
* @internal cpssHalSetMirrorInit function
* @endinternal
*
* @brief  Mirror init
*
* @param[in] devNum                - device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalSetMirrorInit
(
    int                      devId
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChMirrorTreatToAnalyzerDsaPktAsIngressMirroredEnableSet(devNum,
                                                                               GT_TRUE);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Setting cpssDxChMirrorTreatToAnalyzerDsaPktAsIngressMirroredEnableSet failed for devNum=%d",
                  devNum);
            return status;
        }
    }

    return status;
}

/**
* @internal cpssHalSetMirrorTruncateState function
* @endinternal
*
* @brief  This function sets if to truncate TO_ANALYZER packets per analyzer index.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer interface index. (APPLICABLE RANGES: 0..6)
* @param[in] truncate                 - Indicates if truncate TO_ANALYZER packets to 128B
*                                       GT_TRUE  truncate, GT_FALSE  do not truncate.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalSetMirrorTruncateState
(
    int                      devId,
    GT_U32                  index,
    GT_BOOL                 truncate
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChMirrorToAnalyzerTruncateSet(devNum, index, truncate);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Setting truncation state failed for devNum=%d index=%d", devNum, index);
            return status;
        }
    }

    return status;
}

/**
* @internal cpssHalMirrorAnalyzerDpTcSet function
* @endinternal
*
* @brief   The TC/DP assigned to the packet forwarded to the inress/egress analyzer port due
*         to egress mirroring to the analyzer port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devId                    - device number.
* @param[in] analyzerDp               - the Drop Precedence to be set
* @param[in] analyzerTc               - traffic class on analyzer port (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalMirrorAnalyzerDpTcSet
(
    IN GT_U8              devId,
    IN GT_BOOL            isRx,
    IN CPSS_DP_LEVEL_ENT  analyzerDp,
    IN GT_U8              analyzerTc
)
{
    GT_STATUS status = GT_OK;
    int devNum;
    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        if (isRx)
        {
            status = cpssDxChMirrRxAnalyzerDpTcSet(devNum, analyzerDp, analyzerTc);
        }
        else
        {
            status = cpssDxChMirrTxAnalyzerDpTcSet(devNum, analyzerDp, analyzerTc);
        }
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Setting dp/tc failed for devNum=%d direction=%d dp=% tc=%d", devNum, isRx,
                  analyzerDp, analyzerTc);
            return status;
        }
    }

    return status;
}

/**
* @internal cpssHalAnalyzerPortSet function
* @endinternal
*
* @brief   This function sets analyzer interface.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] analyzerIndex            - of analyzer interface. (APPLICABLE RANGES: 0..6)
* @param[in] interfacePtr             - Pointer to analyzer interface.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index, interface type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on wrong port or device number in interfacePtr.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalAnalyzerPortSet
(
    IN GT_U8                   devId,
    IN GT_U32                  analyzerIndex,
    IN CPSS_INTERFACE_TYPE_ENT type,
    IN GT_U32                  analyzerPort,
    IN GT_TRUNK_ID             analyzerTrunkId,
    IN GT_PORT_NUM             ePort
)
{
    GT_STATUS                                 status = GT_OK;
    int                                       devNum;
    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC   interface;
    CPSS_INTERFACE_INFO_STC                   physicalInfo;
    GT_U32                                    cpssDevNum;
    GT_U32                                    cpssPortNum;

    memset(&interface, 0, sizeof(interface));
    memset(&physicalInfo, 0, sizeof(physicalInfo));

    cpssHalSetDeviceSwitchId(devId);
    cpssDevNum = xpsGlobalIdToDevId(devId, analyzerPort);
    cpssPortNum = xpsGlobalPortToPortnum(devId, analyzerPort);

    if (type == CPSS_INTERFACE_PORT_E)
    {
        physicalInfo.type = CPSS_INTERFACE_PORT_E;
        physicalInfo.devPort.hwDevNum = cpssDevNum;
        physicalInfo.devPort.portNum = cpssPortNum;
    }
    else if (type == CPSS_INTERFACE_TRUNK_E)
    {
        physicalInfo.type = CPSS_INTERFACE_TRUNK_E;
        physicalInfo.trunkId = analyzerTrunkId;
    }
    else
    {
        return GT_NOT_SUPPORTED;
    }

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        if (cpssPortNum != CPSS_CPU_PORT_NUM_CNS)
        {
            status = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                         ePort,
                                                                         &physicalInfo);
            if (status!= GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Setting analyzer interface devNum=%d index=%d port=%d", devNum, analyzerIndex,
                      ePort);
                return status;
            }

            // Set ePort to Mirror session
            interface.interface.type = CPSS_INTERFACE_PORT_E;
            interface.interface.devPort.hwDevNum = cpssDevNum;
            interface.interface.devPort.portNum = ePort;
            status = cpssDxChMirrorAnalyzerInterfaceSet(devNum, analyzerIndex, &interface);
            if (status!= GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Setting analyzer interface devNum=%d index=%d port=%d", devNum, analyzerIndex,
                      ePort);
                return status;
            }
        }
        else
        {
            // Set ePort to Mirror session
            interface.interface.type = CPSS_INTERFACE_PORT_E;
            interface.interface.devPort.hwDevNum = cpssDevNum;
            interface.interface.devPort.portNum = CPSS_CPU_PORT_NUM_CNS;
            ePort = CPSS_CPU_PORT_NUM_CNS;
            status = cpssDxChMirrorAnalyzerInterfaceSet(devNum, analyzerIndex, &interface);
            if (status!= GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Setting analyzer interface devNum=%d index=%d port=%d", devNum, analyzerIndex,
                      ePort);
                return status;
            }
            status = cpssDxChMirrorTreatEgressMirroredAsIngressMirroredEnableSet(devNum,
                                                                                 analyzerIndex, GT_TRUE);
            if (status!= GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Setting Egress mirror config devNum=%d index=%d enable=TRUE", devNum,
                      analyzerIndex);
                return status;
            }
        }

    }

    if (cpssPortNum != 0xFFFF)
    {
        status = cpssHalAnalyzerEPortMacSet(devId, analyzerPort, ePort);
        if (status!= GT_OK)
        {
            return status;
        }
    }

    return status;
}

GT_STATUS cpssHalAnalyzerEPortMacSet
(
    IN GT_U8              devId,
    IN GT_U32             portNum,
    IN GT_PORT_NUM        ePort
)
{
    GT_STATUS status = GT_OK;
    GT_U32    macSaTableIndex;
    GT_U32    mtuProfileIdx;
    GT_U32    cpssDevNum;
    GT_U32    cpssPortNum;
    int       devNum;

    cpssHalSetDeviceSwitchId(devId);
    cpssDevNum = xpsGlobalIdToDevId(devId, portNum);
    cpssPortNum = xpsGlobalPortToPortnum(devId, portNum);

    status = cpssDxChIpRouterPortGlobalMacSaIndexGet(cpssDevNum, cpssPortNum,
                                                     &macSaTableIndex);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get MAC SA table index for port id %d\n", cpssPortNum);
        return (status);
    }
    status = cpssDxChIpRouterPortGlobalMacSaIndexSet(cpssDevNum, ePort,
                                                     macSaTableIndex);
    if (status != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set MAC SA table index to port id %d\n", ePort);
        return (status);
    }

    // Set MTU Profile for ePort
    status = cpssDxChBrgGenMtuPortProfileIdxGet(cpssDevNum, cpssPortNum,
                                                &mtuProfileIdx);
    if (status!= GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to get MTU profile Index for analyzer port %d", cpssPortNum);
        return status;
    }
    if (cpssPortNum == CPSS_CPU_PORT_NUM_CNS)
    {
        uint32_t mtuSize = 0;
        uint8_t i = 0;
        for (i = 0; i < MTU_PROFILE_INDEX_MAX_VAL; i++)
        {
            XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
            {
                status = cpssDxChBrgGenMtuProfileGet(devNum, i, &mtuSize);
                if ((status == GT_OK) && (mtuSize > 0))
                {
                    break;
                }
            }
            if (mtuSize > 0)
            {
                break;
            }
        }
        mtuProfileIdx = ((i == MTU_PROFILE_INDEX_MAX_VAL) ? 0 : i);
    }
    status = cpssDxChBrgGenMtuPortProfileIdxSet(cpssDevNum, ePort, mtuProfileIdx);
    if (status!= GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "Failed to set MTU profile Index for ePort %d", ePort);
        return status;
    }
    return status;
}

GT_STATUS cpssHalAnalyzerPortClear
(
    IN GT_U8              devId,
    IN GT_U32             analyzerIndex,
    IN GT_U32             analyzerPort,
    IN GT_PORT_NUM        ePort
)
{
    GT_STATUS                                 status = GT_OK;
    int                                       devNum;
    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC   interface;
    CPSS_INTERFACE_INFO_STC                   physicalInfo;

    GT_U32 cpssPortNum = xpsGlobalPortToPortnum(devId, analyzerPort);

    memset(&interface, 0, sizeof(interface));
    memset(&physicalInfo, 0, sizeof(physicalInfo));

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        if (cpssPortNum != CPSS_CPU_PORT_NUM_CNS)
        {
            // clear ePort to Physical port mapping
            status = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum,
                                                                         ePort,
                                                                         &physicalInfo);
            if (status!= GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to clear eport to physical port mapping with error %d", status);
                return status;
            }

            // clear ePort to Mirror session
            status = cpssDxChMirrorAnalyzerInterfaceSet(devNum, analyzerIndex, &interface);
            if (status!= GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      " Failed to clear eport from analyzer index with error %d", status);
                return status;
            }
        }
        else
        {
            status = cpssDxChMirrorAnalyzerInterfaceSet(devNum, analyzerIndex, &interface);
            if (status!= GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Setting analyzer interface devNum=%d index=%d port=%d", devNum, analyzerIndex,
                      ePort);
                return status;
            }

            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEBUG,
                  "Setting analyzer interface devNum=%d index=%d port=%d", devNum, analyzerIndex,
                  ePort);

            status = cpssDxChMirrorTreatEgressMirroredAsIngressMirroredEnableSet(devNum,
                                                                                 analyzerIndex, GT_FALSE);
            if (status!= GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Setting Egress mirror config devNum=%d index=%d enable=FALSE", devNum,
                      analyzerIndex);
                return status;
            }
        }
    }

    return status;
}

/**
* @internal cpssHalBindAnalyzerToTunnel function
* @endinternal
*
* @brief   This function sets analyzer binding.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                - device number.
* @param[in] analyzerIndex         - of analyzer interface. (APPLICABLE RANGES: 0..6)
* @param[in] tunnelStartIndex      - tunnel start index.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalBindAnalyzerToTunnel
(
    IN GT_U8              devId,
    IN GT_U32             analyzerIndex,
    IN GT_U32             tunnelStartIndex,
    IN GT_PORT_NUM        ePort
)
{
    GT_STATUS                           status = GT_OK;
    int                                 devNum;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC  egressInfo;

    memset(&egressInfo, 0, sizeof(egressInfo));
    egressInfo.tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
    egressInfo.forceArpTsPortDecision = GT_TRUE;
    egressInfo.tunnelStart = GT_TRUE;
    egressInfo.tunnelStartPtr = tunnelStartIndex;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, ePort,
                                                                 &egressInfo);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Setting analyzer ePort as tunnel failed devNum=%d index=%d port=%d tunnel=%d",
                  devNum, analyzerIndex, ePort, tunnelStartIndex);
            return status;
        }
    }

    return status;
}

/**
* @internal cpssHalUnBindAnalyzerFromTunnel function
* @endinternal
*
* @brief   This function unbind tunnel index from eport.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                - device number.
* @param[in] analyzerIndex         - of analyzer interface. (APPLICABLE RANGES: 0..6)
* @param[in] eport                 - eport
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalUnBindAnalyzerFromTunnel
(
    IN GT_U8              devId,
    IN GT_U32             analyzerIndex,
    IN GT_PORT_NUM        ePort
)
{
    GT_STATUS                           status = GT_OK;
    int                                 devNum;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC  egressInfo;

    memset(&egressInfo, 0, sizeof(egressInfo));
    egressInfo.tsPassengerPacketType = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
    egressInfo.forceArpTsPortDecision = GT_TRUE;
    egressInfo.tunnelStart = GT_FALSE;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, ePort,
                                                                 &egressInfo);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Unbinding analyzer ePort to tunnel failed devNum=%d index=%d port=%d\n",
                  devNum, analyzerIndex, ePort);
            return status;
        }
    }

    return status;
}

/**
* @internal cpssHalMirrorAnalyzerSampleRateSet function
* @endinternal
*
* @brief   Set the statistical mirroring rate in the Rx Analyzer port
*         Set Statistic mirroring to analyzer port statistical ratio configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                  - device number.
* @param[in] sampleRate              - Indicates the  of egress mirrored to analyzer port packets
*                                      forwarded to the analyzer port. 1 of every 'ratio' packets are
*                                      forwarded to the analyzer port.
* @param[in] isIngress               - Indicate analyzer position - ingress or egress
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or ratio.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Examples:
*       ratio 0 -> no Rx mirroring
*       ratio 1 -> all Rx mirrored packets are forwarded out the Rx
*       Analyzer port
*       ratio 10 -> 1 in 10 Rx mirrored packets are forwarded out the Rx
*       Analyzer port
*       The maximum ratio value is 2047.
*
*/
GT_STATUS cpssHalMirrorAnalyzerSampleRateSet
(
    IN GT_U8    devId,
    IN GT_U32   index,
    IN GT_U32   sampleRate,
    IN GT_BOOL  isIngress
)
{
    GT_STATUS           status = GT_OK;
    int                 devNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        if (isIngress)
        {
            status = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(devNum, index,
                                                                          true, sampleRate);
        }
        else
        {
            status = cpssDxChMirrorTxStatMirrorToAnalyzerIndexRatioSet(devNum, index,
                                                                       sampleRate);
        }
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Setting analyzer sample rate devNum=%d sampleRate=%d isIngress=%d", devNum,
                  sampleRate, isIngress);
            return status;
        }
    }

    return status;
}

/**
* @internal cpssHalMirrorSampleRateSet function
* @endinternal
*
* @brief   Set the statistical mirroring rate in the Rx Analyzer port
*         Set Statistic mirroring to analyzer port statistical ratio configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                  - device number.
* @param[in] sampleRate              - Indicates the  of egress mirrored to analyzer port packets
*                                      forwarded to the analyzer port. 1 of every 'ratio' packets are
*                                      forwarded to the analyzer port.
* @param[in] isIngress               - Indicate analyzer position - ingress or egress
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or ratio.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Examples:
*       ratio 0 -> no Rx mirroring
*       ratio 1 -> all Rx mirrored packets are forwarded out the Rx
*       Analyzer port
*       ratio 10 -> 1 in 10 Rx mirrored packets are forwarded out the Rx
*       Analyzer port
*       The maximum ratio value is 2047.
*
*/
GT_STATUS cpssHalMirrorSampleRateSet
(
    IN GT_U8    devId,
    IN GT_U32   sampleRate,
    IN GT_BOOL  isIngress
)
{
    GT_STATUS           status = GT_OK;
    int                 devNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        if (isIngress)
        {
            status = cpssDxChMirrRxStatMirroringToAnalyzerRatioSet(devNum, sampleRate);
        }
        else
        {
            status = cpssDxChMirrTxStatMirroringToAnalyzerRatioSet(devNum, sampleRate);
        }
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Setting analyzer sample rate devNum=%d sampleRate=%d isIngress=%d", devNum,
                  sampleRate, isIngress);
            return status;
        }
    }

    return status;
}

/**
* @internal cpssDxChMirrorAnalyzerMirrorOnDropEnableSet function
* @endinternal
*
* @brief   Enable / Disable mirroring of dropped packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
* @param[in] enable                   - GT_TRUE  - dropped packets are mirrored to analyzer interface.
*                                      GT_FALSE - dropped packets are not mirrored to analyzer interface.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalMirrorAnalyzerMirrorOnDropEnableSet
(
    IN GT_U8     devId,
    IN GT_U32    analyzerIndex,
    IN GT_BOOL   enable
)
{
    GT_STATUS           status = GT_OK;
    int                 devNum;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChMirrorAnalyzerMirrorOnDropEnableSet(devNum, analyzerIndex,
                                                             enable);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Failed to enable mirroring for drop packets with error %d", status);
            return status;
        }
    }
    return status;
}

GT_STATUS cpssHalMirrorInit
(
    IN GT_U8 devId
)
{
    GT_STATUS status = GT_OK;
    int       devNum;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        devFamily = cpssHalDevPPFamilyGet(devNum);
        if (devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E &&
            devFamily != CPSS_PP_FAMILY_DXCH_AC5X_E)
        {
            status = cpssDxChMirrorToAnalyzerForwardingModeSet(devNum,
                                                               CPSS_DXCH_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E);
            if (status!= GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "Failed to mirror fwd mode set with error %d", status);
                return status;
            }
        }
    }

    return status;
}

GT_STATUS cpssHalMirrorErspanOAMFlowCounterClear
(
    IN GT_U8     devId,
    IN GT_U32    analyzerIndex
)
{
    GT_STATUS status = GT_OK;
    uint32_t indexStart = CPSSHAL_ERSPAN_EPLR_FLOW_ID_BASE +
                          (CPSSHAL_ERSPAN_EPCL_FLOW_ID_START - CPSSHAL_ERSPAN_EPLR_FLOW_ID_MIN);

    status = cpssHalPolicerCounterEntryClear(devId,
                                             CPSS_DXCH_POLICER_STAGE_EGRESS_E,
                                             (indexStart + analyzerIndex));
    if (status!= GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssHalPolicerCounterEntryClear fail:%d", status);
        return status;
    }

    return status;
}

GT_STATUS cpssHalMirrorErspanOAMFlowConfig
(
    IN GT_U8 devId
)
{
    GT_STATUS status = GT_OK;
    int       devNum;
    CPSS_DXCH_OAM_ENTRY_STC oamEntry;
    uint32_t index = 0;
    uint32_t oamIndex = CPSSHAL_ERSPAN_OAM_PROFILE_INDEX;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        if ((cpssHalDevPPFamilyGet(devNum) == CPSS_PP_FAMILY_DXCH_FALCON_E) ||
            (cpssHalDevPPFamilyGet(devNum) == CPSS_PP_FAMILY_DXCH_AC5X_E))
        {

            /* Configure OAM table for ERSPAN flow IDs */
            status = cpssDxChOamEnableSet(devNum, CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E,
                                          GT_TRUE);
            if (status!= GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChOamEnableSet fail:%d", status);
                return status;
            }
            memset(&oamEntry, 0, sizeof(oamEntry));

            for (index = CPSSHAL_ERSPAN_EPCL_FLOW_ID_START;
                 index < CPSSHAL_ERSPAN_EPCL_FLOW_ID_START+7; index++)
            {

                status = cpssDxChOamEntryGet(devNum, CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E, index,
                                             &oamEntry);
                if (status!= GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChOamEntryGet fail:%d", status);
                    return status;
                }

                oamEntry.oamPtpOffsetIndex      = 0;
                oamEntry.opcodeParsingEnable    = GT_FALSE;
                oamEntry.lmCountingMode         = CPSS_DXCH_OAM_LM_COUNTING_MODE_ENABLE_E;
                oamEntry.lmCounterCaptureEnable = GT_TRUE;

                status = cpssDxChOamEntrySet(devNum, CPSS_DXCH_OAM_STAGE_TYPE_EGRESS_E, index,
                                             &oamEntry);
                if (status!= GT_OK)
                {
                    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                          "cpssDxChOamEntrySet fail:%d", status);
                    return status;
                }
            }

            status = cpssDxChOamLmOffsetTableSet(devNum, oamIndex,
                                                 OAM_TLV_DATA_OFFSET_STC + 4*oamIndex);
            if (status!= GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChOamLmOffsetTableSet fail:%d", status);
                return status;
            }

            status = cpssDxChOamLmStampingEnableSet(devNum, GT_FALSE);
            if (status!= GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChOamLmStampingEnableSet fail:%d", status);
                return status;
            }

            /* Enable EPLR to count pkt based on flow id */

            CPSS_DXCH_POLICER_BILLING_INDEX_CFG_STC  dxChBillingIndexCfg;
            cpssOsMemSet(&dxChBillingIndexCfg, 0, sizeof(dxChBillingIndexCfg));
            dxChBillingIndexCfg.billingIndexMode =
                CPSS_DXCH_POLICER_BILLING_INDEX_MODE_FLOW_ID_E;
            dxChBillingIndexCfg.billingFlowIdIndexBase = CPSSHAL_ERSPAN_EPLR_FLOW_ID_BASE;
            dxChBillingIndexCfg.billingMinFlowId       = CPSSHAL_ERSPAN_EPLR_FLOW_ID_MIN;
            dxChBillingIndexCfg.billingMaxFlowId       = CPSSHAL_ERSPAN_EPLR_FLOW_ID_MAX;

            status = cpssDxChPolicerFlowIdCountingCfgSet(devNum,
                                                         CPSS_DXCH_POLICER_STAGE_EGRESS_E, &dxChBillingIndexCfg);
            if (status!= GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPolicerFlowIdCountingCfgSet fail:%d", status);
                return status;
            }

            /* enable LM counting */
            status = cpssDxChPolicerLossMeasurementCounterCaptureEnableSet(devNum,
                                                                           CPSS_DXCH_POLICER_STAGE_EGRESS_E, GT_TRUE);
            if (status!= GT_OK)
            {
                LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                      "cpssDxChPolicerLossMeasurementCounterCaptureEnableSet fail:%d", status);
                return status;
            }
        }
    }

    return status;
}


/**
* @internal cpssHalMirrorSampleRateGet function
* @endinternal
*
* @brief  This function gets Ingress statistical Mirroring
*         to the Ingress Analyzer Port including ratio per analyzer index.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer interface index. (APPLICABLE RANGES: 0..6)
* @param[out] enablePtr               - (pointer to) enable/disable statistical mirroring.
* @param[out] ratioPtr                - (pointer to) Indicates the  ratio of  mirrored traffic to analyzer port
*                                       ratio 0 -> no Rx mirroring.
*                                       ratio 1 -> all Rx mirrored packets are forwarded out the Rx Analyzer port.
*                                       ratio 31 -> 1 in 31 Rx mirrored packets are forwarded out the Rx  Analyzer port.
*                                                                                     32
*                                                         The maximum ratio value is 2    - 1
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or index.
* @retval GT_HW_ERROR              - reading HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalMirrorSampleRateGet
(
    IN GT_U8      devId,
    IN GT_U32     index,
    IN GT_BOOL    isIngress,
    OUT GT_U32    *ratioPtr
)
{
    GT_STATUS           status = GT_OK;
    int                 devNum;
    GT_BOOL             enable;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        if (isIngress)
        {
            status = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet(devNum, index,
                                                                          &enable, ratioPtr);
        }
        else
        {
            status = cpssDxChMirrorTxStatMirrorToAnalyzerIndexRatioGet(devNum, index,
                                                                       ratioPtr);
        }
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "Getting analyzer sample rate devNum=%d isIngress=%d failed.", devNum,
                  isIngress);
            return status;
        }
    }

    return status;
}


GT_STATUS cpssHalMirrorErspan2V6DummyTagSet(GT_U8 devId, GT_PORT_NUM ePort)
{
    int devNum;
    GT_STATUS status = GT_OK;
    GT_U32 tpidIndex = 6;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgVlanPortPushVlanCommandSet(devNum, ePort,
                                                       CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_TUNNEL_E);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanPortPushVlanCommandSet fail:%d", status);
            return status;
        }

        status = cpssDxChMirrorAnalyzerVlanTagEnable(devNum, ePort, GT_TRUE);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChMirrorAnalyzerVlanTagEnable fail:%d", status);
            return status;
        }

        status = cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet(devNum, ePort,
                                                                 tpidIndex+1);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet fail:%d", status);
            return status;
        }

        status = cpssDxChBrgVlanTpidTagTypeSet(devNum, CPSS_DIRECTION_EGRESS_E,
                                               tpidIndex, CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanTpidTagTypeSet fail:%d", status);
            return status;
        }

        status = cpssDxChBrgVlanTpidTagTypeSet(devNum, CPSS_DIRECTION_EGRESS_E,
                                               tpidIndex+1, CPSS_BRG_TPID_SIZE_TYPE_8_BYTES_E);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanTpidTagTypeSet fail:%d", status);
            return status;
        }

        status = cpssDxChBrgVlanPortEgressTpidSet(devNum, ePort,
                                                  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E, tpidIndex);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanPortEgressTpidSet fail:%d", status);
            return status;
        }

        status = cpssDxChBrgVlanPortEgressTpidSet(devNum, ePort,
                                                  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, tpidIndex+1);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanPortEgressTpidSet fail:%d", status);
            return status;
        }
    }

    return status;
}



GT_STATUS cpssHalMirrorErspan2V6DummyTagClear(GT_U8 devId, GT_PORT_NUM ePort)
{
    int devNum;
    GT_STATUS status = GT_OK;
    GT_U32 tpidIndex = 0;

    XPS_DEVICES_PER_SWITCH_ITER(devId, devNum)
    {
        status = cpssDxChBrgVlanPortPushVlanCommandSet(devNum, ePort,
                                                       CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_NONE_E);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanPortPushVlanCommandSet fail:%d", status);
            return status;
        }

        status = cpssDxChMirrorAnalyzerVlanTagEnable(devNum, ePort, GT_FALSE);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChMirrorAnalyzerVlanTagEnable fail:%d", status);
            return status;
        }

        status = cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet(devNum, ePort,
                                                                 tpidIndex);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet fail:%d", status);
            return status;
        }
#if 0
        status = cpssDxChBrgVlanTpidTagTypeSet(devNum, CPSS_DIRECTION_EGRESS_E,
                                               tpidIndex, CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanTpidTagTypeSet fail:%d", status);
            return status;
        }

        status = cpssDxChBrgVlanTpidTagTypeSet(devNum, CPSS_DIRECTION_EGRESS_E,
                                               tpidIndex+1, CPSS_BRG_TPID_SIZE_TYPE_8_BYTES_E);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanTpidTagTypeSet fail:%d", status);
            return status;
        }
#endif
        status = cpssDxChBrgVlanPortEgressTpidSet(devNum, ePort,
                                                  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E, tpidIndex);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanPortEgressTpidSet fail:%d", status);
            return status;
        }

        status = cpssDxChBrgVlanPortEgressTpidSet(devNum, ePort,
                                                  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, tpidIndex);
        if (status!= GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChBrgVlanPortEgressTpidSet fail:%d", status);
            return status;
        }
    }

    return status;
}

