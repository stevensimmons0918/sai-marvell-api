/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssDxChCatchUpDbg.c
*
* DESCRIPTION:
*       CPSS DxCh CatchUp debug utilities
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/catchUp/private/prvCpssDxChCatchUpDbg.h>
#include <cpss/generic/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define HWINIT_GLOVAR(_var) \
    PRV_SHARED_GLOBAL_VAR_GET(commonMod.genericHwInitDir._var)

#define PRV_GLOBAL_PPDRV_HWINIT(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.hwInitDir.hwInitSrc._var)

#define PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(_var)\
    PRV_NON_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.catchUpDir.catchUpDbgSrc._var)

/**
* @internal prvCpssDxChSystemRecoveryCatchUpRestorePpInfo function
* @endinternal
*
* @brief   Restore PP original info from ppConfigCatchUpParams
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*                                       None.
*/
static GT_STATUS prvCpssDxChSystemRecoveryCatchUpRestorePpInfo
(
    IN  GT_U8   devNum
)
{
    GT_U32                      cntrSetNum;
    GT_PHYSICAL_PORT_NUM        portNum;
    GT_U32                      portMacNum;
    GT_U32                      dpIndex;
    GT_BOOL                     isValid;
    GT_STATUS                   rc;
    GT_U32                      maxDp = 0;

    PRV_CPSS_HW_DEV_NUM_MAC(devNum) = PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].hwDevNum);
    PRV_CPSS_PP_MAC(devNum)->cpuPortMode = PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].cpuPortMode);
    PRV_CPSS_PP_MAC(devNum)->netifSdmaPortGroupId = PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].netifSdmaPortGroupId);
    PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.devTable = PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].devTable);
    PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.vlanMode = PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].vlanMode);
    PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.hashMode = PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].hashMode);
    PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.actionHwDevNum = PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].actionHwDevNum);
    PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.actionHwDevNumMask = PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].actionHwDevNumMask);
    PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.prePendTwoBytesHeader = PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].prePendTwoBytesHeader);
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[0] = PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].policerInfoMemSize[0]);
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[1] = PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].policerInfoMemSize[1]);
    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.securBreachDropCounterInfo.counterMode =
        PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].securBreachDropCounterInfoCounterMode);
    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.securBreachDropCounterInfo.portGroupId =
        PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].securBreachDropCounterInfoPortGroupId);
    for (cntrSetNum = 0; cntrSetNum < 2; cntrSetNum++)
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.portEgressCntrModeInfo[cntrSetNum].portGroupId =
            PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portEgressCntrModeInfoPortGroupId[cntrSetNum]);
        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.bridgeIngressCntrMode[cntrSetNum].portGroupId =
            PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].bridgeIngressCntrModePortGroupId[cntrSetNum]);
    }
    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.cfgIngressDropCntrMode.portGroupId =
        PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].cfgIngressDropCntrModePortGroupId);
    PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.tag1VidFdbEn = PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].tag1VidFdbEn);
    PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.portVlanfltTabAccessMode = PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portVlanfltTabAccessMode);

    if (!(PRV_CPSS_PP_MAC(devNum)->appDevFamily & (CPSS_XCAT_E | CPSS_XCAT2_E)))
    {
        for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            if (PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                rc = cpssDxChPortPhysicalPortMapIsValidGet(devNum, portNum, &isValid);
                if ((rc != GT_OK) || (isValid != GT_TRUE))
                {
                    continue;
                }
            }
            else
            {
                PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum, portNum);
            }

            if ((portNum == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                continue;
            }

            PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portIfMode = PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portIfMode[portMacNum]);
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portSpeed = PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portSpeed[portMacNum]);
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portType = PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portType[portMacNum]);
        }
    }
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        if (PRV_CPSS_PP_MAC(devNum)->tmInfo.tmDevIsInitilized == GT_FALSE)
        {
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.supportMultiDataPath)
            {
                maxDp = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp;
            }
            else
            {
                /* only first index valid*/
                maxDp = 1;
            }
            for (dpIndex = 0; dpIndex < maxDp; dpIndex++)
            {
                PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.usedDescCredits[dpIndex] =
                    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].usedDescCredits[dpIndex]);
                PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.usedHeaderCredits[dpIndex] =
                    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].usedHeaderCredits[dpIndex]);
                PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.usedPayloadCredits[dpIndex] =
                    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].usedPayloadCredits[dpIndex]);

                if( (PRV_CPSS_PP_MAC(devNum)->appDevFamily) & (CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E) )
                {
                    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[dpIndex] =
                        PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].coreOverallSpeedSummary[dpIndex]);
                    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[dpIndex] =
                        PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].coreOverallSpeedSummaryTemp[dpIndex]);
                }
            }
        }
    }

    PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset = PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portGtMacCounterOffset);
    if ( PRV_CPSS_SIP_5_CHECK_MAC(devNum) )
    {
        if ( PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ) /* Bobcat3 */
        {
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode = PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].lpmMemMode);
        }
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.indexForPbr = PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].lpmIndexForPbr);
    }
    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].isValidityCheckEnabled) = GT_FALSE;
    return GT_OK;
}

/**
* @internal prvCpssDxChSystemRecoveryCatchUpValidityCheckEnable function
* @endinternal
*
* @brief   Enable CatchUp validity check
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*
* @note Pre-condition for using this function:
*       CatchUp parameters in PP data structure must be equal to the HW values
*
*/
GT_STATUS prvCpssDxChSystemRecoveryCatchUpValidityCheckEnable
(
    IN  GT_U8   devNum
)
{
    GT_U32                          cntrSetNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    GT_U32                          portMacNum;
    GT_U32                          dpIndex;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;
    GT_BOOL                         isValid;
    GT_STATUS                       rc;
    GT_U32                          maxDp = 0;
    CPSS_PORT_MAC_COUNTERS_ENT      cntrIndex;
    GT_U32                          i;
    CPSS_SYSTEM_RECOVERY_INFO_STC   tempSystemRecovery_Info;

    cpssOsMemSet(&PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum]),0,sizeof(PRV_CPSS_DXCH_CATCHUP_PARAMS_STC));

    /* Store original info in ppConfigCatchUpParams */
    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].hwDevNum) = PRV_CPSS_HW_DEV_NUM_MAC(devNum);
    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].cpuPortMode) = PRV_CPSS_PP_MAC(devNum)->cpuPortMode;
    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].netifSdmaPortGroupId) = PRV_CPSS_PP_MAC(devNum)->netifSdmaPortGroupId;
    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].devTable) = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.devTable;
    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].vlanMode) = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.vlanMode;
    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].hashMode) = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.hashMode;
    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].actionHwDevNum) = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.actionHwDevNum;
    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].actionHwDevNumMask) = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.actionHwDevNumMask;
    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].prePendTwoBytesHeader) = PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.prePendTwoBytesHeader;
    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].policerInfoMemSize[0]) = PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[0];
    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].policerInfoMemSize[1]) = PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[1];
    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].securBreachDropCounterInfoCounterMode) =
        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.securBreachDropCounterInfo.counterMode;
    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].securBreachDropCounterInfoPortGroupId) =
        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.securBreachDropCounterInfo.portGroupId;
    for (cntrSetNum = 0; cntrSetNum < 2; cntrSetNum++)
    {
        PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portEgressCntrModeInfoPortGroupId[cntrSetNum]) =
            PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.portEgressCntrModeInfo[cntrSetNum].portGroupId;
        PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].bridgeIngressCntrModePortGroupId[cntrSetNum]) =
            PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.bridgeIngressCntrMode[cntrSetNum].portGroupId;
    }
    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].cfgIngressDropCntrModePortGroupId) =
        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.cfgIngressDropCntrMode.portGroupId;
    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].tag1VidFdbEn) = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.tag1VidFdbEn;
    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portVlanfltTabAccessMode) = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.portVlanfltTabAccessMode;
    if (!(PRV_CPSS_PP_MAC(devNum)->appDevFamily & (CPSS_XCAT_E | CPSS_XCAT2_E)))
    {
        for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            if (PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                rc = cpssDxChPortPhysicalPortMapIsValidGet(devNum, portNum, &isValid);
                if ((rc != GT_OK) || (isValid != GT_TRUE))
                {
                    continue;
                }
            }
            else
            {
                PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum, portNum);
            }

            if ((portNum == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                continue;
            }

            PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

            PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portIfMode[portMacNum]) = PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portIfMode;
            PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portSpeed[portMacNum]) = PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portSpeed;
            PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portType[portMacNum]) = PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portType;
        }
    }
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        if (PRV_CPSS_PP_MAC(devNum)->tmInfo.tmDevIsInitilized == GT_FALSE)
        {
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.supportMultiDataPath)
            {
                maxDp = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp;
            }
            else
            {
                /* only first index valid*/
                maxDp = 1;
            }

            for (dpIndex = 0; dpIndex < maxDp; dpIndex++)
            {
                PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].usedDescCredits[dpIndex]) =
                    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.usedDescCredits[dpIndex];
                PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].usedHeaderCredits[dpIndex]) =
                    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.usedHeaderCredits[dpIndex];
                PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].usedPayloadCredits[dpIndex]) =
                    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.usedPayloadCredits[dpIndex];

                if( (PRV_CPSS_PP_MAC(devNum)->appDevFamily) & (CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E) )
                {
                    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].coreOverallSpeedSummary[dpIndex]) =
                        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[dpIndex];
                    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].coreOverallSpeedSummaryTemp[dpIndex]) =
                        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[dpIndex];
                }
            }
        }
    }

    PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portGtMacCounterOffset) = PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset;
    if ( PRV_CPSS_SIP_5_CHECK_MAC(devNum) )
    {
        if ( PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ) /* Bobcat3 */
        {
            PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].lpmMemMode) = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode;
        }
        PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].lpmIndexForPbr) = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.indexForPbr;
    }
    /* Set all the parameters that catchup configures to chosen pattern */

    PRV_CPSS_PP_MAC(devNum)->cpuPortMode = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
    if ((PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum)) &&
        (!((PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE) && (PRV_CPSS_SIP_5_10_CHECK_MAC(devNum)))))
    {
        /* Lion2, Bobcat3 */
        PRV_CPSS_PP_MAC(devNum)->netifSdmaPortGroupId = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
    }
    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        /* xCat3, Lion2 */
        PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.devTable = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
    }
    PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.vlanMode = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.hashMode = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.actionHwDevNum = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.actionHwDevNumMask = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.prePendTwoBytesHeader = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[0] = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[1] = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.securBreachDropCounterInfo.counterMode = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.securBreachDropCounterInfo.portGroupId = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
    for (cntrSetNum = 0; cntrSetNum < 2; cntrSetNum++)
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.portEgressCntrModeInfo[cntrSetNum].portGroupId = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.bridgeIngressCntrMode[cntrSetNum].portGroupId = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
    }
    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.cfgIngressDropCntrMode.portGroupId = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.tag1VidFdbEn = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
        PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.portVlanfltTabAccessMode = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
    }
    if (!(PRV_CPSS_PP_MAC(devNum)->appDevFamily & (CPSS_XCAT_E | CPSS_XCAT2_E)))
    {
        for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            if (PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                rc = cpssDxChPortPhysicalPortMapIsValidGet(devNum, portNum, &isValid);
                if ((rc != GT_OK) || (isValid != GT_TRUE))
                {
                    continue;
                }
            }
            else
            {
                PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum, portNum);
            }

            if ((portNum == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                continue;
            }

            PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

            rc =  prvCpssDxChPortInterfaceModeHwGet(devNum, portNum, &ifMode);
            if (rc == GT_NOT_INITIALIZED)
            {
                continue;
            }
            if (rc != GT_OK)
            {
                prvCpssDxChSystemRecoveryCatchUpRestorePpInfo(devNum);
                return rc;
            }

            speed = PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portSpeed;

            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portIfMode = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portSpeed = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;

            rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E != tempSystemRecovery_Info.systemRecoveryProcess)
            {
                continue;
            }
            if (CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                if ((ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) || (speed == CPSS_PORT_SPEED_NA_E))
                {
                    continue;
                }
            }
            else
            {
                if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
                {
                    continue;
                }
            }
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portType = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
        }
    }
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        if (PRV_CPSS_PP_MAC(devNum)->tmInfo.tmDevIsInitilized == GT_FALSE)
        {
            for (dpIndex = 0; dpIndex < maxDp; dpIndex++)
            {
                PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.usedDescCredits[dpIndex] = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
                PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.usedHeaderCredits[dpIndex] = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
                PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.usedPayloadCredits[dpIndex] = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;

                if( (PRV_CPSS_PP_MAC(devNum)->appDevFamily) & (CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E) )
                {
                    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[dpIndex] = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
                    PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[dpIndex] = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
                }
            }
        }
    }

    if ( PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(isMacCounterOffsetInit) == GT_FALSE )
    {
        for (cntrIndex = CPSS_GOOD_OCTETS_RCV_E; cntrIndex < CPSS_LAST_MAC_COUNTER_NUM_E; cntrIndex++)
        {
            PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(gtMacCounterOffset[cntrIndex]) = PRV_CPSS_DXCH_CATCHUP_8_BIT_PATTERN_CNS;
        }
        PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(isMacCounterOffsetInit) = GT_TRUE;
    }
    PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset = PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(gtMacCounterOffset);
    if ( PRV_CPSS_SIP_5_CHECK_MAC(devNum) )
    {
        if ( PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ) /* Bobcat3 */
        {
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
        }
        PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.indexForPbr = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
    }

    /* For dual device id update DB of hw devices modes */
    if ( (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) &&
         (PRV_CPSS_DXCH_PP_HW_INFO_TXQ_UNITS_NUM_2_MAC(devNum) ) )
    {
        PRV_CPSS_HW_DEV_NUM_MAC(devNum) = PRV_CPSS_DXCH_CATCHUP_8_BIT_PATTERN_CNS-1;
        for (i = 0; i < BIT_12; i++)
        {
            PRV_GLOBAL_PPDRV_HWINIT(prvCpssHwDevNumModeDb[i]) = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
        }
    }
    else
    {
        PRV_CPSS_HW_DEV_NUM_MAC(devNum) = PRV_CPSS_DXCH_CATCHUP_PATTERN_CNS;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChSystemRecoveryCatchUpValidityCheck function
* @endinternal
*
* @brief   Validates that SW params hold the correct value from HW after CatchUp
*         process.
*
* @note   APPLICABLE DEVICES:      Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*
* @note To use this API, cpssDxChCatchUpValidityCheckEnable must be called before
*       performing CatchUp
*
*/
GT_STATUS prvCpssDxChSystemRecoveryCatchUpValidityCheck
(
    IN  GT_U8   devNum
)
{
    GT_U32                      cntrSetNum;
    GT_PHYSICAL_PORT_NUM        portNum;
    GT_U32                      portMacNum;
    GT_BOOL                     isValid;
    GT_STATUS                   rc = GT_OK;
    GT_STATUS                   rc1;
    GT_BOOL                     tmPortEnable = GT_FALSE;
    GT_BOOL                     isPortOob;
    CPSS_PORT_MAC_COUNTERS_ENT  cntrIndex;
    GT_U32                  dpIndex;
    GT_U32                  maxDp = 0;
#define RESTORE_AND_EXIT_ON_ERROR(_rc)  \
        rc = _rc;                       \
        CPSS_LOG_ERROR_MAC("Failed");   \
        goto restore


    /* Compare PP info with CatchUp parameters */
    if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].hwDevNum) != PRV_CPSS_HW_DEV_NUM_MAC(devNum))
    {
        RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
    }
    if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].cpuPortMode) != PRV_CPSS_PP_MAC(devNum)->cpuPortMode)
    {
        RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
    }
    if ((PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum)) &&
        (!((PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_TRUE) && (PRV_CPSS_SIP_5_10_CHECK_MAC(devNum)))))
    {
        /* Lion2, Bobcat3 */
        if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].netifSdmaPortGroupId) != PRV_CPSS_PP_MAC(devNum)->netifSdmaPortGroupId)
        {
            RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
        }
    }
    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum) == GT_FALSE)
    {
        /* xCat3, Lion2 */
        if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].devTable) != PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.devTable)
        {
            RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
        }
    }
    if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].vlanMode) != PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.vlanMode)
    {
        RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
    }
    if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].hashMode) != PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.fdbHashParams.hashMode)
    {
        RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
    }
    if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].actionHwDevNum) != PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.actionHwDevNum)
    {
        if (HWINIT_GLOVAR(sysGenGlobalInfo.supportDualDeviceId) == GT_FALSE)
        {
            RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
        }
        else
        {
            /* for devices supported dual device id ignore LSB in hw device number */
            if ( (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].actionHwDevNum) >> 1) != (PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.actionHwDevNum >>1) )
            {
                RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
            }
        }
    }
    if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].actionHwDevNumMask) != PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.actionHwDevNumMask)
    {
        if (HWINIT_GLOVAR(sysGenGlobalInfo.supportDualDeviceId) == GT_FALSE)
        {
            RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
        }
        else
        {
            /* for devices supported dual device id ignore LSB in hw device number */
            if ( (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].actionHwDevNumMask) >> 1) != (PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.actionHwDevNumMask >> 1) )
            {
                RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
            }
        }
    }
    if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].prePendTwoBytesHeader) != PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.prePendTwoBytesHeader)
    {
        RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
    }
    if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].policerInfoMemSize[0]) != PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[0])
    {
        RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
    }
    if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].policerInfoMemSize[1]) != PRV_CPSS_DXCH_PP_MAC(devNum)->policer.memSize[1])
    {
        RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
    }
    if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].securBreachDropCounterInfoCounterMode) !=
        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.securBreachDropCounterInfo.counterMode)
    {
        RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
    }
    if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].securBreachDropCounterInfoPortGroupId) !=
        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.securBreachDropCounterInfo.portGroupId)
    {
        RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
    }
    for (cntrSetNum = 0; cntrSetNum < 2; cntrSetNum++)
    {
        if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portEgressCntrModeInfoPortGroupId[cntrSetNum]) !=
            PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.portEgressCntrModeInfo[cntrSetNum].portGroupId)
        {
            RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
        }
        if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].bridgeIngressCntrModePortGroupId[cntrSetNum]) !=
            PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.bridgeIngressCntrMode[cntrSetNum].portGroupId)
        {
            RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
        }
    }
    if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].cfgIngressDropCntrModePortGroupId) !=
        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.cfgIngressDropCntrMode.portGroupId)
    {
        RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
    }
    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].tag1VidFdbEn) != PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.tag1VidFdbEn)
        {
            RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
        }
        if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portVlanfltTabAccessMode) != PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.portVlanfltTabAccessMode)
        {
            RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
        }
    }
    if (!(PRV_CPSS_PP_MAC(devNum)->appDevFamily & (CPSS_XCAT_E | CPSS_XCAT2_E)))
    {
        for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            if (PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(devNum) == GT_TRUE)
            {
                rc = cpssDxChPortPhysicalPortMapIsValidGet(devNum, portNum, &isValid);
                if ((rc != GT_OK) || (isValid != GT_TRUE))
                {
                    continue;
                }
            }
            else
            {
                PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum, portNum);
            }

            if ((portNum == CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                continue;
            }

            PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
            rc = prvCpssDxChIsPortOob(devNum, portMacNum, &isPortOob);
            if ( rc != GT_OK)
            {
                RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
            }
            else if ( isPortOob )
            {
                continue;
            }

            if( PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].valid )
            {
                if ( PRV_CPSS_DXCH_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].portMap.trafficManagerEn )
                {
                    tmPortEnable = GT_TRUE;
                }
            }

            if ( PRV_CPSS_DXCH_CATCHUP_DBG_IS_DEV_NOT_BOBK_OR_ALDRIN_SIMULATION_CHECK_MAC(devNum) )
            {
                if ( PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portIfMode[portMacNum]) != PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portIfMode )
                {
                    if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portIfMode[portMacNum]) == CPSS_PORT_INTERFACE_MODE_SR_LR_E)
                    {
                        /* there is a temporary problem to distinguish between SR_LP mode and KR mode */
                        if (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portIfMode != CPSS_PORT_INTERFACE_MODE_KR_E)
                        {
                            RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
                        }
                    }
                    else
                    {
                        RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
                    }
                }

                if ( PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portSpeed[portMacNum]) != PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portSpeed )
                {
                    RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
                }

            }
            if (CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
            {
                if ((PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portIfMode[portMacNum]) == CPSS_PORT_INTERFACE_MODE_NA_E) ||
                    (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portSpeed[portMacNum]) == CPSS_PORT_SPEED_NA_E))
                {
                    continue;
                }
            }
            else
            {
                if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portIfMode[portMacNum]) == CPSS_PORT_INTERFACE_MODE_NA_E)
                {
                    continue;
                }
            }

            if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portType[portMacNum]) != PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portType)
            {
                RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
            }
        }
    }

    for (cntrIndex = CPSS_GOOD_OCTETS_RCV_E; cntrIndex <= CPSS_DEFERRED_PKTS_SENT_E; cntrIndex++)
    {
        if( PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].portGtMacCounterOffset[cntrIndex]) != PRV_CPSS_DXCH_PP_MAC(devNum)->port.gtMacCounterOffset[cntrIndex] )
        {
            RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
        }
    }
    if ( PRV_CPSS_SIP_5_CHECK_MAC(devNum) )
    {
        if ( PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) ) /* Bobcat3 */
        {
            if ( PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].lpmMemMode) != PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode )
            {
                RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
            }
        }
        if ( PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].lpmIndexForPbr) != PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.indexForPbr )
        {
            RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
        }
    }
    if(PRV_CPSS_PP_MAC(devNum)->isGmDevice)
    {
        /* dummy line to avoid warning in GM compilation. */
        tmPortEnable = tmPortEnable;
    }

    rc = GT_OK;

    if(PRV_CPSS_PP_MAC(devNum)->isGmDevice)
    {
        /* do nothing */
    }
    else
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        if (PRV_CPSS_PP_MAC(devNum)->tmInfo.tmDevIsInitilized == GT_FALSE)
        {
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.supportMultiDataPath)
            {
                maxDp = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp;
            }
            else
            {
                /* only first index valid*/
                maxDp = 1;
            }

            for (dpIndex = 0; dpIndex < maxDp; dpIndex++)
            {
                if ( PRV_CPSS_DXCH_CATCHUP_DBG_IS_DEV_NOT_BOBK_OR_ALDRIN_SIMULATION_CHECK_MAC(devNum) )
                {
                    if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].usedDescCredits[dpIndex]) !=
                        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.usedDescCredits[dpIndex])
                    {
                        RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
                    }

                    if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].usedHeaderCredits[dpIndex]) !=
                        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.usedHeaderCredits[dpIndex])
                    {
                        RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
                    }

                    if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].usedPayloadCredits[dpIndex]) !=
                        PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.usedPayloadCredits[dpIndex])
                    {
                        RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
                    }

                    if( (PRV_CPSS_PP_MAC(devNum)->appDevFamily) & (CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E) )
                    {
                        if ( tmPortEnable == GT_FALSE )
                        {
                            continue;
                        }
                        if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].coreOverallSpeedSummary[dpIndex]) !=
                                PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummary[dpIndex])
                        {
                            RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
                        }
                        if (PRV_CATCH_UP_DBG_GLOBAL_VAR_GET(ppConfigCatchUpParams[devNum].coreOverallSpeedSummaryTemp[dpIndex]) !=
                                PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.groupResorcesStatus.coreOverallSpeedSummaryTemp[dpIndex])
                        {
                            RESTORE_AND_EXIT_ON_ERROR(GT_FAIL);
                        }
                    }
                }
            }
        }
    }

restore:
    /* Restore PP original info */
    rc1 = prvCpssDxChSystemRecoveryCatchUpRestorePpInfo(devNum);
    if (rc == GT_OK)
    {
        rc = rc1;
    }
    return rc;
}



